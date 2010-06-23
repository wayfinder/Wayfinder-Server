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

#include "InfoMapGenerator.h"

#include "MapProcessor.h"
#include "TCPSocket.h"

#include "CoordinatePacket.h"
#include "DataBuffer.h"
#include "IDTranslationTable.h"
#include "OverviewMap.h"
#include "TimeUtility.h"
#include "MapBits.h"

InfoMapGenerator::InfoMapGenerator(MapHandler* mh,
                                   uint32* mapIDs,
                                   int nbrMaps,
                                   uint32 port,
                                   MapProcessor* mapProc)
      : MapGenerator( mh,
                      mapIDs,
                      nbrMaps,
                      port),
   m_mapProcessor(mapProc)
{
}

InfoMapGenerator::~InfoMapGenerator()
{
   // Nothing
}

TCPSocket*
InfoMapGenerator::waitForConnection(char* handshake,
                                    int maxHandshake)
{
   mc2dbg << "theSocket->accept(" << MAX_WAIT_TIME_FOR_CONNECTION
           << ")\n";
   TCPSocket *socket = theSocket->accept(MAX_WAIT_TIME_FOR_CONNECTION);
   
   if (socket != NULL) {
      mc2dbg << "InfoMapGen::waitforconn  - Connected!!! Reading" << endl;
      
      byte buffer;
      int totalRead = 0;

      // Result from read operations.
      int res = 0;
      do {
         res = socket->readExactly(&buffer, 1);
         ++totalRead;
         // Write into handshake string
         if ( totalRead < maxHandshake )
            handshake[totalRead-1] = buffer;
         
      } while ( res > 0 && buffer != 0 );
      
      handshake[maxHandshake-1] = 0; // Zero terminate
      // Check for error.
      if ( res <= 0 ) {
         delete socket;
         return NULL;
      } else {
         return socket;
      }
   } else {
      mc2dbg << "InfoMapGenerator()->waitForConnection(): No connection"
              << endl;
      delete socket;
      return NULL;
   }
   return socket;
}


void
InfoMapGenerator::run()
{
   const int maxHandshake = 1024;
   char handshake[maxHandshake];
   TCPSocket* socket = waitForConnection(handshake, maxHandshake);
   if ( socket == NULL ) {
      mc2log << warn << "No connection for InfoMapGenerator" << endl;
      return;
   } else {
      mc2dbg << "Got connection (\"" << handshake << "\")" << endl;
   }
   uint32 startTime = TimeUtility::getCurrentTime();
   
   // Now the InfoModule will get the coordinates from the database
   // Hope it will be finished in a short amount of time.
   
   int res = 0;
   DataBuffer sizeBuf(4);
   uint32 nbrCoordinates = 0;
   mc2dbg4 << "Reading..." << endl;
   res = socket->readExactly( sizeBuf.getBufferAddress(), 4 );
   mc2dbg4 << "Reading done.." << endl;
   nbrCoordinates = sizeBuf.readNextLong();
   // Question contains 4 bytes lat
   //                   4 bytes lon
   //                   4 bytes angle 0-359
   const int qSize = nbrCoordinates * 12;
   DataBuffer qBuf(qSize);
   const int ansSize = nbrCoordinates * 20;
   // Answer contains 4 bytes mapID (maybe unneccesary) 0xffffffff = err
   //                 4 bytes nodeID                    0xffffffff = err
   //                 4 bytes distance in meters
   //                 4 bytes offset
   DataBuffer ansBuf(ansSize);
   res = 0;
   int keept = 0;
   int removed = 0;
   if( nbrCoordinates > 0 ) {
      res = socket->readExactly( qBuf.getBufferAddress(), qSize );
      uint32 startTime = TimeUtility::getCurrentMicroTime();
      if ( res == qSize ) {
         mc2dbg4 << "********************* nbrCoordinates "
              << (int)nbrCoordinates << " ***************" << endl;
         for(uint32 i = 0; i < nbrCoordinates; i++) {
            const int lat = qBuf.readNextLong();
            const int lon = qBuf.readNextLong();
            const uint32 angle = qBuf.readNextLong();
            mc2dbg4 << "Got lat = " << lat << " lon = " << lon
                    << " angle " << angle << endl;
            
            const uint32 mapID = mapIDs[0];
            uint32 itemID = MAX_UINT32;
            uint32 itemID2 = MAX_UINT32;
            uint32 distance = MAX_UINT32;
            bool dirFromZero = true;
            uint32 offset = MAX_UINT32;
            
            bool result = false;
            if (angle == 32767) {
               result = m_mapProcessor->
                  processOmniInfoCoordinateRequest(mapIDs[0],
                                                   lat,
                                                   lon,
                                                   angle,
                                                   COORDINATE_PACKET_RESULT_ANGLE,
                                                   itemID,
                                                   itemID2,
                                                   distance,
                                                   dirFromZero,
                                                   offset);
            } else {
               result = m_mapProcessor->
                  processInfoCoordinateRequest(mapIDs[0],
                                               lat,
                                               lon,
                                               angle,
                                               COORDINATE_PACKET_RESULT_ANGLE,
                                               itemID,
                                               distance,
                                               dirFromZero,
                                               offset);
            }

            if ( result && angle < 361 ) {
               if ( !dirFromZero ){
                  itemID |= 0x80000000;
                  // offset = ?;
               }
               
               
               ansBuf.writeNextLong(mapID);
               ansBuf.writeNextLong(itemID);
               ansBuf.writeNextLong(MAX_UINT32);
               ansBuf.writeNextLong(distance);
               ansBuf.writeNextLong(offset);
               keept++;
            } else if( result && (angle == 32767) ) {
               ansBuf.writeNextLong(mapID);
               if (itemID == itemID2) {
                  ansBuf.writeNextLong(itemID & 0x7fffffff);
                  ansBuf.writeNextLong(itemID | 0x80000000);
               } else {
                  ansBuf.writeNextLong(itemID);
                  ansBuf.writeNextLong(itemID2);
               }
               ansBuf.writeNextLong(distance);
               ansBuf.writeNextLong(offset);
               keept++;
            } else {
               ansBuf.writeNextLong(MAX_UINT32);
               ansBuf.writeNextLong(MAX_UINT32);
               ansBuf.writeNextLong(MAX_UINT32);
               ansBuf.writeNextLong(MAX_UINT32);
               ansBuf.writeNextLong(MAX_UINT32);
               removed++;
            }
         }
         res = socket->writeAll( ansBuf.getBufferAddress(), ansSize );
         qBuf.reset();
         ansBuf.reset();
         int averageTime = (TimeUtility::getCurrentMicroTime()-startTime)/
            (nbrCoordinates);
         mc2dbg4 << "TIME: "<< nbrCoordinates
                <<" Coordinate Reauest & Reply handled in " 
                << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0 
                << " ms" << endl << "Average time = " << averageTime
                << " us"  << endl;
         mc2dbg4 << "keept " << keept << "removed "  << removed << endl;
      }
   }
   /*
   // IDTranslation table
   // Send the translation-table if this is an overviewmap!
   GenericMap* theMap = mapHandler->getMap(mapID);
   
   if (MapBits::isOverviewMap(mapID)) {
      const OverviewMap* overview = dynamic_cast<const OverviewMap*>(theMap);
      if (overview != NULL) {
         
         IDTranslationTable transTable;
         
         // We only want to send the routeable items.
         uint32 overviewID, trueMapID, trueItemID;
         uint32 totNbrOverviewItems = overview->getNbrOverviewItems();
         for (uint32 i=0; i<totNbrOverviewItems; i++) {
            Item* curOverviewItem = overview->getOverviewItem(i, 
                                                              overviewID, 
                                                              trueMapID, 
                                                              trueItemID);
            switch ( curOverviewItem->getItemType() ) {
               case ItemTypes::streetSegmentItem:
               case ItemTypes::ferryItem:
                  transTable.addElement(overviewID, trueMapID, trueItemID);
                  break;
               default:
                  break;
            }
         }
         
         transTable.sortElements();
         
         MC2_ASSERT( transTable.sanityCheck() );
         
         const bool sendAsTwoTables = true;
         DataBuffer translationBuffer(
            transTable.dataBufferSize(sendAsTwoTables)+4);
         translationBuffer.writeNextLong(
            transTable.dataBufferSize(sendAsTwoTables) );
         transTable.save(&translationBuffer, sendAsTwoTables);           
         if ( socket->write( translationBuffer.getBufferAddress(), 
                             translationBuffer.getCurrentOffset() ) < 0)
         {
            mc2log << error << here 
                   << "RouteMapGenerator:: Error sending translationbuffer" 
                   << endl;
         }
      }
   }
   */
   uint32 stopTime = TimeUtility::getCurrentTime();
   mc2dbg4 << "No more stuff to send (\"" << handshake << "\")"
           << " t=" << (stopTime-startTime) << " ms"
           << endl;
   
   delete socket;
}





