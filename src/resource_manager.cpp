#include "resource_manager.h"
#include "stb_image_write.h"
#include <stdint.h>
#include <stdio.h>

void parse_scene_file(const char* path, Sphere* spheres, int* sphere_size, Light* lights, int* light_size) {
	FILE* file = fopen(path, "r");


	fclose(file);
}


void write_scene_file(Color* pixels, int width, int height) {
	// create color buffer
	uint8_t* rgba = (uint8_t*)malloc(width * height * 4);

	for (int i = 0; i < width * height; i++) {
		rgba[i * 4 + 0] = pixels[i].r;
		rgba[i * 4 + 1] = pixels[i].g;
		rgba[i * 4 + 2] = pixels[i].b;
		rgba[i * 4 + 3] = pixels[i].a;
	}

	stbi_write_png(
		"scene.png",
		width, 
		height,
		4, 
		rgba,
		width*4 // stride in bytes for each row
	);

	free(rgba);
}
