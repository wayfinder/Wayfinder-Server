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
#include "CountryOverviewMap.h"
#include "TCPSocket.h"

#include "GfxCountryMapGenerator.h"
#include "WaterItem.h"
#include "StreetSegmentItem.h"
#include "StreetItem.h"
#include "MapHandler.h"
#include "Stack.h"
#include "GfxData.h"

GfxCountryMapGenerator::GfxCountryMapGenerator(MapHandler *mh,
                                               uint32 *mapIDs,
                                               uint32 port)
      : GenericGfxMapGenerator( mh, 
                                mapIDs, 
                                port) 
{
   mc2dbg2 << "GfxCountryMapGenerator created" << endl;
}


GfxCountryMapGenerator::~GfxCountryMapGenerator()
{
   mc2dbg2 << "GfxCountryMapGenerator destroyed" << endl;
}


void
GfxCountryMapGenerator::run()
{
   const int maxHandshake = 1024;
   char handshake[maxHandshake];
   
   TCPSocket* socket = waitForConnection(handshake, maxHandshake);

   if ( socket == NULL ) {
      mc2log << error << "No connection for GfxCountryMapGenerator" << endl;
      return;
   }
   
   DEBUG2(uint32 startTime = TimeUtility::getCurrentTime());

   // We've got an request for a map...
   mc2dbg4 << "GfxCountryMapGenerator, sending map with ID " 
           << mapIDs[0] << endl;
   CountryOverviewMap* theMap = 
      dynamic_cast<CountryOverviewMap*>(mapHandler->getMap(mapIDs[0]));
   DEBUG8(cerr << "MAPID " << mapIDs[0] << endl);
      
   // Check if the map is valid or not
   if (theMap != NULL) {
      // OK, map valid!

      uint32 nbrMaps = theMap->getNbrMaps();
      const GfxData* mapGfx = theMap->getGfxData();


      // Calculate the size of the filtered map when written to a buffer.
      uint32 filtMapsBufSize = 0;
      const uint32 nbrFiltLevels = 
         CountryOverviewMap::NBR_SIMPLIFIED_COUNTRY_GFX;
      for (uint32 i = 0; i < nbrFiltLevels; i++ ) {
         for (uint32 j = 0; j < mapGfx->getNbrPolygons(); j++ ) {
            const Stack* filtStack = theMap->getFilterStack(i,j);
            filtMapsBufSize += 4; // writes number of filt stack elements.
            for (uint32 k = 0; k < filtStack->getStackSize(); k++) {
               filtMapsBufSize += 4; // writes filt stack element.
            }
         }            
      }

      // Claculate size of buffer needed.
      uint32 bufSize = 4;     // nbrMaps
      bufSize += nbrMaps * 24; // nbrMaps * (mapId, creationTime, Bbox)
      bufSize += 4;           // polygons closed, nbrPolygons
      bufSize += mapGfx->getNbrPolygons() * 4; // nbr coordinates per polygon.
      bufSize += theMap->getGfxData()->getTotalNbrCoordinates() * 8; // coords.
      bufSize += 4; // nbrFiltLevels
      bufSize += filtMapsBufSize; // Size of the filter data.

      DataBuffer idsAndGfx(bufSize);

      // Note that we multiply the size of the coordinates with two in 
      // order to make space for the filtered versions of the country
      // GfxData as well.
//    DataBuffer idsAndGfx(nbrMaps * 24 + // ID + ver + Bbox
//                         theMap->getGfxData()->getTotalNbrCoordinates()*8*2 +
//                         102400);   // approx gfxdata
      
      idsAndGfx.writeNextLong(nbrMaps);
      for (uint32 i=0; i<nbrMaps; i++) {
         uint32 creationTime;
         int32 maxLat, minLat, maxLon, minLon;
         uint32 mapID = theMap->getMapData(i, creationTime,
                                           maxLat, minLon,
                                           minLat, maxLon);
         idsAndGfx.writeNextLong(mapID);
         idsAndGfx.writeNextLong(creationTime);
         idsAndGfx.writeNextLong(maxLat);
         idsAndGfx.writeNextLong(minLon);
         idsAndGfx.writeNextLong(minLat);
         idsAndGfx.writeNextLong(maxLon);
         mc2dbg1 << "   Wrote mapID=" << mapID << ", version=" 
                 << creationTime << ", bbox="
                 << maxLat << ", " << minLon << ", " << minLat << ", "
                 << maxLon << endl;
      }
      
      // Save the GfxData of the country
      

      //uint32 nbrPolygons = MIN(5, mapGfx->getNbrPolygons());
      uint32 nbrPolygons = 0;
      // The polygons being sorted on nbrCoordinates, we an accept 
      // to skip(send) 5 small polys with many coords, in order to 
      // send larger polygons with fewer coordinates.
      uint32 maxNbrSkipPolys = 5;
      uint32 skipPolys[maxNbrSkipPolys];
      for (uint32 i = 0; i < maxNbrSkipPolys;i++) {
         skipPolys[i] = MAX_UINT32;
      }
      bool cont = true;
      uint32 nbrSkip = 0;
      while (cont && (nbrPolygons < mapGfx->getNbrPolygons())) {
         mc2dbg8 << "poly " << nbrPolygons << " nbrC=" 
                 << mapGfx->getNbrCoordinates(nbrPolygons)
                 << " length=" << mapGfx->getLength(nbrPolygons);
         if (mapGfx->getLength(nbrPolygons) < 35000) {
            skipPolys[nbrSkip] = nbrPolygons;
            mc2dbg8 << "  - \"skipping\" ";
            nbrSkip++;
            if (nbrSkip >= maxNbrSkipPolys) {
               cont = false;
            }
         }
         mc2dbg8 << endl;
         nbrPolygons++;
      }
      // Don't send if any of the small polygons are the last ones..
      uint32 p = maxNbrSkipPolys;
      while ( (p > 0) && (skipPolys[p-1] >= nbrPolygons-1)) {
         if (skipPolys[p-1] == nbrPolygons-1) {
            mc2dbg8 << " skip poly in the end of nbrPolygons" << endl;
            nbrPolygons--;
         }
         p--;
      }

      nbrPolygons=MAX(1, nbrPolygons);
      
      mc2dbg << "Sending " << nbrPolygons << " polygons of "
             << mapGfx->getNbrPolygons() << endl;


      idsAndGfx.writeNextBool(true);        // polygons closed
      idsAndGfx.writeNextByte(0);           // PAD
      idsAndGfx.writeNextShort(nbrPolygons);// # polygons
      for (uint32 p=0; p<nbrPolygons;p++) {
         uint32 nbrCoordinates = mapGfx->getNbrCoordinates(p);
         idsAndGfx.writeNextLong(nbrCoordinates);
         for (uint32 i = 0; i < nbrCoordinates; i++) {
            idsAndGfx.writeNextLong(mapGfx->getLat(p, i));
            idsAndGfx.writeNextLong(mapGfx->getLon(p, i));
         }
      }
      
      // Send filtered gfxdata information.

      // Nbr filtering levels
      idsAndGfx.writeNextLong(nbrFiltLevels);      
      for (uint32 i = 0; i < nbrFiltLevels; i++ ) {
         // Nbr polygons already written before
         for (uint32 j = 0; j < nbrPolygons; j++ ) {
            const Stack* filtStack = theMap->getFilterStack(i,j);
            // Nbr indices in the filtering stack.
            idsAndGfx.writeNextLong(filtStack->getStackSize());
            for (uint32 k = 0; k < filtStack->getStackSize(); k++) {
               // Index
               idsAndGfx.writeNextLong(filtStack->getElementAt(k));
            }
         }            
      }
       
      
      // Create and fill a buffer with the string items in this country
      DataBuffer stringItems(theMap->getMaximunSizeOfStringItems()*2 +
                             40000000); // length of items
      
      if (!theMap->saveStringItems(&stringItems)) {
         mc2log << error <<"Error saving string items for country" << endl;
      }


      // Create and fill buffer with items.
      
      // Nbr items. To be filled in later.
      uint32 offset = stringItems.getCurrentOffset();
      stringItems.writeNextLong(0);
      uint32 maxCountryMapZoom = 3;
      uint32 nbrItems = 0;
      for (uint32 z = 0; z < maxCountryMapZoom; z++) {
         writeItemsInZoomLevel(theMap, &stringItems, z, nbrItems);
      }
      // Fill in nbr items
      stringItems.writeLong(nbrItems, offset);
      
      
      // Create and fill buffer with version and length
      DataBuffer versionAndLengthBuffer(8);
      versionAndLengthBuffer.writeNextLong(theMap->getCreationTime());
      uint32 length = idsAndGfx.getCurrentOffset() +
                      stringItems.getCurrentOffset();
      versionAndLengthBuffer.writeNextLong(length);

      // Send the buffers via TCP
      uint32 nbrBytes = 
         socket->writeAll( versionAndLengthBuffer.getBufferAddress(),
                           versionAndLengthBuffer.getCurrentOffset() );
      mc2dbg4 << "   Sent " << nbrBytes << " with version ("
              << theMap->getCreationTime() << ") and length (" 
              << length << ")" << endl;

      nbrBytes = socket->writeAll( idsAndGfx.getBufferAddress(),
                                   idsAndGfx.getCurrentOffset() );
      mc2dbg4 << "   Sent " << nbrBytes << " with map IDs and GfxData"
              << endl;

      nbrBytes = socket->writeAll( stringItems.getBufferAddress(),
                                   stringItems.getCurrentOffset() );
      mc2dbg4<< "   Sent " << nbrBytes << " with string items"
                  << endl;

      

      // Delete the socket
      delete socket;
   }
   DEBUG2(
   mc2dbg << "GfxCountryMapGenerator sent all data for map " 
               << mapIDs[0] << ", processing time " 
               << TimeUtility::getCurrentTime()-startTime << " ms" << endl;
   );

}

bool 
GfxCountryMapGenerator::includeItem(const Item* item, GenericMap* theMap,
                                    bool& reduceGfxToPoint,
                                    uint16 polygon)
{
   // Default is to not reduce graphics to a point.
   reduceGfxToPoint = false;
   switch ( item->getItemType() ) {
      
      // - - - - - - - - - - - - - - - - - - - - - - Street segment item
      case ( ItemTypes::streetSegmentItem ) : {
         const StreetSegmentItem* ssi = 
            static_cast<const StreetSegmentItem*>(item);
         return (ssi->getRoadClass() == 0);
      } break;
      
      // - - - - - - - - - - - - - - - - - - - - - - Street item
      case ( ItemTypes::streetItem ) : {
         const StreetItem* si = static_cast<const StreetItem*>(item);
         return ( ( si->getGfxData() != NULL ) && 
                  ( si->getRoadClassForPolygon(polygon) == 0 ) );
      } break;
      
      // - - - - - - - - - - - - - - - - - - - - - - Water item
      case (ItemTypes::waterItem) : {
         const WaterItem* wItem = static_cast<const WaterItem*> (item);
         if (wItem->getWaterType() != ItemTypes::ocean) {
            return (true);
         }
      } break;
      
      // - - - - - - - - - - - - - - - - - - - - - - Bua
      case (ItemTypes::builtUpAreaItem) : {
         GfxData* gfx = item->getGfxData();
         if ( gfx != NULL ) {
            // The boundingbox of the bua most be at least 7 * 7 km2 
            if ( gfx->getBBoxArea() > 7000*7000) {
               reduceGfxToPoint = true;
               return (true);
            }
         }
      } break;

      default:
         return (false);
   }
   return (false);
}

