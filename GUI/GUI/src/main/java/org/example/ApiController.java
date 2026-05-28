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

@RestController
public class ApiController {

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
            String exePath = new File(System.getProperty("user.dir"), "silnik.exe").getAbsolutePath();

            List<String> command = new ArrayList<>(Arrays.asList(exePath, "--days", days, "--window", window, "--step", step));            if (tcbh) command.add("--tcbh");
            if (adpqh) command.add("--adpqh");
            if (force) command.add("--force");

            ProcessBuilder pb = new ProcessBuilder(command);
            pb.directory(new File(System.getProperty("user.dir")));
            pb.redirectErrorStream(true);
            Process process = pb.start();

            // Złota zasada ProcessBuilder: ZAWSZE czytaj wyjście, żeby nie zablokować bufora!
            try (java.io.BufferedReader reader = new java.io.BufferedReader(new java.io.InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    System.out.println("[Silnik C++] " + line);
                }
            }

            if (process.waitFor() != 0) {
                throw new RuntimeException("C++ zwrócił błąd. Sprawdź czarną konsolę serwera!");
            }
            List<String> wynikiLines = Files.readAllLines(Path.of(System.getProperty("user.dir"), "wyniki.txt"));
            String metoda = wynikiLines.get(0).trim();

            List<Map<String, Object>> dniDane = new ArrayList<>();
            String wynikGnr = "0";

            if (metoda.equals("TCBH")) {
                wynikGnr = wynikiLines.get(1);
                double peakStartSec = Double.parseDouble(wynikiLines.get(2));
                String[] chartData = wynikiLines.get(3).split(" "); // C++ podaje Erlangi na tacy!

                Map<String, Object> dzienData = zbudujDzien("Średni dzień (TCBH)", peakStartSec, Double.parseDouble(wynikGnr), step, window, chartData);
                dzienData.put("tabela", czytajLinieGnr("gnr_linie.txt"));
                dniDane.add(dzienData);

            } else { // ADPQH
                for (int i = 1; i < wynikiLines.size(); i++) {
                    String[] part = wynikiLines.get(i).split(" ");
                    if (part[0].equals("DZIEN")) {
                        int nrDnia = Integer.parseInt(part[1]);
                        double erlangiDnia = Double.parseDouble(part[2]);
                        double peakStartSec = Double.parseDouble(part[3]);

                        String[] chartData = wynikiLines.get(i+1).split(" "); // Linia z CHART
                        i++; // Przeskakujemy linię z wykresem w pętli

                        Map<String, Object> dzienData = zbudujDzien("Dzień " + nrDnia, peakStartSec, erlangiDnia, step, window, chartData);
                        dzienData.put("tabela", czytajLinieGnr("gnr_linie_" + nrDnia + ".txt"));
                        dniDane.add(dzienData);
                    } else if (part[0].equals("SREDNIA")) {
                        wynikGnr = part[1];
                    }
                }
            }

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

    // Zbudowanie obiektu na podstawie precyzyjnych danych z C++
    private Map<String, Object> zbudujDzien(String etykieta, double peakStartSec, double peakErlangs, String step, String window, String[] chartDataStrings) {
        int stepSec = Integer.parseInt(step);
        int windowSec = Integer.parseInt(window);

        List<Double> erlangiData = new ArrayList<>();
        List<String> etykiety = new ArrayList<>();

        // Zaczynamy od indeksu 1, bo indeks 0 to słowo "CHART"
        for (int i = 1; i < chartDataStrings.length; i++) {
            int totalSec = (i - 1) * stepSec;
            int h = totalSec / 3600;
            int m = (totalSec % 3600) / 60;

            etykiety.add(String.format("%02d:%02d", h, m));
            erlangiData.add(Double.parseDouble(chartDataStrings[i]));
        }

        Map<String, Object> dzienData = new HashMap<>();
        dzienData.put("nazwa", etykieta);
        dzienData.put("erlangiDnia", peakErlangs);
        dzienData.put("peakStartIndex", (int) (peakStartSec / stepSec));
        dzienData.put("binsInWindow", Math.max(1, windowSec / stepSec));
        dzienData.put("labels", etykiety);
        dzienData.put("data", erlangiData);
        return dzienData;
    }

    private List<Map<String, Object>> czytajLinieGnr(String nazwaPliku) {
        List<Map<String, Object>> tabela = new ArrayList<>();
        Path resultPath = Path.of(System.getProperty("user.dir"), nazwaPliku);
        if (Files.exists(resultPath)) {
            try {
                List<String> resultLines = Files.readAllLines(resultPath);
                Pattern linePattern = Pattern.compile("Linia w pliku:\\s*(\\d+).*Czas obslugi:\\s*([0-9.]+)\\s*s");
                for (String linia : resultLines) {
                    Matcher matcher = linePattern.matcher(linia);
                    if (matcher.find()) {
                        tabela.add(Map.of("linia", matcher.group(1), "czas", matcher.group(2) + "s"));
                    }
                }
            } catch (Exception ignored) {}
        }
        return tabela;
    }
}