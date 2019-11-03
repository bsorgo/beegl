

#include "Service.h"
#include "Settings.h"
namespace beegl
{
class Settings;

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

Service::Service(Settings *settings) : ISettingsHandler(settings)
{
  m_webserver = new AsyncWebServer(80);
  m_webserver->onNotFound(notFound);
}
void Service::setup()
{
  webServerSetup();
}

AsyncWebServer *Service::getWebServer()
{
  return m_webserver;
}

void Service::webServerSetup()
{
  blog_i("[WEB Server] Starting web server");
  m_webserver->begin();
  blog_i("[WEB Server] Web server started");
}

} // namespace beegl
