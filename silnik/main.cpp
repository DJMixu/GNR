#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <iomanip>

// Struktura opisująca pojedyncze wywołanie na naszej nowej osi czasu
struct Call {
    double start_time;
    double duration;
    double end_time;
};

// Funkcja pomocnicza: zamienia milisekundy na czytelny zegar (HH:MM:SS)
std::string formatTime(double ms) {
    long long total_seconds = ms / 1000;
    int hours = ((total_seconds / 3600) + 8) % 24;
    int minutes = (total_seconds % 3600) / 60;
    int seconds = total_seconds % 60;

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
    return std::string(buffer);
}

int main() {
    std::ifstream fTime("CZAS.TXT");         // Plik 1: 34953 czasów obsługi
    std::ifstream fHist("INT.TXT"); // Plik 2: 1020 wierszy histogramu odstępów

    if (!fTime.is_open() || !fHist.is_open()) {
        std::cerr << "Blad: Nie mozna otworzyc plikow." << std::endl;
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

    std::cout << "Trwa odbudowa osi czasu (Monte Carlo)..." << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> dist(probabilities.begin(), probabilities.end());

    std::vector<Call> timeline;
    double current_time = 0.0;

    for (size_t i = 0; i < service_times.size(); ++i) {
        int idx = dist(gen);
        double interval = inter_arrival_values[idx];

        current_time += interval;

        Call c;
        c.start_time = current_time;
        c.duration = service_times[i];
        c.end_time = current_time + service_times[i];

        timeline.push_back(c);
    }

    std::cout << "Odbudowano " << timeline.size() << " zapytan." << std::endl;
    std::cout << "Zasymulowany czas trwania pomiaru: " << formatTime(current_time) << "\n" << std::endl;

    std::cout << "Szukam Godziny Najwiekszego Ruchu (GNR)..." << std::endl;

    // USTAWIENIA DLA MILISEKUND
    double window_size = 3600000.0;  // 1 godzina = 3 600 000 ms
    double step_size = 60000.0;      // Przesunięcie okna co 1 minutę = 60 000 ms

    double max_erlangs = 0.0;
    double best_gnr_start = 0.0;

    for (double w_start = 0; w_start <= current_time - window_size; w_start += step_size) {
        double w_end = w_start + window_size;
        double active_time_in_window = 0.0;

        for (const auto& call : timeline) {
            if (call.end_time <= w_start || call.start_time >= w_end) {
                continue;
            }

            double overlap_start = std::max(w_start, call.start_time);
            double overlap_end = std::min(w_end, call.end_time);
            active_time_in_window += (overlap_end - overlap_start);
        }

        // Ruch (Erlangi) obliczamy nadal tak samo: zajętość / szerokość okna
        double erlangs = active_time_in_window / window_size;

        if (erlangs > max_erlangs) {
            max_erlangs = erlangs;
            best_gnr_start = w_start;
        }
    }

    std::cout << "=== WYNIKI GNR ===" << std::endl;
    std::cout << "Najwiekszy ruch zanotowano w oknie:" << std::endl;
    std::cout << "Od: " << formatTime(best_gnr_start) << " do: " << formatTime(best_gnr_start + window_size) << std::endl;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Obciazenie w Godzinie Szczytu (GNR): " << max_erlangs << " Erlangow" << std::endl;
    std::cout << "==================" << std::endl;

    return 0;
}