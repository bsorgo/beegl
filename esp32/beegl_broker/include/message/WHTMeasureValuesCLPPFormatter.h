/*
  LoraMeasurementCayenneLPPMessageFormatter.h - Measure values formatter
  
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

#ifndef WHTMeasureValuesCLPPFormatter_h
#define WHTMeasureValuesCLPPFormatter_h

#include "Message.h"
#include "Measurer.h"
#include "BeeGl.h"
#include "measurer/DHT22TempAndHumidityMeasureProvider.h"
#include "measurer/HX711WeightMeasureProvider.h"
#include <CayenneLPP.h>

namespace beegl
{
class WHTMeasureValuesCLPPFormatter : public IByteMessageSerializer
{
public:
  WHTMeasureValuesCLPPFormatter() {}
  int serializeBinary(JsonDocument *source, uint8_t *target) override;

private:
  CayenneLPP lpp = CayenneLPP(51);
};
} // namespace beegl
#endif
