
/*
  Connection.cpp - Connection class. Manages inbound & outbound connections
  
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


#include "Connection.h"


Connection::Connection(Settings *settings)
{
    serialAT = new HardwareSerial(1);
    modem = new TinyGsm(*serialAT);
    gsmClient = new TinyGsmClient();
    gsmClient->init(modem);
    wifiClient = new WiFiClient();
    m_settings = settings;
}

void Connection::btOff()
{
    blog_i( "[BLE] OFF");
    esp_bt_controller_disable();
}

void Connection::modemOff()
{
    blog_i( "[GSM] OFF");
#ifdef TINY_GSM_MODEM_SIM800
    modem->radioOff();
#endif
    delay(500);
    modem->sendAT("+CSCLK=2");
}

void Connection::suspend()
{
    if (m_settings->outboundMode & 0x2)
    {
#ifdef TINY_GSM_MODEM_SIM800
        modem->sendAT("+CFUN=4");
#endif
        delay(500);
        log_d( "[GSM] SUSPENDED");
    }
}

void Connection::resume()
{
    if (m_settings->outboundMode & 0x2)
    {
    #ifdef TINY_GSM_MODEM_SIM800
        modem->sendAT("+CFUN=1");
    #endif
        blog_d( "[GSM] RESUMED");
    }
    

}

void Connection::shutdown()
{
    if (m_settings->outboundMode & 0x2)
    {
        modemOff();
    }
    if (m_settings->inboundMode & 0x2)
    {
        btOff();
    }
}

void Connection::wifiOff()
{
    blog_i( "[WIFI] OFF");
    WiFi.mode(WIFI_OFF);
}

bool Connection::gprsSetup()
{
    if (m_settings->outboundMode & 0x2)
    {

        blog_i( "[GSM] Connecting to APN %s with username %s and password %s", m_settings->apn, m_settings->apnUser, m_settings->apnPass);
        if (!modem->gprsConnect(m_settings->apn, m_settings->apnUser, m_settings->apnPass))
        {
            blog_i( "[GSM] NOK");
            return false;
        }
        blog_i( "[GSM] OK");
    }
    return true;
}

// Wifi setup
bool Connection::wifiSetup()
{

    int retries = 0;
    blog_i( "[WIFI] Settings: Use custom ip: %u, IP: %s GW: %s NETMASK: %s", m_settings->wifiCustomIp, m_settings->wifiIp.toString().c_str(), m_settings->wifiGateway.toString().c_str(), m_settings->wifiSubnet.toString().c_str());
    blog_i( "[WIFI] APSettings: IP: %s GW: %s NETMASK: %s", m_settings->apIp.toString().c_str(), m_settings->apGateway.toString().c_str(), m_settings->apSubnet.toString().c_str());
    if ((m_settings->inboundMode & 0x1) && (m_settings->outboundMode & 0x1))
    {

        // STA + AP

        blog_i( "[WIFI] Starting AP STA");
        blog_i( "[WIFI] Outbound connecting to %s", m_settings->wifiSSID);
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

        blog_i( "[WIFI] AP started");
        blog_i( "[WIFI] AP IP: %s ", WiFi.softAPIP().toString().c_str());

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
        blog_i( "[WIFI] connected");
        blog_i( "[WIFI] address: %s ", WiFi.localIP().toString().c_str());
    }
    else if ((m_settings->inboundMode & 0x1) && !(m_settings->outboundMode & 0x1))
    {
        blog_i( "[WIFI] Starting AP");
        WiFi.disconnect();
        WiFi.mode(WIFI_AP);
        WiFi.softAP(m_settings->deviceName, m_settings->apPassword);
        blog_i( "[WIFI] AP started");
        blog_i( "[WIFI] AP IP: %s", WiFi.softAPIP().toString().c_str());
        blog_i( "[WIFI] AP password: %s", m_settings->apPassword);
        }
    else if (!(m_settings->inboundMode & 0x1) && (m_settings->outboundMode & 0x1))
    {
        blog_i( "[WIFI] Starting STA");
        blog_i( "[WIFI] Outbound connecting to  %s", m_settings->wifiSSID);
        WiFi.disconnect();
        WiFi.mode(WIFI_STA);
        if (m_settings->wifiCustomIp)
        {
            if (!WiFi.config(m_settings->wifiIp, m_settings->wifiGateway, m_settings->wifiSubnet))
            {
                blog_e( "[WIFI] STA Config Failed");
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
        blog_i( "[WIFI] connected");
        blog_i( "[WIFI] IP: %s", WiFi.localIP().toString().c_str());
        return true;
    }
    else
    {
        // NOWIFI
        Serial.println("[WIFI] No WIFI");
        wifiOff();
    }
    return true;
}

// GPRS setup
bool Connection::gsmSetup()
{
    modem->sendAT("");
    modem->waitResponse(GSM_OK);
    if (m_settings->outboundMode & 0x2)
    {

        
        // Restart takes quite some time
        // To skip it, call init() instead of restart()
        blog_i( "[GSM] Initializing GPRS modem");
        modem->sendAT("+CSCLK=0");
        modem->sendAT("+CFUN=1");
        modem->restart();
        delay(2000);
        blog_i( "[GSM] Modem type: %s , IMEI: %s, ICCID: %s", modem->getModemInfo().c_str(), modem->getIMEI().c_str(), modem->getSimCCID().c_str());
        delay(1000);
        blog_i( "[GSM] Waiting for network...");
        if (!modem->waitForNetwork())
        {
            modemOff();
            return false;
        }
        else
        {

            blog_i("[GSM] OK. Network strenght: %u ", modem->getSignalQuality());
        }
    }
    else
    {
        modemOff();
    }
    return true;
}

bool Connection::setup()
{
    serialAT->begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN, false);
    while (!serialAT)
    {
        ;
    }
    if (!wifiSetup() || !gsmSetup() || !gprsSetup())
    {
        return false;
    }
    return true;
}

void Connection::checkConnect()
{
    if (m_settings->outboundMode & 0x2)
    {
        if (!modem->isGprsConnected())
        {
            gprsSetup();
        }
    }
}

Client *Connection::getClient()
{
    if (m_settings->outboundMode & 0x2)
    {
        return gsmClient;
    }
    else
    {
        return wifiClient;
    }
}
TinyGsm *Connection::getModem() {
    return modem;
}