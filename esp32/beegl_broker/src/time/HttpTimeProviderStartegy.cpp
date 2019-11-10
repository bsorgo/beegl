#include "time/HttpTimeProviderStrategy.h"
namespace beegl
{
HttpTimeProviderStrategy::HttpTimeProviderStrategy(Settings *settings, Connection *connection) : TimeProviderStrategy(settings, connection)
{
}

HttpTimeProviderStrategy *HttpTimeProviderStrategy::createAndRegister(BeeGl *core)
{
    HttpTimeProviderStrategy *i = new HttpTimeProviderStrategy(&core->settings, &core->connection);
    core->registerTimeProviderStrategy(i);
    return i;
}

int HttpTimeProviderStrategy::getMonthFromString(char *s)
{
    if (strcmp(s, "Jan") == 0)
    {
        return 1;
    }
    if (strcmp(s, "Feb") == 0)
    {
        return 2;
    }
    if (strcmp(s, "Mar") == 0)
    {
        return 3;
    }
    if (strcmp(s, "Apr") == 0)
    {
        return 4;
    }
    if (strcmp(s, "May") == 0)
    {
        return 5;
    }
    if (strcmp(s, "Jun") == 0)
    {
        return 6;
    }
    if (strcmp(s, "Jul") == 0)
    {
        return 7;
    }
    if (strcmp(s, "Aug") == 0)
    {
        return 8;
    }
    if (strcmp(s, "Sep") == 0)
    {
        return 9;
    }
    if (strcmp(s, "Oct") == 0)
    {
        return 10;
    }
    if (strcmp(s, "Nov") == 0)
    {
        return 11;
    }
    if (strcmp(s, "Dec") == 0)
    {
        return 12;
    }

    return -1;
}

time_t HttpTimeProviderStrategy::getUTCTime()
{
    return now();
}
bool HttpTimeProviderStrategy::syncTimeFrom(TimeProviderStrategy *sourceStrategy)
{
    if (sourceStrategy != nullptr)
    {
        setUTCTime(sourceStrategy->getUTCTime());
    }
    return false;
}
bool HttpTimeProviderStrategy::syncTime()
{
    Client *client = m_connection->getClient();
    if (client != nullptr)
    {
        // GPRS || WiFi
        btlog_d(TAG_TIME, "Time and setting prefix: %s", m_settings->httpTimeAndSettingsPrefix);

        char hostname[32];
        char path[128];
        m_settings->getSettingsHostname(hostname);
        m_settings->getSettingsPath(path);

        btlog_d(TAG_TIME, "Hostname: %s", hostname);
        btlog_d(TAG_TIME, "Path: %s", path);
        btlog_d(TAG_TIME, "Username: %s, password: %s", m_settings->httpTimeAndSettingUsername, m_settings->httpTimeAndSettingPassword);
        m_connection->checkConnect();
        HttpClient httpClient = HttpClient(*m_connection->getClient(), hostname, 80);
        httpClient.connectionKeepAlive();
        httpClient.beginRequest();
        int res = httpClient.get(path);
        httpClient.sendBasicAuth(m_settings->httpTimeAndSettingUsername, m_settings->httpTimeAndSettingPassword);
        httpClient.endRequest();
        int responseCode = httpClient.responseStatusCode();
        btlog_d(TAG_TIME, "Response code from server: %u", responseCode);
        if (res == 0 && responseCode==200)
        {
            while (httpClient.headerAvailable())
            {
                String headerName = httpClient.readHeaderName();
                if (headerName.equals("Date") || headerName.equals("date"))
                {
                    String dateStr = httpClient.readHeaderValue();

                    btlog_i(TAG_TIME, "Header date string: %s", dateStr.c_str());
                    char p[32];

                    char *token;

                    dateStr.toCharArray(p, 32, 0);

                    // day of week
                    token = strtok(p, " ");

                    // day
                    token = strtok(NULL, " ");
                    int days = atoi(token);
                    // month
                    token = strtok(NULL, " ");

                    int months = getMonthFromString(token);
                    // year
                    token = strtok(NULL, " ");

                    int years = atoi(token);
                    // hour
                    token = strtok(NULL, " ");

                    token = strtok(token, ":");

                    int hours = atoi(token);
                    // minute
                    token = strtok(NULL, ":");

                    int minutes = atoi(token);
                    // seconds
                    token = strtok(NULL, ": ");
                    int seconds = atoi(token);
                    btlog_d(TAG_TIME, "Time fractions: %04d-%02d-%02dT%02d:%02d:%02d.000Z", years, months, days, hours, minutes, seconds);

                    setUTCTime(hours, minutes, seconds, days, months, years);
                    synced = true;
                    break;
                }
            }
            httpClient.stop();
            client->stop();
        }
        return true;
    }
    return false;
}
void HttpTimeProviderStrategy::setUTCTime(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t days, uint8_t months, uint8_t years)
{
    setTime(hours, minutes, seconds, days, months, years);
}

void HttpTimeProviderStrategy::setUTCTime(time_t time)
{
    setTime(time);
}

bool HttpTimeProviderStrategy::isAbsoluteTime()
{
    return synced;
}
} // namespace beegl