/*
RTCTimeProviderStrategy.h - RTC time provider
  
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

#ifndef RTCTimeProviderStrategy_h
#define RTCTimeProviderStrategy_h

#include "BeeGl.h"
#include <DS3231.h>
#include <Wire.h>
namespace beegl
{

class RTCTimeProviderStrategy : public TimeProviderStrategy
{

public:
  RTCTimeProviderStrategy(Settings *settings, Connection *connection);
  static RTCTimeProviderStrategy* createAndRegister(BeeGl *core);

  time_t getUTCTime();
  bool syncTime();
  bool syncTimeFrom(TimeProviderStrategy *sourceStrategy);
  void setUTCTime(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t days, uint8_t months, uint8_t years);
  void setUTCTime(time_t time);
  const char getType() { return 0x08; };
  const char *getName() { return "Real Time Clock"; }
  const char getSupportedConnectionOutboundTypes() { return 0x07; };
  const char getSupportedSyncProviders() { return 0x06; };
  bool isAbsoluteTime();
  static int getMonthFromString(char *s);

private:
  RTClib RTC = RTClib();
  DS3231 ds3231 = DS3231();
};
} // namespace beegl
#endif