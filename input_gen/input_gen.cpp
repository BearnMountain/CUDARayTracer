#include <fstream>
#include <random>
#include <fstream>
#include <string>
#include <cassert>

double randf() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> dis(0.0, 1.0);
    return dis(gen);
}

// arg 1: output file
// arg 2: # of spheres
// arg 3: double r such that x, y, z of spheres are in the range [-r, r]
// arg 4: max radius for spheres
int main(int argc, char** argv) {

	assert(argc == 5);

    int s = std::stoi(argv[2]);
    const double max_range = std::stod(argv[3]);
    const double max_radius = std::stod(argv[4]);

    std::ofstream ostr(argv[1]);

	ostr << "CAMERA ";
	ostr << 0 << " ";
	ostr << 0 << " ";
	ostr << 10 << "\n";

	ostr << "SUN ";
	ostr << (randf() - 0.5) * 4.0 * max_range << " ";
	ostr << (randf() - 0.5) * 4.0 * max_range << " ";
	ostr << (randf() - 0.5) * 4.0 * max_range << "\n";

    while (s != 0) {
        ostr << "SPHERE ";
        ostr << (randf() - 0.5) * 2.0 * max_range << " ";
        ostr << (randf() - 0.5) * 2.0 * max_range << " ";
        ostr << (randf() - 0.5) * 2.0 * max_range << " ";
        ostr << randf() * max_radius << " ";
        ostr << randf() << " ";
        ostr << randf() << " ";
        ostr << randf() << "\n";
        s--;
    }

    return EXIT_SUCCESS;
}
