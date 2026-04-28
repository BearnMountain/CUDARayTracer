#include "resource_manager.h"
#include "obj.h"
#include "bvh.h"
#include "parallel_ray.cu"	

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <algorithm>
#include <vector>
#include <mpi.h>

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

// shared globals
static u32 bounce_limit;
static u32 image_height;
static u32 image_width;

int main(int argc, char** argv) {
	// MPI intialization
	MPI_Init(&argc, &argv);
	i32 world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	if (argc != 5) {
		if (world_rank == 0) fprintf(stderr, "Requires ./exe width height bounce_limit filepath\n");
		MPI_Finalize();
		return 1;
	}

	// args
	image_width = atoi(argv[1]);
	image_height = atoi(argv[2]);
	bounce_limit = atoi(argv[3]);
	char* file_path = argv[4];

	// TODO: replace with write_scene_file so that it takes file and writes the scene
	std::vector<Sphere> scene = {
		{ {0.0,  0.0, -5.0}, 1.0, {1.0, 0.2, 0.2} }, // red
		{ {2.5,  0.0, -5.0}, 0.8, {0.2, 1.0, 0.2} }, // green
		{ {-2.5, 0.0, -5.0}, 1.2, {0.2, 0.2, 1.0} }, // blue
		{ {0.0,  2.0, -4.5}, 0.6, {1.0, 1.0, 0.2} }, // yellow
		{ {0.0, -1001.0, -5.0}, 1000.0, {0.8, 0.8, 0.8} }, // ground plane (huge sphere)
	};
	vec3 camera{0,0,0}; // can move this for different images
	std::vector<vec3>  light_sources = {
		{ vec3{0.6, 1.0, 0.4}.norm() }
	};

	// initialize the bvh
	BVH cpu_bvh(scene);

	// partition rows across ranks(easier than partitioning collumns)
    u32 base = image_height / world_size;
    u32 remainder = image_height % world_size;
    u32 row_start = world_rank * base + (world_rank < remainder ? world_rank : remainder);
    u32 local_rows = base + (world_rank < remainder ? 1 : 0); // rows for each rank

	// partitioning cuda j*bs
	std::vector<vec3> local_image((size_t)local_rows * image_width);

	// RUN CUDA
    run_parallel_kernel(&cpu_bvh,
                        local_image.data(),
                        local_rows,
                        row_start,       // FIX: was row_offset undefined; use row_start
                        image_width,
                        image_height,
                        bounce_limit);


	// gathering back processed partial images
	int total_sent = static_cast<u32>(local_rows * image_width);
	std::vector<i32> recv_count(world_size, 0); // how many elements each rank sends
	std::vector<i32> displacement(world_size, 0); // offset of each ranks data

	MPI_Gather(
		&total_sent,
		1,
		MPI_INT,
		recv_count.data(),
		1,
		MPI_INT
		0, 
		MPI_COMM_WORLD
	);

	std::vector<vec3> full_image;
	if (world_rank == 0) {
		displacement[0] = 0;
		for (u32 i = 1; i < world_size; ++i) {
			displacement[i] = displacement[i-1] + recv_count[i-1];
		}
		// only rank0 needs to allocate full image as its the only one that prints
		full_image.resize((size_t)image_height * image_width);
	}

	// to shared vec3 with mpi, convert to 3 packed doubles
	MPI_Datatype mpi_vec3;
	MPI_Type_contiguous(3, MPI_DOUBLE, &mpi_vec3);
	MPI_Type_commit(&mpi_vec3);

	MPI_Gatherv(
		local_image.data(), total_sent, mpi_vec3,
		full_image.data(), recv_count.data(),
		displacement.data(), mpi_vec3,
		0, MPI_COMM_WORLD
	);
	
	MPI_Type_free(&mpi_vec3);

	// write image
	if (world_rank == 0) {
		write_scene_file(full_image, image_width, image_height);
	}

	MPI_Finalize();

	return 0;
}

#if 0

int BOUNCE_LIMIT;
int WIDTH = 1000;
int HEIGHT = 1000;
float ASPECT_RATIO;

int main(int argc, char** argv) {
	WIDTH = atoi(argv[1]);
	HEIGHT = atoi(argv[2]);
	BOUNCE_LIMIT = atoi(argv[3]);
	ASPECT_RATIO = double(WIDTH)/HEIGHT;

	std::vector<Sphere> scene;
	std::vector<vec3> lights;
	read_scene_file(argv[4], scene, lights);

	bvh = new BVH(scene);

	vec3 camera{0,0,0}; // can move this for different images

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
				auto hit = bvh->intersect(bounce_ray);

				if (!hit) {
					// misses sphere, sets default color
					vec3 sky{0.05, 0.05, 0.12};
					pixel = pixel + ray_intensity * sky;
					break;
				}

				// Direct light contribution at this bounce
				double diff = std::max(0.0, hit->normal.dot(lights.front()));
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
			image[i * WIDTH + j] = pixel;
		}
	}

	write_scene_file(image, WIDTH, HEIGHT);

    return 0;
}
#endif
