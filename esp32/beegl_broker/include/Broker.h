/*
  Broker.h - Broker header file
    
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

#ifndef Broker_h
#define Broker_h

#include "Log.h"
#include "Service.h"
#include "Publisher.h"
#include "Message.h"

#include <AsyncJson.h>
namespace beegl
{
class IBrokerProcessor
{
public:
  virtual void processMessage(const JsonObject &message) {}
  virtual void processMessage(const char *message) {}
};

class BrokerInboundStrategy : public ISettingsHandler
{
public:
  BrokerInboundStrategy(Service *server, Settings *settings) : ISettingsHandler(settings)
  {
    m_server = server;
  }

  void setBroker(IBrokerProcessor *broker) { m_broker = broker; };
  virtual bool setup() { return false; };
  virtual const char getInboundType() const = 0;

protected:
  IBrokerProcessor *m_broker;
  Service *m_server;
};

class Broker : public IBrokerProcessor, public ISettingsHandler
{
public:
  Broker(Connection *connection, Service *server, Settings *settings, Publisher *publisher);
  int registerInboundStrategy(BrokerInboundStrategy *inboundStrategy);
  void setup();
  void processMessage(const JsonObject &message) override;
  void processMessage(const char *message) override;
  void enrich(JsonObject &root);

private:
  Connection *m_connection;
  Service *m_server;
  Publisher *m_publisher;
  JsonMessageSerializer m_serializer;
  void webServerBind();
  BrokerInboundStrategy *m_inboundStrategies[5];
  int inboundStartegyCount = 0;
};
} // namespace beegl

#endif
