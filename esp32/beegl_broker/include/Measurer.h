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

#include "DHTesp.h"
#include <HX711.h>
#include "Service.h"
#include "Settings.h"
#include "Publisher.h"

struct MeasureData
  {
      float weight;
      float temp;
      float humidity;
  };

class Measurer
{
  

public:
    Measurer(Runtime* runtime, Service* server, Settings *settings, Publisher *publisher);
    bool setup();
    long zero();
    char* measure();

private:
    HX711 *m_scale;
    Settings *m_settings;
    Publisher *m_publisher;
    Runtime *m_runtime;
    DHTesp *m_dht;
    const char SCALE_DOUT_PIN = 32;
    const char SCALE_SCK_PIN = 33;
    const int DHT_PIN = 13;
    bool scaleSetup();
    bool dhtSetup();
    char *storeMessage(MeasureData measureData);
    void webServerBind();
    Service* m_server;
};

#endif