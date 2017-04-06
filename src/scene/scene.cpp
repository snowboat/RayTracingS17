#include <cmath>

#include "scene.h"
#include "light.h"
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

void BoundingBox::operator=(const BoundingBox& target)
{
	min = target.min;
	max = target.max;
}

// Does this bounding box intersect the target?
bool BoundingBox::intersects(const BoundingBox &target) const
{
	return ((target.min[0] - RAY_EPSILON <= max[0]) && (target.max[0] + RAY_EPSILON >= min[0]) &&
			(target.min[1] - RAY_EPSILON <= max[1]) && (target.max[1] + RAY_EPSILON >= min[1]) &&
			(target.min[2] - RAY_EPSILON <= max[2]) && (target.max[2] + RAY_EPSILON >= min[2]));
}

// does the box contain this point?
bool BoundingBox::intersects(const vec3f& point) const
{
	return ((point[0] + RAY_EPSILON >= min[0]) && (point[1] + RAY_EPSILON >= min[1]) && (point[2] + RAY_EPSILON >= min[2]) &&
		 (point[0] - RAY_EPSILON <= max[0]) && (point[1] - RAY_EPSILON <= max[1]) && (point[2] - RAY_EPSILON <= max[2]));
}

// if the ray hits the box, put the "t" value of the intersection
// closest to the origin in tMin and the "t" value of the far intersection
// in tMax and return true, else return false.
// Using Kay/Kajiya algorithm.
bool BoundingBox::intersect(const ray& r, double& tMin, double& tMax) const
{
	vec3f R0 = r.getPosition();
	vec3f Rd = r.getDirection();

	tMin = -1.0e308; // 1.0e308 is close to infinity... close enough for us!
	tMax = 1.0e308;
	double ttemp;
	
	for (int currentaxis = 0; currentaxis < 3; currentaxis++)
	{
		double vd = Rd[currentaxis];
		
		// if the ray is parallel to the face's plane (=0.0)
		if( vd == 0.0 )
			continue;

		double v1 = min[currentaxis] - R0[currentaxis];
		double v2 = max[currentaxis] - R0[currentaxis];

		// two slab intersections
		double t1 = v1/vd;
		double t2 = v2/vd;
		
		if ( t1 > t2 ) { // swap t1 & t2
			ttemp = t1;
			t1 = t2;
			t2 = ttemp;
		}

		if (t1 > tMin)
			tMin = t1;
		if (t2 < tMax)
			tMax = t2;

		if (tMin > tMax) // box is missed
			return false;
		if (tMax < 0.0) // box is behind ray
			return false;
	}
	return true; // it made it past all 3 axes.
}


bool Geometry::intersect(const ray&r, isect&i) const
{
    // Transform the ray into the object's local coordinate space
    vec3f pos = transform->globalToLocalCoords(r.getPosition());
    vec3f dir = transform->globalToLocalCoords(r.getPosition() + r.getDirection()) - pos;
    double length = dir.length();
    dir /= length;

    ray localRay( pos, dir );

    if (intersectLocal(localRay, i)) {	//send this iscet point to the intersect local function.
        // Transform the intersection point & normal returned back into global space.
		i.N = transform->localToGlobalCoordsNormal(i.N);
		i.t /= length;

		return true;
    } else {
        return false;
    }
    
}

bool Geometry::intersectLocal( const ray& r, isect& i ) const
{
	return false;	//this method is gonna be overwritten by Geometry's subclasses.
}



bool Geometry::hasBoundingBoxCapability() const
{
	// by default, primitives do not have to specify a bounding box.
	// If this method returns true for a primitive, then either the ComputeBoundingBox() or
    // the ComputeLocalBoundingBox() method must be implemented.

	// If no bounding box capability is supported for an object, that object will
	// be checked against every single ray drawn.  This should be avoided whenever possible,
	// but this possibility exists so that new primitives will not have to have bounding
	// boxes implemented for them.
	
	return false;
}

Scene::~Scene()
{
    giter g;
    liter l;
    
	for( g = objects.begin(); g != objects.end(); ++g ) {
		delete (*g);
	}

	for( g = boundedobjects.begin(); g != boundedobjects.end(); ++g ) {
		delete (*g);
	}

	for( g = nonboundedobjects.begin(); g != boundedobjects.end(); ++g ) {
		delete (*g);
	}

	for( l = lights.begin(); l != lights.end(); ++l ) {
		delete (*l);
	}
}

// Get any intersection with an object.  Return information about the 
// intersection through the reference parameter.
bool Scene::intersect( const ray& r, isect& i ) const
{
	typedef list<Geometry*>::const_iterator iter;
	iter j;

	isect cur;
	bool have_one = false;

	// try the non-bounded objects
	for( j = nonboundedobjects.begin(); j != nonboundedobjects.end(); ++j ) {
		if( (*j)->intersect( r, cur ) ) {
			if( !have_one || (cur.t < i.t) ) {
				i = cur;
				have_one = true;
			}
		}
	}

	// try the bounded objects
	for( j = boundedobjects.begin(); j != boundedobjects.end(); ++j ) {
		if( (*j)->intersect( r, cur ) ) {
			if( !have_one || (cur.t < i.t) ) {
				i = cur;
				have_one = true;
			}
		}
	}


	return have_one;
}

void Scene::initScene()
{
	bool first_boundedobject = true;
	BoundingBox b;
	ambientLight = vec3f(1.0, 1.0, 1.0);
	
	typedef list<Geometry*>::const_iterator iter;
	// split the objects into two categories: bounded and non-bounded
	for( iter j = objects.begin(); j != objects.end(); ++j ) {
		if( (*j)->hasBoundingBoxCapability() )
		{
			boundedobjects.push_back(*j);

			// widen the scene's bounding box, if necessary
			if (first_boundedobject) {
				sceneBounds = (*j)->getBoundingBox();
				first_boundedobject = false;
			}
			else
			{
				b = (*j)->getBoundingBox();
				sceneBounds.max = maximum(sceneBounds.max, b.max);
				sceneBounds.min = minimum(sceneBounds.min, b.min);
			}
		}
		else
			nonboundedobjects.push_back(*j);
	}
}

void Scene::setTexture(unsigned char * tex)
{
	this->textureImg = tex;
}

unsigned char * Scene::getTexture()
{
	return this->textureImg;
}

void Scene::setTextureWidth(int w)
{
	this->textureWidth = w;
}

void Scene::setTextureHeight(int h)
{
	textureHeight = h;
}

int Scene::getTextureWidth()
{
	return this->textureWidth;
}

int Scene::getTextureHeight()
{
	return this->textureHeight;
}

vec3f Scene::getTextureColor(double x, double y)
{
	if (this->getTexture() && x >= 0 && x <= 1 && y >= 0 && y <= 1) {

		int textureWidth = this->getTextureWidth();
		int textureHeight = this->getTextureHeight();

		int pixelx = min(textureWidth - 1, int(x*textureWidth));
		int pixely = min(textureHeight - 1, int(y*textureHeight));
		unsigned char* pixel = this->getTexture() + (pixelx + pixely*textureWidth) * 3;
		int r = (int)*pixel;
		int g = (int)*(pixel + 1);
		int b = (int)*(pixel + 2);
		return vec3f((float)r / float(255), (float)g / 255.0f, (float)b / 255.0f).clamp();
	}
	else {
		//cout << "caused by " << x << " " << y << endl;
		return vec3f(0.0f, 0.0f, 0.0f);
	}
}

void Scene::setSoftShadow(bool sofSha)
{
	this->softShadow = sofSha;
}

bool Scene::getSoftShadow()
{
	return this->softShadow;
}

void Scene::setSoftShadowCoeff(double coeff)
{
	this->softShadowCoeff = coeff;
}

double Scene::getSoftShadowCoeff()
{
	return this->softShadowCoeff;
}


