#include <cmath>

#include "light.h"

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

double PointLight::distanceAttenuation(const vec3f& P) const
{
	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, I assume no attenuation and just return 1.0
	if (constant_attenuation_coeff > 0.0 || linear_attenuation_coeff > 0.0 || quadratic_attenuation_coeff > 0.0) {
		double distance = (position - P).length();
		return min(1.0, 1.0 / (constant_attenuation_coeff + linear_attenuation_coeff*distance + quadratic_attenuation_coeff*distance*distance));
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
