// The main ray tracer.

#include <Fl/fl_ask.h>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"
#include <math.h> 

const double PI = 3.14159265358979323846264338327950288;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace( Scene *scene, double x, double y )
{
    ray r( vec3f(0,0,0), vec3f(0,0,0) );
    scene->getCamera()->rayThrough( x,y,r );
	return traceRay( scene, r, vec3f(1.0,1.0,1.0), 0, true ).clamp();
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay( Scene *scene, const ray& r, 
	const vec3f& thresh, int depth, bool fromAir )
{
	isect i;

	if( scene->intersect( r, i ) ) {
		// YOUR CODE HERE

		// An intersection occured!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.  

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.



		const Material& m = i.getMaterial();

		//Reflection component
		ray reflecRay = r; 
		reflecRay.setPosition(r.at(i.t));
		reflecRay.setDirection(    (r.getDirection().normalize() + i.N.normalize()).normalize()   );
		vec3f reflecColor = { 0.0f,0.0f,0.0f };
		if (depth < depthLimit) {
			reflecColor = prod(traceRay(scene, reflecRay, thresh, depth + 1, fromAir), i.getMaterial().kr);
		}

		//TODO: Refractive component
		ray refracRay = r;
		refracRay.setPosition(r.at(i.t));
		double yeetat = 0.0;
		if (fromAir) {
			double gammai = 1;
			double yeetai = acos(-r.getDirection()*i.N);
			double gammat = i.getMaterial().index;
			yeetat = asin(gammai*sin(yeetai) / gammat);

			vec3f rayProjNor = ((r.getDirection().normalize())* i.N) * i.N;	//projection of ray onto the normal
			vec3f horDirection = r.getDirection() - rayProjNor;	//unit vector in horizontal direction
			horDirection = horDirection.normalize();
			vec3f verDirection = -i.N.normalize();
			
			refracRay.setDirection((horDirection*sin(yeetat) + verDirection * cos(yeetat)).normalize());

		}
		else {
			double gammai = m.index;
			double gammat = 1;
			double yeetai = acos(-r.getDirection() * i.N);
			yeetat = asin(gammat*sin(yeetai) / gammai);
			
			vec3f rayProjNor = ((r.getDirection().normalize())* i.N) * i.N;	//projection of ray onto the normal

			vec3f horDir = r.getDirection() - rayProjNor;
			horDir = horDir.normalize();
			vec3f verDir = -i.N.normalize();
			refracRay.setDirection((horDir*sin(yeetat) + verDir * cos(yeetat)).normalize());
			cout << refracRay.getDirection() << endl;
		}
		vec3f refracColor = { 0.0f, 0.0f, 0.0f };
		if (depth < depthLimit && yeetat > 0 && yeetat < PI/2) {
			refracColor = prod(traceRay(scene, refracRay, thresh, depth + 1, !fromAir), i.getMaterial().kt);
			//cout << refracColor << endl;
		}

		return prod(m.shade(scene, r, i),(vec3f(1.0f,1.0f,1.0f)-m.kt)) +reflecColor + refracColor;
	
	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.

		return vec3f( 0.0, 0.0, 0.0 );
	}
}

RayTracer::RayTracer()
{
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;

	m_bSceneLoaded = false;
	depthLimit = 0;
}


RayTracer::~RayTracer()
{
	delete [] buffer;
	delete scene;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return scene ? scene->getCamera()->getAspectRatio() : 1;
}

bool RayTracer::sceneLoaded()
{
	return m_bSceneLoaded;
}

Scene * RayTracer::getScene()
{
	return this->scene;
}

void RayTracer::setDepthLimit(int depthLim)
{   
	if (depthLimit != depthLim) {
		depthLimit = depthLim;
	}
}

bool RayTracer::loadScene( char* fn )
{
	try
	{
		scene = readScene( fn );
	}
	catch( ParseError pe )
	{
		fl_alert( "ParseError: %s\n", pe );
		return false;
	}

	if( !scene )
		return false;
	
	buffer_width = 256;
	buffer_height = (int)(buffer_width / scene->getCamera()->getAspectRatio() + 0.5);

	bufferSize = buffer_width * buffer_height * 3;
	buffer = new unsigned char[ bufferSize ];
	
	// separate objects into bounded and unbounded
	scene->initScene();
	
	// Add any specialized scene loading code here
	
	m_bSceneLoaded = true;

	return true;
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];
	}
	memset( buffer, 0, w*h*3 );
}

void RayTracer::traceLines( int start, int stop )
{
	vec3f col;
	if( !scene )
		return;

	if( stop > buffer_height )
		stop = buffer_height;

	for( int j = start; j < stop; ++j )
		for( int i = 0; i < buffer_width; ++i )
			tracePixel(i,j);
}

void RayTracer::tracePixel( int i, int j )
{
	vec3f col;

	if( !scene )
		return;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	col = trace( scene,x,y );

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}