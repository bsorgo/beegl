/*
  Settings.cpp - settings & time management:

  - Settings SPIFFS read/write
  - Central server read/write
  - Time functions
  - Other setting | time related functions
  
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

#include "Settings.h"

Settings::Settings()
{
    apIp = IPAddress(192, 168, 4, 1);
    apGateway = IPAddress(0, 0, 0, 0);
    apSubnet = IPAddress(255, 255, 255, 0);

    wifiIp = IPAddress(192, 168, 0, 155);
    wifiGateway = IPAddress(192, 168, 0, 1);
    wifiSubnet = IPAddress(255, 255, 255, 0);

    setTimezone();
}

char *Settings::getSettingsHostname()
{
    char *hostname = getHostname(httpTimeAndSettingsPrefix);
    return hostname;
}
char *Settings::getSettingsPath()
{

    char *path = getPath(httpTimeAndSettingsPrefix);
    strcat(path, "v1/devices/");
    strcat(path, deviceName);

    return path;
}

char *Settings::getSensorPublishPath()
{

    char *path = getPath(httpTimeAndSettingsPrefix);
    strcat(path, "v1/measurements");
    return path;
}

char *Settings::getFirmwarePath()
{

    char *path = getPath(httpTimeAndSettingsPrefix);
    strcat(path, "fws/");
    return path;
}

String Settings::getDateTimeString(time_t utc)
{
    char *buf = (char *)malloc(32);
    TimeChangeRule *tcr;
    time_t t = m_timezone->toLocal(utc, &tcr);
    char *abbrev = getStrTimezoneOffset(tcr);
    sprintf(buf, STR_TIMEFORMAT, year(t), month(t), day(t), hour(t), minute(t), second(t), abbrev);
    free(abbrev);
    String ret = String(buf);
    free(buf);
    return ret;
}

char *Settings::getStrTimezoneOffset(TimeChangeRule *tcr)
{
    int hours = abs(tcr->offset) / 60;
    int minutes = abs(tcr->offset) % 60;
    char *abbrev = (char *)malloc(7);
    sprintf(abbrev, "%s%02d:%02d", (tcr->offset > 0 ? "+" : "-"), hours, minutes);
    return abbrev;
}

Timezone *Settings::getTimezone()
{
    return m_timezone;
}

void Settings::setTimezone()
{

    free(CEST);
    free(CET);
    CET = (TimeChangeRule *)malloc(sizeof(struct TimeChangeRule));
    CEST = (TimeChangeRule *)malloc(sizeof(struct TimeChangeRule));
    *CET = (TimeChangeRule){"", Last, Sun, Oct, 3, 480};
    *CEST = (TimeChangeRule){"", Last, Sun, Mar, 2, 540};
    CET->offset = standardTimeZone;
    CEST->offset = summerTimeZone;

    m_timezone = new Timezone(*CEST, *CET);
}

SchEntryType Settings::getCurrentSchedulerEntry()
{
    time_t t = getTimezone()->toLocal(now());
    int time = hour(t) * 60 + minute(t) + 1;
    for (int i = 0; i < schEntriesLength; i++)
    {
        int iTimeFrom = schEntries[i].schedulerHourFrom * 60 + schEntries[i].schedulerMinFrom;
        int iTimeTo = schEntries[i].schedulerHourTo * 60 + schEntries[i].schedulerMinTo;
        if (iTimeFrom <= time && iTimeTo > time)
        {
            blog_d("[SCHEDULER] Current entry: %u;%u:%u,%u:%u;%d",
                  i,
                  schEntries[i].schedulerHourFrom,
                  schEntries[i].schedulerMinFrom,
                  schEntries[i].schedulerHourTo,
                  schEntries[i].schedulerMinTo,
                  schEntries[i].updateFromServer);
            return schEntries[i];
        }
    }
    SchEntryType entry = {0, 0, 0, 0, false};
    blog_d("[SCHEDULER] Returning default entry: -1;0:0;0:0;false");
    return entry;
}

/**
 * Provides hostname fraction of url
 */
char *Settings::getHostname(char *url)
{
    char *hostname = (char *)malloc(sizeof(char) * 32);
    int firstDelimiter = strcspn(url, "/");
    memcpy(hostname, url, firstDelimiter);
    hostname[firstDelimiter] = '\0';
    return hostname;
}
/**
 * Provides path fraction of url
 */
char *Settings::getPath(char *url)
{
    char *path = (char *)malloc(sizeof(char) * 128);
    int firstDelimiter = strcspn(url, "/");
    strncpy(path, url + firstDelimiter, strlen(url) - firstDelimiter + 1);
    return path;
}
