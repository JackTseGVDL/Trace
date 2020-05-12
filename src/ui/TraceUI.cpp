#include <stdio.h>
#include <time.h>
#include <string.h>
#include <FL/fl_ask.h>
#include "TraceUI.h"
#include "../RayTracer.h"
#include "../RayTracing_Base.h"


// Data Structure
union Graphic_UnionValue {
	int		val_i;
	float	val_f;
	double	val_d;
};


enum Graphic_UnionType {
	GRAPHIC_UNIONTYPE_INTEGER = 0,
	GRAPHIC_UNIONTYPE_FLOAT,
	GRAPHIC_UNIONTYPE_DOUBLE
};


class Graphic_Rect {
public:
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	Graphic_Rect(int x, int y, int w, int h) :
		x(x), y(y), w(w), h(h) {}
};


class Graphic_FontOption {
public:
	const char* text	= nullptr;
	int			size	= 10;
	Fl_Font		font	= FL_COURIER;

	Graphic_FontOption(const char *text, int size, Fl_Font font = FL_COURIER):
		text(text), size(size), font(font) {}
};


class Graphic_Range {
public:
	Graphic_UnionValue	minimum;
	Graphic_UnionValue	maximum;
	Graphic_UnionValue	step;
	Graphic_UnionValue	value;
	Graphic_UnionType	val_type;

	Graphic_Range(int minimum, int maximum, int step, int value) {
		this->minimum.val_i = minimum;
		this->maximum.val_i = maximum;
		this->step.val_i	= step;
		this->value.val_i	= value;
		this->val_type		= GRAPHIC_UNIONTYPE_INTEGER;
	}

	Graphic_Range(float minimum, float maximum, float step, float value) {
		this->minimum.val_f = minimum;
		this->maximum.val_f = maximum;
		this->step.val_f	= step;
		this->value.val_f	= value;
		this->val_type		= GRAPHIC_UNIONTYPE_FLOAT;
	}

	Graphic_Range(double minimum, double maximum, double step, double value) {
		this->minimum.val_d = minimum;
		this->maximum.val_d = maximum;
		this->step.val_d	= step;
		this->value.val_d	= value;
		this->val_type		= GRAPHIC_UNIONTYPE_DOUBLE;
	}
};


// Static Data
static bool done;

Fl_Menu_Item TraceUI::menuitems[] = {
	{ "&File",		0, 0, 0, FL_SUBMENU },
		{ "&Load Scene...",			FL_ALT + 'l',	(Fl_Callback*)TraceUI::cb_load_scene },
		{ "&Save Image...",			FL_ALT + 's',	(Fl_Callback*)TraceUI::cb_save_image },
		{ "&Load Background Image", 0,				(Fl_Callback*)TraceUI::cb_menu_load_background},
		{ "&Exit",					FL_ALT + 'e',	(Fl_Callback*)TraceUI::cb_exit },
		{ 0 },

	{ "&Help",		0, 0, 0, FL_SUBMENU },
		{ "&About",	FL_ALT + 'a', (Fl_Callback*)TraceUI::cb_about },
		{ 0 },

	{ 0 }
};

Fl_Menu_Item TraceUI::menuItem_super_sampling[] = {
	{"None",		0,	(Fl_Callback*)cb_choice_super_sampling, (void*)RAYTRACING_SUPERSAMPLING_NONE},
	{"Grid",		0,	(Fl_Callback*)cb_choice_super_sampling, (void*)RAYTRACING_SUPERSAMPLING_GRID},
	{"Random",		0,	(Fl_Callback*)cb_choice_super_sampling,	(void*)RAYTRACING_SUPERSAMPLING_RANDOM},
	{"Jittered",	0,	(Fl_Callback*)cb_choice_super_sampling, (void*)RAYTRACING_SUPERSAMPLING_JITTER},
	{"Adaptive",	0,	(Fl_Callback*)cb_choice_super_sampling, (void*)RAYTRACING_SUPERSAMPLING_ADAPTIVE},
	{0}
};


// Static Function Prototype
static Fl_Value_Slider* FL_createValueSlider	(Graphic_Rect *rect, Graphic_FontOption *font, Graphic_Range *range, Fl_Align align, void *data, Fl_Callback *cb);
static Fl_Button*		FL_createButton			(Graphic_Rect* rect, Graphic_FontOption* font, Fl_Align align, void *data, Fl_Callback* cb);
static Fl_Light_Button* FL_createLightButton	(Graphic_Rect* rect, Graphic_FontOption* font, Fl_Align align, int value, void* data, Fl_Callback* cb);
static Fl_Choice*		FL_createChoice			(Graphic_Rect* rect, Graphic_FontOption* font, Fl_Menu_Item *item, Fl_Align align, void* data, Fl_Callback* cb);


// Operation Handling
// static
// from menu item back to UI itself
TraceUI* TraceUI::whoami(Fl_Menu_* o) {
	return ( (TraceUI*)(o->parent()->user_data()) );
}


// callback
void TraceUI::cb_load_scene(Fl_Menu_* o, void* v) {
	TraceUI* pUI=whoami(o);
	
	char* newfile = fl_file_chooser("Open Scene?", "*.ray", NULL );

	if (newfile != NULL) {
		char buf[256];

		if (pUI->raytracer->loadScene(newfile)) {
			sprintf(buf, "Ray <%s>", newfile);
			done=true;	// terminate the previous rendering
		} else{
			sprintf(buf, "Ray <Not Loaded>");
		}

		pUI->m_mainWindow->label(buf);
	}
}


void TraceUI::cb_save_image(Fl_Menu_* o, void* v) {
	TraceUI* pUI=whoami(o);
	
	char* savefile = fl_file_chooser("Save Image?", "*.bmp", "save.bmp" );
	if (savefile != NULL) {
		pUI->m_traceGlWindow->saveImage(savefile);
	}
}



void TraceUI::cb_menu_load_background(Fl_Menu_* o, void* v) {

}


void TraceUI::cb_exit(Fl_Menu_* o, void* v) {
	TraceUI* pUI=whoami(o);

	// terminate the rendering
	done=true;

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
}


void TraceUI::cb_exit2(Fl_Widget* o, void* v) {
	TraceUI* pUI=(TraceUI *)(o->user_data());
	
	// terminate the rendering
	done=true;

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
}


void TraceUI::cb_about(Fl_Menu_* o, void* v) {
	fl_message("RayTracer Project, FLTK version for CS 341 Spring 2002. Latest modifications by Jeff Maurer, jmaurer@cs.washington.edu");
}


// callback - slider
void TraceUI::cb_slider_size(Fl_Widget* o, void* v) {
	TraceUI* pUI = (TraceUI*)(o->user_data());
	int height;
	
	pUI->val_size 	= (int)( ((Fl_Slider *)o)->value() ) ;
	height 			= (int)(pUI->val_size / pUI->raytracer->aspectRatio() + 0.5);
	pUI->m_traceGlWindow->resizeWindow( pUI->val_size, height );
}


void TraceUI::cb_slider_depth			(Fl_Widget *o, void *v) { (	(TraceUI*)(o->user_data()) )->val_depth 			= (int)   ( ((Fl_Slider*)o)->value() ); }
void TraceUI::cb_slider_sub_pixel		(Fl_Widget* o, void* v) { ( (TraceUI*)(o->user_data()) )->val_sub_pixel			= (int)	  ( ((Fl_Slider*)o)->value() ); }

void TraceUI::cb_slider_atten_constant	(Fl_Widget *o, void *v) { ( (TraceUI*)(o->user_data()) )->val_atten_constant 	= (double)( ((Fl_Slider*)o)->value() ); }
void TraceUI::cb_slider_atten_linear	(Fl_Widget *o, void *v) { ( (TraceUI*)(o->user_data()) )->val_atten_linear 		= (double)( ((Fl_Slider*)o)->value() ); }
void TraceUI::cb_slider_atten_quadric	(Fl_Widget *o, void *v) { ( (TraceUI*)(o->user_data()) )->val_atten_quadric 	= (double)( ((Fl_Slider*)o)->value() ); }
void TraceUI::cb_slider_atten_ambient	(Fl_Widget *o, void *v) { ( (TraceUI*)(o->user_data()) )->val_ambient 			= (double)( ((Fl_Slider*)o)->value() ); }
void TraceUI::cb_slider_threshold		(Fl_Widget *o, void *v) { ( (TraceUI*)(o->user_data()) )->val_threshold 		= (double)( ((Fl_Slider*)o)->value() ); }


// callback - button
void TraceUI::cb_button_render(Fl_Widget* o, void* v) {
	char buffer[256];

	TraceUI* pUI=((TraceUI*)(o->user_data()));
	
	if (pUI->raytracer->sceneLoaded()) {
		int width=pUI->getSize();
		int	height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
		pUI->m_traceGlWindow->resizeWindow( width, height );

		pUI->m_traceGlWindow->show();

		pUI->raytracer->traceSetup(width, height);
		
		// Save the window label
		const char *old_label = pUI->m_traceGlWindow->label();

		// start to render here	
		done=false;
		clock_t prev, now;
		prev=clock();
		
		pUI->m_traceGlWindow->refresh();
		Fl::check();
		Fl::flush();

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (done) break;
				
				// current time
				now = clock();

				// check event every 1/2 second
				if (((double)(now - prev) / CLOCKS_PER_SEC) > 0.5) {
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

				pUI->raytracer->tracePixel(x, y);
		
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


void TraceUI::cb_button_stop(Fl_Widget *o, void *v) {
	done = true;
}


void TraceUI::cb_button_override_atten(Fl_Widget* o, void* v) {
	((TraceUI*)v)->val_is_override_atten = ((Fl_Light_Button*)o)->value() != 0;
}


void TraceUI::cb_button_override_ambient(Fl_Widget* o, void* v) {
	((TraceUI*)v)->val_is_override_ambient = ((Fl_Light_Button*)o)->value() != 0;
}


void TraceUI::cb_button_background(Fl_Widget* o, void* v) {
	((TraceUI*)v)->val_is_background = ((Fl_Light_Button*)o)->value() != 0;
}


void TraceUI::cb_button_dof(Fl_Widget* o, void* v) {
	((TraceUI*)v)->val_is_dof = ((Fl_Light_Button*)o)->value() != 0;
}


void TraceUI::cb_button_glossy(Fl_Widget* o, void* v) {
	((TraceUI*)v)->val_is_glossy = ((Fl_Light_Button*)o)->value() != 0;
}


void TraceUI::cb_button_soft_shadow(Fl_Widget* o, void* v) {
	((TraceUI*)v)->val_is_soft_shadow = ((Fl_Light_Button*)o)->value() != 0;
}


void TraceUI::cb_button_diffuse(Fl_Widget* o, void* v) {
	((TraceUI*)v)->val_is_diffuse = ((Fl_Light_Button*)o)->value() != 0;
}


// callback - choice
void TraceUI::cb_choice_super_sampling(Fl_Menu_* o, void* v) {
	TraceUI *ui = whoami(o);
	
	int type = (int)v;
	ui->val_super_sampling_method = (RayTracing_SuperSampling_Method)type;

	if (type == RAYTRACING_SUPERSAMPLING_NONE) ui->slider_sub_pixel->deactivate();
	else									   ui->slider_sub_pixel->activate();
}


void TraceUI::show() {
	m_mainWindow->show();
}


void TraceUI::setRayTracer(RayTracer *tracer) {
	raytracer = tracer;
	m_traceGlWindow->setRayTracer(tracer);
}


// getter - value
int 	TraceUI::getSize()					{ return val_size; }
int 	TraceUI::getDepth() 				{ return val_depth; }
int		TraceUI::getSubPixel()				{ return val_sub_pixel; }

double	TraceUI::getAttenConstant()			{ return val_atten_constant; }
double	TraceUI::getAttenLinear()			{ return val_atten_linear; }
double	TraceUI::getAttenQuadric()			{ return val_atten_quadric; }
double	TraceUI::getAmbient()				{ return val_ambient; }
double	TraceUI::getThreshold()				{ return val_threshold; }

bool	TraceUI::getIsOverrideAtten()		{ return val_is_override_atten; }
bool	TraceUI::getIsOverrideAmbient()		{ return val_is_override_ambient; }
bool	TraceUI::getIsBackground()			{ return val_is_background; }
bool	TraceUI::getIsDOF()					{ return val_is_dof; }
bool	TraceUI::getIsGlossy()				{ return val_is_glossy; }
bool	TraceUI::getIsSoftShadow()			{ return val_is_soft_shadow; }
bool	TraceUI::getIsDiffuse()				{ return val_is_diffuse; }

RayTracing_SuperSampling_Method		TraceUI::getSuperSamplingMethod() { return val_super_sampling_method;  }


TraceUI::TraceUI() {
	// init.
	m_mainWindow = new Fl_Window(100, 40, 500, 450, "Ray <Not Loaded>");
	m_mainWindow->user_data((void*)(this));	// record self to be used by static callback functions
	// install menu bar
	m_menubar = new Fl_Menu_Bar(0, 0, 500, 25);
	m_menubar->menu(menuitems);

	// variable preparation
	Graphic_Rect		&rect_slider		= Graphic_Rect(0, 0, 0, 0);
	Graphic_Rect		&rect_render		= Graphic_Rect(0, 0, 0, 0);
	Graphic_Rect		&rect_override		= Graphic_Rect(0, 0, 0, 0);
	Graphic_Rect		&rect_distribute	= Graphic_Rect(0, 0, 0, 0);
	Graphic_Rect		&rect_super_sample	= Graphic_Rect(0, 0, 0, 0);
	Graphic_FontOption	&text				= Graphic_FontOption("", 12);
	Graphic_Range		&range				= Graphic_Range((int)0, 0, 0, 0);

	// slider - depth
	rect_slider = Graphic_Rect(10, 30, 180, 20);
	text = Graphic_FontOption("Depth", 12);
	range = Graphic_Range((int)0, 10, 1, val_depth);
	slider_depth = FL_createValueSlider(&rect_slider, &text, &range, FL_ALIGN_RIGHT, (void*)(this), cb_slider_depth);

	// slider - Size
	rect_slider.y += 25;
	text.text = "Size";
	range = Graphic_Range((int)64, 512, 1, val_size);
	slider_depth = FL_createValueSlider(&rect_slider, &text, &range, FL_ALIGN_RIGHT, (void*)(this), cb_slider_size);

	// slider - attenuation - constant
	rect_slider.y += 25;
	text.text = "Attenuation, Constant";
	range = Graphic_Range((double)0.0, 1.0, 0.01, val_atten_constant);
	slider_atten_constant = FL_createValueSlider(&rect_slider, &text, &range, FL_ALIGN_RIGHT, (void*)(this), cb_slider_atten_constant);

	// slider - attenuation - linear
	rect_slider.y += 25;
	text.text = "Attenuation, Linear";
	range = Graphic_Range((double)0.0, 1.0, 0.01, val_atten_linear);
	slider_atten_linear = FL_createValueSlider(&rect_slider, &text, &range, FL_ALIGN_RIGHT, (void*)(this), cb_slider_atten_linear);

	// slider - attenuation - quadric
	rect_slider.y += 25;
	text.text = "Attenuation, Quadric";
	range = Graphic_Range((double)0.0, 1.0, 0.01, val_atten_quadric);
	slider_atten_quadric = FL_createValueSlider(&rect_slider, &text, &range, FL_ALIGN_RIGHT, (void*)(this), cb_slider_atten_quadric);

	// slider - ambient
	rect_slider.y += 25;
	text.text = "Ambient Light";
	range = Graphic_Range((double)0.0, 1.0, 0.01, val_ambient);
	slider_ambient = FL_createValueSlider(&rect_slider, &text, &range, FL_ALIGN_RIGHT, (void*)(this), cb_slider_atten_ambient);

	// slider - threshold
	rect_slider.y += 25;
	text.text = "Threshold";
	range = Graphic_Range((double)0.0, 1.0, 0.01, val_threshold);
	slider_threshold = FL_createValueSlider(&rect_slider, &text, &range, FL_ALIGN_RIGHT, (void*)(this), cb_slider_threshold);

	// choice - super sampling
	rect_slider.y += 25;
	rect_super_sample = rect_slider;
	rect_super_sample.w = 100;
	text.text = "Super Sampling";
	choice_super_sampling = FL_createChoice(&rect_super_sample, &text, menuItem_super_sampling, FL_ALIGN_RIGHT, nullptr, nullptr);

	// slider - sub pixel
	rect_super_sample.x += rect_super_sample.w + 110;
	rect_super_sample.w = 180;
	text.text = "Sub-Pixel";
	range = Graphic_Range((int)1, 5, 1, val_sub_pixel);
	slider_sub_pixel = FL_createValueSlider(&rect_super_sample, &text, &range, FL_ALIGN_RIGHT, (void*)(this), cb_slider_sub_pixel);
	slider_sub_pixel->deactivate();

	// button - render
	rect_render = Graphic_Rect(240, 27, 70, 25);
	text.text = "&Render";
	button_render = FL_createButton(&rect_render, &text, FL_ALIGN_NOWRAP, (void*)(this), cb_button_render);

	// button - stop
	rect_render = Graphic_Rect(240, 55, 70, 25);
	text.text = "&Stop";
	button_stop = FL_createButton(&rect_render, &text, FL_ALIGN_NOWRAP, (void*)(this), cb_button_stop);

	// button - override atten
	rect_override = Graphic_Rect(350, 27, 140, 25);
	text.text = "Override Atten";
	button_override_atten = FL_createLightButton(&rect_override, &text, FL_ALIGN_NOWRAP, 0, (void*)(this), cb_button_override_atten);
	
	// button - override ambient
	rect_override = Graphic_Rect(350, 55, 140, 25);
	text.text = "Override Ambient";
	button_override_ambient = FL_createLightButton(&rect_override, &text, FL_ALIGN_NOWRAP, 0, (void*)(this), cb_button_override_ambient);

	// button - dof
	rect_distribute.y = rect_slider.y;
	rect_distribute.y += 25;
	rect_distribute.x = 10;
	rect_distribute.w = 100;
	rect_distribute.h = 25;
	text.text = "DOF";
	button_dof = FL_createLightButton(&rect_distribute, &text, FL_ALIGN_NOWRAP, 0, (void*)(this), cb_button_dof);
	button_dof->deactivate();  // TODO: currently deactivated

	// button - glossy
	rect_distribute.x += rect_distribute.w + 10;
	text.text = "Glossy";
	button_glossy = FL_createLightButton(&rect_distribute, &text, FL_ALIGN_NOWRAP, 0, (void*)(this), cb_button_glossy);

	// button - soft shadow
	rect_distribute.x += rect_distribute.w + 10;
	text.text = "Soft Shadow";
	button_soft_shadow = FL_createLightButton(&rect_distribute, &text, FL_ALIGN_NOWRAP, 0, (void*)(this), cb_button_soft_shadow);

	// button - diffuse
	rect_distribute.x += rect_distribute.w + 10;
	text.text = "Diffuse";
	button_diffuse = FL_createLightButton(&rect_distribute, &text, FL_ALIGN_NOWRAP, 0, (void*)(this), cb_button_diffuse);

	// main window
	m_mainWindow->callback(cb_exit2);
	m_mainWindow->when(FL_HIDE);
    m_mainWindow->end();

	// image view
	m_traceGlWindow = new TraceGLWindow(100, 150, val_size, val_size, "Rendered Image");
	m_traceGlWindow->end();
	m_traceGlWindow->resizable(m_traceGlWindow);
}


// Static Function Implementation
static Fl_Value_Slider* FL_createValueSlider(Graphic_Rect* rect, Graphic_FontOption* font, Graphic_Range* range, Fl_Align align, void* data, Fl_Callback* cb) {
	Fl_Value_Slider* widget = new Fl_Value_Slider(rect->x, rect->y, rect->w, rect->h, font->text);
	widget->type(FL_HOR_NICE_SLIDER);
	widget->labelfont(font->font);
	widget->labelsize(font->size);
	widget->align(align);
	widget->user_data(data);
	widget->callback(cb);

	switch (range->val_type) {
	case GRAPHIC_UNIONTYPE_INTEGER:
		widget->minimum(range->minimum.val_i);
		widget->maximum(range->maximum.val_i);
		widget->step(range->step.val_i);
		widget->value(range->value.val_i);
		break;

	case GRAPHIC_UNIONTYPE_FLOAT:
		widget->minimum(range->minimum.val_f);
		widget->maximum(range->maximum.val_f);
		widget->step(range->step.val_f);
		widget->value(range->value.val_f);
		break;

	case GRAPHIC_UNIONTYPE_DOUBLE:
		widget->minimum(range->minimum.val_d);
		widget->maximum(range->maximum.val_d);
		widget->step(range->step.val_d);
		widget->value(range->value.val_d);
		break;
	}

	return widget;
}


static Fl_Button* FL_createButton(Graphic_Rect* rect, Graphic_FontOption* font, Fl_Align align, void* data, Fl_Callback* cb) {
	Fl_Button *widget = new Fl_Button(rect->x, rect->y, rect->w, rect->h, font->text);

	widget->labelfont(font->font);
	widget->labelsize(font->size);
	if (align != FL_ALIGN_NOWRAP) widget->align(align);
	widget->user_data(data);
	widget->callback(cb);

	return widget;
}


static Fl_Light_Button* FL_createLightButton(Graphic_Rect* rect, Graphic_FontOption* font, Fl_Align align, int value, void* data, Fl_Callback* cb) {
	Fl_Light_Button *widget = new Fl_Light_Button(rect->x, rect->y, rect->w, rect->h, font->text);

	widget->labelfont(font->font);
	widget->labelsize(font->size);
	if (align != FL_ALIGN_NOWRAP) widget->align(align);
	widget->value(value);
	widget->user_data(data);
	widget->callback(cb);

	return widget;
}


static Fl_Choice* FL_createChoice(Graphic_Rect* rect, Graphic_FontOption* font, Fl_Menu_Item* item, Fl_Align align, void* data, Fl_Callback* cb) {
	Fl_Choice* widget = new Fl_Choice(rect->x, rect->y, rect->w, rect->h, font->text);

	widget->labelfont(font->font);
	widget->labelsize(font->size);
	if (align != FL_ALIGN_NOWRAP) widget->align(align);
	widget->menu(item);
	widget->user_data(data);
	widget->callback(cb);

	return widget;
}
