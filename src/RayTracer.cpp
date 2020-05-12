// The main ray tracer.

#include <Fl/fl_ask.h>
#include <stdlib.h>
#include <stack>
#include <cmath>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"
#include "vecmath/vecmath.h"
#include "ui/TraceUI.h"



// Data Structure
class vec2f {
public:
	// Data
	double n[2] = { 0, 0 };

public:
	// Operation Handling
	vec2f()									{ n[0] = 0.0; n[1] = 0.0; }
	vec2f(const double x, const double y)	{ n[0] = x; n[1] = y; }
	vec2f(const vec2f& v)					{ n[0] = v.n[0]; n[1] = v.n[1]; }

	vec2f& operator	=	(const vec3f& v)	{ n[0] = v.n[0]; n[1] = v.n[1]; return *this; }
	vec2f& operator +=	(const vec3f& v)	{ n[0] += v.n[0]; n[1] += v.n[1]; return *this; }
	vec2f& operator -=	(const vec3f& v)	{ n[0] -= v.n[0]; n[1] -= v.n[1]; return *this; }
	vec2f& operator *=	(const double d)	{ n[0] *= d; n[1] *= d; return *this; }
	vec2f& operator /=	(const double d)	{ n[0] /= d; n[1] /= d; return *this; }

	double& operator []	(int i)				{ return n[i]; }
	double	operator []	(int i) const		{ return n[i]; }
};


class TracerData {
public:
	// Data
	RayTracer* tracer = nullptr;
	Scene* scene = nullptr;

	// Operation Handling
	TracerData(RayTracer *tracer, Scene* scene):
		tracer(tracer), scene(scene) {}
};


// Typedef
typedef bool(*get_sample_t)(vec2f*, vec2f*, int);


// Static Function Implementation
// ray tracing
static bool		RayTracing_SuperSampling_getSample_grid		(vec2f* dst, vec2f* region, int n);
static bool		RayTracing_SuperSampling_getSample_random	(vec2f* dst, vec2f* region, int n);
static bool		RayTracing_SuperSampling_getSample_jittered	(vec2f* dst, vec2f* region, int n);

static vec3f	RayTracing_SuperSampling_adaptive			(vec2f* src, vec2f *region, int depth, vec3f(*tracer)(double, double, void*), void* info);

// linker
// TODO: should be put into RayTracer class
static vec3f	Linker_tracer								(double x, double y, void *info);


// Static Data
// gaussian kernel
// gaussian kernel calculator: http://dev.theomader.com/gaussian-kernel-calculator/
// ...

// super sampling
// use stack as not to altering the stack too rapidly
static get_sample_t super_sampling_method[RAYTRACING_SUPERSAMPLING_MAX] = {
	RayTracing_SuperSampling_getSample_grid,
	RayTracing_SuperSampling_getSample_random,
	RayTracing_SuperSampling_getSample_jittered
};

// random seed
// for testing purpose, everything should be controlable and the result must be expected 
// ...


// Operation Handling
// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace(Scene *scene, double x, double y) {
    ray r( vec3f(0,0,0), vec3f(0,0,0) );
    scene->getCamera()->rayThrough( x,y,r );

	const int		depth	= traceUI->getDepth();
	const double	thresh	= 1.0;

	return traceRay( scene, r, vec3f(thresh, thresh, thresh), depth).clamp();
}


// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay(Scene *scene, const ray& r, const vec3f& thresh, int depth) {
	// YOUR CODE HERE:
	
	// TODO: not yet completed
	// Material air;
	
	RayData data(scene, &r, nullptr, thresh, depth);
	return traceRay(&data);
}


// the number of parameter is too much, use struct instead
vec3f RayTracer::traceRay(RayData *data) {
	// YOUR CODE HERE:

	// check threshold
	// if the current light ray contributes too less to the pixel,
	// then it can be ignored
	if (data->thresh[0] <= traceUI->getThreshold() &&
		data->thresh[1] <= traceUI->getThreshold() &&
		data->thresh[2] <= traceUI->getThreshold()) return vec3f();

	// no intersection
	// this ray travels to infinity
	// color it according to the background color
	// which in this (simple) case is just black
	isect i;
	if (data->scene->intersect(*(data->r), i) == false) return vec3f();

	// check if leaving or entering the object
	// if leaving the object,
	// then need to correct the normal (for unity)
	if (!data->object_stack.empty() && data->object_stack.top() == i.obj) {  // in object, leaving the object
		i.N = -i.N;
	}

	// variable preparation
	data->i = &i;
	vec3f intensity_result = vec3f();

	const vec3f intensity_shade		= traceLightSource(data);
	const vec3f intensity_reflect	= traceReflection(data);
	const vec3f intensity_refract	= traceRefraction(data);

	intensity_result = intensity_shade + intensity_reflect + intensity_refract;
	return intensity_result;
}


vec3f RayTracer::traceLightSource(const RayData* data) {
	const Material& m = data->i->getMaterial();

	if (!traceUI->getIsSoftShadow()) {
		const vec3f &result = m.shade(data->scene, *(data->r), *(data->i));
		return prod(result, data->thresh);

	} else {
		const double radius_cone = 0.1;
		const vec3f& ray_offset_x = ((data->i->N).cross(data->r->getDirection())).normalize();
		const vec3f& ray_offset_y = ((data->i->N).cross(ray_offset_x)).normalize();

		vec3f& result = vec3f();
		for (int i = 0; i < 5; i++) {
			
			int rand_i;
			double rand_d;
			vec3f point_distributed = data->r->at(data->i->t);

			rand_i = rand() % 100;
			rand_d = (((double)rand_i) / 100.0) - 0.5;
			point_distributed += ray_offset_x * rand_d * radius_cone;

			rand_i = rand() % 100;
			rand_d = (((double)rand_i) / 100.0) - 0.5;
			point_distributed += ray_offset_y * rand_d * radius_cone;

			isect i_next = *(data->i);
			const vec3f& direction = point_distributed - data->r->getPosition();
			i_next.t = direction.length();
			ray r_next = ray(data->r->getPosition(), direction.normalize());
			result += m.shade(data->scene, r_next, i_next);
		}

		result /= 5;
		return prod(result, data->thresh);
	}
}


vec3f RayTracer::traceReflection(const RayData *data) {
	// check if the depth is reached
	// if reached, assume that the light disappeared suddenly
	if (data->depth <= 0) return vec3f();

	// variable preparation
	const Material& m			= data->i->getMaterial();
	const vec3f& point_isect	= data->r->at(data->i->t);
	const double dot_ln			= data->i->N.dot(-data->r->getDirection());  // income (l) * normal (N)

	// check kr
	// if kr == 0, then no need to continue
	if (m.kr.iszero())	return vec3f();
	// if (dot_ln < 0)		return vec3f();  // trust operation

	// reflected ray
	// be careful of the direction
	// direction of data->r->getDirection(): toward the intersection
	const vec3f& ray_reflect	= (2.0 * dot_ln * data->i->N + data->r->getDirection()).normalize();

	// recursion
	// TODO: need prettier
	if (!traceUI->getIsDiffuse()) {
		const vec3f& point_out = point_isect + ray_reflect * RAY_EPSILON;  // need to push the point a little bit forward to prevent hit the same point
		ray r_reflect(point_out, ray_reflect);
		RayData data_next;

		data_next.scene = data->scene;
		data_next.r = &r_reflect;
		data_next.i = nullptr;  // hide information of the prev intersection
		data_next.thresh = prod(data->thresh, m.kr);
		data_next.depth = data->depth - 1;
		data_next.object_stack = data->object_stack;

		return traceRay(&data_next);

	} else {
		const vec3f& ray_offset_x = (ray_reflect.cross(data->i->N)).normalize();
		const vec3f& ray_offset_y = (ray_reflect.cross(ray_offset_x)).normalize();
		const double radius_cone = 0.05;

		// TODO: currently the number of ray fired is fixed
		vec3f& result = vec3f();
		for (int i = 0; i < 5; i++) {

			int rand_i;
			double rand_d;
			vec3f ray_distributed = ray_reflect;

			rand_i = rand() % 100;
			rand_d = (((double)rand_i) / 100.0) - 0.5;
			ray_distributed += ray_offset_x * rand_d * radius_cone;

			rand_i = rand() % 100;
			rand_d = (((double)rand_i) / 100.0) - 0.5;
			ray_distributed += ray_offset_y * rand_d * radius_cone;
			ray_distributed = ray_distributed.normalize();

			const vec3f& point_out = point_isect + ray_distributed * RAY_EPSILON;  // need to push the point a little bit forward to prevent hit the same point
			ray r_reflect(point_out, ray_distributed);
			RayData data_next;

			data_next.scene = data->scene;
			data_next.r = &r_reflect;
			data_next.i = nullptr;  // hide information of the prev intersection
			data_next.thresh = prod(data->thresh, m.kr);
			data_next.depth = data->depth - 1;
			data_next.object_stack = data->object_stack;

			result += traceRay(&data_next);
		}

		result /= 5;
		return result;
	}
}


vec3f RayTracer::traceRefraction(const RayData *data) {
	// check if the depth is reached
	// if reached, assume that the light disappeared suddenly
	if (data->depth <= 0) return vec3f();

	// variable preparation
	const Material& m			= data->i->getMaterial();
	const vec3f& point_isect	= data->r->at(data->i->t);
	const double dot_ln			= data->i->N.dot(-data->r->getDirection());
	std::stack<const SceneObject*> object_stack = data->object_stack;

	// check kr
	// if kr == 0, then no need to continue
	if (m.kt.iszero()) return vec3f();

	// check enter or leave the object
	// TODO: make it prettier
	double n_i, n_t;
	if (data->object_stack.empty()) {
		n_i = 1;
		n_t = m.index;
		object_stack.push(data->i->obj);
	}
	else if (data->i->obj != data->object_stack.top()) {
		n_i = data->object_stack.top()->getMaterial().index;
		n_t = m.index;
		object_stack.push(data->i->obj);
	}
	else {
		n_i = m.index;
		n_t = data->object_stack.size() == 1 ? 
			1 : 
			data->object_stack.top()->getMaterial().index;
		object_stack.pop();
	}

	// refracted ray
	const double n_r = n_i / n_t;
	const double dot_rn = data->i->N.dot(-data->r->getDirection());
	const double root = 1 - n_r * n_r * (1 - dot_rn * dot_rn);

	// check if total internal refraction
	// if (root < RAY_EPSILON) return vec3f();
	if (root < 0.0) return vec3f();

	// refracted ray (continued)
	const double coeff			= n_r * dot_rn - sqrt(root);
	const vec3f& ray_refract	= coeff * data->i->N - n_r * (-data->r->getDirection());

	// recursion
	if (!traceUI->getIsGlossy()) {
		const vec3f& point_out = point_isect + ray_refract * RAY_EPSILON;
		ray r_refract(point_out, ray_refract);
		RayData data_next;

		data_next.scene = data->scene;
		data_next.r = &r_refract;
		data_next.i = nullptr;  // hide information of the prev intersection
		data_next.thresh = prod(data->thresh, m.kt);
		data_next.depth = data->depth - 1;
		data_next.object_stack = object_stack;

		return traceRay(&data_next);

	} else {
		const vec3f& ray_offset_x = (ray_refract.cross(data->i->N)).normalize();
		const vec3f& ray_offset_y = (ray_refract.cross(ray_offset_x)).normalize();

		// TODO: currently the number of ray fired is fixed
		vec3f& result = vec3f();
		for (int i = 0; i < 5; i++) {

			const double radius_cone = 0.05;
			int rand_i;
			double rand_d;
			vec3f ray_distributed = ray_refract;

			rand_i = rand() % 100;
			rand_d = (((double)rand_i) / 100.0) - 0.5;
			ray_distributed += ray_offset_x * rand_d * radius_cone;

			rand_i = rand() % 100;
			rand_d = (((double)rand_i) / 100.0) - 0.5;
			ray_distributed += ray_offset_y * rand_d * radius_cone;
			ray_distributed = ray_distributed.normalize();

			const vec3f& point_out = point_isect + ray_distributed * RAY_EPSILON;  // need to push the point a little bit forward to prevent hit the same point
			ray r_refract(point_out, ray_distributed);
			RayData data_next;

			data_next.scene = data->scene;
			data_next.r = &r_refract;
			data_next.i = nullptr;  // hide information of the prev intersection
			data_next.thresh = prod(data->thresh, m.kt);
			data_next.depth = data->depth - 1;
			data_next.object_stack = object_stack;

			result += traceRay(&data_next);
		}

		result /= 5;
		return result;

	}
}


RayTracer::RayTracer() {
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;

	m_bSceneLoaded = false;
}


RayTracer::~RayTracer() {
	delete [] buffer;
	delete scene;
}


void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h ) {
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}


double RayTracer::aspectRatio() {
	return scene ? scene->getCamera()->getAspectRatio() : 1;
}


bool RayTracer::sceneLoaded() {
	return m_bSceneLoaded;
}


bool RayTracer::loadScene( char* fn ) {
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


void RayTracer::traceSetup( int w, int h ) {
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


void RayTracer::traceLines( int start, int stop ) {
	// vec3f col;
	if(!scene) return;

	if(stop > buffer_height) stop = buffer_height;

	// trace pixel
	for (int j = start; j < stop; ++j) {
		for (int i = 0; i < buffer_width; ++i) {
			tracePixel(i, j);
		}
	}
}


void RayTracer::tracePixel(int i, int j) {
	if (!scene) return;

	// get center pixel
	const double x = double(i) / double(buffer_width);
	const double y = double(j) / double(buffer_height);

	vec3f result = vec3f();
	RayTracing_SuperSampling_Method method = traceUI->getSuperSamplingMethod();

	// trace pixel
	// super sampling - none
	if (method == RAYTRACING_SUPERSAMPLING_NONE) {
		result = trace(scene, x, y);
	}

	// super sampling - adaptive (subdivision method - grid)
	// TODO: currently the threshold of intensity gradient is fixed
	// the value getSubPixel in this case indicate the max depth that can go into
	else if (method == RAYTRACING_SUPERSAMPLING_ADAPTIVE) {
		// variable preparation
		const int		n		= traceUI->getSubPixel();
		const double	pixel_w = 1.0 / double(buffer_width);
		const double	pixel_h = 1.0 / double(buffer_height);
		vec2f			region	= vec2f(pixel_w / 2, pixel_h / 2);
		vec2f			src		= vec2f(x, y);
		TracerData		data	= TracerData(this, scene);

		// adaptve recursion
		result = RayTracing_SuperSampling_adaptive(&src, &region, n, Linker_tracer, &data);
	}

	// super sampling - rest
	else {
		// variable preparation
		const int		n		= traceUI->getSubPixel();
		const double	pixel_w = 1.0 / double(buffer_width);
		const double	pixel_h = 1.0 / double(buffer_height);
		vec2f			region	= vec2f(pixel_w / 2, pixel_h / 2);
		vec2f			sample_dis[5 * 5];  // current max sub-pixel = 5

		// get sampling displacement
		super_sampling_method[method](sample_dis, &region, n);

		// super sampling
		int index = 0;
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				result += trace(scene, x + sample_dis[index][0], y + sample_dis[index][1]);
				index++;
			}
		}
		result /= (double(n) * double(n));
	}

	// fill the pixel
	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;
	pixel[0] = (int)( 255.0 * result[0]);
	pixel[1] = (int)( 255.0 * result[1]);
	pixel[2] = (int)( 255.0 * result[2]);
}


// Static Function Implementation
static bool RayTracing_SuperSampling_getSample_grid(vec2f* dst, vec2f* region, int n) {
	// special case handling
	if (n == 1) {
		dst[0] = vec2f(0, 0);
		return true;
	}

	const double w_region_half = (*region)[0];
	const double h_region_half = (*region)[1];
	const double w_pixel = w_region_half * 2 / (n - 1);
	const double h_pixel = h_region_half * 2 / (n - 1);

	for (int x = 0; x < n; x++) {
		for (int y = 0; y < n; y++) {
			dst[y * n + x] 
				= vec2f(-w_region_half + x * w_pixel, -h_region_half + y * h_pixel);
		}
	}

	return true;
}


static bool RayTracing_SuperSampling_getSample_random(vec2f* dst, vec2f* region, int n) {
	// special case handling
	if (n == 1) {
		dst[0] = vec2f(0, 0);
		return true;
	}

	const double w_region_half = (*region)[0];
	const double h_region_half = (*region)[1];
	const double w_pixel = w_region_half * 2 / (n - 1);
	const double h_pixel = h_region_half * 2 / (n - 1);

	int index = 0;
	for (int x = 0; x < n; x++) {
		for (int y = 0; y < n; y++) {

			const int rand_i = rand() % 100;
			const double rand_d = (((double)rand_i) / 100.0) - 0.5;

			dst[index][0] = w_region_half * rand_d * 2;
			dst[index][1] = h_region_half * rand_d * 2;
			index++;

		}
	}

	return true;
}


static bool RayTracing_SuperSampling_getSample_jittered(vec2f* dst, vec2f* region, int n) {
	// special case handling
	if (n == 1) {
		dst[0] = vec2f(0, 0);
		return true;
	}

	const double w_region_half = (*region)[0];
	const double h_region_half = (*region)[1];
	const double w_pixel = w_region_half * 2 / (n - 1);
	const double h_pixel = h_region_half * 2 / (n - 1);

	for (int x = 0; x < n; x++) {
		for (int y = 0; y < n; y++) {

			const int rand_i = rand() % 100;
			const double rand_d = (((double)rand_i) / 100.0) - 0.5;

			dst[y * n + x]
				= vec2f(
					-w_region_half + x * w_pixel + w_pixel * rand_d, 
					-h_region_half + y * h_pixel + h_pixel * rand_d);

		}
	}

	return true;
}


static vec3f RayTracing_SuperSampling_adaptive(vec2f* src, vec2f* region, int depth, vec3f(*tracer)(double, double, void*), void* info) {
	// variable preparation
	vec3f result = vec3f();
	vec3f temp[3 * 3];
	vec2f sample_dis[3 * 3];
	const double threshold = 0.2;

	// get sample displacement
	RayTracing_SuperSampling_getSample_grid(sample_dis, region, 3);

	// get the result
	int index = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			temp[index] = tracer((*src)[0] + sample_dis[index][0], (*src)[1] + sample_dis[index][1], info);
			result += temp[index];
			index++;
		}
	}

	// check if reach the max depth
	if (depth <= 0) {
		result /= 9;
		return result;
	}

	// analyse if need to going deep
	// if need to go deep, then the result will be replaced by the recursion result
	// TODO: create a function for the gradient kernel work
	const vec3f& gradient_v = temp[1] - temp[7];
	const vec3f& gradient_h = temp[3] - temp[5];

	if (abs(gradient_v[0]) > threshold || abs(gradient_v[1]) > threshold || abs(gradient_v[2]) > threshold ||
		abs(gradient_h[0]) > threshold || abs(gradient_h[1]) > threshold || abs(gradient_h[2]) > threshold) {

		vec2f& region_next = vec2f((*region)[0] / 2, (*region)[1] / 2);
		vec2f& src_next = vec2f();
		result = vec3f();

		// top
		src_next = vec2f((*src)[0], (*src)[1] - region_next[1]);
		result += RayTracing_SuperSampling_adaptive(&src_next, &region_next, depth - 1, tracer, info);

		// left
		src_next = vec2f((*src)[0] - region_next[0], (*src)[1]);
		result += RayTracing_SuperSampling_adaptive(&src_next, &region_next, depth - 1, tracer, info);

		// right
		src_next = vec2f((*src)[0] + region_next[0], (*src)[1]);
		result += RayTracing_SuperSampling_adaptive(&src_next, &region_next, depth - 1, tracer, info);

		// bottom
		src_next = vec2f((*src)[0], (*src)[1] + region_next[1]);
		result += RayTracing_SuperSampling_adaptive(&src_next, &region_next, depth - 1, tracer, info);

		// center
		src_next = vec2f((*src)[0], (*src)[1]);
		result += RayTracing_SuperSampling_adaptive(&src_next, &region_next, depth - 1, tracer, info);

		result /= 5;
	}
	else {  // no need to go deep
		result /= 9;
	}

	return result;
}


// TODO: should be put into RayTracer class
static vec3f Linker_tracer(double x, double y, void* info) {
	TracerData* data = (TracerData*)info;
	return data->tracer->trace(data->scene, x, y);
}
