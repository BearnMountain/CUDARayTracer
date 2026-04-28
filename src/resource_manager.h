#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include "obj.h"
#include <stdint.h>
#include <vector>

void read_scene_file(const char* path, Sphere* spheres, int* sphere_size, vec3* lights, int* light_size);
void write_scene_file(std::vector<vec3>& pixels, int width, int height);

#endif // RESOURCE_MANAGER_H_
