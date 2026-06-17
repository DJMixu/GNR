//
// Created by DJMixu on 16.04.2026.
//

#ifndef SILNIK_TIMELINEBUILDER_H
#define SILNIK_TIMELINEBUILDER_H
#include <vector>
#pragma once
#include "models.h"

/**
 * @brief Struktura grupująca wyniki procesu budowania osi czasu (symulacji dnia).
 * Przechowuje pełną listę wygenerowanych zdarzeń oraz końcowy czas trwania całej symulacji.
 */
struct TimelineResult {
    std::vector<Call> calls;   // Zbiór wszystkich połączeń umiejscowionych chronologicznie na osi czasu
    double max_sim_time = 0.0; // Najpóźniejszy odnotowany czas zakończenia połączenia (koniec symulacji w sekundach)
};

TimelineResult buildTimeline(const std::vector<double>& service_times,
                              const std::vector<MinuteStat>& day_profile);

#endif //SILNIK_TIMELINEBUILDER_H
