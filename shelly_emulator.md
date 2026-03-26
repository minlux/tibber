# Shelly Pro 3EM Emulator

`shelly_emulator.py` makes your Tibber Pulse appear as a **Shelly Pro 3EM** energy meter on the local network. Devices that poll Shelly meters over UDP (e.g. Marstek/DEYE B2500 home batteries) can receive live power data from your existing TibberBridge without any additional hardware.

## How It Works

```
[Energy Meter]
      | SML via IR
[Tibber Pulse]
      | HTTP REST (data.json)
[shelly_emulator.py]  <-- this script, running on any host
      | UDP port 1010 / 2220
[B2500 / other Shelly client]
```

On each incoming request the script:
1. Fetches the raw SML blob from TibberBridge via HTTP with basic auth
2. Pipes it through the local `sml/build/sml` binary to decode it into JSON
3. Formats the result as a Shelly response and sends it back over UDP

## Prerequisites

- The `sml` binary must be built first (see the main README)
- Python 3 — no third-party packages required (stdlib only)
- The host running the script must be reachable by the Shelly client on the network

## Usage

```bash
python3 shelly_emulator.py HOST [--user USER] [--password PASS] [--device-id ID] [--mode {legacy,rpc,both}]
```

`HOST` is mandatory — all other arguments are optional.

| Argument | Default | Description |
|---|---|---|
| `HOST` | _(required)_ | IP address or hostname of the TibberBridge |
| `--user` | `admin` | TibberBridge webserver username |
| `--password` | `C0P5-B8D2` | TibberBridge webserver password |
| `--device-id` | `tibber-pulse-shelly` | Device name echoed in RPC responses |
| `--mode` | `both` | `legacy` (port 1010), `rpc` (port 2220), or `both` |

```bash
python3 shelly_emulator.py 192.168.178.27
python3 shelly_emulator.py 192.168.178.27 --mode rpc
python3 shelly_emulator.py 192.168.178.27 --user admin --password secret --mode legacy
```

`ASSUMED_VOLTAGE` remains as a constant at the top of the script.

The `sml` binary is located automatically: the script first looks for `sml` next to itself, then falls back to `sml/build/sml`.

The script logs every handled request:

```
  UDP 1010 — legacy plain-string protocol  (firmware <= 224)
  UDP 2220 — JSON-RPC protocol             (firmware >= 226)
Shelly emulator started  (SML source: http://192.168.178.27/data.json?node_id=1)
[1010] [192.168.178.42]  power=342 W  energy=34201385 Wh
[2220] [192.168.178.55]  power=342 W  energy=34201385 Wh
```

## Protocol Reference

Two protocol variants are supported, targeting different firmware generations.

### Port 1010 — Legacy (firmware ≤ 224)

**Request:** any UDP payload containing the string `GetStatus`

```
GetStatus
```

**Response:** full Shelly Pro 3EM status JSON

```json
{
  "wifi_sta": {"connected": true, "ssid": "tibber-pulse", "rssi": -60},
  "cloud":    {"connected": false},
  "mqtt":     {"connected": false},
  "em:0": {
    "id": 0,
    "a_act_power":  342.0,
    "a_voltage":    230.0,
    "a_current":    1.487,
    "a_pf":         1.0,
    "a_aprt_power": 342.0,
    "b_act_power":  0.0,
    "b_voltage":    230.0,
    "b_current":    0.0,
    "b_pf":         1.0,
    "b_aprt_power": 0.0,
    "c_act_power":  0.0,
    "c_voltage":    230.0,
    "c_current":    0.0,
    "c_pf":         1.0,
    "c_aprt_power": 0.0,
    "n_current":    null,
    "total_act_power":  342.0,
    "total_aprt_power": 342.0,
    "total_current":    1.487,
    "errors": []
  }
}
```

### Port 2220 — JSON-RPC (firmware ≥ 226)

**Request:** JSON-RPC object

```json
{"id": 1, "src": "b2500", "method": "EM.GetStatus", "params": {"id": 0}}
```

**Response:** JSON-RPC envelope with minimal power fields only

```json
{"id":1,"src":"tibber-pulse-shelly","dst":"unknown","result":{"a_act_power":342.001,"b_act_power":0.001,"c_act_power":0.001,"total_act_power":342.001}}
```

> Note: Because B2500 firmware requires float values and rejects integers,
> whole-number watt values may get `+0.001` added so the JSON serializer always emits a decimal point.
> To enable this fix, comment in the "_as_float" lines.

## Data Mapping

The Tibber Pulse reports only **total active power** and a **total energy counter** — there is no per-phase breakdown. The mapping to Shelly's three-phase format is therefore:

| Shelly field | Source |
|---|---|
| `a_act_power` | `power` from SML (total, in W) |
| `b_act_power` | 0 (not available) |
| `c_act_power` | 0 (not available) |
| `total_act_power` | same as `a_act_power` |
| `a_voltage` | `ASSUMED_VOLTAGE` constant (default 230 V) |
| `a_current` | `power / ASSUMED_VOLTAGE` (estimated, pf = 1 assumed) |

## Manual Testing

### Port 1010 — Legacy

Start emulator:

```bash
sudo python3 shelly_emulator.py 192.168.178.27 --mode legacy
```

Send a `GetStatus` string and print the response:

```bash
echo -n "GetStatus" | nc -u -w1 127.0.0.1 1010 | jq .
```

`-u` selects UDP, `-w1` closes the connection after 1 second of inactivity (giving the response time to arrive).

### Port 2220 — JSON-RPC

Start emulator:

```bash
python3 shelly_emulator.py 192.168.178.27 --mode rpc
```

Send a JSON-RPC request and print the response:

```bash
echo -n '{"id":1,"src":"test","method":"EM.GetStatus","params":{"id":0}}' | nc -u -w1 127.0.0.1 2220 | jq .
```

## Running Without sudo

Port 1010 is below 1024 and therefore requires root privileges by default on Linux. Port 2220 is unaffected. Two ways to lift this restriction:

**Option 1 — Lower the unprivileged port start (system-wide):**

```bash
# Apply immediately
sudo sysctl -w net.ipv4.ip_unprivileged_port_start=1010

# Make permanent across reboots
echo 'net.ipv4.ip_unprivileged_port_start=1010' | sudo tee /etc/sysctl.d/50-unprivileged-ports.conf
```

Note: Assign `0` to completely remove the "privileged port restriction" — any user can bind any port!

**Option 2 — Grant the capability to the Python binary only:**

```bash
sudo setcap 'cap_net_bind_service=+ep' $(readlink -f $(which python3))
```

This allows any Python script to bind low ports without affecting anything else system-wide. Note: must be repeated after Python upgrades.

## References

- [jaapp/slimmelezer-shelly-emulator](https://github.com/jaapp/slimmelezer-shelly-emulator) — ESPHome-based emulator for SlimmeLezer+ (port 1010 protocol source)
- [tomquist/b2500-meter](https://github.com/tomquist/b2500-meter) — Python emulator for B2500 batteries (port 2220 protocol source)
