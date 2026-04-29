#ifndef UTIL_H_
#define UTIL_H_

#include <cmath>
#include <cstdint>

// todo(jqj): use of these is inconsistent
typedef uint32_t u32;
typedef int32_t i32;

#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define HD __host__ __device__

struct vec3 {
    double x = 0, y = 0, z = 0;

	// matrix algebra
	HD vec3 operator+(vec3 a) const { return {x+a.x, y+a.y, z+a.z}; }
	HD vec3 operator-(vec3 a) const { return {x-a.x, y-a.y, z-a.z}; }
	HD vec3 operator*(double t) const { return {x*t, y*t, z*t}; }
	HD vec3 operator*(vec3 a) const { return {x*a.x, y*a.y, z*a.z}; }
	HD vec3 operator/(double t) const { return {x/t, y/t, z/t}; }

    HD double dot(vec3 a) const { return x*a.x + y*a.y + z*a.z; }
    HD double length(void) const { return sqrt(dot(*this)); }
    HD vec3 norm(void) const { return *this / length(); }

    HD double operator[](int i) const { return (&x)[i]; }
    HD double& operator[](int i) { return (&x)[i]; }

    HD static vec3 min(vec3 a, vec3 b) { return (vec3){ MIN(a.x,b.x), MIN(a.y,b.y), MIN(a.z,b.z)}; }
    HD static vec3 max(vec3 a, vec3 b) { return (vec3){ MAX(a.x,b.x), MAX(a.y,b.y), MAX(a.z,b.z)}; }
};

struct Hit {
    vec3 point;
    vec3 normal;
    vec3 color;
    double t; // ray parameter
};

struct Ray {
    vec3 origin, dir; 
    vec3 inverse_dir; // precompute for AABB (b-o) * inverse_dir

    HD Ray(vec3 origin, vec3 dir) : origin(origin), dir(dir.norm()), 
		inverse_dir{
			1.0/dir.norm().x, 
			1.0/dir.norm().y, 
			1.0/dir.norm().z
		}
    {}

    HD vec3 at(double t) const { return origin + dir * t; }
};

struct Sphere {
    vec3   pos;
    double radius;
    vec3   color;
};

#endif
