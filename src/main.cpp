#include <stdio.h>
#include <algorithm>
#include <vector>
#include <mpi.h>
#include <assert.h>

#include "resource_manager.h"
#include "obj.h"
#include "bvh.h"

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

	/* parse args */

	if (argc != 5) {
		fprintf(stderr, "Requires ./out width height bounce_limit filepath\n");
		return 1;
	}

	width = atoi(argv[1]);
	height = atoi(argv[2]);
	bounce_limit = atoi(argv[3]);
	char* file_path = argv[4];
	double aspect_ratio = (double)width / (double)height;

	/* initialize mpi */

	MPI_Init(&argc, &argv);
	i32 num_ranks, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* logically partition pixels */

	int total_pixels = width * height;
	assert(total_pixels % num_ranks == 0);

	// my pixels
	int pixels = total_pixels / num_ranks;
	int pixel_offset = pixels * rank;

	printf("Rank %d doing pixels [%d to %d]\n", rank, pixel_offset, pixel_offset + pixels - 1);
	

	/* start timing */

	ticks start, end;
	if (rank == 0) start = getticks();

	/* load scene */

	std::vector<Sphere> scene;
	std::vector<vec3> lights;
	read_scene_file(file_path, scene, lights);

	// partition rows across ranks(easier than partitioning collumns)
	// todo(jqj): verify makes sense, also prob don't need the remainder stuff, just use nice image sizes
    u32 base = height / num_ranks;
    u32 remainder = height % num_ranks;
    u32 row_start = rank * base + (rank < remainder ? rank : remainder);
    u32 local_rows = base + (rank < remainder ? 1 : 0); // rows for each rank

	/* render my portion */

	std::vector<vec3> image(rank == 0 ? total_pixels : pixels);
	BVH bvh(scene);
	vec3 camera{0,0,5};

	int x = pixels % width;
	int y = pixels / width;
	for (int i = 0; i < pixels; ++i) {

		assert(x < width && y < height);

		// normalized center of pixel 
		double u = (x + 0.5) / width * 2.0 - 1.0;
		double v = -((y + 0.5) / height * 2.0 - 1.0); // flip image rightside up

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
		image[i] = pixel;

		// increment coordinates
		++x;
		if (x == width) {
			// wrap at end of line
			x = 0;
			++y;
		}
	}

	// todo(jqj): maybe don't use vec for color
	if (rank == 0) write_image(image, width, height);

	// todo(jqj): run raytracing for my portion

	// todo(jqj): gather and write image

	/* end */

	if (rank == 0) {
		end = getticks();
		printf("Wrote image in %lf seconds, using %d ranks\n", (double)(end - start) / (double)512000000.0, num_ranks);
	}

	MPI_Finalize();

	return 0;
}
