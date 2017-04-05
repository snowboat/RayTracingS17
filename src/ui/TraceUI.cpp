//
// TraceUI.h
//
// Handles FLTK integration and other user interface tasks
//
#include <stdio.h>
#include <time.h>
#include <string.h>

#include <FL/fl_ask.h>

#include "TraceUI.h"
#include "../RayTracer.h"


static bool done;

//------------------------------------- Help Functions --------------------------------------------
TraceUI* TraceUI::whoami(Fl_Menu_* o)	// from menu item back to UI itself
{
	return ( (TraceUI*)(o->parent()->user_data()) );
}

//--------------------------------- Callback Functions --------------------------------------------
void TraceUI::cb_load_scene(Fl_Menu_* o, void* v) 
{
	TraceUI* pUI=whoami(o);
	
	char* newfile = fl_file_chooser("Open Scene?", "*.ray", NULL );

	if (newfile != NULL) {
		char buf[256];

		if (pUI->raytracer->loadScene(newfile)) {
			sprintf(buf, "Ray <%s>", newfile);
			done=true;	// terminate the previous rendering

			//transfer the atten factors from UI to the new scene
			pUI->raytracer->getScene()->constAttenFactor = pUI->m_constAttenFactor;
			pUI->raytracer->getScene()->linearAttenFactor = pUI->m_linearAttenFactor;
			pUI->raytracer->getScene()->quadAttenFactor = pUI->m_quadAttenFactor;
		} else{
			sprintf(buf, "Ray <Not Loaded>");
		}

		pUI->m_mainWindow->label(buf);
	}
}

void TraceUI::cb_save_image(Fl_Menu_* o, void* v) 
{
	TraceUI* pUI=whoami(o);
	
	char* savefile = fl_file_chooser("Save Image?", "*.bmp", "save.bmp" );
	if (savefile != NULL) {
		pUI->m_traceGlWindow->saveImage(savefile);
	}
}

void TraceUI::cb_load_background(Fl_Menu_ * o, void * v)
{
	TraceUI* pUI = whoami(o);

	char* newfile = fl_file_chooser("Load Background", "*.bmp", NULL);

	if (newfile) {
		unsigned char* data;
		int width, height;
		if ((data = readBMP(newfile, width, height)) == NULL)
		{
			fl_alert("Can't load bitmap file");
			return;
		}
		if (width != pUI->getSize() || height != pUI->getSize()) {
			fl_alert("size doesn't match");
			return;
		}

		pUI->backgroundImg = data;
		pUI->m_enableBackgroundButton->activate();
	}


}

void TraceUI::cb_exit(Fl_Menu_* o, void* v)
{
	TraceUI* pUI=whoami(o);

	// terminate the rendering
	done=true;

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
}

void TraceUI::cb_exit2(Fl_Widget* o, void* v) 
{
	TraceUI* pUI=(TraceUI *)(o->user_data());
	
	// terminate the rendering
	done=true;

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
}

void TraceUI::cb_about(Fl_Menu_* o, void* v) 
{
	fl_message("RayTracer Project, FLTK version for CS 341 Spring 2002. Latest modifications by Jeff Maurer, jmaurer@cs.washington.edu");
}

void TraceUI::cb_sizeSlides(Fl_Widget* o, void* v)
{
	TraceUI* pUI=(TraceUI*)(o->user_data());
	
	pUI->m_nSize=int( ((Fl_Slider *)o)->value() ) ;
	int	height = (int)(pUI->m_nSize / pUI->raytracer->aspectRatio() + 0.5);
	pUI->m_traceGlWindow->resizeWindow( pUI->m_nSize, height );
}

void TraceUI::cb_depthSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nDepth=int( ((Fl_Slider *)o)->value() ) ;
	TraceUI* pUI = (TraceUI*)(o->user_data());
	pUI->m_nDepth = int(((Fl_Slider *)o)->value());
}

void TraceUI::cb_constAttenSlides(Fl_Widget * o, void * v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());
	pUI->m_constAttenFactor = double(((Fl_Slider *)o)->value());

	//Scale up/down the constant_attenuation_coeff for ALL LIGHTS in current scene
	//I should make constAttenFactor a member of Scene
	if (pUI->raytracer->getScene())
		pUI->raytracer->getScene()->constAttenFactor = pUI->m_constAttenFactor;

	
}

void TraceUI::cb_linearAttenSlides(Fl_Widget * o, void * v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());
	pUI->m_linearAttenFactor  = double(((Fl_Slider *)o)->value());
	if (pUI->raytracer->getScene())
		pUI->raytracer->getScene()->linearAttenFactor = pUI->m_linearAttenFactor;

}

void TraceUI::cb_quadAttenSlides(Fl_Widget * o, void * v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());
	pUI->m_quadAttenFactor = double(((Fl_Slider *)o)->value());

	if (pUI->raytracer->getScene())
		pUI->raytracer->getScene()->quadAttenFactor = pUI->m_quadAttenFactor;

}

void TraceUI::cb_enableBackground(Fl_Widget* o, void* v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());

	pUI-> m_enableBackground= bool(((Fl_Light_Button *)o)->value());
}

void TraceUI::cb_enableJittering(Fl_Widget * o, void * v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());

	pUI->m_enableJittering = bool(((Fl_Light_Button *)o)->value());
}

void TraceUI::cb_enableAntialiasing(Fl_Widget * o, void * v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());

	pUI->m_enableAntialiasing = bool(((Fl_Light_Button *)o)->value());
}

void TraceUI::cb_numSubPixelsSlides(Fl_Widget * o, void * v)
{
	TraceUI* pUI = (TraceUI*)(o->user_data());
	pUI->m_numSubPixels = int(((Fl_Slider *)o)->value());
}


void TraceUI::cb_render(Fl_Widget* o, void* v)
{
	char buffer[256];

	TraceUI* pUI=((TraceUI*)(o->user_data()));
	
	if (pUI->raytracer->sceneLoaded()) {
		//set background img for RayTracer
		pUI->raytracer->setBackgroundImg(pUI->backgroundImg);

		//set the linked ui of raytracer to be me
		pUI->raytracer->setUI(pUI);


		int width=pUI->getSize();
		int	height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
		int depth = pUI->getDepth();
		pUI->m_traceGlWindow->resizeWindow( width, height );

		pUI->m_traceGlWindow->show();

		pUI->raytracer->traceSetup(width, height);
		pUI->raytracer->setDepthLimit(depth);
		
		// Save the window label
		const char *old_label = pUI->m_traceGlWindow->label();

		// start to render here	
		done=false;
		clock_t prev, now;
		prev=clock();
		
		pUI->m_traceGlWindow->refresh();
		Fl::check();
		Fl::flush();

		for (int y=0; y<height; y++) {
			for (int x=0; x<width; x++) {
				if (done) break;
				
				// current time
				now = clock();

				// check event every 1/2 second
				if (((double)(now-prev)/CLOCKS_PER_SEC)>0.5) {
					prev=now;

					if (Fl::ready()) {
						// refresh
						pUI->m_traceGlWindow->refresh();
						// check event
						Fl::check();

						if (Fl::damage()) {
							Fl::flush();
						}
					}
				}

				pUI->raytracer->tracePixel( x, y );
		
			}
			if (done) break;

			// flush when finish a row
			if (Fl::ready()) {
				// refresh
				pUI->m_traceGlWindow->refresh();

				if (Fl::damage()) {
					Fl::flush();
				}
			}
			// update the window label
			sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)height * 100.0), old_label);
			pUI->m_traceGlWindow->label(buffer);
			
		}
		done=true;
		pUI->m_traceGlWindow->refresh();

		// Restore the window label
		pUI->m_traceGlWindow->label(old_label);		
	}
}

void TraceUI::cb_stop(Fl_Widget* o, void* v)
{
	done=true;
}

void TraceUI::show()
{
	m_mainWindow->show();
}

void TraceUI::setRayTracer(RayTracer *tracer)
{
	raytracer = tracer;
	m_traceGlWindow->setRayTracer(tracer);
}

int TraceUI::getSize()
{
	return m_nSize;
}

int TraceUI::getDepth()
{
	return m_nDepth;
}

bool TraceUI::getEnableBackground()
{
	return m_enableBackground;
}

bool TraceUI::getEnableAntialiasing()
{
	return m_enableAntialiasing;
}

bool TraceUI::getEnableJittering()
{
	return this->m_enableJittering;
}

int TraceUI::getNumSubpixels()
{
	return this->m_numSubPixels;
}

// menu definition
Fl_Menu_Item TraceUI::menuitems[] = {
	{ "&File",		0, 0, 0, FL_SUBMENU },
		{ "&Load Scene...",	FL_ALT + 'l', (Fl_Callback *)TraceUI::cb_load_scene },
		{ "&Save Image...",	FL_ALT + 's', (Fl_Callback *)TraceUI::cb_save_image },
		{ "&Load Background...",	FL_ALT + 's', (Fl_Callback *)TraceUI::cb_load_background },
		{ "&Exit",			FL_ALT + 'e', (Fl_Callback *)TraceUI::cb_exit },
		{ 0 },

	{ "&Help",		0, 0, 0, FL_SUBMENU },
		{ "&About",	FL_ALT + 'a', (Fl_Callback *)TraceUI::cb_about },
		{ 0 },

	{ 0 }
};

TraceUI::TraceUI() {
	//Value Initializations
	m_nDepth = 0;
	m_nSize = 150;
	m_constAttenFactor = 1.0;
	m_linearAttenFactor = 1.0;
	m_quadAttenFactor = 1.0;
	backgroundImg = NULL;
	m_enableBackground = false;
	m_enableAntialiasing = false;
	m_numSubPixels = 2;
	m_enableJittering = false;

	m_mainWindow = new Fl_Window(100, 40, 400, 400, "Ray <Not Loaded>");
		m_mainWindow->user_data((void*)(this));	// record self to be used by static callback functions
		// install menu bar
		m_menubar = new Fl_Menu_Bar(0, 0, 320, 25);
		m_menubar->menu(menuitems);

		// install slider depth
		m_depthSlider = new Fl_Value_Slider(10, 30, 180, 20, "Depth");
		m_depthSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_depthSlider->type(FL_HOR_NICE_SLIDER);
        m_depthSlider->labelfont(FL_COURIER);
        m_depthSlider->labelsize(12);
		m_depthSlider->minimum(0);
		m_depthSlider->maximum(10);
		m_depthSlider->step(1);
		m_depthSlider->value(m_nDepth);
		m_depthSlider->align(FL_ALIGN_RIGHT);
		m_depthSlider->callback(cb_depthSlides);

		// install slider size
		m_sizeSlider = new Fl_Value_Slider(10, 55, 180, 20, "Size");
		m_sizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_sizeSlider->type(FL_HOR_NICE_SLIDER);
        m_sizeSlider->labelfont(FL_COURIER);
        m_sizeSlider->labelsize(12);
		m_sizeSlider->minimum(64);
		m_sizeSlider->maximum(512);
		m_sizeSlider->step(1);
		m_sizeSlider->value(m_nSize);
		m_sizeSlider->align(FL_ALIGN_RIGHT);
		m_sizeSlider->callback(cb_sizeSlides);


		// install slider size
		m_constAttenSlider = new Fl_Value_Slider(10, 80, 180, 20, "Attenuation, Constant");
		m_constAttenSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_constAttenSlider->type(FL_HOR_NICE_SLIDER);
		m_constAttenSlider->labelfont(FL_COURIER);
		m_constAttenSlider->labelsize(12);
		m_constAttenSlider->minimum(0.00);
		m_constAttenSlider->maximum(5.00);
		m_constAttenSlider->step(0.01);
		m_constAttenSlider->value(m_constAttenFactor);
		m_constAttenSlider->align(FL_ALIGN_RIGHT);
		m_constAttenSlider->callback(cb_constAttenSlides);


		// install linear attenuation slider
		m_constAttenSlider = new Fl_Value_Slider(10, 105, 180, 20, "Attenuation, Linear");
		m_constAttenSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_constAttenSlider->type(FL_HOR_NICE_SLIDER);
		m_constAttenSlider->labelfont(FL_COURIER);
		m_constAttenSlider->labelsize(12);
		m_constAttenSlider->minimum(0.00);
		m_constAttenSlider->maximum(5.00);
		m_constAttenSlider->step(0.01);
		m_constAttenSlider->value(m_linearAttenFactor);
		m_constAttenSlider->align(FL_ALIGN_RIGHT);
		m_constAttenSlider->callback(cb_linearAttenSlides);

		// install slider size
		m_constAttenSlider = new Fl_Value_Slider(10, 130, 180, 20, "Attenuation, Quadratic");
		m_constAttenSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_constAttenSlider->type(FL_HOR_NICE_SLIDER);
		m_constAttenSlider->labelfont(FL_COURIER);
		m_constAttenSlider->labelsize(12);
		m_constAttenSlider->minimum(0.00);
		m_constAttenSlider->maximum(5.00);
		m_constAttenSlider->step(0.01);
		m_constAttenSlider->value(m_quadAttenFactor);
		m_constAttenSlider->align(FL_ALIGN_RIGHT);
		m_constAttenSlider->callback(cb_quadAttenSlides);

		//use background image or not
		m_enableBackgroundButton = new Fl_Light_Button(10, 155, 100, 25, "&Background?");
		m_enableBackgroundButton->user_data((void*)(this));   // record self to be used by static callback functions
		m_enableBackgroundButton->value(m_enableBackground);
		m_enableBackgroundButton->callback(cb_enableBackground);
		m_enableBackgroundButton->deactivate();


		//use background image or not
		m_enableAntialiasingButton = new Fl_Light_Button(115, 155, 100, 25, "&Antialiasing?");
		m_enableAntialiasingButton->user_data((void*)(this));   // record self to be used by static callback functions
		m_enableAntialiasingButton->value(m_enableAntialiasing);
		m_enableAntialiasingButton->callback(cb_enableAntialiasing);
		m_enableAntialiasingButton->activate();

		//use background image or not
		m_enableJitteringButton = new Fl_Light_Button(220, 155, 100, 25, "&Jittering?");
		m_enableJitteringButton->user_data((void*)(this));   // record self to be used by static callback functions
		m_enableJitteringButton->value(m_enableJittering);
		m_enableJitteringButton->callback(cb_enableJittering);
		m_enableJitteringButton->activate();

		//Control Antialiasing Sub-pixels
		m_constAttenSlider = new Fl_Value_Slider(10, 180, 180, 20, "# of subpixels");
		m_constAttenSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_constAttenSlider->type(FL_HOR_NICE_SLIDER);
		m_constAttenSlider->labelfont(FL_COURIER);
		m_constAttenSlider->labelsize(12);
		m_constAttenSlider->minimum(2);
		m_constAttenSlider->maximum(5);
		m_constAttenSlider->step(1);
		m_constAttenSlider->value(m_numSubPixels);
		m_constAttenSlider->align(FL_ALIGN_RIGHT);
		m_constAttenSlider->callback(cb_numSubPixelsSlides);

		m_renderButton = new Fl_Button(240, 27, 70, 25, "&Render");
		m_renderButton->user_data((void*)(this));
		m_renderButton->callback(cb_render);

		m_stopButton = new Fl_Button(240, 55, 70, 25, "&Stop");
		m_stopButton->user_data((void*)(this));
		m_stopButton->callback(cb_stop);

		m_mainWindow->callback(cb_exit2);
		m_mainWindow->when(FL_HIDE);
    m_mainWindow->end();

	// image view
	m_traceGlWindow = new TraceGLWindow(100, 150, m_nSize, m_nSize, "Rendered Image");
	m_traceGlWindow->end();
	m_traceGlWindow->resizable(m_traceGlWindow);
}