#include <cmath>

#include "light.h"

#include <FL/fl_ask.H>

double DirectionalLight::distanceAttenuation( const vec3f& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


vec3f DirectionalLight::shadowAttenuation( const vec3f& P ) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.
	ray r(P, -orientation);

	isect i;
	if (scene->intersect(r, i)) {
		const Material& m = i.getMaterial();
		return m.kt;
	}

    return vec3f(1,1,1);
}

vec3f DirectionalLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f DirectionalLight::getDirection( const vec3f& P ) const
{
	return -orientation;
}

vec3f DirectionalLight::shadowAttenuationSoft(const vec3f & P, double coeff) const
{
	return shadowAttenuation(P);	//NO soft shadow in directional light
}


double PointLight::distanceAttenuation(const vec3f& P) const
{
	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, I assume no attenuation and just return 1.0
	


	if (constant_attenuation_coeff > 0.0 || linear_attenuation_coeff > 0.0 || quadratic_attenuation_coeff > 0.0) {
		cout << constant_attenuation_coeff << " - -" << linear_attenuation_coeff << endl;
		double distance = (position - P).length();
		return min(1.0, 1.0 / (constant_attenuation_coeff*scene->constAttenFactor + 
			linear_attenuation_coeff*distance* scene->linearAttenFactor + 
			quadratic_attenuation_coeff*distance*distance * scene->quadAttenFactor));
	}
	return 1.0;
}

vec3f PointLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}





vec3f PointLight::getDirection( const vec3f& P ) const
{
	return (position - P).normalize();
}


vec3f PointLight::shadowAttenuation(const vec3f& P) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.
	vec3f d = (position - P).normalize();
	ray r(P, d);

	double distance = (position - P).length();

	isect i;
	if (scene->intersect(r, i)) {
		if (i.t <= distance) {
			const Material& m = i.getMaterial();
			return m.kt;
		}
	}
    return vec3f(1,1,1);
}

vec3f PointLight::shadowAttenuationSoft(const vec3f & P, double coeff) const
{
	vec3f attenColor(0.0, 0.0, 0.0);
	for (int i = 0; i < 150; i++) {
		double area = coeff;
		vec3f newPos = position + area * vec3f((double(rand()) / double(RAND_MAX)), (double(rand()) / double(RAND_MAX)), (double(rand()) / double(RAND_MAX)));
		PointLight newLight(scene, newPos, color);
		attenColor += newLight.shadowAttenuation(P);
	}
	return attenColor / 150;
}




vec3f SpotLight::getColor(const vec3f & p) const
{
	vec3f link = (p-this->position).normalize();
	if (acos(centralDirection * link) < angle) {
		return color;
	}
	else {
		return vec3f(0.0f, 0.0f, 0.0f);
	}
}
