# 🖥️ Hardware packages are selected per machine

`rice.txt` intentionally excludes GPU drivers, CPU microcode, kernels, bootloaders
and power policies. The `pkglist-*.txt` files remain reference snapshots of the
original installation, including its NVIDIA 580xx AUR packages and Intel CPU.

- Use the appropriate Intel or AMD microcode for the target CPU.
- Select GPU drivers for the actual GPU and kernel. Do not install the original
  NVIDIA 580xx packages on every new machine. DKMS drivers need matching kernel headers.
- The original machine used NVIDIA initramfs modules, a Nouveau blacklist and
  NVIDIA suspend/resume/hibernate services. Configure these only when required
  by the target driver's documentation; do not copy its boot files blindly.
- `--nvidia-env` in `restore.py` only changes compositor environment variables.
  It does not install or configure a GPU driver.
- The original laptop also enables TLP and Thermald. These are optional policies
  to review on new hardware, not desktop dependencies.
- The original machine has `multilib` enabled for Steam and some NVIDIA libraries.
  Enable that repository in `/etc/pacman.conf` only when your selected packages need it,
  then perform a complete package upgrade. The portable rice list does not require it.

References: [Hyprland NVIDIA guide](https://wiki.hypr.land/Nvidia/),
[Arch microcode guide](https://wiki.archlinux.org/title/Microcode), and
[Arch graphics guide](https://wiki.archlinux.org/title/Xorg#Driver_installation).
