#include "ray.h"
#include "resource_manager.h"
#include "vec3.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>

static int BOUNCE_LIMIT;
static int HEIGHT;
static int WIDTH;
static double ASPECT_RATIO;

__global__ void hello_kernel() {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    printf("Hello from block (%d, %d), thread (%d, %d) | global (%d)\n",
           blockIdx.x, blockIdx.y,
           threadIdx.x, threadIdx.y,
           x + y * 1000);
}

// helper
bool hit_sphere(Sphere* sphere, const ray& r) {
	// https://kylehalladay.com/blog/tutorial/math/2013/12/24/Ray-Sphere-Intersection.html
	// simpler version
    vec3 oc = sphere->pos() - r.getOrigin();

	// quadratic
    auto a = dot(r.getDirection(), r.getDirection());
    auto b = 2.0 * dot(r.getDirection(), oc);
    auto c = dot(oc, oc) - sphere->radius() * sphere->radius();
    auto discriminant = b*b - 4*a*c; // gives total number of roots/intersections
    return (discriminant >= 0); 
}

Color ray_color(const ray& r) {
	Sphere sphere(vec3(0,0,-1), 0.5);
	if (hit_sphere(&sphere, r)) return Color(255,0,0,255);
	return Color(0,0,0,255);
}

int main(int argc, char** argv) {
	BOUNCE_LIMIT = 1;
	HEIGHT = 100;
	WIDTH = 100;
	ASPECT_RATIO = double(WIDTH)/HEIGHT;

	double viewport_height = 2.0;
	double viewport_width = viewport_height * ASPECT_RATIO;
	vec3 camera_center = vec3(0,0,0); // can move this for different images

	// vectors from viewport origin to edges
	vec3 viewport_x = vec3(viewport_width, 0, 0);
	vec3 viewport_y = vec3(0, -viewport_height, 0);
	vec3 pixel_delta_x = viewport_x / WIDTH;
	vec3 pixel_delta_y = viewport_y / HEIGHT;

	// drawing scene
	// Sphere spheres[3];
	// Color image[HEIGHT * WIDTH];


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
			image[i * WIDTH + j] = ray_color(r);
		}
	}

	write_scene_file(image, WIDTH, HEIGHT);

    return 0;
}
