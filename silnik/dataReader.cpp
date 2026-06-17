//
// Created by DJMixu on 16.04.2026.
//

#include "dataReader.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
/**
 * @brief Wczytuje bazowe czasy obsługi z podanego pliku tekstowego.
 * * Funkcja otwiera plik i odczytuje z niego kolejne wartości liczbowe.
 * Ważnym krokiem jest ujednolicenie formatu zapisu dziesiętnego – algorytm automatycznie
 * zamienia przecinki na kropki, co zapobiega błędom konwersji w systemach o różnych ustawieniach regionalnych.
 * * @param filename Ścieżka do pliku zawierającego dane o czasach obsługi.
 * @return std::vector<double> Wektor zawierający wczytane czasy obsługi (np. w sekundach).
 * @throws std::runtime_error Jeśli plik nie istnieje lub nie może zostać otwarty.
 */
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
/**
 * @brief Wczytuje dobowy profil ruchu (statystyki minutowe) z pliku tekstowego.
 * * Funkcja przetwarza plik zawierający pary danych: numer minuty oraz przypisaną do niej wartość
 * (np. intensywność zgłoszeń). Podobnie jak przy czasach obsługi, skrypt dba o bezpieczeństwo
 * parsowania, zamieniając przecinki na kropki dla wartości ułamkowych przed ich zapisaniem do pamięci.
 * * @param filename Ścieżka do pliku z dobowym profilem ruchu (np. INT1.TXT).
 * @return std::vector<MinuteStat> Zbiór struktur reprezentujących statystyki ruchu w poszczególnych minutach doby.
 * @throws std::runtime_error Jeśli plik nie istnieje lub nie może zostać otwarty.
 */
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
