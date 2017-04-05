#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

// The main ray tracer.

#include "scene/scene.h"
#include "scene/ray.h"
#include "ui\TraceUI.h"
class TraceUI;

class RayTracer
{
public:
    RayTracer();
    ~RayTracer();

    vec3f trace( Scene *scene, double x, double y );
	vec3f traceRay( Scene *scene, const ray& r, const vec3f& thresh, int depth, bool fromAir, double xcoord, double ycoord );


	void getBuffer( unsigned char *&buf, int &w, int &h );
	double aspectRatio();
	void traceSetup( int w, int h );
	void traceLines( int start = 0, int stop = 10000000 );
	void tracePixel( int i, int j );

	bool loadScene( char* fn );

	bool sceneLoaded();
	Scene* getScene();
	void setBackgroundImg(unsigned char* img);
	void setDepthLimit(int depthLim);

	vec3f getBackgroundColor(double x, double y);
	void setUI(TraceUI* ui);
private:
	unsigned char *buffer;
	int buffer_width, buffer_height;
	int bufferSize;
	Scene *scene;
	int depthLimit;
	unsigned char* backgroundImg;

	bool m_bSceneLoaded;

	TraceUI* m_pUI;
};

#endif // __RAYTRACER_H__
