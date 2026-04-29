#ifndef SCENE_H_
#define SCENE_H_

#include <memory>

#include "util.h"
#include "bvh.h"

// scene holds scene data, and lets you intersect the scene. Uses BVH internally

struct GPUScene {

	HD bool intersect(const Ray& ray, Hit* out) const;
	void free();

	vec3 cam_pos;
	vec3 sun_dir;

	Sphere* spheres;
	int spheres_len;
	
	uint32_t* sphere_indices;
	int sphere_indices_len;

	BVH::Node* nodes;
	int nodes_len;
};

struct Scene {

public:
	Scene(const char* file_path);

	bool intersect(const Ray& ray, Hit* out) const;
	inline vec3 get_cam_pos() const { return cam_pos; };
	inline vec3 get_sun_dir() const { return sun_dir; };

	GPUScene copy_to_gpu();

private:
	vec3 cam_pos;
	vec3 sun_dir;

	std::vector<Sphere> spheres;
	
	std::unique_ptr<BVH> bvh_ptr;
	BVH* bvh;
};

#endif
