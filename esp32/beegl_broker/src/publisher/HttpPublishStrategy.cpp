/*
  HttpPublishStrategy.cpp - Provides Temporary storage and publishes messages to central server using Http client
  
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

#include "publisher/HttpPublishStrategy.h"
namespace beegl
{
void HttpPublishStrategy::setup() {}

void HttpPublishStrategy::update() {}

HttpPublishStrategy::HttpPublishStrategy(Runtime *runtime, Settings *settings, Connection *connection, Service *service) : PublishStrategy(runtime, settings, connection, service) {}

HttpPublishStrategy *HttpPublishStrategy::createAndRegister(BeeGl *core)
{
  HttpPublishStrategy *i = new HttpPublishStrategy(&core->runtime, &core->settings, &core->connection, &core->service);
  core->registerPublishStrategy(i);
  return i;
}

bool HttpPublishStrategy::publishMessage(JsonDocument *payload)
{
  char message[MESSAGE_SIZE];
  m_serializer.serialize(payload, message);

  char *hostname = ISettingsHandler::m_settings->getSettingsHostname();
  char *path = getSensorPublishPath();

  blog_d("[HTTPPUBLISHER] Hostname: %s", hostname);
  blog_d("[HTTPPUBLISHER] Path: %s", path);
  blog_d("[HTTPPUBLISHER] Username: %s, password: %s", ISettingsHandler::m_settings->httpTimeAndSettingUsername, ISettingsHandler::m_settings->httpTimeAndSettingPassword);
  HttpClient httpClient = HttpClient(*PublishStrategy::m_connection->getClient(), hostname, 80);
  httpClient.connectionKeepAlive();
  httpClient.setHttpResponseTimeout(8000);
  httpClient.beginRequest();
  httpClient.post(path);
  httpClient.sendBasicAuth(ISettingsHandler::m_settings->httpTimeAndSettingUsername, ISettingsHandler::m_settings->httpTimeAndSettingPassword);
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

bool HttpPublishStrategy::reconnect()
{
  delay(5000);
  return true;
}

char *HttpPublishStrategy::getSensorPublishPath() const
{

  char *path = Settings::getPath(m_settings->httpTimeAndSettingsPrefix);
  strcat(path, "v1/measurements");
  return path;
}
} // namespace beegl