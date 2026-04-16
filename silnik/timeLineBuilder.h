//
// Created by DJMixu on 16.04.2026.
//

#ifndef SILNIK_TIMELINEBUILDER_H
#define SILNIK_TIMELINEBUILDER_H
#include <vector>
#pragma once
#include "models.h"

struct TimelineResult {
    std::vector<Call> calls;
    double max_sim_time = 0.0;
};

TimelineResult buildTimeline(const std::vector<double>& service_times,
                              const std::vector<MinuteStat>& day_profile);
#endif //SILNIK_TIMELINEBUILDER_H
