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

#define SERVICE_UUID "ebd31aa0-b7c7-497a-a92c-e18f78f64efa"
#define CHARACTERISTIC_UUID "df878320-0c82-45c9-a5e2-47ff1ee43883"

#include "Log.h"
#include "Service.h"
#include "Publisher.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <AsyncJson.h>

typedef char* (*jsonFunctionPtr)(JsonObject &jsonObj);

class Broker {

class BLESensorCallback : public BLECharacteristicCallbacks
{

private:
  Broker* m_broker;
public:
  BLESensorCallback(Broker* broker)
  {
    m_broker = broker;
  }
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0)
    {
      StaticJsonDocument<512> jsonBuffer;
      auto error = deserializeJson(jsonBuffer, value.c_str());
      if (error)
      {

        log_e( "[BLE] parseObject() failed");
      }
      else
      {
        JsonObject jsonObj = jsonBuffer.as<JsonObject>();
        if (m_broker)
        {
          m_broker->storeMessage(jsonObj);
        }
      }
    }
  }
};

public:
    Broker(Service* server,Settings *settings, Publisher* publisher);
    void setSettings(Settings *settings);
    void setPublisher(Publisher* publisher);

    void setup();
    char* storeMessage(JsonObject jsonObj);
private:
    Service* m_server;
    Settings* m_settings;
    Publisher* m_publisher;
    BLESensorCallback* m_bleCallback;

    BLEServer *pServer;
    BLECharacteristic *pCharacteristic;

    AsyncCallbackJsonWebHandler *sensorsHandler;

    bool bleBind();
    void webServerBind();
    

};

#endif
