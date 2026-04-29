mpiargs  = -I/opt/ibm/spectrum_mpi/include -L/opt/ibm/spectrum_mpi/lib -lmpiprofilesupport -lmpi_ibm

# sources
CPP_SRCS = $(wildcard src/*.cpp)
CU_SRCS  = $(wildcard src/*.cu)

# objs
CPP_OBJS = $(patsubst src/%.cpp,build/%.o,$(CPP_SRCS))
CU_OBJS  = $(patsubst src/%.cu,build/%.o,$(CU_SRCS))
OBJS     = $(CPP_OBJS) $(CU_OBJS)

TARGET = build/out

# link
$(TARGET): $(OBJS) | build
	mpic++ $(OBJS) -o $@ -L/usr/local/cuda-11.2/lib64/ -lcudadevrt -lcudart -lstdc++ -lm

# compile mpi c++
build/%.o: src/%.cpp | build
	g++ -O3 $(mpiargs) -c $< -o $@

# compile cuda
build/%.o: src/%.cu | build
	nvcc -O3 -c $< -o $@

# create build dir
build:
	mkdir -p build

clean:
	rm -rf build
