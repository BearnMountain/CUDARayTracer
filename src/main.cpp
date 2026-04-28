#include "resource_manager.h"
#include "obj.h"
#include "bvh.h"

#include <stdio.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
<<<<<<< HEAD
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
=======
>>>>>>> 0dfa31c (added bvh interface, still needs implementation of functions)

static int BOUNCE_LIMIT;
static int HEIGHT = 450;
static int WIDTH = 800;
static double ASPECT_RATIO;

int main(int argc, char** argv) {
<<<<<<< HEAD
	HEIGHT = std::stoi(argv[2]);
	WIDTH = std::stoi(argv[3]);
    BOUNCE_LIMIT = std::stoi(argv[4]);
	ASPECT_RATIO = double(WIDTH)/HEIGHT;

	vec3 camera_center = vec3(0,0,0); // can move this for different images
	double viewport_height = 2.0;
	double viewport_width = viewport_height * ASPECT_RATIO;

	// vectors from viewport origin to edges
	vec3 viewport_x = vec3(viewport_width, 0, 0);
	vec3 viewport_y = vec3(0, -viewport_height, 0);
	vec3 pixel_delta_x = viewport_x / WIDTH;
	vec3 pixel_delta_y = viewport_y / HEIGHT;

    // parse input
    std::vector<Sphere> spheres;
    std::vector<Light> lights;
    {
        std::ifstream istr(argv[1]);
        std::string buf;
        while (istr >> buf) {
            if (buf == "LIGHT") {
                float x, y, z;
                float r, g, b;
                istr >> x >> y >> z >> r >> g >> b;

<<<<<<< HEAD
                vec3 pos = { x, y, z };
                Color col = { r, g, b };
                lights.emplace_back(pos, col);
            } else if (buf == "SPHERE") {
                float x, y, z;
                float radius;
                float r, g, b;
                istr >> x >> y >> z >> radius >> r >> g >> b;
=======
	// scene objects, replace with general file.txt
	Sphere spheres[] = {
>>>>>>> e470338 (changes to obj file for bvh and aabb intersection method)

                vec3 pos = { x, y, z };
                Color col = { r, g, b };
                spheres.emplace_back(pos, radius, col);
            } else {
                std::cerr << "ERROR: invalid code in file " << argv[1] << "\n";
                return EXIT_FAILURE;
            }
        }
    }
=======
	BOUNCE_LIMIT = 10;
	ASPECT_RATIO = double(WIDTH)/HEIGHT;

	// TODO: remove arbitrary spherse and replace with read_scene_file
	std::vector<Sphere> scene = {
		{ {0.0,  0.0, -5.0}, 1.0, {1.0, 0.2, 0.2} }, // red
		{ {2.5,  0.0, -5.0}, 0.8, {0.2, 1.0, 0.2} }, // green
		{ {-2.5, 0.0, -5.0}, 1.2, {0.2, 0.2, 1.0} }, // blue
		{ {0.0,  2.0, -4.5}, 0.6, {1.0, 1.0, 0.2} }, // yellow
		{ {0.0, -1001.0, -5.0}, 1000.0, {0.8, 0.8, 0.8} }, // ground plane (huge sphere)
	};
	BVH bvh(scene);

	vec3 camera{0,0,0}; // can move this for different images
	vec3 light = vec3{0.6, 1.0, 0.4}.norm();
>>>>>>> 0dfa31c (added bvh interface, still needs implementation of functions)

	// render
	std::vector<vec3> image(HEIGHT * WIDTH);

	for (int i = 0; i < HEIGHT; i++) { 
		for (int j = 0; j < WIDTH; j++) {
			// normalized center of pixel 
			double u = (j + 0.5) / WIDTH * 2.0 - 1.0;
			double v = -((i + 0.5) / HEIGHT * 2.0 - 1.0); // flip image rightside up

			Ray ray(camera, vec3{u * ASPECT_RATIO, v, -1.0});
			vec3 pixel{0,0,0};
			vec3 ray_intensity{1,1,1};
			Ray bounce_ray = ray;

			for (int b = 0; b < BOUNCE_LIMIT; ++b) {
				auto hit = bvh.intersect(bounce_ray);

				if (!hit) {
					// misses sphere, sets default color
					vec3 sky{0.05, 0.05, 0.12};
					pixel = pixel + ray_intensity * sky;
					break;
				}

				// Direct light contribution at this bounce
				double diff = std::max(0.0, hit->normal.dot(light));
				vec3 emitted = hit->color * (0.15 + 0.85 * diff);
				pixel = pixel + ray_intensity * emitted;  // accumulate

				// diffuse albedo: tints reflected light
				ray_intensity = ray_intensity * hit->color;

				// updates ray with direct reflection
				vec3 reflected = bounce_ray.dir - hit->normal * 2.0 * bounce_ray.dir.dot(hit->normal);
				bounce_ray = Ray(hit->point, reflected);

				// intensity cutoff to stop at a point
				if (std::max({ray_intensity.x, ray_intensity.y, ray_intensity.z}) < 0.01) break;
			}

			// creates image based off ray direction
<<<<<<< HEAD
			ray r(camera_center, ray_direction);
			image[i * WIDTH + j] = ray_color(r, spheres.data(), spheres.size(), lights.front());
=======
			image[i * WIDTH + j] = pixel;
>>>>>>> 0dfa31c (added bvh interface, still needs implementation of functions)
		}
	}

	write_scene_file(image, WIDTH, HEIGHT);

    return 0;
}
