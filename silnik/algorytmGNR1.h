//
// Created by DJMixu on 16.04.2026.
//

#ifndef SILNIK_ALGORYTMGNR1_H
#define SILNIK_ALGORYTMGNR1_H
#pragma once
#include <vector>

#include "models.h"

struct GnrResult {
    double max_erlangs  = 0.0;
    double window_start = 0.0;
    const double window_size  = 3600.0;
};
GnrResult findPeakHour(const std::vector<Call>& timeline, double max_sim_time);
#endif //SILNIK_ALGORYTMGNR1_H
