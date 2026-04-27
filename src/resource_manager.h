#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include "vec3.h"
#include <stdint.h>

class Color {
public:
	Color() {}
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
	uint8_t r,g,b,a;
};

void read_scene_file(const char* path, Sphere* spheres, int* sphere_size, Light* lights, int* light_size);
void write_scene_file(Color* pixels, int width, int height);

#endif // RESOURCE_MANAGER_H_
