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
#include <vector>
#include "MapRights.h" //needed for the default value in constructor. 
#ifndef USERRIGHTSMAPINFO_H
#define USERRIGHTSMAPINFO_H

class UserUser;
class Packet;
class IDTranslationTable;
class IDPair_t;

class UserRightsMapInfo {
public:
   /**
    *    Creates UserRightsMapInfo suitable for sending to a module
    *    with the supplied mapID.
    *    (This is for use in packets created in the  server).
    *    @param mapID The mapid that this is about.
    *    @param user  The user with an updated user right cache.
    *    @param mask  Will be used to remove rights that are not interesting.
    */
   UserRightsMapInfo( uint32 mapID,
                      const UserUser* user,
                      const MapRights& mask = ~MapRights());

   /**
    *    Creates UserRightsMapInfo and loads it from a packet.
    *    For use in the module (or packets received there).
    */
   UserRightsMapInfo( const Packet* packet, int& pos );

   /**
    *    Creates empty info.
    */
   UserRightsMapInfo();

   /**
    *    Creates an UserRightsMapInfo with one set of rights on one
    *    map.
    */
   UserRightsMapInfo( uint32 mapID, const MapRights& mapRights );

   /// Swaps the contents of this UserRightsMapInfo with other.
   void swap( UserRightsMapInfo& other );
   
   /**
    *    Returns true if the item is allowed.
    *    @param itemUR     User rights needed to use the item.
    *    @param itemID     ItemID on the current map.
    *    @param transtable Translation table to be used if the map is
    *                      an overview map.
    */
   bool itemAllowed( const MapRights& itemUR,
                     const IDPair_t& itemID,
                     const IDTranslationTable& transtable ) const;

   /**
    *    Returns true if the UserRightsMapInfo is empty.
    */
   bool empty() const;
   
   /**
    *    Loads the UserRightsMapInfo from the packet and updates
    *    pos.
    */
   int load( const Packet* packet, int& pos );

   /**
    *    Saves the UserRightsMapInfo into the packet and updates
    *    pos.
    */
   int save( Packet* packet, int& pos ) const;

   /**
    *    Returns the number of bytes that the UserRightsMapInfo
    *    will use in the packet.
    */
   int getSizeInPacket() const;

   /**
    *    Returns the mapID of the first element in the rights vector.
    */
   uint32 getFirstAllowedMapID() const;
   
   /**
    *    Returns the MapRights for the specified map.
    */
   inline MapRights getURForMap( uint32 mapID ) const;

   /// Prints the info on the ostream.
   friend ostream& operator<<( ostream& o, const UserRightsMapInfo& ur );


   void filterAllRights();
   
private:

   /**
    *    The id of this map.
    */
   uint32 m_mapID;

   /**
    *
    */
   typedef vector<pair<uint32, MapRights> > rightsVector_t;
   
   /**
    *    Sorted vector with map id in first and the rights in second.
    */
   rightsVector_t m_rights;
};

#endif // USERRIGHTSMAPINFO_H
