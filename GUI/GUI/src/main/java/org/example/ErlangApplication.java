package org.example;

//TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
public class ErlangApplication {
    public static void main(String[] args) {
        SpringApplication.run(ErlangApplication.class, args);
        System.out.println("Serwer Spring Boot działa! Wejdź na http://localhost:8080");
    }
}