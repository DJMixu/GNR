//
// Created by DJMixu on 16.04.2026.
//

#ifndef SILNIK_MODELS_H
#define SILNIK_MODELS_H
#pragma once
/**
 * @brief Struktura reprezentująca pojedyncze zgłoszenie (połączenie) w systemie.
 * Używana do budowania chronologicznej osi czasu zdarzeń w trakcie symulacji.
 */
struct Call {
    double start_time; // Czas rozpoczęcia zgłoszenia w sekundach (liczony od początku doby)
    double duration;   // Czas trwania obsługi zgłoszenia w sekundach
    double end_time;   // Czas zakończenia obsługi (wynika z: start_time + duration)
};
/**
 * @brief Struktura przechowująca statystyki obciążenia dla pojedynczej minuty.
 * Wykorzystywana do budowy dobowego profilu ruchu (wczytywanego np. z plików INT1.TXT).
 */
struct MinuteStat {
    int minute;     // Numer minuty w ciągu doby (zazwyczaj od 0 do 1439)
    double lambda;  // Intensywność zgłoszeń (parametr lambda) przypisana do tej minuty
};
#endif //SILNIK_MODELS_H
