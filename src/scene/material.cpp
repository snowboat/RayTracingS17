#include "ray.h"
#include "material.h"
#include "light.h"

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.

vec3f elementMulti(vec3f k, vec3f i) {
	double i0 = k[0] * i[0];
	double i1 = k[1] * i[1];
	double i2 = k[2] * i[2];

	return vec3f(i0, i1, i2);
}

vec3f Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
    // You will need to call both distanceAttenuation() and shadowAttenuation()
    // somewhere in your code in order to compute shadows and light falloff.

	// iteration 0
	vec3f I = ke;

	// iteration 1
	I = I + elementMulti(ka, scene->ambientLight);
	// iteration 2+3
	typedef list<Light*>::const_iterator iter;
	iter j;
	
	for (j = scene->beginLights(); j != scene->endLights(); ++j) {
		vec3f P = r.at(i.t);	//position
		vec3f L = (*j)->getDirection(P);	//direction of light
		vec3f V = r.getDirection();		//direction of eyeray
		vec3f R = 2 * (-L*i.N)*i.N + L; //reflection direction of the light
		vec3f Diffuse = kd*max(0.0, L*i.N);
		vec3f Specular = ks*pow(max(0.0,V*R), shininess*128);
		vec3f Attenuation = (*j)->distanceAttenuation(P)*elementMulti((*j)->shadowAttenuation(P), (*j)->getColor(P));
		I = I + elementMulti(Attenuation,Diffuse+Specular);
	}

	return I;
}

