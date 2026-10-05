# 💻 Install Arch and restore this desktop

This guide separates installing a working operating system from restoring the rice.
The restore script configures a normal user's desktop. Disk layout, encryption,
bootloader, Secure Boot and GPU drivers are selected for the target machine.

## 🧱 1. Prepare Arch

For a blank disk, follow the [official Arch installation guide](https://wiki.archlinux.org/title/Installation_guide)
or use `archinstall` from the official installation image. Create a normal user
with sudo access, install a kernel and firmware, choose the appropriate microcode
and GPU driver, configure networking, and make sure the machine boots from disk.
Reboot into that installation before using this repository.

The original machine uses GRUB, an Intel CPU and NVIDIA 580xx DKMS packages; those
choices are recorded in the package snapshots, not imposed on another machine.
See [hardware notes](../Packages/hardware.md). Do not restore the original
machine's partition identifiers, keys, initramfs or bootloader configuration.

## 📥 2. Get the files

From your normal user account, install the tools needed to clone and run the script:

```sh
sudo pacman -Syu --needed git python
```

Clone your copy of this repository or transfer its complete contents, including
`wallpapers/`, then enter its root directory. Run all commands below from there.
Keep SSH keys, Wi-Fi credentials, VPN activation and browser profiles in a private
backup if you want to migrate them; they are not needed to reproduce the rice.

## 🔎 3. Preview and restore

```sh
python3 restore.py --dry-run --packages --services --shell
python3 restore.py --packages --services --shell
```

The first command changes nothing. The second performs a complete Arch package
upgrade and installs `Packages/rice.txt`, backs up and copies the desktop configs,
builds the Waybar mixer, verifies the Hyprland config, enables the documented
network/Bluetooth/audio/SSH-agent services, and selects Zsh as your login shell.
Run without sudo: only the explicit system-package and system-service steps use it.

If dependencies are installed already, restore only the user configuration with:

```sh
python3 restore.py
```

Existing `hypr/machine.lua`, `hypr/local.lua` and the wallpaper are preserved by
default. Known config files are replaced after backing them up; unrelated files
are left in place. Symlinked destinations are refused, so a dotfile manager's
links cannot silently redirect writes. An interrupted run leaves its backup
manifest available for `--rollback`; failures during the config/build/verification
steps automatically restore the files already touched.

The script does not change GPU drivers, disk/boot settings, lid policy, firewall
rules or VPN accounts. Package, service and shell changes occur only with their
named options. Configuration rollback does not undo those system changes.

Supported configuration versions: Hyprland 0.56+ with Lua, Waybar with CFFI v2
(tested 0.15.0), and Neovim 0.12+ (tested 0.12.5). Private Neovim UI/native-undotree
features are guarded if unavailable. If Hyprland's Lua verification fails, use a
compatible release and consult its current documentation before logging in.

## 🖥️ 4. Review the machine profile

Edit `~/.config/hypr/machine.lua` before the next graphical login. It is generated
from the internal DRM panel and ignored by Git. Automatic scaling is the default;
set `laptop_scale = 1.2` to reproduce the original laptop's scaling.

For a new profile, options can be supplied during restoration:

```sh
python3 restore.py --scale 1.2 --output eDP-1
python3 restore.py --night-light
```

Omit `--output` for detection. On desktops without an internal display, the
internal-panel lid actions do nothing. `--nvidia-env` opts into the original
NVIDIA-specific compositor variables; it does not install a driver and is not
needed for all NVIDIA setups. Existing profiles are preserved; edit them directly
or pass `--reset-machine` to back up and regenerate them using your chosen options.

Waybar gets the detected **system** battery/adapter names. Peripheral batteries
are excluded from detection. On a desktop with no system battery, the installed
Waybar layout omits the battery module. If there are multiple system batteries,
the script chooses the first one; edit `~/.config/waybar/config.jsonc` as needed.

Night light is optional and owned by Hyprland startup when `night_light = true`.
Do not also enable `hyprsunset.service`. `--services --night-light` disables that
user-service enablement when it creates a new profile. To change an existing
profile from service ownership to Hyprland ownership, disable the user service
and set the profile field before the next login:

```sh
systemctl --user disable hyprsunset.service
```

## 🚀 5. Start the desktop

Reboot after restoring packages/services, then log in on a TTY and run:

```sh
Hyprland
```

The compositor starts Waybar, SwayNC, Hypridle, Hyprpaper, the Bluetooth/network
applets and the Polkit agent. This first login does not rely on `hyprctl reload`.
Zsh takes effect at the next login. If using a display manager, install/configure
one separately and select its Hyprland session; Ly is optional, not a dependency
of the rice. Avoid enabling multiple display managers.

The script enables these units for the next login/reboot:

| Scope | Units |
| --- | --- |
| System | `NetworkManager.service`, `bluetooth.service` |
| User | `pipewire.socket`, `pipewire-pulse.socket`, `wireplumber.service`, `ssh-agent.socket` |

It enables rather than restarts current system networking. If another network
manager is already configured, select one manager before rebooting. The original
machine also enables systemd-resolved, TLP, Thermald, UFW, ExpressVPN and NVIDIA
sleep services. Those require their own target-machine configuration and are
not silently enabled by the desktop restore script.

Check the desktop after login:

```sh
hyprctl configerrors
systemctl --user status pipewire.socket pipewire-pulse.socket wireplumber.service ssh-agent.socket
swaync-client -D
notify-send 'Desktop restored' 'Notification banners should work on any workspace.'
```

Test the volume hover mixer, file picker, lock shortcut and power menu when ready.
Do Not Disturb suppresses normal banners; switch it off using the bell's right-click.
The included [Hyprland/GTK portal packages](https://wiki.hypr.land/Hypr-Ecosystem/xdg-desktop-portal-hyprland/)
provide screen sharing and the separate file-picker backend.

## 🎨 Wallpaper, tools and optional apps

The repository includes the current wallpaper. Restoration copies it to
`~/Pictures/Wallpapers/wallpaper.jpeg` if no image exists there already. Both
Hyprpaper and Hyprlock use that path. To replace an existing wallpaper explicitly:

```sh
python3 restore.py --wallpaper /path/to/your-image.jpeg
```

GTK3/GTK4, Qt6ct, Kitty, Tmux, Btop and Starship preferences are included. Qt6ct
window geometry and other temporary UI state are omitted. XSettings preferences
are also copied for use by an XSettings daemon if you choose to run one; the
restore does not start an additional daemon.

The full rice list includes Yazi PDF/image preview and metadata tools. They are
also listed in `Packages/yazi-extras.txt` for adding them to a smaller Yazi-only setup:

```sh
sudo pacman -Syu --needed imagemagick poppler mediainfo perl-image-exiftool ffmpeg 7zip
```

`Packages/apps-optional.txt` lists extra apps. Steam and some hardware packages
need `multilib`; see the hardware notes rather than installing every snapshot.
The portable list contains no AUR packages and needs no AUR helper.

The ExpressVPN Waybar module works once the vendor client/service is installed
and you activate your account privately. Until then it reports an unavailable
client. `optional/expressvpn-client.desktop` preserves the original launcher's X11
workaround; copy it into `~/.local/share/applications/` only if that installation
path exists. Bare Hyprland does not launch `.desktop` autostart entries itself.
To start the VPN GUI at login, add a `hyprland.start` callback in your ignored
`hypr/local.lua`, launching the same command as that desktop entry.

## ↩️ Backups and rollback

Each real restoration prints a backup directory under
`~/.local/state/dotfiles-backups/`. It stores replaced files and a manifest of newly
introduced files. To preview or apply rollback, substitute the printed directory:

```sh
python3 restore.py --dry-run --rollback /path/to/printed-backup
python3 restore.py --rollback /path/to/printed-backup
```

Rollback restores previous files and removes files introduced by that run,
including generated mixer files. Log out and back in afterwards to reload the
old configuration. Backups can contain personal config data and stay outside Git.
