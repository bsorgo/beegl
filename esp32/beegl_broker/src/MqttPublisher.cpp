/*
  MqttPublisher.cpp - Provides Temporary storage and publishes messages to central server over Mqtt
  
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
  blog_i("[MQTT] Setup");
  mqttClient->setClient(*m_connection->getClient());
  blog_i("[MQTT] Server: mqttServer ");
  blog_i("[MQTT] Server: %s, port %u",m_settings->mqttServer, m_settings->mqttPort);
  mqttClient->setServer(m_settings->mqttServer, m_settings->mqttPort);
}

void MqttPublisher::update()
{
}

MqttPublisher::MqttPublisher(Runtime * runtime, Settings *settings, Connection *connection, Service *service) : Publisher(runtime, settings, connection, service)
{
    mqttClient = new PubSubClient(*m_connection->getClient());
}

bool MqttPublisher::publishMessage(const char *message)
{

    blog_d( "[MQTTPUBLISHER] %s", m_settings->sensorTopic);
    
    if (mqttClient->publish(m_settings->sensorTopic, message))
    {
        blog_d( "[MQTTPUBLISHER] Publish OK");
        return true;
    }
    else
    {
        blog_e( "[MQTTPUBLISHER] Publish NOK");
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
        blog_d( "[MQTTPUBLISHER] Client: %s, username: %s ", clientName.c_str(), m_settings->mqttUsername);
        if (mqttClient->connect((char *)clientName.c_str(), m_settings->mqttUsername, m_settings->mqttPassword))
        {
            blog_d( "[MQTTPUBLISHER] Connected. ");
            return true;
        }
        else
        {
            blog_e( "[MQTTPUBLISHER] failed, rc=%u, try again in 3 seconds", mqttClient->state());
            // Wait 5 seconds before retrying
            delay(3000);
            return false;
        }
    }

    return true;
}