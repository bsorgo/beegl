#include "time/RTCTimeProviderStrategy.h"
namespace beegl
{
RTCTimeProviderStrategy::RTCTimeProviderStrategy(Settings *settings, Connection *connection) : TimeProviderStrategy(settings, connection)
{
    Wire.begin();
}

RTCTimeProviderStrategy *RTCTimeProviderStrategy::createAndRegister(BeeGl *core)
{
    RTCTimeProviderStrategy *i = new RTCTimeProviderStrategy(&core->settings, &core->connection);
    core->registerTimeProviderStrategy(i);
    return i;
}

time_t RTCTimeProviderStrategy::getUTCTime()
{
    DateTime now = RTC.now();
    ;
    return now.unixtime();
}
bool RTCTimeProviderStrategy::syncTimeFrom(TimeProviderStrategy *sourceStrategy)
{
    if (sourceStrategy != nullptr)
    {
        setUTCTime(sourceStrategy->getUTCTime());
    }
    return false;
}
bool RTCTimeProviderStrategy::syncTime()
{
    synced = true;
    return true;
}
void RTCTimeProviderStrategy::setUTCTime(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t days, uint8_t months, uint8_t years)
{
    ds3231.setClockMode(false);
    ds3231.setHour(hours);
    ds3231.setMinute(minutes);
    ds3231.setSecond(seconds);
    ds3231.setDate(days);
    ds3231.setMonth(months);
    ds3231.setYear(years);
}

void RTCTimeProviderStrategy::setUTCTime(time_t time)
{
    setUTCTime(hour(time), minute(time), second(time), day(time), month(time), year(time) - 2000);
}

bool RTCTimeProviderStrategy::isAbsoluteTime()
{
    return true;
}

} // namespace beegl
