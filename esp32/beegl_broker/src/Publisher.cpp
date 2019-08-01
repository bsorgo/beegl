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

Publisher::Publisher(Runtime * runtime, Settings *settings, Connection *connection)
{
    m_connection = connection;
    m_settings = settings;
    m_runtime = runtime;
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

char* Publisher::storeMessage(JsonObject &jsonObj)
{
  int i = getIndex();
  jsonObj.printTo(messageStorage[i], jsonObj.measureLength() + 1);
  blog_d( "[STORE] Message");
  return messageStorage[i];
}

bool Publisher::publishMessage(const char *message) {
    return false;
}


bool Publisher::publish()
{
    if (publishIndex != storageIndex)
    {
        m_connection->checkConnect();
        bool connected = false;
        int retries = 0;
        while (!connected)
        {
            connected = reconnect();
            retries++;
            if (!connected)
            {
                if (retries % 10 == 0)
                {
                    m_connection->checkConnect();
                }
                if (retries % 50 == 0)
                {
                    return false;
                }
            }
        }

        while (publishIndex != storageIndex)
        {
            blog_d("%s", messageStorage[publishIndex + 1]);
            if (publishMessage(messageStorage[publishIndex + 1]))
            {
                publishIndex++;
                if (publishIndex > STORAGE_SIZE - 1)
                {
                    publishIndex = -1;
                }
            }
            else
            {
                break;
            }
        }
    }
    return true;
}