# 🏔️ Arch + Hyprland desktop

![OS](https://img.shields.io/badge/OS-Arch_Linux-1793d1?style=flat-square)
![Hyprland](https://img.shields.io/badge/Hyprland-0.56%2B_%28Lua%29-00b5b5?style=flat-square)
![Waybar](https://img.shields.io/badge/Waybar-CFFI_v2-9b59b6?style=flat-square)
![Audio](https://img.shields.io/badge/Audio-PipeWire_%2F_PulseAudio-e67e22?style=flat-square)
![Restore](https://img.shields.io/badge/Restore-Backups_%2B_Rollback-4cbb17?style=flat-square)
![Preview](https://img.shields.io/badge/Preview-Dry_Run-0088cc?style=flat-square)
![Machine settings](https://img.shields.io/badge/Machine_Settings-Local_%26_Ignored-4cbb17?style=flat-square)


A compact teal-and-slate desktop with a readable Waybar, floating dialogs, and keyboard shortcuts you can discover from the bar. A restore script detects laptop/desktop hardware, backs up existing files, and keeps machine preferences outside Git.

Tested with **Hyprland 0.56.2 (Lua)**, **Waybar 0.15.0**, and **Neovim 0.12.5**. This is not a configuration for older Hyprland releases that use `hyprland.conf`.

## 🗂️ What to edit

| File | Purpose |
| --- | --- |
| `hypr/hyprland.lua` | Preferences, monitors, startup, appearance, input, shortcuts, window rules; numbered sections in one file |
| `hypr/browser-dialogs.lua` | Float Google sign-in popups after Chromium assigns their title |
| `restore.py` | Preview, back up, restore, build and roll back the desktop |
| `docs/INSTALL.md` | Fresh Arch preparation, package/service setup and first login |
| `hypr/machine.example.lua` | Portable display, NVIDIA and night-light preferences; restore generates ignored `machine.lua` |
| `hypr/local.lua` | Optional personal overrides, loaded last; ignored by Git |
| `hypr/scripts/shortcuts.sh` | Searchable shortcut guide shown by Super + / |
| `waybar/config.jsonc` | Module order, formats, hover drawers, click actions |
| `waybar/style.css` | Color palette at the top, spacing and widget styling below |
| `waybar/mixer/` | Native hover mixer: source, build script and isolated audio tests |
| `waybar/scripts/power-menu.sh` | Lock, Sleep, Shutdown and Reboot menu |
| `waybar/expressvpn.sh` | Entry point for the VPN module |
| `waybar/scripts/expressvpn.py` | VPN status and click actions, safe JSON encoding, timeouts |
| `waybar/scripts/temperature.py` | CPU/GPU sensors discovered by driver and label |
| `hypr/hypridle.conf` | Lock after 5 minutes, screen off after 5½, suspend after 10 |
| `hypr/hyprlock.conf` / `hypr/hyprpaper.conf` | Lock screen / desktop wallpaper |
| `hypr/hyprsunset.conf` | Optional night-light schedule; enable its startup explicitly |
| `kitty/`, `zsh/`, `starship/`, `nvim/`, `yazi/` | Terminal, shell, prompt, editor and file manager |
| `logind.conf.d/` | Optional system-wide lid policy; review before installing |
| `gtk-3.0/`, `gtk-4.0/`, `qt6ct/` | GTK and Qt appearance settings |
| `tmux/`, `btop/`, `xsettingsd/` | Terminal tools and optional XSettings preferences |
| `wallpapers/` | Current desktop and lock-screen wallpaper |
| `Packages/` | Portable rice list, optional apps, Yazi tool reference, hardware notes and original snapshots |
| `tests/` | Restore/rollback, VPN, temperature and browser-dialog regression checks |

## 🎛️ Waybar controls

The full-width bar is flush with the top edge, with rounded bottom corners, a centered clock and teal active-workspace accents. Hardware details expand on hover. Hovering over volume opens a mixer below the bar with master and per-stream sliders.

| Item | Action |
| --- | --- |
| 🚀 Arch icon | Open the application launcher |
| 🖥️ Workspace number | Switch workspace; teal marks the active workspace |
| 📅 Clock | Click to switch between compact 24-hour and detailed 12-hour date/time; hover for calendar; scroll up over the clock for the previous month, down for the next |
| ⚙️ CPU | Hover for RAM and CPU/GPU temperatures; click CPU or RAM to open Btop in Kitty |
| 🛡️ VPN | Left-click to connect/disconnect; right-click to choose a region; hover for status and connected region |
| 🌐 Network | Click for NetworkManager connection settings; hover for signal quality |
| 🔊 Volume | Hover or click for the master/app mixer; drag each slider to adjust that stream, or use its mute button. Scroll over the bar icon adjusts master volume; right-click mutes the output |
| 🔋 Battery | Hover for remaining time and power draw; amber below 25%, red below 10% while discharging |
| ☕ Stay-awake icon | Toggle idle inhibition for presentations; teal means automatic idle lock/sleep is inhibited |
| 🧩 Tray | Existing network, Bluetooth and application menus |
| 🔔 Bell | Open notifications; right-click toggles do not disturb |
| ❓ Question mark | Searchable keyboard shortcut guide |
| ⏻ Power | Open Lock / Sleep / Shutdown / Reboot; right-click locks immediately. Shutdown and Reboot ask for confirmation |

Sleep uses `systemctl suspend`; the existing Hypridle before-sleep handler locks the session. Escape dismisses the power menu without taking action.

VPN status queries are read-only. Connecting and disconnecting happen only on clicks. The ExpressVPN GUI client or its background mode must be available for control commands. An unavailable client is shown explicitly rather than falsely reporting a disconnected VPN. Regions appear only in the tooltip, keeping the bar compact.

Temperature readings use `coretemp` / AMD CPU sensors and labeled GPU sensors (including Dell's `GPU` sensor). Unsupported or unavailable sensors show a dash rather than a misleading zero. Red starts at 80°C. No NVIDIA polling process wakes a sleeping GPU just to populate the bar.

## ⌨️ Keyboard shortcuts

`Super` is the Windows / logo key. Existing bindings are retained, with fullscreen, notifications, audio keys and a help menu added.

| Shortcut | Action |
| --- | --- |
| Super + R / Q / E | Applications / terminal / Yazi files |
| Super + C | Close focused window |
| Super + V | Toggle floating for any window |
| Super + F | Toggle fullscreen |
| Super + P / J | Pseudo tiling / change split direction |
| Super + left / right mouse drag | Move / resize window |
| Super + arrow keys | Move focus |
| Super + 1…9, 0 | Workspaces 1…10 |
| Super + Shift + 1…9, 0 | Move focused window to workspace |
| Super + mouse wheel | Previous / next workspace |
| Three-finger horizontal swipe | Switch workspace |
| Super + Shift + S | Select a screenshot region |
| Super + L | Lock screen |
| Super + N / Shift + N | Notification center / do not disturb |
| Super + Shift + R | Reload Hyprland and Waybar |
| Super + / | Shortcut guide |
| Super + M | Log out; ends the desktop session |
| Volume / mute / microphone-mute keys | Control audio, including while locked |

Modal dialogs, desktop file-chooser portals, and common file-picker titles float over the tiled layout. Chromium / Chrome Google sign-in popups that start with an empty or Untitled title are handled when their title changes, even when the browser omits dialog metadata. They open centered at up to 560 × 680 logical pixels and can be moved/resized normally. Regular browser windows stay tiled. Other providers or translated titles may need another targeted match; Super + V remains a manual fallback.

## 🔔 Notification pop-ups

SwayNC starts with the Hyprland session. Its default notification window uses the overlay layer, so desktop banners appear on the currently visible workspace rather than belonging to the sender's workspace. Verified with a normal-priority banner across workspaces 3, 1 and 2.

Use Super + N or the bell for notification history. Right-clicking the bell or Super + Shift + N toggles Do Not Disturb, which suppresses normal banners. Check `swaync-client -D` and `swaync-client -I`; both should print `false` when banners are wanted. Apps and browser websites must also allow notifications in their own settings.

Test from a desktop terminal with `notify-send "Notification test" "This should appear on the current workspace"`.

## 📦 Install or restore

For a new PC, first prepare a bootable Arch installation with a normal user, sudo,
networking and the appropriate GPU driver. Then clone/copy this repository and run
these commands from its root **as that user**:

```sh
python3 restore.py --dry-run --packages --services --shell
python3 restore.py --packages --services --shell
```

The preview changes nothing. The restore installs the portable `Packages/rice.txt`
with a complete package upgrade, backs up and copies all rice configs, supplies the
wallpaper, detects display/battery names, rebuilds the mixer, checks Hyprland,
enables network/Bluetooth/audio/SSH-agent units and selects Zsh. The generated
`~/.config/hypr/machine.lua` keeps display scale and NVIDIA preferences private.

To restore config files only when packages are already installed:

```sh
python3 restore.py
```

Existing machine preferences, personal `local.lua` and wallpaper are preserved.
Each run prints a backup path; `python3 restore.py --rollback /path/to/backup`
restores replaced files and removes introduced files. Compilation/config errors
roll back configuration changes automatically. Package, service and shell changes
require their explicit options and are not undone by config rollback.

Log out/reboot after the full restore, then run `Hyprland` from a TTY or choose it
in your configured display manager. See [💻 the complete install guide](docs/INSTALL.md)
for first boot, optional apps, night light, service details and wallpaper replacement.

### 🖥️ Hardware settings

- **Display:** generated `hypr/machine.lua` detects the internal panel and defaults
  to automatic scaling. Set `laptop_scale = 1.2` to reproduce the original laptop.
- **GPU:** NVIDIA environment variables are opt-in through `nvidia_env`; select
  drivers separately using [hardware notes](Packages/hardware.md).
- **Battery:** restoration uses the detected system battery, excluding peripheral
  batteries. Desktops without a battery omit that Waybar module.
- **Night light:** set `night_light = true` or use `--night-light` for a new profile.
  Hyprland owns its startup; do not also enable the Hyprsunset user service.
- **Lid:** the optional logind override is available but is not installed
  automatically. It changes system-wide lid behavior; review it before use.
- **Wallpaper:** the bundled image is copied only when the current wallpaper is
  absent. `--wallpaper /path/to/image.jpeg` explicitly replaces it with a backup.
- **Snapshots:** original package snapshots include Intel/NVIDIA and games; use
  `Packages/rice.txt` for portable desktop dependencies.

## 🛠️ Validate and troubleshoot

```sh
Hyprland --verify-config -c "$PWD/hypr/hyprland.lua"
python3 -m unittest discover -s tests -v
lua tests/test_browser_dialogs.lua
sh -n waybar/expressvpn.sh
sh -n waybar/scripts/power-menu.sh
sh -n hypr/scripts/shortcuts.sh
hyprctl configerrors
```

For Waybar diagnostics, stop the existing instance and run `waybar -l debug` from a terminal. Check for missing commands or modules. Hover CPU / volume to check their expanded layouts. Restore tests use temporary directories and mocks; they do not change your packages or services. Run `python3 waybar/mixer/test-mixer.py` inside your desktop session to test the mixer with a disposable silent stream; it temporarily opens a test popup without modifying other apps' volumes. VPN tests use mocks and do not change your connection. Power, logout, lock and suspend actions should be checked manually when convenient.

Rollback: use `python3 restore.py --rollback /path/to/printed-backup`, then log out and back in. Older manual backups can still be copied into place. Keep backups outside the repository.

## 🔐 Privacy and maintenance

No credentials are required in these files. Keep VPN activation files, API tokens, SSH private keys and `.env` files outside the repository. `.gitignore` excludes common credential files, `local.lua`, generated `machine.lua`, backups, logs and Python caches; it cannot remove files already committed.

The September 2026 refresh checked tracked files and reachable Git history for common credential/token patterns and found no matches. Personal absolute home paths were replaced in the current configs, and an accidentally tracked Hyprland backup was removed. Older commits still contain the previous paths and backup; history was not rewritten. Pattern scanning is not a guarantee that arbitrary secrets cannot be present. Review `git diff --cached` before publishing.

Refresh package snapshots if desired:

```sh
pacman -Qqen > Packages/pkglist-repo.txt
pacman -Qqem > Packages/pkglist-aur.txt
```

Reference: [Hyprland configuration](https://wiki.hypr.land/Configuring/Start/) and [Waybar documentation](https://github.com/Alexays/Waybar/wiki).

## 📄 License

Code and configuration: [LICENSE](LICENSE), GNU General Public License, version 3. The previous README's MIT label was incorrect; the license file itself is unchanged.

### 🔋 Battery crash workaround (October 2026)

Waybar 0.15.0 crashed in `Battery::refreshBatteries()` while attempting to watch a disappearing Logitech `hidpp_battery_*` device. The source config explicitly selects `BAT0` and adapter `AC`, avoiding peripheral battery discovery. The restore script replaces these with the target machine's detected system battery and adapter, or removes the module from a desktop layout.

Hyprland's autostart writes Waybar output to `~/.local/state/waybar.log` (replaced at the next session start). Inspect this log and `coredumpctl list waybar` if the bar exits again. This change addresses the observed battery-watch failure, not every possible Waybar crash.

### 🎚️ Hover audio mixer

Hover over the volume icon for about 0.2 seconds to open the mixer. Move onto it to adjust master volume or any active playback stream. Move away to close it. The popup is attached to the actual bar widget, so there are no hard-coded mouse coordinates or cursor polling. Audio updates use PulseAudio subscriptions (also supported by PipeWire-Pulse).

Browsers may expose multiple playback streams; each gets a slider. If a browser combines tabs into one stream, those tabs share its volume. Apps without a current playback stream will not appear. Master and app sliders, plus scrolling over the volume icon, support 0–150%. The slider tick marks 100%; boosted percentages are highlighted. Change `cffi/audio-mixer.max-volume` in `waybar/config.jsonc` to a value from 100 to 200. Amplification can distort loud audio. Microphone volume is not changed by this mixer.

The **Now playing** section shows tab/media titles supplied through MPRIS (using `playerctl`), and updates when the media changes. Stream labels use `media.title` or `media.name` when available. Chromium currently supplies only “Playback” for its audio streams, so these are labelled with a stream number; its player-wide media title is displayed separately. Chromium does not expose a reliable tab-to-stream mapping, and tabs without media-session metadata cannot be named by this mixer. Media titles are read in memory and are not saved to the repository or a log.

The module uses Waybar's CFFI v2 interface and GTK3/GTK Layer Shell. The restore script builds it automatically; rebuild after incompatible library or Waybar upgrades:

```sh
~/.config/waybar/mixer/build.sh
```

`build.sh` generates `audio-mixer.so` and `module.local.json` in that directory. The JSON contains the absolute library path required by Waybar; both generated files are ignored by Git. Only source and tests are tracked. Restart Waybar after rebuilding (a full restart loads the new library). No root access or package rebuild is needed. If you need device-routing settings, run `pavucontrol` from the application launcher.
