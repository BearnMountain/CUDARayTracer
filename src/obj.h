#ifndef VEC3_H_
#define VEC3_H_

#include <cmath>
#include <cstdint>
#include <algorithm>

// todo(jqj): use of these is inconsistent
typedef uint32_t u32;
typedef int32_t i32;

#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

struct vec3 {
    double x = 0, y = 0, z = 0;

	// matrix algebra
    vec3 operator+(vec3 a) const { return {x+a.x, y+a.y, z+a.z}; }
    vec3 operator-(vec3 a) const { return {x-a.x, y-a.y, z-a.z}; }
    vec3 operator*(double t) const { return {x*t, y*t, z*t}; }
	vec3 operator*(vec3 a) const { return {x*a.x, y*a.y, z*a.z}; }
    vec3 operator/(double t) const { return {x/t, y/t, z/t}; }

    double dot(vec3 a) const { return x*a.x + y*a.y + z*a.z; }
    double length(void) const { return std::sqrt(dot(*this)); }
    vec3 norm(void) const { return *this / length(); }

    double operator[](int i) const { return (&x)[i]; }
    double& operator[](int i) { return (&x)[i]; }

    static vec3 min(vec3 a, vec3 b) { return (vec3){ std::min(a.x,b.x), std::min(a.y,b.y), std::min(a.z,b.z)}; }
    static vec3 max(vec3 a, vec3 b) { return (vec3){ std::max(a.x,b.x), std::max(a.y,b.y), std::max(a.z,b.z)}; }
};

struct Sphere {
    vec3   pos;
    double radius;
    vec3   color;
};

typedef struct Ray {
    vec3 origin, dir; 
    vec3 inverse_dir; // precompute for AABB (b-o) * inverse_dir

    Ray(vec3 origin, vec3 dir) : origin(origin), dir(dir.norm()), 
		inverse_dir{
			1.0/dir.norm().x, 
			1.0/dir.norm().y, 
			1.0/dir.norm().z
		}
    {}

    vec3 at(double t) const { return origin + dir * t; }
} Ray;

struct AABB {
    vec3 min{ 
		std::numeric_limits<double>::max(),
		std::numeric_limits<double>::max(),
		std::numeric_limits<double>::max() 
	};

    vec3 max{ 
		std::numeric_limits<double>::lowest(),
		std::numeric_limits<double>::lowest(),
		std::numeric_limits<double>::lowest() 
	};

    // only spheres, grow to enclose them
    void expand(const Sphere& s) { vec3 r{s.radius, s.radius, s.radius}; min = vec3::min(min, s.pos - r); max = vec3::max(max, s.pos + r); }
    void expand(const AABB& o) { min = vec3::min(min, o.min); max = vec3::max(max, o.max); }

    vec3 center() const { return (min + max) / 2.0; }
    vec3 extent() const { return max - min; }
    double surface_area() const {
        vec3 e = extent();
        return 2.0 * (e.x*e.y + e.y*e.z + e.z*e.x);
    }
    int longest_axis() const {
        vec3 e = extent();
        if (e.x >= e.y && e.x >= e.z) return 0;
        if (e.y >= e.z) return 1;
        return 2;
    }

    // returns true if ray hits in [t_min, t_max]
	// SLAB TEST: https://tavianator.com/2022/ray_box_boundary.html
    bool intersect(const Ray& ray, double t_min, double t_max) const {
        for (int i = 0; i < 3; ++i) {
            double inv = ray.inverse_dir[i];
            double t0  = (min[i] - ray.origin[i]) * inv;
            double t1  = (max[i] - ray.origin[i]) * inv;
            if (inv < 0.0) std::swap(t0, t1);
            t_min = std::max(t_min, t0);
            t_max = std::min(t_max, t1);
            if (t_max <= t_min) return false;
        }
        return true;
    }
};

typedef struct {
    double t; // ray parameter
    vec3 point; // hit position
    vec3 normal;
    vec3 color;
    uint32_t sphere_idx;
} Hit;

#endif
