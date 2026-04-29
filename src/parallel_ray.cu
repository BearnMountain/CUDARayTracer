#include <cuda_runtime.h>
#include "vec3.h"
#include "bvh.h"
#include "ray.h"

__device__ BVH* shared_bvh;

extern __device__ u32 bounce_limit;
extern __device__ u32 image_width;
extern __device__ u32 image_height;

// ─── Device: trace one path from a pixel ─────────────────────────────────────
__device__ vec3 trace_pixel(int px, int py, BVH* bvh,
                             u32 width, u32 height, u32 bounces)
{
    double aspect = (double)width / height;

    // Normalised pixel centre in [-1,1]
    double u =  ((px + 0.5) / width)  * 2.0 - 1.0;
    double v = -((py + 0.5) / height) * 2.0 - 1.0;

    vec3 camera{0, 0, 0};
    vec3 light = vec3{0.6, 1.0, 0.4}.norm();

    Ray ray(camera, vec3{u * aspect, v, -1.0}.norm());

    vec3 pixel{0, 0, 0};
    vec3 throughput{1, 1, 1};

    for (u32 b = 0; b < bounces; ++b) {
        auto hit = bvh->intersect(ray);
        if (!hit) {
            vec3 sky{0.05, 0.05, 0.12};
            pixel = pixel + throughput * sky;
            break;
        }

        double diff = fmax(0.0, hit->normal.dot(light));
        vec3 emitted = hit->color * (0.15 + 0.85 * diff);
        pixel = pixel + throughput * emitted;

        throughput = throughput * hit->color;

        // Early exit if contribution is negligible
        if (fmax(fmax(throughput.x, throughput.y), throughput.z) < 0.01)
            break;

        // Reflect
        vec3 refl = ray.dir - hit->normal * 2.0 * ray.dir.dot(hit->normal);
        ray = Ray(hit->point, refl.norm());
    }

    return pixel;
}

// ─── Kernel: one thread per pixel, covers local_rows rows ────────────────────
//
//  row_offset  – first absolute row this rank owns
//  local_rows  – how many rows this rank owns
//  out_pixels  – device buffer sized  local_rows * image_width
__global__ void ray_kernel(BVH*  bvh,
                            vec3* out_pixels,
                            u32   row_offset,
                            u32   local_rows,
                            u32   width,
                            u32   height,
                            u32   bounces)
{
    // 2-D thread index → pixel coordinate
    u32 px = blockIdx.x * blockDim.x + threadIdx.x;   // column
    u32 ly = blockIdx.y * blockDim.y + threadIdx.y;   // local row index

    if (px >= width || ly >= local_rows) return;

    u32 py = row_offset + ly;   // absolute row in the full image

    out_pixels[ly * width + px] = trace_pixel(px, py, bvh, width, height, bounces);
}

// ─── Host launch function ─────────────────────────────────────────────────────
void run_parallel_kernel(BVH*  cpu_bvh,
                         vec3* out_pixels,   // host output buffer
                         u32   local_rows,
                         u32   row_offset,
                         u32   width,
                         u32   height,
                         u32   bounces)
{
    // ── 1. Upload BVH to device ───────────────────────────────────────────────
    BVH* d_bvh;
    cudaMalloc(&d_bvh, sizeof(BVH));
    cudaMemcpy(d_bvh, cpu_bvh, sizeof(BVH), cudaMemcpyHostToDevice);

    // Store in the global device pointer (optional – kernel receives it directly)
    cudaMemcpyToSymbol(shared_bvh, &d_bvh, sizeof(BVH*));

    // ── 2. Allocate device pixel buffer ──────────────────────────────────────
    size_t n_pixels = (size_t)local_rows * width;
    vec3*  d_pixels;
    cudaMalloc(&d_pixels, n_pixels * sizeof(vec3));

    // ── 3. Launch ─────────────────────────────────────────────────────────────
    // 16×16 thread blocks is a common sweet-spot for pixel workloads
    dim3 block(16, 16);
    dim3 grid((width      + block.x - 1) / block.x,
              (local_rows + block.y - 1) / block.y);

    ray_kernel<<<grid, block>>>(d_bvh, d_pixels,
                                row_offset, local_rows,
                                width, height, bounces);
    cudaDeviceSynchronize();

    // ── 4. Copy result back to host ───────────────────────────────────────────
    cudaMemcpy(out_pixels, d_pixels, n_pixels * sizeof(vec3), cudaMemcpyDeviceToHost);

    // ── 5. Cleanup ────────────────────────────────────────────────────────────
    cudaFree(d_pixels);
    cudaFree(d_bvh);
}
