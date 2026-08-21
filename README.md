# 🏔️ Arch Hyprland Rice

A clean, modular, and performance-focused **Arch Linux + Hyprland** dotfiles setup built for productivity and aesthetic workflows.

![OS](https://img.shields.io/badge/OS-Arch%20Linux-blue?logo=archlinux&logoColor=white)
![WM](https://img.shields.io/badge/WM-Hyprland-blue?logo=hyprland&logoColor=white)
![Shell](https://img.shields.io/badge/Shell-Zsh-green?logo=gnu-bash&logoColor=white)
![Editor](https://img.shields.io/badge/Editor-Neovim-brightgreen?logo=neovim&logoColor=white)

---

## 📸 Key Components Overview

| Component | Software / Tool | Description |
| :--- | :--- | :--- |
| **Window Manager** | [Hyprland](https://hyprland.org/) | Dynamic tiling Wayland compositor configured with Lua (`hyprland.lua`) |
| **Status Bar** | [Waybar](https://github.com/Alexays/Waybar) | Top status bar with hardware monitors & custom ExpressVPN integration |
| **Terminal** | [Kitty](https://sw.kovidgoyal.net/kitty/) | GPU-accelerated terminal with font ligatures & 80% opacity |
| **Shell & Prompt** | [Zsh](https://www.zsh.org/) + [Starship](https://starship.rs/) | Fast shell setup with `zoxide`, `fzf` previews (`bat`/`eza`), & `yazi` integration |
| **Text Editor** | [Neovim](https://neovim.io/) | Modular Lua configuration (`init.lua`) with native autocomplete & theme sync |
| **File Manager** | [Yazi](https://yazi-rs.github.io/) | Modern terminal file manager with directory navigation wrapper |
| **Lock & Idle** | Hyprlock / Hypridle / Hyprpaper / Hyprsunset | Screen locker, idle daemon, wallpaper daemon, & night light filter |
| **System Rules** | Systemd Logind | Custom lid switch handling in `logind.conf.d/` |

---

## 📁 Repository Structure

```text
.
├── hypr/               # Hyprland setup (hyprland.lua, hyprlock, hyprpaper, hypridle, hyprsunset)
├── waybar/             # Waybar bar config (config.jsonc, style.css, expressvpn.sh)
├── kitty/              # Kitty terminal configuration (kitty.conf)
├── nvim/               # Modular Neovim Lua setup (init.lua, lua/*.lua)
├── yazi/               # Yazi file manager config (yazi.toml, keymap.toml)
├── starship/           # Starship prompt theme (starship.toml)
├── zsh/                # Zsh environment & aliases (.zshrc)
├── logind.conf.d/      # Systemd logind rules (99-laptop-lid.conf)
└── Packages/           # Exported official & AUR package lists
    ├── pkglist-repo.txt
    └── pkglist-aur.txt
```

---

## 🚀 Installation & Restoration Guide

Follow these steps to restore this configuration on a fresh Arch Linux installation.

### Prerequisites

Ensure `git`, `base-devel`, and an AUR helper (such as `yay`) are installed.

### 1. Install Official & AUR Packages

Restore all explicitly installed packages using the pre-compiled package lists:

```bash
# Official repository packages
sudo pacman -S --needed - < Packages/pkglist-repo.txt

# AUR packages (using yay)
yay -S --needed - < Packages/pkglist-aur.txt
```

### 2. Deploy Configuration Files

Copy or symlink configuration directories to their target system paths:

```bash
# User configurations (~/.config)
mkdir -p ~/.config
cp -r hypr waybar kitty nvim yazi starship ~/.config/

# Zsh configuration
cp zsh/.zshrc ~/.zshrc

# Systemd logind overrides (handles laptop lid behavior)
sudo mkdir -p /etc/systemd/logind.conf.d/
sudo cp logind.conf.d/99-laptop-lid.conf /etc/systemd/logind.conf.d/
```

> [!NOTE]
> Symlinking (`ln -s`) instead of copying allows you to easily track live changes using git.

---

## ⚙️ Feature Highlights

### 🪟 Hyprland (Lua Config)
- Configured using native Lua (`hyprland.lua`) for dynamic logic and cleaner organization.
- Automatic lid-switch detection (`eDP-1` display disable/enable on laptop close/open).
- Autostart orchestration for status bars (`waybar`), wallpapers (`hyprpaper`), notifications (`swaync`), idle daemons (`hypridle`), and network/bluetooth applets.

### 📊 Custom Waybar
- **Hardware Sensors**: Displays CPU and GPU temperatures via system `hwmon` interfaces, plus CPU & RAM load with interactive click handlers targeting `btop`/`htop`.
- **ExpressVPN Integration**: Custom script (`expressvpn.sh`) showing connection status with left-click toggle and right-click region selection.
- **Audio & Power**: PulseAudio volume slider and battery status with dynamic icon updates.

### 🐚 Zsh + FZF + Zoxide
- Interactive file and directory search with `fzf` using `bat` line previews and `eza` tree previews.
- Smart navigation with `zoxide` (`cd` aliased to `z`).
- Seamless `yazi` integration: exiting `yazi` automatically updates the shell's active working directory.

### 📝 Neovim Setup
- Written in clean Lua (`init.lua` importing modular components in `lua/`).
- Native autocomplete enabled (`vim.o.autocomplete = true`).
- Automatic transparent background sync matching Kitty terminal's `0.80` opacity.

---

## 📦 Maintenance & Exporting Package Lists

If you make changes to your installed packages, update the repository package lists before committing:

```bash
# Official packages
pacman -Qeq > Packages/pkglist-repo.txt

# AUR / foreign packages
pacman -Qmq > Packages/pkglist-aur.txt
```

---

## 📄 License

Distributed under the [MIT License](LICENSE).
