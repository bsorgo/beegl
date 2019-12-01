/*
  BLEBrokerInboundStrategy.h - Broker Inbound STrategy header file for BLE 
    
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

#ifndef BLEBrokerInboundStrategy_h
#define BLEBrokerInboundStrategy_h

#define SERVICE_UUID "ebd31aa0-b7c7-497a-a92c-e18f78f64efa"
#define CHARACTERISTIC_UUID "df878320-0c82-45c9-a5e2-47ff1ee43883"

#include "Broker.h"
#include "BeeGl.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <AsyncJson.h>

#define TAG_BLEBROKER "BLEBROKER"

namespace beegl
{
typedef char *(*jsonFunctionPtr)(JsonObject &jsonObj);

class BLEBrokerInboundStrategy : public BrokerInboundStrategy
{
public:
  class BLESensorCallback : public BLECharacteristicCallbacks
  {
  public:
    BLESensorCallback(IBrokerProcessor *broker)
    {
      m_broker = broker;
    }
    void onWrite(BLECharacteristic *pCharacteristic);
    
  private:
    IBrokerProcessor *m_broker;
  };

  BLEBrokerInboundStrategy(Service *server, Settings *settings);
  static BLEBrokerInboundStrategy* createAndRegister(BeeGl *core);
  
  virtual bool setup() override;
  const char getInboundType() const override { return 0x02; };

  void getInfo(JsonObject &target) override;
private:
  BLESensorCallback *m_bleCallback;
  BLEServer *pServer;
  BLECharacteristic *pCharacteristic;
};
} // namespace beegl
#endif
