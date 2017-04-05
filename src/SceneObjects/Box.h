#ifndef __BOX_H__
#define __BOX_H__

#include "../scene/scene.h"

class Box
	: public MaterialSceneObject
{
public:
	Box( Scene *scene, Material *mat )
		: MaterialSceneObject( scene, mat )
	{
	}

	virtual bool intersectLocal( const ray& r, isect& i ) const;
	virtual bool hasBoundingBoxCapability() const { return true; }
	virtual bool getLocalUV(const ray& r, const isect& i, double& u, double& v) const;	// returns true only if this sceneobject supports texture mapping

    virtual BoundingBox ComputeLocalBoundingBox()
    {
        BoundingBox localbounds;
        localbounds.max = vec3f(0.5, 0.5, 0.5);
		localbounds.min = vec3f(-0.5, -0.5, -0.5);
        return localbounds;
    }
};

#endif // __BOX_H__
