#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float pos[3];
    float radius;
} Sphere;

typedef struct {
    float pos[3];
} Light;

typedef struct {
    uint8_t r, g, b, a; // 0-255
} Color;

void read_scene_file(const char* path, Sphere* spheres, int* sphere_size, Light* lights, int* light_size);
void write_scene_file(Color* pixels, int width, int height);

#ifdef __cplusplus
}
#endif

#endif // RESOURCE_MANAGER_H_
