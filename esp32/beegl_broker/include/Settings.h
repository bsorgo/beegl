/*
  Settings.h- settings header file
  
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

#ifndef BeescaleSettings_h
#define BeescaleSettings_h

#define CONFIG_BUFFER 2600

#define STR_DEVICEID "id"
#define STR_REFRESH "refresh"
#define STR_DEVICENAME "id"

#define STR_DEVICETYPE "type"

#define STR_RESTARTINTERVAL "restInt"

#define STR_TIME "time"
#define STR_EPOCHTIME "etime"
#define STR_DIFFEPOCHTIME "detime"

#define STR_MEASURERSETTINGS "mesS"
#define STR_MEASURERWEIGHT "mesW"
#define STR_MEASURERTEMPHUM "mesTh"

#define STR_VER "ver"
#define STR_FW_PREFIX "fwUrl"
#define CONFIG_JSON "/config.json"
#define CONFIG_JSON_DEFAULT "/config_default.json"
#define CONFIG_JSON_LAST_GOOD "/config_last_good.json"
#define CONFIG_JSON_BACKUP "/config_backup.json"
#define CONFIG_JSON_TEMP "/config_temp.json"

#include "Log.h"
#include <Timezone.h>
#include <WiFi.h>

#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#define TAG_SETTINGS "SETTINGS"
#define TAG_DEVICE "DEVICE"
namespace beegl
{
struct SchEntryType
{
    int schedulerHourFrom;
    int schedulerMinFrom;
    int schedulerHourTo;
    int schedulerMinTo;
    bool updateFromServer;
};

static const char CONFIGJSON[] = CONFIG_JSON;
static const char CONFIGJSONLASTGOOD[] = CONFIG_JSON_LAST_GOOD;
static const char CONFIGJSONTEMP[] = CONFIG_JSON_TEMP;
static const char CONFIGJSONBACKUP[] = CONFIG_JSON_BACKUP;
static const char CONFIGJSONDEFAULT[] = CONFIG_JSON_DEFAULT;

extern const uint8_t index_html_start[] asm("_binary_src_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_src_index_html_end");

class Settings;

class ISettingsHandler
{
public:
    ISettingsHandler(Settings *settings);
    virtual void readSettings(const JsonObject &source){};
    virtual void writeSettings(JsonObject &target, const JsonObject &input){};

protected:
    Settings *m_settings;
};

class IShutdownHandler
{
  public:
  virtual void onShutdown() = 0;
};


class Settings
{

public:
    char deviceName[17] = "BEE000";
    int restartInterval = 3600;
    char deviceType = 0x1;
    char httpTimeAndSettingsPrefix[65] = "www.example.com/beegl/";
    char httpTimeAndSettingUsername[17] = "user";
    char httpTimeAndSettingPassword[17] = "password";
    uint32_t refreshInterval = 60000;
    char firmwareVersion[9];

    Settings();

    void getSettingsHostname(char* buffer);
    void getSettingsPath(char* buffer);

    static void getPath(char* buffer,const char *url);
    static void getHostname(char* buffer,const char *url);

    void registerSettingsHandler(ISettingsHandler *handler);

    ISettingsHandler *settingsHandlers[50];
    int settingHandlerCount = 0;

    void readSettings(const JsonObject &source);
    void writeSettings(JsonObject &target, const JsonObject &input);

    static void merge(JsonObject &dest, const JsonObject &src);
};
} // namespace beegl
#endif
