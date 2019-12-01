/*
  Publisher.cpp - Provides Temporary storage and publishes messages to central server
  
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

#include "Publisher.h"

namespace beegl
{
void Publisher::publishCallback()
{
  Publisher::getInstance()->publish();
}

Timer Publisher::p_publisherTimer;
Publisher *Publisher::p_instance = NULL;

Publisher::Publisher(Runtime *runtime, Settings *settings, Connection *connection, Service *service) : ISettingsHandler(settings)
{
  Publisher::p_instance = this;
  m_connection = connection;
  m_runtime = runtime;
  m_service = service;

  webServerBind();
  backlog();
  storageSemaphore = xSemaphoreCreateMutex();

  m_publishStrategies[0] = new PublishStrategy(runtime, settings, connection, service);
  publishStrategyCount++;
}
void Publisher::backlog()
{
  if (!FILESYSTEM.exists(BACKLOG_DIR))
  {
    FILESYSTEM.mkdir(BACKLOG_DIR);
  }
}
Publisher *Publisher::getInstance()
{
  return p_instance;
}

void Publisher::setup()
{
  Publisher::p_publisherTimer.setCallback(Publisher::publishCallback);
  getSelectedStrategy();
}

void Publisher::update()
{
  if (!m_runtime->getSafeMode())
  {
    PublishStrategy *strategy = getSelectedStrategy();
    if (strategy == nullptr)
    {
      return;
    }
    strategy->update();
    Publisher::p_publisherTimer.update();
  }
}

int Publisher::registerPublishStrategy(PublishStrategy *publishStrategy)
{
  if (publishStrategy != nullptr)
  {
    btlog_d(TAG_PUBLISHER, "Add publish strategy: %u", publishStrategy->getProtocol());
    m_publishStrategies[publishStrategyCount] = publishStrategy;
    publishStrategyCount++;
    return publishStrategyCount;
  }
  return -1;
}

PublishStrategy *Publisher::getSelectedStrategy()
{
  if (m_selectedStrategy == nullptr || m_selectedStrategy->getProtocol() != m_protocol)
  {
    for (int i = 0; i < publishStrategyCount; i++)
    {
      if (m_publishStrategies[i]->getProtocol() == m_protocol && m_publishStrategies[i]->getSupportedOutboundTypes() & m_connection->getOutboundMode())
      {
        btlog_i(TAG_PUBLISHER, "Selected publish strategy: %u with publish interval: %lu", m_publishStrategies[i]->getProtocol(), m_publishStrategies[i]->getInterval());
        m_selectedStrategy = m_publishStrategies[i];
        break;
      }
    }
    if (m_selectedStrategy == nullptr)
    {
      m_selectedStrategy = m_publishStrategies[0];
    }
    m_selectedStrategy->setup();
    if (m_selectedStrategy->getInterval() != -1)
    {
      p_publisherTimer.stop();
      p_publisherTimer.setInterval(m_selectedStrategy->getInterval());
      p_publisherTimer.start();
    }
    else
    {
      p_publisherTimer.stop();
    }
  }
  return m_selectedStrategy;
}

int Publisher::getStrategies(PublishStrategy **strategies, char outboundType)
{
  int j = 1;
  strategies[0] = m_publishStrategies[0];
  for (int i = 1; i < publishStrategyCount; i++)
  {
    PublishStrategy *strategy = m_publishStrategies[i];
    if (strategy->getSupportedOutboundTypes() & outboundType)
    {
      strategies[j] = strategy;
      j++;
    }
  }
  return j;
}

int Publisher::getInterval()
{
  PublishStrategy *strategy = getSelectedStrategy();
  if (strategy != nullptr)
  {
    return strategy->getInterval();
  }
  else
  {
    return 60000;
  }
}

void Publisher::webServerBind()
{
  m_service->getWebServer()->serveStatic("/backlog/", FILESYSTEM, BACKLOG_DIR_PREFIX);

  m_service->getWebServer()->on("/rest/backlogs", HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    StaticJsonDocument<128> jsonBuffer;
    JsonObject root = jsonBuffer.to<JsonObject>();
    JsonObject backlog = root.createNestedObject("backlog");
    backlog["count"] = backlogCount;

    serializeJson(jsonBuffer, *response);
    jsonBuffer.clear();
    request->send(response);
  });

  m_service->getWebServer()->on("/rest/protocols", HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    const String outboundTypeStr = request->getParam(STR_OUTBOUNDMODE, false)->value();

    char outboundType = (char)outboundTypeStr.toInt();
    StaticJsonDocument<256> jsonBuffer;
    JsonObject root = jsonBuffer.to<JsonObject>();
    JsonArray array = root.createNestedArray("protocols");

    PublishStrategy *strategies[5];
    int count = this->getStrategies(strategies, outboundType);
    for (int i = 0; i < count; i++)
    {
      JsonObject proto = array.createNestedObject();
      proto[STR_PUBLISHERPROTOCOL] = (int)strategies[i]->getProtocol();
      proto["name"] = strategies[i]->getProtocolName();
    }
    serializeJson(root, *response);
    jsonBuffer.clear();
    request->send(response);
  });
}

void Publisher::store(JsonDocument *measureValue)
{
  btlog_d(TAG_PUBLISHER, "Begin store message");
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
  char message[MESSAGE_SIZE];
  m_serializer.serialize(measureValue, message);
  btlog_d(TAG_PUBLISHER, "Persist message: %s", message);
#endif
  pushMessage(measureValue);
}
bool Publisher::publish()
{
  btlog_d(TAG_DEVICE, "Free heap: %u", ESP.getFreeHeap());

  if (!m_runtime->getSafeMode())
  {
    PublishStrategy *strategy = getSelectedStrategy();
    if (strategy == nullptr)
    {
      return false;
    }
    if (backMessage() != nullptr || backlogCount > 0)
    {
      m_connection->resume();
      m_connection->checkConnect();
      bool connected = false;
      int retries = 0;
      while (!connected)
      {
        connected = strategy->reconnect();
        retries++;
        if (!connected)
        {
          if (retries % 2 == 0)
          {
            m_connection->checkConnect();
          }
          if (retries % 5 == 0)
          {
            break;
          }
        }
      }
      backlogCount = NVS.getInt(BACKLOG_NVS);
      btlog_d(TAG_PUBLISHER, "Backlog count: %u", backlogCount);
      while (connected && TimeManagement::getInstance()->isAbsoluteTime() && backlogCount > 0)
      {
        String backlogFilename = String(BACKLOG_DIR_PREFIX);
        backlogFilename += backlogCount;
        backlogFilename += BACKLOG_EXTENSION;
        
        File backlogFile = FILESYSTEM.open(backlogFilename, FILE_READ);
        if (backlogFile)
        {
          btlog_e(TAG_PUBLISHER, "Opening backlog file: %s", backlogFilename.c_str());
          StaticJsonDocument<MESSAGE_DOCUMENT_SIZE>* doc = new StaticJsonDocument<MESSAGE_DOCUMENT_SIZE>();
          bool res = m_serializer.deserialize(doc, backlogFile);
          bool pres = false;
          if(res)
          {
            pres = strategy->publishMessage(doc);
            if(pres) 
            {
              m_successCnt++;
            }
            else
            {
              m_failCnt++;
            }
          }
          backlogFile.close();
          delete[] doc;
          if (pres||!res)
          {
            if (!FILESYSTEM.remove(backlogFilename))
            {
              btlog_e(TAG_PUBLISHER, "Failed to remove measurement backlog file: %s", backlogFilename.c_str());
            }
            backlogCount--;
            NVS.setInt(BACKLOG_NVS, backlogCount);
          }
          else
          {
            break;
          }
          
        }
        else
        {

          btlog_e(TAG_PUBLISHER, "Failed to open measurement backlog file: %s", backlogFilename.c_str());
          backlogCount--;
          NVS.setInt(BACKLOG_NVS, backlogCount);
        }
      }
      while (backMessage() != nullptr)
      {
        if (
            (
                !connected ||
                backlogCount > 0 ||
                !getSelectedStrategy()->publishMessage(backMessage())) &&
            backlogCount < MAX_BACKLOG)
        {
          storeToBacklog(backMessage());
          m_failCnt++;
        } else
        {
          m_successCnt++;
        }

        popMessage();
      }
      m_connection->suspend();
    }
  }
  return true;
}

JsonDocument *Publisher::backMessage()
{
  xSemaphoreTake(storageSemaphore, 20000 / portTICK_PERIOD_MS);
  if (messageStorage.size() > 0)
  {
    JsonDocument *message = messageStorage.back();
    xSemaphoreGive(storageSemaphore);
    return message;
  }
  else
  {
    return nullptr;
  }
}
void Publisher::popMessage()
{
  xSemaphoreTake(storageSemaphore, 20000 / portTICK_PERIOD_MS);
  JsonDocument *message = messageStorage.back();
  messageStorage.pop_back();
  delete[] message;
  xSemaphoreGive(storageSemaphore);
}
void Publisher::pushMessage(JsonDocument *message)
{
  xSemaphoreTake(storageSemaphore, 20000 / portTICK_PERIOD_MS);
  if (messageStorage.size() == STORAGE_SIZE)
  {
    storeAllToBacklog();
  }
  messageStorage.push_back(message);
  xSemaphoreGive(storageSemaphore);
}

void Publisher::storeAllToBacklog()
{
  while (!messageStorage.empty())
  {
    storeToBacklog(messageStorage.front());
    delete[] messageStorage.front();
    messageStorage.pop_back();
  }
}

void Publisher::storeToBacklog(JsonDocument *message)
{
  // write to backlog only if absolute time - it makes sense
  if (TimeManagement::getInstance()->isAbsoluteTime())
  {
    backlogCount++;
    // add to backlog
    String backlogFilename = String(BACKLOG_DIR_PREFIX);
    backlogFilename += backlogCount;
    backlogFilename += BACKLOG_EXTENSION;
    File backlogFile = FILESYSTEM.open(backlogFilename, FILE_WRITE);
    if (backlogFile)
    {
      btlog_i(TAG_PUBLISHER, "Writing to measurement backlog file %s:", backlogFilename.c_str());
      char messageStr[MESSAGE_SIZE];
      m_serializer.serialize(message, messageStr);
      int len = strlen(messageStr);
      backlogFile.write((uint8_t *)messageStr, len);
      backlogFile.close();
    }
    else
    {
      btlog_e(TAG_PUBLISHER, "Failed to create measurement backlog file: %s", backlogFilename.c_str());
    }
    NVS.setInt(BACKLOG_NVS, backlogCount);
  }
}

void Publisher::readSettings(const JsonObject &source)
{
  // publisher settings

  JsonObject publisherSettings = source[STR_PUBLISHERSETTINGS];
  m_protocol = publisherSettings[STR_PUBLISHERPROTOCOL];
}

void Publisher::writeSettings(JsonObject &target, const JsonObject &input)
{
  JsonObject publisherSettings = target.createNestedObject(STR_PUBLISHERSETTINGS);
  publisherSettings[STR_PUBLISHERPROTOCOL] = m_protocol;

  Settings::merge(publisherSettings, input[STR_PUBLISHERSETTINGS]);
}


void Publisher::getInfo(JsonObject &target)
{
  JsonObject info = target.createNestedObject("Publisher");
  info["Backlog count"] = NVS.getInt(BACKLOG_NVS);
  info["Success publish count"] = m_successCnt;
  info["Failed publish count"] = m_failCnt;
}
void Publisher::onShutdown()
{
  storeAllToBacklog();
}
} // namespace beegl
