/*
  WiFiBrokerInboundStrategy.h - Broker Inbound trrategy header file for Wifi
    
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

#ifndef WiFiBrokerInboundStrategy_h
#define WiFiBrokerInboundStrategy_h

#define SERVICE_UUID "ebd31aa0-b7c7-497a-a92c-e18f78f64efa"
#define CHARACTERISTIC_UUID "df878320-0c82-45c9-a5e2-47ff1ee43883"

#include "Broker.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <AsyncJson.h>

typedef char *(*jsonFunctionPtr)(JsonObject &jsonObj);

class WiFiBrokerInboundStrategy : public BrokerInboundStrategy
{
public:
  WiFiBrokerInboundStrategy(Service *server, Settings *settings);
  virtual bool setup() override;
  const char getInboundType() { return 0x01;};

private:
  AsyncCallbackJsonWebHandler *sensorsHandler;
};
#endif
