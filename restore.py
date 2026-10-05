#!/usr/bin/env python3
"""Back up and restore this rice onto an existing Arch installation."""
import argparse
from datetime import datetime
import fnmatch
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
CONFIG_DIRS = ('hypr', 'waybar', 'kitty', 'nvim', 'yazi', 'starship',
               'gtk-3.0', 'gtk-4.0', 'qt6ct', 'tmux', 'btop', 'xsettingsd')
SKIP = ('local.lua', 'machine.lua', '*.local.*', '*.so*', '*.bak*', '*.backup*',
        '*~', '*.log', '.env*', '*.pem', '*.key', 'credentials*', 'secrets*',
        'id_rsa*', 'id_ed25519*', '__pycache__', '.cache', '.git')


def packages(filename='rice.txt'):
    result = []
    for line in (ROOT / 'Packages' / filename).read_text().splitlines():
        name = line.split('#', 1)[0].strip()
        if not name:
            continue
        if not re.fullmatch(r'[a-z0-9@+_.-]+', name):
            raise ValueError(f'Invalid package name in {filename}')
        result.append(name)
    return result


def hardware(drm=Path('/sys/class/drm'), power=Path('/sys/class/power_supply')):
    output = None
    for status in sorted(drm.glob('card*-*/status')):
        name = re.sub(r'^card\d+-', '', status.parent.name)
        if name.startswith(('eDP-', 'LVDS-', 'DSI-')) and status.read_text().strip() == 'connected':
            output = name
            break
    battery, adapter = None, None
    for supply in sorted(power.glob('*')):
        kind = (supply / 'type').read_text().strip() if (supply / 'type').exists() else ''
        scope = (supply / 'scope').read_text().strip() if (supply / 'scope').exists() else ''
        if scope == 'Device' or supply.name.startswith(('hid', 'hidpp')):
            continue  # Peripheral batteries caused the previous Waybar crash.
        present = supply / 'present'
        if kind == 'Battery' and battery is None and (not present.exists() or present.read_text().strip() == '1'):
            battery = supply.name
        if kind == 'Mains' and adapter is None:
            adapter = supply.name
    return output, battery, adapter


def machine_config(output, scale, nvidia, night):
    connector = json.dumps(output) if output else 'nil'
    scale_value = json.dumps(scale) if scale == 'auto' else str(float(scale))
    return (f'-- Generated for this machine; ignored by Git. Edit before the next login.\n'
            f'return {{\n    laptop_output = {connector},\n    laptop_scale = {scale_value},\n'
            f'    nvidia_env = {str(nvidia).lower()},\n    night_light = {str(night).lower()},\n}}\n').encode()


def waybar_config(source, battery, adapter):
    # This repository uses whole-line JSONC comments; preserve all JSON strings.
    data = json.loads(re.sub(r'(?m)^\s*//[^\n]*$', '', source.read_text()))
    if battery:
        data['battery']['bat'] = battery
        if adapter:
            data['battery']['adapter'] = adapter
        else:
            data['battery'].pop('adapter', None)
    else:
        data['modules-right'] = [name for name in data['modules-right'] if name != 'battery']
    return (json.dumps(data, ensure_ascii=False, indent=2) + '\n').encode()


def config_files():
    for folder in CONFIG_DIRS:
        base = ROOT / folder
        if not base.is_dir():
            raise FileNotFoundError(f'Missing config directory: {base}')
        for source in sorted(base.rglob('*')):
            rel = source.relative_to(ROOT)
            if source.is_file() and not any(fnmatch.fnmatch(part, pattern) for part in rel.parts for pattern in SKIP):
                yield source, Path('.config') / rel
    yield ROOT / 'zsh/.zshrc', Path('.zshrc')


def check_destination(home, relative):
    if relative.is_absolute() or '..' in relative.parts:
        raise ValueError('Backup contains an unsafe path')
    dest = home / relative
    for candidate in (dest, *dest.parents):
        if candidate == home:
            break
        if candidate.is_symlink():
            raise ValueError(f'{candidate} is a symlink. Restore it manually or move the link first.')
    if dest.exists() and not dest.is_file():
        raise ValueError(f'Expected a file at {dest}')
    return dest


class Backup:
    def __init__(self, home):
        self.home = home
        stamp = datetime.now().strftime('%Y%m%d-%H%M%S-%f')
        self.path = home / '.local/state/dotfiles-backups' / stamp
        self.path.mkdir(parents=True, mode=0o700)
        self.entries = {}

    def record(self, relative):
        relative = Path(relative)
        key = str(relative)
        if key in self.entries:
            return
        dest = check_destination(self.home, relative)
        exists = dest.exists()
        if exists:
            saved = self.path / 'files' / relative
            saved.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(dest, saved)
        self.entries[key] = exists
        (self.path / 'manifest.json').write_text(json.dumps({'files': self.entries}, indent=2) + '\n')

    def write(self, relative, source=None, content=None):
        self.record(relative)
        dest = self.home / relative
        dest.parent.mkdir(parents=True, exist_ok=True)
        if source is not None:
            shutil.copy2(source, dest)
        else:
            dest.write_bytes(content)


def rollback(home, path, dry_run=False):
    entries = json.loads((path / 'manifest.json').read_text())['files']
    targets = [(check_destination(home, Path(rel)), Path(rel), existed) for rel, existed in entries.items()]
    # Validate every backup before touching any destination.
    for dest, relative, existed in targets:
        if existed and not (path / 'files' / relative).is_file():
            raise ValueError(f'Incomplete backup: {relative}')
    for dest, relative, existed in targets:
        if dry_run:
            print(f'{"Restore" if existed else "Remove introduced file"}: {dest}')
        elif existed:
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path / 'files' / relative, dest)
        elif dest.exists():
            dest.unlink()


def parse_args(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--dry-run', action='store_true', help='Show actions without writing files or running commands')
    p.add_argument('--packages', action='store_true', help='Install rice.txt packages with a complete pacman upgrade')
    p.add_argument('--services', action='store_true', help='Enable NetworkManager, Bluetooth, audio and SSH-agent units')
    p.add_argument('--shell', action='store_true', help='Select /usr/bin/zsh as your login shell')
    p.add_argument('--night-light', action='store_true', help='Enable Hyprland-managed night light in a new machine profile')
    p.add_argument('--nvidia-env', action='store_true', help='Opt into NVIDIA environment variables; does not install drivers')
    p.add_argument('--output', help='Internal panel connector; detected from DRM when omitted')
    p.add_argument('--scale', default='auto', help='Internal panel scaling: auto or a number, e.g. 1.2')
    p.add_argument('--reset-machine', action='store_true', help='Back up and regenerate an existing machine.lua')
    p.add_argument('--home', type=Path, default=Path.home(), help='Destination home directory, useful for isolated checks')
    p.add_argument('--wallpaper', type=Path, help='Install a chosen wallpaper instead of preserving an existing image')
    p.add_argument('--rollback', type=Path, help='Restore configuration files from a previous backup')
    args = p.parse_args(argv)
    if args.scale != 'auto':
        try:
            valid = 0.25 <= float(args.scale) <= 4
        except ValueError:
            valid = False
        if not valid:
            p.error('--scale must be auto or a number from 0.25 to 4')
    if (args.home.resolve() != Path.home().resolve() or args.rollback) and any((args.packages, args.services, args.shell)):
        p.error('--home/--rollback cannot be combined with package, service or login-shell changes')
    return args


def main(argv=None):
    args = parse_args(argv)
    home = args.home.expanduser().resolve()
    if os.geteuid() == 0 and not args.dry_run:
        raise ValueError('Run as your normal user; only explicit package/service options use sudo.')
    if args.rollback:
        rollback(home, args.rollback.expanduser().resolve(), args.dry_run)
        print('Rollback preview complete.' if args.dry_run else 'Configuration rollback complete. Log out and back in.')
        return

    output, battery, adapter = hardware()
    output = args.output or output
    files = list(config_files())
    profile = Path('.config/hypr/machine.lua')
    new_profile = not (home / profile).exists() or args.reset_machine
    wallpaper = Path('Pictures/Wallpapers/wallpaper.jpeg')
    wallpaper_source = (args.wallpaper.expanduser().resolve() if args.wallpaper else ROOT / 'wallpapers/wallpaper.jpeg')
    install_wallpaper = args.wallpaper is not None or not (home / wallpaper).exists()
    generated = [Path('.config/waybar/mixer') / name for name in ('audio-mixer.so', 'module.local.json', 'audio-mixer.so.new')]
    targets = [rel for _, rel in files] + generated + ([profile] if new_profile else []) + ([wallpaper] if install_wallpaper else [])
    for relative in targets:
        check_destination(home, relative)
    for source, _ in files:
        if not source.is_file():
            raise FileNotFoundError(source)
    if install_wallpaper and not wallpaper_source.is_file():
        raise FileNotFoundError(wallpaper_source)

    print(f'Destination: {home}\nInternal panel: {output or "none"}; scale: {args.scale}')
    print(f'Battery: {battery or "none (battery module removed)"}; adapter: {adapter or "not detected"}')
    print(f'{"Generate" if new_profile else "Preserve"} machine profile: {home / profile}')
    print(f'{"Install" if install_wallpaper else "Preserve"} wallpaper: {home / wallpaper}')
    print(f'Back up and copy {len(files)} config files; rebuild the Waybar mixer.')
    if args.packages:
        print('Packages: sudo pacman -Syu --needed -- ' + ' '.join(packages()))
    if args.services:
        print('Enable system units: NetworkManager.service bluetooth.service')
        print('Enable user units: pipewire.socket pipewire-pulse.socket wireplumber.service ssh-agent.socket')
    if args.shell:
        print('Login shell: chsh -s /usr/bin/zsh')
    if args.dry_run:
        print('Dry run: no files, packages, services or shell settings changed.')
        return

    if args.packages:
        subprocess.run(['sudo', 'pacman', '-Syu', '--needed', '--', *packages()], check=True)
    for command in ('cc', 'pkg-config', 'Hyprland', 'zsh'):
        if not shutil.which(command):
            raise ValueError(f'Missing {command}; install Packages/rice.txt first or pass --packages.')
    subprocess.run(['pkg-config', '--exists', 'gtk+-3.0', 'gtk-layer-shell-0', 'libpulse-mainloop-glib', 'playerctl'], check=True)
    backup = Backup(home)
    print(f'Backup: {backup.path}', flush=True)
    try:
        for source, relative in files:
            if str(relative) == '.config/waybar/config.jsonc':
                backup.write(relative, content=waybar_config(source, battery, adapter))
            else:
                backup.write(relative, source=source)
        if new_profile:
            backup.write(profile, content=machine_config(output, args.scale, args.nvidia_env, args.night_light))
        if install_wallpaper:
            backup.write(wallpaper, source=wallpaper_source)
        for relative in generated:
            backup.record(relative)
        subprocess.run([str(home / '.config/waybar/mixer/build.sh')], check=True)
        subprocess.run(['zsh', '-n', str(home / '.zshrc')], check=True)
        env = os.environ.copy()
        env.update(XDG_CONFIG_HOME=str(home / '.config'), XDG_DATA_HOME=str(home / '.local/share'), XDG_STATE_HOME=str(home / '.local/state'))
        subprocess.run(['Hyprland', '--verify-config', '-c', str(home / '.config/hypr/hyprland.lua')], env=env, check=True)
    except Exception:
        rollback(home, backup.path)
        print('Config installation failed; backed-up files restored.', file=sys.stderr)
        raise

    if args.services:
        # Enable for the next login/reboot; do not replace running system services.
        subprocess.run(['sudo', 'systemctl', 'enable', 'NetworkManager.service', 'bluetooth.service'], check=True)
        subprocess.run(['systemctl', '--user', 'enable', 'pipewire.socket', 'pipewire-pulse.socket', 'wireplumber.service', 'ssh-agent.socket'], check=True)
        if new_profile and args.night_light:
            subprocess.run(['systemctl', '--user', 'disable', 'hyprsunset.service'], check=True)
    if args.shell:
        subprocess.run(['chsh', '-s', '/usr/bin/zsh'], check=True)
    print('Restore complete. Log out and back in, or start Hyprland from a TTY after system setup.')
    print("To undo configuration changes: " + shlex.join(["python3", str(ROOT / "restore.py"), "--rollback", str(backup.path)]))


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f'Restore failed: {error}', file=sys.stderr)
        sys.exit(1)
