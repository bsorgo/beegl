#ifndef BLEConnectionProvider_h
#define BLEConnectionProvider_h

/*
  BLEConnectionProvider.h - BLE connection
  
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

#include "Log.h"
#include "Connection.h"
#include "Settings.h"
#include "BeeGl.h"
#include <esp_bt.h>

#define TAG_BLE "BLE"

namespace beegl
{
class BLEConnectionProvider : public ConnectionProvider
{

public:
  BLEConnectionProvider(Connection *connection, Settings *settings);
  static BLEConnectionProvider* createAndRegister(BeeGl *core);
  void checkConnect() override;
  bool setup() override;
  void shutdown() override;
  void suspend() override;
  void resume() override;
  const char getInboundType() { return 0x2; };
  const char getOutboundType() { return 0x0; };
  const char *getName() { return "BLE"; }

private:
  bool bleSetup();
  void btOff();
};
} // namespace beegl
#endif