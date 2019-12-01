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
#include "BeeGl.h"
#define STR_WIFISETTINGS "wifiS"
#define STR_WIFISSID "ssid"
#define STR_WIFIPASSWORD "pwd"
#define STR_WIFICUSTOMIP "cip"
#define STR_WIFIIP "ip"
#define STR_WIFIGATEWAY "gw"
#define STR_WIFINETMASK "nmsk"

#define STR_APSSID "ssid"
#define STR_APPASSWORD "pwd"
#define STR_APSETTINGS "apS"
#define STR_APIP "ip"
#define STR_APGATEWAY "gw"
#define STR_APNETMASK "nmsk"

#define TAG_WIFI "WIFI"

namespace beegl
{
class WiFiConnectionProvider : public ConnectionProvider
{

public:
  WiFiConnectionProvider(Connection *connection, Settings *settings);
  static WiFiConnectionProvider*createAndRegister(BeeGl *core);
  Client *getClient() const override;
  void checkConnect() override;
  bool setup() override;
  void shutdown() override;
  void suspend() override;
  void resume() override;
  const char getInboundType() { return 0x1; }
  const char getOutboundType() { return 0x1; }
  const char *getName() { return "WIFI"; }
  const char compatibleInboundType() { return 0x02;}

  void readSettings(const JsonObject &source) override;
  void writeSettings(JsonObject &target, const JsonObject &input) override;

  void getInfo(JsonObject &target) override;

  const char *getWifiPassword()
  {
    return m_wifiPassword;
  }
  const char *getWifiSSID()
  {
    return m_wifiSSID;
  }
  int getWifiCustomIp()
  {
    return m_wifiCustomIp;
  }
  IPAddress getWifiIp()
  {
    return IPAddress(m_wifiIp);
  }
  IPAddress getWifiGateway()
  {
    return IPAddress(m_wifiGateway);
  }
  IPAddress getWifiSubnet()
  {
    return IPAddress(m_wifiSubnet);
  }

  const char *getApPassword()
  {
    return m_apPassword;
  }
  IPAddress getApIp()
  {
    return IPAddress(m_apIp);
  }
  IPAddress getApGateway()
  {
    return IPAddress(m_apGateway);
  }
  IPAddress getApSubnet()
  {
    return IPAddress(m_apSubnet);
  }

private:
  WiFiClient *wifiClient;

  const char m_name[5] = "WIFI";

  /* WIFI STA parameters 
    wifiPassword:
    Wifi password
    wifiSSID:
    SSID
    wifiCustomIp:
    0 - No custom IP (DHCP)
    1 - Custom IP
    wifiGateway:
    Gateway
    wifiSubnet:
    Subnet - netmask
    */
  char m_wifiPassword[32] = "123456789";
  char m_wifiSSID[32] = "WifiSSID";
  int m_wifiCustomIp = 0;
  IPAddress m_wifiIp;
  IPAddress m_wifiGateway;
  IPAddress m_wifiSubnet;
  /* WIFI AP parameters 
    apPassword:
    Access point password
    apIP:
    Access point IP
    apGatway:
    Access point gateway
    apSubnet:
    Subnet - netmask
    */
  char m_apPassword[16] = "123456789";
  IPAddress m_apIp;
  IPAddress m_apGateway;
  IPAddress m_apSubnet;

  void wifiOff();
  bool wifiSetup();

  String m_info;
};
} // namespace beegl
#endif