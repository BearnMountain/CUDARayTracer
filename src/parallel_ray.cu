#pragma once
#include <cuda_runtime.h>
#include "obj.h"   // your vec3 type
#include "bvh.h"    // your BVH / HitRecord types

// ─── device-side BVH pointer (set once via cudaMemcpyToSymbol) ───────────────
__device__ BVH* d_bvh;

// ─── Sky colour for rays that escape the scene ───────────────────────────────
__device__ __forceinline__ vec3 sky_colour() {
    return vec3{0.05, 0.05, 0.12};
}

// ─── Per-pixel ray-tracing kernel ────────────────────────────────────────────
// grid  : (ceil(local_height/TILE), ceil(width/TILE))
// block : (TILE, TILE)
//
// Parameters
//   pixels       – output buffer, row-major [local_height × width]
//   width        – full image width
//   local_height – number of rows this MPI rank owns
//   row_offset   – first absolute row index for this rank
//   aspect_ratio – width / full_image_height
//   bounce_limit – maximum diffuse bounces
// ─────────────────────────────────────────────────────────────────────────────
__global__ void ray_kernel(
    vec3*  pixels,
    u32    width,
    u32    local_height,
    u32    row_offset,
    double aspect_ratio,
    u32    bounce_limit,
    u32    full_height)
{
    const u32 col = blockIdx.x * blockDim.x + threadIdx.x;
    const u32 local_row = blockIdx.y * blockDim.y + threadIdx.y;

    if (col >= width || local_row >= local_height) return;

    const u32 abs_row = row_offset + local_row;   // row in the full image

    // Normalised pixel centre [-1, 1]
    double u =  (col       + 0.5) / width       * 2.0 - 1.0;
    double v = -((abs_row  + 0.5) / full_height * 2.0 - 1.0); // flip Y

    vec3 camera{0.0, 0.0, 0.0};
    vec3 light = vec3{0.6, 1.0, 0.4}.norm();

    Ray   bounce_ray(camera, vec3{u * aspect_ratio, v, -1.0});
    vec3  pixel{0.0, 0.0, 0.0};
    vec3  ray_intensity{1.0, 1.0, 1.0};

    for (u32 b = 0; b < bounce_limit; ++b) {
        auto hit = d_bvh->intersect(bounce_ray);

        if (!hit) {
            pixel = pixel + ray_intensity * sky_colour();
            break;
        }

        // Diffuse shading
        double diff   = fmax(0.0, hit->normal.dot(light));
        vec3   emitted = hit->color * (0.15 + 0.85 * diff);
        pixel          = pixel + ray_intensity * emitted;

        // Attenuate by surface albedo
        ray_intensity  = ray_intensity * hit->color;

        // Mirror reflection
        double ndotd  = bounce_ray.dir.dot(hit->normal);
        vec3 reflected = bounce_ray.dir - hit->normal * 2.0 * ndotd;
        bounce_ray     = Ray(hit->point, reflected);

        // Russian-roulette early exit
        double max_i = fmax(ray_intensity.x, fmax(ray_intensity.y, ray_intensity.z));
        if (max_i < 0.01) break;
    }

    pixels[local_row * width + col] = pixel;
}

// ─── Host-callable wrapper ────────────────────────────────────────────────────
// Allocates device memory, launches the kernel, copies results back.
//
// host_bvh     – pointer to a BVH already constructed in CPU memory;
//                we copy it to the device symbol d_bvh.
// out_pixels   – pre-allocated host buffer [local_height × width]
// ─────────────────────────────────────────────────────────────────────────────
void launch_parallel_ray(
    BVH*   host_bvh,
    vec3*  out_pixels,
    u32    width,
    u32    full_height,
    u32    local_height,
    u32    row_offset,
    u32    bounce_limit)
{
    // ── 1. Copy BVH pointer into device symbol ────────────────────────────
    BVH* d_bvh_ptr;
    cudaMalloc(&d_bvh_ptr, sizeof(BVH));
    cudaMemcpy(d_bvh_ptr, host_bvh, sizeof(BVH), cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(d_bvh, &d_bvh_ptr, sizeof(BVH*));

    // ── 2. Allocate pixel buffer on device ───────────────────────────────
    vec3* d_pixels;
    const size_t buf_bytes = (size_t)local_height * width * sizeof(vec3);
    cudaMalloc(&d_pixels, buf_bytes);
    cudaMemset(d_pixels, 0, buf_bytes);

    // ── 3. Launch kernel ──────────────────────────────────────────────────
    constexpr u32 TILE = 16;
    dim3 block(TILE, TILE);
    dim3 grid(
        (width        + TILE - 1) / TILE,
        (local_height + TILE - 1) / TILE);

    double aspect_ratio = double(width) / double(full_height);

    ray_kernel<<<grid, block>>>(
        d_pixels,
        width,
        local_height,
        row_offset,
        aspect_ratio,
        bounce_limit,
        full_height);

    cudaDeviceSynchronize();

    // ── 4. Copy results back to host ──────────────────────────────────────
    cudaMemcpy(out_pixels, d_pixels, buf_bytes, cudaMemcpyDeviceToHost);

    // ── 5. Clean up ───────────────────────────────────────────────────────
    cudaFree(d_pixels);
    cudaFree(d_bvh_ptr);
}
