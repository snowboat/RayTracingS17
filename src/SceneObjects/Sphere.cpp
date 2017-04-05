#include <cmath>

#include "Sphere.h"




bool Sphere::intersectLocal( const ray& r, isect& i ) const
{
	vec3f v = -r.getPosition();
	double b = v.dot(r.getDirection());
	double discriminant = b*b - v.dot(v) + 1;

	if( discriminant < 0.0 ) {
		return false;
	}

	discriminant = sqrt( discriminant );
	double t2 = b + discriminant;

	if( t2 <= RAY_EPSILON ) {
		return false;
	}

	i.obj = this;

	double t1 = b - discriminant;

	if( t1 > RAY_EPSILON ) {
		i.t = t1;
		i.N = r.at( t1 ).normalize();
	} else {
		i.t = t2;
		//i.N = r.at(t2).normalize();
		i.N = -r.at( t2 ).normalize();
	}

	return true;
}

bool Sphere::getLocalUV(const ray & r, const isect & i, double & u, double & v) const
{
	
	// Transform the ray into the object's local coordinate space
	vec3f pos = transform->globalToLocalCoords(r.getPosition());
	vec3f dir = transform->globalToLocalCoords(r.getPosition() + r.getDirection()) - pos;
	double length = dir.length();
	dir /= length;

	ray localRay(pos, dir);
	isect icopy = i;
	if (intersectLocal(localRay, icopy)) {	//send this iscet point to the intersect local function.
										// Transform the intersection point & normal returned back into global space.
		
										
		vec3f sn = localRay.at(icopy.t);
		vec3f sp = { 0.0f, 0.0f, 1.0f };
		vec3f se = { -1.0f, 0.0f, 0.0f };
		double fai = acos(-sn*sp);
		v = fai / 3.14159265359;
		if (abs(v) < RAY_EPSILON || abs(v - 1) < RAY_EPSILON)
			u = 0;
		else {
			cout << "division" << (se*sn) / sin(fai) << endl;
			cout << "min " << min(1.0, (se*sn) / sin(fai)) << endl;
			double mini = min( 1.0, (se*sn) / sin(fai) );
			if (mini > 1.0)
				mini = 1.0;
			if (mini < -1.0)
				mini = -1.0;
				cout << "acos " << acos(  mini      ) << endl;
			//double theta = acos(min(1.0, (se*sn) / sin(fai))) / (2 * 3.14159265359);	//is the problem due to floating point > 1?
			double theta = acos(mini) / (2 * 3.14159265359);	//is the problem due to floating point > 1?
			cout << "theta" << theta << endl;
			if ((sp.cross(se))  *sn > RAY_EPSILON)
				u = theta;
			else
				u = 1 - theta;
		}
		cout << "uv" << u << "  " << v << endl;
		
		
										
		//i.N = transform->localToGlobalCoordsNormal(i.N);
		//i.t /= length;

		return true;
	}
	else {
		return false;
	}
	
}


