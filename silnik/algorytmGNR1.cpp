//
// Created by DJMixu on 16.04.2026.
//

#include "algorytmGNR1.h"

#include <vector>

#include "models.h"

GnrResult findPeakHour(const std::vector<Call>& timeline, double max_sim_time) {
    GnrResult best;
    const double window_size = best.window_size;
    const double step_size   = 60.0 * 15;


    for (double w_start = 0; w_start <= max_sim_time - window_size; w_start += step_size) {
        const double w_end = w_start + window_size;
        double active_time = 0.0;

        for (const auto& call : timeline) {
            if (call.end_time   <= w_start) continue;
            if (call.start_time >= w_end)   break;

            active_time += std::min(w_end,   call.end_time)
                         - std::max(w_start, call.start_time);
        }

        const double erlangs = active_time / window_size;
        if (erlangs > best.max_erlangs) {
            best.max_erlangs  = erlangs;
            best.window_start = w_start;
        }
    }
    return best;
}
