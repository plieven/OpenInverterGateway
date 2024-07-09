# OpenInverterGateway

Firmware replacement for Growatt ShineWiFi-S (serial), ShineWiFi-X (USB) or custom build sticks (ESP8266/ESP32).

# How to install

* Download a precompiled release from [here](https://github.com/OpenInverterGateway/OpenInverterGateway/releases) (hardcoded to default [settings](https://github.com/OpenInverterGateway/OpenInverterGateway/blob/master/SRC/ShineWiFi-ModBus/Config.h.example)/growatt protocol 1.24)

Or

* Checkout this repo
* Setup platform.io
* Open the project folder and choose the correct env for your hardware
* Rename and adapt [Config.h.example](https://github.com/OpenInverterGateway/OpenInverterGateway/blob/master/SRC/ShineWiFi-ModBus/Config.h.example) to Config.h with your compile time settings

After you obtained an image you want to flash:

* Flash to an esp32/esp8266 or to a shineX/S-stick ([details](https://github.com/OpenInverterGateway/OpenInverterGateway/blob/master/Doc/)).
* Connect to the setup wifi called GrowattConfig (PW: growsolar) and configure the firmware via the webinterface at http://192.168.4.1
* If you need to reconfigure the stick later on you have to either press the ap button (configured in Config.h) or reset the stick twice within 2-10sec

## Features
Implemented Features:
* Built-in simple Webserver
* The inverter is queried using Modbus Protocol
* The data received will be transmitted by MQTT to a server of your choice.
* The data received is also provided as JSON and Prometheus format
* Show a simple live graph visualization  (`http://<ip>`) with help from highcharts.com
* Firmware update via wifiManager
* It supports basic access to arbitrary modbus data
* It tries to autodetect which stick type to use
* Wifi manager with own access point for initial configuration of Wifi and MQTT server (IP: 192.168.4.1, SSID: GrowattConfig, Pass: growsolar)
* Currently Growatt v1.20, v1.24 and v3.05 protocols are implemented and can be easily extended/changed to fit anyone's needs
* TLS support for esp32
* Debugging via Web and Telnet
* Power limiting (allows zero export)

Not supported:
* It does not make use the RTC or SPI Flash of these boards.
* It does not communicate to Growatt Cloud at all.
* The ShineWifi-LAN stick is not supported. But there is another project for [this](https://github.com/mwalle/shinelanx-modbus)

## Supported sticks/microcontrollers
* ShineWifi-S with a Growatt Inverter connected via serial (Modbus over RS232 with level shifter)
* ShineWifi-X with a Growatt Inverter connected via USB (USB-Serial Chip from Exar)
* Wemos-D1 with a Growatt Inverter connected via USB (USB-Serial Chip: CH340)
* NODEMCU V1 (ESP8266) with a Growatt Inverter connected via USB (USB-Serial Chip: CH340)
* ShineWifi-T (untested, please give feedback)
* Lolin32 (ESP32) with a Growatt Inverter connected via USB

I tested several ESP8266-boards with builtin USB-Serial converters so far only boards with CH340 do work (CP21XX and FTDI chips do not work). Almost all ESP8266 modules with added 9-Pin Serial port and level shifter should work with little soldering via Serial.

See the short descriptions to the devices (including some pictures) in the "Doc" directory.

## Supported Inverters
* Growatt 1000-3000S 
* Growatt MIC 600-3300TL-X (Protocol 124 via USB/Protocol 120 via Serial)
* Growatt MID 3-25KTL3-X (Protocol 124 via USB)
* Growatt MOD 3-15KTL3-X-H (Protocol 120 via USB)
* Growatt MOD 12KTL3-X (Protocol 124 via USB)
* Growatt MID 25-40KTL3-X (Protocol 120 via USB)
* Growatt SPH 4000-10000STL3-X BH (Protocol 124 via Serial)
* Growatt MID 15KTL3-XH (Protocol 3000 via USB)
* And others ....

## Modbus Protocol Versions
The documentation from Growatt on the Modbus interface is available, search for "Growatt PV Inverter Modbus RS485 RTU Protocol" on Google.

The older inverters apparently use Protocol v3.05 from year 2013.
The newer inverters apparently use protocol v1.05 from year 2018.
There is also a new protocol version v1.24 from 2020. (used with SPH4-10KTL3 BH-UP inverter)
TL-XH hybrid inverters use version v1.24 with a different set of input and holding registers.

## JSON Format Data
For IoT applications the raw data can now read in JSON format (application/json) by calling `http://<ip>/status`

## Prometheus Format Metrics
If you want to scrape the metrics with a Prometheus server, you can use the endpoint `http://<ip>/metrics`. A possible configuration is described [here](Doc/Prometheus.md).

## Homeassistant configuration

Homeassistant config is described [here](Doc/MQTT.md)

## Debugging

If you turned on `ENABLE_WEB_DEBUG` in the Config.h (see Config.h.example) there is a debug site under `http://<ip>/debug`. You can turn on `ENABLE_TELNET_DEBUG` to get the debug messages via a telnet client. `telnet <ip>`

To enable even more messages, take a look to `DEBUG_MODBUS_OUTPUT`.

## Change log

See [here](CHANGELOG.md)

## Acknowledgements

This arduino sketch will replace the original firmware of the Growatt ShineWiFi stick.

This project is based on Jethro Kairys work on the Modbus interface
https://github.com/jkairys/growatt-esp8266

Some keywords:

ESP8266, ESP-07S, Growatt 1000S, Growatt 600TL, ShineWifi, Arduino, MQTT, JSON, Modbus, Rest
