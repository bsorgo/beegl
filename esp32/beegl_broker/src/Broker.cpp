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
namespace beegl
{
Broker::Broker(Connection *connection, Service *server, Settings *settings, Publisher *publisher) : ISettingsHandler(settings)
{
  m_connection = connection;
  m_server = server;
  m_publisher = publisher;
}
int Broker::registerInboundStrategy(BrokerInboundStrategy *inboundStrategy)
{
  inboundStrategy->setBroker(this);
  m_inboundStrategies[inboundStartegyCount] = inboundStrategy;
  inboundStartegyCount++;
  return inboundStartegyCount;
}

void Broker::setup()
{
  for (int i = 0; i < inboundStartegyCount; i++)
  {
    if (m_connection->getInboundMode() & m_inboundStrategies[i]->getInboundType())
    {
      if (!m_inboundStrategies[i]->setup())
      {
        btlog_e(TAG_BROKER, "Error setting up broker strategy: %u", m_inboundStrategies[i]->getInboundType());
      }
    }
  }
}
void Broker::processMessage(const JsonObject &message)
{
  char temp[MESSAGE_SIZE];
  serializeJson(message, temp);
  StaticJsonDocument<MESSAGE_DOCUMENT_SIZE> *doc = new StaticJsonDocument<MESSAGE_DOCUMENT_SIZE>();
  bool res = m_serializer.deserialize(doc, temp);
  if (res)
  {
    JsonObject root = doc->as<JsonObject>();
    enrich(root);
    m_publisher->store(doc);
  }
  else
  {
    btlog_e(TAG_BROKER, "Cant process message.");
  }
}

void Broker::processMessage(const char *message)
{
  StaticJsonDocument<MESSAGE_DOCUMENT_SIZE> *doc = new StaticJsonDocument<MESSAGE_DOCUMENT_SIZE>();
  bool res = m_serializer.deserialize(doc, message);
  if(res)
  {
    JsonObject root = doc->as<JsonObject>();
    enrich(root);
    m_publisher->store(doc);
  }
  else
  {
    btlog_e(TAG_BROKER, "Cant process message.");
  }
}

void Broker::enrich(JsonObject &root)
{
  root[STR_EPOCHTIME] = TimeManagement::getInstance()->getUTCTime();
}
} // namespace beegl