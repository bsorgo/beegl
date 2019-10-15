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
#include <timer.h>
#include <timerManager.h>
#include "TimeManagement.h"
#include <PubSubClient.h>

#define STORAGE_SIZE 30
#define BACKLOG_NVS "backlog"
#define BACKLOG_DIR "/backlog"
#define BACKLOG_DIR_PREFIX BACKLOG_DIR "/"
#define BACKLOG_EXTENSION ".json"
#ifndef MAX_BACKLOG
#define MAX_BACKLOG 200
#endif

class PublishStrategy
{
  
public:
  PublishStrategy(Runtime *runtime, Settings *settings, Connection *outboundConnection, Service *service);

  virtual void setup(){};
  virtual bool reconnect() { return true; };
  virtual bool publishMessage(const char *message) { return true; };
  virtual void update(){};
  virtual const char getProtocol() { return 0x0; }
  virtual const char *getProtocolName() { return {0x00}; };
  virtual int getInterval() { return 60000; }
  virtual const char getSupportedOutboundTypes() { return {0x00}; }

protected:
  Connection *m_connection;
  Settings *m_settings;
  Runtime *m_runtime;
  Service *m_service;
};

class Publisher
{

public:
  Publisher(Runtime *runtime, Settings *settings, Connection *outboundConnection, Service *service);
  virtual void setup();
  virtual void update();
  bool publish();
  int storeMessage(const char* buffer);
  void getMessage(char* buffer, const int index);
  void addPublishStrategy(PublishStrategy *publishStrategy);
  int getInterval();
  static Publisher *getInstance();
  static void publishCallback();

private:
  int getStrategies(PublishStrategy **strategies, char outboundType);
  static Publisher *p_instance;
  static Timer p_publisherTimer;
  int32_t backlogCount;

  PublishStrategy *m_publishStrategies[5];
  PublishStrategy *m_selectedStrategy = nullptr;
  int publishStrategyCount = 0;
  int storageIndex = -1;
  int publishIndex = -1;
  Connection *m_connection;
  Settings *m_settings;
  Runtime *m_runtime;
  Service *m_service;
  char messageStorage[STORAGE_SIZE][350];
  PublishStrategy *getSelectedStrategy();
  void webServerBind();
  int getIndex();
};

#endif