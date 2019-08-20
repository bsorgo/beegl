# BeeGl server

Provides basic - demonstrative functionality for BeeGl monitoring: 

- store & process measurements from for BeeGl devices
- basic BeeGl device management 
- basic analytics for past n days

## Disclaimer

__This is open source software, use at your own risk__

BeegGl server application is meant only to present basic data provided by BeeGl broker and sub nodes. If deployed on public (cloud) server please ensure proper installation and security as consulted
by creators of server platforms (MQTT, Node Red and MongoDB in our case). Unfortunately BeeGl broker node supports only insecure communication on HTTP and/or MQTT. 

## Getting started 

### New installation

From version 1.2.0 server requires document database - MongoDB. To migrate check 

1. MQTT
  - Install MQTT server [Eclipse Mosquitto](https://mosquitto.org/). There are many instructions on internet. [This one](http://www.steves-internet-guide.com/install-mosquitto-linux/) is quite good for linux. Or 
  - use/install any other MQTT 3.1.1 compatible MQTT server
  
  Don't forget to secure access with username and password. Unfortunately current version of BeeGl Broker supports only usecure MQTT communication.
2. MongoDB 
  - Install MongoDB - [Install MongoDB](https://docs.mongodb.com/manual/installation/). Or
  - use existing MongoDB DB instance(s)
  - using CLI mongodb create MongoDB user and database. In our case user and database are named beegl.
	```
	use beegl
	db.createUser(
	  {
		user: "beegl",
		pwd: "abc123",
		roles: [ { role: "readWrite", db: "beegl" } ]
	  }
	)
	```
3. In beegl_server execute npm install --production. This will install necessary libraries. 
4. Execute database schema script create_mongodb_schema.js . This will create collection schema and required indexes. Change connection parameters, credentials (-dburl) and database name (-dbname) accordingly
    ``` 
    node ./create_mongodb_schema.js --dburl mongodb://beegl:abc123@localhost:27017/beegl --dbname beegl 
    ```	
5. Install node red - [Getting Started](https://nodered.org/docs/getting-started/). To highlight few of them:
  - [Local installation](https://nodered.org/docs/getting-started/local)
  - [Raspberry PI installation](https://nodered.org/docs/getting-started/raspberrypi)
  
  Don't forget to secure node server with setting username and password. Unfortunately current version of BeeGl Broker supports only http communication. 
6. In .node-red home directory install required components:
  - [node-red-dashboard](https://flows.nodered.org/node/node-red-dashboard)
  - [node-red-contrib-mongodb3](https://www.npmjs.com/package/node-red-contrib-mongodb3)

7. Run node red service and navigate to node red admin 

<img src="https://thingiverse-production-new.s3.amazonaws.com/assets/e1/e1/19/be/e3/2019-07-01_12_05_49-Greenshot.png" width="800"/>

8. Import beegl_server_flow.json
9. Configure beegl-mqtt-broker configuration node accordingly (host, port, username, password,...)
10. Configure beegl-mongodb configuration node accordingly (url, database, username, password,...)
11. Deploy
12. Navigate to `http://<node red hostname | ip>:1880/nodered/ui`

<img src="https://thingiverse-production-new.s3.amazonaws.com/assets/7f/aa/e4/93/b1/Node-RED_Dashboard.png" width="800"/>

### Migrate from <1.2.0

1. Execute steps 2, 3, 4, 5 and 6 described in [New installation](#new-installation)
2. Migrate data to mongodb by executing migrate_to_mongodb.js. The script reads node red flow.json file to obtain devices and log.txt file to obtain measurements. Change file locations (--flowcontext, --logfile), connection parameters, credentials (-dburl) and database name (-dbname) accordingly
    ``` 
    node ./migrate_to_mongodb.js  --flowcontext /home/ubuntu/.node-red/context/9a302785.7fd8f8/flow.json --logfile /home/ubuntu/beegl/log.txt --dburl mongodb://beegl:abc123@localhost:27017/beegl --dbname beegl
    ```	





