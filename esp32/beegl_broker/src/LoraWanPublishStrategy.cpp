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

#include "LoraWanPublishStrategy.h"

void callback(
    void *pClientData,
    bool fSuccess)
{
}

LoraMessageFormatter::LoraMessageFormatter(Settings *settings)
{
    m_settings = settings;
}

int LoraMessageFormatter::formatMessage(uint8_t *targetLoraMessage, const char *sourceJsonMessage)
{
    StaticJsonBuffer<LORA_MESSAGE_BUFFER> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(sourceJsonMessage);
    JsonObject *ptrRoot = &root;
    if (root.success() && targetLoraMessage)
    {
        return formatMessageFromJson(targetLoraMessage, ptrRoot);
    }
    else
    {
        return 0;
    }
}

LoraMeasurementMessageFormatter::LoraMeasurementMessageFormatter(Settings *settings) : LoraMessageFormatter(settings)
{
}

int LoraMeasurementMessageFormatter::formatMessageFromJson(uint8_t *targetLoraMessage, JsonObject *source)
{
    int p = 0;
    const uint8_t delimiter[1] = {LORA_DELIMITER};
    const uint8_t messageType[1] = {LORA_MEASUREMENT_MESSAGE_TYPE};
    JsonObject &sourceRef = *source;
    // message type
    memcpy(targetLoraMessage + p, messageType, 1);
    p += 1;
    // device id
    char deviceId[9];
    strcpy(deviceId, sourceRef[STR_DEVICEID]);
    memcpy(targetLoraMessage + p, deviceId, 8);
    p += 8;
    // delimiter
    memcpy(targetLoraMessage + p, delimiter, 1);
    p += 1;
    // epoch time
    if (sourceRef.containsKey(STR_EPOCHTIME))
    {
        if (TimeManagement::getInstance()->isAbsoluteTime())
        {
            char time[11];
            strlcpy(time, sourceRef[STR_EPOCHTIME], 10);
            memccpy(targetLoraMessage + p, time, 0, 10);
            p += 10;
        }
        else
        {
            char time[11];
            long value = sourceRef.get<long>(STR_EPOCHTIME) + now();
            sprintf(time, "%lu", value);
            memccpy(targetLoraMessage + p, time, 0, 10);
            p += 10;
        }
    }
    // time (old time)
    if (sourceRef.containsKey(STR_TIME))
    {
        char time[30];
        strlcpy(time, sourceRef[STR_TIME], 29);
        memccpy(targetLoraMessage + p, time, 0, 29);
        p += 29;
    }
    // delimiter
    memcpy(targetLoraMessage + p, delimiter, 1);
    p += 1;
    // measurement
    // weight
    if (sourceRef.containsKey(STR_WEIGHT))
    {
        char weight[6];
        strlcpy(weight, sourceRef[STR_WEIGHT], 5);
        memcpy(targetLoraMessage + p, weight, 5);
        p += 5;
    }
    // delimiter
    memcpy(targetLoraMessage + p, delimiter, 1);
    p += 1;
    // temperature
    if (sourceRef.containsKey(STR_TEMP))
    {
        char temperature[6];
        strlcpy(temperature, sourceRef[STR_TEMP], 5);
        memcpy(targetLoraMessage + p, temperature, 5);
        p += 5;
    }
    // delimiter
    memcpy(targetLoraMessage + p, delimiter, 1);
    p += 1;
    // humidity
    if (sourceRef.containsKey(STR_HUMIDITY))
    {
        char humidity[4];
        strlcpy(humidity, sourceRef[STR_HUMIDITY], 3);
        memcpy(targetLoraMessage + p, humidity, 3);
        p += 3;
    }
    blog_d("[LORA] Message size: %u", p);

    return p;
}

void LoraPublishStrategy::update()
{
    MyLoRaWAN::GetInstance()->loop();
}

void LoraPublishStrategy::setup()
{
}

LoraPublishStrategy::LoraPublishStrategy(Runtime *runtime, Settings *settings, Connection *connection, Service *service) : PublishStrategy(runtime, settings, connection, service)
{
    m_formatter = LoraMeasurementMessageFormatter(m_settings);
}

bool LoraPublishStrategy::publishMessage(const char *message)
{
    uint8_t *outputMessage = (uint8_t *)malloc(51);
    int len = m_formatter.formatMessage(outputMessage, message);
    if (len > 0)
    {
        return MyLoRaWAN::GetInstance()->SendBuffer(outputMessage, len);
    }
    else
    {
        return true;
    }

    free(outputMessage);
}

bool LoraPublishStrategy::reconnect()
{
    return true;
}
