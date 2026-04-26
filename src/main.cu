#include "resource_manager.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>

__global__ void hello_kernel() {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    printf("Hello from block (%d, %d), thread (%d, %d) | global (%d)\n",
           blockIdx.x, blockIdx.y,
           threadIdx.x, threadIdx.y,
           x + y * 1000);
}

int main() {
    dim3 block(2, 2);   // 4 threads per block
    dim3 grid(2, 2);    // 4 blocks total

    printf("Launching kernel...\n");

    hello_kernel<<<grid, block>>>();

    cudaDeviceSynchronize();

    printf("Done.\n");

	int width = 100;
	int height = 100;

	Color image[100 * 100];

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {

			int idx = i * width + j;

			uint8_t u = (uint8_t)((j * 255) / (width - 1));
			uint8_t v = (uint8_t)((i * 255) / (height - 1));

			image[idx].r = u;        // horizontal gradient
			image[idx].g = v;        // vertical gradient
			image[idx].b = 128;      // constant mid-blue
			image[idx].a = 255;      // fully opaque
		}
	}

	write_scene_file(image, width, height);

    return 0;
}
