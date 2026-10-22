"""Destructive-to-availability tests ONLY for compose.build104-test.yml.
Leaves all volumes/data intact. Run after starting the JAR on port 58080 with local seed
in build104_smoke DB. No external Python packages required.
"""
import json
import socket
import struct
import subprocess
import time
import urllib.request
import urllib.error
import uuid

PROJECT = "swiftletcare-build104-e2e"
COMPOSE = ["docker", "compose", "-p", PROJECT, "-f", "compose.build104-test.yml"]
BASE = "http://127.0.0.1:58080"


def docker(*args):
    return subprocess.check_output([*COMPOSE, *args], text=True, stderr=subprocess.STDOUT)


def sql(query):
    return docker("exec", "-T", "postgres", "psql", "-U", "build104", "-d", "build104_smoke", "-At", "-v", "ON_ERROR_STOP=1", "-c", query).strip()


def http(path, body=None, token=None):
    headers = {"Content-Type": "application/json"}
    if token:
        headers["Authorization"] = "Bearer " + token
    req = urllib.request.Request(BASE + path, data=None if body is None else json.dumps(body).encode(), headers=headers)
    try:
        with urllib.request.urlopen(req, timeout=6) as response:
            return response.status, json.load(response)
    except urllib.error.HTTPError as error:
        return error.code, json.load(error)


def wait_for(fn, seconds=30):
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        try:
            value = fn()
            if value:
                return value
        except (OSError, ValueError, subprocess.CalledProcessError):
            pass
        time.sleep(0.2)
    raise AssertionError("Timed out waiting for recovery")


class MQTT:
    # Minimal MQTT 3.1.1 probe; production uses Paho, not this test implementation.
    def __init__(self, topic):
        self.socket = socket.create_connection(("127.0.0.1", 51883), 3)
        client = "smoke-" + uuid.uuid4().hex
        self.send(0x10, self.string("MQTT") + bytes([4, 2, 0, 30]) + self.string(client))
        assert self.read(3) == (0x20, bytes([0, 0]))
        self.send(0x82, bytes([0, 1]) + self.string(topic + "/telemetry_ack") + bytes([1]))
        assert self.read(3)[0] == 0x90
        self.topic = topic

    @staticmethod
    def string(value):
        data = value.encode()
        return struct.pack("!H", len(data)) + data

    def send(self, header, body):
        size = len(body)
        remaining = bytearray()
        while True:
            digit = size % 128
            size //= 128
            remaining.append(digit | (128 if size else 0))
            if not size:
                break
        self.socket.sendall(bytes([header]) + remaining + body)

    def exact(self, size):
        data = bytearray()
        while len(data) < size:
            chunk = self.socket.recv(size - len(data))
            if not chunk:
                raise OSError("MQTT closed")
            data.extend(chunk)
        return bytes(data)

    def read(self, timeout):
        self.socket.settimeout(timeout)
        header = self.exact(1)[0]
        size = 0
        multiplier = 1
        while True:
            digit = self.exact(1)[0]
            size += (digit & 127) * multiplier
            if not digit & 128:
                break
            multiplier *= 128
        return header, self.exact(size)

    def publish(self, suffix, data):
        self.send(0x30, self.string(self.topic + "/" + suffix) + json.dumps(data).encode())

    def ack(self, event_id, timeout=8):
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            header, body = self.read(max(0.01, end - time.monotonic()))
            if header >> 4 != 3:
                continue
            size = struct.unpack("!H", body[:2])[0]
            offset = size + 2
            if (header >> 1) & 3:
                self.send(0x40, body[offset:offset + 2])
                offset += 2
            result = json.loads(body[offset:])
            if result["event_id"] == event_id:
                assert result["status"] == "STORED"
                return result
        raise TimeoutError("ACK timeout")

    def close(self):
        self.socket.close()


def main():
    assert wait_for(lambda: http("/actuator/health")[0] == 200)
    status, login = http("/api/v1/auth/login", {"email": "local@example.test", "password": "smoke-test-password"})
    assert status == 200
    token = login["accessToken"]
    device, zone, house = map(int, sql("select d.id,z.id,z.house_id from swiftletcare.devices d join swiftletcare.zones z on z.id=d.zone_id where d.device_id='ESP32-LOCAL'").split("|"))
    topic = f"swiftletcare/{house}/{zone}/ESP32-LOCAL"
    mqtt = MQTT(topic)
    data = {"device_id": "ESP32-LOCAL", "event_id": "smoke-" + uuid.uuid4().hex, "timestamp": "2026-01-01T00:00:00Z", "temperature": 29.5, "humidity": 80, "co2": None, "light": 12, "sensor_status": "PARTIAL_ERROR"}
    try:
        # Broker starts/restarts after the JAR. Automatic reconnect must restore subscriptions.
        docker("restart", "mqtt")
        mqtt.close()
        wait_for(lambda: docker("exec", "-T", "mqtt", "mosquitto_pub", "-t", "probe", "-m", "ready") is not None)
        mqtt = MQTT(topic)
        def recovered():
            mqtt.publish("telemetry", data)
            try:
                mqtt.ack(data["event_id"], 0.8)
                return True
            except TimeoutError:
                return False
        wait_for(recovered)
        assert sql("select count(*) from swiftletcare.telemetry where event_id='" + data["event_id"] + "'") == "1"
        print("PASS broker restart -> reconnect/resubscribe -> MQTT/DB/ACK")
        mqtt.publish("heartbeat", {"device_id": "ESP32-LOCAL", "timestamp": "2020-01-01T00:00:00Z"})
        wait_for(lambda: http(f"/api/v1/devices/{device}", token=token)[1]["online"])
        began = time.monotonic()
        wait_for(lambda: not http(f"/api/v1/devices/{device}", token=token)[1]["online"], 10)
        elapsed = time.monotonic() - began
        assert 5 <= elapsed <= 8
        print(f"PASS default heartbeat timeout: Offline after {elapsed:.2f}s (REST polling)")
        mqtt.publish("heartbeat", {"device_id": "ESP32-LOCAL", "timestamp": "2020-01-01T00:00:00Z"})
        wait_for(lambda: http(f"/api/v1/devices/{device}", token=token)[1]["online"])
        print("PASS heartbeat Offline -> Online recovery")
        docker("stop", "postgres")
        assert http("/actuator/health")[0] == 503
        assert http("/actuator/health/readiness")[0] == 503
        assert http("/actuator/health/liveness")[0] == 200
        failed = dict(data, event_id="outage-" + uuid.uuid4().hex)
        mqtt.publish("telemetry", failed)
        try:
            mqtt.ack(failed["event_id"], 5)
            raise AssertionError("Unexpected successful ACK while database is stopped")
        except TimeoutError:
            pass
        assert http(f"/api/v1/devices/{device}", token=token)[0] == 503
        print("PASS stopped DB: no successful ACK, REST/health/readiness 503, liveness 200")
        docker("start", "postgres")
        wait_for(lambda: http("/actuator/health")[0] == 200, 40)
        mqtt.publish("telemetry", failed)
        mqtt.ack(failed["event_id"])
        assert sql("select count(*) from swiftletcare.telemetry where event_id='" + failed["event_id"] + "'") == "1"
        print("PASS DB recovery -> retry -> committed row visible at ACK")
    finally:
        docker("start", "postgres", "mqtt")
        mqtt.close()


if __name__ == "__main__":
    main()
