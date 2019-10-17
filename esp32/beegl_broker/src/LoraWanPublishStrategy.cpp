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
    StaticJsonDocument<LORA_MESSAGE_BUFFER> jsonBuffer;
    auto error = deserializeJson(jsonBuffer, sourceJsonMessage);
    
    if (!error && targetLoraMessage)
    {
        JsonObject root = jsonBuffer.as<JsonObject>();
        return formatMessageFromJson(targetLoraMessage, root);
    }
    else
    {
        return 0;
    }
}

LoraMeasurementMessageFormatter::LoraMeasurementMessageFormatter(Settings *settings) : LoraMessageFormatter(settings)
{
}

int LoraMeasurementMessageFormatter::formatMessageFromJson(uint8_t *targetLoraMessage, const JsonObject& source)
{
    int p = 0;
    const uint8_t delimiter[1] = {LORA_DELIMITER};
    const uint8_t messageType[1] = {LORA_MEASUREMENT_MESSAGE_TYPE};
    // message type
    memcpy(targetLoraMessage + p, messageType, 1);
    p += 1;
    // device id
    char deviceId[9];
    strcpy(deviceId, source[STR_DEVICEID]);
    memcpy(targetLoraMessage + p, deviceId, 8);
    p += 8;
    // delimiter
    memcpy(targetLoraMessage + p, delimiter, 1);
    p += 1;
    // epoch time
    if (source.containsKey(STR_EPOCHTIME))
    {
        if (TimeManagement::getInstance()->isAbsoluteTime())
        {
            char time[11];
            strlcpy(time, source[STR_EPOCHTIME].as<String>().c_str(),10);
            memccpy(targetLoraMessage + p, time, 0, 10);
            p += 10;
        }
        else
        {
            char time[11];
            long value = source[STR_EPOCHTIME].as<long>();
            value += now();
            sprintf(time, "%lu", value);
            memccpy(targetLoraMessage + p, time, 0, 10);
            p += 10;
        }
    }
    // time (old time)
    if (source.containsKey(STR_TIME))
    {
        char time[30];
        strlcpy(time, source[STR_TIME].as<String>().c_str(),29);
        memccpy(targetLoraMessage + p, time, 0, 29);
        p += 29;
    }
    // delimiter
    memcpy(targetLoraMessage + p, delimiter, 1);
    p += 1;
    // measurement
    // weight
    if (source.containsKey(STR_WEIGHT))
    {
        char weight[6];
        strlcpy(weight, source[STR_WEIGHT].as<String>().c_str(),5);
        memcpy(targetLoraMessage + p, weight, 5);
        p += 5;
    }
    // delimiter
    memcpy(targetLoraMessage + p, delimiter, 1);
    p += 1;
    // temperature
    if (source.containsKey(STR_TEMP))
    {
        char temperature[6];
        strlcpy(temperature, source[STR_TEMP].as<String>().c_str(),5);
        memcpy(targetLoraMessage + p, temperature, 5);
        p += 5;
    }
    // delimiter
    memcpy(targetLoraMessage + p, delimiter, 1);
    p += 1;
    // humidity
    if (source.containsKey(STR_HUMIDITY))
    {
        char humidity[4];
        strcpy(humidity, source[STR_HUMIDITY]);
        memcpy(targetLoraMessage + p, humidity, 3);
        p += 3;
    }
    blog_d("[LORA] Message size: %u", p);

    return p;
}

LoraMeasurementCayenneLPPMessageFormatter::LoraMeasurementCayenneLPPMessageFormatter(Settings *settings) : LoraMessageFormatter(settings)
{
}

int LoraMeasurementCayenneLPPMessageFormatter::formatMessageFromJson(uint8_t *targetLoraMessage, const JsonObject& source)
{
    lpp.reset();
     if (source.containsKey(STR_EPOCHTIME))
    {
        if (TimeManagement::getInstance()->isAbsoluteTime())
        {
            long value = source[STR_EPOCHTIME].as<long>();
            lpp.addUnixTime(1, value);
        }
        else
        {
            long value = source[STR_EPOCHTIME].as<long>();
            value += now();
            lpp.addUnixTime(2, value);
        }
    }

    if (source.containsKey(STR_WEIGHT))
    {
        lpp.addGenericSensor(3, source[STR_WEIGHT].as<float>());
    }
    if (source.containsKey(STR_TEMP))
    {
       lpp.addTemperature(4, source[STR_TEMP].as<float>()); 
    }
    if (source.containsKey(STR_HUMIDITY))
    {
        lpp.addPercentage(5, source[STR_HUMIDITY].as<int>()); 
    }
    int size = lpp.copy(targetLoraMessage);
    blog_d("[LORA] CayeneLPP message size: %u", size);
    return size;
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
    m_formatter = new LoraMeasurementCayenneLPPMessageFormatter(m_settings);
}

bool LoraPublishStrategy::publishMessage(const char *message)
{
    uint8_t *outputMessage = (uint8_t *)malloc(51);
    int len = m_formatter->formatMessage(outputMessage, message);
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
