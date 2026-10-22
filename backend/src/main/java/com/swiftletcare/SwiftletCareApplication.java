package com.swiftletcare;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import java.util.TimeZone;

@SpringBootApplication
public class SwiftletCareApplication {
    public static void main(String[] args) {
        // JDBC sends the JVM timezone during connection startup, before Hibernate settings apply.
        TimeZone.setDefault(TimeZone.getTimeZone("UTC"));
        SpringApplication.run(SwiftletCareApplication.class, args);
    }
}
