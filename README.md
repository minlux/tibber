# TibberBridge SML Reader

A set of tools to read live energy data from your electricity meter via TibberBridge.

Modern electricity meters expose their data through an infrared interface using the SML protocol.
If you are a [Tibber](https://tibber.com/de) customer, you may already have a *Tibber Pulse* clipped
to that IR port — it reads the meter data and forwards it to *TibberBridge*, which sends it to Tibber
over the internet.

Fortunately, *TibberBridge* has an embedded webserver that can serve the raw SML data locally.
How to enable it is described [here](https://the78mole.de/doing-the-undone-decoding-sml-or-hacking-the-tibber-raw-data/).
You will need:
- the IP address of your *TibberBridge*
- the webserver username and password


## Build the SML Tool

```bash
cd sml
mkdir build
cd build
cmake ..
make
```


## Configure the Read Script

Edit `read.sh` and set the correct IP address, username, and password for your TibberBridge.


## Retrieve Data

Fetch the current SML data from *TibberBridge* and decode it to JSON:

```bash
./read.sh
```


## Output Format

The tool outputs a JSON object with the following fields:

```json
{
  "energy": 34201385,
  "meter":  "ESY",
  "power":  102,
  "serial": "113440394"
}
```

| Field | Description |
|-------|-------------|
| `energy` | Total energy consumption in Wh |
| `meter` | Meter manufacturer identifier |
| `power` | Current power consumption in W |
| `serial` | Meter serial number |


## Shelly Pro 3EM Emulator

`shelly_emulator.py` makes your TibberBridge appear as a Shelly Pro 3EM energy meter on the local
network. Devices that poll Shelly meters over UDP (e.g. Marstek/DEYE B2500 home batteries) can
receive live power data without any additional hardware.

See [shelly_emulator.md](shelly_emulator.md) for full documentation.


# Further Resources

- github.com/marq24/ha-tibber-pulse-local
