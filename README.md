# Parallel CUDA+MPI Raytracer
Rays are bounced from the camera to the scene, reflected, and used for shadows.

## Instructions
Code can be built with `make`. CUDA and MPI modules must be loaded.
The executable will be output in `build/out`. The executable can be run on
various scene files.

Here is an example which renders a pretty scene using all four GPUs on a compute
node: `mpirun -np 4 ./build/out 2048 2048 8 input/small_scene.txt scene.png`.

`scene.png` will be output in the current directory. It can be copied to your
home computer (perhaps with `scp`) and viewed.
