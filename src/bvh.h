#ifndef BVH_H_
#define BVH_H_

#include "obj.h"
#include <numeric>
#include <vector>
#include <algorithm>
#include <optional>
#include <array>

class BVH {
public:
	static constexpr u32 MAX_LEAF = 4; // max spheres per each leaf
	struct Node {
		AABB aabb;
		i32 left; // start here
		i32 right;
		i32 count; // left + count for all leaves
		bool is_leaf() const { return count > 0; }
	};

	BVH(std::vector<Sphere> spheres);
	bool intersect(const Ray& ray, Hit* out) const;
	const std::vector<Sphere>& spheres() const { return spheres_; }
	const std::vector<Node>& nodes() const { return nodes_; }

private:
	std::vector<Sphere> spheres_;
	std::vector<uint32_t> sphere_indices_; // all spheres stored in static array for easier access
	std::vector<AABB> sphere_aabbs_;
	std::vector<vec3> centroids_;
	std::vector<Node> nodes_;

	i32 build(u32 first, u32 count);
};

#endif
