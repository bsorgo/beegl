/*
LogManagmeent.cpp -  log file management:

- Provide log content over web server 
- Provide log files list
  
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

#include "LogManagement.h"
namespace beegl
{
LogManagement::LogManagement(Settings *settings, Service *service) : ISettingsHandler(settings)
{

    m_server = service;
    webServerBind();
}

void LogManagement::webServerBind()
{
    m_server->getWebServer()->serveStatic("/log/", FILESYSTEM, LOG_DIR_PREFIX);

    m_server->getWebServer()->on("/rest/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        StaticJsonDocument<1024> jsonBuffer;
        JsonObject root = jsonBuffer.to<JsonObject>();
        JsonArray logs = root.createNestedArray("logs");

        long logNumber = log_number();
        for (int i = logNumber; i > logNumber - MAX_LOG_FILES && i > 0; i--)
        {

            String filename = String(LOG_DIR_PREFIX);
            filename += i;
            filename += LOG_EXTENSION;
            log_d("[LOG MANAGEMENT] Filename: %s", filename.c_str());
            if (FILESYSTEM.exists(filename))
            {
                JsonObject log = logs.createNestedObject();
                log["filename"] = filename;
            }
        }
        serializeJson(jsonBuffer, *response);
        jsonBuffer.clear();
        request->send(response);
    });
}
} // namespace beegl