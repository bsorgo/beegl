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
#include <PubSubClient.h>


class MqttPublishStrategy : public PublishStrategy
{
public:
  MqttPublishStrategy(Runtime *runtime, Settings *settings, Connection *outboundConnection, Service *service);
  void setup() override;
  void update() override;
  bool reconnect() override;
  bool publishMessage(const char *message) override;
  const char getProtocol() {return 0x1;}
  const char* getProtocolName() { return "MQTT";}
  int getInterval() { return 60000; }
  const char getSupportedOutboundTypes() { return 0x3;}
  
private:
  PubSubClient *mqttClient;

protected:
  
};

#endif