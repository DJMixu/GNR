//
// Created by DJMixu on 16.04.2026.
//

#ifndef SILNIK_DATAREADER_H
#define SILNIK_DATAREADER_H
#include <string>
#include <vector>
#include "models.h"
#pragma once
std::vector<MinuteStat> loadDayProfile(const std::string& filename);
std::vector<double> loadServiceTimes(const std::string& filename);

#endif //SILNIK_DATAREADER_H
