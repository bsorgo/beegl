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

void Publisher::setup()
{
}

void Publisher::update()
{
}

Publisher::Publisher(Runtime *runtime, Settings *settings, Connection *connection, Service *service)
{
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

bool Publisher::reconnect()
{
    return true;
}

char *Publisher::storeMessage(JsonObject &jsonObj)
{
    int i = getIndex();
    jsonObj.printTo(messageStorage[i], jsonObj.measureLength() + 1);
    blog_d("[STORE] Message");
    char *ret = messageStorage[i];
    return ret;
}

bool Publisher::publishMessage(const char *message)
{
    return false;
}

bool Publisher::publish()
{
    if (!m_runtime->getSafeMode())
    {
        if (publishIndex != storageIndex || backlogCount > 0)
        {
            m_connection->resume();
            m_connection->checkConnect();
            bool connected = false;
            int retries = 0;
            while (!connected)
            {
                connected = reconnect();
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
                    if (publishMessage(backlogMessage.c_str()))
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
                if ((!connected || backlogCount > 0 || !publishMessage(messageStorage[publishIndex + 1])) && backlogCount < MAX_BACKLOG)
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