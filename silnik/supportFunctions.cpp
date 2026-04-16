//
// Created by DJMixu on 16.04.2026.
//

#include "supportFunctions.h"

#include <iostream>
#include <string>
#include <vector>

#include "models.h"

std::string formatTime(double s) {
    long long total_seconds = s;
    int hours   = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int seconds = total_seconds % 60;

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
    return std::string(buffer);
}

void runDiagnostics(const std::vector<double>& service_times,
                    const std::vector<Call>& timeline)
{
    std::cout << "\n=== RAPORT DIAGNOSTYCZNY (SANITY CHECK) ===" << std::endl;

    std::cout << "TEST 1 (Ilosc zapytan): ";
    if (timeline.size() == service_times.size()) {
        std::cout << "ZALICZONY [" << timeline.size() << " / "
                  << service_times.size() << "]" << std::endl;
    } else {
        std::cout << "OBLANY [Rozbieznosc! Plik ma " << service_times.size()
                  << ", a os czasu " << timeline.size() << "]" << std::endl;
    }

    double sum_original = 0.0, sum_timeline = 0.0;
    for (double d : service_times) sum_original += d;
    for (const auto& c : timeline) sum_timeline += c.duration;

    std::cout << "TEST 2 (Konsystencja czasow): ";
    if (std::abs(sum_original - sum_timeline) < 0.1)
        std::cout << "ZALICZONY [Wszystkie milisekundy zachowane]" << std::endl;
    else
        std::cout << "OBLANY [Suma w pliku: " << sum_original
                  << ", Suma na osi: " << sum_timeline << "]" << std::endl;

    bool logic_ok = true;
    for (const auto& c : timeline)
        if (std::abs((c.start_time + c.duration) - c.end_time) > 0.001) {
            logic_ok = false; break;
        }

    std::cout << "TEST 3 (Logika wektorow czasu): ";
    std::cout << (logic_ok ? "ZALICZONY [Brak anomalii]"
                           : "OBLANY [Wykryto zalamanie czaso-przestrzeni :)]")
              << std::endl;
    std::cout << "===========================================\n" << std::endl;
}
