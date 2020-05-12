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
#include "TraceGLWindow.h"
#include "../RayTracing_Base.h"


// Data Structure
class TraceUI {
public:
	// Member
	// Constructor
	TraceUI();

	// Data
	// TODO: should these be private / protected ?
	Fl_Window			*m_mainWindow;
	Fl_Menu_Bar			*m_menubar;

	Fl_Slider			*slider_depth;
	Fl_Slider			*slider_size;
	Fl_Slider			*slider_atten_constant;
	Fl_Slider			*slider_atten_linear;
	Fl_Slider			*slider_atten_quadric;
	Fl_Slider			*slider_ambient;
	Fl_Slider			*slider_threshold;
	Fl_Slider			*slider_sub_pixel;

	Fl_Button			*button_render;
	Fl_Button			*button_stop;

	Fl_Light_Button		*button_override_atten;
	Fl_Light_Button		*button_override_ambient;
	Fl_Light_Button		*button_background;
	Fl_Light_Button		*button_dof;
	Fl_Light_Button		*button_glossy;
	Fl_Light_Button		*button_soft_shadow;
	Fl_Light_Button		*button_diffuse;

	Fl_Choice			*choice_super_sampling;

	TraceGLWindow		*m_traceGlWindow;

	// Operation Handling
	void 				show();
	void				setRayTracer(RayTracer *tracer);

	int					getSize				();
	int					getDepth			();
	int					getSubPixel();

	double				getAttenConstant	();
	double				getAttenLinear		();
	double				getAttenQuadric		();
	double				getAmbient			();
	double				getThreshold		();

	bool				getIsOverrideAtten	();
	bool				getIsOverrideAmbient();
	bool				getIsBackground		();
	bool				getIsDOF			();
	bool				getIsGlossy			();
	bool				getIsSoftShadow		();
	bool				getIsDiffuse		();

	// TODO: the name is f**king too long
	RayTracing_SuperSampling_Method	getSuperSamplingMethod();

private:
	// Data
	RayTracer*	raytracer;

	bool				val_is_override_atten	= false;
	bool				val_is_override_ambient = false;
	bool				val_is_background		= false;
	bool				val_is_dof				= false;
	bool				val_is_glossy			= false;
	bool				val_is_soft_shadow		= false;
	bool				val_is_diffuse			= false;

	int					val_size				= 150;
	int					val_depth				= 0;
	int					val_sub_pixel = 2;

	double				val_atten_constant		= 0.0;
	double				val_atten_linear		= 0.0;
	double				val_atten_quadric		= 1.0;
	double				val_ambient				= 0.0;
	double				val_threshold			= 0.2;

	// TODO: the name is f**king too long
	RayTracing_SuperSampling_Method val_super_sampling_method = RAYTRACING_SUPERSAMPLING_NONE;

protected:
	// Static
	// Static Data
	static Fl_Menu_Item		menuitems[];
	static Fl_Menu_Item		menuItem_super_sampling[RAYTRACING_SUPERSAMPLING_MAX + 1 + 1];

protected:
	// Static Function
	static TraceUI* whoami						(Fl_Menu_ *o);

	// callback
	static void cb_load_scene					(Fl_Menu_ *o, void *v);
	static void cb_save_image					(Fl_Menu_ *o, void *v);
	static void cb_menu_load_background			(Fl_Menu_* o, void* v);
	static void cb_exit							(Fl_Menu_ *o, void *v);
	static void cb_about						(Fl_Menu_ *o, void *v);
	static void cb_exit2						(Fl_Widget *o, void *v);

	static void cb_slider_size					(Fl_Widget *o, void *v);
	static void cb_slider_depth					(Fl_Widget *o, void *v);
	static void cb_slider_atten_constant		(Fl_Widget *o, void *v);
	static void cb_slider_atten_linear			(Fl_Widget *o, void *v);
	static void cb_slider_atten_quadric			(Fl_Widget *o, void *v);
	static void cb_slider_atten_ambient			(Fl_Widget *o, void *v);
	static void cb_slider_threshold				(Fl_Widget *o, void *v);
	static void cb_slider_sub_pixel				(Fl_Widget *o, void *v);

	static void cb_button_render				(Fl_Widget *o, void *v);
	static void cb_button_stop					(Fl_Widget *o, void *v);
	static void cb_button_override_atten		(Fl_Widget* o, void* v);
	static void cb_button_override_ambient		(Fl_Widget* o, void* v);
	static void cb_button_background			(Fl_Widget* o, void* v);
	static void cb_button_dof					(Fl_Widget* o, void* v);
	static void cb_button_glossy				(Fl_Widget* o, void* v);
	static void cb_button_soft_shadow			(Fl_Widget* o, void* v);
	static void cb_button_diffuse				(Fl_Widget* o, void* v);

	static void cb_choice_super_sampling		(Fl_Menu_*o, void *v);
};


// Extern Data
extern TraceUI* traceUI;


#endif
