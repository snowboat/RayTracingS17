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
	if (!m_pUI->getEnableDepthofField()) {

		ray r(vec3f(0, 0, 0), vec3f(0, 0, 0));
		scene->getCamera()->rayThrough(x, y, r);
		vec3f tracedColor = traceRay(scene, r, vec3f(1.0, 1.0, 1.0), 0, true, x, y).clamp();
		return tracedColor;
	}
	else {
		ray primRay(vec3f(0, 0, 0), vec3f(0, 0, 0));
		scene->getCamera()->rayThrough(x, y, primRay);
		vec3f tracedColor = traceRay(scene, primRay, vec3f(1.0, 1.0, 1.0), 0, true, x, y).clamp();

		for (int i = 0; i < 3; i++) {	//fire three extra random rays
			double aperture = m_pUI->getAperture();
			double focalDist = m_pUI->getFocalLength();
			vec3f camPosition = scene->getCamera()->getEye();
			vec3f primDir = primRay.getDirection();
			vec3f focalPoint = camPosition + focalDist * primDir;
			
			cout << "randomnumber is " << (double(rand()) / double(RAND_MAX)) << endl;
			cout << "aperture is " << aperture << endl;

			cout << "offset from pinhole" << (double(rand()) / double(RAND_MAX)) * aperture << endl;
			vec3f randomPoint = camPosition + (  (double(rand()) / double(RAND_MAX)) * aperture) * scene->getCamera()->getv();
			vec3f secondaryDir = (focalPoint - randomPoint).normalize();
			ray secondaryRay(randomPoint, secondaryDir);
			tracedColor += traceRay(scene, secondaryRay, vec3f(1.0, 1.0, 1.0), 0, true, x, y).clamp();
		}

		return tracedColor / 4;
		

	}

}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay( Scene *scene, const ray& r, 
	const vec3f& thresh, int depth, bool fromAir, double xcoord, double ycoord )
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
		//cout << "dotproduct " << i.N.dot(-r.getDirection()) << endl;

		// Reflection component
		ray reflecRay(r.at(i.t),(2 * (i.N.dot(-r.getDirection()))*i.N + r.getDirection()).normalize());
		vec3f reflecColor = { 0.0f,0.0f,0.0f };
		if (depth < depthLimit) {
			reflecColor = prod(traceRay(scene, reflecRay, thresh, depth + 1,fromAir, xcoord, ycoord), m.kr);
		}

		// Refractive component
		
		double mu = 1.0;
		if (fromAir) {	  //Air into object
			mu = 1.0 / m.index;
		}
		else {
			mu = m.index;
		}
		double criticalSin =	1/mu;	//sine of critical angle

		
		double cosphi = i.N.dot(-r.getDirection());
		double phi = acos(cosphi);
		vec3f refracColor = { 0.0f, 0.0f, 0.0f };
		if ( criticalSin - sin(phi) > RAY_EPSILON) {	//no TIR
			
			double theta = asin(sin(phi) * mu);
			double costheta = cos(theta);			
			vec3f newDirection = (mu * r.getDirection() - (costheta - mu*cosphi) * i.N).normalize();
			// vec3f newDirection = (mu * r.getDirection() + (mu*cosphi - costheta) * i.N).normalize();  // paper from greve
			//THE TYPO ON ALAN WATT..... EXCITING!!!!!s
			//cout << depth << " " << mu << " " << acos(cosphi) * 180 / PI << " " << acos(costheta) * 180 / PI << endl;
			ray refracRay(r.at(i.t), newDirection);

			if (depth < depthLimit) {
				refracColor = prod(traceRay(scene, refracRay, thresh, depth + 1, !fromAir, xcoord,ycoord), m.kt);
			}
		}
			return prod(m.shade(scene, r, i, m_pUI->getEnableTextureMapping()), (vec3f(1.0f, 1.0f, 1.0f) - m.kt)) + reflecColor + refracColor;
	
	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.
		if (this->backgroundImg == NULL || !m_pUI->getEnableBackground()) {
			return vec3f(0.0f, 0.0f, 0.0f);
		}
		else {

			vec3f camerau = scene->getCamera()->getu();
			vec3f camerav = scene->getCamera()->getv();
			double projRayontoU = (r.getDirection() * camerau);
			double projRayontoV = (r.getDirection() * camerav);

			return getBackgroundColor(projRayontoU+ 0.5, projRayontoV + 0.5);
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

void RayTracer::tracePixel( int i, int j )
{
	vec3f col;	//color of this pixel

	if( !scene )
		return;

	double x = double(i) / double(buffer_width);	//central x
	double y = double(j) / double(buffer_height);	//central y
	double atomicx = double(1) / double(buffer_width);	//corresponding length of one pixel
	double atomicy = double(1) / double(buffer_height);
	
	if (!m_pUI->getEnableAntialiasing()) {	//only return color of central x & central y
		if (m_pUI->getEnableJittering()) {
			double offsetCoeff = ((double)rand() / (RAND_MAX)) * 2 - 1;
			col = trace(scene, x+offsetCoeff*atomicx, y+offsetCoeff*atomicy);//the point to trace is a random point between x,y plus/minus one atomic length
		}
		else {
			col = trace(scene, x, y);
		}
	}
	else {
		int numSubpixels = m_pUI->getNumSubpixels();
		double startx = x - 0.5 / double(buffer_width);
		double starty = y - 0.5 / double(buffer_height);

		double xstep = (1.0 / double(buffer_width)) / double(numSubpixels - 1);
		double ystep =( 1.0 / double(buffer_height)) / double(numSubpixels - 1);

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


	unsigned char *pixel = this->buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}