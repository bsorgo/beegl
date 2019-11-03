/*
  Measurer.h - Measurer header file
  
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

#ifndef Measurer_h
#define Measurer_h

#include "Log.h"

#include "Service.h"
#include "Settings.h"
#include "Publisher.h"

#include <ArduinoJson.h>

#define MEASURER_TASK "Measure Task"
namespace beegl
{
class MeasureProvider : public ISettingsHandler
{
public:
    MeasureProvider(Runtime *runtime, Service *service, Settings *settings) : ISettingsHandler(settings)
    {
        m_runtime = runtime;
        m_service = service;
    }
    virtual void measure(JsonDocument *values) = 0;
    virtual void setup() {}

protected:
    Runtime *m_runtime;
    Service *m_service;
};

class Measurer : public ISettingsHandler
{

public:
    Measurer(Runtime *runtime, Service *server, Settings *settings, Publisher *publisher);
    static Measurer *getInstance();
    void webServerBind();

    int registerMeasureProvider(MeasureProvider *measureProvider);
    uint32_t getMeasureInterval();
    void setup();
    JsonDocument *measure();
    void measureAndStore();
    void begin();

    void readSettings(const JsonObject &source) override;
    void writeSettings(JsonObject &target, const JsonObject &input) override;

private:
    Publisher *m_publisher;
    Runtime *m_runtime;
    JsonMessageSerializer m_serializer;
    Service *m_server;
    static void measureLoop(void *pvParameters);
    static Measurer *p_instance;
    MeasureProvider *measureProviders[8];
    int measureProviderCount = 0;
};
} // namespace beegl
#endif
