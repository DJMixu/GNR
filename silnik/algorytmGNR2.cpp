//
// Created by foxyu on 27.05.2026.
//

#include "algorytmGNR2.h"



double obliczADPQH(int num_days, const std::vector<double>& service_times) {
    double suma_maksymow_erlanga = 0.0;
    int przetworzone_dni = 0;

    std::cout << "\n=== ROZPOCZYNAM OBLICZENIA ADPQH ===" << std::endl;

    for (int i = 1; i <= num_days; ++i) {
        std::string nazwa_pliku = "INT" + std::to_string(i) + ".TXT";

        try {
            const auto day_profile = loadDayProfile(nazwa_pliku);
            const auto [timeline, max_sim_time] = buildTimeline(service_times, day_profile);
            GnrResult gnr_dnia = findPeakHour(timeline, max_sim_time);

            std::cout << "Dzien " << i << " (" << nazwa_pliku << ") -> Maksymalny kwadrans: "
                      << gnr_dnia.max_erlangs << " Erlangow" << std::endl;

            suma_maksymow_erlanga += gnr_dnia.max_erlangs;
            przetworzone_dni++;

        } catch (const std::exception& e) {
            std::cerr << "Ostrzezenie: Blad w " << nazwa_pliku << " (" << e.what() << ") - pomijam." << std::endl;
        }
    }

    if (przetworzone_dni == 0) {
        throw std::runtime_error("Nie udalo sie przetworzyc zadnego dnia dla algorytmu ADPQH!");
    }

    return suma_maksymow_erlanga / przetworzone_dni;
}
