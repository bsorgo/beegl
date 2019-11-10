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
#include "BeeGl.h"
#include <TinyGsmClient.h>

#define STR_GPRSSETTINGS "gprsS"
#define STR_GPRSAPN "apn"
#define STR_GPRSUSERNAME "user"
#define STR_GPRSPASSWORD "pwd"

#define TAG_GSM "GSM"

namespace beegl
{
class TinyGsmConnectionProvider : public ConnectionProvider
{

public:
  TinyGsmConnectionProvider(Connection *connection, Settings *settings);
  static TinyGsmConnectionProvider* createAndRegister(BeeGl *core);

  Client *getClient() const override;
  void checkConnect() override;
  bool setup() override;
  void shutdown() override;
  TinyGsm *getModem() const;
  void modemOff();
  void suspend() override;
  void resume() override;
  void modemPowerup();
  const char getInboundType() { return 0x0; }
  const char getOutboundType() { return 0x2; }
  const char *getName() { return "GSM/NB-IOT"; }

  void readSettings(const JsonObject &source) override;
  void writeSettings(JsonObject &target, const JsonObject &input) override;

  const char *getApn()
  {
    return m_apn;
  }

  const char *getApnUser()
  {
    return m_apnUser;
  }

  const char *getApnPass()
  {
    return m_apnPass;
  }

private:
  const int MODEM_RX_PIN = 15;
  const int MODEM_TX_PIN = 14;
  const int MODEM_POWER_PIN = 5;

  HardwareSerial serialAT{1};
  TinyGsm modem {serialAT};
  TinyGsmClient gsmClient {};

  /* GPRS parameters 
    apn:
    APN
    apnUser:
    APN username:
    apnPassword:
    APN password
    */
  char m_apn[33] = "internet";
  char m_apnUser[17] = "mobitel";
  char m_apnPass[17] = "internet";

  bool gsmSetup();
  bool gprsSetup();
};
} // namespace beegl
#endif