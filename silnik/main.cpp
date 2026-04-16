#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include "algorytmGNR1.h"
#include "dataReader.h"
#include "dataWriter.h"
#include "supportFunctions.h"
#include "timeLineBuilder.h"


int main() {
    try {
        const auto service_times = loadServiceTimes("CZAS.TXT");
        const auto day_profile   = loadDayProfile("INT.TXT");

        std::cout << "Trwa odbudowa osi czasu na podstawie profilu dobowego..." << std::endl;
        const auto [timeline, max_sim_time] = buildTimeline(service_times, day_profile);
        std::cout << "Odbudowano " << timeline.size() << " zapytan." << std::endl;
        std::cout << "Koniec ostatniego polaczenia: " << formatTime(max_sim_time) << "\n" << std::endl;

        std::cout << "Szukam Godziny Najwiekszego Ruchu (GNR)..." << std::endl;
        const auto gnr = findPeakHour(timeline, max_sim_time);

        std::cout << "=== WYNIKI GNR ===" << std::endl;
        std::cout << "Od: " << formatTime(gnr.window_start)
                  << " do: " << formatTime(gnr.window_start + gnr.window_size) << std::endl;
        std::cout << std::fixed << std::setprecision(0);
        std::cout << "Obciazenie w GNR: " << gnr.max_erlangs << " Erlangow" << std::endl;
        std::cout << "==================" << std::endl;

        runDiagnostics(service_times, timeline);
        exportGnrLines(timeline, gnr, "gnr_linie.txt");

    } catch (const std::exception& e) {
        std::cerr << "Blad: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}