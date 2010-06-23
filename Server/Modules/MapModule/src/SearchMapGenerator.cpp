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

#include "SearchMapGenerator.h"
#include "MapHandler.h"

#include "TCPSocket.h"
#include "DataBuffer.h"

#include "SearchUnitBuilder.h"
#include "SearchUnit.h"
#include "GenericMap.h"

#include "MapBits.h"

// Seems like there is some kind of memory leak in the generation
// of SearchUnits (or at least some kind of fragmentation, because
// MapModule grows rather big when building the SearcUnit and never
// shrinks again, but valgrind does not show anything).
// To avoid this, use fork. It also has the advantage that we do not
// have to wait for the deletion of all the garbage created in the
// process.

#undef  USE_FORK

#ifdef  USE_FORK
#include <sys/types.h>
#include <sys/wait.h>
#endif

SearchMapGenerator::SearchMapGenerator(MapHandler *mh,
                                       uint32 *mapIDs, 
                                       uint32 port) 
                                       
   : MapGenerator(  mh, mapIDs, 1, port)
{
   m_searchUnit = NULL;
   m_builder    = NULL;
   if ( mapIDs ) {
      mc2dbg << "[SMG]: SearchMapGenerator for map "
             << prettyMapIDFill(mapIDs[0])
             << " created" << endl;
   }
}


SearchMapGenerator::~SearchMapGenerator()
{
   if ( mapIDs ) {
      mc2dbg << "[SMG]: SearchMapGenerator for map "
             << prettyMapIDFill(mapIDs[0])
             << " destroyed" << endl;
   }
   delete m_searchUnit;
   m_searchUnit = NULL;
   delete m_builder;
   m_builder = NULL;
}

void
SearchMapGenerator::run()
{
   // If fork is used the generation can be sped up since the
   // garbage does not have to be deleted. But since the generation
   // is so slow anyway...
#ifdef USE_FORK
   pid_t pid = fork();
   if ( pid == 0 ) {
      // Child process
      sendMap(false); // Don't delete garbage
      exit(0);
   } else {
      // Parent process
      int status;
      waitpid(pid, &status, 0);
      mc2dbg << "[SMG]: Child exited with status = " << status << endl;
   }
#else
   // We have to delete all garbage.
   sendMap(true);
#endif
}

DataBuffer*
SearchMapGenerator::generateDataBuffer(const GenericMap* theMap,
                                       const MapModuleNoticeContainer* indexdb)
{
   uint32 startTime = TimeUtility::getCurrentTime();
   delete m_searchUnit;
   delete m_builder;
   
   uint32 curMapID = theMap->getMapID();
   // Do stuff here. Remove the builder etc in the destructor
   // to avoid too long waiting times.
   m_builder = new SearchUnitBuilder;
   if ( ! MapBits::isCountryMap( curMapID ) ) {
      m_builder->addMap(*theMap);
      mc2dbg << "[SMG]: Map " << prettyMapIDFill(curMapID) << " added" << endl;
   }
   
   m_searchUnit = m_builder->createSearchUnit();
   uint32 stopTime  = TimeUtility::getCurrentTime();
   DataBuffer* bufferToSend =
      new DataBuffer(m_searchUnit->getSizeInDataBuffer() + 4);
   bufferToSend->fillWithZeros();
   bufferToSend->writeNextLong(m_searchUnit->getSizeInDataBuffer());
   m_searchUnit->save(*bufferToSend);
   
   mc2dbg << "[SMG]: Buffer creation took " 
          << uint32(stopTime-startTime) << " millis" << endl;

   return bufferToSend;
}

void
SearchMapGenerator::sendMap(bool deleteGarbage)
{
   const int maxHandshake = 1024;
   char handshake[maxHandshake];
   
   TCPSocket* sock = waitForConnection(handshake, maxHandshake);
   
   if ( sock == NULL ) {
      mc2log << error << "No connection for SearchMapGenerator" << endl;
      return;
   }
   
   mc2dbg << "[SMG]: SearchModule says \"" << handshake
          << '"' << endl;
   
   uint32 ip;
   uint16 port;
   sock->getPeerName(ip, port);
   
   uint32 mapID = getMapID(0);
   
   {
      GenericMap* theMap = mapHandler->getMap(getMapID(0));
      
      // Create the databuffer
      DataBuffer* bufferToSend = generateDataBuffer(theMap, NULL);
            
      int bufSize = 0;
      int res = 0;
      if ( bufferToSend ) {
         bufSize = bufferToSend->getCurrentOffset();
         // FIXME: Add return value check
         res = sock->writeAll( bufferToSend->getBufferAddress(),
                               bufSize );
         if ( res != bufSize ) {
            mc2log << warn << "[SMG]: Warning - error when sending map "
                   << strerror(errno) << endl;
         }
         
      }
      
      mc2log << info << "[SMG]: " << res << " bytes for map "
             << prettyMapIDFill(mapID) << " sent to SearchModule at "
             << prettyPrintIP(ip) << ":" << port << endl;
      
      // Delete the buffer and search unit.
      if ( deleteGarbage ) {
         delete bufferToSend;
         delete m_searchUnit;
         m_searchUnit = NULL;
         delete m_builder;
         m_builder = NULL;
      }
   }
   delete sock;
   mc2dbg << "[SMG]: Garbage deleted - sendMap will end" << endl;   
}



