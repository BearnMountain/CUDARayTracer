#include <stdio.h>
#include <algorithm>
#include <vector>
#include <mpi.h>
#include <assert.h>

#include "scene.h"
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

// possible cool rendering ideas
// - proper procedural sky background
// - shadows

void cpu_render(vec3* buf, int starting_pixel, int pixels, int width, int height, Scene* scene, int bounce_limit);

int main(int argc, char** argv) {

	/* parse args */

	if (argc != 5) {
		fprintf(stderr, "Requires ./out width height bounce_limit filepath\n");
		return 1;
	}

	u32 width = atoi(argv[1]);
	u32 height = atoi(argv[2]);
	u32 bounce_limit = atoi(argv[3]);
	char* file_path = argv[4];

	/* initialize mpi */

	MPI_Init(&argc, &argv);
	i32 num_ranks, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// define custom vec3 type
	MPI_Datatype MPI_VEC3;
	MPI_Type_contiguous(3, MPI_DOUBLE, &MPI_VEC3);
	MPI_Type_commit(&MPI_VEC3);

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

	Scene scene(file_path);

	/* render my portion */

	// todo(jqj): parallelize on cuda
	std::vector<vec3> image(rank == 0 ? total_pixels : pixels);
	cpu_render(image.data(), pixel_offset, pixels, width, height, &scene, bounce_limit);

	/* gather and write image */

	if (rank == 0) {

		// grab data
		MPI_Gather(MPI_IN_PLACE, pixels, MPI_VEC3, 
				   image.data(), pixels, MPI_VEC3, 
				   0, MPI_COMM_WORLD);

		// write data to file

		std::vector<uint8_t> rgb(width * height * 3);

		for (int i = 0; i < width * height; i++) {
			rgb[i * 3 + 0] = (uint8_t)(CLAMP(image[i].x, 0.0f, 1.0f) * 255.0f);
			rgb[i * 3 + 1] = (uint8_t)(CLAMP(image[i].y, 0.0f, 1.0f) * 255.0f);
			rgb[i * 3 + 2] = (uint8_t)(CLAMP(image[i].z, 0.0f, 1.0f) * 255.0f);
		}

		stbi_write_png(
			"scene.png",
			width,
			height,
			3,
			rgb.data(),
			width * 3
		);

	} else {
		MPI_Gather(image.data(), pixels, MPI_VEC3, 
				   NULL, pixels, MPI_VEC3, 
				   0, MPI_COMM_WORLD);
	}

	/* end */

	if (rank == 0) {
		end = getticks();
		printf("Wrote image in %lf seconds, using %d ranks\n", (double)(end - start) / (double)512000000.0, num_ranks);
	}

	MPI_Finalize();

	return 0;
}

void cpu_render(vec3* buf, int starting_pixel, int pixels, int width, int height, Scene* scene, int bounce_limit) {

	double aspect_ratio = (double)width / (double)height;

	// iterate over all pixels in my portion
	int x = starting_pixel % width;
	int y = starting_pixel / width;
	for (int i = 0; i < pixels; ++i) {

		assert(x < width && y < height);

		// normalized center of pixel 
		double u = (x + 0.5) / width * 2.0 - 1.0;
		double v = -((y + 0.5) / height * 2.0 - 1.0); // flip image rightside up

		Ray ray(scene->get_cam_pos(), vec3{u * aspect_ratio, v, -1.0});
		vec3 pixel{0,0,0};
		vec3 ray_intensity{1,1,1};
		Ray bounce_ray = ray;

		for (int b = 0; b < bounce_limit; ++b) {

			Hit hit;
			bool did_hit = scene->intersect(bounce_ray, &hit);

			if (!did_hit) {
				// misses objects, sky color
				vec3 sky{0.2, 0.4, 0.6};
				pixel = pixel + ray_intensity * sky;
				break;
			}
			
			// Direct light contribution at this bounce
			double diff = MAX(0.0, hit.normal.dot(scene->get_sun_dir()));
			vec3 emitted = hit.color * (0.15 + 0.85 * diff);
			pixel = pixel + ray_intensity * emitted;  // accumulate

			// diffuse albedo: tints reflected light
			ray_intensity = ray_intensity * hit.color;

			// updates ray with direct reflection
			vec3 reflected = bounce_ray.dir - hit.normal * 2.0 * bounce_ray.dir.dot(hit.normal);
			bounce_ray = Ray(hit.point, reflected);

			// intensity cutoff to stop at a point
			if (ray_intensity.length() < 0.02) break;
		}

		// creates image based off ray direction
		buf[i] = pixel;

		// increment coordinates
		++x;
		if (x == width) {
			// wrap at end of line
			x = 0;
			++y;
		}
	}
}
