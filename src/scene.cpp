#include "scene.h"

#include <iostream>
#include <fstream>

Scene::Scene(const char* file_path) {

	// read scene file
    std::ifstream istr(file_path);
    std::string buf;
    while (istr >> buf) {
        if (buf == "SUN") {
            double x, y, z;
            istr >> x >> y >> z;
			sun_dir = vec3{ x, y, z }.norm();
        } else if (buf == "SPHERE") {
            double x, y, z;
            double radius;
            double r, g, b;
            istr >> x >> y >> z >> radius >> r >> g >> b;

            vec3 pos = { x, y, z };
            vec3 col = { r, g, b };
            spheres.push_back({ pos, radius, col });
        } else if (buf == "CAMERA") {
            double x, y, z;
            istr >> x >> y >> z;
			cam_pos = vec3{ x, y, z };
		} else {
            std::cerr << "ERROR: invalid code in input file\n";
            abort();
        }
    }

	// construct bvh
	bvh = std::make_unique<BVH>(spheres);
}

bool Scene::intersect(const Ray& ray, Hit* out) const {
	return bvh->intersect(ray, out);
}
