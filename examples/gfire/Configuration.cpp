#include "Configuration.h"
using namespace std;

Configuration::Configuration(std::string config_file) {
    std::string field;
    data.open(config_file.c_str());
    while (!data.eof()) {
        data >> field;
        if (data.eof()) {
            break;
        }
        if (field == "width") {
            data >> width;
        } else if (field == "height") {
            data >> height;
        } else if (field == "fuel") {
            load_fuel();
        } else if (field == "fire") {
            load_fire();
        }
    }
    data.close();
}

void Configuration::load_fuel() {
    fuel = new double*[width];
    for (int i = 0; i < width; i++) {
        fuel[i] = new double[height];
    }
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            data >> fuel[i][j];
        }
    }
}

void Configuration::load_fire() {
    fire = new bool*[width];
    for (int i = 0; i < width; i++) {
        fire[i] = new bool[height];
    }
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int value;
            data >> value;
            fire[i][j] = (value != 0);
        }
    }
}

Configuration::~Configuration() {
    for (int j = 0; j < width; j++) {
        delete[] fuel[j];
        delete[] fire[j];
    }
    delete[] fuel;
    delete[] fire;
}
