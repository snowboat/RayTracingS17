#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "scene.h"

class Light
	: public SceneElement
{
public:
	virtual vec3f shadowAttenuation(const vec3f& P) const = 0;
	virtual double distanceAttenuation( const vec3f& P ) const = 0;
	virtual vec3f getColor( const vec3f& P ) const = 0;
	virtual vec3f getDirection( const vec3f& P ) const = 0;

protected:
	Light( Scene *scene, const vec3f& col )
		: SceneElement( scene ), color( col ) {}

	vec3f 		color;
};

class DirectionalLight
	: public Light
{
public:
	DirectionalLight( Scene *scene, const vec3f& orien, const vec3f& color )
		: Light( scene, color ), orientation( orien ) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation( const vec3f& P ) const;
	virtual vec3f getColor( const vec3f& P ) const;
	virtual vec3f getDirection( const vec3f& P ) const;

protected:
	vec3f 		orientation;
};

class PointLight
	: public Light
{
public:
	PointLight( Scene *scene, const vec3f& pos, const vec3f& color )
		: Light( scene, color ), position( pos ), constant_attenuation_coeff(0.0), linear_attenuation_coeff(0.0), quadratic_attenuation_coeff(0.0) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation( const vec3f& P ) const;
	virtual vec3f getColor( const vec3f& P ) const;
	virtual vec3f getDirection( const vec3f& P ) const;
	double constant_attenuation_coeff;
	double linear_attenuation_coeff;
	double quadratic_attenuation_coeff;

protected:
	vec3f position;
};

//class SpotLight : public Light {
//public: 
//	SpotLight(Scene* scene, const vec3f& pos, const vec3f& color, double& ang) :
//		Light(scene, color), position(pos), angle(ang) {}
//
//
//};

class SpotLight : public PointLight {
public:
	SpotLight(Scene* scene, const vec3f& pos, const vec3f& color, const double& ang, const vec3f& centralDir):
		PointLight(scene, pos, color), angle(ang), centralDirection(centralDir.normalize()){}
	virtual vec3f getColor(const vec3f& p) const;

protected:
	double angle;	//the range of angle within which the Spot Light is considered bright.
	vec3f centralDirection;	//self-explanatory
};

#endif // __LIGHT_H__
