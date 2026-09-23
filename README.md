# Arch + Hyprland desktop

A compact teal-and-slate desktop with a readable Waybar, floating dialogs, and keyboard shortcuts you can discover from the bar. These are laptop dotfiles, with hardware-specific settings called out below.

Tested with **Hyprland 0.56.2 (Lua)** and **Waybar 0.15.0**. This is not a configuration for older Hyprland releases that use `hyprland.conf`.

## What to edit

| File | Purpose |
| --- | --- |
| `hypr/hyprland.lua` | Preferences, monitors, startup, appearance, input, shortcuts, window rules; numbered sections in one file |
| `hypr/browser-dialogs.lua` | Float Google sign-in popups after Chromium assigns their title |
| `hypr/local.lua` | Optional personal overrides, loaded last; ignored by Git |
| `hypr/scripts/shortcuts.sh` | Searchable shortcut guide shown by Super + / |
| `waybar/config.jsonc` | Module order, formats, hover drawers, click actions |
| `waybar/style.css` | Color palette at the top, spacing and widget styling below |
| `waybar/scripts/power-menu.sh` | Lock, Sleep, Shutdown and Reboot menu |
| `waybar/expressvpn.sh` | Entry point for the VPN module |
| `waybar/scripts/expressvpn.py` | VPN status and click actions, safe JSON encoding, timeouts |
| `waybar/scripts/temperature.py` | CPU/GPU sensors discovered by driver and label |
| `hypr/hypridle.conf` | Lock after 5 minutes, screen off after 5½, suspend after 10 |
| `hypr/hyprlock.conf` / `hypr/hyprpaper.conf` | Lock screen / desktop wallpaper |
| `hypr/hyprsunset.conf` | Optional night-light schedule; enable its startup explicitly |
| `kitty/`, `zsh/`, `starship/`, `nvim/`, `yazi/` | Terminal, shell, prompt, editor and file manager |
| `logind.conf.d/` | Optional system-wide lid policy; review before installing |
| `Packages/` | Package snapshots, not a minimal dependency list |
| `tests/` | VPN and temperature helper regression tests |

## Waybar controls

The full-width bar is flush with the top edge, with rounded bottom corners, a centered clock and teal active-workspace accents. Hardware details and the volume slider expand on hover to keep the normal layout compact.

| Item | Action |
| --- | --- |
| Arch icon | Open the application launcher |
| Workspace number | Switch workspace; teal marks the active workspace |
| Clock | Click to switch between compact 24-hour and detailed 12-hour date/time; hover for calendar; scroll up over the clock for the previous month, down for the next |
| CPU | Hover for RAM and CPU/GPU temperatures; click CPU or RAM to open Btop in Kitty |
| VPN | Left-click to connect/disconnect; right-click to choose a region; hover for status and connected region |
| Network | Click for NetworkManager connection settings; hover for signal quality |
| Volume | Scroll to adjust; click for Pavucontrol; right-click to mute; hover to reveal the slider |
| Battery | Hover for remaining time and power draw; amber below 25%, red below 10% while discharging |
| Stay-awake icon | Toggle idle inhibition for presentations; teal means automatic idle lock/sleep is inhibited |
| Tray | Existing network, Bluetooth and application menus |
| Bell | Open notifications; right-click toggles do not disturb |
| Question mark | Searchable keyboard shortcut guide |
| Power | Open Lock / Sleep / Shutdown / Reboot; right-click locks immediately. Shutdown and Reboot ask for confirmation |

Sleep uses `systemctl suspend`; the existing Hypridle before-sleep handler locks the session. Escape dismisses the power menu without taking action.

VPN status queries are read-only. Connecting and disconnecting happen only on clicks. The ExpressVPN GUI client or its background mode must be available for control commands. An unavailable client is shown explicitly rather than falsely reporting a disconnected VPN. Regions appear only in the tooltip, keeping the bar compact.

Temperature readings use `coretemp` / AMD CPU sensors and labeled GPU sensors (including Dell's `GPU` sensor). Unsupported or unavailable sensors show a dash rather than a misleading zero. Red starts at 80°C. No NVIDIA polling process wakes a sleeping GPU just to populate the bar.

## Keyboard shortcuts

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

## Notification pop-ups

SwayNC starts with the Hyprland session. Its default notification window uses the overlay layer, so desktop banners appear on the currently visible workspace rather than belonging to the sender's workspace. Verified with a normal-priority banner across workspaces 3, 1 and 2.

Use Super + N or the bell for notification history. Right-clicking the bell or Super + Shift + N toggles Do Not Disturb, which suppresses normal banners. Check `swaync-client -D` and `swaync-client -I`; both should print `false` when banners are wanted. Apps and browser websites must also allow notifications in their own settings.

Test from a desktop terminal with `notify-send "Notification test" "This should appear on the current workspace"`.

## Install or restore

Back up existing files before copying. Run these commands from the repository root.

Core packages for the desktop on Arch (package availability can vary):

```sh
sudo pacman -S --needed hyprland waybar kitty rofi yazi btop python \
  hypridle hyprlock hyprpaper hyprshot hyprpolkitagent swaync \
  networkmanager network-manager-applet blueman pavucontrol wireplumber \
  ttf-jetbrains-mono-nerd
```

Install and enable your audio stack, NetworkManager, Bluetooth and an appropriate `xdg-desktop-portal` backend separately if they are not already configured. `expressvpnctl` comes from the ExpressVPN client and is optional. `hyprsunset` is optional. Other terminal/editor configs have additional dependencies in the package snapshots.

```sh
backup="$HOME/.local/state/dotfiles-backups/$(date +%Y%m%d-%H%M%S)"
mkdir -p "$backup" "$HOME/.config"
for dir in hypr waybar; do
  if [ -d "$HOME/.config/$dir" ]; then
    cp -a "$HOME/.config/$dir" "$backup/"
  fi
  cp -a "$dir" "$HOME/.config/"
done
chmod +x "$HOME/.config/waybar/expressvpn.sh" "$HOME/.config/waybar/scripts/power-menu.sh" "$HOME/.config/hypr/scripts/shortcuts.sh"
Hyprland --verify-config -c "$HOME/.config/hypr/hyprland.lua"
hyprctl reload
pkill -USR2 -x waybar
```

If Waybar is not running, start it with `waybar`. Startup apps only launch when the Hyprland session starts; reloading does not start missing daemons. The supplied bar commands assume configs are installed under `~/.config`.

Copy `kitty`, `nvim`, `yazi`, and `starship` to `~/.config` only if you want those configurations too. Back up `~/.zshrc` before replacing it with `zsh/.zshrc`.

### Hardware settings to review

- **Display:** `eDP-1`, scale `1.2`, near the top of `hyprland.lua`. Use `hyprctl monitors` to find your names. Closing the lid disables the panel only when another monitor is active; reopening enables it.
- **Lid policy:** the optional logind override ignores lid events system-wide. With it installed, closing the lid on the laptop alone does **not** immediately suspend; the configured idle timers still apply. Do not install it if you prefer systemd's default lid suspend behavior.
- **NVIDIA:** the two explicitly marked NVIDIA environment variables are preserved for this laptop. Remove them on systems without NVIDIA.
- **Wallpaper:** provide `~/Pictures/Wallpapers/wallpaper.jpeg`, or edit the two wallpaper paths. Images are not bundled.
- **Screenshots:** saved to `~/Pictures/Screenshots`.
- **Night light:** add `hyprsunset` to the autostart list to use its existing schedule.
- **Brightness:** brightness controls are not bound because `brightnessctl` is not installed in the tested setup.

## Validate and troubleshoot

```sh
Hyprland --verify-config -c "$PWD/hypr/hyprland.lua"
python3 -m unittest discover -s tests -v
lua tests/test_browser_dialogs.lua
sh -n waybar/expressvpn.sh
sh -n waybar/scripts/power-menu.sh
sh -n hypr/scripts/shortcuts.sh
hyprctl configerrors
```

For Waybar diagnostics, stop the existing instance and run `waybar -l debug` from a terminal. Check for missing commands or modules. Hover CPU / volume to check their expanded layouts. VPN tests use mocks and do not change your connection. Power, logout, lock and suspend actions should be checked manually when convenient.

Rollback: copy the backed-up `hypr` and `waybar` contents into `~/.config`, reload Hyprland, then restart Waybar. Newly introduced helpers are inert if the restored config does not reference them. Keep backups outside the repository.

## Privacy and maintenance

No credentials are required in these files. Keep VPN activation files, API tokens, SSH private keys and `.env` files outside the repository. `.gitignore` excludes common credential files, `local.lua`, backups, logs and Python caches; it cannot remove files already committed.

The September 2026 refresh checked tracked files and reachable Git history for common credential/token patterns and found no matches. Personal absolute home paths were replaced in the current configs, and an accidentally tracked Hyprland backup was removed. Older commits still contain the previous paths and backup; history was not rewritten. Pattern scanning is not a guarantee that arbitrary secrets cannot be present. Review `git diff --cached` before publishing.

Refresh package snapshots if desired:

```sh
pacman -Qqen > Packages/pkglist-repo.txt
pacman -Qqem > Packages/pkglist-aur.txt
```

Reference: [Hyprland configuration](https://wiki.hypr.land/Configuring/Start/) and [Waybar documentation](https://github.com/Alexays/Waybar/wiki).

## License

See [LICENSE](LICENSE): GNU General Public License, version 3. The previous README's MIT label was incorrect; the license file itself is unchanged.
