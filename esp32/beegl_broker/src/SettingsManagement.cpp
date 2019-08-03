/*
  SettingsManagement.cpp - settings & time management:

  - Settings FS read/write
  - Central server read/write
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

#include "SettingsManagement.h"

SettingsManagement::SettingsManagement(Settings *settings, Connection *connection, Service *service, Runtime *runtime)
{

    m_server = service;
    m_connection = connection;
    m_settings = settings;
    m_runtime = runtime;
    webServerBind();
}

void SettingsManagement::webServerBind()
{
    m_server->getWebServer()->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", (char *)index_html_start);
    });
    m_server->getWebServer()->on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", (char *)index_html_start);
    });

    m_server->getWebServer()->on("/modem", HTTP_POST, [&](AsyncWebServerRequest *request) {
        if (request->hasParam("at", true, false))
        {
            const String value = request->getParam("at", true, false)->value();
            blog_d("[MODEM] at value: %s", value.c_str());
            m_connection->getModem()->sendAT(value.c_str());
            String response = String();
            if (m_connection->getModem()->waitResponse(3000, response, GSM_OK))
            {
                request->send(200, "text/plain", response);
            }
            else
            {
                request->send(405, "text/plain", response);
            }
        }
        else
        {
            request->send(405);
        }
    });

    m_server->getWebServer()->on("/rest/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, CONFIGJSON, String(), false);
    });

    m_server->getWebServer()->on("/rest/settings/sync", HTTP_POST,
                                 [&](AsyncWebServerRequest *request) {
                                     if (writeSettingsToServer())
                                     {
                                         request->send(200);
                                     }
                                     else
                                     {
                                         request->send(500);
                                     }
                                 });
    m_server->getWebServer()->on("/rest/settings/save", HTTP_POST,
                                 [&](AsyncWebServerRequest *request) {
                                     if (writeConfig())
                                     {
                                         request->send(200);
                                     }
                                     else
                                     {
                                         request->send(500);
                                     }
                                 });
    m_server->getWebServer()->on("/rest/settings", HTTP_POST,
                                 [](AsyncWebServerRequest *request) {},
                                 NULL,
                                 [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
                                     StaticJsonBuffer<CONFIG_BUFFER> jsonBuffer;
                                     File file;
                                     if (!index)
                                     {
                                         blog_i("[WEB] BodyStart: %u B\n", total);
                                         file = SPIFFS.open(CONFIGJSONTEMP, FILE_WRITE);
                                     }
                                     else
                                     {
                                         file = SPIFFS.open(CONFIGJSONTEMP, FILE_APPEND);
                                     }
                                     if (file)
                                     {
                                         for (size_t i = 0; i < len; i++)
                                         {
                                             file.write(data[i]);
                                         }
                                         file.close();
                                     }
                                     else
                                     {
                                         request->send(500);
                                     }
                                     if (index + len == total)
                                     {
                                         blog_i("[WEB] BodyEnd: %u B\n", total);
                                         file = SPIFFS.open(CONFIGJSONTEMP, FILE_READ);
                                         if (file)
                                         {
                                             JsonObject &root = jsonBuffer.parse(file);
                                             file.close();
                                             if (root.success())
                                             {
                                                 if (writeConfig(root))
                                                 {
                                                     blog_i("[WEB] Writing config\n");
                                                     jsonBuffer.clear();
                                                     readConfig();
                                                     request->send(200);
                                                 }
                                                 else
                                                 {
                                                     request->send(500);
                                                 }
                                             }
                                             else
                                             {
                                                 request->send(500);
                                             }
                                         }
                                         else
                                         {
                                             request->send(500);
                                         }
                                     }
                                 });
}

/*
  Writes config to file
 */
bool SettingsManagement::writeConfig(JsonObject &input)
{

    blog_i("[SETTINGS] Writing settings");

    StaticJsonBuffer<CONFIG_BUFFER> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    root[STR_OUTBOUNDMODE] = m_settings->outboundMode;
    root[STR_INBOUNDMODE] = m_settings->inboundMode;
    root[STR_DEVICENAME] = m_settings->deviceName;
    root[STR_RESTARTINTERVAL] = m_settings->restartInterval;
    root[STR_REFRESH] = m_settings->refreshInterval;
    root[STR_VER] = m_settings->firmwareVersion;
    root[STR_SETTINGSURL] = m_settings->httpTimeAndSettingsPrefix;
    root[STR_SETTINGSUSERNAME] = m_settings->httpTimeAndSettingUsername;
    root[STR_SETTINGSPASSWORD] = m_settings->httpTimeAndSettingPassword;

    merge(root, input);

    JsonObject &wifiSettings = root.createNestedObject(STR_WIFISETTINGS);
    wifiSettings[STR_WIFISSID] = m_settings->wifiSSID;
    wifiSettings[STR_WIFIPASSWORD] = m_settings->wifiPassword;
    wifiSettings[STR_WIFICUSTOMIP] = m_settings->wifiCustomIp;
    wifiSettings[STR_WIFIIP] = m_settings->wifiIp.toString();
    wifiSettings[STR_WIFIGATEWAY] = m_settings->wifiGateway.toString();
    wifiSettings[STR_WIFINETMASK] = m_settings->wifiSubnet.toString();

    merge(wifiSettings, input[STR_WIFISETTINGS]);

    JsonObject &scaleSettings = root.createNestedObject(STR_SCALESETTINGS);
    scaleSettings[STR_SCALEFACTOR] = m_settings->scaleFactor;
    scaleSettings[STR_SCALEOFFSET] = m_settings->scaleOffset;
    scaleSettings[STR_SCALEUNIT] = m_settings->scaleUnit;

    merge(scaleSettings, input[STR_SCALESETTINGS]);

    JsonObject &apSettings = root.createNestedObject(STR_APSETTINGS);
    apSettings[STR_APPASSWORD] = m_settings->apPassword;
    apSettings[STR_APIP] = m_settings->apIp.toString();
    apSettings[STR_APGATEWAY] = m_settings->apGateway.toString();
    apSettings[STR_APNETMASK] = m_settings->apSubnet.toString();

    merge(apSettings, input[STR_APSETTINGS]);

    JsonObject &mqttSettings = root.createNestedObject(STR_MQTTSETTINGS);
    mqttSettings[STR_MQTTSERVER] = m_settings->mqttServer;
    mqttSettings[STR_MQTTPORT] = m_settings->mqttPort;
    mqttSettings[STR_MQTTUSERNAME] = m_settings->mqttUsername;
    mqttSettings[STR_MQTTPASSWORD] = m_settings->mqttPassword;
    mqttSettings[STR_MQTTTOPIC] = m_settings->sensorTopic;

    merge(mqttSettings, input[STR_MQTTSETTINGS]);
    JsonObject &gprsSettings = root.createNestedObject(STR_GPRSSETTINGS);
    gprsSettings[STR_GPRSAPN] = m_settings->apn;
    gprsSettings[STR_GPRSPASSWORD] = m_settings->apnPass;
    gprsSettings[STR_GPRSUSERNAME] = m_settings->apnUser;

    merge(gprsSettings, input[STR_GPRSSETTINGS]);

    JsonObject &timeSettings = root.createNestedObject(STR_TIMESETTINGS);
    timeSettings[STR_TIMESZONE] = m_settings->summerTimeZone;
    timeSettings[STR_TIMEZONE] = m_settings->standardTimeZone;

    merge(timeSettings, input[STR_TIMESETTINGS]);

    JsonObject &measurerSettings = root.createNestedObject(STR_MEASURERSETTINGS);
    measurerSettings[STR_MEASURERWEIGHT] = m_settings->measureWeight;
    measurerSettings[STR_MEASURERTEMPHUM] = m_settings->measureTempAndHumidity;

    merge(measurerSettings, input[STR_MEASURERSETTINGS]);

    JsonObject &publisherSettings = root.createNestedObject(STR_PUBLISHERSETTINGS);
    publisherSettings[STR_PUBLISHERPROTOCOL] = m_settings->protocol;

    merge(publisherSettings, input[STR_PUBLISHERSETTINGS]);

    JsonArray &schSettings = root.createNestedArray(STR_SCHSETTINGS);
    if (input[STR_SCHSETTINGS])
    {
        m_settings->schEntriesLength = input[STR_SCHSETTINGS].size();
        for (JsonObject &schEntryInput : input[STR_SCHSETTINGS].asArray())
        {
            JsonObject &schEntry = schSettings.createNestedObject();
            merge(schEntry, schEntryInput);
        }
    }
    else
    {
        for (int i = 0; i < m_settings->schEntriesLength; i++)
        {
            JsonObject &schEntry = schSettings.createNestedObject();
            schEntry[STR_SCHHOURFROM] = m_settings->schEntries[i].schedulerHourFrom;
            schEntry[STR_SCHMINFROM] = m_settings->schEntries[i].schedulerMinFrom;
            schEntry[STR_SCHHOURTO] = m_settings->schEntries[i].schedulerHourTo;
            schEntry[STR_SCHMINTO] = m_settings->schEntries[i].schedulerMinTo;
            schEntry[STR_SCHUPDATE] = m_settings->schEntries[i].updateFromServer;
        }
    }
    blog_i("[SETTINGS] Merge configuration ");
    return writeConfigToFS(CONFIGJSON, root);
    jsonBuffer.clear();
}
/*
Writes config to file
 */
bool SettingsManagement::writeConfig()
{
    StaticJsonBuffer<100> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    return this->writeConfig(root);
}

bool SettingsManagement::readAndParseJson(const char *filename, JsonObject **root, StaticJsonBuffer<CONFIG_BUFFER> *jsonBuffer)
{
    File file = SPIFFS.open(filename);
    if (!file)
    {
        return false;
    }
    JsonObject& rootObj= jsonBuffer->parseObject(file);
    
    file.close();
    if (!rootObj.success())
    {
        return false;
    }
    *root = &rootObj;
    return true;
}

/* Reads config from file
 */
bool SettingsManagement::readConfig()
{
    StaticJsonBuffer<CONFIG_BUFFER> jsonBuffer;
    JsonObject* rootObj;
    if (!readAndParseJson(CONFIGJSON, &rootObj, &jsonBuffer))
    {
        blog_e("[SETTINGS] Failed to open config file for reading.");
        blog_i("[SETTINGS] Trying last good config.");
        if (!readAndParseJson(CONFIGJSONLASTGOOD, &rootObj, &jsonBuffer))
        {
            blog_e("[SETTINGS] Failed to open last good config file for reading.");
            blog_i("[SETTINGS] Trying backup config.");
            if (!readAndParseJson(CONFIGJSONBACKUP, &rootObj, &jsonBuffer))
            {
                blog_e("[SETTINGS] Failed to open backup config file for reading.");
                blog_i("[SETTINGS] Trying default minimalistic config.");
                if (!readAndParseJson(CONFIGJSONDEFAULT, &rootObj, &jsonBuffer))
                {
                    blog_e("[SETTINGS] Failed to open default config file for reading.");
                    blog_i("[SETTINGS] Creating config from defaults.");
                    writeConfig();
                    if (!readAndParseJson(CONFIGJSON, &rootObj, &jsonBuffer))
                    {
                        blog_e("[SETTINGS] Failed even to open config from defaults. Returning");
                        return false;
                    }
                }
            }
        }
    }
    JsonObject &root = *rootObj;

    strlcpy(m_settings->deviceName, root[STR_DEVICENAME] | m_settings->deviceName, 16);
    m_settings->inboundMode = root.get<char>(STR_INBOUNDMODE);
    m_settings->outboundMode = (root.get<char>(STR_OUTBOUNDMODE) ? root.get<char>(STR_OUTBOUNDMODE) : m_settings->outboundMode);
    m_settings->deviceType = (root.get<char>(STR_DEVICETYPE) ? root.get<char>(STR_DEVICETYPE) : m_settings->deviceType);
    m_settings->refreshInterval = (root.get<uint32_t>(STR_REFRESH) ? root.get<uint32_t>(STR_REFRESH) : m_settings->refreshInterval);
    strlcpy(m_settings->firmwareVersion, root[STR_VER] | m_settings->firmwareVersion, 8);
    strlcpy(m_settings->httpTimeAndSettingsPrefix, root[STR_SETTINGSURL] | m_settings->httpTimeAndSettingsPrefix, 64);
    strlcpy(m_settings->httpTimeAndSettingUsername, root[STR_SETTINGSUSERNAME] | m_settings->httpTimeAndSettingUsername, 16);
    strlcpy(m_settings->httpTimeAndSettingPassword, root[STR_SETTINGSPASSWORD] | m_settings->httpTimeAndSettingPassword, 16);

    // wifi settings

    JsonObject &wifiSettings = root[STR_WIFISETTINGS];
    strlcpy(m_settings->wifiSSID, wifiSettings[STR_WIFISSID] | m_settings->wifiSSID, 32);
    strlcpy(m_settings->wifiPassword, wifiSettings[STR_WIFIPASSWORD] | m_settings->wifiPassword, 32);
    m_settings->wifiCustomIp = wifiSettings.get<int>(STR_WIFICUSTOMIP) ? wifiSettings.get<int>(STR_WIFICUSTOMIP) : m_settings->wifiCustomIp;
    m_settings->wifiIp.fromString(wifiSettings[STR_WIFIIP] | "192.168.0.155");
    m_settings->wifiGateway.fromString(wifiSettings[STR_WIFIGATEWAY] | "192.168.0.1");
    m_settings->wifiSubnet.fromString(wifiSettings[STR_WIFINETMASK] | "255.255.255.0");
    // scale settings

    JsonObject &scaleSettings = root[STR_SCALESETTINGS];
    if (scaleSettings[STR_SCALEFACTOR].success())
    {
        m_settings->scaleFactor = scaleSettings.get<float>(STR_SCALEFACTOR);
    }
    m_settings->scaleOffset = scaleSettings.get<long>(STR_SCALEOFFSET) | 0;
    strlcpy(m_settings->scaleUnit, scaleSettings[STR_SCALEUNIT] | m_settings->scaleUnit, 5);

    // wifi ap settings

    JsonObject &apSettings = root[STR_APSETTINGS];
    strlcpy(m_settings->apPassword, apSettings[STR_APPASSWORD] | m_settings->apPassword, 16);
    m_settings->apIp.fromString(apSettings[STR_APIP] | "192.168.4.1");
    m_settings->apGateway.fromString(apSettings[STR_APGATEWAY] | "192.168.4.1");
    m_settings->apSubnet.fromString(apSettings[STR_APNETMASK] | "255.255.255.0");
    // mqtt settings

    JsonObject &mqttSettings = root[STR_MQTTSETTINGS];
    strlcpy(m_settings->mqttServer, mqttSettings[STR_MQTTSERVER] | m_settings->mqttServer, 32);
    m_settings->mqttPort = mqttSettings.get<int>(STR_MQTTPORT) | 1883;
    strlcpy(m_settings->mqttUsername, mqttSettings[STR_MQTTUSERNAME] | m_settings->mqttUsername, 16);
    strlcpy(m_settings->mqttPassword, mqttSettings[STR_MQTTPASSWORD] | m_settings->mqttPassword, 16);
    strlcpy(m_settings->sensorTopic, mqttSettings[STR_MQTTTOPIC] | m_settings->sensorTopic, 64);

    // gprs settings

    JsonObject &gprsSettings = root[STR_GPRSSETTINGS];
    strlcpy(m_settings->apn, gprsSettings[STR_GPRSAPN] | m_settings->apn, 32);
    strlcpy(m_settings->apnUser, gprsSettings[STR_GPRSUSERNAME] | m_settings->apnUser, 16);
    strlcpy(m_settings->apnPass, gprsSettings[STR_GPRSPASSWORD] | m_settings->apnPass, 16);

    // time settings

    JsonObject &timeSettings = root[STR_TIMESETTINGS];
    m_settings->standardTimeZone = timeSettings.get<int>(STR_TIMEZONE) ? timeSettings.get<int>(STR_TIMEZONE) : m_settings->standardTimeZone;
    m_settings->summerTimeZone = timeSettings.get<int>(STR_TIMESZONE) ? timeSettings.get<int>(STR_TIMESZONE) : m_settings->summerTimeZone;

    m_settings->setTimezone();

    // measurer settings

    JsonObject &measurerSettings = root[STR_MEASURERSETTINGS];
    m_settings->measureWeight = measurerSettings.get<bool>(STR_MEASURERWEIGHT) ? measurerSettings.get<bool>(STR_MEASURERWEIGHT) : m_settings->measureWeight;
    m_settings->measureTempAndHumidity = measurerSettings.get<bool>(STR_MEASURERTEMPHUM) ? measurerSettings.get<bool>(STR_MEASURERTEMPHUM) : m_settings->measureTempAndHumidity;

    // publisher settings

    JsonObject &publisherSettings = root[STR_PUBLISHERSETTINGS];
    m_settings->protocol = publisherSettings.get<char>(STR_PUBLISHERPROTOCOL);

    // scheduler settings
    JsonArray &schSettings = root[STR_SCHSETTINGS];
    m_settings->schEntriesLength = schSettings.size();
    int i = 0;
    for (JsonObject &schEntryInput : schSettings)
    {
        m_settings->schEntries[i].schedulerHourFrom = schEntryInput.get<int>(STR_SCHHOURFROM);
        m_settings->schEntries[i].schedulerMinFrom = schEntryInput.get<int>(STR_SCHMINFROM);
        m_settings->schEntries[i].schedulerHourTo = schEntryInput.get<int>(STR_SCHHOURTO);
        m_settings->schEntries[i].schedulerMinTo = schEntryInput.get<int>(STR_SCHMINTO);
        m_settings->schEntries[i].updateFromServer = schEntryInput.get<bool>(STR_SCHUPDATE);
        i++;
    }
    jsonBuffer.clear();
    return true;
}

bool SettingsManagement::writeSettingsToServer()
{
    char *hostname = m_settings->getSettingsHostname();
    char *path = m_settings->getSettingsPath();

    blog_d("[SETTINGS] Hostname: %s", hostname);
    blog_d("[SETTINGS] Path: %s", path);
    blog_d("[SETTINGS] Username: %s, password: %s", m_settings->httpTimeAndSettingUsername, m_settings->httpTimeAndSettingPassword);

    HttpClient httpClient = HttpClient(*m_connection->getClient(), hostname, 80);
    bool res = writeSettings(&httpClient, path, m_settings->httpTimeAndSettingUsername, m_settings->httpTimeAndSettingPassword);
    free(hostname);
    free(path);
    return res;
}

void SettingsManagement::syncTimeAndSettings()
{

    blog_d("[SETTINGS] Time and setting prefix: %s", m_settings->httpTimeAndSettingsPrefix);

    char *hostname = m_settings->getSettingsHostname();
    char *path = m_settings->getSettingsPath();

    blog_d("[SETTINGS] Hostname: %s", hostname);
    blog_d("[SETTINGS] Path: %s", path);
    blog_d("[SETTINGS] Username: %s, password: %s", m_settings->httpTimeAndSettingUsername, m_settings->httpTimeAndSettingPassword);
    m_connection->checkConnect();
    HttpClient httpClient = HttpClient(*m_connection->getClient(), hostname, 80);
    this->readTimeAndSettings(&httpClient, path);
    free(hostname);
    free(path);
}

bool SettingsManagement::readTimeAndSettings(HttpClient *httpClient, char *path)
{

    httpClient->connectionKeepAlive();
    httpClient->beginRequest();
    int res = httpClient->get(path);
    httpClient->sendBasicAuth(m_settings->httpTimeAndSettingUsername, m_settings->httpTimeAndSettingPassword);
    httpClient->endRequest();
    int responseCode = httpClient->responseStatusCode();
    blog_d("[SETTINGS] Response code from server: %u", responseCode);
    if (res == 0)
    {
        while (httpClient->headerAvailable())
        {
            String headerName = httpClient->readHeaderName();
            if (headerName.equals("Date") || headerName.equals("date"))
            {
                String dateStr = httpClient->readHeaderValue();

                blog_i("[SETTINGS] Header date string: %s", dateStr.c_str());
                char p[32];

                char *token;

                dateStr.toCharArray(p, 32, 0);

                // day of week
                token = strtok(p, " ");

                // day
                token = strtok(NULL, " ");
                int days = atoi(token);
                // month
                token = strtok(NULL, " ");

                int months = getMonthFromString(token);
                // year
                token = strtok(NULL, " ");

                int years = atoi(token);
                // hour
                token = strtok(NULL, " ");

                token = strtok(token, ":");

                int hours = atoi(token);
                // minute
                token = strtok(NULL, ":");

                int minutes = atoi(token);
                // seconds
                token = strtok(NULL, ": ");
                int seconds = atoi(token);
                blog_d("[SETTINGS] Time fractions: %04d-%02d-%02dT%02d:%02d:%02d.000Z", years, months, days, hours, minutes, seconds);

                setTime(hours, minutes, seconds, days, months, years);
            }
        }
    }
    SchEntryType schEntry = m_settings->getCurrentSchedulerEntry();
    if (schEntry.updateFromServer && res == 0 && responseCode == 200)
    {
        String responseBody = httpClient->responseBody();
        if (responseBody.startsWith("{"))
        {
            StaticJsonBuffer<CONFIG_BUFFER> jsonBuffer;
            JsonObject &root = jsonBuffer.parseObject(responseBody.c_str());

            if (root.success())
            {
                String preMD5 = getLocalFileMd5(CONFIG_JSON);

                writeConfig(root);
                String postMD5 = getLocalFileMd5(CONFIG_JSON);
                if (preMD5.compareTo(postMD5) != 0)
                {
                    m_runtime->setSafeModeOnRestart(0);
                    delay(100);
                    ESP.restart();
                }
            }
        }
    }
    httpClient->stop();
}

bool SettingsManagement::copyFile(const char *source, const char *destination)
{
    File sourceFile = SPIFFS.open(source, FILE_READ);
    if (!sourceFile)
    {
        return false;
    }
    File destinationFile = SPIFFS.open(destination, FILE_WRITE);
    while (sourceFile.available())
    {
        destinationFile.write(sourceFile.read());
    }
    sourceFile.close();
    destinationFile.close();

    return true;
}

void SettingsManagement::storeLastGood()
{
    copyFile(CONFIGJSON, CONFIGJSONLASTGOOD);
}

/**
 * Merges json objects from src -> dest
 */
void SettingsManagement::merge(JsonObject &dest, JsonObject &src)
{
    if (src.success())
    {
        for (auto kvp : src)
        {
            dest[kvp.key] = kvp.value;
        }
    }
}
/**
 *  Writes json to SPIFFS file
 */
bool SettingsManagement::writeConfigToFS(const char *filename, JsonObject &root)
{

    SPIFFS.rename(CONFIGJSON, CONFIGJSONBACKUP);
    blog_i("[SPIFFS] Writing file %s", filename);
    File file = SPIFFS.open(filename, FILE_WRITE);
    if (!file)
    {
        blog_e("[SPIFFS] Failed to create file %s ", filename);
        Serial.println(filename);
        return false;
    }
    if (root.printTo(file) == 0)
    {
        blog_e("[SPIFFS] Failed to write to file %s", filename);
        return false;
    }
    file.close();
    return true;
}

/**
 * Writes settings through HTTP client
 */
bool SettingsManagement::writeSettings(HttpClient *httpClient, char *path, char *username, char *password)
{
    File sourceFile = SPIFFS.open(CONFIGJSON, FILE_READ);
    if (!sourceFile)
    {
        return false;
    }

    String settingsStr = sourceFile.readString();
    sourceFile.close();
    httpClient->connectionKeepAlive();
    httpClient->beginRequest();
    httpClient->post(path);
    httpClient->sendBasicAuth(m_settings->httpTimeAndSettingUsername, m_settings->httpTimeAndSettingPassword);
    httpClient->sendHeader(CONTENTLENGTH, settingsStr.length());
    httpClient->sendHeader(CONTENTTYPE, CONTENTTYPEJSON);
    httpClient->beginBody();
    httpClient->print(settingsStr);
    httpClient->endRequest();

    int responseCode = httpClient->responseStatusCode();
    httpClient->stop();
    if (responseCode != 200)
    {
        blog_e("[SETTINGS] Error. Response code from server: %u", responseCode);
        return false;
    }
    return true;
}

int SettingsManagement::getMonthFromString(char *s)
{
    if (strcmp(s, "Jan") == 0)
    {
        return 1;
    }
    if (strcmp(s, "Feb") == 0)
    {
        return 2;
    }
    if (strcmp(s, "Mar") == 0)
    {
        return 3;
    }
    if (strcmp(s, "Apr") == 0)
    {
        return 4;
    }
    if (strcmp(s, "May") == 0)
    {
        return 5;
    }
    if (strcmp(s, "Jun") == 0)
    {
        return 6;
    }
    if (strcmp(s, "Jul") == 0)
    {
        return 7;
    }
    if (strcmp(s, "Aug") == 0)
    {
        return 8;
    }
    if (strcmp(s, "Sep") == 0)
    {
        return 9;
    }
    if (strcmp(s, "Oct") == 0)
    {
        return 10;
    }
    if (strcmp(s, "Nov") == 0)
    {
        return 11;
    }
    if (strcmp(s, "Dec") == 0)
    {
        return 12;
    }

    return -1;
}

String SettingsManagement::getLocalFileMd5(const char *path)
{
    File file = SPIFFS.open(path, FILE_READ);
    if (!file)
    {
        blog_e("[SETTINGS] Error. File %s not found.", path);
        ;
        return "0";
    }
    MD5Builder md5;
    md5.begin();
    md5.addStream(file, 50000);
    md5.calculate();
    String md5str = md5.toString();
    blog_i("[SETTINGS] Local file:%s MD5:%s", path, md5str.c_str());
    file.close();
    return md5str;
}