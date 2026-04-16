//
// Created by foxyu on 16.04.2026.
//

#include "generateDay.h"
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <algorithm>
#include <filesystem>


/**
 * Generuje jeden dzień
 */
void generate_day(const std::string& source_path, const std::string& output_path) {
    std::ifstream input_file(source_path);
    std::ofstream output_file(output_path);

    if (!input_file.is_open()) {
        std::cerr << "Nie udalo sie otworzyc pliku zrodlowego: " << source_path << "\n";
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.95, 1.05);

    std::string minute_str, load_str;

    while (input_file >> minute_str >> load_str) {
        // Konwersja wejścia (zamiana , na . aby C++ mogło policzyć)
        std::replace(load_str.begin(), load_str.end(), ',', '.');

        try {
            int minute = std::stoi(minute_str);
            double load = std::stod(load_str);

            double new_load = load * dist(gen);

            // Zapis z powrotem z przecinkiem
            output_file << minute << "\t" << new_load << "\n";
        } catch (...) {
            continue;
        }
    }
}

/**
 * Zarządza generowaniem dni.
 * @param overwrite Jeśli true, nadpisuje istniejące pliki.
 */
void ensure_days_exist(const std::string& source_file, int number_of_days, bool overwrite) {
    for (int i = 1; i <= number_of_days; ++i) {
        std::string filename = "INT" + std::to_string(i) + ".TXT";

        // Sprawdzamy, czy plik istnieje
        bool file_exists = std::filesystem::exists(filename);

        // Generujemy plik jeśli:
        // 1. Plik nie istnieje
        // LUB
        // 2. Jesli overwrite jest true, generujemy nowy zestaw dni
        if (!file_exists || overwrite) {
            if (file_exists && overwrite) {
                std::cout << "Nadpisuje istniejacy plik: " << filename << "\n";
            } else {
                std::cout << "Generowanie nowego pliku: " << filename << "\n";
            }

            generate_day(source_file, filename);
        } else {
            std::cout << "Plik " << filename << " juz istnieje. Pomijam.\n";
        }
    }
}