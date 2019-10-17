
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

#include "WiFiConnectionProvider.h"

// set up the data structures.



WiFiConnectionProvider::WiFiConnectionProvider(Settings *settings) : ConnectionProvider (settings)
{
    wifiClient = new WiFiClient();
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
    blog_i("[WIFI] OFF");
    WiFi.mode(WIFI_OFF);
}


// Wifi setup
bool WiFiConnectionProvider::wifiSetup()
{
    int retries = 0;
    blog_i("[WIFI] Settings: Use custom ip: %u, IP: %s GW: %s NETMASK: %s", m_settings->wifiCustomIp, m_settings->wifiIp.toString().c_str(), m_settings->wifiGateway.toString().c_str(), m_settings->wifiSubnet.toString().c_str());
    blog_i("[WIFI] APSettings: IP: %s GW: %s NETMASK: %s", m_settings->apIp.toString().c_str(), m_settings->apGateway.toString().c_str(), m_settings->apSubnet.toString().c_str());
    if ((m_settings->inboundMode & 0x1) && (m_settings->outboundMode & 0x1))
    {

        // STA + AP

        blog_i("[WIFI] Starting AP STA");
        blog_i("[WIFI] Outbound connecting to %s", m_settings->wifiSSID);
        WiFi.disconnect();
        WiFi.mode(WIFI_AP_STA);
        if (m_settings->wifiCustomIp)
        {
            if (!WiFi.config(m_settings->wifiIp, m_settings->wifiGateway, m_settings->wifiSubnet))
            {
                Serial.println("[WIFI] STA Config Failed");
            }
        }

        WiFi.begin(m_settings->wifiSSID, m_settings->wifiPassword);

        blog_i("[WIFI] AP started");
        blog_i("[WIFI] AP IP: %s ", WiFi.softAPIP().toString().c_str());

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
        blog_i("[WIFI] connected");
        blog_i("[WIFI] address: %s ", WiFi.localIP().toString().c_str());
    }
    else if ((m_settings->inboundMode & 0x1) && !(m_settings->outboundMode & 0x1))
    {
        blog_i("[WIFI] Starting AP");
        WiFi.disconnect();
        WiFi.mode(WIFI_AP);
        WiFi.softAP(m_settings->deviceName, m_settings->apPassword);
        blog_i("[WIFI] AP started");
        blog_i("[WIFI] AP IP: %s", WiFi.softAPIP().toString().c_str());
        blog_i("[WIFI] AP password: %s", m_settings->apPassword);
    }
    else if (!(m_settings->inboundMode & 0x1) && (m_settings->outboundMode & 0x1))
    {
        blog_i("[WIFI] Starting STA");
        blog_i("[WIFI] Outbound connecting to  %s", m_settings->wifiSSID);
        WiFi.disconnect();
        WiFi.mode(WIFI_STA);
        if (m_settings->wifiCustomIp)
        {
            if (!WiFi.config(m_settings->wifiIp, m_settings->wifiGateway, m_settings->wifiSubnet))
            {
                blog_e("[WIFI] STA Config Failed");
            }
        }
        WiFi.begin(m_settings->wifiSSID, m_settings->wifiPassword);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            if (retries > 20)
            {
                return false;
            }
            retries++;
        }
        blog_i("[WIFI] connected");
        blog_i("[WIFI] IP: %s", WiFi.localIP().toString().c_str());
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
    if(!WiFi.isConnected())
    {
        wifiSetup();
    }
}

Client *WiFiConnectionProvider::getClient()
{
    return wifiClient;
}
