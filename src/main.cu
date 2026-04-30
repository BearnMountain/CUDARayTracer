#include <stdio.h>
#include <algorithm>
#include <vector>
#include <mpi.h>
#include <assert.h>
#include <cuda_runtime.h>

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

// possible performance improvements
// - 2d kernel execution (requires the mpi partitioning logic to change)
// - don't use vec3 of doubles for color
// - things to vary: image size, number of spheres, number of threads per block

__global__ void gpu_render(vec3* buf, int starting_pixel, int pixels, int width, int height, GPUScene scene, int bounce_limit);

int main(int argc, char** argv) {

	/* parse args */

	if (argc != 6 && argc != 7) {
		fprintf(stderr, "Requires ./out width height bounce_limit input output [optional verbose 0/1 (default 1)]\n");
		return 1;
	}

	u32 width = atoi(argv[1]);
	u32 height = atoi(argv[2]);
	u32 bounce_limit = atoi(argv[3]);
	char* file_path = argv[4];
	char* output_path = argv[5];
	bool verbose = (argc == 7 ? atoi(argv[6]) : true);

	/* initialize mpi */

	MPI_Init(&argc, &argv);
	i32 num_ranks, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* warm up cuda driver */

	cudaSetDevice(rank % 4);
	cudaFree(0);

	/* logically partition pixels */

	int total_pixels = width * height;
	assert(total_pixels % num_ranks == 0);

	// my pixels
	int pixels = total_pixels / num_ranks;
	int pixel_offset = pixels * rank;

	printf("Rank %d doing pixels [%d to %d]\n", rank, pixel_offset, pixel_offset + pixels - 1);

	/* start timing */

	ticks start, before_render, after_render, end;
	if (rank == 0) start = getticks();

	/* load scene */

	Scene scene(file_path);
	GPUScene gpu_scene = scene.copy_to_gpu();

	/* allocate image memory */

	// allocate memory for image portion (or full image on rank 0)
	vec3* image = NULL;
	cudaError_t err = cudaMallocManaged(&image, (rank == 0 ? total_pixels : pixels) * sizeof(vec3));
	if (err != cudaSuccess) {
		fprintf(stderr, "Failed to allocate managed memory: %s\n", cudaGetErrorString(err));
		abort();
	}

	/* render my portion */

	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0) {
		before_render = getticks();
	}

	int threads = 256;
    int blocks = (pixels + threads - 1) / threads;
	gpu_render<<<blocks, threads>>>(image, pixel_offset, pixels, width, height, gpu_scene, bounce_limit);
	cudaDeviceSynchronize();
	// todo(jqj): investigate performance scaling. The network send is probably the bottleneck

	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0) {
		after_render = getticks();
		if (verbose) printf("Rendering finished, constructing final image\n");
	}

	/* gather and write image */

	if (rank == 0) {

		// grab data
		MPI_Gather(MPI_IN_PLACE, pixels * 3, MPI_DOUBLE, 
				   image, pixels * 3, MPI_DOUBLE, 
				   0, MPI_COMM_WORLD);

		// write data to file

		std::vector<uint8_t> rgb(width * height * 3);

		for (int i = 0; i < width * height; i++) {
			rgb[i * 3 + 0] = (uint8_t)(CLAMP(image[i].x, 0.0, 1.0) * 255.0f);
			rgb[i * 3 + 1] = (uint8_t)(CLAMP(image[i].y, 0.0, 1.0) * 255.0f);
			rgb[i * 3 + 2] = (uint8_t)(CLAMP(image[i].z, 0.0, 1.0) * 255.0f);
		}

		stbi_write_png(
			output_path,
			width,
			height,
			3,
			rgb.data(),
			width * 3
		);

	} else {
		MPI_Gather(image, pixels * 3, MPI_DOUBLE, 
				   NULL, pixels * 3, MPI_DOUBLE, 
				   0, MPI_COMM_WORLD);
	}

	/* end */

	if (rank == 0) {
		end = getticks();
		if (verbose) {
			printf("Started rendering after %lf seconds\n", (double)(before_render - start) / (double)512000000.0);
			printf("Rendered image in %lf seconds\n", (double)(after_render - before_render) / (double)512000000.0);
			printf("Transmit and wrote image in %lf seconds\n", (double)(end - after_render) / (double)512000000.0);
			printf("Total time %lf seconds, with %d ranks\n", (double)(end - start) / (double)512000000.0, num_ranks);
		} else {
			// measure bvh construction and rendering time
			printf("%dx%d,%d,%lf,%lf\n", width, height, num_ranks, (double)(after_render - start) / (double)512000000.0, (double)(end - start) / (double)512000000.0);
		}
	}

	MPI_Finalize();
	gpu_scene.free();
	cudaFree(image);

	return 0;
}

__global__ void gpu_render(vec3* buf, int starting_pixel, int pixels, int width, int height, GPUScene scene, int bounce_limit) {

	// find index in pixels
	int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= pixels) return;

	// find x y coordinate
	int global_pixel = starting_pixel + i;
	int x = global_pixel % width;
	int y = global_pixel / width;

	// normalized center of pixel 
	double aspect_ratio = (double)width / (double)height;
	double u = (x + 0.5) / width * 2.0 - 1.0;
	double v = -((y + 0.5) / height * 2.0 - 1.0); // flip image rightside up

	Ray ray(scene.cam_pos, vec3{u * aspect_ratio, v, -1.0});
	vec3 pixel{0,0,0};
	vec3 ray_intensity{1,1,1};
	Ray bounce_ray = ray;

	for (int b = 0; b < bounce_limit; ++b) {

		Hit hit;
		bool did_hit = scene.intersect(bounce_ray, &hit);

		if (!did_hit) {
			// misses objects, sky color
			vec3 sky{0.2, 0.4 + (ray.dir.y) * 0.4, 0.6};
			pixel = pixel + ray_intensity * sky;
			break;
		}

		/* shadow calculation bounce */
		Hit shadow_hit;
		Ray shadow_ray = Ray(hit.point, {scene.sun_dir.x, scene.sun_dir.y, scene.sun_dir.z});
		bool did_shadow_hit = scene.intersect(shadow_ray, &shadow_hit);
			
		/* calculate light this bounce */

		double diff = MAX(0.0, hit.normal.dot(scene.sun_dir));
		vec3 emitted = hit.color * (0.15 + (!did_shadow_hit ? 0.85 * diff : 0.0 * diff));
		pixel = pixel + ray_intensity * emitted;

		/* update ray for next bound */

		// diffuse albedo: tints reflected light
		ray_intensity = ray_intensity * hit.color;

		// updates ray with direct reflection
		vec3 reflected = bounce_ray.dir - hit.normal * 2.0 * bounce_ray.dir.dot(hit.normal);
		bounce_ray = Ray(hit.point, reflected);

		// intensity cutoff to stop at a point
		if (ray_intensity.length() < 0.02) break;
	}

	// write to image
	buf[i] = pixel;
}
