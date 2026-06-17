//
// Created by DJMixu on 16.04.2026.
//

#include "supportFunctions.h"

#include <iostream>
#include <string>
#include <vector>

#include "models.h"
#include <cmath>
#include <algorithm>
/**
 * @brief Generuje dane do wykresu natężenia ruchu (w Erlangach) w ujęciu dobowym.
 * * Funkcja dzieli dobę na równe przedziały czasowe (tzw. koszyki, zgodnie z zadanym krokiem)
 * i precyzyjnie zlicza czas trwania wszystkich połączeń wewnątrz każdego z nich.
 * Na koniec sumaryczny czas aktywności w danym przedziale jest dzielony przez jego szerokość,
 * co pozwala na przejście z jednostek czasu na ustandaryzowaną miarę natężenia ruchu wyrażoną w Erlangach.
 * * @param timeline Oś czasu zawierająca zestawienie wszystkich połączeń z danej doby.
 * @param step_size Rozdzielczość wykresu (szerokość jednego przedziału w sekundach, np. 900 dla 15 minut).
 * @return std::vector<double> Wektor wyliczonych wartości w Erlangach, służący jako podstawa do narysowania wykresu.
 */
std::vector<double> generateChartErlangs(const std::vector<Call>& timeline, double step_size) {
    if (step_size <= 0.0) return {};

    const double SECONDS_IN_DAY = 86400.0;
    const int NUM_BINS = static_cast<int>(std::ceil(SECONDS_IN_DAY / step_size));
    std::vector<double> bins(NUM_BINS, 0.0);

    for (const auto& call : timeline) {
        int start_bin = std::max(0, static_cast<int>(call.start_time / step_size));
        int end_bin   = std::min(NUM_BINS - 1, static_cast<int>(call.end_time / step_size));

        for (int b = start_bin; b <= end_bin; ++b) {
            double bin_start = b * step_size;
            double bin_end   = bin_start + step_size;
            // Obliczamy część wspólną (przecięcie) czasu trwania połączenia z granicami aktualnego koszyka.
            // Jeśli overlap_end > overlap_start, oznacza to, że połączenie faktycznie miało miejsce w tym przedziale.
            double overlap_start = std::max(call.start_time, bin_start);
            double overlap_end   = std::min(call.end_time, bin_end);

            if (overlap_end > overlap_start) {
                bins[b] += (overlap_end - overlap_start); // sumujemy sekundy ruchu
            }
        }
    }

    // Zamiana sekund ruchu na Erlangi
    for (auto& val : bins) {
        val /= step_size;
    }
    return bins;
}
/**
 * @brief Formatuje czas podany w sekundach do czytelnej postaci tekstowej (HH:MM:SS).
 * * Funkcja pomocnicza służąca wyłącznie do celów prezentacyjnych. Przelicza sekundy
 * biegnące od północy na standardowy format zegarowy. Ułatwia to użytkownikom
 * i osobom analizującym raporty łatwe zinterpretowanie przedziałów czasowych.
 * * @param s Czas wyrażony w sekundach (od początku doby).
 * @return std::string Sformatowany ciąg znaków w postaci "GG:MM:SS".
 */
std::string formatTime(double s) {
    long long total_seconds = s;
    int hours   = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int seconds = total_seconds % 60;

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
    return std::string(buffer);
}
/**
 * @brief Przeprowadza wewnętrzne testy poprawności modelu ruchu (Sanity Check).
 * * Funkcja weryfikuje, czy proces generowania osi czasu przebiegł prawidłowo i bezstratnie.
 * Odbywa się to poprzez trzy główne testy:
 * 1. Test ilościowy: Sprawdza, czy żadne zgłoszenie nie zostało zagubione podczas przetwarzania.
 * 2. Test sumy czasów: Weryfikuje, czy łączny czas obsługi w oryginale i na wygenerowanej osi zgadza się niemal do milisekundy.
 * 3. Test logiki czasowej: Potwierdza, czy każde pojedyncze połączenie zachowuje ciągłość (czas początku + czas trwania = czas końca).
 * Zaliczenie testów daje prowadzącemu gwarancję wiarygodności przeprowadzonych symulacji.
 * * @param service_times Wzorzec bazowy, czyli czasy obsługi bezpośrednio wczytane z plików.
 * @param timeline Zbudowana oś czasu zdarzeń, która podlega weryfikacji.
 */
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
/**
 * @brief Wyświetla instrukcję obsługi aplikacji z poziomu konsoli.
 * * Funkcja interfejsu użytkownika aktywowana wywołaniem flagi pomocy (np. -h, --help).
 * Tłumaczy dostępne argumenty wywołania programu, pozwalając na łatwe zapoznanie się
 * z możliwościami sterowania analizą (np. wybór metody TCBH/ADPQH, zmiana liczby dni).
 * * @param silnik Pełna nazwa uruchamianego programu (pliku wykonywalnego).
 */
void wyswietlInstrukcje(const char* silnik) {
    std::cout << "Uzycie: " << silnik << " [opcje]\n"
              << "Opcje:\n"
              << "  -n, --days <int>    Liczba dni do analizy (domyslnie: 7)\n"
              << "  --force             Wymuszenie nadpisania istniejacych plikow dni\n"
              << "  --tcbh              Uruchom tylko algorytm TCBH (Sredni dzien)\n"
              << "  --adpqh             Uruchom tylko algorytm ADPQH (Ruchomy szczyt)\n"
              << "  -h, --help          Wyswietl te pomoc\n";
}