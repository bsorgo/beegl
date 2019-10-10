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
#include <SPI.h>
#include <WiFi.h>
#include <timer.h>
#include <timerManager.h>
#include <ArduinoNvs.h>
#include <Storage.h>
#include <EEPROM.h>
#include <TimeLib.h>
#include <unistd.h>
#include "esp32-hal-log.h"
#include "Indicator.h"
#include "Service.h"
#include "Settings.h"
#include "Connection.h"
#ifdef SUPPORTSGSM
#include "TinyGsmConnectionProvider.h"
#endif
#ifdef SUPPORTSLORAWAN
#include "LoraWanConnectionProvider.h"
#include "LoraWanPublishStrategy.h"
#endif
#ifdef SUPPORTSBLE
#include "BLEConnectionProvider.h"
#endif
#include "WiFiConnectionProvider.h"
#include "Runtime.h"
#include "Updater.h"
#include "Publisher.h"
#include "HttpPublishStrategy.h"
#include "MqttPublishStrategy.h"

#include "Measurer.h"
#include "SettingsManagement.h"
#include "LogManagement.h"

#include "Broker.h"
#ifdef HEAPTRACE
#include "esp_heap_trace.h"
#define HEAP_TRACE_NUM_RECORDS 100
static heap_trace_record_t trace_record[HEAP_TRACE_NUM_RECORDS]; // This buffer must be in internal RAM
#endif

#define SerialMon Serial



Indicator indicator = Indicator();
Settings settings = Settings();
Service service = Service(&settings);
Connection connection = Connection(&settings);
Runtime runtime = Runtime(&service, &settings, &connection);
Updater updater = Updater(&runtime, &service, &settings, &connection);
Publisher publisher = Publisher(&runtime, &settings, &connection, &service);
SettingsManagement settingsManagement = SettingsManagement(&settings, &connection, &service, &runtime);
LogManagement logManagement = LogManagement(&settings, &service);
Measurer measurer = Measurer(&runtime, &service, &settings, &publisher);
Broker broker = Broker(&service, &settings, &publisher);

void reportStatus()
{
  if (runtime.getSafeMode())
  {
    indicator.reportFail();
  }
  else
  {
    indicator.reportSuccess();
  }
}

bool nvsSetup()
{
  Serial.println("[NVS] Begin NVS");
  if (!NVS.begin())
  {
    Serial.println("[NVS] An Error has occurred while initilizing NVS");
    return false;
  }
  return true;
}

void setup()
{

  Serial.begin(115200);
  while (!Serial)
  {
    ;
  }
  if (!nvsSetup() || !storage_setup())
  {
    indicator.reportFail(1);
    return;
  }
  else
  {
    indicator.reportSuccess(1);
  }
  connection.addConnectionProvider(new WiFiConnectionProvider(&settings));
#ifdef SUPPORTSGSM
  connection.addConnectionProvider(new TinyGsmConnectionProvider(&settings));
#endif
#ifdef SUPPORTSLORAWAN
  connection.addConnectionProvider(new LoraWanConnectionProvider(&settings));
#endif
#ifdef SUPPORTSBLE
  connection.addConnectionProvider(new BLEConnectionProvider(&settings));
#endif
  runtime.initialize();
  runtime.setSafeModeOnRestart(1);

  delay(500);
  Serial.flush();

  runtime.printWakeupReason();

  if (runtime.getSafeMode())
  {
    // start AP and web server only
    settings.outboundMode = 0x0;
    settings.inboundMode = 0x1;
    connection.setup();
    service.setup();
    runtime.setSafeModeOnRestart(0);
    log_i("***************************************");
    log_i("******** SAFE/MAINTENANCE MODE ********");
    log_i("***************************************");
    log_i("Use Wifi device, connect to SSID:%s. By using web browser navigate to http://%s", settings.deviceName, settings.apIp.toString().c_str());
    return;
  }

  if (!settingsManagement.readConfig())
  {
    indicator.reportFail(2);
    runtime.setSafeMode(1);
  }
  else
  {
    indicator.reportSuccess(2);
  }

  log_i("[ESP] Device name: %s ", settings.deviceName);
  log_i("[ESP] Inbound mode: %u ", settings.inboundMode);
  log_i("[ESP] Outbound mode: %u ", settings.outboundMode);

  if (!connection.setup())
  {
    indicator.reportFail(3);
    runtime.deepSleep();
  }
  else
  {
    indicator.reportSuccess(3);
  }

  service.setup();
  settingsManagement.syncTimeAndSettings();
  updater.checkFirmware();
  publisher.addPublishStrategy(new MqttPublishStrategy(&runtime, &settings, &connection, &service));
  publisher.addPublishStrategy(new HttpPublishStrategy(&runtime, &settings, &connection, &service));
#ifdef SUPPORTSLORAWAN
  publisher.addPublishStrategy(new LoraPublishStrategy(&runtime, &settings, &connection, &service));
#endif
  publisher.setup();
  measurer.setup();
  broker.setup();

  runtime.setSafeModeOnRestart(0);
  settingsManagement.storeLastGood();
  indicator.reportSuccess(4);
  connection.suspend();
  delay(2000);
  if (!runtime.getSafeMode())
  {
    measurer.begin();
    publisher.publish();
  }
}

void loop()
{
  reportStatus();
  runtime.update();
  publisher.update();
  delay(10);
}
