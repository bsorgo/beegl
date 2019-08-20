const program = require('commander');
const MongoClient = require('mongodb').MongoClient;
const assert = require('assert');
program
  .version("1.0.0")
  .option('-d, --dburl <dburl>', 'Mongo server url mongodb://<username>:<password>@<host>:<port>')
  .option('-n, --dbname <dbname>', 'Mongo DB name')
program.parse(process.argv);

MongoClient.connect(program.dburl, function (err, client) {
  if (err) throw err;
  console.log("Connected successfully to server");
  const db = client.db(program.dbname);
  db.createCollection("devices", {
    validator: {
      $jsonSchema: {
        bsonType: "object",
        required: ["id"],
        properties: {
          id: {
            bsonType: "string",
            description: "must be a string and is required"
          }
        }
      }
    }
  }, function (err, res) {
    if (err) throw err;
    db.collection("devices").createIndex({ id: 1 },{ unique: true }, function (err, res) {
      if (err) throw err;
      client.close();
    });
  });
});

MongoClient.connect(program.dburl, function (err, client) {
  assert.equal(null, err);
  console.log("Connected successfully to server");
  const db = client.db(program.dbname);
  db.createCollection("measurements", {
    validator: {
      $jsonSchema: {
        bsonType: "object",
        required: ["deviceId", "time"],
        properties: {
          deviceId: {
            bsonType: "string",
            description: "must be a string and is required"
          },
          time: {
            bsonType: "date",
            description: "must be a date and is required"
          }
        }
      }
    }
  }, function (err, res) {
    if (err) throw err;
    db.collection("measurements").createIndex({ time: 1 }, function (err, res) {
      if (err) throw err;
      db.collection("measurements").createIndex({ deviceId: 1 }, function (err, res) {
        if (err) throw err;
        client.close();
      });
    });
  });
});