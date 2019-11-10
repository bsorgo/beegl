/*
  HX711WeightMeasureProvider.h - HX711 weight 
  
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

#ifndef HX711WeightMeasureProvider_h
#define HX711WeightMeasureProvider_h

#include "Measurer.h"
#include "BeeGl.h"

#include <HX711.h>

#define STR_SCALESETTINGS "scaleS"
#define STR_SCALEUNIT "u"
#define STR_SCALEOFFSET "off"
#define STR_SCALEFACTOR "f"

#define STR_WEIGHTSENSOR "whtS"
#define STR_WEIGHT "w"
#define STR_WEIGHTUNIT "u"

#define TAG_HX711 "HX711"

namespace beegl
{
class HX711WeightMeasureProvider : public MeasureProvider
{
public:
  HX711WeightMeasureProvider(Runtime *runtime, Service *service, Settings *settings);
  static HX711WeightMeasureProvider* createAndRegister(BeeGl *core);

  void measure(JsonDocument *values) override;
  void setup() override;

  void readSettings(const JsonObject &source) override;
  void writeSettings(JsonObject &target, const JsonObject &input) override;

private:
  int m_channel;

  HX711 m_scale;

  /* SCALE parameters 
        scaleFactor: 
        Scale factor
        scaleOffset:
        Offset - absolute
        scaleUnit:
        Scale unit
    */
  float m_scaleFactor = 1;
  long m_scaleOffset = 0;
  char m_scaleUnit[5] = "g";

  const char SCALE_DOUT_PIN = 32;
  const char SCALE_SCK_PIN = 33;

  void webServerBind();
  long zero();
};
} // namespace beegl
#endif