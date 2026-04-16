//
// Created by DJMixu on 16.04.2026.
//

#include "timeLineBuilder.h"

#include <cmath>
#include <vector>

#include "models.h"

TimelineResult buildTimeline(const std::vector<double>& service_times,
                              const std::vector<MinuteStat>& day_profile)
{
    double total_lambda = 0.0;
    for (const auto& ms : day_profile) total_lambda += ms.lambda;

    const double scale = service_times.size() / total_lambda;

    TimelineResult result;
    size_t current_idx = 0;
    for (size_t i = 0; i < day_profile.size(); ++i) {
        int num_calls = static_cast<int>(std::round(day_profile[i].lambda * scale));

        if (current_idx + num_calls > service_times.size())
            num_calls = service_times.size() - current_idx;

        if (i == day_profile.size() - 1 && current_idx < service_times.size())
            num_calls = service_times.size() - current_idx;

        if (num_calls <= 0) continue;

        const double minute_start_s = day_profile[i].minute * 60.0;
        const double interval_s     = 60.0 / num_calls;

        for (int c = 0; c < num_calls; ++c) {
            Call call;
            call.start_time = minute_start_s + (c * interval_s);
            call.duration   = service_times[current_idx++];
            call.end_time   = call.start_time + call.duration;

            result.calls.push_back(call);

            if (call.end_time > result.max_sim_time)
                result.max_sim_time = call.end_time;
        }
    }
    return result;
}
