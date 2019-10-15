/*
  LoraWanPublishStrategy.h - Mqtt PublishStrategy header file
  
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

#ifndef LoraWanPublishStrategy_h
#define LoraWanPublishStrategy_h

#include "Publisher.h"
#include "LoraWanConnectionProvider.h"


#define LORA_MESSAGE_BUFFER 512
#define LORA_MEASUREMENT_MESSAGE_TYPE 0x30
#define LORA_DELIMITER 0x7C

class LoraMessageFormatter
{
public:
  LoraMessageFormatter(Settings* settings);
  int formatMessage(uint8_t *targetLoraMessage, const char *sourceJsonMessage);

protected:
  virtual int formatMessageFromJson(uint8_t *targetLoraMessage, JsonObject *source);
  Settings* m_settings;
};

class LoraMeasurementMessageFormatter : public LoraMessageFormatter
{
public:
  LoraMeasurementMessageFormatter(Settings* settings);
protected:
  int formatMessageFromJson(uint8_t *targetLoraMessage, JsonObject *source) override;
};

class LoraPublishStrategy : public PublishStrategy
{
public:
  LoraPublishStrategy(Runtime *runtime, Settings *settings, Connection *outboundConnection, Service *service);
  void setup() override;
  void update() override;
  bool reconnect() override;
  bool publishMessage(const char *message) override;
  const char getProtocol() { return 0x4; }
  const char* getProtocolName() { return "LoraWan";}
  int getInterval() { return 60000; }
  const char getSupportedOutboundTypes() { return 0x4;}
private:
  LoraMeasurementMessageFormatter m_formatter = NULL;
};

#endif