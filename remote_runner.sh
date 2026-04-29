#!/bin/bash

# args: scene size bounces rank

echo -e "\nSYNCING...."
rsync -avz --exclude=".*" --exclude="scene.png" --exclude="remote_runner.sh" ./ aimos:scratch/proj

echo -e "\nCOMPILING..."
ssh aimos "cd scratch/proj && make && echo -e '\nAND RUNNING...' && mpirun -np $4 ./build/out $2 $2 $3 $1"

echo -e "\nFETCHING...."
scp aimos:scratch/proj/scene.png ./
