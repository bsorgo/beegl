#ifndef LoraWanConnectionProvider_h
#define LoraWanConnectionProvider_h

/*
  LoraWanConnectionProvider.h - LoraWan connection provider
  
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

#include "Log.h"
#include "Connection.h"
#include "Settings.h"

class LoraWanConnectionProvider : public ConnectionProvider
{

public:
  LoraWanConnectionProvider(Settings *settings);
  Client *getClient() override;
  void checkConnect() override;
  bool setup() override;
  void shutdown() override;
  void suspend() override;
  void resume() override;
  char getInboundType() { return 0x0;}
  char getOutboundType() { return 0x4;}
  const char* getName() { return m_name;}
private:
  Settings *m_settings;
  const char m_name[8] = "LORAWAN";

};

#endif