#!/bin/sh
# Informational menu: selecting an entry does not run it.
rofi -dmenu -i -p 'Keyboard shortcuts' -no-custom <<'KEYS'
Super + R             Applications
Super + Q             Terminal (Kitty)
Super + E             Files (Yazi)
Super + C             Close window
Super + V             Toggle floating
Super + F             Toggle fullscreen
Super + P             Toggle pseudo tiling
Super + J             Change split direction
Super + left drag     Move window
Super + right drag    Resize window
Super + arrows        Focus window
Super + 1…0           Switch workspace 1…10
Super + Shift + 1…0   Move window to workspace
Super + scroll        Previous / next workspace
Super + Shift + S     Screenshot region
Super + L             Lock screen
Super + N             Notification center
Super + Shift + N     Toggle do not disturb
Super + Shift + R     Reload Hyprland and Waybar
Super + M             Log out (ends this session)
Super + /             This guide
Volume / mute keys    Adjust sound / microphone
KEYS
# Escape is a successful dismissal.
exit 0
