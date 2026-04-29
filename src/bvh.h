#ifndef BVH_H_
#define BVH_H_

#include <numeric>
#include <vector>
#include <algorithm>
#include <optional>
#include <array>

#include "util.h"

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
    HD bool intersect(const Ray& ray, double t_min, double t_max) const {
        for (int i = 0; i < 3; ++i) {
            double inv = ray.inverse_dir[i];
            double t0  = (min[i] - ray.origin[i]) * inv;
            double t1  = (max[i] - ray.origin[i]) * inv;
            if (inv < 0.0) {
				double temp = t1;
				t1 = t0;
				t0 = temp;
			}
            t_min = MAX(t_min, t0);
            t_max = MIN(t_max, t1);
            if (t_max <= t_min) return false;
        }
        return true;
    }
};

class BVH {
public:
	static constexpr u32 MAX_LEAF = 4; // max spheres per each leaf
	struct Node {
		AABB aabb;
		i32 left; // start here
		i32 right;
		i32 count; // left + count for all leaves
		HD bool is_leaf() const { return count > 0; }
	};

	BVH(std::vector<Sphere> spheres);
	HD bool intersect(const Ray& ray, Hit* out) const;

private:
	// there are "c array" variants of the top three vectors so cuda can access them

	std::vector<Sphere> spheres_;
	Sphere* spheres;
	int spheres_len;

	std::vector<uint32_t> sphere_indices_; // all spheres stored in static array for easier access
	uint32_t* sphere_indices;
	int sphere_indices_len;

	std::vector<Node> nodes_;
	Node* nodes;
	int nodes_len;

	std::vector<AABB> sphere_aabbs_;
	std::vector<vec3> centroids_;

	i32 build(u32 first, u32 count);
};

#endif
