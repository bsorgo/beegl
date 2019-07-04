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
    m_bleCallback = new BLESensorCallback(this);
}

void Broker::webServerBind()
{
    if (m_settings->inboundMode & 0x1)
    {
        sensorsHandler = new AsyncCallbackJsonWebHandler("/beegl/v1/measurements", [&](AsyncWebServerRequest *request, JsonVariant &json) {
            JsonObject &jsonObj = json.as<JsonObject>();
            jsonObj[STR_TIME] = m_settings->getDateTimeString(now());
            m_publisher->storeMessage(jsonObj);
            request->send(200, "text/plain", "");
        });
        m_server->getWebServer()->addHandler(sensorsHandler);
    }
}

void Broker::setSettings(Settings *settings)
{
    m_settings = settings;
}
void Broker::setPublisher(Publisher *publisher)
{
    m_publisher = publisher;
}

void Broker::setup()
{
    webServerBind();
    bleBind();
}

bool Broker::bleBind()
{
    if (m_settings->inboundMode & 0x2)
    {
        log_i( "[BLE] Configuring BLE Server %s ", m_settings->deviceName);
        log_i( "[BLE] Service %s ", SERVICE_UUID);
        log_i( "[BLE] Characteristic %s ", CHARACTERISTIC_UUID);

        if (esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P4) == OK)
        {
            log_i( "[BLE] Transmission power changed\n");
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
    }
    return true;
}

char *Broker::storeMessage(JsonObject &jsonObj)
{
    return m_publisher->storeMessage(jsonObj);
}
