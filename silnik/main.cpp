// #include <iostream>
// #include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <iostream>

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
    bool uruchom_adpqh = true;
    bool tylko_jeden_wybrany = false;

    // Parsowanie flag konsolowych
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            wyswietlInstrukcje(argv[0]);
            return 0;
        } else if ((arg == "-n" || arg == "--days") && i + 1 < argc) {
            number_of_days = std::stoi(argv[++i]);
        } else if (arg == "--force") {
            force_overwrite = true;
        } else if (arg == "--tcbh") {
            uruchom_tcbh = true;
            if (!tylko_jeden_wybrany) { uruchom_adpqh = false; tylko_jeden_wybrany = true; }
        } else if (arg == "--adpqh") {
            uruchom_adpqh = true;
            if (!tylko_jeden_wybrany) { uruchom_tcbh = false; tylko_jeden_wybrany = true; }
        } else {
            std::cerr << "Nieznany argument: " << arg << "\n";
            wyswietlInstrukcje(argv[0]);
            return 1;
        }
    }

    // Korekta w przypadku podania obu flag naraz
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--tcbh") uruchom_tcbh = true;
        if (arg == "--adpqh") uruchom_adpqh = true;
    }

    try {
        // KROK 1: Ładowanie podstawowych czasów obsługi
        const auto service_times = loadServiceTimes("CZAS.TXT");

        // KROK 2: Walidacja obecności plików na dysku z użyciem naszej flagi
        std::cout << "Sprawdzam dostepnosc plikow dni (Wymuszenie nadpisania: "
                  << (force_overwrite ? "TAK" : "NIE") << ")..." << std::endl;
        ensure_days_exist("INT.TXT", number_of_days, force_overwrite);

        double wynik_adpqh = 0.0;

        // KROK 3: OBLICZENIA METODĄ 3.1: TCBH
        if (uruchom_tcbh) {
            std::cout << "\n--- [METODA TCBH] ---" << std::endl;
            wygenerujSredniDzien(number_of_days, "INT_SR.TXT");

            const auto day_profile_tcbh = loadDayProfile("INT_SR.TXT");
            std::cout << "Odbudowa osi czasu dla profilu sredniego..." << std::endl;
            const auto [timeline_tcbh, max_sim_time_tcbh] = buildTimeline(service_times, day_profile_tcbh);

            // Inicjalizacja bezpośrednia (naprawiony błąd "use of deleted function")
            const GnrResult gnr_tcbh = findPeakHour(timeline_tcbh, max_sim_time_tcbh);

            runDiagnostics(service_times, timeline_tcbh);
            exportGnrLines(timeline_tcbh, gnr_tcbh, "gnr_linie.txt");

            std::cout << "\n=====================================" << std::endl;
            std::cout << "========= WYNIK METODY TCBH =========" << std::endl;
            std::cout << "=====================================" << std::endl;
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "3.1 TCBH  (Stala godzina szczytu): " << gnr_tcbh.max_erlangs << " Erlangow" << std::endl;
            std::cout << "    -> W oknie od: " << formatTime(gnr_tcbh.window_start)
                      << " do: " << formatTime(gnr_tcbh.window_start + gnr_tcbh.window_size) << std::endl;
        } else {
            std::cout << "\n3.1 TCBH  (Stala godzina szczytu): POMINIETO" << std::endl;
        }

        // KROK 4: OBLICZENIA METODĄ 3.2: ADPQH
        if (uruchom_adpqh) {
            std::cout << "\n--- [METODA ADPQH] ---" << std::endl;
            wynik_adpqh = obliczADPQH(number_of_days, service_times);

            std::cout << "\n=====================================" << std::endl;
            std::cout << "========= WYNIK METODY ADPQH ========" << std::endl;
            std::cout << "=====================================" << std::endl;
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "3.2 ADPQH (Ruchomy szczyt 15-min): " << wynik_adpqh << " Erlangow" << std::endl;
            std::cout << "=====================================" << std::endl;
        } else {
            std::cout << "\n3.2 ADPQH (Ruchomy szczyt 15-min): POMINIETO" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Blad krytyczny: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}