/*
  LoraWanPublishStrategy.h - Mqtt PublishStrategy header file
  
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

#ifndef LoraWanPublishStrategy_h
#define LoraWanPublishStrategy_h

#include "Publisher.h"
#include <Arduino_LoRaWAN_ttn.h>
#include <arduino_lmic_hal_boards.h>
#include <Arduino_LoRaWAN_lmic.h>
#include <mcciadk_baselib.h>
#include <hal/hal.h>

#define LORA_MESSAGE_BUFFER 512
#define LORA_MEASUREMENT_MESSAGE_TYPE 0x30
#define LORA_DELIMITER 0x7C
class MyLoRaWAN : public Arduino_LoRaWAN_ttn
{
private:
  Arduino_LoRaWAN::OtaaProvisioningInfo *m_provisioningInfo;

public:
  void setProvisioningInfo(Arduino_LoRaWAN::OtaaProvisioningInfo *provisioningInfo)
  {
    m_provisioningInfo = provisioningInfo;
  }

protected:
  // you'll need to provide implementations for each of the following.
  virtual bool GetOtaaProvisioningInfo(Arduino_LoRaWAN::OtaaProvisioningInfo *) override;
  virtual void NetSaveFCntUp(uint32_t uFCntUp) override;
  virtual void NetSaveFCntDown(uint32_t uFCntDown) override;
  virtual void NetSaveSessionInfo(const SessionInfo &Info, const uint8_t *pExtraInfo, size_t nExtraInfo) override;
};

class LoraMessageFormatter
{
public:
  LoraMessageFormatter();
  int formatMessage(uint8_t *targetLoraMessage, const char *sourceJsonMessage);

protected:
  virtual int formatMessageFromJson(uint8_t *targetLoraMessage, JsonObject *source);
};

class LoraMeasurementMessageFormatter : public LoraMessageFormatter
{
public:
  
protected:
  int formatMessageFromJson(uint8_t *targetLoraMessage, JsonObject *source) override;
};

class LoraPublishStrategy : public PublishStrategy
{
public:
  LoraPublishStrategy(Runtime *runtime, Settings *settings, Connection *outboundConnection, Service *service);
  void setup() override;
  void update() override;
  bool reconnect() override;
  bool publishMessage(const char *message) override;
  const char getProtocol() {return 0x4;}
  int getInterval() { return 420000; }
private:
  MyLoRaWAN loraWan{};
  LoraMeasurementMessageFormatter m_formatter{};
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

protected:
  
};

#endif