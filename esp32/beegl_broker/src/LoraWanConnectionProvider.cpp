
/*
  LoraWanConnectionProvider.cpp - Lora Wan connection implementation
  
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

#include "LoraWanConnectionProvider.h"




// set up the data structures.

LoraWanConnectionProvider::LoraWanConnectionProvider(Settings *settings) : ConnectionProvider(settings)
{
}

void LoraWanConnectionProvider::suspend()
{
}

void LoraWanConnectionProvider::resume()
{
}

void LoraWanConnectionProvider::shutdown()
{
}

bool LoraWanConnectionProvider::setup()
{
  
}

void LoraWanConnectionProvider::checkConnect()
{
}

Client *LoraWanConnectionProvider::getClient()
{
  return nullptr;
}
