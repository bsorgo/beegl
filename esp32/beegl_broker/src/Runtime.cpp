/*
  Runtime.cpp - Controls ESP32 runtime states:
  
  - Safe mode
  - Scheduler - operational times
  - Deep sleep
  
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

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 600         /* wake up interval */

#include "Runtime.h"

void check_scheduler()
{
  Runtime::getInstance()->checkOperationalTime();
}

Runtime *Runtime::p_instance = NULL;
Timer Runtime::p_schedulerTimer = Timer();

Runtime::Runtime(Service *server, Settings *settings, Connection *connection)
{
  p_instance = this;
  m_settings = settings;
  m_connection = connection;
  m_server = server;

  webServerBind();
  p_schedulerTimer.setInterval(60000);
  p_schedulerTimer.setCallback(check_scheduler);
  p_schedulerTimer.start();
}

Runtime *Runtime::getInstance()
{
  return p_instance;
}

void Runtime::update()
{
  if (!getSafeMode())
  {
    p_schedulerTimer.update();
  }
  if (getSafeMode() && millis() > 600000)
  {
    ESP.restart();
  }
}

void Runtime::initialize()
{
  blog_i("[ESP] Firmware version %s", FIRMWAREVERSION);
  m_safeMode = getSafeModeOnRestart();
}

void Runtime::webServerBind()
{
  m_server->getWebServer()->on("/info", HTTP_GET, [&](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", this->FIRMWAREVERSION);
  });
  m_server->getWebServer()->on("/rest/reboot", HTTP_POST,
                               [](AsyncWebServerRequest *request) {
                                 ESP.restart();
                                 request->send(200);
                               });
}

void Runtime::checkOperationalTime()
{
  uint32_t sleepTime = 0;
  if (millis() > 600000 && !getSafeMode())
  {

    time_t t = m_settings->getTimezone()->toLocal(now());
    // in seconds

    int time = hour(t) * 60 + minute(t);
    for (int i = 0; i < m_settings->schEntriesLength; i++)
    {

      int nextEntry = (i == m_settings->schEntriesLength - 1 ? 0 : i + 1);
      int iTimeFrom = m_settings->schEntries[i].schedulerHourFrom * 60 + m_settings->schEntries[i].schedulerMinFrom;
      int iTimeTo = m_settings->schEntries[i].schedulerHourTo * 60 + m_settings->schEntries[i].schedulerMinTo;
      int nextTimeFrom = m_settings->schEntries[nextEntry].schedulerHourFrom * 60 + m_settings->schEntries[nextEntry].schedulerMinFrom;

      blog_d("[SCHEDULER] Entries:%u, Entry: %u;%u:%u,%u:%u  Next entry: %u;%u:%u,%u:%u", m_settings->schEntriesLength, i,
             m_settings->schEntries[i].schedulerHourFrom,
             m_settings->schEntries[i].schedulerMinFrom,
             m_settings->schEntries[i].schedulerHourTo,
             m_settings->schEntries[i].schedulerMinTo,

             nextEntry,
             m_settings->schEntries[nextEntry].schedulerHourFrom,
             m_settings->schEntries[nextEntry].schedulerMinFrom,
             m_settings->schEntries[nextEntry].schedulerHourTo,
             m_settings->schEntries[nextEntry].schedulerMinTo);

      blog_d("[SCHEDULER] Calculated time: %u, iFrom:%u, iTo:%u, nextFrom:%u", time, iTimeFrom, iTimeTo, nextTimeFrom);
      if ((iTimeFrom > 0 || iTimeTo > 0) && ((i == 0 && time < iTimeFrom) || (time > iTimeTo)) &&
          ((time < nextTimeFrom) ||
           nextEntry == 0))
      {

        if (time > nextTimeFrom)
        {
          sleepTime = (24 - hour(t)) * 3600 + (0 - minute(t)) * 60;
          sleepTime += nextTimeFrom * 60;
        }
        else
        {
          sleepTime = (nextTimeFrom - time) * 60;
        }
        blog_d("[SCHEDULER] Sleep time: %u seconds\n\r", sleepTime);
        if (sleepTime)
        {
          break;
        }
      }
    }
  }

  if (sleepTime)
  {
    deepSleep(sleepTime);
  }
}

void Runtime::deepSleep(uint32_t timeToSleep)
{
  uint64_t timeToSleepuSeconds = timeToSleep * uS_TO_S_FACTOR;
  blog_i("[ESP32] Deep sleep for %u s", timeToSleep);
  m_connection->shutdown();
  Serial.flush();
  delay(1000);
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_sleep_enable_timer_wakeup(timeToSleepuSeconds);
  esp_deep_sleep_start();
}

void Runtime::deepSleep()
{
  deepSleep(TIME_TO_SLEEP);
}

void Runtime::printWakeupReason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    blog_i("[ESP32] Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    blog_i("[ESP32] Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    blog_i("[ESP32] Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    blog_i("[ESP32] Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    blog_i("[ESP32] Wakeup caused by ULP program");
    break;
  default:
    blog_i("[ESP32] Wakeup was not caused by deep sleep. Reason: %u", wakeup_reason);
    break;
  }
}

int8_t Runtime::getSafeModeOnRestart()
{
  return NVS.getInt("sfres");
}

void Runtime::setSafeModeOnRestart(int8_t safeModeOnRestart)
{
  NVS.setInt("sfres", safeModeOnRestart);
}

int8_t Runtime::getSafeMode()
{
  return m_safeMode;
}

void Runtime::setSafeMode(int8_t safeMode)
{
  m_safeMode = safeMode;
}