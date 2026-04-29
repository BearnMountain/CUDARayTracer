mpiccargs  = -I/opt/ibm/spectrum_mpi/include
mpildargs  = -L/opt/ibm/spectrum_mpi/lib -lmpiprofilesupport -lmpi_ibm

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
	nvcc -arch=sm_70 -ccbin mpic++ -rdc=true $(OBJS) -o $@ -lstdc++ -lm $(mpildargs)

# compile c++ with mpi
build/%.o: src/%.cpp | build
	g++ -O3 $(mpiccargs) -c $< -o $@

# compile cuda with mpi
build/%.o: src/%.cu | build
	nvcc -O3 -arch=sm_70 -rdc=true $(mpiccargs) -c $< -o $@

# create build dir
build:
	mkdir -p build

# remove build folder
clean:
	rm -rf build
