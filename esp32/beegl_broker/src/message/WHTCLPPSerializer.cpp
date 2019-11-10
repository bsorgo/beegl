/*
  WHTCLPPSerializer.cpp - Measure values
  
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

#include "message/WHTCLPPSerializer.h"
namespace beegl
{
int WHTCLPPSerializer::serializeBinary(JsonDocument *source, uint8_t *target)
{
  JsonObject root = source->as<JsonObject>();
  lpp.reset();

  if (TimeManagement::getInstance()->isAbsoluteTime())
  {
    lpp.addUnixTime(1, root[STR_EPOCHTIME]);
  }
  else
  {
    long value = root[STR_EPOCHTIME];
    value += now()*1000;
    lpp.addUnixTime(2, value);
  }

  if (root.containsKey(STR_WEIGHTSENSOR))
  {
    lpp.addGenericSensor(4, root[STR_WEIGHTSENSOR][STR_WEIGHT]);
  }
  if (root.containsKey(STR_TEMPSENSOR))
  {
    lpp.addTemperature(5, root[STR_TEMPSENSOR][STR_TEMP]);
  }
  if (root.containsKey(STR_HUMIDITYSENSOR))
  {
    lpp.addRelativeHumidity(6, root[STR_HUMIDITYSENSOR][STR_HUMIDITY]);
  }
  int size = lpp.copy(target);
  btlog_d(TAG_LPP, "CayenneLPP message size: %u", size);
  return size;
}
} // namespace beegl