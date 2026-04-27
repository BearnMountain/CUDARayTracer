#ifndef VEC3_H_
#define VEC3_H_

#include <cmath>
#include <cstdint>


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

class Color {
public:
	Color() {}
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
	Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b), a(255) {}
	uint8_t r,g,b,a;
};

inline Color operator+(const Color& a, const Color& b) { return Color(a.r + b.r, a.g + b.g, a.b + b.b, a.a); }
inline Color operator+(const Color& a, double b) { return Color(a.r + b, a.g + b, a.b + b, a.a); }
inline Color operator+(double b, const Color& a) { return Color(a.r + b, a.g + b, a.b + b, a.a); }
inline Color operator*(double b, const Color& a) { return Color(a.r * b, a.g * b, a.b * b, a.a * b); }
inline Color operator*(const Color& a, double b) { return Color(a.r * b, a.g * b, a.b * b, a.a * b); }

class Sphere {
public:
	Sphere(vec3 pos, double radius, Color color) : pos(pos), radius(radius), color(color) {}
    vec3 pos;
    double radius;
	Color color;
};

class Light {
public:
	Light(vec3 pos, Color color) : pos(pos), color(color) {}
	vec3 pos;
	Color color;
};

#endif
