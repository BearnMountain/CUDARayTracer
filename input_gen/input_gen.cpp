#include <fstream>
#include <random>
#include <fstream>
#include <string>

double randf() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> dis(0.0, 1.0);
    return dis(gen);
}

// arg 1: output file
// arg 2: # of spheres
// arg 3: # of lights
// arg 4: double r such that x, y, z of spheres are in the range [-r, r]
// arg 5: max radius for spheres
int main(int argc, char** argv) {
    int s = std::stoi(argv[2]);
    int l = std::stoi(argv[3]);
    const double max_range = std::stod(argv[4]);
    const double max_radius = std::stod(argv[5]);

    std::ofstream ostr(argv[1]);

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

    while (l != 0) {
        ostr << "LIGHT ";
        ostr << (randf() - 0.5) * 4.0 * max_range << " ";
        ostr << (randf() - 0.5) * 4.0 * max_range << " ";
        ostr << (randf() - 0.5) * 4.0 * max_range << "\n";
        l--;
    }

    return EXIT_SUCCESS;
}
