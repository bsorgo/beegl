/*
  Message.h - Message header file
  
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

#ifndef Message_h
#define Message_h

#include "Log.h"

#include <ArduinoJson.h>

#ifndef MESSAGE_SIZE
#define MESSAGE_SIZE 350
#endif
#ifndef MESSAGE_DOCUMENT_SIZE
#define MESSAGE_DOCUMENT_SIZE 512
#endif

#define TAG_MESSAGE "MESSAGE"

namespace beegl
{
class JsonMessageSerializer
{

public:
  JsonMessageSerializer() {}
  // JSON
  bool serialize(JsonDocument *source, char *target)
  {
    size_t len = measureJson(*source);
    btlog_d(TAG_MESSAGE, "Serializing json of size: %u", len);
    serializeJson(*source, target, len + 1);
    return true;
  }
  bool serialize(JsonDocument *source, File &target)
  {
    serializeJson(*source, target);
    return true;
  }

  JsonDocument *deserialize(const char *source)
  {
    StaticJsonDocument<MESSAGE_DOCUMENT_SIZE> *doc = new StaticJsonDocument<MESSAGE_DOCUMENT_SIZE>();
    auto error = deserializeJson(*doc, source);
    if (error)
    {
      btlog_e(TAG_MESSAGE, "Error deserializing:%s", error);
      return NULL;
    }
    return doc;
  }

  JsonDocument *deserialize(File &source)
  {
    StaticJsonDocument<MESSAGE_DOCUMENT_SIZE> *doc = new StaticJsonDocument<MESSAGE_DOCUMENT_SIZE>();
    auto error = deserializeJson(*doc, source);
    if (error)
    {
      btlog_e(TAG_MESSAGE, "Error deserializing:%s", error);
      return NULL;
    }
    return doc;
  }
};

class IByteMessageSerializer
{
public:
  IByteMessageSerializer() {}
  virtual int serializeBinary(JsonDocument *source, uint8_t *target) { return false; }
};
} // namespace beegl
#endif