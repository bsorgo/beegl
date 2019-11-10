/*
  BeeGl.cpp - Beegl Core wrapper
  
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

#include "BeeGl.h"
#include "connection/WiFiConnectionProvider.h"
namespace beegl
{
BeeGl::BeeGl()
{
}
BeeGl::~BeeGl()
{
}

int BeeGl::registerConnectionProvider(ConnectionProvider *connectionProvider)
{
    return connection.registerConnectionProvider(connectionProvider);
}
int BeeGl::registerBrokerInboundStrategy(BrokerInboundStrategy *inboundStrategy)
{
    return broker.registerInboundStrategy(inboundStrategy);
}
int BeeGl::registerMeasureProvider(MeasureProvider *measureProvider)
{
    return measurer.registerMeasureProvider(measureProvider);
}
int BeeGl::registerPublishStrategy(PublishStrategy *publishStrategy)
{
    return publisher.registerPublishStrategy(publishStrategy);
}
int BeeGl::registerTimeProviderStrategy(TimeProviderStrategy *timeProviderStrategy)
{
    return timeManagement.registerTimeProviderStrategy(timeProviderStrategy);
}

void BeeGl::prepare()
{
    
    if (!nvsSetup() || !storage_setup())
    {
        indicator.reportFail(1);
        return;
    }
    else
    {
        indicator.reportSuccess(1);
    }

    WiFiConnectionProvider::createAndRegister(this);
    timeManagement.registerTimeProviderStrategy(new TimeProviderStrategy(&settings, &connection));
}

void BeeGl::begin()
{
    runtime.initialize();
    runtime.setSafeModeOnRestart(1);

    delay(500);
    Serial.flush();

    runtime.printWakeupReason();

    if (runtime.getSafeMode())
    {
        // start AP and web server only
        connection.setOutboundMode(0x0);
        connection.setInboundMode(0x1);
        connection.setup();
        runtime.setSafeModeOnRestart(0);
        timeManagement.setup();
        log_i("***************************************");
        log_i("******** SAFE/MAINTENANCE MODE ********");
        log_i("***************************************");
        log_i("Use Wifi device, connect to SSID:%s. By using web browser navigate to http://192.168.4.1", settings.deviceName);
        return;
    }

    if (!settingsManagement.readConfig())
    {
        indicator.reportFail(2);
        runtime.setSafeMode(1);
    }
    else
    {
        indicator.reportSuccess(2);
    }

    btlog_i(TAG_DEVICE, "Device name: %s ", settings.deviceName);
    btlog_i(TAG_DEVICE, "Inbound mode: %u ", connection.getInboundMode());
    btlog_i(TAG_DEVICE, "Outbound mode: %u ", connection.getOutboundMode());

    if (!connection.setup())
    {
        indicator.reportFail(3);
        runtime.deepSleep();
    }
    else
    {
        indicator.reportSuccess(3);
    }
    
    
    timeManagement.setup();
    timeManagement.syncTime();
    settingsManagement.syncSettings();
    updater.checkFirmware();

    publisher.setup();
    measurer.setup();
    broker.setup();

    settingsManagement.storeLastGood();
    indicator.reportSuccess(4);
    connection.suspend();
    delay(2000);
    runtime.setSafeModeOnRestart(0);
    if (!runtime.getSafeMode())
    {
        measurer.begin();
        publisher.publish();
    }
}

void BeeGl::reportStatus()
{
    if (runtime.getSafeMode())
    {
        indicator.reportFail();
    }
    else
    {
        indicator.reportSuccess();
    }
}

bool BeeGl::nvsSetup()
{
    Serial.printf("[%s] Begin NVS\n", TAG_DEVICE);
    if (!NVS.begin())
    {
        Serial.printf("[%s] An Error has occurred while initilizing NVS\n", TAG_DEVICE);
        return false;
    }
    return true;
}

void BeeGl::update()
{
    reportStatus();
    runtime.update();
    publisher.update();
}
} // namespace beegl
