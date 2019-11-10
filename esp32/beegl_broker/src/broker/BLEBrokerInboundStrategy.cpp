/*
  BLEBrokerInboundStrategy.cpp -   Broker strategy for BLE bluetooth
  
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

#include "broker/BLEBrokerInboundStrategy.h"
namespace beegl
{

BLEBrokerInboundStrategy::BLEBrokerInboundStrategy(Service *server, Settings *settings) : BrokerInboundStrategy(server, settings)
{
  m_bleCallback = new BLESensorCallback(this->m_broker);
}
BLEBrokerInboundStrategy *BLEBrokerInboundStrategy::createAndRegister(BeeGl *core)
{
  BLEBrokerInboundStrategy * i = new BLEBrokerInboundStrategy(&core->service, &core->settings);
  core->registerBrokerInboundStrategy(i);
  return i;
}
bool BLEBrokerInboundStrategy::setup()
{
  btlog_i(TAG_BLEBROKER, "Configuring BLE Server %s", m_settings->deviceName);
  btlog_i(TAG_BLEBROKER, "Service %s", SERVICE_UUID);
  btlog_i(TAG_BLEBROKER, "Characteristic %s", CHARACTERISTIC_UUID);
  if (esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P4) == OK)
  {
    btlog_i(TAG_BLEBROKER, "Transmission power changed\n");
  }
  BLEDevice::init(m_settings->deviceName);
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_INDICATE);

  ;

  pCharacteristic->setCallbacks(m_bleCallback);
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();

  return true;
}

void BLEBrokerInboundStrategy::BLESensorCallback::onWrite(BLECharacteristic *pCharacteristic)
{
  std::string value = pCharacteristic->getValue();
  if (value.length() > 0)
  {
    m_broker->processMessage(value.c_str());
  }
}
} // namespace beegl