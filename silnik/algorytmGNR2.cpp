//
// Created by foxyu on 27.05.2026.
//

#include "algorytmGNR2.h"
#include "algorytmGNR1.h"
#include <iomanip>
#include <iostream>

#include "dataReader.h"
#include "models.h"
#include "timeLineBuilder.h"


GnrResult obliczADPQH(int num_days, const std::vector<double>& service_times, double window_size, double step_size) {
    double suma_maksymow_erlanga = 0.0;
    int przetworzone_dni = 0;

    std::cout << "\n=== ROZPOCZYNAM OBLICZENIA ADPQH ===" << std::endl;
    std::cout << "Konfiguracja - Okno: " << (window_size / 60.0) << " min, "
              << "Krok (dokladnosc): " << (step_size / 60.0) << " min\n" << std::endl;

    for (int i = 1; i <= num_days; ++i) {
        std::string nazwa_pliku = "INT" + std::to_string(i) + ".TXT";

        try {
            const auto day_profile = loadDayProfile(nazwa_pliku);
            const auto [timeline, max_sim_time] = buildTimeline(service_times, day_profile);

            // Wywołanie uniwersalnego algorytmu dla pojedynczego dnia
            GnrResult gnr_dnia = findPeakHour(timeline, window_size, step_size);

            int godziny = static_cast<int>(gnr_dnia.window_start) / 3600;
            int minuty = (static_cast<int>(gnr_dnia.window_start) % 3600) / 60;

            std::cout << "Dzien " << i << " (" << nazwa_pliku << ") -> Szczyt od "
                      << (godziny < 10 ? "0" : "") << godziny << ":"
                      << (minuty < 10 ? "0" : "") << minuty << " do "
                      << std::setw(2) << std::setfill('0') << static_cast<int>(gnr_dnia.window_start + window_size) / 3600 << ":"
                      << std::setw(2) << std::setfill('0') << (static_cast<int>(gnr_dnia.window_start + window_size) % 3600) / 60
                      << " | Ruch: " << gnr_dnia.max_erlangs << " Erlangow" << std::endl;

            suma_maksymow_erlanga += gnr_dnia.max_erlangs;
            przetworzone_dni++;

        } catch (const std::exception& e) {
            std::cerr << "Ostrzezenie: Blad w " << nazwa_pliku << " (" << e.what() << ") - pomijam." << std::endl;
        }
    }

    if (przetworzone_dni == 0) {
        throw std::runtime_error("Nie udalo sie przetworzyc zadnego dnia dla algorytmu ADPQH!");
    }

    double adpqh_erlangs = suma_maksymow_erlanga / przetworzone_dni;

    // Zwracamy GnrResult.
    // window_start = -1.0 oznacza, że jest to wynik zagregowany, bez jednego punktu w czasie.
    return GnrResult(adpqh_erlangs, -1.0, window_size);
}
