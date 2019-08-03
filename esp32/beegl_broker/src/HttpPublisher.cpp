/*
  HttpPublisher.cpp - Provides Temporary storage and publishes messages to central server using Http client
  
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

#include "HttpPublisher.h"

#define MESSAGE "{test:\"test\"}"
#define CONTENTTYPEJSON "application/json"
#define CONTENTTYPE "Content-Type"
#define CONTENTLENGTH "Content-Length"

void HttpPublisher::setup()
{
}

HttpPublisher::HttpPublisher(Runtime *runtime, Settings *settings, Connection *connection, Service *service) : Publisher(runtime, settings, connection, service)
{
}

bool HttpPublisher::publishMessage(const char *message)
{

  char *hostname = m_settings->getSettingsHostname();
  char *path = m_settings->getSensorPublishPath();

  blog_d("[HTTPPUBLISHER] Hostname: %s", hostname);
  blog_d("[HTTPPUBLISHER] Path: %s", path);
  blog_d("[HTTPPUBLISHER] Username: %s, password: %s", m_settings->httpTimeAndSettingUsername, m_settings->httpTimeAndSettingPassword);
  HttpClient httpClient = HttpClient(*m_connection->getClient(), hostname, 80);
  httpClient.connectionKeepAlive();
  httpClient.setHttpResponseTimeout(8000);
  httpClient.beginRequest();
  httpClient.post(path);
  httpClient.sendBasicAuth(m_settings->httpTimeAndSettingUsername, m_settings->httpTimeAndSettingPassword);
  httpClient.sendHeader(CONTENTLENGTH, strlen(message));
  httpClient.sendHeader(CONTENTTYPE, CONTENTTYPEJSON);
  httpClient.beginBody();
  httpClient.print(message);
  httpClient.endRequest();
  int responseCode = httpClient.responseStatusCode();
  httpClient.responseBody();
  httpClient.stop();
  free(hostname);
  free(path);
  if (responseCode == 200)
  {
    return true;
  }
  else
  {
     blog_e("[HTTPPUBLISHER] Error. Response code from server: %u", responseCode);
    return false;
  }
}

bool HttpPublisher::reconnect()
{
  delay(5000);
  return true;
}