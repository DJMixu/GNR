#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <cmath>

// Struktura opisująca pojedyncze wywołanie na osi czasu
struct Call {
    double start_time;
    double duration;
    double end_time;
};

// Struktura na dane z profilu dobowego (z pliku INT.TXT)
struct MinuteStat {
    int minute;
    double lambda;
};

// Funkcja zamieniająca milisekundy na czytelny zegar (HH:MM:SS)
std::string formatTime(double ms) {
    long long total_seconds = ms ;
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
    std::ifstream fHist("INT.TXT");          // Plik 2: Profil dobowy (Minuta | Lambda)

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

    std::vector<MinuteStat> day_profile;
    std::string mStr, lStr;

    // Wczytywanie Minuty i Lambdy z INT.TXT
    while (fHist >> mStr >> lStr) {
        std::replace(lStr.begin(), lStr.end(), ',', '.');
        day_profile.push_back({std::stoi(mStr), std::stod(lStr)});
    }

    // KROK 2: Precyzyjna odbudowa osi czasu (Profil dobowy)
    std::cout << "Trwa odbudowa osi czasu na podstawie profilu dobowego..." << std::endl;

    double total_lambda = 0.0;
    for (const auto& ms : day_profile) {
        total_lambda += ms.lambda;
    }

    // Współczynnik pozwalający rozdzielić wszystkie zapytania zgodnie z siłą lambdy
    double scale = service_times.size() / total_lambda;

    std::vector<Call> timeline;
    int current_service_idx = 0;
    double max_sim_time = 0.0; // Przechowuje czas zakończenia ostatniego zapytania

    for (size_t i = 0; i < day_profile.size(); ++i) {
        // Liczymy, ile zapytań przypada na tę konkretną minutę
        int num_calls = static_cast<int>(std::round(day_profile[i].lambda * scale));

        // Zabezpieczenie na wypadek zaokrągleń (żeby nie przekroczyć rozmiaru i przypisać wszystko)
        if (current_service_idx + num_calls > service_times.size()) {
            num_calls = service_times.size() - current_service_idx;
        }
        if (i == day_profile.size() - 1 && current_service_idx < service_times.size()) {
            // Ostatnia minuta w pliku "zgarnia" pozostałe żądania, by nic nie zginęło
            num_calls = service_times.size() - current_service_idx;
        }

        if (num_calls <= 0) continue;

        // Start tej konkretnej minuty w milisekundach
        double minute_start_ms = day_profile[i].minute * 60.0;

        // Równomierne rozłożenie startów zapytań wewnątrz danej minuty (dla okna przesuwnego)
        double interval_ms = 60.0 / num_calls;

        for (int c = 0; c < num_calls; ++c) {
            Call call;
            call.start_time = minute_start_ms + (c * interval_ms);
            call.duration = service_times[current_service_idx];
            call.end_time = call.start_time + call.duration;

            timeline.push_back(call);
            current_service_idx++;

            if (call.end_time > max_sim_time) {
                max_sim_time = call.end_time;
            }
        }
    }

    std::cout << "Odbudowano " << timeline.size() << " zapytan." << std::endl;
    std::cout << "Koniec ostatniego polaczenia na osi czasu: " << formatTime(max_sim_time) << "\n" << std::endl;

    // KROK 4: Algorytm Okna Przesuwnego (Szukanie GNR)
    std::cout << "Szukam Godziny Najwiekszego Ruchu (GNR)..." << std::endl;

    double window_size = 3600.0;       // 1 godzina = 3 600 000 ms
    double step_size = 60.0 * 15;      // Przesunięcie o 15 minut (wg Twojego kodu)

    double max_erlangs = 0.0;
    double best_gnr_start = 0.0;

    for (double w_start = 0; w_start <= max_sim_time - window_size; w_start += step_size) {
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
        fOut << "Wywolania z pliku czas.txt nalezace do GNR\n";
        fOut << "Okno: " << formatTime(best_gnr_start) << " - " << formatTime(best_gnr_start + window_size) << "\n";
        fOut << "Ruch: " << max_erlangs << " Erlangow\n";
        fOut << "--------------------------------------------------\n";

        int line_number = 1;
        int count_in_gnr = 0;

        for (const auto& call : timeline) {
            // Sprawdzamy, czy połączenie "zahaczyło" o nasze okno GNR
            if (call.end_time > best_gnr_start && call.start_time < best_gnr_start + window_size) {
                fOut << "Linia w pliku: " << line_number << " \t| Czas obslugi: " << call.duration << " ms\n";
                count_in_gnr++;
            }
            line_number++;
        }
        fOut.close();
        std::cout << "\nZnaleziono " << count_in_gnr << " zapytan tworzacych GNR." << std::endl;
        std::cout << "Zapisano dokladne numery linii do pliku: gnr_linie.txt" << std::endl;
    } else {
        std::cerr << "Nie udalo sie utworzyc pliku gnr_linie.txt" << std::endl;
    }

    return 0;
}