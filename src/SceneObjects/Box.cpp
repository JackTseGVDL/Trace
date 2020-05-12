#include <cmath>
#include <assert.h>
#include <algorithm>
#include <limits>
#include "Box.h"


// TODO: need prettier
bool Box::intersectLocal( const ray& r, isect& i ) const {
	// double tfar = std::numeric_limits<double>::max();
	// double tnear = -std::numeric_limits<double>::max();
	double tfar = DBL_MAX;
	double tnear = -DBL_MAX;
	int tnear_axis = 0;

	int index;
	for (index = 0; index < 3; index++) {

		// check if parallel to plane
		if (r.getDirection()[index] == 0) {
			if (r.getPosition()[index] < -0.5 || r.getPosition()[index] > 0.5) return false;
			continue;
		}

		double t1 = (-0.5 - r.getPosition()[index]) / r.getDirection()[index];
		double t2 = (0.5 - r.getPosition()[index]) / r.getDirection()[index];

		if (t1 > t2) {
			std::swap(t1, t2);
		}
		if (t1 > tnear) {
			tnear = t1;
			tnear_axis = index;
		}
		if (t2 < tfar) {
			tfar = t2;
		}
		if (tnear > tfar || tfar <= RAY_EPSILON) return false;
	}

	i.obj = this;
	i.t = tnear;

	// direction of normal is reverse of the direction of ray
	switch (tnear_axis) {
	default:
	case 0:
		i.N = vec3f(((r.getDirection()[0] < 0.0) ? 1.0 : -1.0), 0.0, 0.0);
		break;

	case 1:
		i.N = vec3f(0.0, ((r.getDirection()[1] < 0.0) ? 1.0 : -1.0), 0.0);
		break;

	case 2:
		i.N = vec3f(0.0, 0.0, ((r.getDirection()[2] < 0.0) ? 1.0 : -1.0));
		break;
	}
	
	return true;
}
