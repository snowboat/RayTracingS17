#include "ray.h"
#include "material.h"
#include "light.h"

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.

vec3f Material::getTextureColor(Scene* scene, double x, double y) const
{
	if (scene->getTexture() && x>=0 && x<=1 && y>=0 && y<=1){

		//currently I only have one texture, whose size is 1024*1024
		int pixelx = min(1023,int(x*1024));	//hard-code for now
		int pixely = min(1023,int(y*1024));	//hard-code for now
		unsigned char* pixel = scene->getTexture() + (pixelx + pixely*1024) * 3;
		int r = (int)*pixel;
		int g = (int)*(pixel + 1);
		int b = (int)*(pixel + 2);
		return vec3f((float)r/ float(255), (float)g / 255.0f, (float)b / 255.0f).clamp();
		//return vec3f(0.5f, 0.5f, 0.5f).clamp();
	}
	else {
		cout << "caused by " << x << " " << y << endl;
		return vec3f(0.0f, 0.0f, 0.0f);
	}
}

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
	return shade(scene, r, i, false);

}


//overloaded function to handle the shading in texture mapping
vec3f Material::shade(Scene * scene, const ray & r, const isect & i, bool textureMap) const
{
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
		vec3f lightColor = (**j).getColor(P);
		
		
		vec3f Diffuse;
		if (textureMap) {
			double u, v;
			if (i.obj->getLocalUV(r, i, u, v)) {
				//cout << "u v " << u << "  " << v << endl;
				Diffuse = getTextureColor(scene, u, v);	//TODO: get the color of texture based on u,v
				//cout << Diffuse << endl;
				return Diffuse;
			}
			else {

				Diffuse = kd * max(0.0, L*i.N);	//if the shape of i.obj doesn't support texture mapping, then diffuse color is still the original one.
			}
		}
		else {
			Diffuse = kd * max(0.0, L*i.N);

		}

		vec3f Specular = ks*pow(max(0.0, V*R), shininess * 128);
		vec3f Attenuation = (*j)->distanceAttenuation(P)*   prod((*j)->shadowAttenuation(P), (*j)->getColor(P));
		I = I + elementMulti(Attenuation, Diffuse + Specular);
	}

	return I;
}



