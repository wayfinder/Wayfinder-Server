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

#include "TCPSocket.h"
#include "ItemNames.h"
#include "Item.h"
#include "MapHandler.h"
#include "GenericMap.h"

#include "StringTableMapGenerator.h"

#include "PointOfInterestItem.h"
#include "StringUtility.h"
#include "UTF8Util.h"

#include "ScopedArray.h"
#include "TimeUtility.h"

#include <memory>

StringTableMapGenerator::StringTableMapGenerator(
                                 MapHandler *mh,
                                 uint32 *mapIDs,
                                 uint16 port) : MapGenerator(  mh, 
                                                               mapIDs, 
                                                               1,
                                                               port) 
{
   mc2dbg2 << "StringTableMapGenerator created" << endl;
}


StringTableMapGenerator::~StringTableMapGenerator()
{
   mc2dbg2 << "StringTableMapGenerator destroyed" << endl;
}


void
StringTableMapGenerator::run()
{
   const int maxHandshake = 1024;
   char handshake[maxHandshake];
   
   auto_ptr<TCPSocket> socket( waitForConnection(handshake, maxHandshake) );
   
   if ( socket.get() == NULL ) {
      mc2log << warn << "No connection for StringTableMapGenerator" << endl;
      return;
   }

   uint32 startTime = TimeUtility::getCurrentTime();
   
   
   // We've got an request for a map...
   uint32 mapID = mapIDs[0];  // Always one map!

   GenericMap* theMap = mapHandler->getMap(mapID);
   if (theMap == NULL) {
      mc2log << warn << "StringTableMapGenerator theMap == NULL" << endl;
      return;
   }

   ItemNames* itemNames = theMap->getItemNames();
   if (itemNames == NULL) {
      mc2log << warn << "StringTableMapGenerator itemNames == NULL"<< endl;
      return;
   }

   // Find out the strings to send
   // First create a temporary array with the strings
   uint32 totNbrStrings = itemNames->getNbrStrings();
   ScopedArray< ScopedArray<char> > 
      stringsToSend( new ScopedArray<char>[ totNbrStrings ] ); 
   
   // Check all items and set the names
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         Item* item = theMap->getItem(z, i);
         if ( (item != NULL) &&
              ( 
                (item->getItemType() == ItemTypes::streetSegmentItem) ||
                (item->getItemType() == ItemTypes::individualBuildingItem) ||
                (item->getItemType() == ItemTypes::buildingItem) ||
                (item->getItemType() == ItemTypes::builtUpAreaItem) ||
                (item->getItemType() == ItemTypes::cityPartItem) ||
                (item->getItemType() == ItemTypes::islandItem) ||
                (item->getItemType() == ItemTypes::municipalItem) ||
                (item->getItemType() == ItemTypes::parkItem) ||
                (item->getItemType() == ItemTypes::railwayItem) ||
                (item->getItemType() == ItemTypes::zipCodeItem) ||
                (item->getItemType() == ItemTypes::waterItem) ||
                ( (item->getItemType() == ItemTypes::pointOfInterestItem) &&
                  (static_cast<PointOfInterestItem*>(item)
                   ->getPointOfInterestType() != ItemTypes::unknownType) &&
                  (static_cast<PointOfInterestItem*>(item)
                   ->getPointOfInterestType() != ItemTypes::company)
                )
              )) {
            for (uint32 j=0; j<item->getNbrNames(); j++) {
               uint32 index = item->getStringIndex(j);
               if (stringsToSend[index].get() == NULL) {
                  stringsToSend[index].reset(
                     StringUtility::newStrDup(
                        UTF8Util::mc2ToIso(
                           itemNames->getString(index) ).c_str() ) );
               }
            }
         }
      }
   }

   // Calculate size of strings
   uint32 n = 0;
   uint32 stringSize = 0;
   for (uint32 i=0; i<totNbrStrings; i++) {
      if (stringsToSend[i].get() != NULL) {
         stringSize += strlen(stringsToSend[i].get());
         n++;
      }
   }
   // Compensate for all the '\0's
   stringSize += totNbrStrings;

   // Allocate a temoporary block for the strings to send
   ScopedArray<char> allStrings( new char[stringSize] );
   char* allStringPointer = allStrings.get();
   for (uint32 i=0; i<totNbrStrings; i++) {
      if (stringsToSend[i].get() != NULL) {
         strcpy(allStringPointer, stringsToSend[i].get());
         mc2dbg8 << "   " << i << " " << allStringPointer << endl;
         allStringPointer += strlen(stringsToSend[i].get());
      }
      *allStringPointer = '\0';
      allStringPointer++;  
   }
   mc2dbg2 << "Will send " << n << " strings of totaly " 
           << totNbrStrings << " total size " << stringSize << endl;

   // Send string data
   DataBuffer dataBuffer(8);
   dataBuffer.writeNextLong(stringSize);
   dataBuffer.writeNextLong(totNbrStrings);
   if( socket->writeAll( dataBuffer.getBufferAddress(), 8) != 8 ) {
      mc2log << error
             <<"Error sending string data in StringTableMapGenerator()"
             << endl;
      return;
   }

   // Send strings
   if( socket->writeAll( (byte*)allStrings.get(), stringSize ) != 
       (int)stringSize )
   {
      mc2log << error
             <<"Error sending strings in StringTableMapGenerator()"
             << endl;
      return;
   }
   
   /*
   // Calculate data for the strings
   uint32 stringSize = itemNames->getStringSize();
   uint32 nbrStrings = itemNames->getNbrStrings();
   mc2dbg4 << "   sizeOfStrings = " << stringSize << endl;
   mc2dbg4 << "   nbrStrings = " << nbrStrings << endl;

   // Send string data
   DataBuffer dataBuffer(8);
   dataBuffer.writeNextLong(stringSize);
   dataBuffer.writeNextLong(nbrStrings);

   if( socket->writeAll( dataBuffer.getBufferAddress(), 8 ) != 8 ) {
      mc2log << error
             <<"Error sending string data in StringTableMapGenerator()"
             << endl;
      socket->close();
      delete socket;
      return;
   }

   // Send the strings
   for (uint32 i=0; i < itemNames->getNbrBlocks(); i++) {
      int curStringSize = itemNames->getBlockSize(i);
      mc2dbg4 << "Sending " << curStringSize << " bytes with strings" 
              << endl;
      if (socket->writeAll( (byte *) itemNames->getBlockStart( i ), 
                            curStringSize) != curStringSize )
      {
         mc2log << error
             <<"Error sending strings in StringTableMapGenerator()"
             << endl;
      }
   }
   */

   mc2dbg2 << "StringTableMapGenerator sent all data for map " 
           << mapID << ", processing time " 
           << TimeUtility::getCurrentTime()-startTime << " ms" << endl;
}


