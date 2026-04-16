#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <iomanip>
#include <cmath>
#include <chrono>

// Struktura opisująca pojedyncze wywołanie na osi czasu
struct Call {
    double start_time;
    double duration;
    double end_time;
};

// Funkcja zamieniająca milisekundy na czytelny zegar (HH:MM:SS)
std::string formatTime(double ms) {
    long long total_seconds = ms / 1000;
    int hours = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int seconds = total_seconds % 60;

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
    return std::string(buffer);
}

int main() {
    // KROK 1: Wczytanie plików wejściowych
    std::ifstream fTime("CZAS.TXT");         // Plik 1: 34953 czasów obsługi
    std::ifstream fHist("INT.TXT"); // Plik 2: 1020 wierszy histogramu odstępów

    if (!fTime.is_open() || !fHist.is_open()) {
        std::cerr << "Blad: Nie mozna otworzyc plikow txt." << std::endl;
        return 1;
    }

    std::vector<double> service_times;
    std::string line;
    while (fTime >> line) {
        std::replace(line.begin(), line.end(), ',', '.');
        service_times.push_back(std::stod(line));
    }

    std::vector<double> inter_arrival_values;
    std::vector<double> probabilities;
    std::string valStr, probStr;

    while (fHist >> valStr >> probStr) {
        std::replace(valStr.begin(), valStr.end(), ',', '.');
        std::replace(probStr.begin(), probStr.end(), ',', '.');
        inter_arrival_values.push_back(std::stod(valStr));
        probabilities.push_back(std::stod(probStr));
    }

    // KROK 2: Przygotowanie i tasowanie TYLKO odstępów (przerw)
    std::cout << "Trwa odbudowa osi czasu..." << std::endl;

    double sum_probs = 0.0;
    for(double p : probabilities) {
        sum_probs += p;
    }

    std::vector<double> exact_intervals;
    exact_intervals.reserve(service_times.size());

    // Obliczamy dokładną liczbę wystąpień dla każdej przerwy
    for(size_t i = 0; i < inter_arrival_values.size(); ++i) {
            int count = std::max(1, static_cast<int>(std::round((probabilities[i] / sum_probs) * service_times.size())));        for(int c = 0; c < count; ++c) {
            exact_intervals.push_back(inter_arrival_values[i]);
        }
    }

    // Wyrównanie zaokrągleń
    while(exact_intervals.size() > service_times.size()) exact_intervals.pop_back();
    while(exact_intervals.size() < service_times.size()) exact_intervals.push_back(inter_arrival_values[0]);

    // Tasujemy TYLKO odstępy (używamy czasu systemowego dla prawdziwej losowości)
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::shuffle(exact_intervals.begin(), exact_intervals.end(), gen);

    // UWAGA: Nie tasujemy 'service_times', bo plik ma naturalną, chronologiczną kolejność!

    // KROK 3: Mapowanie czasów i budowa ostatecznej linii zdarzeń
    std::vector<Call> timeline;
    double current_time = 0.0;

    for (size_t i = 0; i < service_times.size(); ++i) {
        current_time += exact_intervals[i]; // Używamy potasowanego odstępu

        Call c;
        c.start_time = current_time;
        c.duration = service_times[i]; // Używamy oryginalnej kolejności pobierania
        c.end_time = current_time + service_times[i];

        timeline.push_back(c);
    }

    std::cout << "Odbudowano " << timeline.size() << " zapytan." << std::endl;
    std::cout << "GWARANTOWANY zasymulowany czas trwania pomiaru: " << formatTime(current_time) << "\n" << std::endl;

    // KROK 4: Algorytm Okna Przesuwnego (Szukanie GNR)
    std::cout << "Szukam Godziny Najwiekszego Ruchu (GNR)..." << std::endl;

    double window_size = 3600000.0;  // 1 godzina = 3 600 000 ms
    double step_size = 60000.0 * 15;      // Przesunięcie o 1 minutę = 60 000 ms

    double max_erlangs = 0.0;
    double best_gnr_start = 0.0;

    for (double w_start = 0; w_start <= current_time - window_size; w_start += step_size) {
        double w_end = w_start + window_size;
        double active_time_in_window = 0.0;

        for (const auto& call : timeline) {
            // Przerwij sprawdzanie wczesnych połączeń, które minęły
            if (call.end_time <= w_start) continue;
            // Przerwij pętlę, jeśli kolejne połączenia są już za oknem (optymalizacja dla posortowanej osi czasu)
            if (call.start_time >= w_end) break;

            double overlap_start = std::max(w_start, call.start_time);
            double overlap_end = std::min(w_end, call.end_time);
            active_time_in_window += (overlap_end - overlap_start);
        }

        double erlangs = active_time_in_window / window_size;

        if (erlangs > max_erlangs) {
            max_erlangs = erlangs;
            best_gnr_start = w_start;
        }
    }

    // KROK 5: Prezentacja wyników
    std::cout << "=== WYNIKI GNR ===" << std::endl;
    std::cout << "Najwiekszy ruch zanotowano w oknie:" << std::endl;
    std::cout << "Od: " << formatTime(best_gnr_start) << " do: " << formatTime(best_gnr_start + window_size) << std::endl;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Obciazenie w Godzinie Szczytu (GNR): " << max_erlangs << " Erlangow" << std::endl;
    std::cout << "==================" << std::endl;
    // KROK 6: Zrzut linii GNR do osobnego pliku
    std::ofstream fOut("gnr_linie.txt");
    if (fOut.is_open()) {
        fOut << "Wywołania z pliku czas.txt należące do GNR\n";
        fOut << "Okno: " << formatTime(best_gnr_start) << " - " << formatTime(best_gnr_start + window_size) << "\n";
        fOut << "Ruch: " << max_erlangs << " Erlangow\n";
        fOut << "--------------------------------------------------\n";

        int line_number = 1;
        int count_in_gnr = 0;

        for (const auto& call : timeline) {
            // Sprawdzamy, czy połączenie "zahaczyło" o nasze okno GNR
            if (call.end_time > best_gnr_start && call.start_time < best_gnr_start + window_size) {
                fOut << "Linia w pliku: " << line_number << " \t| Czas obsługi: " << call.duration << " ms\n";
                count_in_gnr++;
            }
            line_number++;
        }
        fOut.close();
        std::cout << "\nZnaleziono " << count_in_gnr << " zapytań tworzących GNR." << std::endl;
        std::cout << "Zapisano dokladne numery linii do pliku: gnr_linie.txt" << std::endl;
    } else {
        std::cerr << "Nie udalo sie utworzyc pliku gnr_linie.txt" << std::endl;
    }
    return 0;
}