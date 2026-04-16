//
// Created by DJMixu on 16.04.2026.
//

#include "dataReader.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<double> loadServiceTimes(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Nie mozna otworzyc pliku: " + filename);

    std::vector<double> times;
    std::string token;
    while (file >> token) {
        std::replace(token.begin(), token.end(), ',', '.');
        times.push_back(std::stod(token));
    }
    return times;
}

std::vector<MinuteStat> loadDayProfile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Nie mozna otworzyc pliku: " + filename);

    std::vector<MinuteStat> profile;
    std::string mStr, lStr;
    while (file >> mStr >> lStr) {
        std::replace(lStr.begin(), lStr.end(), ',', '.');
        profile.push_back({std::stoi(mStr), std::stod(lStr)});
    }
    return profile;
}
