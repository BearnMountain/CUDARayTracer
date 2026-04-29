#include "bvh.h"

const double T_MIN = 1e-4;
const double T_MAX = std::numeric_limits<double>::max();

BVH::BVH(std::vector<Sphere> spheres) {
	if (spheres.empty()) return;
	spheres_ = std::move(spheres);
	sphere_indices_.resize(spheres_.size());
	for (u32 i = 0; i < spheres_.size(); i++)  sphere_indices_[i] = i;

	// precompute aabb and centroids(what box each child will split into)
	sphere_aabbs_.resize(spheres_.size());
	centroids_.resize(spheres_.size());
	for (u32 i = 0; i < spheres_.size(); i++) {
		sphere_aabbs_[i].expand(spheres_[i]);
		centroids_[i] = sphere_aabbs_[i].center();
	}

	nodes_.reserve(2 * spheres_.size());
	build(0, static_cast<u32>(spheres_.size()));
}

bool BVH::intersect(const Ray& ray, Hit* out) const {

	double t_min = T_MIN;
	double t_max = T_MAX;

	if (nodes_.empty()) return false;

	Hit best;
	bool found = false;

	// Iterative traversal with an explicit stack
	std::array<int32_t, 64> stack;
	int stack_top = 0;
	stack[stack_top++] = 0; // root

	while (stack_top > 0) {
		const Node& node = nodes_[stack[--stack_top]];
		if (!node.aabb.intersect(ray, t_min, t_max)) continue;

		if (node.is_leaf()) {
			for (int i = 0; i < node.count; ++i) {
				uint32_t si = sphere_indices_[node.left + i];

				// check if it intersects sphere

				const Sphere& s = spheres_[si];
				vec3   oc = ray.origin - s.pos;
				double a  = ray.dir.dot(ray.dir);
				double hb = oc.dot(ray.dir);         // half-b
				double c  = oc.dot(oc) - s.radius * s.radius;
				double disc = hb*hb - a*c;
				if (disc < 0.0) continue;

				double sqrt_disc = std::sqrt(disc);
				double t = (-hb - sqrt_disc) / a;
				if (t < t_min || t > t_max) {
					t = (-hb + sqrt_disc) / a;
					if (t < t_min || t > t_max) continue;
				}

				// construct hit
				
				vec3 point  = ray.at(t);
				vec3 normal = (point - s.pos) / s.radius; // unit outward normal
				best = {point, normal, s.color, t};
				found = true;

				// shrink window — keeps only closest

				t_max = t;
			}
		} else {

			// Push farther child first so nearer child is processed first
			int32_t left  = node.left;
			int32_t right = node.right;

			double tl = t_max, tr = t_max;
			bool hl = nodes_[left ].aabb.intersect(ray, t_min, tl);
			bool hr = nodes_[right].aabb.intersect(ray, t_min, tr);

			if (hl && hr) {
				// Push farther first
				if (tl < tr) { stack[stack_top++] = right; stack[stack_top++] = left; }
				else         { stack[stack_top++] = left;  stack[stack_top++] = right; }
			} else if (hl) { stack[stack_top++] = left; }
			  else if (hr) { stack[stack_top++] = right; }
		}
	}

	*out = best;
	return found;
}

i32 BVH::build(u32 first, u32 count) {
	// compute bounds of all spheres
	AABB bounds;
	for (u32 i = first; i < first + count; i++) {
		bounds.expand(sphere_aabbs_[sphere_indices_[i]]);
	}

	i32 node_idx = static_cast<i32>(nodes_.size());
	nodes_.push_back({});
	nodes_[node_idx].aabb = bounds;

	if (count <= MAX_LEAF) {
		// Leaf node
		nodes_[node_idx].left  = first;
		nodes_[node_idx].count = count;
		return node_idx;
	}

	// ── SAH binned split ────────────────────────────────────────────
	// Compute bounds of centroids to choose the split axis
	AABB centroid_bounds;
	for (int i = first; i < first + count; ++i) {
		vec3 c = centroids_[sphere_indices_[i]];
		centroid_bounds.min = vec3::min(centroid_bounds.min, c);
		centroid_bounds.max = vec3::max(centroid_bounds.max, c);
	}

	constexpr int NUM_BINS = 12;
	double best_cost = std::numeric_limits<double>::max();
	int best_axis = 0;
	int best_bin  = NUM_BINS / 2;

	for (int axis = 0; axis < 3; ++axis) {
		double lo = centroid_bounds.min[axis];
		double hi = centroid_bounds.max[axis];
		if (hi - lo < 1e-12) continue; // degenerate axis

		struct Bin { AABB aabb; int count = 0; };
		std::array<Bin, NUM_BINS> bins{};

		double inv_extent = NUM_BINS / (hi - lo);
		for (int i = first; i < first + count; ++i) {
			uint32_t si = sphere_indices_[i];
			int b = static_cast<int>((centroids_[si][axis] - lo) * inv_extent);
			b = CLAMP(b, 0, NUM_BINS - 1);
			bins[b].aabb.expand(sphere_aabbs_[si]);
			bins[b].count++;
		}

		// Sweep left-to-right and right-to-left to compute SAH for each split
		std::array<double, NUM_BINS - 1> cost{};
		AABB left_aabb;  int left_count  = 0;
		AABB right_aabb; int right_count = 0;
		std::array<AABB, NUM_BINS> prefix({}), suffix({});
		std::array<int,  NUM_BINS> pcnt{}, scnt{};

		for (int k = 0; k < NUM_BINS; ++k) {
			if (k == 0) { prefix[0] = bins[0].aabb; pcnt[0] = bins[0].count; }
			else { prefix[k] = prefix[k-1]; prefix[k].expand(bins[k].aabb); pcnt[k] = pcnt[k-1] + bins[k].count; }
		}
		for (int k = NUM_BINS-1; k >= 0; --k) {
			if (k == NUM_BINS-1) { suffix[k] = bins[k].aabb; scnt[k] = bins[k].count; }
			else { suffix[k] = suffix[k+1]; suffix[k].expand(bins[k].aabb); scnt[k] = scnt[k+1] + bins[k].count; }
		}

		for (int k = 0; k < NUM_BINS - 1; ++k) {
			double sa_l = pcnt[k] ? prefix[k].surface_area()   : 0.0;
			double sa_r = scnt[k+1] ? suffix[k+1].surface_area() : 0.0;
			cost[k] = 0.125 + (pcnt[k] * sa_l + scnt[k+1] * sa_r) / bounds.surface_area();
			if (cost[k] < best_cost) {
				best_cost = cost[k];
				best_axis = axis;
				best_bin  = k;
			}
		}
	}

	// Fallback: if SAH can't split, make a leaf (cost of leaf = count)
	if (best_cost >= static_cast<double>(count)) {
		nodes_[node_idx].left  = first;
		nodes_[node_idx].count = count;
		return node_idx;
	}

	// Partition sphere_indices_ around best split
	double lo = centroid_bounds.min[best_axis];
	double hi = centroid_bounds.max[best_axis];
	double inv_ext = NUM_BINS / (hi - lo + 1e-12);
	auto mid_it = std::partition(
		sphere_indices_.begin() + first,
		sphere_indices_.begin() + first + count,
		[&](uint32_t si) {
			int b = static_cast<int>((centroids_[si][best_axis] - lo) * inv_ext);
			return CLAMP(b, 0, NUM_BINS - 1) <= best_bin;
	});
	int mid = static_cast<int>(mid_it - sphere_indices_.begin());
	if (mid == first || mid == first + count) mid = first + count / 2;

	// build the children
	int32_t left_child  = build(first, mid - first);
	int32_t right_child = build(mid, first + count - mid);

	nodes_[node_idx].left  = left_child;
	nodes_[node_idx].right = right_child;
	nodes_[node_idx].count = 0; // marks as inner

	return node_idx;
}
