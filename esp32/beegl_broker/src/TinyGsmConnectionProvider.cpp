
/*
  TinyGsmConnectionProvder.cpp 
  
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

#include "TinyGsmConnectionProvider.h"

// set up the data structures.

TinyGsmConnectionProvider::TinyGsmConnectionProvider(Settings *settings) : ConnectionProvider(settings)
{

    serialAT = new HardwareSerial(1);
    modem = new TinyGsm(*serialAT);
    gsmClient = new TinyGsmClient();
    gsmClient->init(modem);
 

    pinMode(MODEM_POWER_PIN, OUTPUT);
    digitalWrite(MODEM_POWER_PIN, HIGH);
}


void TinyGsmConnectionProvider::modemOff()
{
    blog_i("[GSM] OFF");
    modem->poweroff();
#if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM800)
    delay(500);
    modem->sendAT(GF("+CSCLK=2"));
    modem->waitResponse(5000L);
#endif
}

void TinyGsmConnectionProvider::modemPowerup()
{
#if defined(TINY_GSM_MODEM_SIM7020) || defined(TINY_GSM_MODEM_SIM800)
    digitalWrite(MODEM_POWER_PIN, LOW);
    delay(400);
    digitalWrite(MODEM_POWER_PIN, HIGH);
    delay(800);
#endif
}

void TinyGsmConnectionProvider::suspend()
{
    
#if defined(TINY_GSM_MODEM_SIM7020)
    modem->sendAT(GF("+CPSMS=1"));
    modem->waitResponse(5000L);
#endif
#if defined(TINY_GSM_MODEM_SIM7020) || defined(TINY_GSM_MODEM_SIM800)
    modem->sendAT(GF("+CSCLK=2"));
    modem->waitResponse(5000L);
#endif
    log_d("[GSM] SUSPENDED");
    
}

void TinyGsmConnectionProvider::resume()
{
#if defined(TINY_GSM_MODEM_SIM7020) || defined(TINY_GSM_MODEM_SIM800)
    modemPowerup();
    modem->testAT();
    delay(200);
    modem->testAT();
    delay(200);
    modem->waitResponse(10000L);
#endif
    blog_d("[GSM] RESUMED");
}

void TinyGsmConnectionProvider::shutdown()
{
    modemOff();
}

bool TinyGsmConnectionProvider::gprsSetup()
{
    if (m_settings->outboundMode & 0x2)
    {

        blog_i("[GSM] Connecting to APN %s with username %s and password %s", m_settings->apn, m_settings->apnUser, m_settings->apnPass);
        if (!modem->gprsConnect(m_settings->apn, m_settings->apnUser, m_settings->apnPass))
        {
            blog_i("[GSM] NOK");
            return false;
        }
        blog_i("[GSM] OK");
    }
    return true;
}

// GPRS setup
bool TinyGsmConnectionProvider::gsmSetup()
{
    modemPowerup();
    modem->sendAT("");
    modem->waitResponse(GSM_OK);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    blog_i("[GSM] Initializing GPRS modem");
    modem->restart();
    if (modem->testAT() != 1)
    {
        return false;
    }
    blog_i("[GSM] Modem type: %s , IMEI: %s, ICCID: %s", modem->getModemInfo().c_str(), modem->getIMEI().c_str(), modem->getSimCCID().c_str());
    if (modem->testAT() != 1)
    {
        return false;
    }
    blog_i("[GSM] Waiting for network...");
    if (!modem->waitForNetwork())
    {
        modemOff();
        return false;
    }
    else
    {

        blog_i("[GSM] OK. Network strenght: %u ", modem->getSignalQuality());
    }
    return true;
}
bool TinyGsmConnectionProvider::setup()
{

    serialAT->begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN, false);
    while (!serialAT)
    {
        ;
    }
    if (gsmSetup() && gprsSetup())
    {
        return true;
    }
    return false;
}
void TinyGsmConnectionProvider::checkConnect()
{
    if (!modem->isGprsConnected())
    {
        gprsSetup();
    }
}
Client *TinyGsmConnectionProvider::getClient()
{
        return gsmClient;
}
TinyGsm *TinyGsmConnectionProvider::getModem()
{
    return modem;
}
