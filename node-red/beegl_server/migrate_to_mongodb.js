const program = require('commander');

program
    .version("1.0.0")
    .option('-f, --flowcontext <flowcontextfilename>', 'Node-red flow context file path')
    .option('-l, --logfile <beeglmeasurementfilename>', 'Beegl legacy log.txt file path')
    .option('-d, --dburl <dburl>', 'Mongo server url mongodb://<username>:<password>@<host>:<port>')
    .option('-n, --dbname <dbname>', 'Mongo DB name')
program.parse(process.argv);
console.log(`Context flow path: ${program.flowcontext}`);
console.log(`Log path: ${program.flowcontext}`);
var fs = require("fs");
const MongoClient = require('mongodb').MongoClient;
const assert = require('assert');
const csv = require('csv-parser');

var logFilename = program.logfile;
console.log("Reading context flow")
var nodeRedFlowsContextContent = fs.readFileSync(program.flowcontext)
console.log("Parsing ...")
var nodeRedContextFlow = JSON.parse(nodeRedFlowsContextContent);

const url = program.dburl;
// Create a new MongoClien

MongoClient.connect(url, function (err, client) {
    assert.equal(null, err);
    console.log("Connected successfully to server");
    const db = client.db(program.dbname);
    const col = db.collection('devices');
    var bulkUpdateOps = [],
        counter = 0;

    nodeRedContextFlow.scales.scales.forEach(function (scale) {
        if (typeof scale.id !== 'undefined') {
            bulkUpdateOps.push({
                "updateOne": {
                    "filter": { "id": scale.id },
                    "update": { "$set": scale },
                    "upsert": true
                }
            });
        }
        counter++;

        if (counter % 500 == 0) {
            console.log(`Writing ${counter}`)
            col.bulkWrite(bulkUpdateOps, function (err, r) {
                assert.equal(null, err);
            });
            bulkUpdateOps = [];
        }

    })

    if (counter % 500 != 0) {
        console.log(`Writing ${counter}`)
        col.bulkWrite(bulkUpdateOps, function (err, r) {
            assert.equal(null, err);
            col.ensureIndex({ id: 1 }, function (err, r) {
                assert.equal(null, err);
                client.close();
            })
        });
    }
});
console.log("Reading log filename: " + logFilename);
console.log("Parsing ...")
var measurements = [];
fs.createReadStream(logFilename)
    .pipe(csv({ headers: false, separator: ';' }))
    .on('data', (data) => {
        if (data['1'] != "" && data['0'] != "") {
            measurements.push(
                {
                    "time": new Date(data['0']),
                    "deviceId": data['1'],
                    "weight": data['2'],
                    "weightUnit": data['3'],
                    "temperature": data['4'],
                    "temperatureUnit": data['5'],
                    "humidity": data['6'],
                    "humidityUnit": data['7']
                }
            )
        }
    })
    .on('end', () => {
        MongoClient.connect(url, function (err, client) {
            assert.equal(null, err);
            console.log("Connected successfully to server");

            const db = client.db(program.dbname);
            const col = db.collection('measurements');
            var bulkUpdateOps = [],
                counter = 0;

            measurements.forEach(function (measurement) {
                bulkUpdateOps.push({
                    "updateOne": {
                        "filter": { "time": measurement.time, "deviceId": measurement.deviceId },
                        "update": { "$set": measurement },
                        "upsert": true
                    }
                });
                counter++;

                if (counter % 500 == 0) {
                    console.log(`Writing ${counter}`)
                    col.bulkWrite(bulkUpdateOps, function (err, r) {
                        assert.equal(null, err);
                    });
                    bulkUpdateOps = [];
                }
            })
            if (counter % 500 != 0) {
                console.log(`Writing ${counter}`)
                col.bulkWrite(bulkUpdateOps, function (err, r) {
                    assert.equal(null, err);
                    client.close();
                });
            }
        });
    });