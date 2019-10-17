/*
  Broker.cpp - 
  Broker functionality using inbound wifi and/or BLE bluetooth
  
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

#include "Broker.h"

Broker::Broker(Service *server, Settings *settings, Publisher *publisher)
{
  m_server = server;
  m_settings = settings;
  m_publisher = publisher;
}

void Broker::registerInboundStrategy(BrokerInboundStrategy *inboundStrategy)
{
  inboundStrategy->setBroker(this);
  m_inboundStrategies[inboundStartegyCount] = inboundStrategy;
  inboundStartegyCount++;
}

void Broker::webServerBind()
{
}

void Broker::setup()
{
  bool ret = false;
  for (int i = 0; i < inboundStartegyCount; i++)
  {
    if(m_settings->inboundMode & m_inboundStrategies[i]->getInboundType())
    {
      if(!m_inboundStrategies[i]->setup())
      {
        blog_e("[BROKER] Error setting up broker strategy: %u", m_inboundStrategies[i]->getInboundType());
      }
    }
  }
}
int Broker::processMessage(const JsonObject &jsonObject)
{
  jsonObject[STR_EPOCHTIME] = TimeManagement::getInstance()->getUTCTime();
  size_t size = measureJson(jsonObject);
  char buffer[size + 1];
  serializeJson(jsonObject, buffer, size);

  if (m_publisher)
  {
    m_publisher->storeMessage(buffer);
  }
  return size;
}
int Broker::processMessage(const char *buffer)
{
  StaticJsonDocument<512> jsonBuffer;
  auto error = deserializeJson(jsonBuffer, buffer);
  if (error)
  {
    log_e("[BROKER] parseObject() failed");
    return -1;
  }
  else
  {
    JsonObject jsonObj = jsonBuffer.as<JsonObject>();
    this->processMessage(jsonObj);
  }
}
