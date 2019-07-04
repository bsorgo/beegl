#ifndef Connection_h
#define Connection_h


/*
  Connection.h - Connection header file
  
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

#include "Settings.h"
#include <TinyGsmClient.h>
#include <WiFiClient.h>
#include <esp_bt.h>


class Connection
{

public:
    Connection(Settings *settings);
    Client *getClient();
    void checkConnect();
    bool setup();
    void shutdown();
    TinyGsm * getModem();

private:
    const int MODEM_RX_PIN = 15;
    const int MODEM_TX_PIN = 14;

    Settings *m_settings;

    HardwareSerial *serialAT;
    TinyGsm *modem;
    TinyGsmClient *gsmClient;

    WiFiClient *wifiClient;

    void btOff();
    void modemOff();
    void wifiOff();

    bool gsmSetup();
    bool gprsSetup();
    bool wifiSetup();
};
#endif 