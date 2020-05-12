#include <algorithm>
#include "ray.h"
#include "material.h"
#include "light.h"
#include "../ui/TraceUI.h"


// Static Function Prototype
// TODO: may need to move to other places
// find the ambient intensity of certain point
static vec3f RayTrace_PhongModel_getAmbientLightIntensity(Scene *scene, const vec3f &point);


// Operation
// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
vec3f Material::shade( Scene *scene, const ray& r, const isect& i ) const {
	// YOUR CODE HERE:

	// Naming Convention
	// although they both use the same variable type, but they are not the same thing
	// naming convention must straightly follow to prevent any unecessary debugging time
	// vec3f:
	// - raw (usually const, can be used by others, i.e. point, vector, etc)
	// - point
	// - vector
	// - intensity
	// - result (intensity)
	//
	// double:
	// - dot

	// variable preparation
	const vec3f point_isect = r.at(i.t);
	const vec3f raw_one(1.0, 1.0, 1.0);
	
	// emission
	// explanation to kt
	// some light will directly pass throught the material
	// causing the intensity reduction
	// intensity remain = 1 - kt, where 0 <= kt <= 1
	vec3f intensity_result = ke;
	const vec3f &intensity_ambient = RayTrace_PhongModel_getAmbientLightIntensity(scene, point_isect);
	
	// ambient
	if (traceUI->getIsOverrideAmbient()) {
		const double ambient = traceUI->getAmbient();
		intensity_result += prod(prod(ka, vec3f(ambient, ambient, ambient)), raw_one - kt);
	} else {
		intensity_result += prod(prod(ka, intensity_ambient), raw_one - kt);
	}

	for (auto* light : scene->getLights()) {
		const double dot_ln = i.N.dot(light->getDirection(point_isect));
		
		// if the light source is behind the plane,
		// then ignore it
		if (dot_ln <= 0.0) continue;

		// shadow attenuation
		// if the atten_shadow is zero, which means no light from the source (blocked)
		// then ignore it
		const vec3f &atten_shadow = light->shadowAttenuation(point_isect);
		if (atten_shadow.iszero()) continue;

		// distance attenuation
		// normally the value will not be zero (1 / d^2) if d != inf
		// TODO: give a threshold to a light intensity smaller than some value to be zero
		// TOOD: to reduce the workload
		const double atten_distance = light->distanceAttenuation(point_isect);
		const vec3f& attenuation = atten_shadow * atten_distance;

		// diffuse term
		const vec3f& term_diffuse = prod(kd * dot_ln, raw_one - kt);

		// specular term
		// reflected = 2 * projection of income ray on normal - income ray
		// be careful of the direction
		// direction of light->getDirection(): away from the intersection
		const vec3f& ray_reflect	= (2.0 * dot_ln * i.N - light->getDirection(point_isect)).normalize();
		const double dot_rv			= std::max<double>(ray_reflect.dot(-r.getDirection()), 0.0);
		const double coeff_specular = pow(dot_rv, shininess * 128);  // 128 is power of 2
		const vec3f& term_specular	= ks * coeff_specular;

		// diffuse term + specular term
		const vec3f& term_result = term_diffuse + term_specular;

		// light intensity
		const vec3f& intensity_light = light->getColor(point_isect);

		// result += attenuation * term
		intensity_result += prod(prod(attenuation, intensity_light), term_result);
	}

	return intensity_result;
}


// Static Function Implementation
static vec3f RayTrace_PhongModel_getAmbientLightIntensity(Scene *scene, const vec3f &point) {
	vec3f result;
	for (auto* light : scene->getAmbientLights()) {
		result += light->getColor(point);  // vec3f += vec3f;
	}
	return result;
}
