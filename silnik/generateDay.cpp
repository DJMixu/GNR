//
// Created by foxyu on 16.04.2026.
//

#include "generateDay.h"
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <algorithm>
#include <filesystem>


/**
 * @brief Generuje nowy profil dnia na podstawie danych źródłowych, wprowadzając drobną losowość.
 * * Funkcja służy do symulowania kolejnych dni pomiarowych na podstawie głównego wzorca.
 * Logika działania:
 * 1. Otwiera plik źródłowy z danymi bazowymi oraz plik wyjściowy.
 * 2. Przechodzi przez każdą minutę doby i odczytuje przypisane do niej obciążenie ruchu (dbając o poprawne parsowanie ułamków).
 * 3. Modyfikuje każdą wartość obciążenia, nakładając na nią losowe odchylenie na poziomie ±5% (mnożnik od 0.95 do 1.05).
 * 4. Zapisuje zmienione dane do nowego pliku.
 * Dzięki temu procesowi uzyskujemy realistyczne, różniące się od siebie dni, które jednak nadal zachowują pierwotny trend i kształt dobowego ruchu.
 * * @param source_path Ścieżka do pliku ze wzorcowym profilem dnia.
 * @param output_path Ścieżka do pliku docelowego, w którym zostanie zapisany wygenerowany, zmodyfikowany dzień.
 */
void generate_day(const std::string& source_path, const std::string& output_path) {
    std::ifstream input_file(source_path);
    std::ofstream output_file(output_path);

    if (!input_file.is_open()) {
        std::cerr << "Nie udalo sie otworzyc pliku zrodlowego: " << source_path << "\n";
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.95, 1.05);

    std::string minute_str, load_str;

    while (input_file >> minute_str >> load_str) {
        std::replace(load_str.begin(), load_str.end(), ',', '.');

        try {
            int minute = std::stoi(minute_str);
            double load = std::stod(load_str);

            double new_load = load * dist(gen);

            // Zapis do pliku
            output_file << minute << "\t" << new_load << "\n";
        } catch (...) {
            continue;
        }
    }
}

/**
 * @brief Przygotowuje środowisko pracy, upewniając się, że istnieją wszystkie wymagane pliki pomiarowe.
 * * Funkcja weryfikuje obecność plików z dobowymi profilami ruchu (np. INT1.TXT, INT2.TXT)
 * dla zadeklarowanej liczby dni. Jeśli któregoś pliku brakuje, wywołuje generator,
 * który tworzy go na bazie podanego profilu wzorcowego. Funkcja posiada również mechanizm
 * wymuszania nadpisywania plików, co pozwala na szybkie wyczyszczenie starych wyników
 * i wygenerowanie zupełnie nowych, różniących się od siebie dni do symulacji.
 * * @param source_file Ścieżka do pliku wzorcowego, używanego jako baza do generowania nowych wariantów ruchu.
 * @param number_of_days Oczekiwana liczba plików dziennych, które mają być gotowe do dalszej analizy.
 * @param overwrite Flaga decydująca o tym, czy istniejące już pliki mają zostać stworzone od nowa (true), czy zachowane (false).
 */
void ensure_days_exist(const std::string& source_file, int number_of_days, bool overwrite) {
    for (int i = 1; i <= number_of_days; ++i) {
        std::string filename = "INT" + std::to_string(i) + ".TXT";

        // Sprawdzamy, czy plik istnieje
        bool file_exists = std::filesystem::exists(filename);
        if (!file_exists || overwrite) {
            if (file_exists && overwrite) {
                std::cout << "Nadpisuje istniejacy plik: " << filename << "\n";
            } else {
                std::cout << "Generowanie nowego pliku: " << filename << "\n";
            }

            generate_day(source_file, filename);
        } else {
            std::cout << "Plik " << filename << " juz istnieje. Pomijam.\n";
        }
    }
}