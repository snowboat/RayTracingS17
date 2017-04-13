#include <cmath>

#include "Hyperboloid.h"


bool Hyperboloid::intersectLocal(const ray & r, isect & i) const
{


	double A = r.getDirection()[0] * r.getDirection()[0] + r.getDirection()[1] * r.getDirection()[1] - r.getDirection()[2] * r.getDirection()[2];
	double B = 2 * (r.getDirection()[0] * r.getPosition()[0] + r.getDirection()[1] * r.getPosition()[1] - r.getDirection()[2] * r.getPosition()[2]);
	double C = r.getPosition()[0] * r.getPosition()[0] + r.getPosition()[1] * r.getPosition()[1] - r.getPosition()[2] * r.getPosition()[2] - 1;
	double discriminant = B*B - 4 * A*C;


	if (discriminant < 0.0) {
		return false;
	}

	discriminant = sqrt(discriminant);
	double t2 = (-B + discriminant) / (2*A);	//a=1, since direction is normalized

	if (t2 <= RAY_EPSILON) {	//larger root
		return false;
	}

	i.obj = this;

	double t1 = (-B - discriminant) / (2 * A);	//a=1, since direction is normalized

	if (t1 > RAY_EPSILON) {
		i.t = t1;
		i.N = vec3f(2 * r.at(t1)[0], 2 * r.at(t1)[1], 1).normalize();
	}
	else {
		i.t = t2;
		i.N = vec3f(2 * r.at(t2)[0], 2 * r.at(t2)[1], 1).normalize();
	}

	//flip normal if necessary
	if (r.getDirection().dot(i.N) > 0)
		i.N *= -1.0;


	return true;
}

bool Hyperboloid::getLocalUV(const ray & r, const isect & i, double & u, double & v) const
{
	return false;
}

bool Hyperboloid::getLocalPertubation(const ray & r, isect & i, double & u, double & v)
{
	return false;
}
