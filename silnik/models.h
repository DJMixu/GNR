//
// Created by DJMixu on 16.04.2026.
//

#ifndef SILNIK_MODELS_H
#define SILNIK_MODELS_H
#pragma once
struct Call {
    double start_time;
    double duration;
    double end_time;
};

struct MinuteStat {
    int minute;
    double lambda;
};
#endif //SILNIK_MODELS_H
