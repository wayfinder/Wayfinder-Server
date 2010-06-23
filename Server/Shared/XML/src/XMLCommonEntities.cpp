/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLCommonEntities.h"

#ifdef USE_XML
#include "CoordinateTransformer.h"
#include "GfxUtility.h"
#include "GfxConstants.h"
#include "StringTable.h"

TopRegionMatch::topRegion_t
XMLCommonEntities::stringToTopRegionType( const char* str ) {
   if ( StringUtility::strcmp( str, "country" ) == 0 ) {
      return TopRegionMatch::country;
   } else if ( StringUtility::strcmp( str, "state" ) == 0 ) {
      return TopRegionMatch::state;
   } else if ( StringUtility::strcmp( str, "internationalRegion" ) == 0 ) {
      return TopRegionMatch::internationalRegion;
   } else if ( StringUtility::strcmp( str, "metaregion" ) == 0 ) {
      return TopRegionMatch::metaregion;
   } else {
      return TopRegionMatch::nbr_topregion_t;
   }
}


const char*
XMLCommonEntities::topRegionTypeToString(
   TopRegionMatch::topRegion_t type )
{
   switch ( type ) {
      case TopRegionMatch::country :
         return "country";
      case TopRegionMatch::state :
         return "state";
      case TopRegionMatch::internationalRegion :
         return "internationalRegion";
         case TopRegionMatch::metaregion :
         return "metaregion";
      case TopRegionMatch::nbr_topregion_t :
         mc2log << error << "XMLCommonEntities::topRegionTypeToString "
                << "topRegion_t is nbr_topregion_t, this is wrong!"
                << endl;
         return "nbr_topregion_t";
   }

   // Unreachable code
   return "nbr_topregion_t";
}


XMLCommonEntities::coordinateType
XMLCommonEntities::coordinateFormatFromString(
   const char* const str,
   coordinateType defaultFormat )
{
   coordinateType coordinateSystem = defaultFormat;

   if ( strcmp( str, "WGS84" ) == 0 ) {
      coordinateSystem = WGS84;
   } else if ( strcmp( str, "WGS84Rad" ) == 0 ) {
      coordinateSystem = WGS84Rad;
   } else if ( strcmp( str, "WGS84Deg" ) == 0 ) {
      coordinateSystem = WGS84Deg;
   } else if ( strcmp( str, "MC2" ) == 0 ) {
      coordinateSystem = MC2;
   } else {
      mc2log << warn << "XMLCommonEntities::coordinateFormatFromString "
                "odd position_system, using default."
             << "coordinateFormat " << str << endl;
   }

   return coordinateSystem;
}


const char*
XMLCommonEntities::coordinateFormatToString(
   coordinateType format )
{
   if ( format == WGS84 ) {
      return "WGS84";
   } else  if ( format == WGS84Rad ) {
      return "WGS84Rad";
   } else  if ( format == WGS84Deg ) {
      return "WGS84Deg";
   } else  if ( format == MC2 ) {
      return "MC2";
   } else {
      mc2log << warn<< "XMLCommonEntities::coordinateFormatToString "
                "odd position_system, using MC2."
             << "coordinateFormat " << (int)format << endl;
      return "MC2";
   }
}


char*
XMLCommonEntities::coordinateToString(
   char* target,
   int32 ordinate,
   coordinateType format,
   bool latitude )
{
   if ( format == WGS84 || format == WGS84Rad || format == WGS84Deg ) {
      float64 wgs84Ordinate, out2, out3;
      GfxUtility::coordianteRepresentations coordType =
         GfxUtility::wgs84Style;
      uint32 precision = 4;
      CoordinateTransformer::format_t coordSystem =
         CoordinateTransformer::sweref93_LLA;
      if ( format == WGS84 ) {
         coordType = GfxUtility::wgs84Style;
         precision = 4;
      } else if ( format == WGS84Rad ) {
         coordType = GfxUtility::wgs84Rad;
         precision = 10;
      } else if ( format == WGS84Deg ) {
         coordType = GfxUtility::wgs84Deg;
         precision = 8;
      }

      if ( latitude ) {
         CoordinateTransformer::transformFromMC2( ordinate, 0,
                                                  coordSystem,
                                                  wgs84Ordinate,
                                                  out2,
                                                  out3 );
         GfxUtility::printLatLon( target,
                                  wgs84Ordinate,
                                  latitude,
                                  StringTable::ENGLISH,// NWSE not any
                                  precision,          // other (NSÖV)
                                  coordType );
      } else {
         CoordinateTransformer::transformFromMC2( 0, ordinate,
                                                  coordSystem,
                                                  out2,
                                                  wgs84Ordinate,
                                                  out3 );
         GfxUtility::printLatLon( target,
                                  wgs84Ordinate,
                                  latitude,
                                  StringTable::ENGLISH,// NWSE not any
                                  precision,          // other (NSÖV)
                                  coordType );
      }
   } else { // MC2
      sprintf( target, "%d", ordinate );
   }

   return target;
}


bool
XMLCommonEntities::coordinateFromString(
   const char* coordianteStr,
   int32& ordinate,
   coordinateType format )
{
   if ( coordianteStr == NULL ) {
      return false;
   }
   bool ok = true;

   if ( format == WGS84 ) {
      // Perhaps N 55ï¿½ 40' 29.4059"
      char signChar[2];
      int degs = 0;
      int mins  = 0;
      float secs = 0.0;

      int res = sscanf( coordianteStr,
                        "%1[NESW] %d %d' %f\""
                        , signChar, &degs, &mins, &secs );

      if ( res != 4 ) {
         // Try with degree sign
         res = sscanf( coordianteStr,
#ifdef MC2_UTF8
                       "%1[NESW] %dÂ° %d' %f\""
#else
                       "%1[NESW] %d° %d' %f\""
#endif
                       , signChar, &degs, &mins, &secs );
      }

      if ( res == 4 ) {
         // Newer format
         double coordVal = GfxConstants::degreeFactor * degs;
         coordVal += GfxConstants::minuteFactor * mins;
         coordVal += GfxConstants::secondFactor * secs;
         if ( signChar[ 0 ] == 'S' || signChar[ 0 ] == 'W' ) {
            coordVal = -coordVal;
         }
         ordinate = int32( rint( coordVal ) );
      } else if ( strlen( coordianteStr ) >= 7 ) { // "N 01234"
         ordinate = GfxUtility::convLatLonStringToInt( coordianteStr );
      } else {
         ok = false;
      }
   } else if ( format == WGS84Rad ) {
      char* tmp = NULL;
      errno = 0;
      float64 radOrdinate = strtod( coordianteStr, &tmp );
      if ( !(tmp == NULL || *tmp != '\0' || tmp == coordianteStr)
           && errno != ERANGE &&
           radOrdinate >= -M_PI && radOrdinate <= M_PI )
      {
         ordinate = int32( rint(
            radOrdinate * GfxConstants::radianFactor ) );
      } else {
         ok = false;
      }
   } else if ( format == WGS84Deg ) {
      char* tmp = NULL;
      errno = 0;
      float64 degOrdinate = strtod( coordianteStr, &tmp );
      if ( !(tmp == NULL || *tmp != '\0' || tmp == coordianteStr) &&
           errno != ERANGE && degOrdinate >= -180.0 && degOrdinate <= 180.0 )
      {
         ordinate = int32( rint(
            degOrdinate * GfxConstants::degreeFactor ) );
      } else {
         ok = false;
      }
   } else if ( format == MC2 ) {
      char* endPtr = NULL;
      errno = 0;
      ordinate = strtol( coordianteStr, &endPtr, 10 );
      if ( (endPtr == NULL || *endPtr != '\0' || endPtr == coordianteStr) ||
           errno == ERANGE ) {
         ok = false;
      }
   }

   return ok;
}


#endif // USE_XML
