#include "resource_manager.h"
#include "stb_image_write.h"
#include <stdint.h>
#include <stdio.h>

void parse_scene_file(const char* path, Sphere* spheres, int* sphere_size, Light* lights, int* light_size) {
	// file format:
	// LIGHT x y z r g b
	FILE* file = fopen(path, "r");

	fclose(file);
}


void write_scene_file(Color* pixels, int width, int height) {
	// create color buffer
	uint8_t* rgb = (uint8_t*)malloc(width * height * 4);

	for (int i = 0; i < width * height; i++) {
		rgb[i * 3 + 0] = uint8_t(pixels[i].r * 255);
		rgb[i * 3 + 1] = uint8_t(pixels[i].g * 255);
		rgb[i * 3 + 2] = uint8_t(pixels[i].b * 255);
	}

	stbi_write_png(
		"scene.png",
		width, 
		height,
		3, 
		rgb,
		width*3 // stride in bytes for each row
	);

	free(rgb);
}
