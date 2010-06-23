/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPGENERATOR_H
#define MAPGENERATOR_H

#include "config.h"

#include <vector>

#include "ISABThread.h"

class TCPSocket;
class MapHandler;
class MapProcessor;
class GenericMap;
class MapModuleNoticeContainer;
class DataBuffer;

/**
  *   Subclasses of this class extracts an map and sends it via 
  *   socket to the module or server that asked for it.
  *
  */
class MapGenerator : public ISABThread {
   /**
     *   The time we wait for connection on the TCP-socket (us).
     */
   #define MAX_WAIT_TIME_FOR_CONNECTION 10000000

   public:

      /**
       *    MapGenerator version. Used when loading maps.
       *    Change this when changing the format of any of 
       *    the map generators.
       */
      static const int generatorVersion;

      /**
       *    Creates a new MapGenerator with the correct dynamic
       *    type. This function is for use in the MapProcessor.
       */
      static MapGenerator* createGenerator(MapProcessor* mapProc,
                                           MapHandler* mh,
                                           uint32 mapType,
                                           uint32 mapID,
                                           uint16 freePort,
                                           uint32 zoomLevel,
                                           const vector<uint32>& overview);

      /**
       *    Creates a new MapGenerator for the supplied type of map.
       *    To be used when creating maps for caching, i.e.
       *    generateDataBuffer will be used instead of start.
       *    @param mapType Type of mapGenerator to create.
       *    @return NULL if not supported.
       */
      static MapGenerator* createGeneratorForDataBuffer(uint32 mapType);
      
      /**
        *   Creates one MapGenerator that generates a map for the
        *   specified area. This is an abstract class, it is the 
        *   subclasses that actually generates the map.
        *
        *   @param   mh The MapHandler to use to retreiving the Map:s
        *   @param   mapIDs ID of the maps to use.
        *   @param   nbrMaps How many maps in mapID.
        *   @param   port The port to wait for connection.
        */
      MapGenerator(MapHandler *mh,
                   uint32 *mapIDs, 
                   uint32 nbrMaps, 
                   uint16 port);

      /**
       *    Returns true if the MapModuleNoticeContainer is needed in
       *    generateDataBuffer. If this function returns false NULL
       *    can be sent in.
       *    @return True if index.db is needed in generateDataBuffer.
       */
      virtual bool getIndexDBNeeded() const;

      /**
       *    Generates the DataBuffer that is sent to the module.
       *    Not implemented for all modules.
       *    The DataBuffer should be deleted by the caller.
       *    @param theMap  The map to generate the modulemap for.
       *    @param indexdb MapModuleNoticeContainer needed if getIndexDBNeeded
       *                   returns true.
       */
      virtual DataBuffer*
          generateDataBuffer(const GenericMap* theMap,
                             const MapModuleNoticeContainer* indexdb);
   
      /**
        *   Deletes the MapGenerator and releases the used memory.
        *   If this destructor isn't used in a subclass, remember to release
        *   the WriteLock of the map.
        */
      virtual ~MapGenerator();

      /**
        *   Waits for a connection to port, in an amount of time,
        *   responds with the map asked for. If no connection in time
        *   the thread kills it self.
        */
      virtual void run();

      /**
        *   Get the port that this map generater uses.
        *   @return  The port this MapGenerator is binded to.
        */
      inline uint16 getPortInUse();

      /**
        *   Get a ID of one map that should be processed.
        *
        *   @param   The cardinal number of the map.
        *   @return  mapID of map number i. If i is an invalid number
        *            MAX_UINT32 is returned. 
        */
      inline uint32 getMapID(uint32 i);


   protected:
      
      /**
       *    Waits for a connection to write the map to. Waits
       *    <code>MAX_WAIT_TIME_FOR_CONNECTION</code> for a connection
       *    and if there is one in time, the function will read data
       *    from the socket until no more can be read. Then it will
       *    return the socket.
       *    @param handshake    A string to put the handshake into.
       *    @param maxHandshake Maximum number of characters to put
       *                        into the handshake string.
       *    @return A socket if connected. NULL if failure.
       */
      TCPSocket* waitForConnection(char* handshake,
                                   int maxHandshake);
      /**
        *   The maphandler that should be used for retreiving maps etc.
        */
      MapHandler *mapHandler;

      /**
        *   The maps this extracter should use.
        */
      uint32 *mapIDs;

      /**
        *   The number of maps in the maps-vector.
        */
      uint32 nbrMaps;

      /**
        *   The port to wait for a connection.
        */
      uint16 port;

      /**
        *   The socket that the run-method should do accept on.
        */
      TCPSocket *theSocket;
};

// =======================================================================
//                                 Implementation of the inlined methods =

inline uint16 
MapGenerator::getPortInUse() 
{
   return (port);
};

inline uint32 
MapGenerator::getMapID(uint32 i) 
{
   if (i<nbrMaps)
      return (mapIDs[i]);
   else
      return (MAX_UINT32);
};




#endif

