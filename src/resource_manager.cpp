#include "resource_manager.h"
#include "stb_image_write.h"
#include <stdint.h>
#include <stdio.h>
#include "obj.h"
#include <algorithm>
#include <fstream>
#include <iostream>

void read_scene_file(const char* path, std::vector<Sphere>& spheres, std::vector<vec3>& lights) {
    std::ifstream istr(path);
    std::string buf;
    while (istr >> buf) {
        if (buf == "LIGHT") {
            double x, y, z;
            istr >> x >> y >> z;
            lights.emplace_back() = vec3{ x, y, z }.norm();
        } else if (buf == "SPHERE") {
            double x, y, z;
            double radius;
            double r, g, b;
            istr >> x >> y >> z >> radius >> r >> g >> b;

            vec3 pos = { x, y, z };
            vec3 col = { r, g, b };
            spheres.emplace_back() = { pos, radius, col };
        } else {
            std::cerr << "ERROR: invalid code in input file\n";
            abort();
        }
    }
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
