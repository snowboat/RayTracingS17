#include <cmath>
#include <assert.h>

#include "Box.h"

bool Box::intersectLocal(const ray& r, isect& i) const
{
	// YOUR CODE HERE:
	// Add box intersection code here.
	// it currently ignores all boxes and just returns false.
	vec3f p = r.getPosition();
	vec3f d = r.getDirection();

	double tmin = -INFINITY, tmax = INFINITY;
	int plane = -1;

	if (d[0] != 0.0) {
		double tx1 = (-0.5 - p[0]) / d[0];
		double tx2 = (0.5 - p[0]) / d[0];

		tmin = min(tx1, tx2);
		tmax = max(tx1, tx2);

		plane = 0;
	}

	if (d[1] != 0.0) {
		double ty1 = (-0.5 - p[1]) / d[1];
		double ty2 = (0.5 - p[1]) / d[1];

		if (min(ty1, ty2) > tmin) {
			tmin = min(ty1, ty2);
			plane = 1;
		}
		tmax = min(tmax, max(ty1, ty2));
	}

	if (d[2] != 0.0) {
		double tz1 = (-0.5 - p[2]) / d[2];
		double tz2 = (0.5 - p[2]) / d[2];

		if (min(tz1, tz2) > tmin) {
			tmin = min(tz1, tz2);
			plane = 2;
		}
		tmax = min(tmax, max(tz1, tz2));
	}

	if ( tmax >= tmin && tmin > RAY_EPSILON ) {
		i.obj = this;
		i.t = tmin;
		if (plane == 0) {
			if (d[0] > 0.0) {
				i.N = vec3f(-1.0, 0.0, 0.0);
			}
			else {
				i.N = vec3f(1.0, 0.0, 0.0);
			}
		}
		if (plane == 1) {
			if (d[1] > 0.0) {
				i.N = vec3f(0.0, -1.0, 0.0);
			}
			else {
				i.N = vec3f(0.0, 1.0, 0.0);
			}
		}
		if (plane == 2) {
			if (d[2] > 0.0) {
				i.N = vec3f(0.0, 0.0, -1.0);
			}
			else {
				i.N = vec3f(0.0, 0.0, 1.0);
			}
		}
		return true;
	}

	return false;
}
