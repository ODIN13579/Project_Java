"""Observe real firmware MQTT; optional command tests intentionally operate FAN."""
import argparse
import datetime as dt
import json
import pathlib
import threading
import time
import uuid
import sys
import paho.mqtt.client as mqtt


def utc_after(seconds):
    return (dt.datetime.now(dt.timezone.utc) + dt.timedelta(seconds=seconds)).strftime('%Y-%m-%dT%H:%M:%SZ')


class Probe:
    def __init__(self, host, port, base, output):
        self.base, self.host, self.port = base.rstrip('/'), host, port
        self.device = self.base.rsplit('/', 1)[-1]
        self.cv = threading.Condition()
        self.connected = threading.Event()
        self.ready = threading.Event()
        self.state = None
        self.state_at = 0
        self.error = None
        self.acks, self.sent_ids, self.telemetry, self.stored = {}, set(), set(), set()
        self.rows = []
        pathlib.Path(output).mkdir(parents=True, exist_ok=True)
        stamp = dt.datetime.now().strftime('%Y%m%d-%H%M%S')
        self.log = open(pathlib.Path(output) / f'mqtt-{stamp}.jsonl', 'w', encoding='utf-8')
        self.report = pathlib.Path(output) / f'result-{stamp}.json'
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id='sc-test-' + uuid.uuid4().hex[:12])
        self.client.on_connect = self.on_connect
        self.client.on_subscribe = self.on_subscribe
        self.client.on_message = self.on_message

    def on_connect(self, client, userdata, flags, reason, props):
        if reason.is_failure:
            self.error = f'Broker rejected CONNECT: {reason}'
        else:
            self.connected.set()
            client.subscribe(self.base + '/#', qos=1)

    def on_subscribe(self, client, userdata, mid, reasons, props):
        if any(r.is_failure for r in reasons):
            self.error = 'Broker rejected subscription'
        else:
            self.ready.set()

    def on_message(self, client, userdata, message):
        try:
            d = json.loads(message.payload)
            if not isinstance(d, dict):
                return
        except (ValueError, UnicodeError):
            return
        event = {'receivedUtc': dt.datetime.now(dt.timezone.utc).isoformat(),
                 'topic': message.topic, 'retained': message.retain, 'payload': d}
        with self.cv:
            self.log.write(json.dumps(event, ensure_ascii=False) + '\n')
            self.log.flush()
            suffix = message.topic.rsplit('/', 1)[-1]
            if suffix == 'state' and not message.retain and d.get('device_id') == self.device:
                self.state, self.state_at = d, time.monotonic()
            elif suffix == 'command_ack' and not message.retain and d.get('device_id') == self.device:
                self.acks.setdefault(d.get('command_id'), []).append(d)
            elif suffix == 'telemetry' and d.get('device_id') == self.device:
                self.telemetry.add(d.get('event_id'))
            elif suffix == 'telemetry_ack' and not message.retain and d.get('device_id') == self.device and d.get('status') == 'STORED':
                self.stored.add(d.get('event_id'))
            self.cv.notify_all()

    def wait(self, predicate, seconds):
        end = time.monotonic() + seconds
        with self.cv:
            while not predicate():
                remaining = end - time.monotonic()
                if remaining <= 0:
                    return False
                self.cv.wait(min(remaining, 1))
            return True

    def record(self, name, status, detail=''):
        self.rows.append({'test': name, 'status': status, 'detail': detail})
        print(f'{status}: {name}: {detail}', flush=True)

    def command(self, action='SET_MODE', target='MANUAL', **fields):
        d = {'command_id': str(uuid.uuid4()), 'action': action, 'actuator': 'FAN', 'expiresAt': utc_after(10)}
        d['targetMode' if action == 'SET_MODE' else 'targetState'] = target
        d.update(fields)
        return d

    def send(self, name, d, status, reason='OK', **expected):
        cid = d['command_id']
        with self.cv:
            before = len(self.acks.get(cid, []))
        info = self.client.publish(self.base + '/command', json.dumps(d, separators=(',', ':')), qos=1, retain=False)
        info.wait_for_publish(timeout=4)
        if not info.is_published():
            self.record(name, 'FAIL', 'No Broker PUBACK')
            return False
        ok = self.wait(lambda: len(self.acks.get(cid, [])) > before, 8)
        if not ok:
            self.record(name, 'FAIL', f'Broker accepted publish but no firmware ACK; id={cid}')
            return False
        ack = self.acks[cid][before]
        checks = {'status': status, 'reason': reason, **expected}
        passed = all(ack.get(k) == v for k, v in checks.items())
        self.record(name, 'PASS' if passed else 'FAIL', json.dumps({'expected': checks, 'ack': ack}, ensure_ascii=False))
        return passed

    def run_commands(self):
        if not self.wait(lambda: self.state is not None and time.monotonic() - self.state_at < 10, 20):
            self.record('command suite', 'BLOCKED', 'No fresh state. Check simulator running, firmware identity and topics.')
            return
        state = dict(self.state)
        try:
            firmware_time = dt.datetime.fromisoformat(state['timestamp'].replace('Z', '+00:00'))
            gap = (dt.datetime.now(dt.timezone.utc) - firmware_time).total_seconds()
        except (KeyError, TypeError, ValueError):
            self.record('command suite', 'BLOCKED', 'Firmware clock unavailable; wait for NTP.')
            return
        if abs(gap) > 3:
            self.record('command suite', 'BLOCKED', f'Firmware clock differs by {gap:.1f}s; expiry rules remain enabled.')
            return
        if state.get('safetyOverride') is not False:
            self.record('command suite', 'BLOCKED', 'Safety override active; need 3 valid DHT readings.')
            return
        try:
            d = self.command()
            if not self.send('MANUAL', d, 'APPLIED', mode='MANUAL'):
                return
            self.send('duplicate', d, 'DUPLICATE', 'ALREADY_PROCESSED', originalStatus='APPLIED', originalReason='OK')
            conflict = dict(d, targetMode='AUTO')
            self.send('same ID different content', conflict, 'REJECTED', 'COMMAND_ID_CONFLICT')
            self.send('FAN ON', self.command('SET_STATE', 'ON'), 'APPLIED', state='ON', gpio_state='HIGH')
            self.send('FAN OFF', self.command('SET_STATE', 'OFF'), 'APPLIED', state='OFF', gpio_state='LOW')
            self.send('expired', self.command(expiresAt=utc_after(-10)), 'REJECTED', 'EXPIRED')
            self.send('far future', self.command(expiresAt=utc_after(60)), 'REJECTED', 'INVALID_EXPIRY')
            self.send('bad calendar', self.command(expiresAt='2026-02-31T00:00:00Z'), 'REJECTED', 'INVALID_EXPIRY')
            self.send('bad actuator', self.command(actuator='UNKNOWN'), 'REJECTED', 'INVALID_ACTUATOR')
            self.send('bad target', self.command('SET_STATE', 'INVALID'), 'REJECTED', 'INVALID_TARGET_STATE')
            self.send('unknown field', self.command(unexpected=True), 'REJECTED', 'INVALID_PAYLOAD')
            self.send('AUTO', self.command(target='AUTO'), 'APPLIED', mode='AUTO')
            self.send('OFF in AUTO', self.command('SET_STATE', 'OFF'), 'REJECTED', 'MODE_NOT_MANUAL')
        finally:
            self.send('cleanup AUTO', self.command(target='AUTO'), 'APPLIED', mode='AUTO')

    def close(self):
        self.client.disconnect()
        self.client.loop_stop()
        self.log.close()
        self.report.write_text(json.dumps({'host': self.host, 'port': self.port, 'base': self.base,
                                          'tests': self.rows}, ensure_ascii=False, indent=2), encoding='utf-8')
        print('Report:', self.report)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--host', default='127.0.0.1')
    ap.add_argument('--port', type=int, default=1883)
    ap.add_argument('--base', default='swiftletcare/1/1/ESP32-HOUSE01-ZONE01')
    ap.add_argument('--seconds', type=int, default=20)
    ap.add_argument('--commands', action='store_true', help='Operate FAN and restore AUTO at end')
    ap.add_argument('--output', default='test-results')
    a = ap.parse_args()
    p = Probe(a.host, a.port, a.base, a.output)
    try:
        p.client.connect(a.host, a.port, keepalive=30)
        p.client.loop_start()
        if not p.ready.wait(8):
            raise RuntimeError(p.error or 'No Broker SUBACK; check broker connection')
        print('Subscribed. Observing firmware...', flush=True)
        # Time-limited waits are in local test script, not an assistant tool wait.
        p.wait(lambda: False, a.seconds)
        p.record('telemetry observed', 'PASS' if p.telemetry else 'NOT TESTED', f'{len(p.telemetry)} distinct event IDs')
        matching = p.telemetry & p.stored
        p.record('Backend STORED ACK observed', 'PASS' if matching else 'NOT TESTED', f'{len(matching)} matching IDs; firmware buffer deletion still needs Serial evidence')
        if a.commands:
            p.run_commands()
    except (OSError, RuntimeError, ValueError) as exc:
        p.record('connection/test execution', 'FAIL', str(exc))
    finally:
        p.close()
    return 1 if any(r['status'] in ('FAIL', 'BLOCKED') for r in p.rows) else 0


if __name__ == '__main__':
    sys.exit(main())
