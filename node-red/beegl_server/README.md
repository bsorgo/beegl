# BeeGl server

Provides basic - demonstrative functionality for BeeGl monitiring: 

- store & process measurements from for BeeGl devices
- basic BeeGl device management 
- basic analytics for past n days

## Disclaimer

__This is open source software, use at your own risk__

BeegGl server application is meant only to present basic data provided by BeeGl broker and sub nodes. If deployed on public (cloud) server please ensure proper installation and security as consulted
by creators of server platforms (MQTT and Node Red in our case). Unfortunatelly BeeGl broker node supports only unsecure communication on HTTP and/or MQTT. 

## Getting started 

1. MQTT
  - Install MQTT server [Eclipse Mosquitto](https://mosquitto.org/). There're many instructions on internet. [This one](http://www.steves-internet-guide.com/install-mosquitto-linux/) is quite good for linux. Or 
  - use/install any other MQTT 3.1.1 compatible MQTT server
  
  Don't forget to secure access with username and password. Unfortunatelly current version of BeeGl Broker supports only usecure MQTT communication.
  
2. Install node red - [Getting Started](https://nodered.org/docs/getting-started/). To highlight few of them:
  - [Local installation](https://nodered.org/docs/getting-started/local)
  - [Raspberry PI installation](https://nodered.org/docs/getting-started/raspberrypi)
  
  Don't forget to secure node server with setting username and password. Unfortunatelly current version of BeeGl Broker supports only http communication. 

3. Install [node-red-dashboard] (https://flows.nodered.org/node/node-red-dashboard)
4. Run node red serivice and navigate to node red admin 

<img src="https://thingiverse-production-new.s3.amazonaws.com/assets/e1/e1/19/be/e3/2019-07-01_12_05_49-Greenshot.png" width="800"/>

5. Import beegl_server_flow.json
6. Configure beegl-mqtt-broker configuration node accordingly (host, port, username, password,..)
7. Deploy
8. Navigate to `http://<node red hostname | ip>:1880/nodered/ui`

<img src="https://thingiverse-production-new.s3.amazonaws.com/assets/7f/aa/e4/93/b1/Node-RED_Dashboard.png" width="800"/>




