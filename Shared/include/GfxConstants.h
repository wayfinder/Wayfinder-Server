/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXCONSTANTS_H
#define GFXCONSTANTS_H

// TileMapConfig includes config.h or similar
#include "TileMapConfig.h"

/**
 *   Class containing the constants (previosly) found in GfxUtility.
 */
class GfxConstants {
public:
   /**
    *   An impossible value for coordinates. This value
    *   is impossible for LATITUDE but not for LONGITUDE.
    *   The reason for being impossible is that it is situated beyond
    *   the poles.
    */
   static const int32 IMPOSSIBLE; 
   
   /**
    *    @name Multiplicative scalefactors (distance).
    *    Multiplicative constants to convert between distances in
    *    meters and MC2-scale. In the MC2-scale the circumference
    *    at the equator is 2^32.
    */
   //@{
      /// From MC2-scale to meter.
      static const float64 MC2SCALE_TO_METER;
   
      /// From MC2-scale to centimeter (float).
      static const float64 MC2SCALE_TO_CENTIMETER;
   
      /// From MC2-scale to centimeter (integer).
      static const int32 MC2SCALE_TO_CENTIMETER_INT32;
   
      /// From (MC2-scale)^2 to (meter)^2.
      static const float64 SQUARE_MC2SCALE_TO_SQUARE_METER;
   
      /// From meter to MC2-scale.
      static const float64 METER_TO_MC2SCALE;
   
      /// From (meter)^2 to (MC2-scale)^2.
      static const float64 SQUARE_METER_TO_SQUARE_MC2SCALE;
   //@}

   /**
    *   @name  Multiplicative scalefactors (position).
    *   Multiplicative scalefactors used to convert from degrees, 
    *   minutes, seconds and sweref93_lla to "our" unit.
    *
    *   The factors are calculated as:<BR>
    *      degreeFactor = (PI / 180.0) * ((float64) pow(2,32)/(2*PI)) 
    *                    = (1/360)*2^32 = 2^32/(360) = 11930464.7111<BR>
    *      minuteFactor = degreeFactor/60 = 198841.078518<BR>
    *      secondFactor = degreeFactor/3600 = 3314.01797531<BR>
    *      radianFactor = degreeFactor * 180 / M_PI<BR>
    *      sweref93LLAFactor = ( ((float64) MAX_UINT32) / (2.0 * M_PI))
    */
   //@{
      /// From degrees to our unit.
      static const double degreeFactor;
   
      /// From our unit to degrees.
      static const double invDegreeFactor;
   
      /// From minutes to our unit.
      static const double minuteFactor;
   
      /// From seconds to our unit.
      static const double secondFactor;
   
      /// From radians to out unit.
      static const double radianFactor;
   
      /// From our unit to radians.
      static const double invRadianFactor;
   
      /// From sweref93 lla to our unit.
      static const double sweref93LLAFactor;
   //@}

   /// Radius of earth in meters
   static const double EARTH_RADIUS;
         
   /**
    *    @name Multiplicative factors, degrees <-> radians.
    *    Multiplicative factors for degrees <-> radians.
    *    The factors are calculated as:<BR>
    *         degreeToRadianFactor = ( PI / 180 )<BR>
    *         radianTodegreeFactor = ( 180 / PI ) <BR>
    */
   //@{
      /// From degrees to radians.
      static const double degreeToRadianFactor;
   
      /// From radians to degrees.
      static const double radianTodegreeFactor;
   //@}
};

#endif
