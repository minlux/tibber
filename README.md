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


# Further Resources

- github.com/marq24/ha-tibber-pulse-local


