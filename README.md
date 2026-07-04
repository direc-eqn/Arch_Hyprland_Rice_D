# 🏔️ Arch Hyprland Rice

This repository contains my **Arch Linux + Hyprland** configuration (rice).

If you want to recreate this setup on another machine, you'll need two things:

* 📦 A list of installed packages
* ⚙️ Your configuration files

---

# 📦 Export Installed Packages

To avoid reinstalling unnecessary dependencies, export **official repository packages** and **AUR packages** separately.

## 1. Official Repository Packages

Save all explicitly installed packages from the official Arch repositories:

```bash
pacman -Qeq > pkglist-repo.txt
```

## 2. AUR / Foreign Packages

Save all explicitly installed packages that are not from the official repositories (typically AUR packages):

```bash
pacman -Qmq > pkglist-aur.txt
```

This creates two files:

```
pkglist-repo.txt
pkglist-aur.txt
```

---

# ⚙️ Back Up Configuration Files

To fully recreate your desktop, back up both your system-wide and user-specific configuration files.

## System Configuration

Archive the `/etc` directory:

```bash
sudo tar -czvf etc_backup.tar.gz /etc
```

## User Configuration

Archive your `~/.config` directory:

```bash
tar -czvf config_backup.tar.gz ~/.config
```

This produces:

```
etc_backup.tar.gz
config_backup.tar.gz
```

---

# 📁 Files to Keep

Your backup should contain:

```
pkglist-repo.txt
pkglist-aur.txt
etc_backup.tar.gz
config_backup.tar.gz
```

---

# 🚀 Restoring on a Fresh Arch Installation

## Install Official Packages

```bash
sudo pacman -S --needed - < pkglist-repo.txt
```

## Install AUR Packages

Using an AUR helper such as `yay`:

```bash
yay -S --needed - < pkglist-aur.txt
```

## Restore Configuration Files

Restore your configuration archives:

```bash
sudo tar -xzvf etc_backup.tar.gz -C /
tar -xzvf config_backup.tar.gz -C ~
```

> **Note:** Restoring `/etc` may overwrite existing system configuration files. Review the archive before extracting if you're restoring onto an already configured system.

---

# 📌 Notes

* Export package lists **before reinstalling** your system.
* Keep backups somewhere safe (GitHub, external drive, NAS, or cloud storage).
* This repository only contains configuration files. Package lists and backups should be generated from your own installation.

Happy ricing! 🎉# Arch_Hyprland_Rice_D

To replicate an Arch Linux system, save both your installed package lists and system configuration files. 

Run the following commands to export all explicitly installed repository and AUR packages, then back up 

your /etc and ~/.config directories.

1. Save Package Lists

   To successfully replicate your setup without carrying over unnecessary bloat,

   it is best to separate official packages from AUR or manually installed foreign packages.

   Official Packages: Extract a list of packages explicitly installed from official repos:

    --- pacman -Qeq > pkglist-repo.txt

   AUR / Foreign Packages: Extract a list of packages explicitly installed from the AUR:

    --- pacman -Qmq > pkglist-aur.txt

2. Back Up Configurations

   Your /etc directory holds system-wide configurations, while ~/.config holds user-specific settings. You can back them up using a utility like tar:

    --- tar -czvf etc_backup.tar.gz /etc

    --- tar -czvf config_backup.tar.gz ~/.config

look at the read.md

can you make it nicer # Arch_Hyprland_Rice_D

To replicate an Arch Linux system, save both your installed package lists and system configuration files. 

Run the following commands to export all explicitly installed repository and AUR packages, then back up 

your /etc and ~/.config directories.


1. Save Package Lists

   To successfully replicate your setup without carrying over unnecessary bloat,

   it is best to separate official packages from AUR or manually installed foreign packages.

   Official Packages: Extract a list of packages explicitly installed from official repos:

    --- pacman -Qeq > pkglist-repo.txt

   AUR / Foreign Packages: Extract a list of packages explicitly installed from the AUR:

    --- pacman -Qmq > pkglist-aur.txt


2. Back Up Configurations

   Your /etc directory holds system-wide configurations, while ~/.config holds user-specific settings. You can back them up using a utility like tar:

    --- tar -czvf etc_backup.tar.gz /etc

    --- tar -czvf config_backup.tar.gz ~/.config


look at the read.md


can you make it nicer 
