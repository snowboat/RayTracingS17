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
	
	
	
	// iteration 0:emissive
	vec3f I = ke;
	
	// iteration 1: ambient illumination
	//I = I + elementMulti(ka, scene->ambientLight);
	I += prod(prod(ka, scene->ambientLight), vec3f(1.0, 1.0, 1.0) - kt);
	
	// iteration 2+3 :specular and diffuse, multiplied by shadow+distance attenuation
	typedef list<Light*>::const_iterator iter;
	iter j;

	for (j = scene->beginLights(); j != scene->endLights(); ++j) {
		vec3f P = r.at(i.t);	//position
		vec3f L = (*j)->getDirection(P);	//direction of light
		vec3f V = r.getDirection();		//direction of eyeray
		vec3f R = 2 * (-L*i.N)*i.N + L; //reflection direction of the light
		vec3f lightColor = (**j).getColor(P);


		vec3f Diffuse;
		vec3f diffuseCoeff = this->kd;
		
		double u, v;
		if (scene->getTextureMapping() && i.obj->getLocalUV(r, i, u, v)) {
			vec3f newNormal = i.N;		//newNormal is used to do the diffuse shading. it will be pertubated in bumpmappingcase and keeps unchanged in non-bumpmapping case
			diffuseCoeff = scene->getTextureColor(u, v);
			isect icopy = i;

			if (scene->bumpMapping && i.obj->preturbNormal(r, icopy, u, v, scene->getTexture(), scene->getTextureWidth(), scene->getTextureHeight(), scene)) {
				newNormal = icopy.N;
				//Diffuse = this->kd *max(0.0, L*newNormal);
				cout << "diffuse color " << Diffuse << endl;
			}
			
			Diffuse = diffuseCoeff * max(0.0, L*newNormal);	//if the shape of i.obj doesn't support texture mapping, then diffuse color is still the original one.
			Diffuse = prod(Diffuse, vec3f(1.0, 1.0, 1.0) - kt);

		}
		else {
			Diffuse = diffuseCoeff * max(0.0, L*i.N);	//if the shape of i.obj doesn't support texture mapping, then diffuse color is still the original one.
			Diffuse = prod(Diffuse, vec3f(1.0, 1.0, 1.0) - kt);
		}


		vec3f Specular = ks*pow(max(0.0, V*R), shininess * 128);


		vec3f Attenuation;

		//TODO: TO ACCELERATE, IF DISTANCE ATTENUATION IS ALREADY < THRESH, THEN DON'T CHECK SHADOW ATTENUATION, DIRECTLY RETURN 000 INSTEAD.

		//if soft shadow is enabled, use "soft shadow attenuation" instead.
		if (scene->getSoftShadow()) {
			if ((*j)->getColor(P).iszero())		//no need to check attenuation. thus it's faster
				Attenuation = { 0.0,0.0,0.0 };
			else
				Attenuation = (*j)->distanceAttenuation(P)*   prod((*j)->shadowAttenuationSoft(P, scene->getSoftShadowCoeff()), (*j)->getColor(P));
		}
		else {
			if ((*j)->getColor(P).iszero())		//no need to check attenuation. thus it's faster
				Attenuation = { 0.0,0.0,0.0 };
			else
				Attenuation = (*j)->distanceAttenuation(P)*   prod((*j)->shadowAttenuation(P), (*j)->getColor(P));
		}
		I += elementMulti(Attenuation, Diffuse + Specular);
	}

	return I;
}
