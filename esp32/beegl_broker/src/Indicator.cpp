/*
  Indicator.cpp - Provides (so far) LED indicator for success/failed flows.
  
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

namespace beegl
{
#define BLING_PAUSE 80
Indicator::Indicator()
{
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
}

void Indicator::reportSuccess(int blinks)
{
    btlog_d(TAG_INDICATOR, "Success - blinks: %u", blinks);
    digitalWrite(RED_LED_PIN, LOW);
    for (int i = 0; i < blinks; i++)
    {
        digitalWrite(GREEN_LED_PIN, HIGH);
        delay(BLING_PAUSE);
        digitalWrite(GREEN_LED_PIN, LOW);
        delay(BLING_PAUSE);
    }
}

void Indicator::reportFail(int blinks)
{
    btlog_d(TAG_INDICATOR, "Success - blinks: %u", blinks);
    digitalWrite(RED_LED_PIN, LOW);
    for (int i = 0; i < blinks; i++)
    {
        digitalWrite(RED_LED_PIN, HIGH);
        delay(BLING_PAUSE);
        digitalWrite(RED_LED_PIN, LOW);
        delay(BLING_PAUSE);
    }
}

void Indicator::reportFail()
{
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
}
void Indicator::reportSuccess()
{
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
}

} // namespace beegl