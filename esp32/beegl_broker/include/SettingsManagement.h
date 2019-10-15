/*
  Service.h - Service header file
  
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

#ifndef SettingsManagement_h
#define SettingsManagement_h

#define CONTENTTYPEJSON "application/json"
#define CONTENTTYPE "Content-Type"
#define CONTENTLENGTH "Content-Length"

#include "Log.h"
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include "Settings.h"
#include "Connection.h"
#include "Service.h"
#include "Runtime.h"
#include <MD5Builder.h>

class SettingsManagement
{

public:
    SettingsManagement(Settings *settings, Connection *connection, Service *service, Runtime *runtime);
    void setup();
    void readFromFilesystem(char *path);
    bool writeConfig(JsonObject input);
    bool writeConfig();
    bool readConfig();
    bool writeSettingsToServer();
    void syncSettings();
    void storeLastGood();

private:
    Settings *m_settings;
    Service *m_server;
    Connection *m_connection;
    Runtime *m_runtime;
    bool copyFile(const char *source, const char *destination);
    void webServerBind();
    bool readTimeAndSettings(HttpClient *httpClient, char *path);
    void merge(JsonObject dest, JsonObject src);
    bool writeSettings(HttpClient *httpClient, char *path, char *username, char *password);
    bool writeConfigToFS(const char *filename, JsonObject root);
    String getLocalFileMd5(const char *filename);
    bool readAndParseJson(const char *filename, JsonObject *root, StaticJsonDocument<CONFIG_BUFFER> *jsonBuffer);
};

#endif