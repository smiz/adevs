#include <fstream>
#include <iostream>




int main(int argc, char** argv) {

    ifstream fin(argv[1]);

    double max_count = 0.0, count = 0.0, t = 0.0, tl = 0.0, cost = 0.0;

    while (!fin.eof()) {
        cost += count * (t - tl);
        tl = t;
        max_count = max(max_count, count);
        fin >> t >> count;
    }

    fin.close();

    std::cout << "Cost/year: " << cost << std::endl;
    std::cout << "Max: " << max_count << std::endl;

    return 0;
}
