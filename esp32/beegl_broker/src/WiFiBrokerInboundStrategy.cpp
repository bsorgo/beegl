/*
  WiFiBrokerInboundStrategy.cpp -   Broker strategy for BLE bluetooth
  
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

#include "WiFiBrokerInboundStrategy.h"

WiFiBrokerInboundStrategy::WiFiBrokerInboundStrategy(Service *server, Settings *settings) : BrokerInboundStrategy(server, settings)
{
  
}
bool WiFiBrokerInboundStrategy::setup()
{
    sensorsHandler = new AsyncCallbackJsonWebHandler("/beegl/v1/measurements", [&](AsyncWebServerRequest *request, JsonVariant &json) {
      JsonObject jsonObj = json.as<JsonObject>();
     
      int size = m_broker->processMessage(jsonObj);
      if(size)
      {
        request->send(200, "text/plain", "");
      }
      else
      {
        request->send(405, "text/plain", "");
      }
    });
    m_server->getWebServer()->addHandler(sensorsHandler);
  

  return true;
}

