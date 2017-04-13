//
// scene.h
//
// The Scene class and the geometric types that it can contain.
//

#ifndef __SCENE_H__
#define __SCENE_H__

#include <list>
#include <algorithm>

using namespace std;

#include "ray.h"
#include "material.h"
#include "camera.h"
#include "../vecmath/vecmath.h"
#include <vector>

class Light;
class Scene;

class SceneElement
{
public:
	virtual ~SceneElement() {}

	Scene *getScene() const { return scene; }

protected:
	SceneElement( Scene *s )
		: scene( s ) {}

    Scene *scene;
};

class BoundingBox
{
public:
	vec3f min;
	vec3f max;

	void operator=(const BoundingBox& target);

	// Does this bounding box intersect the target?
	bool intersects(const BoundingBox &target) const;
	
	// does the box contain this point?
	bool intersects(const vec3f& point) const;

	// if the ray hits the box, put the "t" value of the intersection
	// closest to the origin in tMin and the "t" value of the far intersection
	// in tMax and return true, else return false.
	bool intersect(const ray& r, double& tMin, double& tMax) const;
};

class TransformNode
{
protected:

    // information about this node's transformation
    mat4f    xform;
	mat4f    inverse;
	mat3f    normi;

    // information about parent & children
    TransformNode *parent;
    list<TransformNode*> children;
    
public:
   	typedef list<TransformNode*>::iterator          child_iter;
	typedef list<TransformNode*>::const_iterator    child_citer;

	mat4f getXform();
	void setXform(mat4f newxform);

    ~TransformNode()
    {
        for(child_iter c = children.begin(); c != children.end(); ++c )
            delete (*c);
    }

    TransformNode *createChild(const mat4f& xform)
    {
        TransformNode *child = new TransformNode(this, xform);
        children.push_back(child);
        return child;
    }
    
    // Coordinate-Space transformation
    vec3f globalToLocalCoords(const vec3f &v)
    {
        return inverse * v;
    }

    vec3f localToGlobalCoords(const vec3f &v)
    {
        return xform * v;
    }

    vec4f localToGlobalCoords(const vec4f &v)
    {
        return xform * v;
    }

    vec3f localToGlobalCoordsNormal(const vec3f &v)
    {
        return (normi * v).normalize();
    }

protected:
    // protected so that users can't directly construct one of these...
    // force them to use the createChild() method.  Note that they CAN
    // directly create a TransformRoot object.
    TransformNode(TransformNode *parent, const mat4f& xform ): children()
    {
        this->parent = parent;
        if (parent == NULL)
            this->xform = xform;
        else
            this->xform = parent->xform * xform;
        
        inverse = this->xform.inverse();
        normi = this->xform.upper33().inverse().transpose();
    }
};

class TransformRoot : public TransformNode
{
public:
    TransformRoot()
        : TransformNode(NULL, mat4f()) {}
};

// A Geometry object is anything that has extent in three dimensions.
// It may not be an actual visible scene object.  For example, hierarchical
// spatial subdivision could be expressed in terms of Geometry instances.
class Geometry: public SceneElement
{
public:
    // intersections performed in the global coordinate space.
    virtual bool intersect(const ray&r, isect&i) const;
    
    // intersections performed in the object's local coordinate space
    // do not call directly - this should only be called by intersect()
	virtual bool intersectLocal( const ray& r, isect& i ) const;


	virtual bool hasBoundingBoxCapability() const;
	const BoundingBox& getBoundingBox() const { return bounds; }
	virtual void ComputeBoundingBox()
    {
        // take the object's local bounding box, transform all 8 points on it,
        // and use those to find a new bounding box.

        BoundingBox localBounds = ComputeLocalBoundingBox();
        
        vec3f min = localBounds.min;
		vec3f max = localBounds.max;

		vec4f v, newMax, newMin;

		v = transform->localToGlobalCoords( vec4f(min[0], min[1], min[2], 1) );
		newMax = v;
		newMin = v;
		v = transform->localToGlobalCoords( vec4f(max[0], min[1], min[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		v = transform->localToGlobalCoords( vec4f(min[0], max[1], min[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		v = transform->localToGlobalCoords( vec4f(max[0], max[1], min[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		v = transform->localToGlobalCoords( vec4f(min[0], min[1], max[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		v = transform->localToGlobalCoords( vec4f(max[0], min[1], max[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		v = transform->localToGlobalCoords( vec4f(min[0], max[1], max[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		v = transform->localToGlobalCoords( vec4f(max[0], max[1], max[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		
		bounds.max = vec3f(newMax);
		bounds.min = vec3f(newMin);
    }

    // default method for ComputeLocalBoundingBox returns a bogus bounding box;
    // this should be overridden if hasBoundingBoxCapability() is true.
    virtual BoundingBox ComputeLocalBoundingBox() { return BoundingBox(); }

    void setTransform(TransformNode *transform) { this->transform = transform; };
	TransformNode* getTransformNode();

	Geometry( Scene *scene ) : SceneElement( scene ) {}

protected:
	BoundingBox bounds;
    TransformNode *transform;
};

// A SceneObject is a real actual thing that we want to model in the 
// world.  It has extent (its Geometry heritage) and surface properties
// (its material binding).  The decision of how to store that material
// is left up to the subclass.
class SceneObject: public Geometry
{
public:
	virtual const Material& getMaterial() const = 0;
	virtual void setMaterial( Material *m ) = 0;
	virtual bool getLocalUV(const ray& r, const isect& i, double& u, double& v) const { return false; }// returns true only if this sceneobject supports texture mapping
	virtual bool preturbNormal(const ray& r, isect& i, const double& u, const double& v, unsigned char* preturbImg, const int& imgWidth, const int& imgHeight, Scene* scene) const {
		return false;
	}


protected:
	SceneObject( Scene *scene )
		: Geometry( scene ) {}
};






// A simple extension of SceneObject that adds an instance of Material
// for simple material bindings.
class MaterialSceneObject: public SceneObject
{
public:
	virtual ~MaterialSceneObject() { if( material ) delete material; }

	virtual const Material& getMaterial() const { return *material; }
	virtual void setMaterial( Material *m )	{ material = m; }
	virtual bool getLocalUV(const ray& r, const isect& i, double& u, double& v) const {
		return false;
	}	// returns true only if this sceneobject supports texture mapping
	virtual bool preturbNormal(const ray& r, isect& i, const double& u, const double& v, unsigned char* preturbImg, const int& imgWidth, const int& imgHeight, Scene* scene) const {
		return false;
	}
protected:
	MaterialSceneObject( Scene *scene, Material *mat ) 
		: SceneObject( scene ), material( mat ) {}
    //	MaterialSceneObject( Scene *scene ) 
	//	: SceneObject( scene ), material( new Material ) {}

	Material *material;
};













class Scene
{
public:
	typedef list<Light*>::iterator 			liter;
	typedef list<Light*>::const_iterator 	cliter;

	typedef list<Geometry*>::iterator 		giter;	//iterator of all Geometries in the scene
	typedef list<Geometry*>::const_iterator cgiter;

    TransformRoot transformRoot;

	// Ia
	vec3f ambientLight;	//TODO: ADD SLIDER TO CONTRol

public:
	Scene() 
		: transformRoot(), objects(), lights() {
		constAttenFactor = 1.0;
		linearAttenFactor = 1.0;
		quadAttenFactor = 1.0;
		textureImg = NULL;
		heightFieldColor = NULL;
		heightFieldIntensity = nullptr;
		terminationThreshold = 1.0;
		ambientLight = vec3f(1.0, 1.0, 1.0);
		accShadowAttenThresh = 0.0;
	}
	virtual ~Scene();

	void add( Geometry* obj )
	{
		obj->ComputeBoundingBox();
		objects.push_back( obj );
	}
	void add( Light* light )
	{ lights.push_back( light ); }

	bool intersect( const ray& r, isect& i ) const;
	void initScene();

	list<Light*>::const_iterator beginLights() const { return lights.begin(); }
	list<Light*>::const_iterator endLights() const { return lights.end(); }
	list<Geometry*>::iterator beginGeometries()  { return objects.begin(); }
	list<Geometry*>::iterator endGeometries() { return objects.end(); }



	Camera *getCamera() { return &camera; }

	double constAttenFactor;
	double linearAttenFactor;
	double quadAttenFactor;
	void setTexture(unsigned char* tex);
	unsigned char* getTexture();
	void setTextureWidth(int w);
	void setTextureHeight(int h);
	int getTextureWidth();
	int getTextureHeight();
	vec3f getTextureColor(double x, double y);
	vec3f getBitmapColor(unsigned char* bitmap, int bmpwidth, int bmpheight, double x, double y);	//given two values 0.0~1.0, returns the corresponding color in bitmap
	vec3f getBitmapColorFromPixel(unsigned char* bitmap, int bmpwidth, int bmpheight, int x, int y);
	double getPixelIntensity(unsigned char* bitmap, int bmpwidth, int bmpheight, int x, int y);

	void setSoftShadow(bool sofSha);
	bool getSoftShadow();
	void setSoftShadowCoeff(double coeff);
	double getSoftShadowCoeff();

	void setGlossyReflection(bool glossy);
	bool getGlossyReflection();

	void setMotionBlur(bool mb);
	bool getMotionBlur();

	unsigned char* getHFIntensityImg();
	void setHFIntensityImg(unsigned char* hfi, int hfw, int hfh);
	unsigned char* getHFColorImg();
	void setHFColorImg(unsigned char* hfc);

	void showHeightField();
	void setTextureMapping(bool tm);
	bool getTextureMapping();
	bool	bumpMapping;
	double accShadowAttenThresh;

	void preturbNormal(vec3f& normal, const ray& r, double& u, double& v);
	
	double getTerimnationThreshold();
	void setTerminationThreshold(double terThresh);

private:
    list<Geometry*> objects;
	list<Geometry*> nonboundedobjects;
	list<Geometry*> boundedobjects;
    list<Light*> lights;
    Camera camera;
	bool textureMapping;
	unsigned char* textureImg;	//texture image, shared with the one loaded to Trace UI
	int textureWidth;
	int textureHeight;

	bool	softShadow;
	bool	glossyReflection;
	bool	motionBlur;
	double	softShadowCoeff;

	unsigned char* heightFieldIntensity;
	unsigned char* heightFieldColor;
	int hfWidth;
	int hfHeight;
	double terminationThreshold;

	// Each object in the scene, provided that it has hasBoundingBoxCapability(),
	// must fall within this bounding box.  Objects that don't have hasBoundingBoxCapability()
	// are exempt from this requirement.
	BoundingBox sceneBounds;
};

#endif // __SCENE_H__
