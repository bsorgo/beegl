#ifndef Indicator_h
#define Indicator_h
/*
  Indicator.h - Indicator header file
  
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
#include <Arduino.h>

class Indicator
{
    
public:
    Indicator();
    void reportSuccess(int blinks);
    void reportFail(int blinks);
    void reportFail();
    void reportSuccess();
private:
    const int RED_LED_PIN = 25;
    const int GREEN_LED_PIN = 26;
};

#endif