#include "algorytmGNR2.h"
#include "algorytmGNR1.h"
#include "dataWriter.h"
#include "dataReader.h"
#include "models.h"
#include "timeLineBuilder.h"
#include <iomanip>
#include <iostream>
#include <fstream>

#include "supportFunctions.h"
/**
 * @brief Główna metoda obliczająca uśrednione natężenie ruchu w godzinie największego ruchu (ADPQH).
 * * Funkcja przeprowadza analizę danych ruchowych z zadanego okresu pomiarowego.
 * Logika działania dla każdego dnia:
 * 1. Wczytuje dobowy profil ruchu z plików wejściowych (np. INT1.TXT, INT2.TXT).
 * 2. Odtwarza oś czasu zdarzeń na podstawie podanych czasów obsługi.
 * 3. Wyznacza Godzinę Największego Ruchu (GNR) korzystając z algorytmu przesuwnego okna czasowego.
 * 4. Generuje i zapisuje dane pomocnicze (wykresy w Erlangach, pliki z liniami) w celach raportowych.
 * * Po przetworzeniu wszystkich poprawnych plików, funkcja oblicza końcowy wynik ADPQH
 * jako średnią arytmetyczną z maksymalnych natężeń ruchu (w Erlangach) z poszczególnych dni,
 * a następnie zapisuje podsumowanie do pliku "wyniki.txt".
 * * @param num_days Liczba dni (plików) przewidzianych do analizy.
 * @param service_times Wektor reprezentujący czasy obsługi zgłoszeń.
 * @param window_size Szerokość okna czasowego używanego do znalezienia szczytu ruchu.
 * @param step_size Rozdzielczość (krok) przesuwania okna analitycznego.
 * @return GnrResult Obiekt zawierający końcową wartość ADPQH (uśrednione Erlangi z całego okresu).
 */
GnrResult obliczADPQH(int num_days, const std::vector<double>& service_times, double window_size, double step_size) {
    double suma_maksymow_erlanga = 0.0;
    int przetworzone_dni = 0;

    std::ofstream out("wyniki.txt");
    out << "ADPQH\n";

    for (int i = 1; i <= num_days; ++i) {
        std::string nazwa_pliku = "INT" + std::to_string(i) + ".TXT";

        try {
            const auto day_profile = loadDayProfile(nazwa_pliku);
            const auto [timeline, max_sim_time] = buildTimeline(service_times, day_profile);

            GnrResult gnr_dnia = findPeakHour(timeline, window_size, step_size);

            // NOWOŚĆ: Eksportujemy linie dla każdego dnia osobno!
            std::string nazwa_wyjscia_linii = "gnr_linie_" + std::to_string(i) + ".txt";
            exportGnrLines(timeline, gnr_dnia, nazwa_wyjscia_linii);
            std::vector<double> chart_data = generateChartErlangs(timeline, step_size);

            out << "DZIEN " << i << " " << gnr_dnia.max_erlangs << " " << gnr_dnia.window_start << "\nCHART";
            for(double val : chart_data) out << " " << val;
            out << "\n";

            suma_maksymow_erlanga += gnr_dnia.max_erlangs;
            przetworzone_dni++;

        } catch (const std::exception& e) {
            std::cerr << "Blad w " << nazwa_pliku << " - " << e.what() << std::endl;
        }
    }

    double adpqh_erlangs = suma_maksymow_erlanga / przetworzone_dni;
    out << "SREDNIA " << adpqh_erlangs << "\n";
    out.close();

    return GnrResult(adpqh_erlangs, -1.0, window_size);
}
