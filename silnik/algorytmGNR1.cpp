//
// Created by DJMixu on 16.04.2026.
//

#include "algorytmGNR1.h"

#include <cmath>
#include <iostream>
#include <vector>

#include "models.h"

GnrResult findPeakHour (const std::vector<Call>& timeline, double window_size, double step_size) {
    // Podstawowe zabezpieczenie przed niepoprawnymi parametrami
    if (step_size <= 0.0 || window_size <= 0.0 || window_size < step_size) {
        std::cerr << "Blad: Niepoprawne parametry okna lub kroku!" << std::endl;
        return GnrResult(0.0, 0.0, window_size);
    }

    const double SECONDS_IN_DAY = 86400.0;

    // Obliczamy dynamicznie liczbę koszyków dla całej doby oraz dla jednego okna
    const int NUM_BINS = static_cast<int>(std::ceil(SECONDS_IN_DAY / step_size));
    const int BINS_IN_WINDOW = static_cast<int>(std::round(window_size / step_size));

    // Tworzymy wektor koszyków o dynamicznym rozmiarze (zamiast std::array)
    std::vector<double> bins(NUM_BINS, 0.0);

    // --- KROK 1: Wypełnianie koszyków z zadaną dokładnością (step_size) ---
    for (const auto& call : timeline) {
        int start_bin = std::max(0, static_cast<int>(call.start_time / step_size));
        int end_bin   = std::min(NUM_BINS - 1, static_cast<int>(call.end_time / step_size));

        for (int b = start_bin; b <= end_bin; ++b) {
            double bin_start = b * step_size;
            double bin_end   = bin_start + step_size;

            double overlap_start = std::max(call.start_time, bin_start);
            double overlap_end   = std::min(call.end_time, bin_end);

            if (overlap_end > overlap_start) {
                bins[b] += (overlap_end - overlap_start);
            }
        }
    }

    // --- KROK 2: Przesuwne okno grupujące koszyki ---
    double max_active_time = 0.0;
    int best_start_bin = 0;

    // Iterujemy po koszykach tak, aby zmieściło się w nich całe okno (BINS_IN_WINDOW)
    for (int i = 0; i <= NUM_BINS - BINS_IN_WINDOW; ++i) {
        double current_sum = 0.0;

        // Sumujemy ruch ze wszystkich koszyków należących do bieżącego okna
        for (int j = 0; j < BINS_IN_WINDOW; ++j) {
            current_sum += bins[i + j];
        }

        if (current_sum > max_active_time) {
            max_active_time = current_sum;
            best_start_bin = i;
        }
    }

    // --- KROK 3: Wyliczenie wyników i stworzenie obiektu GnrResult ---
    double max_erlangs = max_active_time / window_size;
    double window_start_time = best_start_bin * step_size;

    return GnrResult(max_erlangs, window_start_time, window_size);
}
