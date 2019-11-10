/*
  Service.h - Service header file
  
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

#ifndef Service_h
#define Service_h

#include "Log.h"
#include <ESPAsyncWebServer.h>
#include "Settings.h"

#define TAG_SERVICE "SERVICE"

namespace beegl
{
class Service : public ISettingsHandler
{

public:
  Service(Settings *settings);
  AsyncWebServer *getWebServer() const;
  void setup();



private:
  AsyncWebServer m_webserver {80};
  void webServerSetup();
};
}
#endif