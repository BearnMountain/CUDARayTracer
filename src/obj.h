#ifndef VEC3_H_
#define VEC3_H_

#include <cmath>
#include <cstdint>

// VEC
class vec3 {
public:
	vec3() : x(0), y(0), z(0) {} 
	vec3(double x, double y, double z) : x(x), y(y), z(z) {}
	double x,y,z;

	vec3& operator +=(const vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
	vec3& operator -=(const vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	vec3& operator *=(double v) { x *= v; y *= v; z *= v; return *this; }
	vec3& operator /=(double v) { return *this *= 1/v; }
};

inline vec3 operator+(const vec3& u, const vec3& v) { return vec3(u.x + v.x, u.y + v.y, u.z + v.z); }
inline vec3 operator-(const vec3& u, const vec3& v) { return vec3(u.x - v.x, u.y - v.y, u.z - v.z); }
inline vec3 operator*(const vec3& u, const vec3& v) { return vec3(u.x * v.x, u.y * v.y, u.z * v.z); }
inline vec3 operator*(double t, const vec3& v) { return vec3(t * v.x, t * v.y, t * v.z); }
inline vec3 operator*(const vec3& v, double t) { return t * v; }
inline vec3 operator/(const vec3& v, double t) { return (1/t) * v; }
inline double dot(const vec3& u, const vec3& v) { return u.x * v.x + u.y * v.y + u.z * v.z; }
inline vec3 cross(const vec3& u, const vec3& v) { return vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x); }
inline double length(const vec3& v) { return std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z); }
inline vec3 unit_vector(const vec3& v) { return v / length(v); }

// COLOR
class Color {
public:
	Color() {}
	Color(double r, double g, double b) : r(r), g(g), b(b) {}
	double r,g,b;
};

inline Color operator+(const Color& a, const Color& b) { return Color(a.r + b.r, a.g + b.g, a.b + b.b); }
inline Color operator+(const Color& a, double b) { return Color(a.r + b, a.g + b, a.b + b); }
inline Color operator+(double b, const Color& a) { return Color(a.r + b, a.g + b, a.b + b); }
inline Color operator*(const Color& a, const Color& b) { return Color(a.r * b.r, a.g * b.g, a.b * b.b); }
inline Color operator*(double b, const Color& a) { return Color(a.r * b, a.g * b, a.b * b); }
inline Color operator*(const Color& a, double b) { return Color(a.r * b, a.g * b, a.b * b); }

// SPHERE
class Sphere {
public:
	Sphere(vec3 pos, double radius, Color color) : pos(pos), radius(radius), color(color) {}
    vec3 pos;
    double radius;
	Color color;
};

// LIGHT
class Light {
public:
	Light(vec3 pos, Color color) : pos(pos), color(color) {}
	vec3 pos;
	Color color;
};

// RAY
class ray {
public:
	ray() {}
	ray(const vec3& origin, const vec3& direction) : origin(origin), direction(direction) {}

	const vec3& getOrigin() const { return origin; }
	const vec3& getDirection() const { return direction; }

	vec3 at(double t) const { return t * direction + origin; }

private:
	vec3 origin;
	vec3 direction;
};



#endif
