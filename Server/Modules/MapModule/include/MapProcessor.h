/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAP_PROCESSOR_H
#define MAP_PROCESSOR_H

#include "config.h"

#include "StringTable.h"
#include "ItemTypes.h"
#include "MapHandlingProcessor.h"

#include "GetAdditionalPOIInfo.h"
#include <memory>

class MapSafeVector;
class Packet;
class RequestPacket;
class ReplyPacket;
class MapHandler;
class ExpandRouteProcessor;

class CoordinateOnItemReplyPacket;
class CoordinateOnItemRequestPacket;
class CoveredIDsReplyPacket;
class CoveredIDsRequestPacket;
class TrafficPointReplyPacket;
class TrafficPointRequestPacket;
class ExpandItemReplyPacket;
class ExpandItemRequestPacket;
class RouteExpandItemReplyPacket;
class RouteExpandItemRequestPacket;
class StreetSegmentItemReplyPacket;
class StreetSegmentItemRequestPacket;
class IDTranslationReplyPacket;
class IDTranslationRequestPacket;
class CoordinateReplyPacket;
class CoordinateRequestPacket;
class MatchInfoRequestPacket;
class MatchInfoReplyPacket;
class ItemInfoRequestPacket;
class ItemInfoReplyPacket;
class QueueStartRequestPacket;
class QueueStartReplyPacket;
class ExpandRequestPacket;
class ExpandReplyPacket;
class InfoCoordinateReplyPacket;
class InfoCoordinateRequestPacket;

class Item;
class GenericMap;
class StreetSegmentItem;
class Node;

class SearchMatch;
class UserRightsMapInfo;
class VanillaMatch;
class VanillaStreetMatch;
class VanillaRegionMatch;

/**
  *   Processes requestpackets that is amied for the MapModule.
  * 
  */
class MapProcessor : public MapHandlingProcessor {
public:
   // category id type
   typedef uint16 CategoryID;
   /// maps category to string
   typedef std::map< CategoryID, MC2String> CategoryStrings;
   /// maps category to its translations
   typedef std::map< LangTypes::language_t, 
                     CategoryStrings > CategoryTranslationMap;


      /**
        *   @name The interval to use when finding free ports.
        */
      //@{
         /** The first port in the interval */
         #define FIRST_PORT_NUMBER 7000

         /** The last port in the interval */
         #define LAST_PORT_NUMBER 7500
      //@}

      /**
        *   Creates a new MapProcessor that uses the loadedMaps to
        *   communicate with the reader.
        *
        *   @param   loadedMaps  Vector used to tell the Reader about
        *                        the currently loaded maps.
        *   @param   packetFileName Name of file to write packets to for
        *                           debugging.
        */
      MapProcessor(MapSafeVector* loadedMaps, const char* packetFileName);

      /**
        *   Deletes the MapProcessor and releases the allocated 
        *   memory.
        */
      virtual ~MapProcessor();


      /**
        *   Used to return the current status (load) of this processor.
        *   This is currently not used.
        *
        *   @return  The status (load) of this processor.
        */
      int getCurrentStatus();

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
       *   Take care of an CoordinateRequest packet and produce the answer.
       *   {\bf NB!} The reply is created inside this method and both
       *   the request and the reply {\bf must} be deleted by the caller!
       *
       *   {Used by InfoMapGenerator when translating coordinates to
       *   itemIDs. That's why it's public.} Not anymore! 
       *
       *   If the angle in the request packet is valid (0 <= angle <= 360)
       *   and the only allowed item-type is StreetSegmentItem, the 
       *   getBestStreetSegmentItem-method is used to get the SSI. 
       *   Otherwise the closest item is returned (by only using the 
       *   hashtable).
       */
      CoordinateReplyPacket* processCoordinateRequestPacket(
                           const CoordinateRequestPacket* req);

      /**
       *   Take care of an CoordinatRequest  from the InfoMapGenerator
       *   and put the answer in the reply. This is a special function 
       *   to handle cameras and traffic situations with angle 32767 
       *   (omni directional). This is needed since there can be double 
       *   digitalised roads that in real life are only one road.
       *   {\bf NB!} The reply is not created inside this method and both
       *   the request and the reply {\bf must} be deleted by the caller!
       *
       *
       *   If the angle in the request packet is valid (0 <= angle <= 360)
       *   and the only allowed item-type is StreetSegmentItem, the 
       *   getBestStreetSegmentItem-method is used to get the SSI. 
       *   
       */
      bool processOmniInfoCoordinateRequest(uint32 mapID,
                                            int32 latitude,
                                            int32 longitude,
                                            uint32 angle,
                                            uint32 outDataTypes,
                                            uint32& itemID_0,
                                            uint32& itemID_1,
                                            uint32& distance,
                                            bool& dirFromZero,
                                            uint32& offset);

      /**
       *   Take care of an CoordinatRequest  from the InfoMapGenerator
       *   and put the answer in the reply.
       *   {\bf NB!} The reply is not created inside this method and both
       *   the request and the reply {\bf must} be deleted by the caller!
       *
       *
       *   If the angle in the request packet is valid (0 <= angle <= 360)
       *   and the only allowed item-type is StreetSegmentItem, the 
       *   getBestStreetSegmentItem-method is used to get the SSI. 
       *   
       */
      bool processInfoCoordinateRequest(uint32 mapID,
                                        int32 latitude,
                                        int32 longitude,
                                        uint32 angle,
                                        uint32 outDataTypes,
                                        uint32& itemID,
                                        uint32& distance,
                                        bool& dirFromZero,
                                        uint32& offset);
protected:
      /**
        *   The ``main method'' that the JobThread calls to get answers
        *   to the request packets.
        *
        *   @param   p  The request packet to handle.
        *   @return  A packet that is a reply of the request given as
        *            parameter.
        */
      Packet *handleRequestPacket( const RequestPacket& p,
                                   char* packetInfo );
      
private:
      /** 
       * Adds "Category" field to item info packet.
       * @param replyPacket the packet to add the information to.
       * @param map the map to fetch the categories from.
       * @param lang the language to translate the categories to.
       */
      void addCategories( ItemInfoData& reply, 
                          const Item& item,
                          const GenericMap& map,
                          LangTypes::language_t lang ) const;

      /**
        *   The object thats holds all the loaded maps.
        */
      auto_ptr<MapHandler> m_mapHandler;

      /**
        *
        */
      auto_ptr<ExpandRouteProcessor> m_routeExpander;

      
      /**
        *   The next port to try with when searching for a free port.
        */
      uint16 m_nextFreePort;

      /**
        *   Take care of an ItemLatLongRequestPacket and produces 
        *   the answer. The answer contains the coordinate for the
        *   items in the request packet. \\
        *   {\bf NB!} The reply is created inside this method and both
        *             the request and the reply {\bf must} be 
        *             deleted by the caller!
        */
      CoordinateOnItemReplyPacket* processCoordinateOnItemRequestPacket(
                           const CoordinateOnItemRequestPacket* p);

      /**
        *   Take care of an CoverdIDsRequest and produces the answer. \\
        *   {\bf NB!} The reply is created inside this method and both
        *   the request and the reply {\bf must} be deleted by the caller!
        */
      CoveredIDsReplyPacket* processCoveredIDsRequestPacket(
                           const CoveredIDsRequestPacket* req);

      /**
        *   Take care of an CoverdIDsRequest and produces the answer. \\
        *   {\bf NB!} The reply is created inside this method and both
        *   the request and the reply {\bf must} be deleted by the caller!
        */
      TrafficPointReplyPacket* processTrafficPointRequestPacket(
         const TrafficPointRequestPacket* p ) ;

      /**
        *   Take care of an HighlightRequest and produces the answer. \\
        *   {\bf NB!} The reply is created inside this method and both
        *   the request and the reply {\bf must} be deleted by the caller!
        */
/*        HighlightReplyPacket* processHighlightRequestPacket( */
/*                             HighlightRequestPacket* req); */

      /**
        *   Take care of an ExpandItemRquest and produces the answer. \\
        *   {\bf NB!} The reply is created inside this method and both
        *   the request and the reply {\bf must} be deleted by the caller!
        */
      ExpandItemReplyPacket* processExpandItemRequestPacket(
                              const ExpandItemRequestPacket* req);

      /**
       *    Take care of an ItemInfoRequestPacket.
       *    The packets must be deleted by the caller.
       */
      ItemInfoReplyPacket* processItemInfoPacket( const ItemInfoRequestPacket* p,
                                                  char*& packetInfo );

      /**
       * Take care of an ItemInfoRequestPacket.
       * The packets must be deleted by the caller.
       */
      ItemInfoReplyPacket* processGetItemInfoPacket( 
         const GetItemInfoRequestPacket* p, char*& packetInfo );

      /**
       *    Follows the gfxdata of the supplied item and puts
       *    the closest StreetSegmentItems around the edges in
       *    the replypacket. 
       *    @param item   The item to expand.
       *    @param p      The packet to add the expanded items to.
       *    @param theMap The current map.
       */
      void findRouteableItemsNear(Item* item,
                                  RouteExpandItemReplyPacket* p,
                                  GenericMap* theMap);
      
      /**
        *   Take care of an ExpandItemRquest and produces the answer. \\
        *   {\bf NB!} The reply is created inside this method and both
        *   the request and the reply {\bf must} be deleted by the caller!
        */
      RouteExpandItemReplyPacket* processRouteExpandItemRequestPacket(
                              const RouteExpandItemRequestPacket* req);


      /**
       *    Processes a StreetSegmentItemRequestPacket from the TRISS-
       *    server. The MapModule gets a coordinate and a direction and
       *    returns the coordinate on the closest StreetSegmentItem and
       *    the angle and the itemID.
       *    @param req The StreetSegmentItemRequestPacket.
       *    @return The StreetSegmentItemReplyPacket.
       */
      StreetSegmentItemReplyPacket* processStreetSegmentItemRequestPacket(
                           const StreetSegmentItemRequestPacket* req);
      
      /**
       *    Takes care of a IDTranslationRequestPacket and returns
       *    a reply. 
       *    {\bf NB!} The reply is created inside this method and both
       *    the request and the reply {\bf must} be deleted by the caller!
       */
      IDTranslationReplyPacket* processIDTranslationRequestPacket(
                                       const IDTranslationRequestPacket* p );

      /**
       *    Looks up information about matches which is not found
       *    in the SearchModule.
       *    @param req The Request.
       *    @return The Reply.
       */
      MatchInfoReplyPacket* processMatchInfoRequest(
         const MatchInfoRequestPacket* req) const;


      /**
       *    Looks up information about matches which is not found
       *    in the SearchModule.
       *    @param req The Request.
       *    @return The Reply.
       */
      QueueStartReplyPacket* processQueueStartPacket(
         const QueueStartRequestPacket* req) const;

      /**
       *   Creates a match from an Item.
       *   @param Item The item to create a match from.
       *   @param theMap The map where the Item is located.
       *   @param lang The requested language.
       *   @return The new VanillaMatch.
       */
      VanillaMatch* createMatchFromItem(Item* item,
                                        GenericMap* theMap,
                                        LangTypes::language_t lang) const;

      /**
       *  Processes an ExpandRequestPacket. It returns all the Items
       *  that are inside the item with the following itemID. But it
       *  only returns the Items with the requested item types.
       *  @param req The ExpandRequestPacket.
       *  @return The ExpandReplyPacket.
       */
      ExpandReplyPacket* processExpandRequest(
         const ExpandRequestPacket* req) const;

      InfoCoordinateReplyPacket* processInfoCoordinateRequest(
         const InfoCoordinateRequestPacket* req );

      /**
       *    Used by processMatchInfoRequest to set the best matched
       *    street number in the streetMatch. Regions are added.
       *    @param streetMatch The number in the streetMatch is used
       *                       to find the closest number in the map.
       *    @param theMap      The map is needed for item-getting etc.
       *    @param lang        Requested language for regions.
       *    @param allowed     Allowed search types for the regions.
       */
      inline void setBestHouseNbr(VanillaStreetMatch* streetMatch,
                                  const GenericMap& theMap,
                                  LangTypes::language_t lang,
                                  uint32 allowedSearchTypes) const;

      /**
       *    Adds region matches to a match.
       *    @param match              Match to add the regions to.
       *    @param idToAddFrom        The id of the item to add the
       *                              regions from.
       *    @param theMap             Map is needed to find the regions.
       *    @param reqLang            Requested language.
       *    @param allowedSearchTypes Search types of regions that are
       *                              to be added.
       *    @param level              Level of recursion.
       */
      inline void addRegionsToMatch(SearchMatch* match,
                                    uint32 idToAddFrom,
                                    const GenericMap& theMap,
                                    LangTypes::language_t reqLang,
                                    uint32 allowedSearchTypes,
                                    int level = 0) const;

      /**
       *   Creates a VanillaRegionMatch from the supplied itemID.
       *   @param itemID  The itemID of the item to lookup.
       *   @param reqLang Requested language for the names.
       *   @return A new VanillaRegionMatch.
       */
      inline VanillaRegionMatch*
         createRegionMatch(uint32 itemID,
                           const GenericMap& theMap,
                           LangTypes::language_t reqLang) const;
      
      /**
       *    Get the street segment item where it is most likely that the
       *    user (vehicle) is when the GPS-coordinate and angle is known.
       *    @param map   The map where the given coordinate is located.
       *    @param lat   The latitude part of the coordinate (probably from 
       *                 the GPS).
       *    @param lat   The longitude part of the coordinate (probably from 
       *                 the GPS).
       *    @param angle The heading of the vehicle (in degrees from the
       *                 north direction).
       *    @param dist  Outparameter that is set to the distance (in
       *                 meters) from the coordinate to the returned item.
       *    @return The item where is most likely that the user is located.
       */
      Item* getBestStreetSegmentItem(GenericMap* map,
                                     int32 lat, int32 lon, uint16 angle,
                                     uint32 &dist);

      
      /**
        *   Find the connection between fromItemID and toItem.
        *
        *   @param   fromItemID  ID of the item where the connection is from.
        *   @param   toItem      The item to turn into.
        *   @param   fromConnection A connection from an external connection.
        *   @return  StringCode describing the turn from fromItemID to
        *            toItem.
        */
      bool getConnectionData( StreetSegmentItem* fromItem,
                              StreetSegmentItem* toItem,
                              uint32 &dist,
                              uint32 &time,
                              uint32 &standStillTime,
                              uint32 &turnDesc,
                              int32 &lat, int32 &lon);

      /**
        *   Update the number of streets to the left, respective to the 
        *   right for a specified node.
        */
      void updateStreetCount(Node* curNode,
                             uint32 &nbrLeft, 
                             uint32 &nbrRight);

      /**
       *    Adds information to reply.
       */
      void addOneItemInfo( ItemInfoDataCont& reply,
                           const Item* item,
                           LangTypes::language_t langType,
                           const GenericMap& curMap,
                           const UserRightsMapInfo& rights,
                           ItemInfoEnums::InfoTypeFilter infoFilterLevel);

      /**
       * Get and add information about one item to reply.
       */
      uint32 getItemInfo( ItemTypes::itemType reqType, Item* curItem,
                          uint32 curItemID,
                          GenericMap* curMap, const MC2Coordinate& poiCoord, 
                          const char* poiName, LangTypes::language_t language,
                          const UserRightsMapInfo& rights,
                          ItemInfoEnums::InfoTypeFilter infoFilterLevel,
                          ItemInfoDataCont& reply );


      GetAdditionalPOIInfo m_getPOIInformation;



   CategoryTranslationMap m_poiCategories;
};

#endif


