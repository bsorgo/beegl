/*
  Publisher.h - Http Publisher header file
  
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

#ifndef HttpPublisher_h
#define HttpPublisher_h


#include "Publisher.h"

class HttpPublisher : public Publisher
{
public:
  HttpPublisher(Runtime *runtime, Settings *settings, Connection *outboundConnection, Service *service);
  void setup();

protected:
  bool reconnect();
  bool publishMessage(const char *message);
};

#endif