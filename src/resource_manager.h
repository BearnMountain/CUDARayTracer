#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include "vec3.h"
#include <stdint.h>

void read_scene_file(const char* path, Sphere* spheres, int* sphere_size, Light* lights, int* light_size);
void write_scene_file(Color* pixels, int width, int height);

#endif // RESOURCE_MANAGER_H_
