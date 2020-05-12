#include <cmath>
#include "light.h"
#include "../ui/TraceUI.h"


// Static Function Prototype
// ...


// Operation Handling
double DirectionalLight::distanceAttenuation( const vec3f& P ) const {
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	// range: 1.0 - 1.0
	return 1.0;
}


vec3f DirectionalLight::shadowAttenuation( const vec3f& P ) const {
    // YOUR CODE HERE:
	// range: 0.0 - 1.0

	// variable preparation
	const vec3f& ray_dir = getDirection(P);  // from P to light source
	vec3f point_light = P;
	vec3f intensity_result(1.0, 1.0, 1.0);

	// first push the ray a little bit forward to prevent hit the same point
	// and cause dead loop
	point_light = point_light + ray_dir * RAY_EPSILON;
	while (!intensity_result.iszero()) {

		// TODO: not yet decided the exact naming
		isect i;
		ray r(point_light, ray_dir);

		// check if no intersect or intersection is behind the light source
		if (!scene->intersect(r, i)) return intensity_result;

		// light intensity reduction due to energy loss when passing through the material
		// 
		// Problem:
		// in the actual calculation,
		// that this reduction will only happen when enter or leave the object
		// but the in realility there will be some loss inside the object
		//
		// ps: loss in object is diff from air
		// ps: currently distanceAttenuation only solve the air part
		intensity_result = prod(intensity_result, i.getMaterial().kt);

		// push the ray forward a little bit
		point_light = r.at(i.t) + ray_dir * RAY_EPSILON;

	}

	return intensity_result;
}


vec3f DirectionalLight::getColor( const vec3f& P ) const {
	// Color doesn't depend on P 
	return color;
}


vec3f DirectionalLight::getDirection( const vec3f& P ) const {
	return -orientation;
}


double PointLight::distanceAttenuation( const vec3f& P ) const {
	// YOUR CODE HERE
	// range: 0.0 - 1.0
	// 1 / d ^ 2

	double coeff_1, coeff_2, coeff_3;
	if (traceUI->getIsOverrideAtten()) {
		coeff_1 = traceUI->getAttenConstant();
		coeff_2 = traceUI->getAttenLinear();
		coeff_3 = traceUI->getAttenQuadric();
	} else {
		coeff_1 = attenuation_coeff[0];
		coeff_2 = attenuation_coeff[1];
		coeff_3 = attenuation_coeff[2];
	}

	const double d2 = (P - position).length_squared();
	const double d1 = sqrt(d2);
	const double result = coeff_1 + coeff_2 * d1 + coeff_3 * d2;

	// do not divide by zero !
	return result == 0.0 ? 1.0 : std::min<double>(1 / result, 1.0);
}


vec3f PointLight::getColor( const vec3f& P ) const {
	// Color doesn't depend on P 
	return color;
}


vec3f PointLight::getDirection( const vec3f& P ) const {
	return (position - P).normalize();
}


vec3f PointLight::shadowAttenuation(const vec3f& P) const {
	// YOUR CODE HERE:
	// range: 0.0 - 1.0

	// variable preparation
	const vec3f& ray_dir = getDirection(P); // from P to light source
	vec3f point_light = P;					// the point of ray toward the light source
	vec3f intensity_result(1.0, 1.0, 1.0);

	// first push the ray a little bit forward to prevent hit the same point
	// and cause dead loop
	point_light = point_light + ray_dir * RAY_EPSILON;
	while (!intensity_result.iszero()) {

		// TODO: not yet decided the exact naming
		isect i;
		ray r(point_light, ray_dir);
		const double length_light = (position - point_light).length();

		// check if no intersect or intersection is behind the light source
		if (!scene->intersect(r, i))	return intensity_result;
		if (i.t >= length_light)		return intensity_result;

		// light intensity reduction due to energy loss when passing through the material
		// 
		// Problem:
		// in the actual calculation,
		// that this reduction will only happen when enter or leave the object
		// but the in realility there will be some loss inside the object
		//
		// ps: loss in object is diff from air
		// ps: currently distanceAttenuation only solve the air part
		intensity_result = prod(intensity_result, i.getMaterial().kt);

		// push the ray forward a little bit
		point_light = r.at(i.t) + ray_dir * RAY_EPSILON;

	}

	return intensity_result;
}


void PointLight::setDistanceAttenuationCoeff(const vec3f& coeff) {
	attenuation_coeff = coeff;
}


// TODO: P should be marked as __UNUSED__
vec3f AmbientLight::getColor(const vec3f& P) const {
	return color;
}


// Static Function Implementation
// ...
