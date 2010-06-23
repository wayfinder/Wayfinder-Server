/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCH_PROCESSOR_H
#define SEARCH_PROCESSOR_H

#include "config.h"


#include "MapHandlingProcessor.h"

#include <map>

class SearchExpandItemReplyPacket;
class SearchExpandItemRequestPacket;

class SearchRequestPacket;
class VanillaSearchReplyPacket;
class ReplyPacket;

class StringSearchUnit;

// - New
class SearchableSearchUnit;


class IDPair_t;

/**
 *   
 */
class SearchProcessor : public MapHandlingProcessor {
  public:
   /**
     *   Creates a new SearchProcessor.
     *   @param   loadedMaps  A SafeVector that is used as communication
     *                        between the Processor and the Reader.
     *                        Contains ID of all maps that are loaded
     *                        by this SearchProcessor.
     */
   SearchProcessor(MapSafeVector* loadedMaps);

   /**
     *   Deletes the SearchProcessor.
     */
   virtual ~SearchProcessor();

   /**
     *   Implements the virtual method in the base class, Processor.
     *   This is currently not used (and not really implemented).
     */
   virtual int getCurrentStatus();

    /**
     *    Virtual function for loading a map. Should be called
     *    by handleRequest when that function gets a loadMapRequest
     *    and isn't virtual anymore.
     *    @param mapID The map to load.
     *    @param mapSize Outparameter describing the size of the
     *                   map.
     *    @return StringTable::OK if ok.
     */
   virtual StringTable::stringCode loadMap(uint32 mapID,
                                           uint32& mapSize);
   
   /**
    *    Virtual function to be called when a map should
    *    be deleted.
    *    @param mapID The map to be deleted.
    *    @return StringTable::OK if ok.
    */
   virtual StringTable::stringCode deleteMap(uint32 mapID);

   /**
    *    Current alarm limit.
    */
   static uint32 alarmLimit;

   /**
    *    Alarm handling function.
    *    Will quit with panic.
    */
   static void handleAlarm(int iMsg);

protected:
   /**
     *   Processes the incoming packets. The RequestPacket is given as 
     *   parameter and the ReplyPacket is returned. If the reply is
     *   created with new, the request packet must be deleted!
     *
     *   @param   p  The request packet that should be processed.
     *   @param   packetInfo A string to fill with info about the packet.
     *            Will be printed by JobThread. <b>WARNING</b> GMSMap sends
     *            in NULL here, so you have to check.
     *   @return  A (reply)Packet that is the reply to the given request.
     */
   virtual Packet* handleRequestPacket( const RequestPacket& p,
                                        char* packetInfo );


   /// the number of packets received
   uint32 m_nbrPackets;
      
   /// the number of packets returned with status != ok.
   uint32 m_nbrFailedPackets;
      
private:

   /**
    *    Handles the SearchExpandItemRequestPacket.
    *    @param req The request.
    *    @return The reply.
    */  
   SearchExpandItemReplyPacket* handleSearchExpandItem(
      const SearchExpandItemRequestPacket*
      req);

   
   /**
    *    Handles the new SearchRequestPackets.
    *    @param req        The SearchRequestPacket.
    *    @param packetInfo String to put JobThread output in.
    *    @return ReplyPacket since OverviewSearches return
    *            OverviewSearchReplyPacket.
    */
   ReplyPacket* handleSearchRequestPacket(
      const SearchRequestPacket* req,
      char* packetInfo);
   
   /**
    *   Gets the SearchUnit that holds a specific mapID.
    *   @param mapID The mapID.
    *   @return A pointer to the SearchUnit or NULL if not found.
    */
   const SearchableSearchUnit* getSearchUnitByMapID(uint32 mapID) const;
   
   /**
    *   Sets the alarm timer.
    *   If set to 0, the alarm is off.
    *   Once the alarm is triggered, handleAlarm will kill the program,
    *   assuming it has hanged.
    *   @param seconds The time in seconds to the alarm.
    */
   void setAlarm(uint32 seconds);

   // - Maps map-ids to search units.
   map<uint32, SearchableSearchUnit*> m_searchUnits;
      
};

#endif   // SEARCH_PROCESSOR_H

