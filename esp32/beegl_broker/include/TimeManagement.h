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

#define STR_TIMESOURCE "tSrc"
#define STR_TIMESETTINGS "timeS"
#define STR_TIMESZONE "sZone"
#define STR_TIMEZONE "zone"

#define STR_TIMEFORMAT "%04d-%02d-%02dT%02d:%02d:%02d%s"
namespace beegl
{
class TimeProviderStrategy : public ISettingsHandler
{

public:
    TimeProviderStrategy(Settings *settings, Connection *connection);
    virtual time_t getUTCTime() { return now(); }
    virtual bool syncTime() { return false; };
    virtual bool syncTimeFrom(TimeProviderStrategy *sourceStrategy) { return false; };
    virtual void setUTCTime(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t days, uint8_t months, uint8_t years) { setTime(hours, minutes, seconds, days, months, years); };
    virtual void setUTCTime(time_t time) { setTime(time); };
    virtual const char getType() { return 0x01; };
    virtual const char *getName() { return "No time"; };
    virtual const char getSupportedConnectionOutboundTypes() { return 0xFF; };
    virtual const char getSupportedSyncProviders() { return 0x00; };
    virtual bool isAbsoluteTime() { return false; }

protected:
    Connection *m_connection;
    bool synced = false;
};

class TimeManagement : public ISettingsHandler
{
public:
    TimeManagement(Service *service, Settings *settings, Connection *connection);
    int registerTimeProviderStrategy(TimeProviderStrategy *source);
    time_t getUTCTime();
    time_t getLocalTime();
    bool syncTime(char sourceType = 0x00);
    int getTimeProviderStrategies(TimeProviderStrategy **providers, char outboundType);
    TimeProviderStrategy *getSelectedTimeProviderStrategy();
    bool setup();
    static TimeManagement *getInstance();
    bool isAbsoluteTime();

    String getDateTimeString(time_t utc);
    Timezone *getTimezone();

    void setTimezone();

    void readSettings(const JsonObject &source) override;
    void writeSettings(JsonObject &target, const JsonObject &input) override;

    char getTimeSource()
    {
        return m_timeSource;
    }

    int getSummerTimezone()
    {
        return m_summerTimeZone;
    }
    int getStandardTimezone()
    {
        return m_standardTimeZone;
    }

private:
    static TimeManagement *p_instance;

    TimeChangeRule *CEST;
    TimeChangeRule *CET;
    Timezone *m_timezone;

    Service *m_service;
    Connection *m_connection;

    TimeProviderStrategy *m_selectedProvider = nullptr;
    TimeProviderStrategy *m_providers[5];
    int providerCount = 0;

    char m_timeSource = 0x1;

    /* Timezone
        summerTimeZone: 
        Timezone for summer time. 
        standardTimeZone
        Std timezone - winter time
    */
    int m_summerTimeZone = 120;
    int m_standardTimeZone = 60;

    char *getStrTimezoneOffset(TimeChangeRule *tcr);
    void webServerBind();
};
} // namespace beegl
#endif