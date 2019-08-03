/*
  Publisher.h - Publisher header file
  
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

#ifndef Publisher_h
#define Publisher_h


#include "Settings.h"
#include "Connection.h"
#include "Runtime.h"
#include "Service.h"

#include <PubSubClient.h>

#define STORAGE_SIZE 30
#define BACKLOG_NVS "backlog"
#define BACKLOG_DIR  "/backlog"
#define BACKLOG_DIR_PREFIX  BACKLOG_DIR "/"
#define BACKLOG_EXTENSION ".json"
#ifndef MAX_BACKLOG 
  #define MAX_BACKLOG 200
#endif

class Publisher
{

public:
  Publisher(Runtime *runtime, Settings *settings, Connection *outboundConnection, Service *service);
  void setSettings(Settings *settings);
  virtual void setup();
  bool publish();
  char *storeMessage(JsonObject &jsonObj);

private:
  int32_t backlogCount;
  void webServerBind();
  
protected:
  Connection *m_connection;
  Settings *m_settings;
  Runtime *m_runtime;
  Service *m_service;
  char messageStorage[STORAGE_SIZE][350];
  int storageIndex = -1;
  int publishIndex = -1;
  
  int getIndex();
  virtual bool reconnect();
  virtual bool publishMessage(const char *message);

};
#endif