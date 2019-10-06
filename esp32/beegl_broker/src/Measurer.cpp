/*
  Measurer.cpp - Performs scale measures like weight, temp and humidity
  
  This file is part of the BeeGl distribution (https://github.com/bsorgo/beegl).
  Copyright (c) 2019 Bostjan Sorgo
  
  This program is free software: you can redistribute it and/or modify  
  it under the terms of the GNU General Public License as published by  
  the Free Software Foundation, version 3.
 
  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
  General Public License for more details.
 
  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

#include "Measurer.h"



Measurer::Measurer(Runtime *runtime, Service *server, Settings *settings, Publisher *publisher)
{

    m_server = server;
    m_settings = settings;
    m_publisher = publisher;
    m_runtime = runtime;

    m_scale = new HX711();
    m_scale->begin(SCALE_DOUT_PIN, SCALE_SCK_PIN);
    m_dht = new DHTesp();

    webServerBind();
}

void Measurer::webServerBind()
{
    m_server->getWebServer()->on("/rest/measure", HTTP_GET, [&](AsyncWebServerRequest *request) {
        if(request->hasParam("scaleFactor")) m_settings->scaleFactor = atof(request->getParam("scaleFactor", false,false)->value().c_str());
        if(request->hasParam("scaleOffset")) m_settings->scaleOffset = atoi(request->getParam("scaleOffset", false,false)->value().c_str());
        blog_d("Scale factor: %f, offset: %u", m_settings->scaleFactor, m_settings->scaleOffset);
        char *message = measure();
        if (message)
        {
            blog_d("Payload: %s", message);
            request->send(200, "text/plain", message);
        }
        else
        {
            request->send(500);
        }
    });

    m_server->getWebServer()->on("/rest/scale/tare", HTTP_POST,
                                 [&](AsyncWebServerRequest *request) {
                                     long tareValue = zero();
                                    
                                     String strValue = String(tareValue);
                                     request->send(200, "text/plain", strValue);
                                 });
}

uint32_t Measurer::getMeasureInterval() 
{
    return m_settings->refreshInterval;
}

bool Measurer::scaleSetup()
{
    if (m_settings->measureWeight)
    {
        blog_i("[HX711] Setup");
        blog_i("[HX711] Scale factor: %f", m_settings->scaleFactor);
        blog_i("[HX711] Scale offset: %u", m_settings->scaleOffset);
        blog_i("[HX711] Scale unit: %s", m_settings->scaleUnit);
    }
    return true;
}

bool Measurer::dhtSetup()
{
    if (m_settings->measureTempAndHumidity)
    {
        blog_i("[DHT] Setup");
        m_dht->setup(DHT_PIN, DHTesp::DHT22);
    }
    return true;
}

bool Measurer::setup()
{
    if (!scaleSetup() || !dhtSetup())
    {
        return false;
    }
    return true;
}

long Measurer::zero()
{
    blog_d("[HX711] Powerup");
    m_scale->power_up();
    m_scale->set_scale(m_settings->scaleFactor);
    m_scale->set_offset(m_settings->scaleOffset);
    blog_i("[HX711] Tare");
    m_scale->tare(10);
    long tareValue = m_scale->get_offset();
    blog_d("[HX711] Tare value: %u", tareValue);
    m_settings->scaleOffset = m_scale->get_offset();
    blog_d("[HX711] Shutdown");
    m_scale->power_down();
    return tareValue;
}

char *Measurer::measure()
{

    MeasureData data;

    if (m_settings->measureWeight)
    {
        blog_d("[HX711] Powerup");
        m_scale->power_up();
        delay(200);
        m_scale->set_scale(m_settings->scaleFactor);
        m_scale->set_offset(m_settings->scaleOffset);
        delay(200);
        data.weight = -1;
        data.weight = m_scale->get_units(10);
        blog_d("[MEASURER] Read weight %f ", data.weight);
        blog_d("[HX711] Shutdown");
        m_scale->power_down();
    }
    if (m_settings->measureTempAndHumidity)
    {
        TempAndHumidity sensorData = m_dht->getTempAndHumidity();
        data.temp = sensorData.temperature;
        data.humidity = sensorData.humidity;
        blog_d("[MEASURER] Read temperature and humidity: %.2f C %.2f %% ", data.temp, data.humidity);
    }
    char *message = storeMessage(data);

    return message;
}

char *Measurer::storeMessage(MeasureData measureData)
{
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root[STR_DEVICEID] = m_settings->deviceName;
    //root[STR_TIME] = m_settings->getDateTimeString(now());
    root[STR_EPOCHTIME] =  static_cast<long int> (now());
    root[STR_VER] = m_runtime->FIRMWAREVERSION;
    if (m_settings->measureWeight && measureData.weight == measureData.weight)
    {

        JsonObject &weightSensor = root.createNestedObject(STR_WEIGHTSENSOR);
        weightSensor[STR_WEIGHT] = measureData.weight;
        weightSensor[STR_WEIGHTUNIT] = STR_WEIGHTUNITG;
    }

    if (m_settings->measureTempAndHumidity && measureData.temp == measureData.temp)
    {
        JsonObject &tempSensor = root.createNestedObject(STR_TEMPSENSOR);
        tempSensor[STR_TEMP] = measureData.temp;
        tempSensor[STR_TEMPUNIT] = STR_TEMPUNITC;

        JsonObject &humiditySensor = root.createNestedObject(STR_HUMIDITYSENSOR);
        humiditySensor[STR_HUMIDITY] = measureData.humidity;
        humiditySensor[STR_HUMIDITYUNIT] = STR_HUMIDITYUNITPERCENT;
    }
    return m_publisher->storeMessage(root);
}

void Measurer::measureLoop( void * pvParameters )
{
   Measurer* measurer = (Measurer*) pvParameters;
   for( ;; )
   {
        delay(measurer->getMeasureInterval());
        measurer->measure();
   }
}



void Measurer::begin() 
{
   // Create the task, storing the handle.  Note that the passed parameter ucParameterToPass
   // must exist for the lifetime of the task, so in this case is declared static.  If it was just an
   // an automatic stack variable it might no longer exist, or at least have been corrupted, by the time
   // the new task attempts to access it.
    blog_d("[MEASURER] Creating measurer task.");
    xTaskCreate(measureLoop, MEASURER_TASK, 4096, this, tskIDLE_PRIORITY, NULL );
}


