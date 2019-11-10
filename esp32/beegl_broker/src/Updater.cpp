/*
  Updater.cpp - provides firmware & resources update functions

  - Explicit firmware update through web interface
  - Implicit firmware update from server
  - Implicit resource update (e.g. index.html) from server based on MD5 
  
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

#include "Updater.h"
namespace beegl
{
Updater::Updater(Runtime *runtime, Service *service, Settings *settings, Connection *connection) : ISettingsHandler(settings)
{
  m_server = service;
  m_connection = connection;
  m_runtime = runtime;
  webServerBind();
}

void Updater::webServerBind()
{
  m_server->getWebServer()->on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200);
    ESP.restart(); }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

    if(!index){
      btlog_i(TAG_UPDATER, "UploadStart: %s\n", filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { 
        Update.printError(Serial);
      }
    }
    if(Update.write(data, len)!=len)
    {
      Update.printError(Serial);
    }
    if(final) {
    btlog_i(TAG_UPDATER, "BodyEnd: %u B\n", index+len);
     if(Update.end(true))
     {
        btlog_i(TAG_UPDATER, "Update Success: %u\nRebooting...\n", index+len);
        ESP.restart();
     }
    } });
}

void Updater::checkFirmware()
{

  SchEntryType schEntry = m_runtime->getCurrentSchedulerEntry();

  // is this operational time entry for updates?
  if (m_connection->getClient() != nullptr && schEntry.updateFromServer)
  {
    // firmware
    if (strcmp(m_runtime->FIRMWAREVERSION, m_settings->firmwareVersion) < 0)
    {
      char hostname[32];
      char path[128];
      m_settings->getSettingsHostname(hostname);
      getFirmwarePath(path);
      strcat(path, SYSTEM_VARIANT);
      strcat(path, "_");
      strcat(path, m_settings->firmwareVersion);
      strcat(path, ".bin");
      btlog_i(TAG_UPDATER, "Firmware hostname: %s", hostname);
      btlog_i(TAG_UPDATER, "Firmware file path: %s", path);
      m_connection->checkConnect();
      HttpClient httpClient = HttpClient(*m_connection->getClient(), hostname, 80);
      downloadFirmware(&httpClient, path);
      httpClient.stop();
    }
  }
}

String Updater::getLocalFileMd5(const char *filename)
{
  char localFilename[32];
  strcpy(localFilename, "/");
  strcat(localFilename, filename);
  File file = FILESYSTEM.open(localFilename, FILE_READ);

  if (!file)
  {
    btlog_e(TAG_UPDATER, "Error. File %s not found.", localFilename);
    ;
    return "0";
  }
  MD5Builder md5;
  md5.begin();
  md5.addStream(file, 50000);
  md5.calculate();
  String md5str = md5.toString();
  btlog_i(TAG_UPDATER, "Local file:%s MD5:%s", filename, md5str.c_str());
  file.close();
  return md5str;
}

String Updater::getServerFileMd5(const char *filename)
{

  char hostname[32];
  char path[128];
  m_settings->getSettingsHostname(hostname);
  getFirmwarePath(path);
  strcat(path, filename);
  strcat(path, ".md5");
  m_connection->checkConnect();
  HttpClient httpClient = HttpClient(*m_connection->getClient(), hostname, 80);
  int res = httpClient.get(path);
  int responseCode = httpClient.responseStatusCode();
  if (res == 0 && responseCode == 200)
  {
    httpClient.skipResponseHeaders();
    String md5str = httpClient.readString();
    btlog_i(TAG_UPDATER, "Server file:%s MD5:%s", path, md5str.c_str());
    return md5str;
  }
  else if (res == 0 && responseCode == 404)
  {
    btlog_i(TAG_UPDATER, "No server file on path:%s", path);
    return "";
  }
  else
  {
    btlog_e(TAG_UPDATER, "Error obtaining md5 file :%s. Response code:%u", path, responseCode);
    return "";
  }
}

void Updater::downloadFirmware(HttpClient *httpClient, char *filePath)
{
  int err = 0;
  httpClient->connectionKeepAlive();
  err = httpClient->get(filePath);
  if (err == 0 && httpClient->responseStatusCode() == 200)
  {
    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
    {
      Update.printError(Serial);
      return;
    }
    btlog_i(TAG_UPDATER, "Download start: %s\n", filePath);
    int len = httpClient->contentLength();
    btlog_d(TAG_UPDATER, "Found file size: %u ", len);
    uint8_t buff[1024] = {0};
    while (httpClient->connected() && (len > 0 || len == -1))
    {

      size_t size = httpClient->available();

      size_t c = httpClient->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
      if (Update.write(buff, c) != c)
      {
        Update.printError(Serial);
        return;
      }
      if (len > 0)
      {
        len -= c;
      }
      btlog_d(TAG_UPDATER, "Downloading .. left %u ", len);
      delay(2);
    }
    delay(100);
    if (Update.end(true))
    {
      btlog_i(TAG_UPDATER, "Update success: %u. \nRebooting...\n", len);
      m_runtime->setSafeModeOnRestart(0);
      ESP.restart();
    }
    else
    {
      Update.printError(Serial);
      return;
    }
  }
}

bool Updater::checkDownloadFile(const char *filename)
{
  String serverMd5 = getServerFileMd5(filename);
  String localMd5 = getLocalFileMd5(filename);
  serverMd5.trim();
  localMd5.trim();
  if (serverMd5.compareTo(localMd5) != 0)
  {
    return downloadFile(filename);
  }
  return true;
}
bool Updater::downloadFile(const char *filename)
{
  char hostname[32];
  char path[128];
  m_settings->getSettingsHostname(hostname);
  getFirmwarePath(path);
  strcat(path, filename);
  int result = downloadFile(hostname, path, filename);

  return result;
}
bool Updater::downloadFile(const char *hostname, const char *path, const char *filename)
{
  int err = 0;
  char localFilename[32];
  strcpy(localFilename, "/");
  strcat(localFilename, filename);
  m_connection->checkConnect();

  HttpClient httpClient = HttpClient(*m_connection->getClient(), hostname, 80);
  httpClient.connectionKeepAlive();
  err = httpClient.get(path);
  if (err == 0 && httpClient.responseStatusCode() == 200)
  {
    File f = FILESYSTEM.open(localFilename, FILE_WRITE);
    if (!f)
    {

      return false;
    }
    btlog_i(TAG_UPDATER, "Download start: %s\n", path);
    int len = httpClient.contentLength();
    btlog_d(TAG_UPDATER, "File size: %u ", len);
    uint8_t buff[512] = {0};
    while (httpClient.connected() && (len > 0 || len == -1))
    {

      size_t size = httpClient.available();

      size_t c = httpClient.readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
      f.write(buff, c);
      if (len > 0)
      {
        len -= c;
      }
      btlog_d(TAG_UPDATER, "Downloading .. left %u ", len);
      delay(2);
    }
    f.close();
  }
  else
  {
    return false;
  }
  return true;
}

void Updater::getFirmwarePath(char *buffer)
{

  Settings::getPath(buffer, m_settings->httpTimeAndSettingsPrefix);
  strcat(buffer, "fws/");
}
} // namespace beegl