//
// Created by foxyu on 27.05.2026.
//

#include "srDzien.h"


/**
 * @brief Generuje uśredniony profil dobowy na podstawie zadanej liczby dni pomiarowych.
 * * Funkcja przygotowuje dane wejściowe, które są kluczowe m.in. dla algorytmu TCBH
 * (Time Consistent Busy Hour), bazującego z założenia na analizie tzw. "średniego dnia".
 * Logika działania:
 * 1. Iteruje przez pliki źródłowe reprezentujące poszczególne dni (np. INT1.TXT, INT2.TXT).
 * 2. Sumuje wartości obciążenia ruchu dla odpowiadających sobie minut ze wszystkich dni, gromadząc je w strukturze mapy.
 * 3. Dzieli zgromadzone sumy przez całkowitą liczbę dni, uzyskując precyzyjną średnią arytmetyczną dla każdej minuty.
 * 4. Zapisuje tak powstały "średni dzień" do nowego pliku (z zachowaniem notacji naukowej), utrzymując pełną kompatybilność z dalszymi etapami programu.
 * * @param num_days Liczba dni (plików wejściowych), z których dane zostaną zagregowane i uśrednione.
 * @param nazwa_wyjsciowa Ścieżka i nazwa pliku docelowego (np. "INT_SR.TXT"), w którym zostanie zapisany wynik.
 * @throws std::runtime_error Jeśli wystąpi problem z utworzeniem lub zapisaniem pliku wyjściowego.
 */
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

    // Zapisujemy w formacie naukowym
    plik_wyjsciowy << std::scientific;

    for (const auto& para : suma_obciazen) {
        int minuta = para.first;
        double srednie_obciazenie = para.second / num_days;

        // Zapis: minuta + tabulacja + średnie obciążenie
        plik_wyjsciowy << minuta << "\t" << srednie_obciazenie << "\n";
    }

    plik_wyjsciowy.close();
    std::cout << "Pomyslnie wygenerowano plik ze srednim dniem: " << nazwa_wyjsciowa << std::endl;
}