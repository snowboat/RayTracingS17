//
// rayUI.h
//
// The header file for the UI part
//

#ifndef __rayUI_h__
#define __rayUI_h__

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>

#include <FL/fl_file_chooser.H>		// FLTK file chooser
#include "../fileio/bitmap.h"

#include "TraceGLWindow.h"
#include "../scene/light.h"
#include "../RayTracer.h"
class RayTracer;
class TraceGLWindow;

class TraceUI {
public:
	TraceUI();

	// The FLTK widgets
	Fl_Window*			m_mainWindow;
	Fl_Menu_Bar*		m_menubar;

	Fl_Slider*			m_sizeSlider;
	Fl_Slider*			m_depthSlider;
	Fl_Slider*			m_constAttenSlider;
	Fl_Slider*			m_linearAttenSlider;
	Fl_Slider*			m_quadAttenSlider;

	Fl_Light_Button*	m_enableBackgroundButton;
	Fl_Light_Button*	m_enableAntialiasingButton;
	Fl_Light_Button*	m_enableJitteringButton;
	Fl_Light_Button*	m_enableTextureMappingButton;
	Fl_Slider*			m_numSubPixelsSlider;
	Fl_Light_Button*	m_enableDepthofFieldButton;
	Fl_Slider*			m_focalLengthSlider;
	Fl_Slider*			m_apertureSlider;
	Fl_Light_Button*	m_enableSoftShadowButton;
	Fl_Slider*			m_softshadowCoeffSlider;
	Fl_Light_Button*	m_glossyReflectionButton;



	Fl_Button*			m_renderButton;
	Fl_Button*			m_stopButton;

	TraceGLWindow*		m_traceGlWindow;

	// member functions
	void show();

	void		setRayTracer(RayTracer *tracer);

	int			getSize();
	int			getDepth();

	unsigned char* backgroundImg;	//for bonus
	unsigned char* textureImg;	//for bonus
	int textureWidth;
	int textureHeight;

	bool		getEnableBackground();
	bool		getEnableAntialiasing();
	bool		getEnableJittering();
	bool		getEnableTextureMapping();
	bool		getEnableDepthofField();
	int			getNumSubpixels();
	double		getFocalLength();
	double		getAperture();
	bool		getEnableSoftShadow();
	double		getSoftshadowCoeff();
	bool		getGlossyReflection();
private:
	RayTracer*	raytracer;

	int			m_nSize;
	int			m_nDepth;
	double		m_constAttenFactor;
	double		m_linearAttenFactor;
	double		m_quadAttenFactor;
	int			m_numSubPixels;

	bool		m_enableBackground;
	bool		m_enableAntialiasing;
	bool		m_enableJittering;
	bool		m_enableTextureMapping;
	bool		m_enableDepthofField;
	bool		m_enableSoftShadow;
	bool		m_glossyReflection;

	double		focalLength;
	double		aperture;
	double		softshadowCoeff;


// static class members
	static Fl_Menu_Item menuitems[];

	static TraceUI* whoami(Fl_Menu_* o);

	static void cb_load_scene(Fl_Menu_* o, void* v);
	static void cb_save_image(Fl_Menu_* o, void* v);
	static void cb_load_background(Fl_Menu_* o, void* v);
	static void cb_load_texture(Fl_Menu_* o, void* v);
	static void cb_exit(Fl_Menu_* o, void* v);
	static void cb_about(Fl_Menu_* o, void* v);

	static void cb_exit2(Fl_Widget* o, void* v);
		
	//Sliders
	static void cb_sizeSlides(Fl_Widget* o, void* v);
	static void cb_depthSlides(Fl_Widget* o, void* v);
	static void cb_constAttenSlides(Fl_Widget* o, void* v);
	static void cb_linearAttenSlides(Fl_Widget* o, void* v);
	static void cb_quadAttenSlides(Fl_Widget* o, void* v);
	static void cb_enableBackground(Fl_Widget* o, void* v);
	static void cb_enableJittering(Fl_Widget* o, void* v);
	static void cb_enableAntialiasing(Fl_Widget* o, void* v);
	static void cb_enableDepthofField(Fl_Widget* o, void* v);
	static void cb_enableTextureMapping(Fl_Widget* o, void* v);
	static void cb_enableSoftShadow(Fl_Widget* o, void* v);
	static void cb_numSubPixelsSlides(Fl_Widget* o, void* v);
	static void cb_focalLengthSlides(Fl_Widget* o, void* v);
	static void cb_apertureSlides(Fl_Widget* o, void* v);
	static void cb_softshadowCoeffSlides(Fl_Widget* o, void* v);
	static void cb_glossyReflection(Fl_Widget* o, void* v);

	static void cb_render(Fl_Widget* o, void* v);
	static void cb_stop(Fl_Widget* o, void* v);
};

#endif
