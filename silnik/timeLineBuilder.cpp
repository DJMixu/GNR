//
// Created by DJMixu on 16.04.2026.
//

#include "timeLineBuilder.h"

#include <cmath>
#include <vector>

#include "models.h"
/**
 * @brief Buduje chronologiczną oś czasu (harmonogram) połączeń dla pojedynczego dnia.
 * * Funkcja integruje surowe czasy obsługi z dobowym profilem ruchu. Logika działania:
 * 1. Oblicza współczynnik skalowania, aby dopasować łączną liczbę dostępnych czasów obsługi do sumarycznego natężenia ruchu w profilu.
 * 2. Dla każdej minuty doby wylicza, ile zgłoszeń powinno w niej wystąpić (proporcjonalnie do jej wagi, tzw. lambdy).
 * 3. Równomiernie rozkłada wyliczoną liczbę zgłoszeń na przestrzeni 60 sekund wewnątrz danej minuty.
 * 4. Każdemu zgłoszeniu przypisuje dokładny czas rozpoczęcia, pobiera kolejny czas trwania z puli i wyznacza czas jego zakończenia.
 * Algorytm posiada zabezpieczenia brzegowe – dba o to, aby nie przekroczyć rozmiaru tablicy i gwarantuje, że wszystkie wczytane czasy obsługi zostaną w pełni wykorzystane (korekta w ostatniej iteracji).
 * * @param service_times Zbiór dostępnych czasów obsługi, które po kolei są przypisywane do generowanych zgłoszeń.
 * @param day_profile Wektor ze statystykami, definiujący kształt rozkładu obciążenia w trakcie trwania całej doby.
 * @return TimelineResult Obiekt agregujący gotową listę połączeń umiejscowionych na osi czasu oraz maksymalny, końcowy czas symulacji.
 */
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
        // Jeśli w minucie mamy np. 4 zgłoszenia, dzielimy 60 sekund przez 4,
        // co daje interwał 15 sekund między kolejnymi połączeniami.
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
