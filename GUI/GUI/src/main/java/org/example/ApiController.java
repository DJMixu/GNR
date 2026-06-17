package org.example;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @brief Główny kontroler API (wzorzec RESTful) odpowiedzialny za komunikację z silnikiem w C++.
 * * Klasa odbiera żądania HTTP (GET) z interfejsu przeglądarki, buduje polecenie konsolowe
 * uruchamiające napisany w C++ silnik symulacyjny, a następnie przetwarza wygenerowane przez
 * niego pliki tekstowe na format JSON, który jest zwracany do frontendu (np. do wykresów).
 */
@RestController
public class ApiController {

    /**
     * @brief Endpoint API do uruchamiania obliczeń symulacyjnych i pobierania wyników.
     * * Metoda obsługuje żądanie pod adresem "/api/uruchom". Zbiera parametry od użytkownika,
     * uruchamia zewnętrzny proces (plik .exe), czeka na jego zakończenie, a na koniec parsinguje
     * pliki wyjściowe i buduje kompletną odpowiedź z wynikami GNR (Godziny Największego Rchu) oraz danymi do wykresów.
     * * @param days Liczba dni do analizy (domyślnie 7).
     * @param tcbh Flaga wymuszająca uruchomienie algorytmu na średnim dniu.
     * @param adpqh Flaga wymuszająca uruchomienie algorytmu ruchomego szczytu dla wielu dni.
     * @param window Szerokość okna czasowego GNR w sekundach (domyślnie 3600, czyli 1h).
     * @param step Rozdzielczość analizy w sekundach (domyślnie 900, czyli 15 min).
     * @param force Flaga wymuszająca nadpisanie istniejących plików tekstowych dni z danymi.
     * @return Map<String, Object> Struktura danych automatycznie serializowana przez Springa do formatu JSON z odpowiedziami.
     */
    @GetMapping("/api/uruchom")
    public Map<String, Object> uruchom(
            @RequestParam(name = "days", defaultValue = "7") String days,
            @RequestParam(name = "tcbh", defaultValue = "false") boolean tcbh,
            @RequestParam(name = "adpqh", defaultValue = "false") boolean adpqh,
            @RequestParam(name = "window", defaultValue = "3600") String window,
            @RequestParam(name = "step", defaultValue = "900") String step,
            @RequestParam(name = "force", defaultValue = "false") boolean force
    ) {
        Map<String, Object> response = new HashMap<>();
        try {
            // Lokalizowanie silnika symulacyjnego C++ w bieżącym katalogu
            String exePath = new File(System.getProperty("user.dir"), "silnik.exe").getAbsolutePath();

            // Budowanie listy argumentów dla silnika C++
            List<String> command = new ArrayList<>(Arrays.asList(exePath, "--days", days, "--window", window, "--step", step));
            if (tcbh) command.add("--tcbh");
            if (adpqh) command.add("--adpqh");
            if (force) command.add("--force");

            // Konfiguracja i uruchomienie zewnętrznego procesu
            ProcessBuilder pb = new ProcessBuilder(command);
            pb.directory(new File(System.getProperty("user.dir")));
            pb.redirectErrorStream(true); // Scalamy błędy (stderr) ze standardowym wyjściem (stdout)
            Process process = pb.start();

            // Złota zasada ProcessBuilder: ZAWSZE czytaj wyjście, żeby nie zablokować bufora!
            // Konsumujemy strumień wyjściowy procesu, aby zapobiec zawieszeniu się (deadlock) aplikacji.
            try (java.io.BufferedReader reader = new java.io.BufferedReader(new java.io.InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    System.out.println("[Silnik C++] " + line); // Przekierowanie logów C++ do konsoli Javy
                }
            }

            // Oczekiwanie na zakończenie procesu. Status inny niż 0 oznacza błąd w C++.
            if (process.waitFor() != 0) {
                throw new RuntimeException("C++ zwrócił błąd. Sprawdź czarną konsolę serwera!");
            }

            // --- Odczyt wyników przygotowanych przez silnik ---
            List<String> wynikiLines = Files.readAllLines(Path.of(System.getProperty("user.dir"), "wyniki.txt"));
            String metoda = wynikiLines.get(0).trim(); // Pierwsza linijka określa metodę (TCBH/ADPQH)

            List<Map<String, Object>> dniDane = new ArrayList<>();
            String wynikGnr = "0";

            if (metoda.equals("TCBH")) {
                // Logika parsowania dla jednego, średniego dnia
                wynikGnr = wynikiLines.get(1);
                double peakStartSec = Double.parseDouble(wynikiLines.get(2));
                String[] chartData = wynikiLines.get(3).split(" "); // C++ podaje Erlangi na tacy!

                Map<String, Object> dzienData = zbudujDzien("Średni dzień (TCBH)", peakStartSec, Double.parseDouble(wynikGnr), step, window, chartData);
                dzienData.put("tabela", czytajLinieGnr("gnr_linie.txt"));
                dniDane.add(dzienData);

            } else { // ADPQH
                // Logika parsowania dla wielu dni
                for (int i = 1; i < wynikiLines.size(); i++) {
                    String[] part = wynikiLines.get(i).split(" ");

                    if (part[0].equals("DZIEN")) {
                        // Pobieranie danych dla konkretnego dnia
                        int nrDnia = Integer.parseInt(part[1]);
                        double erlangiDnia = Double.parseDouble(part[2]);
                        double peakStartSec = Double.parseDouble(part[3]);

                        String[] chartData = wynikiLines.get(i+1).split(" "); // Linia z CHART
                        i++; // Przeskakujemy linię z wykresem w pętli

                        Map<String, Object> dzienData = zbudujDzien("Dzień " + nrDnia, peakStartSec, erlangiDnia, step, window, chartData);
                        dzienData.put("tabela", czytajLinieGnr("gnr_linie_" + nrDnia + ".txt"));
                        dniDane.add(dzienData);

                    } else if (part[0].equals("SREDNIA")) {
                        wynikGnr = part[1]; // Ostateczny uśredniony wynik ADPQH
                    }
                }
            }

            // --- Formowanie odpowiedzi końcowej ---
            response.put("status", "Sukces");
            response.put("metoda", metoda);
            response.put("wynikGnr", wynikGnr);
            response.put("dni", dniDane);

        } catch (Exception e) {
            e.printStackTrace();
            response.put("status", "Błąd");
            response.put("wiadomosc", e.getMessage());
        }
        return response;
    }

    /**
     * @brief Tworzy uporządkowany zbiór danych gotowych do wyrenderowania wykresu na frontendzie.
     * * Metoda zamienia surowe dane (tekstowe) z pliku wyjściowego na odpowiednio sformatowaną strukturę,
     * przeliczając numery indeksów koszyków (bin) na konkretne godziny na osi X (np. 14:00).
     * * @param etykieta Nazwa serii danych (np. "Średni dzień", "Dzień 1").
     * @param peakStartSec Sekunda doby, w której rozpoczyna się obliczona GNR.
     * @param peakErlangs Wyliczone szczytowe natężenie ruchu (w Erlangach).
     * @param step Rozmiar kroku (w sekundach) w jakim C++ podzielił dobę.
     * @param window Rozmiar zadanego okna GNR (w sekundach).
     * @param chartDataStrings Tablica z surowymi wartościami Erlangów dla poszczególnych koszyków.
     * @return Map<String, Object> Zestaw danych sformatowany pod bibliotekę wykresów z frontendu (np. Chart.js).
     */
    private Map<String, Object> zbudujDzien(String etykieta, double peakStartSec, double peakErlangs, String step, String window, String[] chartDataStrings) {
        int stepSec = Integer.parseInt(step);
        int windowSec = Integer.parseInt(window);

        List<Double> erlangiData = new ArrayList<>();
        List<String> etykiety = new ArrayList<>();

        // Zaczynamy od indeksu 1, bo indeks 0 to słowo kluczowe "CHART" w pliku wynikowym
        for (int i = 1; i < chartDataStrings.length; i++) {
            // Obliczanie upływającego czasu, bazując na ilości kroków
            int totalSec = (i - 1) * stepSec;
            int h = totalSec / 3600;
            int m = (totalSec % 3600) / 60;

            // Tworzenie etykiet osi X w czytelnym formacie GG:MM
            etykiety.add(String.format("%02d:%02d", h, m));
            erlangiData.add(Double.parseDouble(chartDataStrings[i]));
        }

        Map<String, Object> dzienData = new HashMap<>();
        dzienData.put("nazwa", etykieta);
        dzienData.put("erlangiDnia", peakErlangs);

        // Określenie na którym indeksie na wykresie powinno zacząć się zaznaczenie (highlight) szczytu
        dzienData.put("peakStartIndex", (int) (peakStartSec / stepSec));
        // Określenie jak szerokie na wykresie powinno być okno (ile słupków obejmuje)
        dzienData.put("binsInWindow", Math.max(1, windowSec / stepSec));

        dzienData.put("labels", etykiety);
        dzienData.put("data", erlangiData);
        return dzienData;
    }

    /**
     * @brief Odczytuje szczegóły poszczególnych wywołań wewnątrz Godziny Największego Ruchu z plików wygenerowanych przez C++.
     * * Używa wyrażeń regularnych (RegEx) do wyłuskania numeru linii oraz czasu trwania
     * tylko tych zgłoszeń, które fizycznie przyczyniły się do zaistnienia obciążenia w wyznaczonym oknie GNR.
     * * @param nazwaPliku Nazwa pliku wygenerowanego z detalami, np. "gnr_linie.txt" (w zależności od metody lub dnia).
     * @return List<Map<String, Object>> Lista obiektów (numer linii oraz sformatowany czas obsługi), do wygenerowania tabeli w interfejsie.
     */
    private List<Map<String, Object>> czytajLinieGnr(String nazwaPliku) {
        List<Map<String, Object>> tabela = new ArrayList<>();
        Path resultPath = Path.of(System.getProperty("user.dir"), nazwaPliku);

        // Zabezpieczenie przed błędem, gdy C++ nie wygenerował pliku
        if (Files.exists(resultPath)) {
            try {
                List<String> resultLines = Files.readAllLines(resultPath);

                // Wzorzec RegEx wychwytujący nr linii oraz czas trwania ze zdania wygenerowanego przez C++
                Pattern linePattern = Pattern.compile("Linia w pliku:\\s*(\\d+).*Czas obslugi:\\s*([0-9.]+)\\s*s");

                for (String linia : resultLines) {
                    Matcher matcher = linePattern.matcher(linia);
                    if (matcher.find()) {
                        // Jeśli pasuje do wzorca, grupujemy wyciągnięte dane w słownik i dodajemy do tabeli
                        tabela.add(Map.of("linia", matcher.group(1), "czas", matcher.group(2) + "s"));
                    }
                }
            } catch (Exception ignored) {
                // W razie problemów we/wy na plikach pomocniczych, aplikacja główna nie może przerwać działania (fail-safe)
            }
        }
        return tabela;
    }
}