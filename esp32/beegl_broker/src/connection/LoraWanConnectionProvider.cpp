
/*
  LoraWanConnectionProvider.cpp - Lora Wan connection implementation
  
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

#include "connection/LoraWanConnectionProvider.h"

namespace beegl
{
void from_hex_char(uint8_t *dest, const char *source, const size_t size, bool lsb)
{
    for (int i = lsb ? (size / 2) - 1 : 0, j = 0; j < size; lsb ? --i : ++i, j += 2)
    {
        int val[1];
        sscanf(source + j, "%2x", val);
        dest[i] = val[0];
    }
}

// this method is called when the LMIC needs OTAA info.
// return false to indicate "no provisioning", otherwise
// fill in the data and return true.
bool MyLoRaWAN::GetOtaaProvisioningInfo(OtaaProvisioningInfo *pInfo)
{
    if (pInfo)
    {
        memcpy(pInfo->AppEUI, m_provisioningInfo->AppEUI, sizeof(m_provisioningInfo->AppEUI));
        memcpy(pInfo->DevEUI, m_provisioningInfo->DevEUI, sizeof(m_provisioningInfo->DevEUI));
        memcpy(pInfo->AppKey, m_provisioningInfo->AppKey, sizeof(m_provisioningInfo->AppKey));
    }
    return true;
}

void MyLoRaWAN::NetSaveFCntDown(uint32_t uFCntDown)
{
    // save uFcntDown somwwhere
}

void MyLoRaWAN::NetSaveFCntUp(uint32_t uFCntUp)
{
    // save uFCntUp somewhere
}

void MyLoRaWAN::NetSaveSessionInfo(
    const SessionInfo &Info,
    const uint8_t *pExtraInfo,
    size_t nExtraInfo)
{
    // write to log
    blog_i("[LORAWAN] Session info. Country: %u, NetID: %u, FCntUp: %u, FCntDown:", Info.V2.Country, Info.V2.NetID, Info.V2.FCntUp, Info.V2.FCntDown);
}

// set up the data structures.

LoraWanConnectionProvider::LoraWanConnectionProvider(Connection *connection, Settings *settings) : ConnectionProvider(connection, settings)
{
}

LoraWanConnectionProvider *LoraWanConnectionProvider::createAndRegister(BeeGl *core)
{
    LoraWanConnectionProvider *i = new LoraWanConnectionProvider(&core->connection, &core->settings);
    core->registerConnectionProvider(i);
    return i;
}

void LoraWanConnectionProvider::readSettings(const JsonObject &source)
{
    //lorawan settings

    JsonObject loraSettings = source[STR_LORASETTINGS];
    strlcpy(m_loraAppEUI, loraSettings[STR_LORAAPPEUI] | m_loraAppEUI, 17);
    strlcpy(m_loraDeviceEUI, loraSettings[STR_LORADEVEUI] | m_loraDeviceEUI, 17);
    strlcpy(m_loraAppKey, loraSettings[STR_LORAAPPKEY] | m_loraAppKey, 33);
}
void LoraWanConnectionProvider::writeSettings(JsonObject &target, const JsonObject &input)
{
    JsonObject loraSettings = target.createNestedObject(STR_LORASETTINGS);
    loraSettings[STR_LORAAPPEUI] = m_loraAppEUI;
    loraSettings[STR_LORADEVEUI] = m_loraDeviceEUI;
    loraSettings[STR_LORAAPPKEY] = m_loraAppKey;

    Settings::merge(loraSettings, input[STR_LORASETTINGS]);
}

void LoraWanConnectionProvider::suspend()
{
    // not supported
}

void LoraWanConnectionProvider::resume()
{
    // not supported
}

void LoraWanConnectionProvider::shutdown()
{
    //loraWan.Shutdown();
}

bool LoraWanConnectionProvider::setup()
{
    blog_i("[LORAWAN] Begin");
    blog_i("[LORAWAN] App EUI: %s", m_loraAppEUI);
    blog_i("[LORAWAN] Dev EUI: %s", m_loraDeviceEUI);
    blog_d("[LORAWAN] App Key: %s", m_loraAppKey);
    from_hex_char(m_provisioningInfo.AppKey, m_loraAppKey, 32, false);
    from_hex_char(m_provisioningInfo.DevEUI, m_loraDeviceEUI, 16, true);
    from_hex_char(m_provisioningInfo.AppEUI, m_loraAppEUI, 16, true);
    loraWan.setProvisioningInfo(&m_provisioningInfo);
    loraWan.begin(loraDevicePinMap);
    loraWan.SetLinkCheckMode(0);
    blog_i("[LORAWAN] Is provisioned: %s", loraWan.IsProvisioned() ? "Yes" : "No");
    return true;
}

void LoraWanConnectionProvider::checkConnect()
{
    // not supported
}

Client *LoraWanConnectionProvider::getClient()
{
    // client not supported
    return NULL;
}
} // namespace beegl