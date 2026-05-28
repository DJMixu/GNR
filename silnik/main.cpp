#include <vector>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>

#include "algorytmGNR1.h"
#include "algorytmGNR2.h"
#include "dataReader.h"
#include "dataWriter.h"
#include "supportFunctions.h"
#include "timeLineBuilder.h"
#include "generateDay.h"
#include "srDzien.h"

int main(int argc, char* argv[]) {
    int number_of_days = 7;
    bool force_overwrite = false;
    bool uruchom_tcbh = true;
    bool uruchom_adpqh = false;
    double window_size = 3600.0;
    double step_size = 900.0;

    // Parsowanie flag konsolowych
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            wyswietlInstrukcje(argv[0]); return 0;
        } else if ((arg == "-n" || arg == "--days") && i + 1 < argc) {
            number_of_days = std::stoi(argv[++i]);
        } else if (arg == "--force") {
            force_overwrite = true;
        } else if (arg == "--window" && i + 1 < argc) {
            window_size = std::stod(argv[++i]);
        } else if (arg == "--step" && i + 1 < argc) {
            step_size = std::stod(argv[++i]);
        } else if (arg == "--tcbh") {
            uruchom_tcbh = true; uruchom_adpqh = false;
        } else if (arg == "--adpqh") {
            uruchom_adpqh = true; uruchom_tcbh = false;
        }
    }

    try {
        const auto service_times = loadServiceTimes("CZAS.TXT");

        std::cout << "Sprawdzam dostepnosc plikow dni (Wymuszenie nadpisania: "
                  << (force_overwrite ? "TAK" : "NIE") << ")..." << std::endl;
        ensure_days_exist("INT.TXT", number_of_days, force_overwrite);

        // ZAWSZE generuj średni dzień dla Javy
        wygenerujSredniDzien(number_of_days, "INT_SR.TXT");

        if (uruchom_tcbh) {
            std::cout << "\n--- [METODA TCBH] ---" << std::endl;
            const auto day_profile_tcbh = loadDayProfile("INT_SR.TXT");
            const auto [timeline_tcbh, max_sim_time_tcbh] = buildTimeline(service_times, day_profile_tcbh);

            const GnrResult gnr_tcbh = findPeakHour(timeline_tcbh, window_size, step_size);
            exportGnrLines(timeline_tcbh, gnr_tcbh, "gnr_linie.txt"); // Zapisujemy linie dla TCBH
            std::vector<double> chart_data = generateChartErlangs(timeline_tcbh, step_size);
            // Dedkowany zapis dla Javy
            std::ofstream out("wyniki.txt");
            out << "TCBH\n" << gnr_tcbh.max_erlangs << "\n" << gnr_tcbh.window_start << "\nCHART";
            for(double val : chart_data) out << " " << val;
            out << "\n";
            out.close();

            std::cout << "3.1 TCBH ZAKONCZONE" << std::endl;

        } else if (uruchom_adpqh) {
            std::cout << "\n--- [METODA ADPQH] ---" << std::endl;
            // Wywołujemy z oknem i krokiem (wynik sam stworzy wyniki.txt)
            GnrResult wynik_adpqh = obliczADPQH(number_of_days, service_times, window_size, step_size);
            std::cout << "3.2 ADPQH ZAKONCZONE" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Blad krytyczny: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}