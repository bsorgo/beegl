/*
  MqttPublishStrategy.cpp - Provides Temporary storage and publishes messages to central server over Mqtt
  
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

#include "publisher/MqttPublishStrategy.h"
namespace beegl
{
MqttPublishStrategy::MqttPublishStrategy(Runtime *runtime, Settings *settings, Connection *connection, Service *service) : PublishStrategy(runtime, settings, connection, service)
{
  mqttClient = new PubSubClient(*PublishStrategy::m_connection->getClient());
}

MqttPublishStrategy *MqttPublishStrategy::createAndRegister(BeeGl *core)
{
  MqttPublishStrategy *i = new MqttPublishStrategy(&core->runtime, &core->settings, &core->connection, &core->service);
  core->registerPublishStrategy(i);
  return i;
}

void MqttPublishStrategy::setup()
{
  blog_i("[MQTT] Setup");
  mqttClient->setClient(*PublishStrategy::m_connection->getClient());
  blog_i("[MQTT] Server: mqttServer ");
  blog_i("[MQTT] Server: %s, port %u", m_mqttServer, m_mqttPort);
  mqttClient->setServer(m_mqttServer, m_mqttPort);
}
void MqttPublishStrategy::update()
{
}
bool MqttPublishStrategy::reconnect()
{
  if (!mqttClient->connected())
  {
    String clientName = "";
    clientName += ISettingsHandler::m_settings->deviceName;
    clientName += "-";
    clientName += String(micros() & 0xff, 16);
    // Attempt to connect
    blog_d("[MQTTPUBLISHER] Client: %s, username: %s ", clientName.c_str(), m_mqttUsername);
    if (mqttClient->connect((char *)clientName.c_str(), m_mqttUsername, m_mqttPassword))
    {
      blog_d("[MQTTPUBLISHER] Connected. ");
      return true;
    }
    else
    {
      blog_e("[MQTTPUBLISHER] failed, rc=%u, try again in 3 seconds", mqttClient->state());
      // Wait 5 seconds before retrying
      delay(3000);
      return false;
    }
  }

  return true;
}

bool MqttPublishStrategy::publishMessage(JsonDocument *payload)
{
  char message[MESSAGE_SIZE];
  m_serializer.serialize(payload, message);
  blog_d("[MQTTPUBLISHER] %s", m_sensorTopic);

  if (mqttClient->publish(m_sensorTopic, message))
  {
    blog_d("[MQTTPUBLISHER] Publish OK");
    return true;
  }
  blog_e("[MQTTPUBLISHER] Publish NOK");
  return false;
}
const char getProtocol() { return 0x1; }
const char *getProtocolName() { return "MQTT"; }
int getInterval() { return 60000; }
const char getSupportedOutboundTypes() { return 0x3; }

void MqttPublishStrategy::readSettings(const JsonObject &source)
{
  // mqtt settings

  JsonObject mqttSettings = source[STR_MQTTSETTINGS];
  strlcpy(m_mqttServer, mqttSettings[STR_MQTTSERVER] | m_mqttServer, 33);
  m_mqttPort = mqttSettings[STR_MQTTPORT] | 1883;
  strlcpy(m_mqttUsername, mqttSettings[STR_MQTTUSERNAME] | m_mqttUsername, 17);
  strlcpy(m_mqttPassword, mqttSettings[STR_MQTTPASSWORD] | m_mqttPassword, 17);
  strlcpy(m_sensorTopic, mqttSettings[STR_MQTTTOPIC] | m_sensorTopic, 65);
}
void MqttPublishStrategy::writeSettings(JsonObject &target, const JsonObject &input)
{
  JsonObject mqttSettings = target.createNestedObject(STR_MQTTSETTINGS);
  mqttSettings[STR_MQTTSERVER] = m_mqttServer;
  mqttSettings[STR_MQTTPORT] = m_mqttPort;
  mqttSettings[STR_MQTTUSERNAME] = m_mqttUsername;
  mqttSettings[STR_MQTTPASSWORD] = m_mqttPassword;
  mqttSettings[STR_MQTTTOPIC] = m_sensorTopic;

  Settings::merge(mqttSettings, input[STR_MQTTSETTINGS]);
}
} // namespace beegl