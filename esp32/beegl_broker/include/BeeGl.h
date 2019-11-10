#ifndef BeeGl_h
#define BeeGl_h
/*
  BeeGl.h - BeeGl Core Wrapper header file
  
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
#include "Indicator.h"
#include "Log.h"
#include "Service.h"
#include "Settings.h"
#include "Runtime.h"
#include "Connection.h"
#include "Publisher.h"
#include "Broker.h"
#include "TimeManagement.h"
#include "SettingsManagement.h"
#include "LogManagement.h"
#include "Updater.h"
#include "Measurer.h"



namespace beegl
{
class BeeGl
{

public:
  BeeGl();
  ~BeeGl();
  void prepare();
  void begin();
  void update();

  Indicator indicator = Indicator();
  Settings settings = Settings();
  Service service = Service(&settings);
  Connection connection = Connection(&service, &settings);
  TimeManagement timeManagement = TimeManagement(&service, &settings, &connection);
  Runtime runtime = Runtime(&service, &settings, &connection);
  Updater updater = Updater(&runtime, &service, &settings, &connection);
  Publisher publisher = Publisher(&runtime, &settings, &connection, &service);
  SettingsManagement settingsManagement = SettingsManagement(&settings, &connection, &service, &runtime);
  LogManagement logManagement = LogManagement(&settings, &service);
  Measurer measurer = Measurer(&runtime, &service, &settings, &publisher);
  Broker broker = Broker(&connection, &service, &settings, &publisher);

  int registerConnectionProvider(ConnectionProvider *connection);
  int registerBrokerInboundStrategy(BrokerInboundStrategy *inboundStrategy);
  int registerMeasureProvider(MeasureProvider *measureProvider);
  int registerPublishStrategy(PublishStrategy *publishStrategy);
  int registerTimeProviderStrategy(TimeProviderStrategy *source);

private:
  void reportStatus();
  bool nvsSetup();
};
} // namespace beegl
#endif