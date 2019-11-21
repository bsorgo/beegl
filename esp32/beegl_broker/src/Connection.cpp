
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
namespace beegl
{
ConnectionProvider::ConnectionProvider(Connection *connection, Settings *settings) : ISettingsHandler(settings)
{
    m_connection = connection;
}

Connection::Connection(Service *service, Settings *settings) : ISettingsHandler(settings)
{
    m_service = service;
    m_connection[0] = new ConnectionProvider(this, settings);
    connectionSize++;
}
void Connection::readSettings(const JsonObject &source)
{
    m_inboundMode = source[STR_INBOUNDMODE];
    m_outboundMode = (source[STR_OUTBOUNDMODE] ? source[STR_OUTBOUNDMODE] : m_outboundMode);
}
void Connection::writeSettings(JsonObject &target, const JsonObject &input)
{
    target[STR_OUTBOUNDMODE] = m_outboundMode;
    target[STR_INBOUNDMODE] = m_inboundMode;
}

int Connection::registerConnectionProvider(ConnectionProvider *connection)
{
    btlog_d(TAG_CONNECTION, "Adding connection provider: %s, inbound: %u, outbound: %u", connection->getName(), connection->getInboundType(), connection->getOutboundType());
    m_connection[connectionSize] = connection;
    connectionSize++;
    return connectionSize;
}

int Connection::getOutboundConnectionProviders(ConnectionProvider **providers, char outboundTypeMask)
{
    int j = 1;
    providers[0] = this->m_connection[0];
    for (int i = 1; i < connectionSize; i++)
    {
        if ((m_connection[i]->getOutboundType() & outboundTypeMask))
        {
            providers[j] = m_connection[i];
            j++;
        }
    }
    return j;
}
int Connection::getInboundConnectionProviders(std::vector<std::pair<char, String>> &list, char inboundTypeMask)
{
    int j = 1;
    std::pair<char, String> n;
    n.first = m_connection[0]->getInboundType();
    n.second = String(m_connection[0]->getName());
    list.push_back(n);
    for (int i = 1; i < connectionSize; i++)
    {
        if (m_connection[i]->getInboundType() & inboundTypeMask)
        {
            std::pair<char, String> c;
            c.first = m_connection[i]->getInboundType();
            c.second = String(m_connection[i]->getName());
            list.push_back(c);
        }
    }
    for (int i = 1; i < connectionSize; i++)
    {
        for (int k = 1; k < connectionSize; k++)
        {
            if (m_connection[i]->getInboundType() & inboundTypeMask && m_connection[k]->getInboundType() & inboundTypeMask && m_connection[i]->getInboundType() != m_connection[k]->getInboundType() && m_connection[i]->compatibleInboundType() & m_connection[k]->getInboundType())
            {
                char mask = m_connection[k]->getInboundType() + m_connection[i]->getInboundType();
                auto found = std::find_if(list.begin(), list.end(), [&mask](const std::pair<char, String> &element) { return element.first == mask; });
                if (found == list.end())
                {
                    std::pair<char, String> c;
                    c.first = mask;
                    c.second = String(m_connection[i]->getName()) + " & " + m_connection[k]->getName();
                    list.push_back(c);
                    j++;
                }
            }
        }
    }
    return j;
}

void Connection::webServerBind()
{
    m_service->getWebServer()->on("/rest/connections/outbound", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");

        StaticJsonDocument<256> jsonBuffer;
        JsonObject root = jsonBuffer.to<JsonObject>();
        JsonArray array = root.createNestedArray("conn");

        ConnectionProvider *providers[5];
        int count = this->getOutboundConnectionProviders(providers, 0xFF);
        for (int i = 0; i < count; i++)
        {

            JsonObject proto = array.createNestedObject();
            proto["name"] = providers[i]->getName();
            proto[STR_OUTBOUNDMODE] = (int)providers[i]->getOutboundType();
            proto[STR_INBOUNDMODE] = (int)providers[i]->getInboundType();
        }
        serializeJson(jsonBuffer, *response);
        jsonBuffer.clear();
        request->send(response);
    });
    m_service->getWebServer()->on("/rest/connections/inbound", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");

        StaticJsonDocument<512> jsonBuffer;
        JsonObject root = jsonBuffer.to<JsonObject>();
        JsonArray array = root.createNestedArray("conn");

        std::vector<std::pair<char, String>> list;
        int count = this->getInboundConnectionProviders(list, 0xFF);

        for (std::vector<std::pair<char, String>>::iterator it = list.begin(); it != list.end(); ++it)
        {
            JsonObject conn = array.createNestedObject();
            conn[STR_INBOUNDMODE] = it->first;
            conn["name"] = it->second;
        }
        serializeJson(jsonBuffer, *response);
        jsonBuffer.clear();
        request->send(response);
    });
}

void Connection::suspend()
{
    for (int i = 0; i < connectionSize; i++)
    {
        ConnectionProvider *connection = m_connection[i];
        if (m_outboundMode & connection->getOutboundType())
        {
            connection->suspend();
        }
    }
}

void Connection::resume()
{
    for (int i = 0; i < connectionSize; i++)
    {
        ConnectionProvider *connection = m_connection[i];
        if (m_outboundMode & connection->getOutboundType())
        {
            connection->resume();
        }
    }
}

void Connection::shutdown()
{
    for (int i = 0; i < connectionSize; i++)
    {
        ConnectionProvider *connection = m_connection[i];
        if (m_inboundMode & connection->getInboundType() || m_outboundMode & connection->getOutboundType())
        {
            btlog_d(TAG_CONNECTION, "Shutdown connection provider: %s", connection->getName());
            connection->shutdown();
        }
    }
}

bool Connection::setup()
{
    bool ret = true;
    for (int i = 0; i < connectionSize; i++)
    {
        ConnectionProvider *connection = m_connection[i];
        if (m_inboundMode & connection->getInboundType() || m_outboundMode & connection->getOutboundType())
        {
            btlog_d(TAG_CONNECTION, "Setup connection provider: %s", connection->getName());
            if (!connection->setup())
            {
                ret = false;
            }
        }
        else
        {
            btlog_d(TAG_CONNECTION, "Shutdown connection provider: %s", connection->getName());
            connection->shutdown();
        }
    }
    webServerBind();
    if (m_inboundMode & 0x1 || m_outboundMode & 0x1)
    {
        m_service->setup();
    }
    return ret;
}

void Connection::checkConnect()
{
    for (int i = 0; i < connectionSize; i++)
    {
        ConnectionProvider *connection = m_connection[i];
        if (m_inboundMode & connection->getInboundType() || m_outboundMode & connection->getOutboundType())
        {
            connection->checkConnect();
        }
    }
}

Client *Connection::getClient()
{
    for (int i = 0; i < connectionSize; i++)
    {
        ConnectionProvider *connection = m_connection[i];
        if (m_outboundMode & connection->getOutboundType())
        {
            return connection->getClient();
        }
    }
    return nullptr;
}

void Connection::onShutdown()
{
    for (int i = 0; i < connectionSize; i++)
    {
        ConnectionProvider *connection = m_connection[i];
        connection->shutdown();
    }
}
} // namespace beegl
