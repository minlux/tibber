# TibberBridge SML Reader

This tools are used to read out data of your enery meter via TibberBridge.

Todays energy meter output their data via an IR interface, using SML protocol.
To read that data you can use an IR read and use that data for your smart home etc.

However, if you are using [tibber](https://tibber.com/de) the iterface may be occupied
by *Tibber Pulse* (that reads the data and forwards it to *TibberBridge*, to be sent to *tibber* through the internet).

Luckily, you can enable an embedded webserver in the *TibberBridge* to get the SML data.
How to do that, is described [here](https://the78mole.de/doing-the-undone-decoding-sml-or-hacking-the-tibber-raw-data/).
While doing so, note:
- the IP address of *TibberBridge*
- the webserver username and password

No you are ready to use my tool :-)


## Build SML Tool

```
cd sml
mkdir build
cd build
cmake ..
make
```

## Configure Read Script

In `read.sh`, adapt the command with the correct IP address, username and password.


## Retrieve Data

To retriev the current SML data of your energy meter from *TibberBridge* and pipe it into the *sml* tool.
The tool will decode the SML data and output it as JSON (which can be further processed according your demand).

```
./read.sh
```

## Output Format

The tool outputs a JSON object with the following fields:

```json
{"energy":34201385,"meter":"ESY","power":102,"serial":"113440394"}
```

| Field | Description |
|-------|-------------|
| `energy` | Total energy consumption in Wh |
| `meter` | Meter manufacturer identifier |
| `power` | Current power consumption in W |
| `serial` | Meter serial number |


## Shelly Pro 3EM Emulator

`shelly_emulator.py` emulates a Shelly Pro 3EM energy meter over UDP, using live data from the TibberBridge as source. This allows devices that poll Shelly meters (e.g. home batteries like the Marstek/DEYE B2500) to receive real power data without additional hardware.

See [shelly_emulator.md](shelly_emulator.md) for full documentation.


# Further Resources

- github.com/marq24/ha-tibber-pulse-local


