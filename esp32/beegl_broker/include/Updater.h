/*
  Updater.h - Updater header file
  
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


#ifndef Updater_h
#define Updater_h


#include "Settings.h"
#include "Connection.h"
#include "Service.h"
#include "Runtime.h"

#include <Update.h>
#include <MD5Builder.h>

#define STORAGE_SIZE 30
#define INDEXHTML "index.html"
class Updater
{

public:
    Updater(Runtime *runtime, Service *service, Settings *settings, Connection *connection);
    void checkFirmware();
    bool checkDownloadFile(const char* filename);
private:
    Connection *m_connection;
    Settings *m_settings;
    Service *m_server;
    Runtime *m_runtime;
    const char* m_currentVersion;
    void webServerBind();
    void downloadFirmware(HttpClient *httpClient, char *filePath);
    String getLocalFileMd5(const char* filename);
    String getServerFileMd5(const char* filename);
    bool downloadFile(const char* hostname, const char *path, const char* filename);
    bool downloadFile(const char *filename);
};

#endif 