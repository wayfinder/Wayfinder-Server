/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxConstants.h"

#include <cmath>

// Define POW2_32 to avoid having to include Utility.h
#undef POW2_32 
#define POW2_32 4294967296.0

#undef SQUARE
#define SQUARE(x) ((x)*(x))

const double
GfxConstants::EARTH_RADIUS = 6378137.0;

/* This number is an impossible number for latitude as it is beyond the 
 * poles.
 * For latitude it is a possible value and is situated at 180degrees
 * east or west
 */
const int32
GfxConstants::IMPOSSIBLE = MAX_INT32 - 1;

const float64
GfxConstants::MC2SCALE_TO_METER = EARTH_RADIUS*2.0*M_PI / POW2_32; 

const float64
GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER = SQUARE(MC2SCALE_TO_METER);

const float64
GfxConstants::METER_TO_MC2SCALE = POW2_32 / (EARTH_RADIUS*2.0*M_PI);

const float64
GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE = SQUARE(METER_TO_MC2SCALE);

const float64
GfxConstants::MC2SCALE_TO_CENTIMETER = MC2SCALE_TO_METER*100;

const int32
GfxConstants::MC2SCALE_TO_CENTIMETER_INT32 = int32(MC2SCALE_TO_CENTIMETER);

//   2^32/360
const double 
GfxConstants::degreeFactor = 11930464.7111;

const double 
GfxConstants::invDegreeFactor = 1.0 / GfxConstants::degreeFactor;

// 2^32/(360*60)
const double 
GfxConstants::minuteFactor = 198841.078518;

// 2^32/(360*60*60)
const double 
GfxConstants::secondFactor = 3314.01797531;

// 2^32/(2*M_PI)=2^31/M_PI
const double 
GfxConstants::radianFactor = GfxConstants::degreeFactor * 180 / M_PI;

const double 
GfxConstants::invRadianFactor = 1.0 / GfxConstants::radianFactor;

const double 
GfxConstants::sweref93LLAFactor = ( ((float64) MAX_UINT32) / (2.0 * M_PI));

const double 
GfxConstants::degreeToRadianFactor = ( M_PI / 180 );

const double 
GfxConstants::radianTodegreeFactor = ( 180 / M_PI );

