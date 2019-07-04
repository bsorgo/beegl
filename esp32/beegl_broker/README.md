# BeeGl ESP32 Source/Broker 

## Installing / Getting started

### Hardware
 1. Assemble 
   - build broker /source node device using instructions [BeeGl Beehive Monitoring](https://www.thingiverse.com/thing:3718552) or
   - assemble components on test board using ESP32 board of your choice following [BeeGl Broker/Source Node Schematic](https://thingiverse-production-new.s3.amazonaws.com/assets/a2/c6/a6/41/1a/BeeGl_Broker_rev_1.0.png) or
   - just use any ESP32 board with at least 4MB of flash without components just to play with app
 2. Connect device to serial port using appropriate wiring
### Software
#### From source
 1. Clone repository: git clone https://github.com/bsorgo/beegl.git
 2. Install Platform.IO IDE [PlatformIO IDE](https://platformio.org/platformio-ide?utm_source=github&utm_medium=arduino-esp32)
 3. Run Platform.IO and install Espressif 32 Platform
 4. Open Folder which represents local path of esp32/platformio/beegl_broker
 5. Modify platform.ini [configuration](https://docs.platformio.org/en/latest/platforms/espressif32.html#configuration) `[env:esp32dev]`, `[env:env:lolin_d32_pro_16mb]` 
    or create a new one. In particulal `monitor_port` and  `upload_port`should be changed to match assigned local COM port
```
[env:esp32dev]
platform = espressif32
lib_deps = ${common.lib_deps}
board = esp32dev
framework = arduino
board_build.partitions = customparts_tight.csv
upload_port = COM10
monitor_port = COM10
monitor_speed = 115200
build_flags = 
    -DCOMPONENT_EMBED_TXTFILES=src/index.html 
    -DCORE_DEBUG_LEVEL=4
    -DTINY_GSM_MODEM_SIM800
    -DVER=\"1.0.1\"
```
  6. Upload 
  7. Examine serial output using Monitor 

#### Development/Stable releases

TBD

## Runtime

### First run/safe/maintenance mode

At first run or when startup sequence is interrupted by hardware reset/error, device will terminate in safe/maintenance mode. Also indicated by continious red LED. In this case all outbound connections are disabled (WiFi, GPRS) and WiFi access point with SSID: BEE000 and password: 123456789 is brought up. 

Using WiFi capable mobile device establish a connection with SSID named BEE000. 

With web browser on device navigate to http://192.168.4.1 - BeeGl Broker settings page.

The serial output should look something like this:

```
[I][Connection.cpp:133] wifiSetup(): [WIFI] AP started
[I][Connection.cpp:134] wifiSetup(): [WIFI] AP IP: 192.168.4.1
[D][WiFiGeneric.cpp:336] _eventCallback(): Event: 13 - AP_START
[I][Connection.cpp:135] wifiSetup(): [WIFI] AP password: 123456789
[I][Connection.cpp:44] modemOff(): [GSM] OFF
[I][Service.cpp:27] webServerSetup(): [WEB Server] Starting web server
[I][Service.cpp:33] webServerSetup(): [WEB Server] Web server started
[D][Runtime.cpp:223] setSafeModeOnRestart(): [ESP32] Open done
[D][Runtime.cpp:233] setSafeModeOnRestart(): [ESP32] Write fdone.
[D][Runtime.cpp:243] setSafeModeOnRestart(): [ESP32] Commit done.
[I][main.cpp:128] setup(): ***************************************
[I][main.cpp:129] setup(): ******** SAFE/MAINTENANCE MODE ********
[I][main.cpp:130] setup(): ***************************************
[I][main.cpp:131] setup(): Use Wifi device, connect to SSID:BEE000. By using web browser navigate to http://192.168.4.1
```

Note:

On some Android phones mobile data should be disabled when connected to non-internet AP.

### Operational mode

It's normal broker/source node mode. At startup setup sequence is executed:

1. Mount FS filesystem - 1 green LED blink
2. Read config file - 2 green LED blinks
3. Establish inbound/outbound connection and services (web server) - 3 blinks
4. Initiate publisher, sensors and broker - 4 blinks

After setup sequence green LED lights continiously. If any of above steps fails the device goes to deep sleep or safe/maintanance mode.
In addition updates are checked between step 3 and 4 if set accordingly in time interval/frame definition (`"upd" : "true"`)

During main program loop:

- Measurements are performed (weight & temp/humidity)
- Publish measurement to central server
- Check operational time frame and put device in deep sleep

```
[I][Runtime.cpp:41] initialize(): [ESP] Firmware version 1.4
[D][Runtime.cpp:181] getSafeModeOnRestart(): [ESP32] Open done
[D][Runtime.cpp:188] getSafeModeOnRestart(): [ESP32] Read OK, value: 0
[SPIFFS] Begin FS
[D][Runtime.cpp:223] setSafeModeOnRestart(): [ESP32] Open done
[D][Runtime.cpp:233] setSafeModeOnRestart(): [ESP32] Write fdone.
[D][Runtime.cpp:243] setSafeModeOnRestart(): [ESP32] Commit done.
[I][Runtime.cpp:155] printWakeupReason(): [ESP32] Wakeup was not caused by deep sleep. Reason: 0
[I][main.cpp:136] setup(): [ESP] Device name: BEE007
[I][main.cpp:137] setup(): [ESP] Inbound mode: 0
[I][main.cpp:138] setup(): [ESP] Outbound mode: 1
[I][Connection.cpp:90] wifiSetup(): [WIFI] Settings: Use custom ip: 0, IP: 192.168.4.1 GW: 192.168.4.1 NETMASK: 255.255.255.0
[I][Connection.cpp:91] wifiSetup(): [WIFI] APSettings: IP: 192.168.4.1 GW: 0.0.0.0 NETMASK: 255.255.255.0
[I][Connection.cpp:139] wifiSetup(): [WIFI] Starting STA
[I][Connection.cpp:140] wifiSetup(): [WIFI] Outbound connecting to MyWifi
[D][WiFiGeneric.cpp:336] _eventCallback(): Event: 0 - WIFI_READY
[D][WiFiGeneric.cpp:336] _eventCallback(): Event: 2 - STA_START
[D][WiFiGeneric.cpp:336] _eventCallback(): Event: 4 - STA_CONNECTED
[D][WiFiGeneric.cpp:336] _eventCallback(): Event: 7 - STA_GOT_IP
[D][WiFiGeneric.cpp:379] _eventCallback(): STA IP: 192.168.0.163, MASK: 255.255.255.0, GW: 192.168.0.1
[I][Connection.cpp:160] wifiSetup(): [WIFI] connected
[I][Connection.cpp:161] wifiSetup(): [WIFI] IP: 192.168.0.163
[I][Connection.cpp:44] modemOff(): [GSM] OFF
[I][Service.cpp:27] webServerSetup(): [WEB Server] Starting web server
[I][Service.cpp:33] webServerSetup(): [WEB Server] Web server started
[D][Settings.cpp:429] syncTimeAndSettings(): [SETTINGS] Time and setting prefix: www.allium4.com/beegl/
[D][Settings.cpp:434] syncTimeAndSettings(): [SETTINGS] Hostname: www.allium4.com
[D][Settings.cpp:435] syncTimeAndSettings(): [SETTINGS] Path: /beegl/v1/devices/BEE007
[D][Settings.cpp:436] syncTimeAndSettings(): [SETTINGS] Username: bee
[I][Settings.cpp:454] readTimeAndSettings(): [SETTINGS] Response code from server: 200
[I][Settings.cpp:464] readTimeAndSettings(): [SETTINGS] Header date string: Mon, 01 Jul 2019 18:05:21 GMT
[D][Settings.cpp:498] readTimeAndSettings(): [SETTINGS] Time fractions: 2019-07-01T18:05:21.000Z
[D][Settings.cpp:751] getCurrentSchedulerEntry(): [SCHEDULER] Current entry: 7;20:0,20:10;0
[D][Settings.cpp:751] getCurrentSchedulerEntry(): [SCHEDULER] Current entry: 7;20:0,20:10;0
[I][MqttPublisher.cpp:25] setup(): [MQTT] Setup
[I][MqttPublisher.cpp:36] setup(): [MQTT] Server: internalMqttServer
[I][MqttPublisher.cpp:37] setup(): [MQTT] Server: mqtt.allium4.com, port 1883
[I][Measurer.cpp:64] scaleSetup(): [HX711] Setup
[I][Measurer.cpp:65] scaleSetup(): [HX711] Scale factor: -23.209999
[I][Measurer.cpp:66] scaleSetup(): [HX711] Scale offset: 1828191
[I][Measurer.cpp:67] scaleSetup(): [HX711] Scale unit: g
[I][Measurer.cpp:79] dhtSetup(): [DHT] Setup
[D][Runtime.cpp:223] setSafeModeOnRestart(): [ESP32] Open done
[D][Runtime.cpp:233] setSafeModeOnRestart(): [ESP32] Write fdone.
[D][Runtime.cpp:243] setSafeModeOnRestart(): [ESP32] Commit done.
[D][Measurer.cpp:116] measure(): [HX711] Powerup
[D][Measurer.cpp:122] measure(): [MEASURER] Read weight 23451.12
[D][Measurer.cpp:123] measure(): [HX711] Shutdown
[D][Measurer.cpp:131] measure(): [MEASURER] Read temperature and humidity: 23.5 C 65 %
[D][Publisher.cpp:60] storeMessage(): [STORE] Message
[D][MqttPublisher.cpp:74] reconnect(): [MQTTPUBLISHER] Client: BEE007-20, username: bee
[D][MqttPublisher.cpp:77] reconnect(): [MQTTPUBLISHER] Connected.
{"id":"BEE007","time":"2019-07-01T20:05:21+02:00","ver":"1.4","whtS":{"w":23451.12,"u":"g"},"tmpS":{"t":23.5,"u":"C"},"humS":{"h":65,"u":"pct"}}
[D][MqttPublisher.cpp:50] publishMessage(): [MQTTPUBLISHER] scale_sensors
[D][MqttPublisher.cpp:51] publishMessage():
[E][MqttPublisher.cpp:59] publishMessage(): [MQTTPUBLISHER] Publish OK
```

### Settings

Settings can be managed:

- Using web interface on http: `'http://<device ip>`
- Using rest api GET/POST `'http://<device ip>/rest/settings'`

<img src="https://thingiverse-production-new.s3.amazonaws.com/assets/12/31/a1/04/04/Screenshot_20190626-173703.jpg" width="100" />

#### Basic settings

1. Device name - Unique device name 
2. Outbound mode. Defines upstream communication preference. For broker node this sets communication with internet/intranet towards central server. For source node this defines communication channel with broker node.
  - Select WiFi if outbound communication is WiFi. Specify Outbound WiFi Settings (SSID, Password and IP related settings)
  - Select GPRS for GPRS communication. Specify Outbound GSM/GPRS Settings (APN, Username and Password). Needles to say SIM card should be inserted in SIM module. These settings are GSM G2 network operator specific. 
3. Setting URL prefix defines central server url in format `<hostname or ip>/<path>/`. Example `192.168.0.250/beegl/`. Currently only http on port 80 is supported. Provide also username and password for basic authentication if required
4. Publisher settings
  - Select MQTT if publisher uses MQTT protocol. Measurements are published on `measurements` topic. 
  - Select HTTP to publish over HTTP. Path is concatenated using Setting URL Prefix and `v1/measurements` path fraction. For example if setting URL prefix is `192.168.0.250/beegl/` measurements resource measurement url is will be published on `192.168.0.250/beegl/v1/measurements`.
 5. MQTT Settings set upstream MQTT server connection parameters
   - Internal server address - MQTT server IP or hostname when outbound connection is WiFi. 
   - External server address - MQTT server IP or hostname when outbound connection is GPRS.
   - Port (TCP Port Number) - MQTT server port. Default port is 1883.
   - Username and password - MQTT server username an password
   Please note MQTT SSL/TLS connections are not supported.
6. Operational Time Intervals are used to define time frames when broker/source node will wake up from deep sleep to operational mode. Specify hour interval From and To in format `hh:mm`. Up to 10 intervals can be specified. Recurring intervals (e.g. wake up every 10 minutes) are not yet supported.

#### Measurement settings
 
 1. Measurer settings:
   - Measure weight (Yes/No)
   - Measure temperature and humidity (Yes/No)
 2. Scale settings
   - Factor (Decimal  number) - a calibration factor
   - Offset (Whole number) - calibration offset. By pressing button __Tare__ this value is determined based on current load. 
   __Measure weight__ button does weight measurement with current calibration factor and offset
     
   
 #### Advanced settings
 
1. Inbound mode. Provides inbound channels for other source nodes - when device act as broker:

- None - No inbound connection available
- WiFi - AP Wifi is initiated. Source nodes can connect on broker AP and publish settings based on source node Setting URL prefix to path  `<hostname or ip>/<path>/v1/measurements`
- Bluetooth(BLE) - BT radio and BLE service is established. Used for BLE source nodes. This funcitonality is still under heavy development
- WiFi+Bluetooth(BLE) - both AP WiFi and BLE service are established

2. Refresh (Milliseconds) - refresh interval for measures and publishes. Default 60000 (1 minute).
3. Time settings (Timezone selector). Sets standard and summer timezones. 


