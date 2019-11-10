

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
  m_webserver.onNotFound(notFound);
}
void Service::setup()
{
  webServerSetup();
}

AsyncWebServer *Service::getWebServer() const
{
  return (AsyncWebServer*) &m_webserver;
}

void Service::webServerSetup()
{
  
  btlog_i(TAG_SERVICE, "Starting web server");
  m_webserver.begin();
  btlog_i(TAG_SERVICE, "Web server started");
}

} // namespace beegl
