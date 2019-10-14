/*
  TimeManagement.h - Time management
  
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

#ifndef TimeManagement_h
#define TimeManagement_h

#include "Settings.h"
#include "Connection.h"
#include "Service.h"

class TimeProviderStrategy
{

public:
    TimeProviderStrategy(Settings *settings, Connection *connection);
    virtual time_t getUTCTime() { return now();}
    virtual bool syncTime(){ return false;};
    virtual bool syncTimeFrom(TimeProviderStrategy *sourceStrategy) { return false;};
    virtual void setUTCTime(uint8_t hours, uint8_t minutes, uint8_t seconds,uint8_t days, uint8_t months, uint8_t years) { setTime(hours, minutes, seconds, days, months, years);};
    virtual void setUTCTime(time_t time) { setTime(time);};
    virtual const char getType() { return 0x01; };
    virtual const char *getName() { return "No time"; };
    virtual const char getSupportedConnectionOutboundTypes() { return 0xFF; };
    virtual const char getSupportedSyncProviders() { return 0x00; };
    virtual bool isAbsoluteTime() { return false;}
protected:
    Settings *m_settings;
    Connection *m_connection;
    bool synced = false;
};

class TimeManagement
{
public:
    TimeManagement(Service *service, Settings *settings, Connection *connection);
    int addTimeProviderStrategy(TimeProviderStrategy *source);
    time_t getUTCTime();
    time_t getLocalTime();
    bool syncTime(char sourceType=0x00);
    int getTimeProviderStrategies(TimeProviderStrategy **providers, char outboundType);
    TimeProviderStrategy* getSelectedTimeProviderStrategy();
    bool setup();
    static TimeManagement *getInstance();
    bool isAbsoluteTime();
private:
    static TimeManagement *p_instance;
    void webServerBind();
    Service *m_service;
    Settings *m_settings;
    Connection *m_connection;

    TimeProviderStrategy *m_selectedProvider = nullptr;
    TimeProviderStrategy *m_providers[5];
    int providerCount = 0;
};

#endif