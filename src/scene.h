#ifndef SCENE_H_
#define SCENE_H_

#include <memory>

#include "util.h"
#include "bvh.h"

// scene holds scene data, and lets you intersect the scene. Uses BVH internally

struct Scene {

public:
	Scene(const char* file_path);

	bool intersect(const Ray& ray, Hit* out) const;
	inline vec3 get_cam_pos() const { return cam_pos; };
	inline vec3 get_sun_dir() const { return sun_dir; };

private:
	vec3 cam_pos;
	vec3 sun_dir;
	
	std::unique_ptr<BVH> bvh;
	std::vector<Sphere> spheres;
};

#endif
