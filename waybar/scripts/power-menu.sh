#!/bin/sh
# Rofi returns a row index, so display labels are never evaluated as commands.
choice=$(printf '%s\n' 'Lock' 'Sleep' 'Shutdown' 'Reboot' |
    rofi -dmenu -i -no-custom -format i -p 'Power' \
         -theme-str 'window { width: 320px; } listview { lines: 4; }') || exit 0

confirm() {
    answer=$(printf '%s\n' 'Cancel' "$1" |
        rofi -dmenu -no-custom -format i -selected-row 0 -p "$1?" \
             -theme-str 'window { width: 320px; } listview { lines: 2; }') || return 1
    [ "$answer" = 1 ]
}

case "$choice" in
    0) pidof hyprlock >/dev/null || hyprlock ;;
    # Hypridle locks before sleep using the existing before_sleep_cmd handler.
    1) systemctl suspend ;;
    2) confirm Shutdown || exit 0; systemctl poweroff ;;
    3) confirm Reboot || exit 0; systemctl reboot ;;
    *) exit 0 ;;
esac
result=$?
if [ "$result" -ne 0 ]; then
    notify-send 'Power menu' 'The action failed. Check your session permissions.'
fi
exit "$result"
