#include "measurer/DHT22TempAndHumidityMeasureProvider.h"
namespace beegl
{
DHT22TempAndHumidityMeasureProvider::DHT22TempAndHumidityMeasureProvider(
    Runtime *runtime, Service *service, Settings *settings) : MeasureProvider(runtime, service, settings)
{
}

DHT22TempAndHumidityMeasureProvider *DHT22TempAndHumidityMeasureProvider::createAndRegister(BeeGl *core)
{
    DHT22TempAndHumidityMeasureProvider *i = new DHT22TempAndHumidityMeasureProvider(&core->runtime, &core->service, &core->settings);
    core->registerMeasureProvider(i);
    return i;
}

void DHT22TempAndHumidityMeasureProvider::setup()
{
    btlog_i(TAG_DHT, "Setup, pin: %u", m_pin);
    m_dht.setup(m_pin, DHTesp::DHT22);
}

void DHT22TempAndHumidityMeasureProvider::measure(JsonDocument *values)
{
    btlog_d(TAG_DHT, "Measure start");
    TempAndHumidity sensorData = m_dht.getTempAndHumidity();
    JsonObject root = values->as<JsonObject>();

    JsonObject humidity = root.createNestedObject(STR_HUMIDITYSENSOR);
    humidity[STR_HUMIDITY] = sensorData.humidity;
    humidity[STR_HUMIDITYUNIT] = String(STR_HUMIDITYUNITPERCENT);

    JsonObject temp = root.createNestedObject(STR_TEMPSENSOR);
    temp[STR_TEMP] = sensorData.temperature;
    temp[STR_TEMPUNIT] = String(STR_TEMPUNITC);

    btlog_d(TAG_DHT, "Measure end");
}
} // namespace beegl