#ifndef TinyGsmConnectionProvider_h
#define TinyGsmConnectionProvider_h

/*
  TinyGsmConnectionProvider.h - TinyGSM connection
  
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
#include <TinyGsmClient.h>




class TinyGsmConnectionProvider : public ConnectionProvider
{

public:
  TinyGsmConnectionProvider(Settings *settings);
  Client *getClient() override;
  void checkConnect() override;
  bool setup() override;
  void shutdown() override;
  TinyGsm *getModem();
  void modemOff();
  void suspend() override;
  void resume() override;
  void modemPowerup();
  char getInboundType() { return 0x0;}
  char getOutboundType() { return 0x2;}
  const char* getName() { return m_name;}
private:
  const int MODEM_RX_PIN = 15;
  const int MODEM_TX_PIN = 14;
  const int MODEM_POWER_PIN = 5;

  HardwareSerial *serialAT;
  TinyGsm *modem;
  TinyGsmClient *gsmClient;
  bool gsmSetup();
  bool gprsSetup();
  const char m_name[11] = "GSM/NB-IOT";
};

#endif