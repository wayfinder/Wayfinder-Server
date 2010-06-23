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

#include "MapGenerator.h"

#include "MapHandler.h"
#include "TCPSocket.h"

#include "MapPacket.h"

#include "SearchMapGenerator.h"
#include "RouteMapGenerator.h"
#include "GfxMapGenerator.h"
#include "GfxCountryMapGenerator.h"
#include "InfoMapGenerator.h"
#include "StringTableMapGenerator.h"

const int MapGenerator::generatorVersion = 26;

MapGenerator::MapGenerator( MapHandler *mh,
                            uint32 *mapIDs,
                            uint32 nbrMaps,
                            uint16 port ) {

   mapHandler = mh;
   
   mc2dbg2 << "MapGenerator created" << endl;
   
   // If mh == NULL the MapGenerator cannot be run but generateDataBuffer
   // can.
   if ( mh ) {
      mapHandler = mh;
      this->nbrMaps = nbrMaps;
      this->mapIDs = new uint32[nbrMaps];
      for (uint32 i=0; i<nbrMaps; i++) {
         this->mapIDs[i] = mapIDs[i];
      }
      
      // Create and open an TCP-socket
      theSocket = new TCPSocket();
      if (!theSocket->open()) {
         mc2log << fatal
                << "MapGenerator->generateMap(): Failed to open theSocket"
                << endl;
         delete theSocket;
         theSocket = NULL;
         PANIC("theSocket = NULL", "")
            }
      
      // NOT hanging (bind and listen)
      this->port = theSocket->listen(port, TCPSocket::FINDFREEPORT);
   } else {
      this->mapIDs     = NULL;
      this->mapHandler = NULL;
      this->nbrMaps    = 0;
   }
}

MapGenerator*
MapGenerator::createGenerator(MapProcessor* mapProc,
                              MapHandler* mh,
                              uint32 mapType,
                              uint32 mapID,
                              uint16 freePort,
                              uint32 zoomLevel,
                              const vector<uint32>& overview)
{
   MapGenerator* mapGenerator = NULL;
   switch ( mapType ) {
      case MapRequestPacket::MAPREQUEST_SEARCH:
         mapGenerator = new SearchMapGenerator(mh,
                                               &mapID, 
                                               freePort);
         break;
         
      case MapRequestPacket::MAPREQUEST_ROUTE: {
         mapGenerator = new RouteMapGenerator(mh, 
                                              &mapID,
                                              1,
                                              freePort,
                                              overview);
      }
      break;
      
      case MapRequestPacket::MAPREQUEST_INFO:
         mapGenerator = new InfoMapGenerator(mh,
                                             &mapID,
                                             1,
                                             freePort,
                                             mapProc);
         break;
         
      case MapRequestPacket::MAPREQUEST_GFX : {
         mapGenerator = new GfxMapGenerator( mh, 
                                             &mapID, 
                                             freePort,
                                             zoomLevel );
      }
      break;
      
      case MapRequestPacket::MAPREQUEST_GFXCOUNTRY :
         mapGenerator = new GfxCountryMapGenerator( mh, 
                                                    &mapID, 
                                                    freePort);
         break;
         
      case MapRequestPacket::MAPREQUEST_STRINGTABLE :
         mapGenerator = new StringTableMapGenerator( mh, 
                                                     &mapID, 
                                                     freePort);
         break;
         
   }
   return mapGenerator;
}

MapGenerator*
MapGenerator::createGeneratorForDataBuffer(uint32 mapType)
{
   switch ( mapType ) {
      case MapRequestPacket::MAPREQUEST_SEARCH:
      case MapRequestPacket::MAPREQUEST_ROUTE:
         return createGenerator( NULL, NULL, mapType, 0, 0, 0,
                                 vector<uint32>());
      default:
         // Not supported type
         return NULL;
   }
   return NULL;
}

MapGenerator::~MapGenerator()
{
   if ( mapHandler ) {
      delete theSocket;
      mc2dbg4 << "   theSocket deleted" << endl;
      
      // Very important as MapProcessor will hang if this isn't released
      mapHandler->readReleaseMap(mapIDs[0]);
      mc2dbg4 << "ReadMapLock on map " << mapIDs[0] <<" released." << endl;
      
      delete [] mapIDs;
      mc2dbg4 << "   mapIDs deleted" << endl; 
      mc2dbg4 << "   MapGenerator destroyed" << endl;
   }
}

TCPSocket*
MapGenerator::waitForConnection(char* handshake,
                                int maxHandshake)
{
   mc2dbg4 << "theSocket->accept(" << MAX_WAIT_TIME_FOR_CONNECTION
           << ")\n";
   TCPSocket *socket = theSocket->accept(MAX_WAIT_TIME_FOR_CONNECTION);
  
   if (socket != NULL) {
      mc2dbg4 << "MapGen::waitforconn  - Connected!!! Reading" << endl;
      int  nbrBytes = 0;
      int  handshakePos = 0;
      int  totNbrBytes = 0;
      const int inbufsize = 255;
      byte inbuf[inbufsize];
      
      do {
         nbrBytes = socket->readMaxBytes(inbuf, inbufsize);
         mc2dbg4 << "RouteMapGenerator()->generateMap(): read "
                 << nbrBytes << " bytes (" << inbuf << ")" << endl;
         totNbrBytes += nbrBytes;
         int bytestocopy = MIN(nbrBytes, (maxHandshake - 1) - handshakePos);
    
         if ( bytestocopy > 0 ) {
            memcpy(&handshake[handshakePos], inbuf, bytestocopy);
            handshakePos += bytestocopy;
         }
      } while ( nbrBytes == inbufsize );
      handshake[MIN(handshakePos, maxHandshake-1)] = 0;      
   } else {
      mc2log << warn << "MapGenerator()->waitForConnection(): No connection";
      delete socket;
      return NULL;
   }
   return socket;
}

bool
MapGenerator::getIndexDBNeeded() const
{
   return false;
}

DataBuffer*
MapGenerator::generateDataBuffer(const GenericMap* theMap,
                                 const MapModuleNoticeContainer* indexdb)
{
   return NULL;
}

void
MapGenerator::run() {
;
}


