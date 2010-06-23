/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRAFFIC_IPC_H
#define TRAFFIC_IPC_H

#include "config.h"
#include "TrafficDataTypes.h"
#include "MC2String.h"

#include <vector>
#include <map>

class ParserThread;
class PacketContainer;
class RouteRequest;
class MC2Coordinate;
class DisturbanceElement;
class IDPair_t;
class TrafficSituation;

/**
 * Interface for a traffic inter process communication (ipc) that communicates
 * traffic changes to a destination process/module.
 */
class TrafficIPC {
public:

   /// Holds disturbances
   typedef std::vector< DisturbanceElement* > Disturbances;   

   /**
    * Sends the change set to the Info module.
    * @param newElements Situations to be updated.
    * @param removedElements Situations to be removed.
    * @return true if the send was succesful.
    */
   virtual bool sendChangeset( const Disturbances& newElements,
                               const Disturbances& removedElements ) = 0;
   /**
    * Get all disturbances from a specific \c provider.
    * @param provider A unique provider ID.
    * @param disturbances Will contain all the disturbances from the \c provider
    *                     if the fetch was successful.
    * @return True if the fetch was successful.
    */
   virtual bool getAllDisturbances( const MC2String& provider,
                                    Disturbances& disturbances ) = 0;

   /// A vector of PacketContainer pointers.
   typedef std::vector< PacketContainer* > PacketContainers;
   /**
    * Sends PacketContainer's and if successful it will contains the answers in
    * the PacketContainer's
    * @param pcs The PacketContainers.
    * @return true if the send was fuccesful.
    */
   virtual bool sendPacketContainers( PacketContainers& pcs ) = 0;

   typedef uint32 RouteIndex;
   typedef std::pair< MC2Coordinate, RouteIndex > CoordPair;
   typedef std::map< IDPair_t, CoordPair > IDPairsToCoords;

   /**
    * Gets all the valid mapIDs, nodeIDs, coordinates and routeIndexes for a
    * TrafficSituation.
    *
    * @param traffSit The TrafficSituation.
    * @param idPairsToCoords The id pair to coords map for this situation.
    */
   virtual void getMapIDsNodeIDsCoords( const TrafficSituation& traffSit,
                                        IDPairsToCoords& idPairsToCoords ) = 0;

};

#endif // TRAFFIC_IPC_H
