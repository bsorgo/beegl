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
#include "Message.h"
#include <timer.h>
#include <timerManager.h>
#include "TimeManagement.h"
#define STORAGE_SIZE 30
#define BACKLOG_NVS "backlog"
#define BACKLOG_DIR "/backlog"
#define BACKLOG_DIR_PREFIX BACKLOG_DIR "/"
#define BACKLOG_EXTENSION ".json"
#ifndef MAX_BACKLOG
#define MAX_BACKLOG 200
#endif

#define STR_PUBLISHERSETTINGS "pubS"
#define STR_PUBLISHERPROTOCOL "proto"

#define TAG_PUBLISHER "PUBLISHER"

namespace beegl
{
class PublishStrategy : public ISettingsHandler
{

public:
  PublishStrategy(Runtime *runtime, Settings *settings, Connection *connection, Service *service) : ISettingsHandler(settings)
  {
    m_connection = connection;
    m_runtime = runtime;
    m_service = service;
  }

  virtual void setup() = 0;
  virtual bool reconnect() { return true; };
  virtual bool publishMessage(JsonDocument *message) = 0;
  virtual void update(){};
  virtual const char getProtocol() const = 0;
  virtual const char *getProtocolName() const = 0;
  virtual const int getInterval() const { return 60000; }
  virtual const char getSupportedOutboundTypes() const = 0;

protected:
  Connection *m_connection;
  Runtime *m_runtime;
  Service *m_service;
};

class Publisher : public ISettingsHandler
{

public:
  Publisher(Runtime *runtime, Settings *settings, Connection *connection, Service *service);
  void backlog();
  void setup();
  void update();
  bool publish();
  int store(JsonDocument *measureValue);
  int registerPublishStrategy(PublishStrategy *publishStrategy);
  int getInterval();
  static Publisher *getInstance();
  static void publishCallback();

  void readSettings(const JsonObject &source);
  void writeSettings(JsonObject &target, const JsonObject &input);

  char getProtocol()
  {
    return m_protocol;
  }

private:
  static Publisher *p_instance;
  static Timer p_publisherTimer;
  int32_t backlogCount;

  PublishStrategy *m_publishStrategies[5];
  PublishStrategy *m_selectedStrategy = nullptr;
  int publishStrategyCount = 0;
  int storageIndex = -1;
  int publishIndex = -1;
  Connection *m_connection;
  Runtime *m_runtime;
  Service *m_service;
  JsonMessageSerializer m_serializer;
  JsonDocument *messageStorage[20];
  char m_protocol = 0x1;

  int getStrategies(PublishStrategy **strategies, char outboundType);
  PublishStrategy *getSelectedStrategy();
  void webServerBind();
  int getIndex();
};
} // namespace beegl
#endif
