
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
    digitalWrite(MODEM_POWER_PIN, HIGH);
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
    modem.poweroff();
#if defined(TINY_GSM_MODEM_SIM800)
    delay(500);
    modem.sendAT(GF("+CSCLK=2"));
    modem.waitResponse(5000L);
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
    modem.sendAT(GF("+CPSMS=1"));
    modem.waitResponse(5000L);
#endif
#if defined(TINY_GSM_MODEM_SIM7020) || defined(TINY_GSM_MODEM_SIM800)
    modem.sendAT(GF("+CSCLK=2"));
    modem.waitResponse(5000L);
#endif
    btlog_d(TAG_GSM, "SUSPENDED");
}

void TinyGsmConnectionProvider::resume()
{
#if defined(TINY_GSM_MODEM_SIM7020) || defined(TINY_GSM_MODEM_SIM800)
    modemPowerup();
    modem.testAT();
    delay(200);
    modem.testAT();
    delay(200);
    modem.waitResponse(10000L);
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
    modemPowerup();
    modem.sendAT("");
    modem.waitResponse(GSM_OK);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    btlog_i(TAG_GSM, "Initializing GPRS modem");
    modem.restart();
    if (modem.testAT() != 1)
    {
        return false;
    }
    btlog_i(TAG_GSM, "Modem type: %s , IMEI: %s, ICCID: %s", modem.getModemInfo().c_str(), modem.getIMEI().c_str(), modem.getSimCCID().c_str());
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

        btlog_i(TAG_GSM, "OK. Network strenght: %u ", modem.getSignalQuality());
    }
    return true;
}
bool TinyGsmConnectionProvider::setup()
{

    serialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN, false);
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
} // namespace beegl