#include <cmath>

#include "Square.h"

bool Square::intersectLocal( const ray& r, isect& i ) const
{
	vec3f p = r.getPosition();
	vec3f d = r.getDirection();

	if( d[2] == 0.0 ) {
		return false;
	}

	double t = -p[2]/d[2];

	if( t <= RAY_EPSILON ) {
		return false;
	}

	vec3f P = r.at( t );

	if( P[0] < -0.5 || P[0] > 0.5 ) {	
		return false;
	}

	if( P[1] < -0.5 || P[1] > 0.5 ) {	
		return false;
	}

	i.obj = this;
	i.t = t;
	if( d[2] > 0.0 ) {
		i.N = vec3f( 0.0, 0.0, -1.0 );
	} else {
		i.N = vec3f( 0.0, 0.0, 1.0 );
	}

	return true;
}

bool Square::getLocalUV(const ray & r, const isect & i, double & u, double & v) const
{

	// Transform the ray into the object's local coordinate space
	vec3f pos = transform->globalToLocalCoords(r.getPosition());
	vec3f dir = transform->globalToLocalCoords(r.getPosition() + r.getDirection()) - pos;
	double length = dir.length();
	dir /= length;

	ray localRay(pos, dir);
	isect icopy = i;
	if (intersectLocal(localRay, icopy)) {
		vec3f localIscePoint = localRay.at(icopy.t);
		u = localIscePoint[0] + 0.5;
		v = localIscePoint[1] + 0.5;
		return true;
	}
	return false;

}



bool Square::preturbNormal(const ray & r, isect & i, const double & u, const double & v, unsigned char * preturbImg, const int & imgWidth, const int & imgHeight, Scene* scene) const
{
	// Transform the ray into the object's local coordinate space
	vec3f pos = transform->globalToLocalCoords(r.getPosition());
	vec3f dir = transform->globalToLocalCoords(r.getPosition() + r.getDirection()) - pos;
	double length = dir.length();
	dir /= length;

	ray localRay(pos, dir);
	if (preturbImg) {
		int pixelx = min(imgWidth - 1, int(u*double(imgWidth)));
		int pixely = min(imgHeight - 1, int(v*double(imgHeight)));


		int x = pixelx;
		int y = pixely;
		if (x >= 1 && x <= imgWidth - 1 && y >= 1 && y <= imgWidth) {
			double Gx = scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, x - 1, y + 1) * -1 +
				scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, x - 1, y) * -2 +
				scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, x - 1, y - 1) * -1 +
				scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, x + 1, y + 1) * 1 +
				scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, x - 1, y) * 2 +
				scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, x - 1, y - 1) * 1;

			double Gy = scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, x - 1, y + 1) * -1 +
				scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, x , y+1) * -2 +
				scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, x + 1, y + 1) * -1 +
				scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, x - 1, y - 1) * 1 +
				scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, x, y-1) * 2 +
				scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, x + 1, y - 1) * 1;
			vec3f Pu(0.0, 1.0, 0.0);
			vec3f Pv(1.0, 0.0, 0.0);
			Gx *= 32.0;
			Gy *= 32.0;
			if (sqrt(Gx*Gx + Gy*Gy) > 0.9) {
				i.N = i.N + Pv*Gx + Pu * Gy;
			}
			i.N = i.N.normalize();
			
		}
		return true;



		//double intensity = scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, pixelx, pixely);
		//double intensityBelow = scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, pixelx, min(pixely + 1, imgHeight - 1));
		//double intensityRight = scene->getPixelIntensity(preturbImg, imgWidth, imgHeight, max(pixelx - 1, 0), pixely);
		//double Bv = intensity - intensityBelow;
		//double Bu = intensity - intensityRight;
		//Bv *= 64.0;
		//Bu *= 64.0;
		//vec3f Pu(0.0, 1.0, 0.0);
		//vec3f Pv(1.0, 0.0, 0.0);
		//cout << "my intensity " << intensity << " right " << intensityRight << "down " << intensityBelow << endl;
		//if (Bv > 0.05 && Bu > 0.05)
		//	i.N = i.N + Pu * Bv + Pv*Bu;
		//i.N = i.N.normalize();
		//cout << "i.n updated to " << i.N << endl;
		//return true;
	}
	return false;
}

