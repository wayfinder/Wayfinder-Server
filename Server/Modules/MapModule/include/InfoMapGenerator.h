/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INFOMAPGENERATOR_H
#define INFOMAPGENERATOR_H

#include "MapGenerator.h"

class MapProcessor;

/**
 *    Class for sending maps to the <code>InfoModule</code>. The loading
 *    procedure starts with a handshake terminated by '\0'. After the
 *    handshake the InfoModule sends the MapID. The InfoMapGenerator
 *    responds with the boundingbox for the map. After this the
 *    InfoModule gets the disturbances in the database which are inside
 *    the bbox and sends the coordinates and angles of the disturbances
 *    to the InfoMapGenerator. The InfoMapGenerator responds with the
 *    itemID for the best StreetSegmentItem. It also sends the distance
 *    from the received coordinate.
 *
 */
class InfoMapGenerator : public MapGenerator {
   
public:

   /**
    *   Creates a InfoMapGenerator, du.
    *
    *   @param mh      MapHandler for getting the maps.
    *   @param mapID   The mapIDs to send.
    *   @param nbrMaps The number of mapID:s.
    *   @param port    The port to try to listen to.
    *   @param mapProc Pointer to the MapProcessor.
    */
   InfoMapGenerator(MapHandler* mh,
                    uint32* mapID,
                    int nbrMaps,
                    uint32 port,
                    MapProcessor* mapProc);
   
   /**
    *    Delete this map generator.
    */
   virtual ~InfoMapGenerator();
   
   /**
    *   Performs the actual generation of the map.
    */
   virtual void run();

protected:

   /**
    *    Waits for a connection to write the map to. Waits
    *    <code>MAX_WAIT_TIME_FOR_CONNECTION</code> for a connection
    *    and if there is one in time, the function will read data
    *    from the socket until a zero occurs. This function is different
    *    from the waitForConnection in MapGenerator, since we must know
    *    exactly when the handshake ends, since data is sent from the
    *    module on the same socket as the handshake.
    *    @param handshake    A string to put the handshake into.
    *    @param maxHandshake Maximum number of characters to put
    *                        into the handshake string.
    *    @return A socket if connected. NULL if failure.
    */
   TCPSocket* waitForConnection(char* handshake,
                                int maxHandshake);
private:

   /** MapProcessor is used for some functions */
   MapProcessor* m_mapProcessor;
   
};

#endif
