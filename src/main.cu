#include "resource_manager.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>

static int BOUNCE_LIMIT;
static int LIGHT_DISTANCE; // helps limit ray distance travel
static int HEIGHT;
static int WIDTH;

__global__ void hello_kernel() {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    printf("Hello from block (%d, %d), thread (%d, %d) | global (%d)\n",
           blockIdx.x, blockIdx.y,
           threadIdx.x, threadIdx.y,
           x + y * 1000);
}

int main(int argc, char** argv) {
	BOUNCE_LIMIT = 1;
	LIGHT_DISTANCE = 100;
	HEIGHT = 100;
	WIDTH = 100;

	Sphere spheres[3];



	Color image[HEIGHT * WIDTH];


	write_scene_file(image, WIDTH, HEIGHT);

    return 0;
}
