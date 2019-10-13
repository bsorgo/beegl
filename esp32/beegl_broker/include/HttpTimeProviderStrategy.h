/*
HttpTimeProviderStrategy.h - HTTP time provider
  
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

#ifndef HttpTimeProviderStrategy_h
#define HttpTimeProviderStrategy_h

#include "TimeManagement.h"

class HttpTimeProviderStrategy : public TimeProviderStrategy
{

public:
    HttpTimeProviderStrategy(Settings *settings, Connection *connection);
    time_t getUTCTime();
    bool syncTime();
    bool syncTimeFrom(TimeProviderStrategy *sourceStrategy);
    void setUTCTime(int hours, int minutes, int seconds, int days, int months, int years);
    void setUTCTime(time_t time);
    const char getType() { return 0x02; };
    const char *getName() { return "HTTP"; }
    const char getSupportedConnectionOutboundTypes() { return 0x03; };
    const char getSupportedSyncProviders() { return 0x00; };
    bool isAbsoluteTime();
    static int getMonthFromString(char *s);

};


#endif