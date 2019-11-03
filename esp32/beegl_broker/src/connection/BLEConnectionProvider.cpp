
/*
  BLEConnectionProvider.cpp 
  
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

#include "connection/BLEConnectionProvider.h"

// set up the data structures.

namespace beegl
{

BLEConnectionProvider::BLEConnectionProvider(Connection *connection, Settings *settings) : ConnectionProvider(connection, settings)
{
}
BLEConnectionProvider *BLEConnectionProvider::createAndRegister(BeeGl *core)
{
    BLEConnectionProvider *i = new BLEConnectionProvider(&core->connection, &core->settings);
    core->registerConnectionProvider(i);
    return i;
}
void BLEConnectionProvider::btOff()
{
    blog_i("[BLE] OFF");
    esp_bt_controller_disable();
}

void BLEConnectionProvider::suspend()
{
    // not yet implemented
}

void BLEConnectionProvider::resume()
{
    // not yet implemented
}

void BLEConnectionProvider::shutdown()
{
    btOff();
}

bool BLEConnectionProvider::setup()
{
    return true;
}

void BLEConnectionProvider::checkConnect()
{
    // not yet implemented
}

Client *BLEConnectionProvider::getClient()
{
    // not supported
    return nullptr;
}
} // namespace beegl