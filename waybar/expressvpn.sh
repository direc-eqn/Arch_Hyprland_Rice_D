#!/bin/sh
# Keep the original entry point; Python safely encodes JSON and CLI arguments.
exec python3 "$(dirname -- "$0")/scripts/expressvpn.py" "$@"
