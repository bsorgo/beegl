/*
  MqttPublishStrategy.h - Mqtt PublishStrategy header file
  
  This file is part of the BeeGl distribution (https://github.com/bsorgo/beegl).
  Copyright (c) 2019 Bostjan Sorgo
  
  This program is free software: you can redistribute it and/or modify  
  it under the terms of the GNU General Public License as published by  
  the Free Software Foundation, version 3.
 
  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
  General Public License for more details.
 
  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef MqttPublishStrategy_h
#define MqttPublishStrategy_h

#include "Publisher.h"
#include "Message.h"
#include "BeeGl.h"
#include <PubSubClient.h>

#define STR_MQTTSETTINGS "mqttS"
#define STR_MQTTSERVER "server"
#define STR_MQTTPORT "port"
#define STR_MQTTUSERNAME "user"
#define STR_MQTTPASSWORD "pwd"
#define STR_MQTTTOPIC "topic"
namespace beegl
{
class MqttPublishStrategy : public PublishStrategy
{
public:
  MqttPublishStrategy(Runtime *runtime, Settings *settings, Connection *connection, Service *service);
  static MqttPublishStrategy* createAndRegister(BeeGl *core);

  void setup() override;
  void update() override;
  bool reconnect() override;
  bool publishMessage(JsonDocument *payload) override;

  const char getProtocol() const override { return 0x1; }
  const char *getProtocolName() const override { return "MQTT"; }
  const int getInterval() const override { return 60000; }
  const char getSupportedOutboundTypes() const override { return 0x3; }

  void readSettings(const JsonObject &source) override;
  void writeSettings(JsonObject &target, const JsonObject &input) override;

  const char *getMqttServer()
  {
    return m_mqttServer;
  }
  int getMqttPort()
  {
    return m_mqttPort;
  }
  const char *getMqttUsername()
  {
    return m_mqttUsername;
  }
  const char *getMqttPassword()
  {
    return m_mqttPassword;
  }
  const char *getSensorTopic()
  {
    return m_sensorTopic;
  }

private:
  PubSubClient *mqttClient;
  char m_mqttServer[33] = "mqtt.example.com";
  int m_mqttPort = 1883;
  char m_mqttUsername[17] = "iot_username";
  char m_mqttPassword[17] = "iot_password";
  char m_sensorTopic[64] = "measurements";
  JsonMessageSerializer m_serializer;
};
} // namespace beegl
#endif
