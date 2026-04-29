#include "scene.h"

#include <iostream>
#include <fstream>

#include <cuda_runtime.h>

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
	bvh_ptr = std::make_unique<BVH>(spheres);
	bvh = bvh_ptr.get();
}

bool Scene::intersect(const Ray& ray, Hit* out) const {
	return bvh->intersect(ray, out);
}

GPUScene Scene::copy_to_gpu() {
	GPUScene out;
	
	// copy scene properties
	out.cam_pos = cam_pos;
	out.sun_dir = sun_dir;

	// copy spheres
	out.spheres_len = bvh->spheres_.size();
	size_t spheres_size = out.spheres_len * sizeof(Sphere);
	cudaError_t err = cudaMallocManaged(&out.spheres, spheres_size);
	if (err != cudaSuccess) {
		fprintf(stderr, "Failed to allocate managed memory: %s\n", cudaGetErrorString(err));
		abort();
	}
	memcpy(out.spheres, bvh->spheres_.data(), spheres_size);

	// copy sphere indices
	out.sphere_indices_len = bvh->sphere_indices_.size();
	size_t sphere_indices_size = out.sphere_indices_len * sizeof(uint32_t);
	err = cudaMallocManaged(&out.sphere_indices, sphere_indices_size);
	if (err != cudaSuccess) {
		fprintf(stderr, "Failed to allocate managed memory: %s\n", cudaGetErrorString(err));
		abort();
	}
	memcpy(out.sphere_indices, bvh->sphere_indices_.data(), sphere_indices_size);

	// copy nodes
	out.nodes_len = bvh->nodes_.size();
	size_t nodes_size = out.nodes_len * sizeof(BVH::Node);
	err = cudaMallocManaged(&out.nodes, nodes_size);
	if (err != cudaSuccess) {
		fprintf(stderr, "Failed to allocate managed memory: %s\n", cudaGetErrorString(err));
		abort();
	}
	memcpy(out.nodes, bvh->nodes_.data(), nodes_size);

	return out;
}

void GPUScene::free() {
	cudaFree(spheres);
	cudaFree(sphere_indices);
	cudaFree(nodes);
}

HD bool GPUScene::intersect(const Ray& ray, Hit* out) const {
	return intersect_bvh(ray, out, spheres, spheres_len, sphere_indices, sphere_indices_len, nodes, nodes_len);
}
