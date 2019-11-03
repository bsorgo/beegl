/*
  DHT22TempAndHumidityMeasureProvider.h - DHT22 temperature and humidity measure
  
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

#ifndef DHT22TempAndHumidityMeasureProvider_h
#define DHT22TempAndHumidityMeasureProvider_h

#include <DHTesp.h>
#include "Measurer.h"
#include "BeeGl.h"
#define STR_TEMPSENSOR "tmpS"
#define STR_TEMP "t"
#define STR_TEMPUNIT "u"
#define STR_TEMPUNITC "C"
#define STR_HUMIDITYSENSOR "humS"
#define STR_HUMIDITY "h"
#define STR_HUMIDITYUNIT "u"
#define STR_HUMIDITYUNITPERCENT "pct"

#define DHT_PIN 22
namespace beegl
{
class DHT22TempAndHumidityMeasureProvider : public MeasureProvider
{
public:
  DHT22TempAndHumidityMeasureProvider(Runtime *runtime, Service *service, Settings *settings);
  static DHT22TempAndHumidityMeasureProvider* createAndRegister(BeeGl *core);
  void measure(JsonDocument *values) override;
  void setup() override;

private:
  const char m_pin = DHT_PIN;
  DHTesp m_dht;
};
} // namespace beegl
#endif