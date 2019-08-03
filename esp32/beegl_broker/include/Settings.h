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

#define STR_WEIGHTSENSOR "whtS"
#define STR_WEIGHT "w"
#define STR_WEIGHTUNIT "u"
#define STR_WEIGHTUNITG "g"
#define STR_TEMPSENSOR "tmpS"
#define STR_TEMP "t"
#define STR_TEMPUNIT "u"
#define STR_TEMPUNITC "C"
#define STR_HUMIDITYSENSOR "humS"
#define STR_HUMIDITY "h"
#define STR_HUMIDITYUNIT "u"
#define STR_HUMIDITYUNITPERCENT "pct"
#define STR_DEVICEID "id"
#define STR_REFRESH "refresh"
#define STR_DEVICENAME "id"
#define STR_INBOUNDMODE "inM"
#define STR_OUTBOUNDMODE "outM"
#define STR_DEVICETYPE  "type"
#define STR_WIFISETTINGS "wifiS"
#define STR_WIFISSID "ssid"
#define STR_WIFIPASSWORD "pwd"
#define STR_WIFICUSTOMIP "cip"
#define STR_WIFIIP "ip"
#define STR_WIFIGATEWAY "gw"
#define STR_WIFINETMASK "nmsk"
#define STR_SCALESETTINGS "scaleS"
#define STR_SCALEUNIT "u"
#define STR_SCALEOFFSET "off"
#define STR_SCALEFACTOR "f"
#define STR_APSSID "ssid"
#define STR_APPASSWORD "pwd"
#define STR_APSETTINGS "apS"
#define STR_APIP "ip"
#define STR_APGATEWAY "gw"
#define STR_APNETMASK "nmsk"
#define STR_MQTTSETTINGS "mqttS"
#define STR_MQTTSERVER "server"
#define STR_MQTTPORT "port"
#define STR_MQTTUSERNAME "user"
#define STR_MQTTPASSWORD "pwd"
#define STR_MQTTTOPIC "topic"
#define STR_GPRSSETTINGS "gprsS"
#define STR_GPRSAPN "apn"
#define STR_GPRSUSERNAME "user"
#define STR_GPRSPASSWORD "pwd"
#define STR_RESTARTINTERVAL "restInt"
#define STR_SCHSETTINGS "schS"
#define STR_SCHENTRY "schE"
#define STR_SCHHOURFROM "hf"
#define STR_SCHMINFROM "mf"
#define STR_SCHHOURTO "ht"
#define STR_SCHMINTO "mt"
#define STR_SCHUPDATE "upd"
#define STR_TIMEFORMAT "%04d-%02d-%02dT%02d:%02d:%02d%s"
#define STR_TIME "time"
#define STR_TIMESETTINGS "timeS"
#define STR_TIMESZONE "sZone"
#define STR_TIMEZONE "zone"
#define STR_SETTINGSURL "setUrl"
#define STR_SETTINGSUSERNAME "setUser"
#define STR_SETTINGSPASSWORD "setPwd"
#define STR_MEASURERSETTINGS "mesS"
#define STR_MEASURERWEIGHT "mesW"
#define STR_MEASURERTEMPHUM "mesTh"
#define STR_PUBLISHERSETTINGS "pubS"
#define STR_PUBLISHERPROTOCOL "proto"
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
class Settings
{

public:
    /* General parameters 
        deviceName:
        Device name. Also AP SSID.
        restartInterval:
        outboundMode:
        0x1 - WiFi - connect to AP
        0x2 - GPRS - use GSM 2G network
        inboundMode:
        0x1 - Wifi - establish AP
        0x2 - Bluetooth - register BLE service & characteristic
        deviceType:
        0x1 - Broker
        0x2 - Measure unit
        httpTimeAndSettingsPrefix:
        Server resource path prefix
        httpTimeAndSettingUsername:
        Username
        httpTimeAndSettingUPassword:
        Password


    */
   char deviceName[16] = "BEE000";
    int restartInterval = 3600;
    char outboundMode = 0x1;
    char inboundMode = 0x0;
    char deviceType = 0x1;
    char httpTimeAndSettingsPrefix[64] = "www.example.com/beegl/";
    char httpTimeAndSettingUsername[16] = "user";
    char httpTimeAndSettingPassword[16] = "password";
    uint32_t refreshInterval = 60000;
    char firmwareVersion[8];

    /* Timezone
        summerTimeZone: 
        Timezone for summer time. 
        standardTimeZone
        Std timezone - winter time
    */
    int summerTimeZone=120;
    int standardTimeZone=60;
    /* Publisher 
       protocol:
       0x1 - Publish over MQTT - Use MQTT settings
       0x2 - Publish over HTTP - use Setting resource path & credentials
    */
    char protocol =  0x1;

    /* Measurer
       measureWeight:
       To measure weight
       measureTempAndHumidity:
       To measure weight and humidity
    */
    bool measureWeight =  true;
    bool measureTempAndHumidity = true;

    /* SCALE parameters 
        scaleFactor: 
        Scale factor
        scaleOffset:
        Offset - absolute
        scaleUnit:
        Scale unit
    */
    float scaleFactor = 1;
    long scaleOffset = 0;
    char scaleUnit[5] = "g";
    /* WIFI STA parameters 
    wifiPassword:
    Wifi password
    wifiSSID:
    SSID
    wifiCustomIp:
    0 - No custom IP (DHCP)
    1 - Custom IP
    wifiGateway:
    Gateway
    wifiSubnet:
    Subnet - netmask
    */
    char wifiPassword[32] = "123456789";
    char wifiSSID[32] = "WifiSSID";
    int wifiCustomIp = 0;
    IPAddress wifiIp;
    IPAddress wifiGateway;
    IPAddress wifiSubnet;
    /* WIFI AP parameters 
    apPassword:
    Access point password
    apIP:
    Access point IP
    apGatway:
    Access point gateway
    apSubnet:
    Subnet - netmask
    */
    char apPassword[16] = "123456789";
    IPAddress apIp;
    IPAddress apGateway;
    IPAddress apSubnet;
    /* MQTT parameters 
    externalMqttServer:
    Mqtt hostname when GPRS is used. 
    internalMqttServer:
    Mqtt hostname when Wifi is used.
    mqttPort:
    Mqtt port
    mqttUsername:
    Mqtt username
    mqttPassword:
    Mqtt password
    sensorTopic
    Measurement publish topic    
    */
    char mqttServer[32] = "mqtt.example.com";
    int mqttPort = 1883;
    char mqttUsername[16] = "iot_username";
    char mqttPassword[16] = "iot_password";
    char sensorTopic[64] = "measurements";
    /* GPRS parameters 
    apn:
    APN
    apnUser:
    APN username:
    apnPassword:
    APN password
    */
    char apn[32] = "internet";
    char apnUser[16] = "mobitel";
    char apnPass[16] = "internet";
    /* Scheduler parameters 

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
    struct SchEntryType schEntries[10] = {{0, 0, 23, 59, true}};
    int schEntriesLength = 1;
    Settings();

    
    char *getSettingsHostname();
    char *getSettingsPath();
    char *getFirmwarePath();
    char *getSensorPublishPath();
    String getDateTimeString(time_t utc);
    Timezone *getTimezone();
 
    void setTimezone();
    SchEntryType getCurrentSchedulerEntry();
    char* getPath(char *url);
    char* getHostname(char *url);
private:
    

    TimeChangeRule* CEST;
    TimeChangeRule* CET;
    Timezone *m_timezone;

    
   
    char* getStrTimezoneOffset(TimeChangeRule* tcr);
};

#endif
