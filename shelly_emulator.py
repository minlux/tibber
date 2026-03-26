#!/usr/bin/env python3
"""
Tibber Pulse -> Shelly Pro 3EM UDP emulator

Listens simultaneously on two UDP ports and responds to GetStatus requests
with live energy data fetched from TibberBridge:

  Port 1010 — legacy protocol (firmware <= 224)
               Request: any UDP payload containing the plain string "GetStatus"
               Response: full Shelly Pro 3EM JSON (voltage, current, pf, …)

  Port 2220 — new RPC protocol (firmware >= 226)
               Request: JSON-RPC  {"id":…, "method":"EM.GetStatus", "params":{"id":0}}
               Response: JSON-RPC {"id":…, "src":…, "dst":…, "result":{a/b/c_act_power, total_act_power}}

Usage:
    python3 shelly_emulator.py HOST [--user USER] [--password PASS]
                                    [--device-id ID] [--mode {legacy,rpc,both}]
"""

import argparse
import json
import os
import select
import socket
import subprocess
import urllib.request


# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

# Path to the compiled sml binary — try next to this script first, then the build directory
_here = os.path.dirname(os.path.abspath(__file__))
_candidates = [os.path.join(_here, "sml"), os.path.join(_here, "sml/build/sml")]
SML_BINARY = next((p for p in _candidates if os.path.isfile(p)), _candidates[-1])

# Assumed line voltage (V) — used to estimate current for the legacy response,
# since the Tibber Pulse does not report per-phase voltage or current via SML.
ASSUMED_VOLTAGE = 230.0
# ---------------------------------------------------------------------------


def fetch_energy_data(url: str, user: str, password: str) -> dict:
    """Fetch the SML blob from TibberBridge and decode it with the sml binary.

    Returns a dict with at least:
        power  (int, W)   — current active power (positive = consumption)
        energy (int, Wh)  — total energy counter
        meter  (str)      — manufacturer identifier
        serial (str)      — meter serial number
    """
    password_mgr = urllib.request.HTTPPasswordMgrWithDefaultRealm()
    password_mgr.add_password(None, url, user, password)
    opener = urllib.request.build_opener(urllib.request.HTTPBasicAuthHandler(password_mgr))

    with opener.open(url, timeout=5) as resp:
        sml_blob = resp.read()

    proc = subprocess.run(
        [SML_BINARY],
        input=sml_blob,
        capture_output=True,
        timeout=5,
    )
    proc.check_returncode()
    return json.loads(proc.stdout)


def _as_float(value: float) -> float:
    """Ensure the value serializes with a decimal point.

    The B2500 firmware (port 2220) requires float values. Python's json.dumps
    serializes whole numbers without a decimal point (e.g. 100 instead of 100.0),
    which confuses the firmware. Adding a tiny offset forces the decimal point
    while being negligible for energy accounting.
    """
    if value == int(value):
        return value + 0.001
    return value


def build_legacy_response(data: dict) -> bytes:
    """Port 1010 — full Shelly Pro 3EM status response (plain JSON).

    The Tibber Pulse reports total active power only (no per-phase breakdown).
    All power is mapped to phase A; phases B and C are reported as zero.
    Current is estimated from power / ASSUMED_VOLTAGE (power factor assumed = 1).
    """
    power_w  = float(data.get("power", 0))
    current_a = round(power_w / ASSUMED_VOLTAGE, 3) if ASSUMED_VOLTAGE else 0.0

    response = {
        "wifi_sta": {"connected": True, "ssid": "tibber-pulse", "rssi": -60},
        "cloud":    {"connected": False},
        "mqtt":     {"connected": False},
        "em:0": {
            "id": 0,
            # Phase A — carries the full reported power
            "a_act_power":  power_w,
            "a_voltage":    ASSUMED_VOLTAGE,
            "a_current":    current_a,
            "a_pf":         1.0,
            "a_aprt_power": power_w,
            # Phase B — not available
            "b_act_power":  0.0,
            "b_voltage":    ASSUMED_VOLTAGE,
            "b_current":    0.0,
            "b_pf":         1.0,
            "b_aprt_power": 0.0,
            # Phase C — not available
            "c_act_power":  0.0,
            "c_voltage":    ASSUMED_VOLTAGE,
            "c_current":    0.0,
            "c_pf":         1.0,
            "c_aprt_power": 0.0,
            # Neutral current — not available
            "n_current": None,
            # Totals
            "total_act_power":  power_w,
            "total_aprt_power": power_w,
            "total_current":    current_a,
            "errors": [],
        },
    }
    return json.dumps(response).encode("utf-8")


def build_rpc_response(request_id: int, data: dict, device_id: str) -> bytes:
    """Port 2220 — minimal Shelly RPC GetStatus response.

    Only act_power fields are returned (no voltage, current, pf, etc.).
    The request id is echoed back so the caller can match request to response.
    """
    power_w = float(data.get("power", 0))

    response = {
        "id":  request_id,
        "src": device_id,
        "dst": "unknown",
        "result": {
            "a_act_power":     power_w,      # _as_float(power_w),
            "b_act_power":     0.0,          # _as_float(0.0),
            "c_act_power":     0.0,          # _as_float(0.0),
            "total_act_power": power_w,      # _as_float(power_w),
        },
    }
    return json.dumps(response, separators=(",", ":")).encode("utf-8")


def handle_legacy(sock: socket.socket, url: str, user: str, password: str) -> None:
    """Handle one incoming packet on the legacy port 1010."""
    raw, addr = sock.recvfrom(4096)
    if b"GetStatus" not in raw:
        return
    data = fetch_energy_data(url, user, password)
    sock.sendto(build_legacy_response(data), addr)
    print(f"[1010] [{addr[0]}]  power={data.get('power')} W  energy={data.get('energy')} Wh")


def handle_rpc(sock: socket.socket, url: str, user: str, password: str, device_id: str) -> None:
    """Handle one incoming packet on the RPC port 2220."""
    raw, addr = sock.recvfrom(4096)
    try:
        request = json.loads(raw)
    except json.JSONDecodeError:
        return
    method = request.get("method", "")
    if method != "EM.GetStatus":
        return
    params = request.get("params", {})
    if not isinstance(params.get("id"), int):
        return
    data = fetch_energy_data(url, user, password)
    sock.sendto(build_rpc_response(request.get("id", 0), data, device_id), addr)
    print(f"[2220] [{addr[0]}]  power={data.get('power')} W  energy={data.get('energy')} Wh")


def serve(mode: str, url: str, user: str, password: str, device_id: str) -> None:
    sockets = {}

    if mode in ("legacy", "both"):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind(("", 1010))
        sockets[s] = lambda sock: handle_legacy(sock, url, user, password)
        print("  UDP 1010 — legacy plain-string protocol  (firmware <= 224)")

    if mode in ("rpc", "both"):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind(("", 2220))
        sockets[s] = lambda sock: handle_rpc(sock, url, user, password, device_id)
        print("  UDP 2220 — JSON-RPC protocol             (firmware >= 226)")

    print(f"Shelly emulator started  (SML source: {url})")

    while True:
        readable, _, _ = select.select(list(sockets.keys()), [], [])
        for sock in readable:
            try:
                sockets[sock](sock)
            except Exception as exc:
                print(f"error: {exc}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Tibber Pulse -> Shelly Pro 3EM UDP emulator")
    parser.add_argument(
        "host",
        metavar="HOST",
        help="IP address or hostname of the TibberBridge",
    )
    parser.add_argument(
        "--user",
        default="admin",
        help="TibberBridge webserver username (default: admin)",
    )
    parser.add_argument(
        "--password",
        default="C0P5-B8D2",
        help="TibberBridge webserver password (default: C0P5-B8D2)",
    )
    parser.add_argument(
        "--device-id",
        default="tibber-pulse-shelly",
        help="Device identifier echoed in RPC responses (default: tibber-pulse-shelly)",
    )
    parser.add_argument(
        "--mode",
        choices=["legacy", "rpc", "both"],
        default="both",
        help="Protocol to serve: 'legacy' (port 1010, firmware <= 224), "
             "'rpc' (port 2220, firmware >= 226), or 'both' (default)",
    )
    args = parser.parse_args()
    tibber_url = f"http://{args.host}/data.json?node_id=1"
    serve(args.mode, tibber_url, args.user, args.password, args.device_id)
