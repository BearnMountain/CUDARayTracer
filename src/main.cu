#include "resource_manager.h"
#include "obj.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>

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

bool hit_world(
    const ray& r,
    Sphere* spheres,
    int sphere_count,
    double& t_min,
    Sphere*& hit_sphere_out
) {
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

// Color ray_color(const ray& r, spheres, 2, light) {
// 	Sphere sphere(vec3(0,0,-1), 0.5);
// 	double root = intersect(sphere, r);
// 	if (root != -1.0) {
// 		vec3 norm = unit_vector(r.at(root) - vec3(0,0,-1));
//         return Color((uint8_t)((norm.x()+1)*122), (uint8_t)((norm.y()+1)*122), (uint8_t)((norm.z()+1)*122), 255);
// 	}
//
// 	return Color(0,0,0,255);
// }

int main(int argc, char** argv) {
	BOUNCE_LIMIT = 1;
	HEIGHT = 100;
	WIDTH = 100;
	ASPECT_RATIO = double(WIDTH)/HEIGHT;

	vec3 camera_center = vec3(0,0,0); // can move this for different images
	double viewport_height = 2.0;
	double viewport_width = viewport_height * ASPECT_RATIO;

	// vectors from viewport origin to edges
	vec3 viewport_x = vec3(viewport_width, 0, 0);
	vec3 viewport_y = vec3(0, -viewport_height, 0);
	vec3 pixel_delta_x = viewport_x / WIDTH;
	vec3 pixel_delta_y = viewport_y / HEIGHT;


	// scene objects
	Sphere spheres[] = {

		// small red sphere (center)
		{ vec3(0, 0, -1), 0.5, Color(1, 0, 0) },

		// green "ground" sphere
		{ vec3(0, -100.5, -1), 100.0, Color(0.2, 0.8, 0.2) },

		// blue sphere slightly behind
		{ vec3(1.0, 0.0, -1.5), 0.5, Color(0.2, 0.3, 1.0) },

		// yellow sphere left side
		{ vec3(-1.2, 0.1, -0.8), 0.4, Color(1.0, 0.9, 0.2) },

		// small dark sphere far away (tests depth precision)
		{ vec3(0.0, 0.5, -3.0), 0.3, Color(0.4, 0.4, 0.4) },

		// big overlapping sphere behind everything
		{ vec3(0.0, 1.0, -2.0), 1.2, Color(0.8, 0.2, 0.6) }
	};

	int sphere_count = sizeof(spheres) / sizeof(Sphere);
	Light light = { vec3(-2,5,0), Color(1.0, 1.0, 1.0) };

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
			image[i * WIDTH + j] = ray_color(r, spheres, sphere_count, light);
		}
	}

	write_scene_file(image, WIDTH, HEIGHT);

    return 0;
}
