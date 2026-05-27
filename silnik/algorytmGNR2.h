//
// Created by foxyu on 27.05.2026.
//

#ifndef SILNIK_ALGORYTMGNR2_H
#define SILNIK_ALGORYTMGNR2_H
#pragma once
#include <string>
#include <vector>
#include "algorytmGNR1.h"
#include <iostream>
#include "dataWriter.h"
#include "dataReader.h"
#include "timeLineBuilder.h"

double obliczADPQH(int num_days, const std::vector<double>& service_times);

#endif //SILNIK_ALGORYTMGNR2_H
