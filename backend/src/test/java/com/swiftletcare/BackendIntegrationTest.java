package com.swiftletcare;

import jakarta.persistence.EntityManagerFactory;
import org.flywaydb.core.Flyway;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.web.server.LocalServerPort;
import org.springframework.jdbc.core.JdbcTemplate;

import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;

import static org.assertj.core.api.Assertions.assertThat;

@EnabledIfEnvironmentVariable(named = "DB_TEST_ENABLED", matches = "true")
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.RANDOM_PORT,
        useMainMethod = SpringBootTest.UseMainMethod.ALWAYS)
class BackendIntegrationTest {
    @LocalServerPort
    int port;
    @Autowired
    Flyway flyway;
    @Autowired
    JdbcTemplate jdbc;
    @Autowired
    EntityManagerFactory entityManagerFactory;

    @Test
    void migrationCreatesSchemaAndCanBeRepeated() {
        assertThat(jdbc.queryForObject(
                "select count(*) from information_schema.schemata where schema_name = 'swiftletcare'",
                Integer.class)).isEqualTo(1);
        assertThat(flyway.info().current().getVersion().toString()).isEqualTo("1");
        assertThat(flyway.migrate().migrationsExecuted).isZero();
        assertThat(entityManagerFactory.isOpen()).isTrue();
    }

    @Test
    void healthIncludesWorkingDatabaseAndProbes() throws Exception {
        try (var client = HttpClient.newHttpClient()) {
            for (String path : new String[]{"/actuator/health", "/actuator/health/liveness",
                    "/actuator/health/readiness"}) {
                var response = client.send(HttpRequest.newBuilder(
                        URI.create("http://localhost:" + port + path)).GET().build(),
                        HttpResponse.BodyHandlers.ofString());
                assertThat(response.statusCode()).isEqualTo(200);
                assertThat(response.body()).contains("\"status\":\"UP\"");
                assertThat(response.body()).doesNotContain("jdbc:postgresql", "password");
            }
        }
    }
}
