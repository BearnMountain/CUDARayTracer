#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include "obj.h"
#include <stdint.h>
#include <vector>

void read_scene_file(const char* path, std::vector<Sphere>& spheres, std::vector<vec3>& lights);
void write_scene_file(std::vector<vec3>& pixels, int width, int height);

#endif // RESOURCE_MANAGER_H_
