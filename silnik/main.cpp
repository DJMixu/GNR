#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <cmath>

struct Call {
    double start_time;
    double duration;
    double end_time;
};

struct MinuteStat {
    int minute;
    double lambda;
};

// ─── Funkcje pomocnicze ───────────────────────────────────────────────────────

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

// ─── Wczytywanie danych ───────────────────────────────────────────────────────

std::vector<double> loadServiceTimes(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Nie mozna otworzyc pliku: " + filename);

    std::vector<double> times;
    std::string token;
    while (file >> token) {
        std::replace(token.begin(), token.end(), ',', '.');
        times.push_back(std::stod(token));
    }
    return times;
}

std::vector<MinuteStat> loadDayProfile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Nie mozna otworzyc pliku: " + filename);

    std::vector<MinuteStat> profile;
    std::string mStr, lStr;
    while (file >> mStr >> lStr) {
        std::replace(lStr.begin(), lStr.end(), ',', '.');
        profile.push_back({std::stoi(mStr), std::stod(lStr)});
    }
    return profile;
}

// ─── Budowanie osi czasu ──────────────────────────────────────────────────────

struct TimelineResult {
    std::vector<Call> calls;
    double max_sim_time = 0.0;
};

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

// ─── Algorytm okna przesuwnego (GNR) ─────────────────────────────────────────

struct GnrResult {
    double max_erlangs  = 0.0;
    double window_start = 0.0;
    const double window_size  = 3600.0;
};

GnrResult findPeakHour(const std::vector<Call>& timeline, double max_sim_time) {
    GnrResult best;
    const double window_size = best.window_size;
    const double step_size   = 60.0 * 15;


    for (double w_start = 0; w_start <= max_sim_time - window_size; w_start += step_size) {
        const double w_end = w_start + window_size;
        double active_time = 0.0;

        for (const auto& call : timeline) {
            if (call.end_time   <= w_start) continue;
            if (call.start_time >= w_end)   break;

            active_time += std::min(w_end,   call.end_time)
                         - std::max(w_start, call.start_time);
        }

        const double erlangs = active_time / window_size;
        if (erlangs > best.max_erlangs) {
            best.max_erlangs  = erlangs;
            best.window_start = w_start;
        }
    }
    return best;
}

// ─── Eksport wyników ──────────────────────────────────────────────────────────

void exportGnrLines(const std::vector<Call>& timeline,
                    const GnrResult& gnr,
                    const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Nie udalo sie utworzyc pliku: " + filename);

    const double w_end = gnr.window_start + gnr.window_size;

    file << std::fixed << std::setprecision(0);
    file << "Wywolania z pliku czas.txt nalezace do GNR\n";
    file << "Okno: " << formatTime(gnr.window_start) << " - " << formatTime(w_end) << "\n";
    file << "Ruch: " << gnr.max_erlangs << " Erlangow\n";
    file << "--------------------------------------------------\n";

    int line_number   = 1;
    int count_in_gnr  = 0;
    for (const auto& call : timeline) {
        if (call.end_time > gnr.window_start && call.start_time < w_end) {
            file << "Linia w pliku: " << line_number
                 << " \t| Czas obslugi: " << call.duration << " s\n";
            ++count_in_gnr;
        }
        ++line_number;
    }

    std::cout << "\nZnaleziono " << count_in_gnr << " zapytan tworzacych GNR." << std::endl;
    std::cout << "Zapisano dokladne numery linii do pliku: " << filename << std::endl;
}

// ─── main ─────────────────────────────────────────────────────────────────────

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