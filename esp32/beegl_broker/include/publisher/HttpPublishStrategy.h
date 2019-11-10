/*
  HttpPublishStrategy.h - Http PublishStrategy header file
  
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

#ifndef HttpPublishStrategy_h
#define HttpPublishStrategy_h

#include "Publisher.h"
#include "Message.h"
#include "BeeGl.h"

#define CONTENTTYPEJSON "application/json"
#define CONTENTTYPE "Content-Type"
#define CONTENTLENGTH "Content-Length"

#define HTTPPUBLISHER_NAME "HTTP"
#define TAG_HTTPPUBLISHER "HTTPPUBLISHER"
namespace beegl
{
class HttpPublishStrategy : public PublishStrategy
{
public:
  HttpPublishStrategy(Runtime *runtime, Settings *settings, Connection *connection, Service *service);
  static HttpPublishStrategy* createAndRegister(BeeGl *core);

  void setup() override;
  void update() override;
  bool reconnect() override;
  bool publishMessage(JsonDocument *payload) override;
  void readSettings(const JsonObject &source) {}
  void writeSettings(JsonObject &target, const JsonObject &input) {}
  const char getProtocol() const override { return 0x2; }
  const char *getProtocolName() const override { return HTTPPUBLISHER_NAME; }
  const int getInterval() const override { return 60000; }
  const char getSupportedOutboundTypes() const override { return 0x3; }

private:
  JsonMessageSerializer m_serializer;
  void getSensorPublishPath(char* buffer);
};
} // namespace beegl

#endif
