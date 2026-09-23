"""No network / VPN changes: exercise JSON output, error cases, and sensor discovery."""
import contextlib
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]


def load(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / 'waybar/scripts' / f'{name}.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


vpn = load('expressvpn')
sensor = load('temperature')


class VPNTests(unittest.TestCase):
    def status(self, outputs, action='status'):
        stream = io.StringIO()
        with patch.object(vpn.shutil, 'which', return_value='/test/expressvpnctl'), \
             patch.object(vpn, 'run', side_effect=outputs) as run, contextlib.redirect_stdout(stream):
            vpn.main(action)
        output = json.loads(stream.getvalue()) if stream.getvalue() else None
        return output, run

    def test_connected_region_is_valid_json(self):
        data, run = self.status(['Connected', 'Region "A" \\ B\nSecond line'])
        self.assertEqual(data['class'], 'connected')
        self.assertIn('Region "A" \\ B\nSecond line', data['tooltip'])
        self.assertTrue(all(call.args[0][1] == 'get' for call in run.call_args_list))

    def test_disconnected_and_transitions(self):
        for state, expected in [('Disconnected', 'disconnected'), ('Connecting', 'connecting'),
                                ('Reconnecting', 'connecting'), ('Disconnecting', 'connecting'),
                                ('Interrupted', 'error')]:
            with self.subTest(state=state):
                data, _ = self.status([state])
                self.assertEqual(data['class'], expected)

    def test_missing_client(self):
        output = io.StringIO()
        with patch.object(vpn.shutil, 'which', return_value=None), contextlib.redirect_stdout(output):
            vpn.main()
        self.assertEqual(json.loads(output.getvalue())['class'], 'unavailable')

    def test_toggle_uses_correct_action(self):
        for state, action in [('Connected', 'disconnect'), ('Connecting', 'disconnect'), ('Disconnected', 'connect')]:
            _, run = self.status([state, ''], 'toggle')
            self.assertEqual(run.call_args.args[0][-1], action)

    def test_region_cancel_does_not_connect(self):
        cancelled = subprocess.CompletedProcess(['rofi'], 1, '', '')
        with patch.object(vpn.subprocess, 'run', return_value=cancelled):
            _, run = self.status(['Connected', 'region-one\nregion-two'], 'change-region')
        self.assertEqual(run.call_count, 2)

    def test_region_passed_as_single_argument(self):
        region = 'region with spaces; echo never-executed'
        selected = subprocess.CompletedProcess(['rofi'], 0, region, '')
        with patch.object(vpn.subprocess, 'run', return_value=selected):
            _, run = self.status(['Connected', region, ''], 'change-region')
        self.assertEqual(run.call_args.args[0], ['/test/expressvpnctl', 'connect', region])

    def test_cli_failure_is_not_reported_as_disconnected(self):
        failed = subprocess.CompletedProcess(['expressvpnctl'], 1, '', 'private error detail')
        with patch.object(vpn.subprocess, 'run', return_value=failed):
            with self.assertRaises(RuntimeError):
                vpn.run(['expressvpnctl', 'get', 'connectionstate'])


class SensorTests(unittest.TestCase):
    def test_dynamic_sensor_paths_and_priority(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            for name, driver, label, value in [('hwmon37', 'coretemp', 'Package id 0', '83000'),
                                               ('hwmon4', 'dell_smm', 'CPU', '49000'),
                                               ('hwmon95', 'dell_smm', 'GPU', '54000')]:
                device = root / name
                device.mkdir()
                for file, text in [('name', driver), ('temp1_label', label), ('temp1_input', value)]:
                    (device / file).write_text(text)
            self.assertEqual(sensor.temperature('cpu', root)['class'], 'critical')
            self.assertEqual(sensor.temperature('cpu', root)['text'], 'CPU 83°')
            self.assertEqual(sensor.temperature('gpu', root)['text'], 'GPU 54°')

    def test_missing_sensor_is_not_zero_degrees(self):
        with tempfile.TemporaryDirectory() as folder:
            self.assertEqual(sensor.temperature('gpu', Path(folder))['class'], 'unavailable')


if __name__ == '__main__':
    unittest.main()
