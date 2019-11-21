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
#define STORAGE_SIZE 10
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

  virtual void setup() {}
  virtual bool reconnect() { return true; };
  virtual bool publishMessage(JsonDocument *message) { return false;}
  virtual void update(){};
  virtual const char getProtocol() const { return 0x00;};
  virtual const char *getProtocolName() const { return "No publish";}
  virtual const int getInterval() const { return -1; }
  virtual const char getSupportedOutboundTypes() const { return 0xFF;};

protected:
  Connection *m_connection;
  Runtime *m_runtime;
  Service *m_service;
};

class Publisher : public ISettingsHandler, public IShutdownHandler
{

public:
  Publisher(Runtime *runtime, Settings *settings, Connection *connection, Service *service);
  void backlog();
  void setup();
  void update();
  bool publish();

  void store(JsonDocument *measureValue);
  int registerPublishStrategy(PublishStrategy *publishStrategy);
  int getInterval();
  static Publisher *getInstance();
  static void publishCallback();

  void readSettings(const JsonObject &source) override;
  void writeSettings(JsonObject &target, const JsonObject &input) override;
  
  void onShutdown() override;

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
  Connection *m_connection;
  Runtime *m_runtime;
  Service *m_service;
  JsonMessageSerializer m_serializer;
  std::vector<JsonDocument*> messageStorage;
  char m_protocol = 0x1;
  SemaphoreHandle_t storageSemaphore;
  int getStrategies(PublishStrategy **strategies, char outboundType);
  PublishStrategy *getSelectedStrategy();
  void webServerBind();
  //int getStoreIndex();
  //void incPublishIndex(bool lock = true);
  void popMessage();
  void pushMessage(JsonDocument* message);
  JsonDocument* backMessage();
  void storeToBacklog(JsonDocument* message);
  void storeAllToBacklog();
};
} // namespace beegl
#endif
