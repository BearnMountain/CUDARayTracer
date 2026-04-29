#!/bin/bash

rsync -avz --exclude=".*" ./ aimos:scratch/proj
ssh aimos 'cd scratch/proj && make && mpirun -np 4 ./build/out 500 500 4 input/small_scene.txt'
scp aimos:scratch/proj/scene.png ./
