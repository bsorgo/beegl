/*
  Runtime.h - Runtime header file
  
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

#ifndef Runtime_h
#define Runtime_h

#include "Log.h"
#include "Settings.h"
#include "Connection.h"
#include "Service.h"

#include <ArduinoNvs.h>
#include <Timer.h>

class Runtime
{

public:
    Runtime(Service* server, Settings* settings, Connection* connection);
    void checkOperationalTime();
    void update();
    void deepSleep(uint32_t timeToSleep);
    void deepSleep();
    void printWakeupReason();
    int8_t getSafeModeOnRestart();
    void setSafeModeOnRestart(int8_t safeModeOnRestart);
    int8_t getSafeMode();
    void setSafeMode(int8_t value);
    void initialize();
    static Runtime* getInstance();
    #ifdef VER
    const char* FIRMWAREVERSION = VER;
    #else
    const char* FIRMWAREVERSION = "1.5.0";
    #endif
private:
    static Runtime* p_instance;
    static Timer p_schedulerTimer;
    Settings *m_settings;
    Connection *m_connection;
    Service* m_server;
    int8_t m_safeMode;
    void webServerBind();
    
    
};

#endif