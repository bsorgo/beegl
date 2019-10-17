
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

#include "BLEConnectionProvider.h"

// set up the data structures.

BLEConnectionProvider::BLEConnectionProvider(Settings *settings) : ConnectionProvider(settings)
{

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

