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
#include "connection/LoraWanConnectionProvider.h"
#include "Message.h"
#include "BeeGl.h"
#define LORA_MESSAGE_BUFFER 51
#define LORA_MEASUREMENT_MESSAGE_TYPE 0x30
#define LORA_DELIMITER 0x7C

#define TAG_LORAWANPUBLISHER "LORAWANPUBLISHER"
#define LORAWANPUBLISHER "LoraWan"
namespace beegl
{
class LoraWanPublishStrategy : public PublishStrategy
{
public:
  LoraWanPublishStrategy(Runtime *runtime, Settings *settings, Connection *connection, Service *service, IByteMessageSerializer *serializer);
  static LoraWanPublishStrategy* createAndRegister(BeeGl *core, IByteMessageSerializer *serializer);
  void setup() override;
  void update() override;
  bool reconnect() override;

  bool publishMessage(JsonDocument *payload) override;
  void readSettings(const JsonObject &source) override {}
  void writeSettings(JsonObject &target, const JsonObject &input) override {}

  const char getProtocol() const override { return 0x4; }
  const char *getProtocolName() const override { return LORAWANPUBLISHER; }
  const int getInterval() const override { return 60000; }
  const char getSupportedOutboundTypes() const override { return 0x4; }

private:
  IByteMessageSerializer *m_serializer;
};
}
#endif
