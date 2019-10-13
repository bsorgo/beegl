#include "TimeManagement.h"

TimeManagement *TimeManagement::p_instance = NULL;
TimeProviderStrategy::TimeProviderStrategy(Settings *settings, Connection *connection)
{
    m_settings = settings;
    m_connection = connection;
}

TimeManagement::TimeManagement(Service * service, Settings *settings, Connection *connection)
{
    m_settings = settings;
    m_connection = connection;
    m_service = service;
    p_instance = this;
    webServerBind();
}

bool TimeManagement::setup()
{
    for (int i = 0; i < providerCount; i++)
    {
        if (m_providers[i]->getType() == m_settings->timeSource)
        {
            m_selectedProvider = m_providers[i];
            blog_i("[TIME] Using time provider: %s", m_selectedProvider->getName());
            return true;
        }
    }
    m_selectedProvider = new TimeProviderStrategy(m_settings, m_connection);
    blog_i("[TIME] Using default time provider %s", m_selectedProvider->getName());
    return false;
}

void TimeManagement::webServerBind()
{

    m_service->getWebServer()->on("/rest/timesources", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        const String outboundTypeStr = request->getParam(STR_OUTBOUNDMODE, false)->value();
        
        char outboundType = (char)outboundTypeStr.toInt();        
        StaticJsonBuffer<256> jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        JsonArray &array = root.createNestedArray("timesources");
        
        TimeProviderStrategy* strategies[5];
        int count = this->getTimeProviderStrategies(strategies, outboundType);
        for(int i=0;i<count;i++)
        {
            JsonObject & proto = array.createNestedObject();
            proto[STR_TIMESOURCE] =  (int) strategies[i]->getType();
            proto["name"] = strategies[i]->getName();
        }
        root.printTo(*response);
        jsonBuffer.clear();
        request->send(response);
    });
}


TimeProviderStrategy *TimeManagement::getSelectedTimeProviderStrategy()
{
    return m_selectedProvider;
}
int TimeManagement::addTimeProviderStrategy(TimeProviderStrategy *source)
{
    m_providers[providerCount] = source;
    return providerCount++;
    
}
time_t TimeManagement::getUTCTime()
{
    return m_selectedProvider->getUTCTime();
}
time_t TimeManagement::getLocalTime()
{
    return m_settings->getTimezone()->toLocal(m_selectedProvider->getUTCTime());
}
bool TimeManagement::syncTime(char sourceType)
{
    TimeProviderStrategy *sourceProvider = nullptr;
    if (sourceType != 0x00 && m_selectedProvider != nullptr && m_selectedProvider->getSupportedSyncProviders() & sourceType)
    {
        for (int i = 0; i < providerCount; i++)
        {
            if (m_providers[i]->getSupportedSyncProviders() & sourceType)
            {

                sourceProvider = m_providers[i];
                blog_i("[TIME] Using time source provider: %s for sync.", sourceProvider->getName());
                break;
            }
        }
    }
    bool res = false;
    if (sourceProvider != nullptr)
    {
        res = m_selectedProvider->syncTimeFrom(sourceProvider);
    }
    else
    {
        res = m_selectedProvider->syncTime();
    }
    if (!res)
    {
        blog_e("[TIME] Can't sync time. Check settings, connection, ...");
    }
    return res;
}
int TimeManagement::getTimeProviderStrategies(TimeProviderStrategy **providers, char outboundType)
{
    int j = 0;
    if (providers != nullptr)
    {
        for (int i = 0; i < providerCount; i++)
        {
            if(m_providers[i]->getSupportedConnectionOutboundTypes() & outboundType)
            {
                providers[j] = m_providers[i];
                j++;
            }
        }
    }
    return j;
}
bool TimeManagement::isAbsoluteTime()
{
    if (m_selectedProvider != nullptr)
    {
        return m_selectedProvider->isAbsoluteTime();
    }
    else
    {
        return false;
    }
}
TimeManagement *TimeManagement::getInstance()
{
    return p_instance;
}