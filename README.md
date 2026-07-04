# Arch_Hyprland_Rice_D
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

