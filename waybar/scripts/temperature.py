#!/usr/bin/env python3
"""Discover sensors by driver / label instead of unstable hwmon numbers."""
import json
from pathlib import Path
import sys


def temperature(kind, root=Path("/sys/class/hwmon")):
    candidates = []
    for device in root.glob("hwmon*"):
        try:
            driver = (device / "name").read_text().strip()
            for sensor in device.glob("temp*_input"):
                label_file = sensor.with_name(sensor.name.replace("_input", "_label"))
                label = label_file.read_text().strip() if label_file.exists() else ""
                rank = None
                if kind == "cpu":
                    if driver == "coretemp" and label.startswith("Package id"):
                        rank = 0
                    elif driver in ("k10temp", "zenpower") and label in ("Tctl", "Tdie"):
                        rank = 0
                    elif label == "CPU":
                        rank = 1
                else:
                    if driver in ("amdgpu", "nouveau"):
                        rank = 0
                    elif label == "GPU":
                        rank = 1
                if rank is not None:
                    value = int(sensor.read_text()) / 1000
                    if 0 < value < 150:
                        candidates.append((rank, value))
        except (OSError, ValueError):
            continue
    if not candidates:
        return {"text": f"{kind.upper()} —", "class": "unavailable",
                "tooltip": "No supported temperature sensor found"}
    best = min(rank for rank, _ in candidates)
    value = max(value for rank, value in candidates if rank == best)
    return {"text": f"{kind.upper()} {value:.0f}°", "class": "critical" if value >= 80 else "normal",
            "tooltip": f"{kind.upper()} temperature: {value:.1f} °C"}


if __name__ == "__main__":
    kind = sys.argv[1] if len(sys.argv) > 1 else "cpu"
    if kind not in ("cpu", "gpu"):
        raise SystemExit("Usage: temperature.py cpu|gpu")
    print(json.dumps(temperature(kind)))
