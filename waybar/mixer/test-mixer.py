#!/usr/bin/env python3
"""Run in the desktop session. Only changes a disposable silent stream."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import time
import wave

root = Path(__file__).resolve().parent

def command(*args):
    return subprocess.check_output(args, text=True, timeout=20).strip()

with tempfile.TemporaryDirectory(prefix="waybar-mixer-test-") as folder:
    folder = Path(folder)
    binary = folder / "test-mixer"
    flags = shlex.split(command("pkg-config", "--cflags", "--libs", "gtk+-3.0", "gtk-layer-shell-0", "libpulse-mainloop-glib", "playerctl"))
    subprocess.run(["cc", "-g", "-O1", str(root / "test-mixer.c"), "-o", str(binary), *flags], check=True)
    silence = folder / "silence.wav"
    with wave.open(str(silence), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(8000)
        wav.writeframes(b"\0\0" * 8000 * 30)
    sink = f"waybar_mixer_test_{os.getpid()}"
    module = None
    stream = None
    try:
        module = command("pactl", "load-module", "module-null-sink", f"sink_name={sink}",
                         "sink_properties=device.description=WaybarMixerTest")
        stream = subprocess.Popen(["paplay", f"--device={sink}", "--client-name=Disposable mixer test",
                                   "--stream-name=Disposable mixer test", str(silence)])
        time.sleep(0.5)
        subprocess.run([str(binary)], cwd=root, timeout=20, check=True)
    finally:
        if stream and stream.poll() is None:
            stream.terminate()
            stream.wait(timeout=5)
        if module:
            command("pactl", "unload-module", module)
