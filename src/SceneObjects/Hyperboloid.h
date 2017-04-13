#pragma once
#ifndef __Hyperboloid_H__
#define __Hyperboloid_H__

#include "../scene/scene.h"

class Hyperboloid
	: public MaterialSceneObject
{
public:
	Hyperboloid(Scene *scene, Material *mat)
		: MaterialSceneObject(scene, mat)
	{
	}

	virtual bool intersectLocal(const ray& r, isect& i) const;
	virtual bool hasBoundingBoxCapability() const { return true; }

	virtual bool getLocalUV(const ray& r, const isect& i, double& u, double& v) const;	// returns true only if this sceneobject supports texture mapping

	bool getLocalPertubation(const ray& r, isect& i, double& u, double& v);

	virtual BoundingBox ComputeLocalBoundingBox()
	{
		BoundingBox localbounds;
		localbounds.min = vec3f(-1.0f, -1.0f, -1.0f);
		localbounds.max = vec3f(1.0f, 1.0f, 1.0f);
		return localbounds;
	}
};
#endif // __Hyperboloid_H__
