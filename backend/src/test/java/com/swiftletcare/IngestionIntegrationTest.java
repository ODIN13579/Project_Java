package com.swiftletcare;

import static org.assertj.core.api.Assertions.*;

import com.swiftletcare.iot.Ingestion;
import java.net.URI;
import java.net.http.*;
import java.nio.charset.StandardCharsets;
import java.time.*;
import java.util.*;
import java.util.concurrent.*;
import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.junit.jupiter.api.*;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.web.server.LocalServerPort;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import tools.jackson.databind.json.JsonMapper;

@EnabledIfEnvironmentVariable(named = "DB_TEST_ENABLED", matches = "true")
@SpringBootTest(
    webEnvironment = SpringBootTest.WebEnvironment.RANDOM_PORT,
    useMainMethod = SpringBootTest.UseMainMethod.ALWAYS,
    properties = {
      "app.jwt-secret=build104-test-secret-at-least-32-bytes",
          "app.ai-service-key=build104-test-service-key-at-least-24",
      "app.mqtt.url=${TEST_MQTT_URL:tcp://localhost:51883}",
          "app.mqtt.client-id=build104-ingestion-test",
      "app.heartbeat-timeout-ms=1200", "app.status-scan-ms=100"
    })
@TestInstance(TestInstance.Lifecycle.PER_CLASS)
class IngestionIntegrationTest {
  @LocalServerPort int port;
  @Autowired JdbcTemplate jdbc;
  @Autowired JsonMapper json;
  @Autowired Ingestion ingestion;
  @Autowired BCryptPasswordEncoder passwords;
  @Autowired org.springframework.security.oauth2.jwt.JwtEncoder encoder;
  HttpClient http = HttpClient.newHttpClient();
  MqttClient mqtt;
  BlockingQueue<String> acks = new LinkedBlockingQueue<>();
  BlockingQueue<String> events = new LinkedBlockingQueue<>();
  long house, zone, device, camera, user;
  String code, topic, token, email;

  @BeforeAll
  void setup() throws Exception {
    String suffix = UUID.randomUUID().toString();
    code = "ESP-" + suffix;
    email = suffix + "@example.test";
    house =
        jdbc.queryForObject(
            "insert into swiftletcare.houses(name,active) values('integration',true) returning id",
            Long.class);
    zone =
        jdbc.queryForObject(
            "insert into swiftletcare.zones(house_id,name,active) values(?,'integration',true)"
                + " returning id",
            Long.class,
            house);
    device =
        jdbc.queryForObject(
            "insert into swiftletcare.devices(zone_id,device_id,name,type,enabled)"
                + " values(?,?,'ESP','ESP32',true) returning id",
            Long.class,
            zone,
            code);
    camera =
        jdbc.queryForObject(
            "insert into swiftletcare.devices(zone_id,device_id,name,type,enabled)"
                + " values(?,?,'Camera','CAMERA',true) returning id",
            Long.class,
            zone,
            "CAM-" + suffix);
    user =
        jdbc.queryForObject(
            "insert into swiftletcare.users(email,password_hash,role,active)"
                + " values(?,?,'OWNER',true) returning id",
            Long.class,
            email,
            passwords.encode("test-password"));
    jdbc.update(
        "insert into swiftletcare.house_memberships(user_id,house_id,role,active,joined_at)"
            + " values(?,?,'OWNER',true,now())",
        user,
        house);
    var login =
        request(
            "POST",
            "/api/v1/auth/login",
            json.writeValueAsString(Map.of("email", email, "password", "test-password")),
            null);
    assertThat(login.statusCode()).isEqualTo(200);
    token = json.readTree(login.body()).get("accessToken").asText();
    topic = "swiftletcare/" + house + "/" + zone + "/" + code;
    mqtt =
        new MqttClient(
            System.getenv().getOrDefault("TEST_MQTT_URL", "tcp://localhost:51883"),
            "probe-" + suffix,
            new MemoryPersistence());
    mqtt.connect();
    mqtt.subscribe(
        topic + "/telemetry_ack",
        1,
        (t, m) -> acks.add(new String(m.getPayload(), StandardCharsets.UTF_8)));
    // Wait for backend subscription by probing a valid heartbeat, rather than a fixed startup
    // sleep.
    long until = System.nanoTime() + TimeUnit.SECONDS.toNanos(15);
    while (System.nanoTime() < until) {
      publish(
          "heartbeat",
          json.writeValueAsString(
              Map.of("device_id", code, "timestamp", Instant.now().toString())));
      if (Boolean.TRUE.equals(
          jdbc.queryForObject(
              "select online from swiftletcare.devices where id=?", Boolean.class, device))) return;
      Thread.sleep(150);
    }
    fail("Backend MQTT subscription not ready");
  }

  @AfterAll
  void close() throws Exception {
    if (mqtt != null) {
      mqtt.disconnect();
      mqtt.close();
    }
    http.close();
  }

  HttpResponse<String> request(String method, String path, String body, String auth)
      throws Exception {
    var b =
        HttpRequest.newBuilder(URI.create("http://localhost:" + port + path))
            .header("Content-Type", "application/json");
    if (auth != null) b.header("Authorization", "Bearer " + auth);
    if (path.contains("/internal/ai/"))
      b.header("X-Service-Key", "build104-test-service-key-at-least-24");
    return http.send(
        b.method(
                method,
                body == null
                    ? HttpRequest.BodyPublishers.noBody()
                    : HttpRequest.BodyPublishers.ofString(body))
            .build(),
        HttpResponse.BodyHandlers.ofString());
  }

  String payload(String id) {
    return "{\"device_id\":\""
        + code
        + "\",\"event_id\":\""
        + id
        + "\",\"timestamp\":\"2026-01-01T00:00:00Z\",\"temperature\":29.5,\"humidity\":80,\"co2\":null,\"light\":12,\"sensor_status\":\"PARTIAL_ERROR\"}";
  }

  void publish(String kind, String body) throws Exception {
    mqtt.publish(topic + "/" + kind, body.getBytes(StandardCharsets.UTF_8), 0, false);
  }

  String ack(String id) throws Exception {
    long end = System.nanoTime() + TimeUnit.SECONDS.toNanos(8);
    while (System.nanoTime() < end) {
      var a = acks.poll(200, TimeUnit.MILLISECONDS);
      if (a != null && json.readTree(a).get("event_id").asText().equals(id)) return a;
    }
    throw new AssertionError("No ACK for " + id);
  }

  WebSocket socket(long id) throws Exception {
    return http.newWebSocketBuilder()
        .header("Authorization", "Bearer " + token)
        .header("Origin", "http://localhost:5173")
        .buildAsync(
            URI.create("ws://localhost:" + port + "/ws/devices/" + id),
            new WebSocket.Listener() {
              StringBuilder text = new StringBuilder();

              public void onOpen(WebSocket w) {
                w.request(1);
              }

              public CompletionStage<?> onText(WebSocket w, CharSequence data, boolean last) {
                text.append(data);
                if (last) {
                  events.add(text.toString());
                  text.setLength(0);
                }
                w.request(1);
                return null;
              }
            })
        .get(5, TimeUnit.SECONDS);
  }

  @Test
  void mqttCommitAckDuplicateConflictInvalidAndWebsocket() throws Exception {
    acks.clear();
    events.clear();
    var ws = socket(device);
    String id = "mqtt-" + UUID.randomUUID();
    try {
      publish("telemetry", payload(id));
      assertThat(ack(id)).contains("STORED");
      // JdbcTemplate uses a separate connection from the completed ingestion transaction.
      assertThat(
              jdbc.queryForObject(
                  "select count(*) from swiftletcare.telemetry where device_id=? and event_id=?",
                  Integer.class,
                  device,
                  id))
          .isEqualTo(1);
      assertThat(events.poll(5, TimeUnit.SECONDS))
          .contains("telemetry.updated", id, "PARTIAL_ERROR");
      publish("telemetry", payload(id));
      assertThat(ack(id)).contains("STORED");
      assertThat(
              jdbc.queryForObject(
                  "select count(*) from swiftletcare.telemetry where device_id=? and event_id=?",
                  Integer.class,
                  device,
                  id))
          .isEqualTo(1);
      publish("telemetry", payload(id).replace("29.5", "30.5"));
      assertThat(acks.poll(700, TimeUnit.MILLISECONDS)).isNull();
      publish("telemetry", payload("bad").replace("\"humidity\":80", "\"humidity\":180"));
      publish("telemetry", "{invalid");
      publish("telemetry", payload("wrong-device").replace(code, "UNKNOWN"));
      assertThat(acks.poll(700, TimeUnit.MILLISECONDS)).isNull();
      assertThat(
              jdbc.queryForObject(
                  "select count(*) from swiftletcare.telemetry where device_id=? and event_id in"
                      + " ('bad','wrong-device')",
                  Integer.class,
                  device))
          .isZero();
    } finally {
      ws.sendClose(1000, "done").get(3, TimeUnit.SECONDS);
    }
  }

  @Test
  void concurrentDuplicateIsAtomic() throws Exception {
    String id = "race-" + UUID.randomUUID();
    var p = json.readValue(payload(id), Ingestion.Payload.class);
    try (var executor = Executors.newFixedThreadPool(8)) {
      var start = new CountDownLatch(1);
      List<Future<Ingestion.Stored>> futures = new ArrayList<>();
      for (int i = 0; i < 8; i++)
        futures.add(
            executor.submit(
                () -> {
                  start.await();
                  return ingestion.store(house, zone, p);
                }));
      start.countDown();
      int inserted = 0;
      for (var f : futures) if (f.get(10, TimeUnit.SECONDS).inserted()) inserted++;
      assertThat(inserted).isEqualTo(1);
    }
    assertThat(
            jdbc.queryForObject(
                "select count(*) from swiftletcare.telemetry where device_id=? and event_id=?",
                Integer.class,
                device,
                id))
        .isEqualTo(1);
  }

  @Test
  void databaseFailureDoesNotAckOrBroadcastAndRetryWorks() throws Exception {
    String id = "db-fail-" + UUID.randomUUID();
    acks.clear();
    events.clear();
    var ws = socket(device);
    // Fail only this test event at the DB layer; do not stop or alter the user's database.
    jdbc.execute(
        "create or replace function swiftletcare.test_reject_ingest() returns trigger language"
            + " plpgsql as $$ begin if NEW.event_id='"
            + id
            + "' then raise exception 'injected storage failure'; end if; return NEW; end $$");
    jdbc.execute(
        "create constraint trigger test_reject_ingest after insert on swiftletcare.telemetry"
            + " deferrable initially deferred for each row execute function"
            + " swiftletcare.test_reject_ingest()");
    try {
      publish("telemetry", payload(id));
      assertThat(acks.poll(1200, TimeUnit.MILLISECONDS)).isNull();
      var notification = events.poll(200, TimeUnit.MILLISECONDS);
      if (notification != null) assertThat(notification).doesNotContain("telemetry.updated");
      assertThat(
              jdbc.queryForObject(
                  "select count(*) from swiftletcare.telemetry where event_id=?",
                  Integer.class,
                  id))
          .isZero();
    } finally {
      jdbc.execute("drop trigger test_reject_ingest on swiftletcare.telemetry");
      jdbc.execute("drop function swiftletcare.test_reject_ingest()");
      ws.sendClose(1000, "done").get(3, TimeUnit.SECONDS);
    }
    publish("telemetry", payload(id));
    assertThat(ack(id)).contains("STORED");
  }

  @Test
  void heartbeatOnlineTimeoutOfflineRecovery() throws Exception {
    Thread.sleep(1500);
    events.clear();
    var ws = socket(device);
    try {
      long began = System.nanoTime();
      publish(
          "heartbeat",
          json.writeValueAsString(Map.of("device_id", code, "timestamp", "2020-01-01T00:00:00Z")));
      String online = events.poll(5, TimeUnit.SECONDS);
      assertThat(online).contains("device.status.changed", "\"online\":true");
      var seen =
          jdbc.queryForObject(
                  "select last_seen_at from swiftletcare.devices where id=?",
                  java.sql.Timestamp.class,
                  device)
              .toInstant();
      assertThat(seen).isAfter(Instant.now().minusSeconds(5));
      String offline = events.poll(5, TimeUnit.SECONDS);
      assertThat(offline).contains("\"online\":false");
      assertThat(Duration.ofNanos(System.nanoTime() - began)).isLessThan(Duration.ofSeconds(5));
      publish(
          "heartbeat",
          json.writeValueAsString(
              Map.of("device_id", code, "timestamp", Instant.now().toString())));
      assertThat(events.poll(5, TimeUnit.SECONDS)).contains("\"online\":true");
    } finally {
      ws.sendClose(1000, "done").get(3, TimeUnit.SECONDS);
    }
  }

  @Test
  void restFilteringPagingLatestAndAuthorization() throws Exception {
    for (int i = 0; i < 3; i++) {
      var body =
          payload("rest-" + i + "-" + UUID.randomUUID())
              .replace("2026-01-01T00:00:00Z", "2026-01-0" + (i + 2) + "T00:00:00Z");
      ingestion.store(house, zone, json.readValue(body, Ingestion.Payload.class));
    }
    assertThat(request("GET", "/api/v1/devices/" + device, null, token).statusCode())
        .isEqualTo(200);
    assertThat(
            request("GET", "/api/v1/zones/" + zone + "/devices?page=0&size=1", null, token).body())
        .contains("totalElements\":2");
    var latest = request("GET", "/api/v1/devices/" + device + "/telemetry/latest", null, token);
    assertThat(latest.body()).contains("2026-01-04T00:00:00Z");
    String path =
        "/api/v1/devices/"
            + device
            + "/telemetry?from=2026-01-02T00:00:00Z&to=2026-01-04T00:00:00Z";
    var first = json.readTree(request("GET", path + "&page=0&size=1", null, token).body());
    assertThat(first.get("items").size()).isEqualTo(1);
    assertThat(first.get("page").get("totalElements").asInt()).isEqualTo(2);
    var second = json.readTree(request("GET", path + "&page=1&size=1", null, token).body());
    assertThat(second.get("items").get(0).get("id").asLong())
        .isNotEqualTo(first.get("items").get(0).get("id").asLong());
    assertThat(request("GET", path + "&size=101", null, token).statusCode()).isEqualTo(400);
    assertThat(
            request(
                    "GET",
                    "/api/v1/devices/"
                        + device
                        + "/telemetry?from=2026-01-04T00:00:00Z&to=2026-01-02T00:00:00Z",
                    null,
                    token)
                .statusCode())
        .isEqualTo(400);
    assertThat(request("GET", "/api/v1/devices/" + device, null, null).statusCode()).isEqualTo(401);
    assertThat(request("GET", "/api/v1/devices/" + device, null, token + "x").statusCode())
        .isEqualTo(401);
    jdbc.update("update swiftletcare.house_memberships set active=false where user_id=?", user);
    try {
      assertThat(request("GET", "/api/v1/devices/" + device, null, token).statusCode())
          .isEqualTo(403);
    } finally {
      jdbc.update("update swiftletcare.house_memberships set active=true where user_id=?", user);
    }
    assertThat(
            request(
                    "POST",
                    "/api/v1/auth/login",
                    "{\"email\":\"" + email + "\",\"password\":\"wrong\"}",
                    null)
                .statusCode())
        .isEqualTo(401);
  }

  @Test
  void countValidInvalidDuplicateConflictAndConcurrent() throws Exception {
    String path = "/api/v1/internal/ai/count-logs";
    String body =
        json.writeValueAsString(
            Map.of(
                "deviceId",
                camera,
                "periodStart",
                "2026-01-01T00:00:00Z",
                "periodEnd",
                "2026-01-01T01:00:00Z",
                "entryCount",
                12,
                "exitCount",
                3,
                "detectCount",
                15,
                "aggregationType",
                "HOURLY",
                "processedAt",
                "2026-01-01T01:00:01Z"));
    var first = request("POST", path, body, null);
    assertThat(first.statusCode()).isEqualTo(201);
    var again = request("POST", path, body, null);
    assertThat(again.statusCode()).isEqualTo(200);
    assertThat(again.body()).isEqualTo(first.body());
    assertThat(
            request("POST", path, body.replace("\"entryCount\":12", "\"entryCount\":13"), null)
                .statusCode())
        .isEqualTo(409);
    assertThat(
            request("POST", path, body.replace("\"exitCount\":3", "\"exitCount\":-1"), null)
                .statusCode())
        .isEqualTo(400);
    assertThat(
            request(
                    "POST",
                    path,
                    body.replace("\"deviceId\":" + camera, "\"deviceId\":" + device),
                    null)
                .statusCode())
        .isEqualTo(400);
    var noKey =
        http.send(
            HttpRequest.newBuilder(URI.create("http://localhost:" + port + path))
                .header("Content-Type", "application/json")
                .POST(HttpRequest.BodyPublishers.ofString(body))
                .build(),
            HttpResponse.BodyHandlers.ofString());
    assertThat(noKey.statusCode()).isEqualTo(401);
    String race = body.replace("2026-01-01", "2026-01-02");
    try (var ex = Executors.newFixedThreadPool(4)) {
      List<Future<Integer>> fs = new ArrayList<>();
      for (int i = 0; i < 4; i++)
        fs.add(ex.submit(() -> request("POST", path, race, null).statusCode()));
      List<Integer> statuses = new ArrayList<>();
      for (var f : fs) statuses.add(f.get(10, TimeUnit.SECONDS));
      assertThat(statuses).containsOnly(200, 201);
      assertThat(statuses.stream().filter(s -> s == 201).count()).isEqualTo(1);
    }
  }

  @Test
  void jwtExpiryInactiveUserAndWebSocketCookieOrigin() throws Exception {
    var now = Instant.now();
    var claims =
        org.springframework.security.oauth2.jwt.JwtClaimsSet.builder()
            .issuer("swiftletcare")
            .subject(Long.toString(user))
            .issuedAt(now.minusSeconds(600))
            .expiresAt(now.minusSeconds(300))
            .build();
    var expired =
        encoder
            .encode(
                org.springframework.security.oauth2.jwt.JwtEncoderParameters.from(
                    org.springframework.security.oauth2.jwt.JwsHeader.with(
                            org.springframework.security.oauth2.jose.jws.MacAlgorithm.HS256)
                        .build(),
                    claims))
            .getTokenValue();
    assertThat(request("GET", "/api/v1/devices/" + device, null, expired).statusCode())
        .isEqualTo(401);
    jdbc.update("update swiftletcare.users set active=false where id=?", user);
    try {
      assertThat(request("GET", "/api/v1/devices/" + device, null, token).statusCode())
          .isEqualTo(403);
    } finally {
      jdbc.update("update swiftletcare.users set active=true where id=?", user);
    }
    var cookieSocket =
        http.newWebSocketBuilder()
            .header("Cookie", "SC_WS=" + token)
            .header("Origin", "http://localhost:5173")
            .buildAsync(
                URI.create("ws://localhost:" + port + "/ws/devices/" + device),
                new WebSocket.Listener() {})
            .get(5, TimeUnit.SECONDS);
    cookieSocket.sendClose(1000, "done").get(3, TimeUnit.SECONDS);
    assertThatThrownBy(
            () ->
                http.newWebSocketBuilder()
                    .header("Authorization", "Bearer " + token)
                    .header("Origin", "http://untrusted.example")
                    .buildAsync(
                        URI.create("ws://localhost:" + port + "/ws/devices/" + device),
                        new WebSocket.Listener() {})
                    .get(5, TimeUnit.SECONDS))
        .hasCauseInstanceOf(WebSocketHandshakeException.class);
    var cors =
        http.send(
            HttpRequest.newBuilder(
                    URI.create("http://localhost:" + port + "/api/v1/devices/" + device))
                .header("Origin", "http://localhost:5173")
                .header("Access-Control-Request-Method", "GET")
                .method("OPTIONS", HttpRequest.BodyPublishers.noBody())
                .build(),
            HttpResponse.BodyHandlers.ofString());
    assertThat(cors.headers().firstValue("Access-Control-Allow-Origin"))
        .contains("http://localhost:5173");
  }

  @Test
  void precisionNullStatusRetainedAndDisabledDevice() throws Exception {
    String id = "precision-" + UUID.randomUUID();
    String body = payload(id).replace("2026-01-01T00:00:00Z", "2026-01-01T00:00:00.123456789Z");
    publish("telemetry", body);
    assertThat(ack(id)).contains("STORED");
    publish("telemetry", body);
    assertThat(ack(id)).contains("STORED");
    acks.clear();
    publish("telemetry", payload("inconsistent-status").replace("PARTIAL_ERROR", "OK"));
    assertThat(acks.poll(500, TimeUnit.MILLISECONDS)).isNull();
    publish("telemetry", payload("unknown-field").replace("29.5", "29.5,\"extra\":1"));
    assertThat(acks.poll(500, TimeUnit.MILLISECONDS)).isNull();
    jdbc.update("update swiftletcare.devices set enabled=false where id=?", device);
    try {
      publish("telemetry", payload("disabled"));
      assertThat(acks.poll(500, TimeUnit.MILLISECONDS)).isNull();
    } finally {
      jdbc.update("update swiftletcare.devices set enabled=true where id=?", device);
    }
    // Directly exercise retained-delivery flag, as MQTT only sets it on retained replay to new
    // subscriptions.
    var retained = new MqttMessage(payload("retained").getBytes(StandardCharsets.UTF_8));
    retained.setRetained(true);
    gateway.messageArrived(topic + "/telemetry", retained);
    assertThat(acks.poll(500, TimeUnit.MILLISECONDS)).isNull();
  }

  private String rangePayload(
      String id, String temperature, String humidity, String co2, String light, String status) {
    return "{\"device_id\":\""
        + code
        + "\",\"event_id\":\""
        + id
        + "\",\"timestamp\":\"2026-01-01T00:00:00Z\",\"temperature\":"
        + temperature
        + ",\"humidity\":"
        + humidity
        + ",\"co2\":"
        + co2
        + ",\"light\":"
        + light
        + ",\"sensor_status\":\""
        + status
        + "\"}";
  }

  @Test
  void telemetryAcceptsNfrInclusiveBoundariesAndNullSensorErrors() throws Exception {
    String[][] cases = {
      {"-10", "0", "0", "0", "OK"},
      {"60", "100", "5000", "100000", "OK"},
      {"null", "null", "null", "null", "ERROR"}
    };
    for (String[] values : cases) {
      String id = "nfr-valid-" + UUID.randomUUID();
      publish("telemetry", rangePayload(id, values[0], values[1], values[2], values[3], values[4]));
      assertThat(ack(id)).contains("STORED");
      assertThat(
              jdbc.queryForObject(
                  "select count(*) from swiftletcare.telemetry where device_id=? and event_id=?",
                  Integer.class,
                  device,
                  id))
          .isEqualTo(1);
    }
  }

  @Test
  void telemetryRejectsBothSidesOfEveryNfrMeasurementRangeWithoutAckOrRow() throws Exception {
    String[][] cases = {
      {"-10.01", "50", "1000", "100"},
      {"60.01", "50", "1000", "100"},
      {"25", "-0.01", "1000", "100"},
      {"25", "100.01", "1000", "100"},
      {"25", "50", "-0.01", "100"},
      {"25", "50", "5000.01", "100"},
      {"25", "50", "1000", "-0.01"},
      {"25", "50", "1000", "100000.01"}
    };
    for (String[] values : cases) {
      String id = "nfr-invalid-" + UUID.randomUUID();
      acks.clear();
      publish("telemetry", rangePayload(id, values[0], values[1], values[2], values[3], "OK"));
      // A subsequent valid MQTT message is a processing barrier, avoiding a timing-only assertion.
      String barrier = "nfr-barrier-" + UUID.randomUUID();
      publish("telemetry", rangePayload(barrier, "25", "50", "1000", "100", "OK"));
      String first = acks.poll(8, TimeUnit.SECONDS);
      assertThat(first).isNotNull();
      assertThat(json.readTree(first).get("event_id").asText()).isEqualTo(barrier);
      assertThat(acks.poll(200, TimeUnit.MILLISECONDS)).isNull();
      assertThat(
              jdbc.queryForObject(
                  "select count(*) from swiftletcare.telemetry where device_id=? and event_id=?",
                  Integer.class,
                  device,
                  id))
          .isZero();
    }
  }

  @Test
  void telemetryRejectsNonFiniteValuesInEveryMeasurement() {
    for (double value :
        new double[] {Double.NaN, Double.POSITIVE_INFINITY, Double.NEGATIVE_INFINITY}) {
      for (int field = 0; field < 4; field++) {
        Double[] values = {25.0, 50.0, 1000.0, 100.0};
        values[field] = value;
        var p =
            new Ingestion.Payload(
                "nfr-nonfinite-" + UUID.randomUUID(),
                code,
                Instant.parse("2026-01-01T00:00:00Z"),
                values[0],
                values[1],
                values[2],
                values[3],
                "OK");
        assertThatThrownBy(() -> ingestion.store(house, zone, p))
            .isInstanceOf(com.swiftletcare.api.ApiError.Failure.class)
            .hasMessage("Measurement must be finite");
        assertThat(
                jdbc.queryForObject(
                    "select count(*) from swiftletcare.telemetry where device_id=? and event_id=?",
                    Integer.class,
                    device,
                    p.event_id()))
            .isZero();
      }
    }
  }

  @Autowired com.swiftletcare.iot.MqttGateway gateway;
}
