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

void Publisher::publishCallback()
{
  Publisher::getInstance()->publish();
}

Timer Publisher::p_publisherTimer ;
Publisher* Publisher::p_instance = NULL;


void Publisher::setup()
{
    p_publisherTimer.setCallback(Publisher::publishCallback);
    getSelectedStrategy();
}

void Publisher::update()
{
    if(!m_runtime->getSafeMode())
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

PublishStrategy::PublishStrategy(Runtime *runtime, Settings *settings, Connection *connection, Service *service)
{
    m_connection = connection;
    m_settings = settings;
    m_runtime = runtime;
    m_service = service;
}

Publisher::Publisher(Runtime *runtime, Settings *settings, Connection *connection, Service *service)
{
    p_instance = this;
    m_connection = connection;
    m_settings = settings;
    m_runtime = runtime;
    m_service = service;
    if (!FILESYSTEM.exists(BACKLOG_DIR))
    {
        FILESYSTEM.mkdir(BACKLOG_DIR);
    }
    webServerBind();
    
}

Publisher* Publisher::getInstance() {
    return p_instance;
}

void Publisher::addPublishStrategy(PublishStrategy *publishStrategy)
{
    if(publishStrategy!=nullptr)
    {
        blog_d("[PUBLISHER] Add publish strategy: %u", publishStrategy->getProtocol());
        m_publishStrategies[publishStrategyCount] = publishStrategy;
        publishStrategyCount++;
    }
}

PublishStrategy *Publisher::getSelectedStrategy()
{
    if (m_selectedStrategy == nullptr || m_selectedStrategy->getProtocol() != m_settings->protocol)
    {
        for (int i = 0; i < publishStrategyCount; i++)
        {
            if (m_publishStrategies[i]->getProtocol() == m_settings->protocol && m_publishStrategies[i]->getSupportedOutboundTypes() & m_settings->outboundMode)
            {
                blog_i("[PUBLISHER] Selected publish strategy: %u with publish interval: %lu", m_publishStrategies[i]->getProtocol(), m_publishStrategies[i]->getInterval());
                m_selectedStrategy = m_publishStrategies[i];
                m_selectedStrategy->setup();
                p_publisherTimer.stop();
                p_publisherTimer.setInterval(m_selectedStrategy->getInterval());
                p_publisherTimer.start();
                
                break;
            }
        }
    }
    return m_selectedStrategy;
}

int Publisher::getStrategies(PublishStrategy** strategies, char outboundType)
{
    int j = 0;
    for(int i=0;i<publishStrategyCount;i++)
    {
        PublishStrategy* strategy = m_publishStrategies[i];
        if(strategy->getSupportedOutboundTypes() & outboundType)
        {
            strategies[j] = strategy;
            j++;
        }
    }

    return j;
}

int Publisher::getInterval()
{
    PublishStrategy* strategy = getSelectedStrategy();
    if(strategy!=nullptr)
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
        StaticJsonBuffer<128> jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        JsonObject &backlog = root.createNestedObject("backlog");
        backlog["count"] = backlogCount;

        root.printTo(*response);
        jsonBuffer.clear();
        request->send(response);
    });

    m_service->getWebServer()->on("/rest/protocols", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        const String outboundTypeStr = request->getParam(STR_OUTBOUNDMODE, false)->value();
        
        char outboundType = (char)outboundTypeStr.toInt();        
        StaticJsonBuffer<256> jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        JsonArray &array = root.createNestedArray("protocols");
        
        PublishStrategy* strategies[5];
        int count = this->getStrategies(strategies, outboundType);
        for(int i=0;i<count;i++)
        {
            JsonObject & proto = array.createNestedObject();
            proto[STR_PUBLISHERPROTOCOL] =  (int) strategies[i]->getProtocol();
            proto["name"] = strategies[i]->getProtocolName();
        }
        root.printTo(*response);
        jsonBuffer.clear();
        request->send(response);
    });
}

int Publisher::getIndex()
{
    storageIndex++;
    if (storageIndex > STORAGE_SIZE - 1)
    {
        storageIndex = 0;
    }
    if (storageIndex == publishIndex)
    {
        publishIndex = -1;
    }
    return storageIndex;
}

char *Publisher::storeMessage(JsonObject &jsonObj)
{
    int i = getIndex();
    jsonObj.printTo(messageStorage[i], jsonObj.measureLength() + 1);
    blog_d("[STORE] Message");
    char *ret = messageStorage[i];
    return ret;
}

bool Publisher::publish()
{
    if (!m_runtime->getSafeMode())
    {
        PublishStrategy *strategy = getSelectedStrategy();
        if(strategy==nullptr)
        {
            return false;
        }
        if (strategy == nullptr)
        {
            return false;
        }
        if (publishIndex != storageIndex || backlogCount > 0)
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
            blog_d("[PUBLISHER] Backlog count: %u", backlogCount);
            long fileNumber = 0;
            while (connected && backlogCount > 0)
            {
                fileNumber++;
                String backlogFilename = String(BACKLOG_DIR_PREFIX);
                backlogFilename += fileNumber;
                backlogFilename += BACKLOG_EXTENSION;
                File backlogFile = FILESYSTEM.open(backlogFilename, FILE_READ);
                if (backlogFile)
                {
                    String backlogMessage = backlogFile.readString();
                    blog_d("[PUBLISHER] Backlog: %s", backlogMessage.c_str());
                    backlogFile.close();
                    if (strategy->publishMessage(backlogMessage.c_str()))
                    {
                        if (!FILESYSTEM.remove(backlogFilename))
                        {
                            blog_e("[PUBLISHER] Failed to remove measurement backlog file: %s", backlogFilename.c_str());
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

                    blog_e("[PUBLISHER] Failed to open measurement backlog file: %s", backlogFilename.c_str());
                    backlogCount--;
                    NVS.setInt(BACKLOG_NVS, backlogCount);
                }
            }

            while (publishIndex != storageIndex)
            {
                blog_d("[PUBLISHER] Message: %s", messageStorage[publishIndex + 1]);
                if (
                    (
                    !connected || 
                    backlogCount > 0 || 
                    !getSelectedStrategy()->publishMessage(messageStorage[publishIndex + 1])            
                    ) 
                    && backlogCount < MAX_BACKLOG)
                {
                    // write to backlog only if absolute time- it makes sense
                    if(m_settings->absoluteTime)
                    {
                         backlogCount++;
                        // add to backlog
                        String backlogFilename = String(BACKLOG_DIR_PREFIX);
                        backlogFilename += backlogCount;
                        backlogFilename += BACKLOG_EXTENSION;
                        File backlogFile = FILESYSTEM.open(backlogFilename, FILE_WRITE);
                        if (backlogFile)
                        {
                            blog_i("[PUBLISHER] Writing to measurement backlog file %s:", backlogFilename.c_str());
                            int len = strlen(messageStorage[publishIndex + 1]);
                            backlogFile.write((uint8_t *)messageStorage[publishIndex + 1], len);
                            backlogFile.close();
                        }
                        else
                        {
                            blog_e("[PUBLISHER] Failed to create measurement backlog file: %s", backlogFilename.c_str());
                        }
                        NVS.setInt(BACKLOG_NVS, backlogCount);
                    }
                   
                }
                if (backlogCount <= MAX_BACKLOG)
                {
                    publishIndex++;
                    if (publishIndex > STORAGE_SIZE - 1)
                    {
                        publishIndex = -1;
                    }
                }
            }
            m_connection->suspend();
        }
    }
    return true;
}