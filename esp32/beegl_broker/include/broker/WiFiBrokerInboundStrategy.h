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


#include "Broker.h"
#include "BeeGl.h"
#include <AsyncJson.h>

#define TAG_WIFIBROKER "WIFIBROKER"

namespace beegl
{
class WiFiBrokerInboundStrategy : public BrokerInboundStrategy
{
public:
  WiFiBrokerInboundStrategy(Service *server, Settings *settings);
  static WiFiBrokerInboundStrategy* createAndRegister(BeeGl *core);

  virtual bool setup() override;
  const char getInboundType() const override { return 0x01; };

private:
  AsyncCallbackJsonWebHandler *sensorsHandler;
};
} // namespace beegl
#endif
