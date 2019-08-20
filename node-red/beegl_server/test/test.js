var supertest = require("supertest");
var should = require("should");
var server = supertest.agent("http://localhost:1880");
var mqtt = require('mqtt');

describe("HTTP Devices", function () {
    describe("POST /beegl/v1/devices/:id", function () {
        it("should insert device", function (done) {
            server
                .post("/beegl/v1/devices/BEETTT")
                .send({ "id": "BEETTT" })
                .expect("Content-type", /json/)
                .expect(200)
                .end(function (err, res) {
                    res.status.should.equal(200);
                    done();
                });
        });
        it("should not insert device", function (done) {
            server
                .post("/beegl/v1/devices/BEETTT")
                .send({ "idxxxx": "BEETTT" })
                .expect("Content-type", /json/)
                .expect(405)
                .end(function (err, res) {
                    res.status.should.equal(405);
                    res.body.should.have.property('error');
                    done();
                });
        });

        it("should update device", function (done) {
            server
                .post("/beegl/v1/devices/BEETTT")
                .send({ "id": "BEETTT", "restInt": 3600 })
                .expect("Content-type", /json/)
                .expect(200)
                .end(function (err, res) {
                    res.status.should.equal(200);
                    server
                        .get("/beegl/v1/devices/BEETTT")
                        .expect("Content-type", /json/)
                        .expect(200)
                        .end(function (err, res) {
                            res.status.should.equal(200);
                            res.body.id.should.equal("BEETTT");
                            res.body.restInt.should.equal(3600);
                            done();
                        });
                });
        });
    });
    describe("GET /beegl/v1/devices/:id", function () {
        it("should get device", function (done) {
            server
                .get("/beegl/v1/devices/BEETTT")
                .expect("Content-type", /json/)
                .expect(200)
                .end(function (err, res) {
                    res.status.should.equal(200);
                    res.body.id.should.equal("BEETTT");
                    done();
                });
        });
        it("should not get device", function (done) {
            server
                .get("/beegl/v1/devices/BEETTTxxx")
                .expect("Content-type", /json/)
                .expect(404)
                .end(function (err, res) {
                    res.status.should.equal(404);
                    done();
                });
        });
    });
    describe("GET /beegl/v1/devices", function () {
        it("should get devices", function (done) {
            server
                .get("/beegl/v1/devices")
                .expect("Content-type", /json/)
                .expect(200)
                .end(function (err, res) {
                    res.status.should.equal(200);
                    var len = res.body.length;
                    len.should.be.above(0);
                    done();
                });
        });
    });
    describe("DELETE /beegl/v1/devices/:id", function () {
        it("should remove device", function (done) {
            server
                .delete("/beegl/v1/devices/BEETTT?removeMeasurements=true")
                .expect("Content-type", /json/)
                .expect(200)
                .end(function (err, res) {
                    res.status.should.equal(200);
                    server
                        .get("/beegl/v1/devices/BEETTT")
                        .expect("Content-type", /json/)
                        .expect(404)
                        .end(function (err, res) {
                            res.status.should.equal(404);
                            done();
                        });
                });
        });
    });
});

describe("HTTP Measurements", function () {

    beforeEach(done => setTimeout(done, 2000));
    before(function (done) {
        server
            .post("/beegl/v1/devices/BEETTT")
            .send({ "id": "BEETTT" })
            .expect("Content-type", /json/)
            .expect(200)
            .end(function (err, res) {
                res.status.should.equal(200);
                done();
            });
    });
    after(function (done) {
        server
            .delete("/beegl/v1/devices/BEETTT?removeMeasurements=true")
            .expect("Content-type", /json/)
            .expect(200)
            .end(function (err, res) {
                res.status.should.equal(200);
                done();
            });
    });
    describe("POST /beegl/v1/measurements", function () {
        it("should insert measurement(2) - id", function (done) {
            server
                .post("/beegl/v1/measurements")
                .send({ "time": new Date("2017-05-18T16:02:00Z"), "id": "BEETTT" })
                .expect(200)
                .end(function (err, res) {
                    res.status.should.equal(200);
                    server
                    .post("/beegl/v1/measurements")
                    .send({ "time": new Date("2017-05-18T16:01:01Z"), "id": "BEETTT" })
                    .expect(200)
                    .end(function (err, res) {
                        res.status.should.equal(200);
                    });
                    done();
                });
        });
    });
    describe("POST /beegl/v1/measurements", function () {
        it("should insert measurement - deviceId", function (done) {
            server
                .post("/beegl/v1/measurements")
                .send({ "time": new Date("2017-05-18T16:00:01Z"), "deviceId": "BEETTT" })
                .expect(200)
                .end(function (err, res) {
                    
                    res.status.should.equal(200);
                    done();
                });
        });
    });
    describe("POST /beegl/v1/measurements", function () {
        it("should not insert measurement - no time", function (done) {
            server
                .post("/beegl/v1/measurements")
                .send({ "timexxxx": new Date(), "id": "BEETTT" })
                .expect(405)
                .end(function (err, res) {

                    res.status.should.equal(405);
                    done();
                });
        });
    });
    describe("POST /beegl/v1/measurements", function () {
        it("should not insert measurement - no id ", function (done) {
            server
                .post("/beegl/v1/measurements")
                .send({ "time": new Date() })
                .expect(405)
                .end(function (err, res) {

                    res.status.should.equal(405);
                    done();
                });
        });
    });
    describe("GET /beegl/v1/measurements", function () {
        it("should get measurements", function (done) {
            server
                .get("/beegl/v1/measurements?deviceId=BEETTT")
                .expect(200)
                .end(function (err, res) {
                    res.status.should.equal(200);
                    res.body.should.have.length(3);
                    var time = new Date(res.body[0].time);
                    time.should.eql(new Date("2017-05-18T16:00:01Z"));
                    done();
                });

        });
        it("should get measurements with page & size", function (done) {
            server
                .get("/beegl/v1/measurements?deviceId=BEETTT&pageNo=2&size=1")
                .expect(200)
                .end(function (err, res) {
                    res.status.should.equal(200);
                    res.body.should.have.length(1);
                    done();
                });

        });
    });
});

describe("MQTT & HTTP Measurements", function () {
    before(function (done) {
        server
            .post("/beegl/v1/devices/BEETTT")
            .send({ "id": "BEETTT" })
            .expect("Content-type", /json/)
            .expect(200)
            .end(function (err, res) {
                res.status.should.equal(200);
                done();
            });
    });
    after(function (done) {
        server
            .delete("/beegl/v1/devices/BEETTT?removeMeasurements=true")
            .expect("Content-type", /json/)
            .expect(200)
            .end(function (err, res) {
                res.status.should.equal(200);
                done();
            });
    });
    beforeEach(done => setTimeout(done, 2000));
    
    describe("Publish measurements", function () {
        it("should publish measurement", function (done) {
            var mqttClient  = mqtt.connect('mqtt://localhost:1883');
            mqttClient.on('connect', function () {
                mqttClient.publish('measurements', JSON.stringify({ "time": new Date(), "id": "BEETTT" }), function(err) {
                    should.not.exist(err);
                    done();
                });
            });
        });
        it("should publish measurement no id", function (done) {
            var mqttClient  = mqtt.connect('mqtt://localhost:1883');
            mqttClient.on('connect', function () {
                mqttClient.publish('measurements', JSON.stringify({ "time": new Date(),  }), function(err) {
                    should.not.exist(err);
                    done();
                });
            });
        });
    });
    
    describe("Publish scale_sensors", function () {
        it("should publish measurement", function (done) {
            var mqttClient  = mqtt.connect('mqtt://localhost:1883');
            mqttClient.on('connect', function () {
                mqttClient.publish('scale_sensors', JSON.stringify({ "time": new Date(), "id": "BEETTT" }), function(err) {
                    should.not.exist(err);
                    done();
                    return;
                });
            });
        });
    });
    describe("GET /beegl/v1/measurements", function () {
        it("should get measurements", function (done) {
            server
                .get("/beegl/v1/measurements?deviceId=BEETTT")
                .expect(200)
                .end(function (err, res) {
                    res.status.should.equal(200);
                    res.body.should.have.length(2);
                    done();
                });

        });
    });
});













