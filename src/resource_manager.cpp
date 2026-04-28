#include "resource_manager.h"
#include "stb_image_write.h"
#include <stdint.h>
#include <stdio.h>
#include "obj.h"
#include <algorithm>

void parse_scene_file(const char* path, Sphere* spheres, int* sphere_size, vec3* lights, int* light_size) {
	// file format:
	// LIGHT x y z r g b
	FILE* file = fopen(path, "r");

	fclose(file);
}


void write_scene_file(std::vector<vec3>& pixels, int width, int height) {
    uint8_t* rgb = new uint8_t[width * height * 3];

    for (int i = 0; i < width * height; i++) {
        rgb[i * 3 + 0] = (uint8_t)(std::clamp<double>(pixels[i].x, 0.0f, 1.0f) * 255.0f);
        rgb[i * 3 + 1] = (uint8_t)(std::clamp<double>(pixels[i].y, 0.0f, 1.0f) * 255.0f);
        rgb[i * 3 + 2] = (uint8_t)(std::clamp<double>(pixels[i].z, 0.0f, 1.0f) * 255.0f);
    }

    stbi_write_png(
        "scene.png",
        width,
        height,
        3,
        rgb,
        width * 3
    );

    delete[] rgb;
}
