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
namespace beegl
{
SettingsManagement::SettingsManagement(Settings *settings, Connection *connection, Service *service, Runtime *runtime) : ISettingsHandler(settings)
{

    m_server = service;
    m_connection = connection;
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

    m_server->getWebServer()->on("/rest/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, CONFIGJSON, String(), false);
    });

    m_server->getWebServer()->on("/rest/settings/default", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->writeConfig();
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
                                     StaticJsonDocument<CONFIG_BUFFER> jsonBuffer;
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
                                             auto error = deserializeJson(jsonBuffer, file);

                                             file.close();
                                             if (!error)
                                             {
                                                 JsonObject root = jsonBuffer.as<JsonObject>();
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
bool SettingsManagement::writeConfig(const JsonObject &input)
{
    blog_i("[SETTINGS] Writing settings");

    StaticJsonDocument<CONFIG_BUFFER> jsonBuffer;
    JsonObject root = jsonBuffer.to<JsonObject>();

    m_settings->writeSettings(root, input);

    root[STR_DEVICENAME] = m_settings->deviceName;
    root[STR_RESTARTINTERVAL] = m_settings->restartInterval;
    root[STR_REFRESH] = m_settings->refreshInterval;
    root[STR_VER] = m_settings->firmwareVersion;
    root[STR_SETTINGSURL] = m_settings->httpTimeAndSettingsPrefix;
    root[STR_SETTINGSUSERNAME] = m_settings->httpTimeAndSettingUsername;
    root[STR_SETTINGSPASSWORD] = m_settings->httpTimeAndSettingPassword;

    Settings::merge(root, input);
    
    blog_i("[SETTINGS] Merge configuration ");
    return writeConfigToFS(CONFIGJSON, jsonBuffer);
    jsonBuffer.clear();
}
/*
Writes config to file
 */
bool SettingsManagement::writeConfig()
{
    StaticJsonDocument<100> jsonBuffer;
    JsonObject root = jsonBuffer.to<JsonObject>();
    return this->writeConfig(root);
}

bool SettingsManagement::readAndParseJson(const char *filename, StaticJsonDocument<CONFIG_BUFFER> *jsonBuffer)
{
    File file = SPIFFS.open(filename);
    if (!file)
    {
        return false;
    }
    auto error = deserializeJson(*jsonBuffer, file);
    if (error)
    {
        file.close();
        return false;
    }
    file.close();
    return true;
}

/* Reads config from file
 */
bool SettingsManagement::readConfig()
{
    StaticJsonDocument<CONFIG_BUFFER> jsonBuffer;

    if (!readAndParseJson(CONFIGJSON, &jsonBuffer))
    {
        blog_e("[SETTINGS] Failed to open config file for reading.");
        blog_i("[SETTINGS] Trying last good config.");
        if (!readAndParseJson(CONFIGJSONLASTGOOD, &jsonBuffer))
        {
            blog_e("[SETTINGS] Failed to open last good config file for reading.");
            blog_i("[SETTINGS] Trying backup config.");
            if (!readAndParseJson(CONFIGJSONBACKUP, &jsonBuffer))
            {
                blog_e("[SETTINGS] Failed to open backup config file for reading.");
                blog_i("[SETTINGS] Trying default minimalistic config.");
                if (!readAndParseJson(CONFIGJSONDEFAULT, &jsonBuffer))
                {
                    blog_e("[SETTINGS] Failed to open default config file for reading.");
                    blog_i("[SETTINGS] Creating config from defaults.");
                    writeConfig();
                    if (!readAndParseJson(CONFIGJSON, &jsonBuffer))
                    {
                        blog_e("[SETTINGS] Failed even to open config from defaults. Returning");
                        return false;
                    }
                }
            }
        }
    }
    JsonObject root = jsonBuffer.as<JsonObject>();

    m_settings->readSettings(root);

    strlcpy(m_settings->deviceName, root[STR_DEVICENAME] | m_settings->deviceName, 17);

    m_settings->deviceType = (root[STR_DEVICETYPE] ? root[STR_DEVICETYPE] : m_settings->deviceType);

    m_settings->refreshInterval = (root[STR_REFRESH] ? root[STR_REFRESH] : m_settings->refreshInterval);
    strlcpy(m_settings->firmwareVersion, root[STR_VER] | m_settings->firmwareVersion, 9);
    strlcpy(m_settings->httpTimeAndSettingsPrefix, root[STR_SETTINGSURL] | m_settings->httpTimeAndSettingsPrefix, 64);
    strlcpy(m_settings->httpTimeAndSettingUsername, root[STR_SETTINGSUSERNAME] | m_settings->httpTimeAndSettingUsername, 17);
    strlcpy(m_settings->httpTimeAndSettingPassword, root[STR_SETTINGSPASSWORD] | m_settings->httpTimeAndSettingPassword, 17);
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

void SettingsManagement::syncSettings()
{
    Client *client = m_connection->getClient();
    SchEntryType schEntry = m_runtime->getCurrentSchedulerEntry();
    if (schEntry.updateFromServer && client != nullptr && m_connection->getOutboundMode() & 0x3)
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
        client->stop();
        free(hostname);
        free(path);
    }
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
        }
    }

    if (res == 0 && responseCode == 200)
    {
        String responseBody = httpClient->responseBody();
        if (responseBody.startsWith("{"))
        {
            StaticJsonDocument<CONFIG_BUFFER> jsonBuffer;
            auto error = deserializeJson(jsonBuffer, responseBody.c_str());

            if (!error)
            {
                JsonObject root = jsonBuffer.as<JsonObject>();
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
    return true;
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

/**
 *  Writes json to SPIFFS file
 */
bool SettingsManagement::writeConfigToFS(const char *filename, const JsonDocument &doc)
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
    if (serializeJson(doc, file) == 0)
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
} // namespace beegl