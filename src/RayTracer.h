#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__


#include <stack>
#include "scene/scene.h"
#include "scene/ray.h"


// Data Structure
class RayTracer {

// Data
protected:

	class RayData{
	// Data
	public:
		Scene*			scene	= nullptr;
		const ray*		r		= nullptr;
		const isect*	i		= nullptr;
		vec3f			thresh	= vec3f();
		int				depth	= -1;

		// record whether in which object and in what order
		std::stack<const SceneObject*> object_stack;

	// Operation Handling
	public:
		explicit RayData() {}

		RayData(Scene* scene, const ray* r, const isect* i, vec3f thresh, int depth) :
			scene(scene), r(r), i(i), thresh(thresh), depth(depth)
		{}
	};

public:
	// Operation Handling
    RayTracer();
    ~RayTracer();

    vec3f trace(Scene *scene, double x, double y);
	vec3f traceRay(Scene *scene, const ray& r, const vec3f& thresh, int depth );
	vec3f traceRay(RayData* data);

	void getBuffer( unsigned char *&buf, int &w, int &h );
	double aspectRatio();
	void traceSetup( int w, int h );
	void traceLines( int start = 0, int stop = 10000000 );
	void tracePixel( int i, int j );

	bool loadScene( char* fn );

	bool sceneLoaded();

protected:
	vec3f	traceLightSource(const RayData* data);
	vec3f	traceReflection(const RayData *data);
	vec3f	traceRefraction(const RayData *data);

private:
	unsigned char *buffer;
	int buffer_width, buffer_height;
	int bufferSize;
	Scene *scene;

	bool m_bSceneLoaded;
};


#endif // __RAYTRACER_H__
