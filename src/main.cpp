#include <stdio.h>
#include <algorithm>
#include <vector>
#include <mpi.h>

#include "resource_manager.h"
#include "obj.h"
#include "bvh.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using ticks = unsigned long;
static __inline__ ticks getticks(void) {
    unsigned int tbl, tbu0, tbu1;
    do {
        __asm__ __volatile__ ("mftbu %0" : "=r"(tbu0));
        __asm__ __volatile__ ("mftb %0" : "=r"(tbl));
        __asm__ __volatile__ ("mftbu %0" : "=r"(tbu1));
    } while (tbu0 != tbu1);
    return (((unsigned long long)tbu0) << 32) | tbl;
}

// args
u32 width;
u32 height;
u32 bounce_limit;

int main(int argc, char** argv) {

	if (argc != 5) {
		fprintf(stderr, "Requires ./out width height bounce_limit filepath\n");
		return 1;
	}

	// MPI intialization
	MPI_Init(&argc, &argv);
	i32 world_size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// args
	width = atoi(argv[1]);
	height = atoi(argv[2]);
	bounce_limit = atoi(argv[3]);
	char* file_path = argv[4];
	double aspect_ratio = (double)width / (double)height;

	std::vector<Sphere> scene;
	std::vector<vec3> lights;
	read_scene_file(file_path, scene, lights);

	// partition rows across ranks(easier than partitioning collumns)
	// todo(jqj): verify makes sense, also prob don't need the remainder stuff, just use nice image sizes
    u32 base = height / world_size;
    u32 remainder = height % world_size;
    u32 row_start = rank * base + (rank < remainder ? rank : remainder);
    u32 local_rows = base + (rank < remainder ? 1 : 0); // rows for each rank

	/* temp run */

	BVH bvh(scene);

	vec3 camera{0,0,0}; // can move this for different images

	// render
	std::vector<vec3> image(height * width);

	for (int i = 0; i < height; i++) { 
		for (int j = 0; j < width; j++) {

			// normalized center of pixel 
			double u = (j + 0.5) / width * 2.0 - 1.0;
			double v = -((i + 0.5) / height * 2.0 - 1.0); // flip image rightside up

			Ray ray(camera, vec3{u * aspect_ratio, v, -1.0});
			vec3 pixel{0,0,0};
			vec3 ray_intensity{1,1,1};
			Ray bounce_ray = ray;

			for (int b = 0; b < bounce_limit; ++b) {

				Hit hit;
				bool did_hit = bvh.intersect(bounce_ray, &hit);

				if (!did_hit) {
					// misses objects, sky color
					vec3 sky{0.2, 0.4, 0.6};
					pixel = pixel + ray_intensity * sky;
					break;
				}
			
				// Direct light contribution at this bounce
				double diff = std::max(0.0, hit.normal.dot(lights.front()));
				vec3 emitted = hit.color * (0.15 + 0.85 * diff);
				pixel = pixel + ray_intensity * emitted;  // accumulate

				// diffuse albedo: tints reflected light
				ray_intensity = ray_intensity * hit.color;

				// updates ray with direct reflection
				vec3 reflected = bounce_ray.dir - hit.normal * 2.0 * bounce_ray.dir.dot(hit.normal);
				bounce_ray = Ray(hit.point, reflected);

				// intensity cutoff to stop at a point
				if (std::max({ray_intensity.x, ray_intensity.y, ray_intensity.z}) < 0.01) break;
			}

			// creates image based off ray direction
			image[i * width + j] = pixel;
		}
	}

	// todo(jqj): maybe don't use vec for color
	if (rank == 0) write_image(image, width, height);

	// todo(jqj): run raytracing for my portion

	// todo(jqj): gather and write image

	MPI_Finalize();

	return 0;
}

// int BOUNCE_LIMIT;
// int width = 1000;
// int height = 1000;
// float ASPECT_RATIO;

// int main(int argc, char** argv) {
// 	width = atoi(argv[1]);
// 	height = atoi(argv[2]);
// 	BOUNCE_LIMIT = atoi(argv[3]);
// 	ASPECT_RATIO = double(width)/height;

// 	std::vector<Sphere> scene;
// 	std::vector<vec3> lights;
// 	read_scene_file(argv[4], scene, lights);

// 	bvh = new BVH(scene);

// 	vec3 camera{0,0,0}; // can move this for different images

// 	// render
// 	std::vector<vec3> image(height * width);

// 	for (int i = 0; i < height; i++) { 
// 		for (int j = 0; j < width; j++) {
// 			// normalized center of pixel 
// 			double u = (j + 0.5) / width * 2.0 - 1.0;
// 			double v = -((i + 0.5) / height * 2.0 - 1.0); // flip image rightside up

// 			Ray ray(camera, vec3{u * ASPECT_RATIO, v, -1.0});
// 			vec3 pixel{0,0,0};
// 			vec3 ray_intensity{1,1,1};
// 			Ray bounce_ray = ray;

// 			for (int b = 0; b < BOUNCE_LIMIT; ++b) {
// 				auto hit = bvh->intersect(bounce_ray);

// 				if (!hit) {
// 					// misses sphere, sets default color
// 					vec3 sky{0.05, 0.05, 0.12};
// 					pixel = pixel + ray_intensity * sky;
// 					break;
// 				}

			
// 	// Direct light contribution at this bounce
// 				double diff = std::max(0.0, hit.normal.dot(lights.front()));
// 				vec3 emitted = hit.color * (0.15 + 0.85 * diff);
// 				pixel = pixel + ray_intensity * emitted;  // accumulate

// 				// diffuse albedo: tints reflected light
// 				ray_intensity = ray_intensity * hit.color;

// 				// updates ray with direct reflection
// 				vec3 reflected = bounce_ray.dir - hit.normal * 2.0 * bounce_ray.dir.dot(hit.normal);
// 				bounce_ray = Ray(hit.point, reflected);

// 				// intensity cutoff to stop at a point
// 				if (std::max({ray_intensity.x, ray_intensity.y, ray_intensity.z}) < 0.01) break;
// 			}

// 			// creates image based off ray direction
// 			image[i * width + j] = pixel;
// 		}
// 	}

// 	write_scene_file(image, width, height);

//     return 0;
// }
