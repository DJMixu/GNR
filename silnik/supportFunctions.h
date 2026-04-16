//
// Created by DJMixu on 16.04.2026.
//

#ifndef SILNIK_SUPPORTFUNCTIONS_H
#define SILNIK_SUPPORTFUNCTIONS_H
#include <string>
#include <vector>
#pragma once
#include "models.h"

std::string formatTime(double s);
void runDiagnostics(const std::vector<double>& service_times,
                    const std::vector<Call>& timeline);
#endif //SILNIK_SUPPORTFUNCTIONS_H
