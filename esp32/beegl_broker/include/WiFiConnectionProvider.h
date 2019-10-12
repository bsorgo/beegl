#ifndef WiFiConnectionProvider_h
#define WiFiConnectionProvider_h

/*
  WifiConnectionProvider.h - WiFi connection
  
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

class WiFiConnectionProvider : public ConnectionProvider
{

public:
  WiFiConnectionProvider(Settings *settings);
  Client *getClient() override;
  void checkConnect() override;
  bool setup() override;
  void shutdown() override;
  void suspend() override;
  void resume() override;
  const char getInboundType() { return 0x1;}
  const char getOutboundType() { return 0x1;}
  const char* getName() { return "WIFI";}

private:
  bool wifiSetup();
  WiFiClient *wifiClient;
  void wifiOff();
  const char m_name[5] = "WIFI";
};

#endif