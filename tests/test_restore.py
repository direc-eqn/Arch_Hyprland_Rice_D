"""Restore checks use only temporary homes, fake hardware and mocked commands."""
import contextlib
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('restore', Path(__file__).resolve().parents[1] / 'restore.py')
restore = importlib.util.module_from_spec(spec)
spec.loader.exec_module(restore)


class RestoreTests(unittest.TestCase):
    def test_hardware_ignores_peripheral_batteries(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            panel = root / 'drm/card1-eDP-2/status'
            panel.parent.mkdir(parents=True)
            panel.write_text('connected\n')
            for name, kind, scope in [('mouse', 'Battery', 'Device'), ('BAT1', 'Battery', 'System'), ('ACAD', 'Mains', 'System')]:
                supply = root / 'power' / name
                supply.mkdir(parents=True)
                (supply / 'type').write_text(kind)
                (supply / 'scope').write_text(scope)
            self.assertEqual(restore.hardware(root / 'drm', root / 'power'), ('eDP-2', 'BAT1', 'ACAD'))

    def test_desktop_has_no_battery_module(self):
        source = restore.ROOT / 'waybar/config.jsonc'
        desktop = json.loads(restore.waybar_config(source, None, None))
        self.assertNotIn('battery', desktop['modules-right'])
        laptop = json.loads(restore.waybar_config(source, 'BAT1', 'ACAD'))
        self.assertEqual(laptop['battery']['bat'], 'BAT1')
        self.assertEqual(laptop['battery']['adapter'], 'ACAD')
        self.assertEqual(laptop['cffi/audio-mixer']['max-volume'], 150)

    def test_dry_run_neither_writes_nor_executes(self):
        with tempfile.TemporaryDirectory() as folder:
            home = Path(folder) / 'new-home'
            with patch.object(restore.subprocess, 'run') as run, contextlib.redirect_stdout(io.StringIO()):
                restore.main(['--home', str(home), '--dry-run'])
            run.assert_not_called()
            self.assertFalse(home.exists())

    def test_backup_and_rollback_restore_old_and_remove_new_files(self):
        with tempfile.TemporaryDirectory() as folder:
            home = Path(folder)
            (home / '.zshrc').write_text('original')
            backup = restore.Backup(home)
            backup.write(Path('.zshrc'), content=b'changed')
            backup.write(Path('.config/hypr/machine.lua'), content=b'new')
            restore.rollback(home, backup.path)
            self.assertEqual((home / '.zshrc').read_text(), 'original')
            self.assertFalse((home / '.config/hypr/machine.lua').exists())

    def test_failed_verification_rolls_back_and_preserves_local_preferences(self):
        with tempfile.TemporaryDirectory() as folder:
            home = Path(folder)
            config = home / '.config/hypr'
            config.mkdir(parents=True)
            (home / '.zshrc').write_text('original shell')
            (config / 'machine.lua').write_text('return { laptop_scale = 1.2 }')
            (config / 'local.lua').write_text('-- private overrides')

            def run(command, **kwargs):
                if command[0] == 'Hyprland':
                    raise subprocess.CalledProcessError(1, command)
                return subprocess.CompletedProcess(command, 0)

            with patch.object(restore.os, 'geteuid', return_value=1000), patch.object(restore.subprocess, 'run', side_effect=run), contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(subprocess.CalledProcessError):
                    restore.main(['--home', str(home)])
            self.assertEqual((home / '.zshrc').read_text(), 'original shell')
            self.assertEqual((config / 'machine.lua').read_text(), 'return { laptop_scale = 1.2 }')
            self.assertEqual((config / 'local.lua').read_text(), '-- private overrides')
            self.assertFalse((config / 'hyprland.lua').exists())
            self.assertFalse((home / 'Pictures/Wallpapers/wallpaper.jpeg').exists())

    def test_symlink_target_is_refused_without_touching_link_destination(self):
        with tempfile.TemporaryDirectory() as folder:
            home = Path(folder)
            original = home / 'original'
            original.write_text('leave me')
            (home / '.zshrc').symlink_to(original)
            with self.assertRaises(ValueError):
                restore.check_destination(home, Path('.zshrc'))
            self.assertEqual(original.read_text(), 'leave me')

    def test_incomplete_backup_is_refused_before_modifying_any_file(self):
        with tempfile.TemporaryDirectory() as folder:
            home = Path(folder)
            (home / '.zshrc').write_text('old')
            backup = restore.Backup(home)
            backup.write(Path('.zshrc'), content=b'new')
            (backup.path / 'manifest.json').write_text(json.dumps({'files': {'.zshrc': True, '.config/missing': True}}))
            with self.assertRaises(ValueError):
                restore.rollback(home, backup.path)
            self.assertEqual((home / '.zshrc').read_text(), 'new')

    def test_system_changes_cannot_target_another_home(self):
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            restore.parse_args(['--home', '/tmp/someone-else', '--packages'])


if __name__ == '__main__':
    unittest.main()
