/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxFeatureMapUtility.h"

#include "GfxFeatureMapUtility.h"
#include "PointOfInterestItem.h"
#include "GenericMap.h"
#include "POIInfo.h"
#include "ServerTileMapFormatDesc.h"
#include "MapSettings.h"

namespace GfxFeatureMapUtility {

void getExtraInfoForPOI( const PointOfInterestItem& poi,
                         const GenericMap& map,
                         byte& extraPOIInfo,
                         MC2String& imageName ) {

   auto_ptr<POIInfo> poiInfo = map.getPOIInfo( &poi );

   if ( ! poiInfo.get() ) {
      return;
   }
      
   // 100 = subtype icon key in database
   const POIInfo::InfoArray subTypeIconData = poiInfo->findData( 100 );
   MC2String iconSubType;
   if ( ! subTypeIconData.empty() ) {
      iconSubType = subTypeIconData[ 0 ].getInfo();
   } 


   MC2String uniqueIcon;
   // Get unique icon path string
   // 99 = unique icon
   const POIInfo::InfoArray uniqueIconData = poiInfo->findData( 99 );
   if ( ! uniqueIconData.empty() ) {
      uniqueIcon = uniqueIconData[ 0 ].getInfo();
   }

   if ( uniqueIcon.empty() && iconSubType.empty() ) {
      // nothing to do with empty icon
      return;
   }

   //
   // compose image name as:
   // "unique_icon/_poiset_subtype"
   // For instance:
   // "gardstangamacken/_mapsupplier_stationbrand"
   //

   // add unique name first + "/"
   if ( ! uniqueIcon.empty() ) {
      imageName += uniqueIcon + "/";
   }

   //
   // TODO: insert poi set name here. 
   //       if needed.
   //

   // add icon sub type
   if ( ! iconSubType.empty() ) {
      imageName += "_";
      imageName += iconSubType;
   }
   // all done with image name

   // setup some strange extra poi info here
   // ...( hackish, not to be confused with haggis )
   // the extra poi info is used later in tilemodule to create
   // the correct poi type.
   extraPOIInfo = 7;
   if ( iconSubType == "bestprice" ) {
      extraPOIInfo = 1;
   }

   // this is a special special poi
   extraPOIInfo |= ServerTileMapFormatDesc::SPECIAL_CUSTOM_POI_MASK;
}

/// Extra check to remove pois before checking the coordinates.
bool checkPOIExtra( const PointOfInterestItem& poi,
                    const GenericMap& theMap,
                    const MapSettings& mapSettings,
                    bool includeFreePOIs ) {
   switch ( poi.getPointOfInterestType() ) {
      case ItemTypes::airport: 
      case ItemTypes::cityCentre:
         // Airport and city centers should be included in 
         // the map layer ( = show city centers setting )
         
         // we like city centers, but only if we want them.
         if ( mapSettings.getShowCityCentres() ) {
            return true;
         }
         break;
      case ItemTypes::unknownType:
      case ItemTypes::notCategorised:
         return false;
      default:
         break;
   }



   // exclude other pois if show city center is requested
   // this is mainly for map layer tile maps
   if ( !includeFreePOIs && mapSettings.getShowCityCentres() ) {
      return false;
   }

   // check poi rights
   const MapRights& rights = theMap.getRights( poi.getID() );
   // if no rights needed and we are not currently looking
   // for specific rights, then let this one thru
   if ( rights == ~MapRights() ) {
      // include full right pois and ignore map rights setting
      if ( includeFreePOIs &&
           ! ( mapSettings.getMapRights() & MapRights::DISABLE_POI_LAYER ) ) {
         return true;
      }

      if ( ! mapSettings.getMapRights() ) {
         // no rights needed
         return true;
      } else {
         // needs specific rights!
         return false;
      }

   } 

   // are the rights correct?
   return mapSettings.getMapRights() & rights;
}

}
