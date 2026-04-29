#include <fstream>
#include <random>
#include <fstream>
#include <string>

double randf() {
    return (double)rand() / (double)RAND_MAX;
}

int main(int argc, char** argv) {
    srand(0);
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
