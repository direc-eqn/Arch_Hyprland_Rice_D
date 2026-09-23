#!/usr/bin/env python3
"""Waybar status and explicit click actions for the ExpressVPN GUI client CLI."""
import json
import shutil
import subprocess
import sys


def run(args, timeout=8, input=None):
    result = subprocess.run(args, input=input, text=True, capture_output=True, timeout=timeout)
    if result.returncode:
        raise RuntimeError("Command failed")
    return result.stdout.strip()


def emit(text, state, tooltip):
    print(json.dumps({"text": text, "class": state, "tooltip": tooltip}))


def main(action="status"):
    if action not in ("status", "toggle", "change-region"):
        raise ValueError("Unknown action")
    cli = shutil.which("expressvpnctl")
    if not cli:
        emit("󰖂 N/A", "unavailable", "ExpressVPN client is not installed")
        return
    # Reading status never connects, disconnects, or changes VPN settings.
    state = run([cli, "get", "connectionstate"])
    busy = {"Connecting", "Reconnecting", "DisconnectingToReconnect", "Disconnecting"}
    if action == "toggle":
        command = "disconnect" if state == "Connected" or state in busy else "connect"
        run([cli, command], timeout=30)
    elif action == "change-region":
        regions = run([cli, "get", "regions"])
        if not regions:
            return
        # Cancellation is normal. Pass the selected region as one argument, never a shell command.
        choice = subprocess.run(["rofi", "-dmenu", "-i", "-p", "VPN region"],
                                input=regions, text=True, capture_output=True)
        selected = choice.stdout.strip()
        if choice.returncode == 0 and selected:
            if selected not in regions.splitlines():
                raise ValueError("Choose a region from the list")
            run([cli, "connect", selected], timeout=30)
    elif state == "Connected":
        region = run([cli, "get", "region"])
        emit("󰖂 VPN", "connected", f"Connected · {region}\nLeft-click: disconnect\nRight-click: choose region")
    elif state in busy:
        emit("󰖂 …", "connecting", f"ExpressVPN: {state}\nLeft-click: disconnect")
    elif state == "Disconnected":
        emit("󰖂 Off", "disconnected", "VPN disconnected\nLeft-click: connect\nRight-click: choose region")
    else:
        emit("󰖂 !", "error", f"ExpressVPN: {state or 'unknown status'}\nOpen ExpressVPN to check the connection")


if __name__ == "__main__":
    action = sys.argv[1] if len(sys.argv) > 1 else "status"
    try:
        main(action)
    except (OSError, RuntimeError, ValueError, subprocess.TimeoutExpired):
        emit("󰖂 !", "error", "ExpressVPN unavailable or action failed. Open the client to check.")
        if action != "status" and shutil.which("notify-send"):
            subprocess.run(["notify-send", "ExpressVPN", "Action failed. Check the ExpressVPN client."], check=False)
