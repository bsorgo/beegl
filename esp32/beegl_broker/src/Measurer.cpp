/*
  Measurer.cpp - Performs scale measures like weight, temp and humidity
  
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

#include "Measurer.h"
namespace beegl
{
Measurer *Measurer::p_instance = NULL;

Measurer::Measurer(Runtime *runtime, Service *server, Settings *settings, Publisher *publisher) : ISettingsHandler(settings)
{

  m_server = server;
  m_publisher = publisher;
  m_runtime = runtime;
  p_instance = this;
  webServerBind();
}

int Measurer::registerMeasureProvider(MeasureProvider *measureProvider)
{
  measureProviders[measureProviderCount] = measureProvider;
  measureProviderCount++;
  return measureProviderCount;
}

Measurer *Measurer::getInstance()
{
  return p_instance;
}

void Measurer::webServerBind()
{
  m_server->getWebServer()->on("/rest/measure", HTTP_GET, [&](AsyncWebServerRequest *request) {
    JsonDocument *value = measure();
    char message[MESSAGE_SIZE];
    m_serializer.serialize(value, message);
    if (strlen(message) > 0)
    {
      btlog_d(TAG_MEASURER, "JsonDocument: %s", message);
      request->send(200, JSON_MIMETYPE, message);
    }
    else
    {
      request->send(500);
    }
  });
}

uint32_t Measurer::getMeasureInterval()
{
  return m_settings->refreshInterval;
}
void Measurer::setup()
{
  for (int i = 0; i < measureProviderCount; i++)
  {
    measureProviders[i]->setup();
  }
}

JsonDocument *Measurer::measure()
{
  StaticJsonDocument<512> *document = new StaticJsonDocument<512>();
  JsonObject root = document->to<JsonObject>();
  root[STR_DEVICEID] = String(m_settings->deviceName);
  root[STR_EPOCHTIME] = TimeManagement::getInstance()->getUTCTime();
  root[STR_VER] = m_runtime->FIRMWAREVERSION;
  for (int i = 0; i < measureProviderCount; i++)
  {
    measureProviders[i]->measure(document);
  }
  return document;
}
void Measurer::measureAndStore()
{
  JsonDocument *values = measure();
  m_publisher->store(values);
}

void Measurer::measureLoop(void *pvParameters)
{
  Measurer *measurer = (Measurer *)pvParameters;

  for (;;)
  {
    int publisherInterval = Publisher::getInstance()->getInterval();
    int measurerInterval = measurer->getMeasureInterval();
    measurer->measureAndStore();
    delay(publisherInterval > measurerInterval ? publisherInterval : measurerInterval);
  }
}

void Measurer::begin()
{
  btlog_d(TAG_MEASURER, "First measure.");
  measureAndStore();
  // Create the task, storing the handle.  Note that the passed parameter ucParameterToPass
  // must exist for the lifetime of the task, so in this case is declared static.  If it was just an
  // an automatic stack variable it might no longer exist, or at least have been corrupted, by the time
  // the new task attempts to access it.
  btlog_d(TAG_MEASURER, "Creating measurer task.");
  xTaskCreate(measureLoop, MEASURER_TASK, 8192, this, tskIDLE_PRIORITY, NULL);
}

void Measurer::readSettings(const JsonObject &source)
{
  for (int i = 0; i < measureProviderCount; i++)
  {
    measureProviders[i]->readSettings(source);
  }
}
void Measurer::writeSettings(JsonObject &target, const JsonObject &input)
{
  for (int i = 0; i < measureProviderCount; i++)
  {
    measureProviders[i]->writeSettings(target, input);
  }
}
}