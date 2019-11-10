/*
  main.cpp - BeeGl Broker Main Program
  
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
#undef CONFIG_ARDUHAL_ESP_LOG
#define DEBUG 1

#ifndef ESP32
#error "Only ESP32 supported"
#endif
#include <Arduino.h>

#include "esp32-hal-log.h"

#include "time/HttpTimeProviderStrategy.h"

#ifdef SUPPORTSGSM
#include "connection/TinyGsmConnectionProvider.h"
#endif
#ifdef SUPPORTSLORAWAN
#include "message/WHTCLPPSerializer.h"
#include "connection/LoraWanConnectionProvider.h"
#include "publisher/LoraWanPublishStrategy.h"
#endif
#ifdef SUPPORTSBLE
#include "connection/BLEConnectionProvider.h"
#include "broker/BLEBrokerInboundStrategy.h"
#endif
#ifdef SUPPORTSRTC
#include "time/RTCTimeProviderStrategy.h"
#endif
#include "publisher/HttpPublishStrategy.h"
#include "publisher/MqttPublishStrategy.h"
#include "broker/WiFiBrokerInboundStrategy.h"

#include "measurer/DHT22TempAndHumidityMeasureProvider.h"
#include "measurer/HX711WeightMeasureProvider.h"

#ifdef HEAPTRACE
#include "esp_heap_trace.h"
#define HEAP_TRACE_NUM_RECORDS 100
static heap_trace_record_t trace_record[HEAP_TRACE_NUM_RECORDS]; // This buffer must be in internal RAM
#endif

#define SerialMon Serial
using namespace beegl;

BeeGl beeGl;

void setup()
{

  Serial.begin(115200);
  while (!Serial)
  {
    ;
  }

  beeGl.prepare();

#ifdef SUPPORTSGSM
  TinyGsmConnectionProvider::createAndRegister(&beeGl);
#endif
#ifdef SUPPORTSLORAWAN
  LoraWanConnectionProvider::createAndRegister(&beeGl);
#endif
#ifdef SUPPORTSBLE
  BLEConnectionProvider::createAndRegister(&beeGl);
#endif
  // Add default - No Time

  HttpTimeProviderStrategy::createAndRegister(&beeGl);
#ifdef SUPPORTSRTC
  RTCTimeProviderStrategy::createAndRegister(&beeGl);
#endif
  HX711WeightMeasureProvider::createAndRegister(&beeGl);
  DHT22TempAndHumidityMeasureProvider::createAndRegister(&beeGl);
  MqttPublishStrategy::createAndRegister(&beeGl);
  HttpPublishStrategy::createAndRegister(&beeGl);
#ifdef SUPPORTSLORAWAN
  WHTCLPPSerializer *loraFormatter = new WHTCLPPSerializer();
  LoraWanPublishStrategy::createAndRegister(&beeGl, loraFormatter);
#endif

  WiFiBrokerInboundStrategy::createAndRegister(&beeGl);
#ifdef SUPPORTSBLE
  BLEBrokerInboundStrategy::createAndRegister(&beeGl);
#endif
  beeGl.begin();
}

void loop()
{
  beeGl.update();
  delay(10);
}
