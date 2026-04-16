//
// Created by DJMixu on 16.04.2026.
//

#include "dataWriter.h"

#include <fstream>
#include <iomanip>
#include <iosfwd>
#include <iostream>
#include <vector>

#include "algorytmGNR1.h"
#include "models.h"
#include "supportFunctions.h"



void exportGnrLines(const std::vector<Call>& timeline,
                    const GnrResult& gnr,
                    const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Nie udalo sie utworzyc pliku: " + filename);

    const double w_end = gnr.window_start + gnr.window_size;

    file << std::fixed << std::setprecision(0);
    file << "Wywolania z pliku czas.txt nalezace do GNR\n";
    file << "Okno: " << formatTime(gnr.window_start) << " - " << formatTime(w_end) << "\n";
    file << "Ruch: " << gnr.max_erlangs << " Erlangow\n";
    file << "--------------------------------------------------\n";

    int line_number   = 1;
    int count_in_gnr  = 0;
    for (const auto& call : timeline) {
        if (call.end_time > gnr.window_start && call.start_time < w_end) {
            file << "Linia w pliku: " << line_number
                 << " \t| Czas obslugi: " << call.duration << " s\n";
            ++count_in_gnr;
        }
        ++line_number;
    }

    std::cout << "\nZnaleziono " << count_in_gnr << " zapytan tworzacych GNR." << std::endl;
    std::cout << "Zapisano dokladne numery linii do pliku: " << filename << std::endl;
}
