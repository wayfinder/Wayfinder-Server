/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavUtil.h"
#include "LangTypes.h"
#include "NameUtility.h"


StringTable::languageCode 
NavUtil::mc2LanguageCode( uint32 language )
{
   uint32 dist = 0;
   // Get the lang type
   LangTypes::language_t langType =
      LangTypes::getNavLangAsLanguage( language );

   // Translate that into a languageCode for use in the
   // StringTable.
   StringTable::languageCode langCode =
      NameUtility::getBestLanguage( langType, dist );

   // This is for debug
   if ( dist != NameUtility::maxLangPoints ) {
      mc2log << warn << "[NavUtil]: Language "
             << LangTypes::getLanguageAsString( langType )
             << " became "
             << StringTable::getString(
                StringTable::getLanguageAsStringCode( langCode ),
                StringTable::ENGLISH ) << endl;
   }
   
   return langCode;
}


ItemTypes::vehicle_t 
NavUtil::mc2Vehicle( uint8 vehicle ) {
   switch( vehicle ) {
      case 0x01:
         return ItemTypes::passengerCar;
      case 0x02:
         return ItemTypes::pedestrian;
      case 0x03:
         return ItemTypes::emergencyVehicle;
      case 0x04:
         return ItemTypes::taxi;
      case 0x05:
         return ItemTypes::publicBus;
      case 0x06:
         return ItemTypes::deliveryTruck;
      case 0x07:
         return ItemTypes::transportTruck;
      case 0x08:
         return ItemTypes::highOccupancyVehicle;
      case 0x09:
         return ItemTypes::bicycle;
      case 0x0a:
         return ItemTypes::publicTransportation;

      default:
         return ItemTypes::passengerCar;
   }
}


RouteTypes::routeCostType 
NavUtil::mc2RouteCost( uint8 routeCost ) {
   switch ( routeCost ) {
      case 0x00:
         return RouteTypes::DISTANCE;
      case 0x01:
         return RouteTypes::TIME;
      case 0x02:
         return RouteTypes::TIME_WITH_DISTURBANCES;

      default:
         return RouteTypes::TIME;
   }
}
