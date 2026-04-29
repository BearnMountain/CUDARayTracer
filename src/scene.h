#ifndef SCENE_H_
#define SCENE_H_

#include <memory>

#include "util.h"
#include "bvh.h"

// scene holds scene data, and lets you intersect the scene. Uses BVH internally

struct Scene {

public:
	Scene(const char* file_path);

	HD bool intersect(const Ray& ray, Hit* out) const;
	HD inline vec3 get_cam_pos() const { return cam_pos; };
	HD inline vec3 get_sun_dir() const { return sun_dir; };

private:
	vec3 cam_pos;
	vec3 sun_dir;

	std::vector<Sphere> spheres;
	
	std::unique_ptr<BVH> bvh_ptr;
	BVH* bvh;
};

#endif
