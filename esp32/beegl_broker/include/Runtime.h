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
#include "Service.h"
#include "TimeManagement.h"

#include <ArduinoNvs.h>
#include <timer.h>
#include <timerManager.h>

#define STR_SCHSETTINGS "schS"
#define STR_SCHENTRY "schE"
#define STR_SCHHOURFROM "hf"
#define STR_SCHMINFROM "mf"
#define STR_SCHHOURTO "ht"
#define STR_SCHMINTO "mt"
#define STR_SCHUPDATE "upd"

#define TAG_RUNTIME "RUNTIME"
#define TAG_SCHEDULER "SCHEDULER"

namespace beegl
{
class Runtime : public ISettingsHandler
{


public:
  Runtime(Service *server, Settings *settings);
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
  static Runtime *getInstance();
  static void checkScheduler();

  SchEntryType getCurrentSchedulerEntry();

  void readSettings(const JsonObject &source) override;
  void writeSettings(JsonObject &target, const JsonObject &input) override;

  void getInfo(JsonObject &target) override;

  void registerShutdownHandler(IShutdownHandler* handler);
  void executeShutdownHandlers();
  const struct SchEntryType *getSchEntries()
  {
    return m_schEntries;
  }

#ifdef VER
  const char *FIRMWAREVERSION = VER;
#else
  const char *FIRMWAREVERSION = "1.0.0";
#endif

private:
  static Timer p_schedulerTimer;
  static Runtime *p_instance;
  Service *m_server;
  int8_t m_safeMode;
  /*
  Scheduler entries:
        schedulerHourFrom:
        Operational hour from
        schedulerMinFrom:
        Operational minute from
        schedulerHourTo:
        Operational hour to
        schedulerMinTo:
        Operational minute to
        updateFromServer:
        Perform upate from server (firmware, settings, resources) in this time interval
    */
  struct SchEntryType m_schEntries[10] = {{0, 0, 23, 59, false}};
  IShutdownHandler* shutdownHandlers[10];
  int shutdownHandlerSize;
  int schEntriesLength = 1;

  void webServerBind();
};
} // namespace beegl
#endif