#include "measurer/HX711WeightMeasureProvider.h"
namespace beegl
{
HX711WeightMeasureProvider::HX711WeightMeasureProvider(Runtime *runtime, Service *service, Settings *settings) : MeasureProvider(runtime, service, settings)
{
    m_scale = new HX711();
    m_scale->begin(SCALE_DOUT_PIN, SCALE_SCK_PIN);
    webServerBind();
}

HX711WeightMeasureProvider *HX711WeightMeasureProvider::createAndRegister(BeeGl *core)
{
    HX711WeightMeasureProvider *i = new HX711WeightMeasureProvider(&core->runtime, &core->service, &core->settings);
    core->registerMeasureProvider(i);
    return i;
}

void HX711WeightMeasureProvider::readSettings(const JsonObject &source)
{
    // scale settings

    JsonObject scaleSettings = source[STR_SCALESETTINGS];
    if (!scaleSettings[STR_SCALEFACTOR].isNull())
    {
        m_scaleFactor = scaleSettings[STR_SCALEFACTOR];
    }
    m_scaleOffset = scaleSettings[STR_SCALEOFFSET] | 0;
    strlcpy(m_scaleUnit, scaleSettings[STR_SCALEUNIT] | m_scaleUnit, 6);
}
void HX711WeightMeasureProvider::writeSettings(JsonObject &target, const JsonObject &input)
{
    JsonObject scaleSettings = target.createNestedObject(STR_SCALESETTINGS);
    scaleSettings[STR_SCALEFACTOR] = m_scaleFactor;
    scaleSettings[STR_SCALEOFFSET] = m_scaleOffset;
    scaleSettings[STR_SCALEUNIT] = m_scaleUnit;

    Settings::merge(scaleSettings, input[STR_SCALESETTINGS]);
}

void HX711WeightMeasureProvider::setup()
{
    blog_i("[HX711] Setup");
    blog_i("[HX711] Scale factor: %f", m_scaleFactor);
    blog_i("[HX711] Scale offset: %u", m_scaleOffset);
    blog_i("[HX711] Scale unit: %s", m_scaleUnit);
}

void HX711WeightMeasureProvider::measure(JsonDocument *values)
{

    blog_d("[HX711] Measure start");
    float weight = -1;
    blog_d("[HX711] Powerup");
    m_scale->power_up();
    delay(200);
    m_scale->set_scale(m_scaleFactor);
    m_scale->set_offset(m_scaleOffset);
    delay(200);

    weight = m_scale->get_units(10);
    blog_d("[MEASURER] Read weight %f ", weight);
    blog_d("[HX711] Shutdown");
    m_scale->power_down();
    JsonObject target = values->as<JsonObject>();
    JsonObject weightv = target.createNestedObject(STR_WEIGHTSENSOR);
    weightv[STR_WEIGHT] = weight;
    String unit;
    unit = m_scaleUnit;
    weightv[STR_WEIGHTUNIT] = unit;
    blog_d("[HX711] Measure end");
}

void HX711WeightMeasureProvider::webServerBind()
{

    m_service->getWebServer()->on("/rest/scale/tare", HTTP_POST,
                                  [&](AsyncWebServerRequest *request) {
                                      long tareValue = zero();

                                      String strValue = String(tareValue);
                                      request->send(200, "text/plain", strValue);
                                  });
}

long HX711WeightMeasureProvider::zero()
{
    blog_d("[HX711] Powerup");
    m_scale->power_up();
    m_scale->set_scale(m_scaleFactor);
    m_scale->set_offset(m_scaleOffset);
    blog_i("[HX711] Tare");
    m_scale->tare(10);
    long tareValue = m_scale->get_offset();
    blog_d("[HX711] Tare value: %u", tareValue);
    m_scaleOffset = m_scale->get_offset();
    blog_d("[HX711] Shutdown");
    m_scale->power_down();
    return tareValue;
}
} // namespace beegl