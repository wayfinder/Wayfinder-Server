/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PRECACHEROUTEDATA_H
#define PRECACHEROUTEDATA_H

#include "config.h"

#include "RouteID.h"
#include "LangTypes.h"
#include "STLUtility.h"
#include "MC2Coordinate.h"
#include <vector>
#include <set>

/**
 * Holds information about a route in coordinates and speed for
 * precache generation.
 */
struct PreCacheRouteData {
   RouteID m_id; //< uniq route id
   uint32 m_extraPixels; //< pixel size of the clients screen 
   LangType::language_t m_language; //< language for text
   set<int32> m_layerIDs; //< layers to be used
   /// data for one route segment   
   struct SubData {
      /// coordinate of a specific route segment
      vector<MC2Coordinate> m_coords;
      /// speed limit at this segment
      uint32 m_speedLimit;
   };
   /// route segment datas 
   STLUtility::AutoContainer< vector<SubData*> > m_routes;
};

#endif //  PRECACHEROUTEDATA_H

