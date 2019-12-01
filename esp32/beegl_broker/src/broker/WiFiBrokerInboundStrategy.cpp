/*
  WiFiBrokerInboundStrategy.cpp -   Broker strategy for WiFi (HTTP)
  
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

#include "broker/WiFiBrokerInboundStrategy.h"

namespace beegl
{

WiFiBrokerInboundStrategy::WiFiBrokerInboundStrategy(Service *server, Settings *settings) : BrokerInboundStrategy(server, settings)
{
}
WiFiBrokerInboundStrategy *WiFiBrokerInboundStrategy::createAndRegister(BeeGl *core)
{
  WiFiBrokerInboundStrategy * i = new WiFiBrokerInboundStrategy(&core->service, &core->settings);
  core->registerBrokerInboundStrategy(i);
  return i;
}
bool WiFiBrokerInboundStrategy::setup()
{
  sensorsHandler = new AsyncCallbackJsonWebHandler(WIFIBROKER_PATH, [&](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject root = json.as<JsonObject>();
    m_broker->processMessage(root);
    request->send(200, "text/plain", "");
  });
  m_server->getWebServer()->addHandler(sensorsHandler);

  return true;
}

void WiFiBrokerInboundStrategy::getInfo(JsonObject &target)
{
  JsonObject info = target.createNestedObject("WiFi HTTP broker");
  info["Handler path"] = WIFIBROKER_PATH;
}
} // namespace beegl
