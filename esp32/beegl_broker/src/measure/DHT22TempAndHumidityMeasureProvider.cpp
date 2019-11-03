#include "measurer/DHT22TempAndHumidityMeasureProvider.h"
namespace beegl
{
DHT22TempAndHumidityMeasureProvider::DHT22TempAndHumidityMeasureProvider(
    Runtime *runtime, Service *service, Settings *settings) : MeasureProvider(runtime, service, settings)
{
    m_dht = DHTesp();
}

DHT22TempAndHumidityMeasureProvider *DHT22TempAndHumidityMeasureProvider::createAndRegister(BeeGl *core)
{
    DHT22TempAndHumidityMeasureProvider *i = new DHT22TempAndHumidityMeasureProvider(&core->runtime, &core->service, &core->settings);
    core->registerMeasureProvider(i);
    return i;
}

void DHT22TempAndHumidityMeasureProvider::setup()
{
    blog_i("[DHT] Setup, pin: %u", m_pin);
    m_dht.setup(m_pin, DHTesp::DHT22);
}

void DHT22TempAndHumidityMeasureProvider::measure(JsonDocument *values)
{
    blog_d("[DHT] Measure start");
    TempAndHumidity sensorData = m_dht.getTempAndHumidity();
    JsonObject root = values->as<JsonObject>();

    JsonObject humidity = root.createNestedObject(STR_HUMIDITYSENSOR);
    humidity[STR_HUMIDITY] = sensorData.humidity;
    humidity[STR_HUMIDITYUNIT] = String(STR_HUMIDITYUNITPERCENT);

    JsonObject temp = root.createNestedObject(STR_TEMPSENSOR);
    temp[STR_TEMP] = sensorData.temperature;
    temp[STR_TEMPUNIT] = String(STR_TEMPUNITC);

    blog_d("[DHT] Measure end");
}
} // namespace beegl