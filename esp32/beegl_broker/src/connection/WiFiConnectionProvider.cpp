
/*
  WiFiConnectionProvider.cpp - Connection class. Manages inbound & outbound connections for WiFi
  
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

#include "connection/WiFiConnectionProvider.h"

namespace beegl
{

WiFiConnectionProvider::WiFiConnectionProvider(Connection *connection, Settings *settings) : ConnectionProvider(connection, settings)
{
    wifiClient = new WiFiClient();

    m_apIp = IPAddress(192, 168, 4, 1);
    m_apGateway = IPAddress(0, 0, 0, 0);
    m_apSubnet = IPAddress(255, 255, 255, 0);

    m_wifiIp = IPAddress(192, 168, 0, 155);
    m_wifiGateway = IPAddress(192, 168, 0, 1);
    m_wifiSubnet = IPAddress(255, 255, 255, 0);
}

WiFiConnectionProvider *WiFiConnectionProvider::createAndRegister(BeeGl *core)
{
    WiFiConnectionProvider *i = new WiFiConnectionProvider(&core->connection, &core->settings);
    core->registerConnectionProvider(i);
    return i;
}

void WiFiConnectionProvider::readSettings(const JsonObject &source)
{
    // wifi settings

    JsonObject wifiSettings = source[STR_WIFISETTINGS];
    strlcpy(m_wifiSSID, wifiSettings[STR_WIFISSID] | m_wifiSSID, 33);
    strlcpy(m_wifiPassword, wifiSettings[STR_WIFIPASSWORD] | m_wifiPassword, 33);
    m_wifiCustomIp = wifiSettings[STR_WIFICUSTOMIP] ? wifiSettings[STR_WIFICUSTOMIP] : m_wifiCustomIp;
    m_wifiIp.fromString(wifiSettings[STR_WIFIIP] | "192.168.0.155");
    m_wifiGateway.fromString(wifiSettings[STR_WIFIGATEWAY] | "192.168.0.1");
    m_wifiSubnet.fromString(wifiSettings[STR_WIFINETMASK] | "255.255.255.0");

    // wifi ap settings

    JsonObject apSettings = source[STR_APSETTINGS];
    strlcpy(m_apPassword, apSettings[STR_APPASSWORD] | m_apPassword, 17);
    m_apIp.fromString(apSettings[STR_APIP] | "192.168.4.1");
    m_apGateway.fromString(apSettings[STR_APGATEWAY] | "192.168.4.1");
    m_apSubnet.fromString(apSettings[STR_APNETMASK] | "255.255.255.0");
}
void WiFiConnectionProvider::writeSettings(JsonObject &target, const JsonObject &input)
{
    JsonObject wifiSettings = target.createNestedObject(STR_WIFISETTINGS);
    wifiSettings[STR_WIFISSID] = m_wifiSSID;
    wifiSettings[STR_WIFIPASSWORD] = m_wifiPassword;
    wifiSettings[STR_WIFICUSTOMIP] = m_wifiCustomIp;
    wifiSettings[STR_WIFIIP] = m_wifiIp.toString();
    wifiSettings[STR_WIFIGATEWAY] = m_wifiGateway.toString();
    wifiSettings[STR_WIFINETMASK] = m_wifiSubnet.toString();

    Settings::merge(wifiSettings, input[STR_WIFISETTINGS]);

    JsonObject apSettings = target.createNestedObject(STR_APSETTINGS);
    apSettings[STR_APPASSWORD] = m_apPassword;
    apSettings[STR_APIP] = m_apIp.toString();
    apSettings[STR_APGATEWAY] = m_apGateway.toString();
    apSettings[STR_APNETMASK] = m_apSubnet.toString();

    Settings::merge(apSettings, input[STR_APSETTINGS]);
}

void WiFiConnectionProvider::suspend()
{
    // not supported
}

void WiFiConnectionProvider::resume()
{
    // not supported
}

void WiFiConnectionProvider::shutdown()
{
    wifiOff();
}

void WiFiConnectionProvider::wifiOff()
{
    btlog_i(TAG_WIFI, "OFF");
    WiFi.mode(WIFI_OFF);
}

// Wifi setup
bool WiFiConnectionProvider::wifiSetup()
{
    
    int retries = 0;
    btlog_i(TAG_WIFI, "Settings: Use custom ip: %u, IP: %s GW: %s NETMASK: %s", m_wifiCustomIp, m_wifiIp.toString().c_str(), m_wifiGateway.toString().c_str(), m_wifiSubnet.toString().c_str());
    btlog_i(TAG_WIFI, "APSettings: IP: %s GW: %s NETMASK: %s", m_apIp.toString().c_str(), m_apGateway.toString().c_str(), m_apSubnet.toString().c_str());
    if ((m_connection->getInboundMode() & 0x1) && (m_connection->getOutboundMode() & 0x1))
    {

        // STA + AP

        btlog_i(TAG_WIFI, "Starting AP STA");
        btlog_i(TAG_WIFI, "Outbound connecting to %s", m_wifiSSID);
        m_info+= "SSID: ";
        m_info+= m_wifiSSID;
        WiFi.disconnect();
        WiFi.mode(WIFI_AP_STA);
        if (m_wifiCustomIp)
        {
            if (!WiFi.config(m_wifiIp, m_wifiGateway, m_wifiSubnet))
            {
                btlog_e(TAG_WIFI,"STA Config Failed");
            }
        }

        WiFi.begin(m_wifiSSID, m_wifiPassword);

        btlog_i(TAG_WIFI, "AP started");
        btlog_i(TAG_WIFI, "AP IP: %s ", WiFi.softAPIP().toString().c_str());
        m_info+= " AP IP: ";
        m_info+= WiFi.softAPIP().toString();
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
            if (retries > 20)
            {
                return false;
            }
            retries++;
        }
        btlog_i(TAG_WIFI, "connected");
        btlog_i(TAG_WIFI, "address: %s ", WiFi.localIP().toString().c_str());
        m_info+= " IP: ";
        m_info+= WiFi.localIP().toString();
        m_info+=" Signal: ";
        m_info+=WiFi.RSSI();
    }
    else if ((m_connection->getInboundMode() & 0x1) && !(m_connection->getOutboundMode() & 0x1))
    {
        btlog_i(TAG_WIFI, "Starting AP");
        WiFi.disconnect();
        WiFi.mode(WIFI_AP);
        WiFi.softAP(m_settings->deviceName, m_apPassword);
        btlog_i(TAG_WIFI, "AP started");
        btlog_i(TAG_WIFI, "AP IP: %s", WiFi.softAPIP().toString().c_str());
        btlog_i(TAG_WIFI, "AP password: %s", m_apPassword);
        m_info+= "AP IP: ";
        m_info+= WiFi.softAPIP().toString();
    }
    else if (!(m_connection->getInboundMode() & 0x1) && (m_connection->getOutboundMode() & 0x1))
    {
        btlog_i(TAG_WIFI, "Starting STA");
        btlog_i(TAG_WIFI, "Outbound connecting to  %s", m_wifiSSID);
        m_info+= "SSID: ";
        m_info+= m_wifiSSID;
        WiFi.disconnect();
        WiFi.mode(WIFI_STA);
        if (m_wifiCustomIp)
        {
            if (!WiFi.config(m_wifiIp, m_wifiGateway, m_wifiSubnet))
            {
                btlog_e(TAG_WIFI, "STA Config Failed");
            }
        }
        WiFi.begin(m_wifiSSID, m_wifiPassword);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            if (retries > 20)
            {
                return false;
            }
            retries++;
        }
        btlog_i(TAG_WIFI, "connected");
        btlog_i(TAG_WIFI, "IP: %s", WiFi.localIP().toString().c_str());
        m_info+= " IP: ";
        m_info+= WiFi.localIP().toString();
        m_info+=" Signal: ";
        m_info+=WiFi.RSSI();
        return true;
    }
    
    return true;
}

bool WiFiConnectionProvider::setup()
{
    return wifiSetup();
}

void WiFiConnectionProvider::checkConnect()
{
    if (m_connection->getOutboundMode() & 0x1 && !WiFi.isConnected())
    {
        wifiSetup();
    }
}

Client *WiFiConnectionProvider::getClient() const
{
    return wifiClient;
}

void WiFiConnectionProvider::getInfo(JsonObject &target)
{
  JsonObject info = target.createNestedObject("WiFi");
  info["Status"] = m_info.c_str();
}
} // namespace beegl