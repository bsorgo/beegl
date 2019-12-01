
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

#include "connection/TinyGsmConnectionProvider.h"

namespace beegl
{

TinyGsmConnectionProvider::TinyGsmConnectionProvider(Connection *connection, Settings *settings) : ConnectionProvider(connection, settings), serialAT(1), modem(serialAT)
{

    gsmClient.init(&modem);
    pinMode(MODEM_POWER_PIN, OUTPUT);
    digitalWrite(MODEM_POWER_PIN, LOW);
    serialAT.begin(38400, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN, false);
    while (!serialAT)
    {
        ;
    }
}

TinyGsmConnectionProvider *TinyGsmConnectionProvider::createAndRegister(BeeGl *core)
{
    TinyGsmConnectionProvider *i = new TinyGsmConnectionProvider(&core->connection, &core->settings);
    core->registerConnectionProvider(i);
    return i;
}

void TinyGsmConnectionProvider::readSettings(const JsonObject &source)
{
    // gprs settings

    JsonObject gprsSettings = source[STR_GPRSSETTINGS];
    strlcpy(m_apn, gprsSettings[STR_GPRSAPN] | m_apn, 32);
    strlcpy(m_apnUser, gprsSettings[STR_GPRSUSERNAME] | m_apnUser, 16);
    strlcpy(m_apnPass, gprsSettings[STR_GPRSPASSWORD] | m_apnPass, 16);
}
void TinyGsmConnectionProvider::writeSettings(JsonObject &target, const JsonObject &input)
{
    JsonObject gprsSettings = target.createNestedObject(STR_GPRSSETTINGS);
    gprsSettings[STR_GPRSAPN] = m_apn;
    gprsSettings[STR_GPRSPASSWORD] = m_apnPass;
    gprsSettings[STR_GPRSUSERNAME] = m_apnUser;

    Settings::merge(gprsSettings, input[STR_GPRSSETTINGS]);
}

void TinyGsmConnectionProvider::modemOff()
{
    btlog_i(TAG_GSM, "OFF");
    modem.radioOff();
#if defined(TINY_GSM_MODEM_SIM7020) || defined(TINY_GSM_MODEM_SIM800)
    delay(500);
#else
    modem.poweroff();
#endif
}

bool TinyGsmConnectionProvider::modemPowerup()
{
    if(modem.testAT())
    {
        return true;
    }
    int i = 0;
    do
    {
        i++;
        if (i > 10)
        {
            return false;
        }

#if defined(TINY_GSM_MODEM_SIM7020) || defined(TINY_GSM_MODEM_SIM800)

        digitalWrite(MODEM_POWER_PIN, LOW);
        delay(400);
        digitalWrite(MODEM_POWER_PIN, HIGH);
        delay(800);
        if (!Serial)
        {
            serialAT.begin(38400, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN, false);
        }
        while (!serialAT)
        {
            ;
        }
#endif
        modem.testAT();
    } while (!modem.testAT());
    return true;
}

void TinyGsmConnectionProvider::suspend()
{
#ifdef TINY_GSM_MODEM_SIM800
    modem.sendAT(GF("+CFUN=4"));
    modem.waitResponse(5000L);
#endif
    btlog_d(TAG_GSM, "SUSPENDED");
}

void TinyGsmConnectionProvider::resume()
{
    modemPowerup();
#ifdef TINY_GSM_MODEM_SIM800
    modem.sendAT(GF("+CFUN=1"));
    modem.waitResponse(5000L);
#endif
    btlog_d(TAG_GSM, "RESUMED");
}

void TinyGsmConnectionProvider::shutdown()
{
    modemOff();
}

bool TinyGsmConnectionProvider::gprsSetup()
{
    btlog_i(TAG_GSM, "Connecting to APN %s with username %s and password %s", m_apn, m_apnUser, m_apnPass);
    if (!modem.gprsConnect(m_apn, m_apnUser, m_apnPass))
    {
        btlog_e(TAG_GSM, "NOK");
        return false;
    }
    btlog_i(TAG_GSM, "OK");

    return true;
}

// GPRS setup
bool TinyGsmConnectionProvider::gsmSetup()
{
    if(!modemPowerup())
    {
        return false;
    }
    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    btlog_i(TAG_GSM, "Initializing GPRS modem");
#if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM7020)
    modem.sendAT("+CSCLK=0");
    modem.sendAT("+CFUN=1");
#endif


    modem.restart();
    if (modem.testAT() != 1)
    {
        return false;
    }
    String modemInfo = "Modem type: " + modem.getModemInfo() + ", IMEI: " + modem.getIMEI();
    btlog_i(TAG_GSM, "%s", modemInfo.c_str());
    strcat(m_modemInfo, modemInfo.c_str());
    if (modem.testAT() != 1)
    {
        return false;
    }
    btlog_i(TAG_GSM, "Waiting for network...");
    if (!modem.waitForNetwork())
    {
        modemOff();
        return false;
    }
    else
    {

        btlog_i(TAG_GSM, "OK. Network strenght: %u", modem.getSignalQuality());
        btlog_i(TAG_GSM, "Time: %s", modem.getGSMDateTime(DATE_FULL).c_str());
    }
    return true;
}
bool TinyGsmConnectionProvider::setup()
{
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
    if (!modem.isGprsConnected())
    {
        gprsSetup();
    }
}
Client *TinyGsmConnectionProvider::getClient() const
{
    return (Client *)&gsmClient;
}
TinyGsm *TinyGsmConnectionProvider::getModem() const
{
    return (TinyGsm *)&modem;
}

void TinyGsmConnectionProvider::getInfo(JsonObject &target)
{
  JsonObject info = target.createNestedObject("GSM or NB IoT");
  float voltage = (float)modem.getBattVoltage() / 1000;
  info["Modem info"] = m_modemInfo;
  info["Battery voltage"] = voltage;
  info["Signal quality"] = modem.getSignalQuality();
}

} // namespace beegl