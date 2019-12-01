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
#include "BeeGl.h"
#include <Arduino_LoRaWAN_ttn.h>
#include <arduino_lmic_hal_boards.h>
#include <Arduino_LoRaWAN_lmic.h>
#include <mcciadk_baselib.h>
#include <hal/hal.h>

#define STR_LORASETTINGS "loraS"
#define STR_LORAAPPEUI "aeui"
#define STR_LORADEVEUI "deui"
#define STR_LORAAPPKEY "akey"

#define TAG_LORAWAN "LORAWAN"

namespace beegl
{
class MyLoRaWAN : public Arduino_LoRaWAN_ttn
{
private:
  Arduino_LoRaWAN::OtaaProvisioningInfo *m_provisioningInfo;

public:
  void setProvisioningInfo(Arduino_LoRaWAN::OtaaProvisioningInfo *provisioningInfo)
  {
    m_provisioningInfo = provisioningInfo;
  }

  uint16_t m_country = 0;
  uint32_t m_netId = 0;
  uint32_t m_cntUp = 0;
  uint32_t m_cntDown = 0;

protected:
  // you'll need to provide implementations for each of the following.
  virtual bool GetOtaaProvisioningInfo(Arduino_LoRaWAN::OtaaProvisioningInfo *) override;
  virtual void NetSaveFCntUp(uint32_t uFCntUp) override;
  virtual void NetSaveFCntDown(uint32_t uFCntDown) override;
  virtual void NetSaveSessionInfo(const SessionInfo &Info, const uint8_t *pExtraInfo, size_t nExtraInfo) override;
};

class LoraWanConnectionProvider : public ConnectionProvider
{

public:
  LoraWanConnectionProvider(Connection *connection, Settings *settings);
  static LoraWanConnectionProvider* createAndRegister(BeeGl *core);

  void checkConnect() override;
  bool setup() override;
  void shutdown() override;
  void suspend() override;
  void resume() override;
  const char getInboundType() { return 0x0; }
  const char getOutboundType() { return 0x4; }
  const char *getName() { return "LoraWan"; }

  void readSettings(const JsonObject &source) override;
  void writeSettings(JsonObject &target, const JsonObject &input) override;

  void getInfo(JsonObject &target) override;

  const char *getLoraAppEUI()
  {
    return m_loraAppEUI;
  }

  const char *getLoraDeviceEUI()
  {
    return m_loraDeviceEUI;
  }

  const char *getLoraAppKey()
  {
    return m_loraAppKey;
  }

private:
  MyLoRaWAN loraWan{};

  Arduino_LoRaWAN::OtaaProvisioningInfo m_provisioningInfo;
  const MyLoRaWAN::lmic_pinmap loraDevicePinMap = {
      .nss = 5,
      .rxtx = MyLoRaWAN::lmic_pinmap::LMIC_UNUSED_PIN,
      .rst = 15,
      .dio = {14, 12, MyLoRaWAN::lmic_pinmap::LMIC_UNUSED_PIN},
      .rxtx_rx_active = 0,
      .rssi_cal = 8,
      .spi_freq = 8000000,
  };

  /* Lorawan parameters
    loraAppEUI:
    TTN App EUI
    loraDevicEUI:
    TTN Device EUI
    loraAppKey
    TTN App key
    */

  char m_loraAppEUI[17] = "0000000000000000";
  char m_loraDeviceEUI[17] = "0000000000000000";
  char m_loraAppKey[33] = "00000000000000000000000000000000";

};
} // namespace beegl
#endif