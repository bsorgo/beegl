/*
  LoraWanPublishStrategy.cpp - Publishes messages over Lorawan
  
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

#include "publisher/LoraWanPublishStrategy.h"
namespace beegl
{
void callback(
    void *pClientData,
    bool fSuccess)
{
}

LoraWanPublishStrategy::LoraWanPublishStrategy(Runtime *runtime, Settings *settings, Connection *connection, Service *service, IByteMessageSerializer *serializer) : PublishStrategy(runtime, settings, connection, service)
{
    m_serializer = serializer;
}

LoraWanPublishStrategy *LoraWanPublishStrategy::createAndRegister(BeeGl *core, IByteMessageSerializer *serializer)
{
    LoraWanPublishStrategy *i = new LoraWanPublishStrategy(&core->runtime, &core->settings, &core->connection, &core->service, serializer);
    core->registerPublishStrategy(i);
    return i;
}

void LoraWanPublishStrategy::LoraWanPublishStrategy::update()
{
    MyLoRaWAN::GetInstance()->loop();
}

void LoraWanPublishStrategy::LoraWanPublishStrategy::setup()
{
}

bool LoraWanPublishStrategy::publishMessage(JsonDocument *payload)
{
    btlog_d(TAG_LORAWANPUBLISHER, "Publishing....");
    uint8_t outputMessage[LORA_MESSAGE_BUFFER];
    if (m_serializer == nullptr)
    {
        btlog_e(TAG_LORAWANPUBLISHER, "No formatter.");
        return false;
    }
    int len = m_serializer->serializeBinary(payload, outputMessage);
    if (len > 0)
    {
        return MyLoRaWAN::GetInstance()->SendBuffer(outputMessage, len);
    }
    else
    {
        btlog_e(TAG_LORAWANPUBLISHER, "No message.");
        return true;
    }
    free(outputMessage);
}

bool LoraWanPublishStrategy::reconnect()
{
    return true;
}
} // namespace beegl
