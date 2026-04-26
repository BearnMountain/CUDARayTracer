#ifndef VEC3_H_
#define VEC3_H_

#include <cmath>


class vec3 {
public:
	vec3() : x_(0), y_(0), z_(0) {}
	vec3(double x, double y, double z) : x_(x), y_(y), z_(z) {}

	double x() const { return x_; }
	double y() const { return y_; }
	double z() const { return z_; }

	vec3& operator +=(const vec3& v);
	vec3& operator -=(const vec3& v);
	vec3& operator *=(double v);
	vec3& operator /=(double v);

	double length() const { return std::sqrt(x_*x_ + y_*y_ + z_*z_); } 
private:
	double x_,y_,z_;
};

inline vec3 operator+(const vec3& u, const vec3& v) { return vec3(u.x() + v.x(), u.y() + v.y(), u.z() + v.z()); }
inline vec3 operator-(const vec3& u, const vec3& v) { return vec3(u.x() - v.x(), u.y() - v.y(), u.z() - v.z()); }
inline vec3 operator*(const vec3& u, const vec3& v) { return vec3(u.x() * v.x(), u.y() * v.y(), u.z() * v.z()); }
inline vec3 operator*(double t, const vec3& v) { return vec3(t * v.x(), t * v.y(), t * v.z()); }
inline vec3 operator*(const vec3& v, double t) { return t * v; }
inline vec3 operator/(const vec3& v, double t) { return (1/t) * v; }
inline double dot(const vec3& u, const vec3& v) { return u.x() * v.x() + u.y() * v.y() + u.z() * v.z(); }
inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(
        u.y() * v.z() - u.z() * v.y(),
        u.z() * v.x() - u.x() * v.z(),
        u.x() * v.y() - u.y() * v.x()
    );
}
inline vec3 unit_vector(const vec3& v) { return v / v.length(); }

#endif
