package org.example;

//TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

/**
 * @brief Główna klasa uruchomieniowa aplikacji w frameworku Spring Boot.
 * * Adnotacja @SpringBootApplication to wygodny skrót, który automatycznie konfiguruje aplikację,
 * włącza skanowanie komponentów (wyszukiwanie kontrolerów, serwisów czy repozytoriów w projekcie)
 * oraz ładuje wbudowany serwer webowy (zazwyczaj Tomcat). Stanowi ona punkt wejścia dla
 * całej webowej części projektu.
 */
@SpringBootApplication
public class ErlangApplication {

    /**
     * @brief Metoda startowa uruchamiająca serwer i kontekst aplikacji Spring.
     * * Funkcja deleguje uruchomienie do klasy SpringApplication, co inicjalizuje
     * całe środowisko, wstrzykiwanie zależności (Dependency Injection) i udostępnia endpointy.
     * Po udanym starcie wyświetla w konsoli czytelny komunikat z adresem lokalnym.
     * * @param args Tablica argumentów wiersza poleceń przekazywana podczas startu programu.
     */
    public static void main(String[] args) {
        SpringApplication.run(ErlangApplication.class, args);
        System.out.println("Serwer Spring Boot działa! Wejdź na http://localhost:8080");
    }
}