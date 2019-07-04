/*
  Publisher.cpp - Provides Temporary storage and publishes messages to central server over Mqtt
  
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

#include "MqttPublisher.h"

void MqttPublisher::setup()
{
  log_i("[MQTT] Setup");
  mqttClient->setClient(*m_connection->getClient());
  log_i("[MQTT] Server: mqttServer ");
  log_i("[MQTT] Server: %s, port %u",m_settings->mqttServer, m_settings->mqttPort);
  mqttClient->setServer(m_settings->mqttServer, m_settings->mqttPort);
}

MqttPublisher::MqttPublisher(Runtime * runtime, Settings *settings, Connection *connection) : Publisher(runtime, settings, connection)
{
    mqttClient = new PubSubClient(*m_connection->getClient());
}

bool MqttPublisher::publishMessage(const char *message)
{

    log_d( "[MQTTPUBLISHER] %s", m_settings->sensorTopic);
    
    if (mqttClient->publish(m_settings->sensorTopic, message))
    {
        log_i( "[MQTTPUBLISHER] Publish OK");
        return true;
    }
    else
    {
        log_e( "[MQTTPUBLISHER] Publish NOK");
        return false;
    }
}


bool MqttPublisher::reconnect()
{
    if (!mqttClient->connected())
    {
        String clientName = "";
        clientName += m_settings->deviceName;
        clientName += "-";
        clientName += String(micros() & 0xff, 16);
        // Attempt to connect
        log_d( "[MQTTPUBLISHER] Client: %s, username: %s ", clientName.c_str(), m_settings->mqttUsername);
        if (mqttClient->connect((char *)clientName.c_str(), m_settings->mqttUsername, m_settings->mqttPassword))
        {
            log_d( "[MQTTPUBLISHER] Connected. ");
            return true;
        }
        else
        {
            log_e( "[MQTTPUBLISHER] failed, rc=%u, try again in 5 seconds", mqttClient->state());
            // Wait 5 seconds before retrying
            delay(5000);
            return false;
        }
    }

    return true;
}