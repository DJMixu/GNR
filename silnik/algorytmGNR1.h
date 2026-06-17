//
// Created by DJMixu on 16.04.2026.
//

#ifndef SILNIK_ALGORYTMGNR1_H
#define SILNIK_ALGORYTMGNR1_H
#pragma once
#include <vector>

#include "models.h"

/**
 * @brief Struktura przechowująca wyniki analizy Godziny Największego Ruchu (GNR).
 * Służy do zwracania i agregacji kompletnych informacji o odnalezionym szczytowym oknie czasowym.
 */
struct GnrResult {
    double max_erlangs = 0.0;     // Maksymalne wyliczone natężenie ruchu wyrażone w Erlangach
    double window_start = 0.0;    // Czas początku okna szczytowego w sekundach (liczony od początku doby)
    const double window_size;     // Zdefiniowana szerokość okna w sekundach (zabezpieczona modyfikatorem const)

    /**
     * @brief Konstruktor obiektu z listą inicjalizacyjną.
     * @param erlangs Obliczone natężenie ruchu (domyślnie 0.0).
     * @param start Punkt początkowy okna szczytowego w sekundach (domyślnie 0.0).
     * @param size Rozmiar okna czasowego (domyślnie 3600.0 s, czyli 1 godzina).
     */
    GnrResult(double erlangs = 0.0, double start = 0.0, double size = 3600.0)
        : max_erlangs(erlangs), window_start(start), window_size(size) {}
};
GnrResult findPeakHour(const std::vector<Call>& timeline, double window_size, double step_size);
#endif //SILNIK_ALGORYTMGNR1_H
