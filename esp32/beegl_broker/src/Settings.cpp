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
namespace beegl
{
ISettingsHandler::ISettingsHandler(Settings *settings)
{
    m_settings = settings;
    m_settings->registerSettingsHandler(this);
}

Settings::Settings() {}

void Settings::merge(JsonObject &dest, const JsonObject &src)
{
    if (!src.isNull())
    {
        for (auto kvp : src)
        {
            dest[kvp.key()] = kvp.value();
        }
    }
}

void Settings::registerSettingsHandler(ISettingsHandler *handler)
{
    settingsHandlers[settingHandlerCount] = handler;
    settingHandlerCount++;
}

void Settings::readSettings(const JsonObject &source)
{
    for (int i = 0; i < settingHandlerCount; i++)
    {
        settingsHandlers[i]->readSettings(source);
    }
}
void Settings::writeSettings(JsonObject &target, const JsonObject &input)
{
    for (int i = 0; i < settingHandlerCount; i++)
    {
        settingsHandlers[i]->writeSettings(target, input);
    }
}

void Settings::getSettingsPath(char *buffer)
{

    getPath(buffer, httpTimeAndSettingsPrefix);
    strcat(buffer, "v1/devices/");
    strcat(buffer, deviceName);
}

void Settings::getSettingsHostname(char *buffer)
{
    getHostname(buffer, httpTimeAndSettingsPrefix);
}

/**
 * Provides hostname fraction of url
 */
void Settings::getHostname(char *buffer, const char *url)
{
    int firstDelimiter = strcspn(url, "/");
    memcpy(buffer, url, firstDelimiter);
    buffer[firstDelimiter] = '\0';
}
/**
 * Provides path fraction of url
 */
void Settings::getPath(char *buffer, const char *url)
{

    int firstDelimiter = strcspn(url, "/");
    strncpy(buffer, url + firstDelimiter, strlen(url) - firstDelimiter + 1);
}

} // namespace beegl
