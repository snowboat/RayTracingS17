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

double degreeRadian(vec3f v1, vec3f v2) {
	vec3f v1n = v1.normalize();
	vec3f v2n = v2.normalize();
	
	return acos(v1n*v2n);
}

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace( Scene *scene, double x, double y )
{
	vec3f thresh(scene->getTerimnationThreshold(), scene->getTerimnationThreshold(), scene->getTerimnationThreshold());
	std::stack<const SceneObject*> objStack;	//empty stack for tracking overlapping objects
	std::stack<isect> isectStack;	//empty stack for tracking overlapping objects

	if (m_pUI->getEnableDepthofField()) {
		ray primRay(vec3f(0, 0, 0), vec3f(0, 0, 0));
		scene->getCamera()->rayThrough(x, y, primRay);
		vec3f tracedColor(0.0, 0.0, 0.0);
		for (int i = 0; i < 100; i++) {	//fire 100 random rays instead of the primary ray
			double aperture = m_pUI->getAperture();
			double focalDist = m_pUI->getFocalLength();
			vec3f camPosition = scene->getCamera()->getEye();
			vec3f primDir = primRay.getDirection();
			vec3f focalPoint = camPosition + focalDist * primDir;
			vec3f randomPoint = camPosition + ((double(rand()) / double(RAND_MAX)) * aperture) * scene->getCamera()->getv();
			vec3f secondaryDir = (focalPoint - randomPoint).normalize();
			ray secondaryRay(randomPoint, secondaryDir);
			tracedColor += traceRay(scene, secondaryRay, thresh, 0,  1.0, isectStack ).clamp();
		}

		return tracedColor / 100.0;

	}
	else if (scene->getMotionBlur()) {	//Assume that motion blur and DOF will not happen simutaneously
		//backup the geometries in this scene
		//for every geometry in scene, translate 100 times
		//do raytracing 100 times
		mat4f slightTranslation  ( vec4f(1.0, 0.0, 0.0, 0.005),
									vec4f(0.0, 1.0, 0.0, 0.005),
									vec4f(0.0, 0.0, 1.0, 0.005),
									vec4f(0.0, 0.0, 0.0, 1.0) );

		vec3f tracedColor(0.0, 0.0, 0.0);
		
		//backup the xforms
		std::vector<mat4f> backupMatrices;
		for (list<Geometry*>::iterator itr = scene->beginGeometries(); itr != scene->endGeometries(); itr++) {
			backupMatrices.push_back((*itr)->getTransformNode()->getXform());
		}

		for (int i = 0; i < 100; i++) {
			//update the position of all objects
			for (list<Geometry*>::iterator itr = scene->beginGeometries(); itr != scene->endGeometries(); itr++) {
				mat4f backup = (*itr)->getTransformNode()->getXform();
				(*itr)->getTransformNode()->setXform(slightTranslation * backup);
			}


			//trace a ray normally
			ray r(vec3f(0, 0, 0), vec3f(0, 0, 0));
			scene->getCamera()->rayThrough(x, y, r);
			tracedColor += traceRay(scene, r, thresh, 0,1.0, isectStack).clamp();
		}
		//restore the xforms after finishing up this pixel
		int counter = 0;
		for (list<Geometry*>::iterator itr = scene->beginGeometries(); itr != scene->endGeometries(); itr++) {
			(*itr)->getTransformNode()->setXform(backupMatrices[counter]);
			counter++;
		}
		return tracedColor / 100.0;


	}
	else {

		
		ray r(vec3f(0, 0, 0), vec3f(0, 0, 0));
		scene->getCamera()->rayThrough(x, y, r);
		vec3f tracedColor = traceRay(scene, r, thresh, 0,1.0 ,isectStack).clamp();
		return tracedColor;
	}

}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay( Scene *scene, const ray& r, 
	const vec3f& thresh, int depth,  double currIndex, std::stack<isect> isectStack)
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

		//Direct component 
		//vec3f directColor = prod(m.shade(scene, r, i), (vec3f(1.0f, 1.0f, 1.0f) - m.kt));
		vec3f directColor = m.shade(scene, r, i);
		vec3f reflecColor = { 0.0f,0.0f,0.0f };
		vec3f refracColor = { 0.0f, 0.0f, 0.0f };


		//reflective component
		if (directColor.length() >= thresh.length()) {
			if (scene->getGlossyReflection() && depth<depthLimit) {
				ray reflecRay(r.at(i.t), (2 * (i.N.dot(-r.getDirection()))*i.N + r.getDirection()).normalize());
				vec3f primDirection = reflecRay.getDirection();
				for (int j = 0; j < 100; j++) {
					vec3f uDistortion = primDirection.cross(i.N).normalize() * (double(rand()) * 0.1 / double(RAND_MAX));
					vec3f vDistortion = primDirection.cross(uDistortion).normalize() * (double(rand()) * 0.1 / double(RAND_MAX));
					ray secondaryRay(r.at(i.t), primDirection + uDistortion + vDistortion);
					reflecColor += prod(traceRay(scene, secondaryRay, thresh, depth + 1,  1.0 ,isectStack), m.kr);
				}
				reflecColor /= 100.0;
			}
			else {
				ray reflecRay(r.at(i.t), (2 * (i.N.dot(-r.getDirection()))*i.N + r.getDirection()).normalize());
				if (depth < depthLimit) {
					reflecColor = prod(traceRay(scene, reflecRay, thresh, depth + 1,1.0 ,isectStack), m.kr);
				}
			}




			// Refractive component
			double indexofNextMedium;
			if (!isectStack.empty() && isectStack.top().obj == i.obj) {	//leaving this object
				isectStack.pop();
				if (isectStack.empty())
					indexofNextMedium = 1.0;	//back into air, since the only element was poped
				else
					indexofNextMedium = isectStack.top().getMaterial().index;
			}
			else {	//entering another object
				isectStack.push(i);
				indexofNextMedium = m.index;
			}
			double mu = currIndex / indexofNextMedium;

			//double mu;
			//if (fromAir) {	  //Air into object
			//	mu = 1.0 / m.index;
			//}
			//else {	//medium into air
			//	mu = m.index;
			//}
			double criticalSin = 1 / mu;	//sine of critical angle
			double cosphi = i.N.dot(-r.getDirection());
			double phi = acos(cosphi);
			if (criticalSin - sin(phi) > RAY_EPSILON) {	//no TIR
				double theta = asin(sin(phi) * mu);
				double costheta = cos(theta);
				vec3f newDirection = (mu * r.getDirection() - (costheta - mu*cosphi) * i.N).normalize();
				ray refracRay(r.at(i.t), newDirection);
				if (depth < depthLimit) {
					refracColor = prod(traceRay(scene, refracRay, thresh, depth + 1, indexofNextMedium, isectStack	), m.kt);
				}
			}
		}

			return directColor + reflecColor + refracColor;
	
	} else {
		// No intersection. Return background color
		if (this->backgroundImg  && m_pUI->getEnableBackground()) {
			vec3f camerau = scene->getCamera()->getu();
			vec3f camerav = scene->getCamera()->getv();
			double projRayontoU = (r.getDirection() * camerau);
			double projRayontoV = (r.getDirection() * camerav);

			return getBackgroundColor(projRayontoU + 0.5, projRayontoV + 0.5);
		}
		else {
			return vec3f(0.0f, 0.0, 0.0f);

		}
	}
}

RayTracer::RayTracer()
{
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;

	m_bSceneLoaded = false;
	depthLimit = 0;
	backgroundImg = NULL;
	m_pUI = NULL;
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

void RayTracer::setBackgroundImg(unsigned char * img)
{
	this->backgroundImg = img;
}

void RayTracer::setDepthLimit(int depthLim)
{   
	if (depthLimit != depthLim) {
		depthLimit = depthLim;
	}
}

vec3f RayTracer::getBackgroundColor(double x, double y)
{
	if (this->backgroundImg == NULL || x<0 || x>1 || y<0 || y>1) {
		return vec3f(0.0f, 0.0f, 0.0f);

	}
	else {
		int pixelx = x*buffer_width;
		int pixely = y*buffer_height;
		unsigned char* pixel = backgroundImg + ( pixelx + pixely*buffer_width) * 3;
		int r = (int)*pixel;
		int g = (int)*(pixel+1);
		int b = (int)*(pixel+2);
		return vec3f((float)r/float(255), (float)g/255.0f, (float)b/255.0f).clamp();
		//return vec3f(0.5f, 0.5f, 0.5f).clamp();
	}
}

void RayTracer::setUI(TraceUI * ui)
{
	m_pUI = ui;
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

vec3f RayTracer::getAdaptivelySupersampledColor(Scene* scene, double x, double y, int depth) {
	//x and y are the lower-left corner
	double atomicx = double(1) / (double(buffer_width) * depth);	//grid size (determined in accordance to current depth)
	double atomicy = double(1) / (double(buffer_height) * depth);

	
	vec3f colorLowerLeft = trace(scene, x, y);
	vec3f colorLowerRight = trace(scene, x + atomicx, y);
	vec3f colorUpperLeft = trace(scene, x, y + atomicy);
	vec3f colorUpperRight = trace(scene, x + atomicx, y + atomicy);
	vec3f colorCentre = trace(scene, x + atomicx / 2, y + atomicy / 2);
	if (depth <= adaSupLimit) {
		if ((colorLowerLeft - colorCentre).length() > 0.01)
			colorLowerLeft = getAdaptivelySupersampledColor(scene, x, y, depth + 1);
		if ((colorLowerRight - colorCentre).length() > 0.01)
			colorLowerRight = getAdaptivelySupersampledColor(scene, x + atomicx / 2, y, depth + 1);
		if ((colorUpperLeft - colorCentre).length() > 0.01)
			colorUpperLeft = getAdaptivelySupersampledColor(scene, x, y + atomicy / 2, depth + 1);
		if ((colorUpperRight - colorCentre).length() > 0.01)
			colorUpperRight = getAdaptivelySupersampledColor(scene, x + atomicx / 2, y + atomicy / 2, depth + 1);
	}
	return (colorLowerLeft + colorLowerRight + colorUpperLeft + colorUpperRight) / 4;
}

void RayTracer::tracePixel( int i, int j )
{
	vec3f col;	//color of this pixel

	if( !scene )
		return;

	double x = double(i) / double(buffer_width);	//central x
	double y = double(j) / double(buffer_height);	//central y
	double atomicx = double(1) / double(buffer_width);	//corresponding length of one pixel
	double atomicy = double(1) / double(buffer_height);
	
	if (m_pUI->getEnableAntialiasing()) {	//only return color of central x & central y
		if (m_pUI->getAdaptiveSupersampling()) {
			col = getAdaptivelySupersampledColor(scene, x, y, 1);	//it's much faster. the effect is similar to non-adaptive supersampling with 4/5 subpixels, which is super expensive
		}
		else {		//non-adaptive supersampling
			int numSubpixels = m_pUI->getNumSubpixels();
			double startx = x - 0.5 / double(buffer_width);
			double starty = y - 0.5 / double(buffer_height);

			double xstep = (1.0 / double(buffer_width)) / double(numSubpixels - 1);
			double ystep = (1.0 / double(buffer_height)) / double(numSubpixels - 1);

			for (int i = 0; i < numSubpixels; i++) {
				for (int j = 0; j < numSubpixels; j++) {
					if (m_pUI->getEnableJittering()) {	//random direction witin +- one atomic range
						double offsetCoeff = ((double)rand() / (RAND_MAX)) * 2 - 1;
						col = trace(scene, startx + xstep*i + offsetCoeff*atomicx, starty + ystep*j + offsetCoeff*atomicy);//the point to trace is a random point between x,y plus/minus one atomic length
					}
					else {//determined direction
						col += trace(scene, startx + xstep*i, starty + ystep*j) / (numSubpixels*numSubpixels);
					}
				}
			}
		}

		
	}
	else {
		if (m_pUI->getEnableJittering()) {
			double offsetCoeff = ((double)rand() / (RAND_MAX)) * 2 - 1;
			col = trace(scene, x + offsetCoeff*atomicx, y + offsetCoeff*atomicy);//the point to trace is a random point between x,y plus/minus one atomic length
		}
		else {
			col = trace(scene, x, y);
		}
	}


	unsigned char *pixel = this->buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}