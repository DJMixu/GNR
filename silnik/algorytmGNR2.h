//
// Created by foxyu on 27.05.2026.
//

#ifndef SILNIK_ALGORYTMGNR2_H
#define SILNIK_ALGORYTMGNR2_H
#pragma once
#include <vector>

#include "algorytmGNR1.h"
#include "algorytmGNR2.h"


GnrResult obliczADPQH(int num_days,
                      const std::vector<double>& service_times,
                      double window_size = 3600.0,
                      double step_size = 900.0);

#endif //SILNIK_ALGORYTMGNR2_H
