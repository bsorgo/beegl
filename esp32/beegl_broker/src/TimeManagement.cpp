#include "TimeManagement.h"
namespace beegl
{
TimeManagement *TimeManagement::p_instance = NULL;
TimeProviderStrategy::TimeProviderStrategy(Settings *settings, Connection *connection) : ISettingsHandler(settings)
{
    m_connection = connection;
}

TimeManagement::TimeManagement(Service *service, Settings *settings, Connection *connection) : ISettingsHandler(settings)
{

    m_connection = connection;
    m_service = service;
    p_instance = this;

    setTimezone();

    webServerBind();
}

void TimeManagement::readSettings(const JsonObject &source)
{
    // time settings

    JsonObject timeSettings = source[STR_TIMESETTINGS];
    m_standardTimeZone = timeSettings[STR_TIMEZONE] ? timeSettings[STR_TIMEZONE] : m_standardTimeZone;
    m_summerTimeZone = timeSettings[STR_TIMESZONE] ? timeSettings[STR_TIMESZONE] : m_summerTimeZone;
    m_timeSource = (timeSettings[STR_TIMESOURCE] ? timeSettings[STR_TIMESOURCE] : m_timeSource);
    setTimezone();
}
void TimeManagement::writeSettings(JsonObject &target, const JsonObject &input)
{

    JsonObject timeSettings = target.createNestedObject(STR_TIMESETTINGS);
    timeSettings[STR_TIMESOURCE] = m_timeSource;
    timeSettings[STR_TIMESZONE] = m_summerTimeZone;
    timeSettings[STR_TIMEZONE] = m_standardTimeZone;

    Settings::merge(timeSettings, input[STR_TIMESETTINGS]);
}

bool TimeManagement::setup()
{
    for (int i = 0; i < providerCount; i++)
    {
        if (m_providers[i]->getType() == m_timeSource)
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

    m_service->getWebServer()->on("/rest/time/sources", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        const String outboundTypeStr = request->getParam(STR_OUTBOUNDMODE, false)->value();

        char outboundType = (char)outboundTypeStr.toInt();
        StaticJsonDocument<256> jsonBuffer;
        JsonObject root = jsonBuffer.to<JsonObject>();
        JsonArray array = root.createNestedArray("timesources");

        TimeProviderStrategy *strategies[5];
        int count = this->getTimeProviderStrategies(strategies, outboundType);
        for (int i = 0; i < count; i++)
        {
            JsonObject proto = array.createNestedObject();
            proto[STR_TIMESOURCE] = (int)strategies[i]->getType();
            proto["name"] = strategies[i]->getName();
        }
        serializeJson(jsonBuffer, *response);
        jsonBuffer.clear();
        request->send(response);
    });

    m_service->getWebServer()->addHandler(new AsyncCallbackJsonWebHandler("/rest/time", [this](AsyncWebServerRequest *request, JsonVariant &json) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        JsonObject jsonObj = json.as<JsonObject>();
        tmElements_t tm;
        int year = jsonObj["year"];
        tm.Year = (uint8_t)(year - 1970);
        tm.Month = jsonObj["month"];
        tm.Day = jsonObj["day"];
        tm.Hour = jsonObj["hour"];
        tm.Minute = jsonObj["minute"];
        tm.Second = jsonObj["second"];
        time_t utcTime = getTimezone()->toUTC(makeTime(tm));
        this->getSelectedTimeProviderStrategy()->setUTCTime(utcTime);
        response->setCode(200);
        request->send(response);
    }));

    m_service->getWebServer()->on("/rest/time", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        StaticJsonDocument<256> jsonBuffer;
        JsonObject root = jsonBuffer.to<JsonObject>();
        time_t t = getLocalTime();

        root["year"] = year(t);
        root["month"] = month(t);
        root["day"] = day(t);
        root["hour"] = hour(t);
        root["minute"] = minute(t);
        root["second"] = second(t);
        serializeJson(jsonBuffer, *response);
        jsonBuffer.clear();
        request->send(response);
    });
}

TimeProviderStrategy *TimeManagement::getSelectedTimeProviderStrategy()
{
    return m_selectedProvider;
}
int TimeManagement::registerTimeProviderStrategy(TimeProviderStrategy *source)
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
    return getTimezone()->toLocal(m_selectedProvider->getUTCTime());
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
    else
    {
        blog_i("[TIME] %s", getDateTimeString(getUTCTime()).c_str());
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
            if (m_providers[i]->getSupportedConnectionOutboundTypes() & outboundType)
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

String TimeManagement::getDateTimeString(time_t utc)
{
    char *buf = (char *)malloc(32);
    TimeChangeRule *tcr;
    time_t t = m_timezone->toLocal(utc, &tcr);
    char *abbrev = getStrTimezoneOffset(tcr);
    sprintf(buf, STR_TIMEFORMAT, year(t), month(t), day(t), hour(t), minute(t), second(t), abbrev);
    free(abbrev);
    String ret = String(buf);
    free(buf);
    return ret;
}

char *TimeManagement::getStrTimezoneOffset(TimeChangeRule *tcr)
{
    int hours = abs(tcr->offset) / 60;
    int minutes = abs(tcr->offset) % 60;
    char *abbrev = (char *)malloc(7);
    sprintf(abbrev, "%s%02d:%02d", (tcr->offset > 0 ? "+" : "-"), hours, minutes);
    return abbrev;
}

Timezone *TimeManagement::getTimezone()
{
    return m_timezone;
}

void TimeManagement::setTimezone()
{

    free(CEST);
    free(CET);
    CET = (TimeChangeRule *)malloc(sizeof(struct TimeChangeRule));
    CEST = (TimeChangeRule *)malloc(sizeof(struct TimeChangeRule));
    *CET = (TimeChangeRule){"", Last, Sun, Oct, 3, 480};
    *CEST = (TimeChangeRule){"", Last, Sun, Mar, 2, 540};
    CET->offset = m_standardTimeZone;
    CEST->offset = m_summerTimeZone;

    m_timezone = new Timezone(*CEST, *CET);
}
} // namespace beegl