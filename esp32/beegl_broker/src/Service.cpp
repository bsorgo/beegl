

#include "Service.h"
#include "Settings.h"
class Settings;

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

Service::Service(Settings* settings) 
{
    m_settings = settings;
    m_webserver = new AsyncWebServer(80);
    m_webserver->onNotFound(notFound);
    
}
void Service::setup() {
  webServerSetup();
}




AsyncWebServer* Service::getWebServer() {
    return m_webserver;
}

void Service::webServerSetup()
{
  
  if(m_settings->inboundMode & 0x1 || m_settings->outboundMode & 0x1) {
    log_i("[WEB Server] Starting web server");
    m_webserver->begin();
    log_i("[WEB Server] Web server started");
  }
  
  
}
