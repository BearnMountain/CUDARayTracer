#include "vec3.h"

vec3& vec3::operator +=(const vec3& v) {
	x_ += v.x_;
	y_ += v.y_;
	z_ += v.z_;
	return *this;
}

vec3& vec3::operator -=(const vec3& v) {
	x_ -= v.x_;
	y_ -= v.y_;
	z_ -= v.z_;
	return *this;
}

vec3& vec3::operator *=(double v) {
	x_ *= v;
	y_ *= v;
	z_ *= v;
	return *this;
}

vec3& vec3::operator /=(double v) {
	return *this *= 1/v;
}
