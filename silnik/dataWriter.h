//
// Created by DJMixu on 16.04.2026.
//

#ifndef SILNIK_DATAWRITER_H
#define SILNIK_DATAWRITER_H
#include <string>
#include <vector>
#pragma once
#include "algorytmGNR1.h"
#include "models.h"

void exportGnrLines(const std::vector<Call>& timeline,
                    const GnrResult& gnr,
                    const std::string& filename);
#endif //SILNIK_DATAWRITER_H
