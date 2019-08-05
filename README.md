# BeeGl

Beehive monitoring system based on _ESP32 based boards_ and _Node Red_.  Main system application is beehive monitoring but can be used also as weather station.

# Main features

- Measures weight, temperature and humidity 
- Outbound communication through WiFi and GPRS (2G)
- Inbound communication using WiFi and Bluetooth (BLE)
- Web based management API - Web GUI and rest
- Pull/push OTA and setting updates
- Node operational time frame rules and power management
- Forms a distributed system where source and broker nodes can be linked together
- Basic server capable of displaying realtime data and statistics

<img src="https://thingiverse-production-new.s3.amazonaws.com/assets/0e/e0/21/0f/2a/BeeGl_Architecture.png" width="640"/>

If you like **BeeGl**, give it a star, or fork it and contribute! If you find this system useful please [![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=bostjan.sorgo%40gmail.com&currency_code=EUR&source=url)

## Quick Install

- Broker Device - [README.md](esp32/beegl_broker/README.md)
- Server [README.md](node-red/beegl_server/README.md)

## Libraries Used

### Broker component

- [ArduinoHTTPClient](https://github.com/arduino-libraries/ArduinoHttpClient) - forked (https://github.com/bsorgo/ArduinoHttpClient) - pending fixes for ESP32 environment
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [AsyncTcp](https://github.com/me-no-dev/AsyncTCP)
- [DHT sensor library for ESPx](https://github.com/beegee-tokyo/DHTesp)
- [ESP Async WebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [HX711](https://github.com/bogde/HX711)
- [PubSubClient](https://github.com/knolleary/pubsubclient)
- [Time](https://github.com/PaulStoffregen/Time) - forked (https://github.com/bsorgo/Time) 
- [Timezone](https://github.com/JChristensen/Timezone)
- [TinyGSM](https://github.com/vshymanskyy)

### Server components


- [Mosquitto](https://mosquitto.org/)
- [Node Red](https://nodered.org/)
- [Node Red Dashboard](https://flows.nodered.org/node/node-red-dashboard)

## License

This program is licensed under GPL-3.0
