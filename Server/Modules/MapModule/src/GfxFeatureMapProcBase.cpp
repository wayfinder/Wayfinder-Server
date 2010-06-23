/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "GfxFeatureMapProcBase.h"

#include "GenericMap.h"
#include "GetAdditionalPOIInfo.h"
#include "Item.h"
#include "Properties.h"
#include "PointOfInterestItem.h"
#include "POIInfo.h"
#include "StringConvert.h"
#include "MapBits.h"

GfxFeatureMapProcBase::GfxFeatureMapProcBase( GenericMap& m,
                                              GfxFeatureMapProcBase* dataMan)
{
   m_map = &m;
   m_poiInfoCached = false;
   m_theOneWithTheData = dataMan;
}

GfxFeatureMapProcBase::~GfxFeatureMapProcBase()
{
}

void
GfxFeatureMapProcBase::precacheDisplayClasses()
{
   // Cache poi info if needed.
   if ( m_poiInfoCached ) {
      return;
   }

   mc2dbg << "[GFMPB]: START Precaching wasp. " << endl;
   // City centres are by default on zoom 14 poiiZoomLevel
   // but in co maps, seom city centres are also on zoom 1
   set<uint32> zoomlevels;
   zoomlevels.insert(ItemTypes::poiiZoomLevel);
   if ( MapBits::isCountryMap(m_map->getMapID()) ) {
      zoomlevels.insert(1);
   }
   for ( set<uint32>::const_iterator it = zoomlevels.begin();
         it != zoomlevels.end(); it++) {
      // Loop over all the pois.
      for ( uint32 i = 0;
            i < m_map->getNbrItemsWithZoom( *it );
            ++i ) {

         const Item* curItem = m_map->getItem(*it, i);
         
         if ( curItem == NULL ||
              curItem->getItemType() != ItemTypes::pointOfInterestItem ) {
            continue;
         }

         // only interested in city center
         const PointOfInterestItem* poi = 
            static_cast<const PointOfInterestItem*>( curItem );
         if ( ! poi->isPointOfInterestType( ItemTypes::cityCentre ) ) {
            continue;
         }

         // fetch DISPLAY_CLASS types and add the values to cache
         auto_ptr<POIInfo> poiInfo( m_map->getPOIInfo( poi ) );
         if ( poiInfo.get() == NULL ) {
            continue;
         }
         for ( uint32 infoIdx = 0; 
               infoIdx < poiInfo->getInfos().size(); 
               ++infoIdx ) {
            POIInfoData* data = poiInfo->getInfos()[ infoIdx ];
            if ( data->getType() ==
                 GetAdditionalPOIInfo::DISPLAY_CLASS ) {
               try {
                  m_ccDisplayClassByPOIID[ poi->getWASPID() ] =
                     StringConvert::convert<uint32>( data->getInfo() );
               } catch( const StringConvert::ConvertException& ignore ) {
               }
            }
         }
      }
   }
   mc2dbg << "[GFMPB]: END Precaching wasp." << endl;

   
   m_poiInfoCached = true;   
}

bool
GfxFeatureMapProcBase::getDisplayClassForCC( uint32 poiID, 
                                             uint8& displayClass )
{
   // Check if running as "slave"
   if ( m_theOneWithTheData ) {
      return m_theOneWithTheData->getDisplayClassForCC( poiID,
                                                        displayClass );
   }

   if ( m_poiInfoCached == false ) {
      precacheDisplayClasses();
   }
   
   // And now to the finding.
   map<uint32, uint8>::const_iterator it = 
      m_ccDisplayClassByPOIID.find( poiID );
   if ( it != m_ccDisplayClassByPOIID.end() ) {
      // Found in the table!
      displayClass = it->second;
      return true;
   }
   return false;
}


