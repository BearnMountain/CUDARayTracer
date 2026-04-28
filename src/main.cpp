#include "resource_manager.h"
#include "obj.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>

static int BOUNCE_LIMIT;
static int HEIGHT;
static int WIDTH;
static double ASPECT_RATIO;

// __global__ void hello_kernel() {
//     int x = blockIdx.x * blockDim.x + threadIdx.x;
//     int y = blockIdx.y * blockDim.y + threadIdx.y;
//
//     printf("Hello from block (%d, %d), thread (%d, %d) | global (%d)\n",
//            blockIdx.x, blockIdx.y,
//            threadIdx.x, threadIdx.y,
//            x + y * 1000);
// }

// helper
bool intersect(const Sphere& s, const ray& r, double& t_hit) {
    vec3 oc = r.getOrigin() - s.pos;

    double a = dot(r.getDirection(), r.getDirection());
    double b = 2.0 * dot(oc, r.getDirection());
    double c = dot(oc, oc) - s.radius * s.radius;

    double discriminant = b*b - 4*a*c;
    if (discriminant < 0) return false;

    double sqrtd = std::sqrt(discriminant);

    double t1 = (-b - sqrtd) / (2*a);
    double t2 = (-b + sqrtd) / (2*a);

    if (t1 > 0) {
        t_hit = t1;
        return true;
    }
    if (t2 > 0) {
        t_hit = t2;
        return true;
    }

    return false;
}

bool hit_world(const ray& r, Sphere* spheres, int sphere_count, double& t_min, Sphere*& hit_sphere_out) {
    bool hit_anything = false;
    t_min = 1e30;

    for (int i = 0; i < sphere_count; i++) {
        double t;
        if (intersect(spheres[i], r, t)) {
            if (t < t_min) {
                t_min = t;
                hit_sphere_out = &spheres[i];
                hit_anything = true;
            }
        }
    }

    return hit_anything;
}

Color shade(const vec3& hit_point, const vec3& normal, const Light& light, const Color& base_color) {
    vec3 light_dir = unit_vector(light.pos - hit_point);
    double diff = std::max(0.0, dot(normal, light_dir));
    return Color(base_color.r * diff, base_color.g * diff, base_color.b * diff);
}

Color ray_color(const ray& r, Sphere* spheres, int sphere_count, const Light& light) {
    double t;
    Sphere* hit_sphere = nullptr;

    if (hit_world(r, spheres, sphere_count, t, hit_sphere)) {

        vec3 hit_point = r.at(t);
        vec3 normal = unit_vector(hit_point - hit_sphere->pos);

        return shade(hit_point, normal, light, hit_sphere->color);
    }

    // background
    vec3 unit_dir = unit_vector(r.getDirection());
    double a = 0.5 * (unit_dir.y + 1.0);

    return (1.0 - a) * Color(1.0, 1.0, 1.0) + a * Color(0.5, 0.7, 1.0);
}

int main(int argc, char** argv) {
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

	// render
	Color image[HEIGHT * WIDTH];

	printf("%d\n", BOUNCE_LIMIT);

	vec3 viewport_start = camera_center - vec3(0,0,1.0) - viewport_x/2 - viewport_y/2; // gets 0,0 for render
	vec3 start_pixel_center = viewport_start + 0.5 * (pixel_delta_x + pixel_delta_y); // gets center of starting pixel for ray
	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			vec3 pixel_center = start_pixel_center + (j * pixel_delta_x) + (i * pixel_delta_y);
			vec3 ray_direction = pixel_center - camera_center;

			// creates image based off ray direction
			ray r(camera_center, ray_direction);
			image[i * WIDTH + j] = ray_color(r, spheres.data(), spheres.size(), lights.front());
		}
	}

	write_scene_file(image, WIDTH, HEIGHT);

    return 0;
}
