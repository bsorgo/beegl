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


Updater::Updater(Runtime *runtime, Service *service, Settings *settings, Connection *connection)
{
  m_server = service;
  m_settings = settings;
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
      Serial.printf("[UPDATER] UploadStart: %s\n", filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { 
        Update.printError(Serial);
      }
    }
    if(Update.write(data, len)!=len)
    {
      Update.printError(Serial);
    }
    if(final) {
    Serial.printf("[UPDATER] BodyEnd: %u B\n", index+len);
     if(Update.end(true))
     {
        Serial.printf("[UPDATER] Update Success: %u\nRebooting...\n", index+len);
        ESP.restart();
     }
    } });
}

void Updater::checkFirmware()
{

  SchEntryType schEntry = m_settings->getCurrentSchedulerEntry();
  
  // is this operational time entry for updates?
  if(m_connection->getClient()!=nullptr && schEntry.updateFromServer) {
     // firmware
    if (strcmp(m_runtime->FIRMWAREVERSION, m_settings->firmwareVersion) < 0)
    {
      char *hostname = m_settings->getSettingsHostname();
      char *path = m_settings->getFirmwarePath();
      strcat(path, SYSTEM_VARIANT);
      strcat(path, "_");
      strcat(path, m_settings->firmwareVersion);
      strcat(path, ".bin");
      blog_i("[UPDATER] Firmware hostname: %s", hostname);
      blog_i("[UPDATER] Firmware file path: %s", path);
      m_connection->checkConnect();
      HttpClient httpClient = HttpClient(*m_connection->getClient(), hostname, 80);
      downloadFirmware(&httpClient, path);
      httpClient.stop();
      free(hostname);
      free(path);
    }
  }
}

String Updater::getLocalFileMd5(const char *filename)
{
  char *localFilename = (char *)malloc(sizeof(char) * 32);
  strcpy(localFilename, "/");
  strcat(localFilename, filename);
  File file = FILESYSTEM.open(localFilename, FILE_READ);

  if (!file)
  {
    blog_e("[UPDATER] Error. File %s not found.", localFilename);;
    return "0";
  }
  MD5Builder md5;
  md5.begin();
  md5.addStream(file, 50000);
  md5.calculate();
  String md5str = md5.toString();
  blog_i("[UPDATER] Local file:%s MD5:%s", filename, md5str.c_str());
  file.close();
  free(localFilename);
  return md5str;
}

String Updater::getServerFileMd5(const char *filename)
{

  char *hostname = m_settings->getSettingsHostname();
  char *path = m_settings->getFirmwarePath();
  strcat(path, filename);
  strcat(path, ".md5");
  m_connection->checkConnect();
  HttpClient httpClient = HttpClient(*m_connection->getClient(), hostname, 80);
  int res = httpClient.get(path);
  int responseCode = httpClient.responseStatusCode();
  if (res == 0 && responseCode == 200)
  {
    httpClient.skipResponseHeaders();
    String md5str =  httpClient.readString();
    blog_i("[UPDATER] Server file:%s MD5:%s", path, md5str.c_str());
    return md5str;
  }
  else if (res == 0 && responseCode == 404)
  {
    blog_i("[UPDATER] No server file on path:%s", path);
    return "";
  } else {
    blog_e("[UPDATER] Error obtaining md5 file :%s. Response code:%u", path, responseCode);
    return "";
  }
  free(hostname);
  free(path);
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
    blog_i("[UPDATER] Download start: %s\n", filePath);
    int len = httpClient->contentLength();
    blog_d("[UPDATER] Found file size: %u ", len);
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
      blog_d("[UPDATER] Downloading .. left %u ", len);
      delay(2);
    }
    delay(100);
    if (Update.end(true))
    {
      blog_i("[UPDATER] Update success: %u. \nRebooting...\n", len);
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
  if (serverMd5.compareTo(localMd5)!=0)
  {
    return downloadFile(filename);
  }
  return true;
}
bool Updater::downloadFile(const char *filename)
{
  char *hostname = m_settings->getSettingsHostname();
  char *path = m_settings->getFirmwarePath();
  strcat(path, filename);
  int result = downloadFile(hostname, path, filename);
  free(hostname);
  free(path);
  return result;
}
bool Updater::downloadFile(const char *hostname, const char *path, const char *filename)
{
  int err = 0;
   char *localFilename = (char *)malloc(sizeof(char) * 32);
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
      free(localFilename);
      return false;
    }
    blog_i("[UPDATER] Download start: %s\n", path);
    int len = httpClient.contentLength();
    blog_d("[UPDATER] File size: %u ", len);
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
      blog_d("[UPDATER] Downloading .. left %u ", len);
      delay(2);
    }
    f.close();
  }
  else
  {
    free(localFilename);
    return false;
  }
  free(localFilename);
  return true;
}