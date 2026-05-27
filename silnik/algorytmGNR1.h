//
// Created by DJMixu on 16.04.2026.
//

#ifndef SILNIK_ALGORYTMGNR1_H
#define SILNIK_ALGORYTMGNR1_H
#pragma once
#include <vector>

#include "models.h"

struct GnrResult {
    double max_erlangs = 0.0;
    double window_start = 0.0;
    const double window_size;

    // Konstruktor pozwalający na dynamiczną inicjalizację stałej
    GnrResult(double erlangs = 0.0, double start = 0.0, double size = 3600.0)
        : max_erlangs(erlangs), window_start(start), window_size(size) {}
};
GnrResult findPeakHour(const std::vector<Call>& timeline, double window_size, double step_size);
#endif //SILNIK_ALGORYTMGNR1_H
