//
// Created by foxyu on 27.05.2026.
//

#include "srDzien.h"


// Funkcja tworząca plik ze średnimi wartościami z X dni
void wygenerujSredniDzien(int num_days, const std::string& nazwa_wyjsciowa ) {
    std::map<int, double> suma_obciazen;

    // 1. Odczyt danych ze wszystkich plików INTX.TXT
    for (int i = 1; i <= num_days; ++i) {
        std::string nazwa_wejsciowa = "INT" + std::to_string(i) + ".TXT";
        std::ifstream plik(nazwa_wejsciowa);

        if (!plik.is_open()) {
            std::cerr << "Ostrzezenie: Nie mozna otworzyc pliku " << nazwa_wejsciowa << " - pomijam." << std::endl;
            continue;
        }

        int minuta;
        double obciazenie;
        while (plik >> minuta >> obciazenie) {
            suma_obciazen[minuta] += obciazenie;
        }
        plik.close();
    }

    // 2. Zapis uśrednionych danych do nowego pliku
    std::ofstream plik_wyjsciowy(nazwa_wyjsciowa);
    if (!plik_wyjsciowy.is_open()) {
        throw std::runtime_error("Nie udalo sie utworzyc pliku wyjsciowego: " + nazwa_wyjsciowa);
    }

    // Zapisujemy w formacie naukowym (notacja 'e'), dokładnie tak jak w oryginale
    plik_wyjsciowy << std::scientific;

    for (const auto& para : suma_obciazen) {
        int minuta = para.first;
        double srednie_obciazenie = para.second / num_days;

        // Zapis: minuta (czysty int) + tabulacja + średnie obciążenie
        plik_wyjsciowy << minuta << "\t" << srednie_obciazenie << "\n";
    }

    plik_wyjsciowy.close();
    std::cout << "Pomyslnie wygenerowano plik ze srednim dniem: " << nazwa_wyjsciowa << std::endl;
}