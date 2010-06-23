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

#include "Packet.h"

#include <vector>
#include <utility>

class UserUser;
class DisturbanceListElement;
class DisturbanceVector;
class UserRightsMapInfo;

/**
 *   Request to be sent to the InfoModule to get all the
 *   disturbances on a map that are allowed for the user.
 */
class RouteTrafficCostRequestPacket : public RequestPacket {
public:
   /**
    *    Constructor (to be used in the server).
    *    @param user         The user.
    *    @param mapID        Map id to get the disturbances for.
    *    @param userDefined  Data that will be copied into the reply.
    */   
   RouteTrafficCostRequestPacket( const UserUser* user,
                                  uint32 mapID,
                                  uint32 userDefined = MAX_UINT32 );

   /**
    *    Returns the user defined data.
    */
   uint32 getUserDefData() const;

   /**
    *    Fills in the rights from the packet.
    */
   int getRights( UserRightsMapInfo& rights ) const;
   
private:
   /// Calculates the needed size of the packet buffer.
   static int calcPacketSize( const UserRightsMapInfo& rights );
   /**  The position of the user definded data. */
   static const int USER_DEF_POS = REQUEST_HEADER_SIZE;
   /**  The position where the rights start */
   static const int RIGHTS_POS = USER_DEF_POS + 4;
};

/**
 *   Reply to the RouteTrafficCostRequestPacket.
 */
class RouteTrafficCostReplyPacket : public ReplyPacket {
public:
   /**
    *    Constructor
    *    @param req    The request that caused the reply.
    *    @param status StringTable::stringCode
    *    @param dists  The disturbances to add to the packet.
    *                  The nodeID should be in first and the raw factor in
    *                  second.
    */
   RouteTrafficCostReplyPacket( const RouteTrafficCostRequestPacket* req,
                                uint32 status,
                                const vector<pair<uint32,uint32> >& dists );
   /**
    *    Returns the user defined data.
    */
   uint32 getUserDefData() const;

   /**
    *    Returns the mapID of the request / reply.
    */
   uint32 getMapID() const;

   /**
    *    Puts the traffic info in the vector.
    */
   uint32 getTraffic( DisturbanceVector& resVect, bool costC ) const;
      
private:
   /**  The position of the map id */
   static const int MAP_ID_POS   = REPLY_HEADER_SIZE;
   
   /**  The position of the user definded data. */
   static const int USER_DEF_POS = MAP_ID_POS + 4;

   /**  The position where the traffic data starts (length) */
   static const int NBR_TRAFFIC_DATA_POS = USER_DEF_POS + 4;
      
   /**
    *    Returns the size of the packet with the supplied data.
    */
   static int calcPacketSize( uint32 nbrDists );
};
