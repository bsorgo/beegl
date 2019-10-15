
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

// set up the data structures.



Connection::Connection(Service *service, Settings *settings)
{
    m_settings = settings;
    m_service = service;
}

void Connection::addConnectionProvider(ConnectionProvider *connection)
{
    blog_d("[CONNECTION] Adding connection provider: %s, inbound: %u, outbound: %u", connection->getName(), connection->getInboundType(), connection->getOutboundType());
    m_connection[connectionSize] = connection;
    connectionSize++;
}

int Connection::getOutboundConnectionProviders(ConnectionProvider** providers,char outboundTypeMask)
{
    int j = 0;
    for(int i=0;i<connectionSize;i++)
    {
        if( (m_connection[i]->getOutboundType() & outboundTypeMask))
        {
            providers[j] = m_connection[i];
            j++;
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
        
        ConnectionProvider* providers[5];
        int count = this->getOutboundConnectionProviders(providers, 0xFF);
        for(int i=0;i<count;i++)
        {
            
            JsonObject  proto = array.createNestedObject();
            proto["name"] =  providers[i]->getName();
            proto[STR_OUTBOUNDMODE] = (int) providers[i]->getOutboundType();
            proto[STR_INBOUNDMODE] = (int) providers[i]->getInboundType();
        }
        serializeJson(jsonBuffer, *response);
        jsonBuffer.clear();
        request->send(response);
    });
}

void Connection::suspend()
{
    for(int i=0;i<connectionSize;i++)
    {
        ConnectionProvider *connection = m_connection[i];
        if(m_settings->inboundMode & connection->getInboundType() || m_settings->outboundMode & connection->getOutboundType())
        {
            connection->suspend();
        }
    }
}

void Connection::resume()
{
    for(int i=0;i<connectionSize;i++)
    {
        ConnectionProvider *connection = m_connection[i];
        if(m_settings->inboundMode & connection->getInboundType() || m_settings->outboundMode & connection->getOutboundType())
        {
            connection->suspend();
        }
    }
}

void Connection::shutdown()
{
    for(int i=0;i<connectionSize;i++)
    {
        ConnectionProvider *connection = m_connection[i];
        if(m_settings->inboundMode & connection->getInboundType() || m_settings->outboundMode & connection->getOutboundType())
        {
            blog_d("[CONNECTION] Shutdown connection provider: %s", connection->getName());
            connection->shutdown();
        }
    }
}

bool Connection::setup()
{
    bool ret = true;
    for(int i=0;i<connectionSize;i++)
    {
        ConnectionProvider *connection = m_connection[i];
        if(m_settings->inboundMode & connection->getInboundType() || m_settings->outboundMode & connection->getOutboundType())
        {
            blog_d("[CONNECTION] Setup connection provider: %s", connection->getName());
            if(!connection->setup())
            {
                ret = false;
            }
        }
        else
        {
            blog_d("[CONNECTION] Shutdown connection provider: %s", connection->getName());
            connection->shutdown();
        }
    }
    webServerBind();
    return ret;
}

void Connection::checkConnect()
{
    for(int i=0;i<connectionSize;i++)
    {
        ConnectionProvider *connection = m_connection[i];
        if(m_settings->inboundMode & connection->getInboundType() || m_settings->outboundMode & connection->getOutboundType())
        {
            connection->checkConnect();
        }
    }
}

Client *Connection::getClient()
{
    for(int i=0;i<connectionSize;i++)
    {
        ConnectionProvider *connection = m_connection[i];
        if(m_settings->inboundMode & connection->getInboundType() || m_settings->outboundMode & connection->getOutboundType())
        {
            return connection->getClient();
        }
    }
    return nullptr;
}
