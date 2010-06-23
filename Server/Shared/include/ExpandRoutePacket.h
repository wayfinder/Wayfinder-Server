/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPAND_ROUTE_REPLY
#define EXPAND_ROUTE_REPLY

#include "config.h"

#include "Packet.h"

#include "ItemTypes.h"
#include "TrafficDataTypes.h"
#include "StringTable.h"

#include <map>
#include <vector>
#include <list>

class GenericMap;

class LandmarkHead;
class LandmarkLink;
class DisturbanceDescription;
class DescProps;
class ExpandStringLanesCont;
class ExpandStringSignPosts;


#define EXPAND_ROUTE_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define EXPAND_ROUTE_REPLY_PRIO   DEFAULT_PACKET_PRIO


#define EXPAND_REQ_NUMITEMS        0
#define EXPAND_REQ_HEADING         2
#define EXPAND_REQ_IGNORE          2
#define EXPAND_REQ_TYPE            3
#define EXPAND_REQ_BOOL_BYTE       4
#define EXPAND_REQ_PASSEDSTREET    5
#define EXPAND_REQ_NO_CONCATINATE  6
#define EXPAND_REQ_NAME_CHANGE_AS  7
#define EXPAND_REQ_START_OFFSET    8
#define EXPAND_REQ_END_OFFSET     10
#define EXPAND_REQ_LANGUAGE       12
#define EXPAND_REQ_ORDINAL        16
#define EXPAND_REQ_WANTED_NAMES   18
#define EXPAND_REQ_LAST_ID        20
#define EXPAND_REQ_LAST_MAP       24
#define EXPAND_REQ_LAST_COUNTRY   28
#define EXPAND_REQ_NBR_MAPID_TO_COUNTRY 32
#define EXPAND_REQ_TRAF_INFO_NBR   36
#define EXPAND_REQ_TRAF_INFO_POS   38
#define EXPAND_REQ_MAPID_TO_COUNTRY 40

class ExpandItemID;
class ExpandStringItem;

/**
  *   The expand route request. After the normal header this packet 
  *   contains (subType = EXPAND_ROUTE_REQUEST):
  *   Table begins at REQUEST_HEADER_SIZE.
  *   @packetdesc
  *      @row 0     @sep 2 bytes @sep  Number of items in the request @endrow
  *      @row 2     @sep 1 bit   @sep  The start (origin) heading, in terms of 
  *                                    towards node 0 or 1 ( 1 == node 0 ) 
  *                                    (bit index 7). @endrow
  *      @row 2.125 @sep 1 bit   @sep  The end (destination) heading in terms 
  *                                    of towards node 0 or 1 ( 1 == node 0 )
  *                                    (bit index 6). @endrow
  *      @row 2.25  @sep 1 bit   @sep  Set if the endOffset should be ignored
  *                                    and use all of the segment. This is due
  *                                    to that we don't know in which direction 
  *                                    we are driving on the segment 
  *                                    (bit index 5). @endrow
  *      @row 2.375 @sep 1 bit   @sep  Set if we should ignore startOffset 
  *                                    (bit index 4).  @endrow
  *      @row 2.5   @sep 1 bit   @sep  Set to 1, all names should be printed 
  *                                    in the turndescriptions (bit index 3).
  *      @row 2.625 @sep 1 bit   @sep  Set to 1 if you want to remove the
  *                                    street name in the route if you have 
  *                                    an ahead-turn and the street changes 
  *                                    name (bit index 2).
  *      @row 2.75  @sep 1 bit   @sep  Set if we have an u-turn (bit index 1).
  *      @row 2.875 @sep 1 bit   @sep  Set if the street names should be 
  *                                    abbreviated (bit index 0).
  *      @row 3     @sep 1 byte  @sep  Wanted route type. @endrow
  *      @row 4     @sep 1 byte  @sep  States if landmarks should be inlcuded
  *                                    or not. @endrow
  *      @row 5     @sep 1 byte  @sep  One byte padding. @endrow
  *      @row 6     @sep 2 bytes @sep  Two bytes padding. @endrow
  *      @row 8     @sep 4 bytes @sep  Start and end offset. @endrow
  *      @row 12    @sep 4 bytes @sep  The requested language. @endrow
  *      @row 16    @sep 2 bytes @sep  Ordinal number of the packet. @endrow
  *      @row 18    @sep 2 bytes @sep  Type of name wanted in reply. @endrow
  *      @row 20    @sep 4 bytes @sep  Last ID on preceding map (multimap 
  *                                    expand). @endrow
  *      @row 24    @sep 4 bytes @sep  ID of preceding map (multimap expand).
  *                                    @endrow
  *      @row 28    @sep 4 bytes @sep  ID of the country of preceding map 
  *                                    (multimap expand). @endrow
  *      @row 32    @sep 4 bytes @sep  Number of mapID 2 countryID 
  *      @row 36    @sep 8 bytes each @sep mapID and countryID pair. @endrow
  *      @row 36 + (Number of mapID 2 countryID) * (4 + 4)    
            @sep 4 bytes each @sep Start of the itemID vector. @endrow
  *   @endpacketdesc
  */
class ExpandRouteRequestPacket : public RequestPacket {
  public:
      /** 
       *   This constructor is not empty anymore.
       *   Creates a packet and sets the typeID.
       */
      ExpandRouteRequestPacket();
   
      /**
       *   Get the number of items included in this request packet.
       *   @return The number of items in the header.
       */
      inline uint16 getNbrItems() const;

      /**
       *    Set the number of items included in this request packet.
       *    @param num The number of items.
       */
      inline void setNbrItems(uint16 num );
      
      /**
        *   @name Directions.
        *   Methods to get/set the start and end directions.
        */
      //@{
         /** 
           *   Get the direction at the last segment.
           *   @return True if start towards node 0 .
           */
         inline bool getStartDir() const;

         /** 
           *   Set the start direction.
           *   @param   towards0 The new start direction.
           */
         inline void setStartDir(bool startDir);

         /** 
           *   Returns true if we have an U-turn.
           *   @return  Returns true if we have an U-turn.
           */
         inline bool getUturn() const;
         
         /**
          *    Set the U-turn parameter.
          *    @param   U-turn True if we have an U-turn.
          */
         inline void setUturn(bool uTurn);
            
         /** 
           *   Get the direction at the end.
           *   @return True if end towards node 0 .
           */
         inline bool getEndDir() const;
         
         /**
           *   Set the end direction.
           *   @param   towards0 The new value of the end direction.
           */
         inline void setEndDir(bool towards0);

         /**
          *    Find out if the street names should be abbreviated or
          *    not. An example of abbreviation is (if the language is
          *    swedish) "Baravägen" --> "Barav.".
          *    @return  True if the street names should be abbreviated,
          *             false otherwise.
          */
         inline bool  getAbbreviate() const;

         /**
          *    Set if the street names should be abbreviated or not. 
          *    @param abbreviate True if the street names should be 
          *                      abbreviated, false otherwise.
          */
         inline void  setAbbreviate(bool abbreviate);
      //@}
    
      /**
        *   @name Offsets
        *   Methods to get/set the "ignore offset" variables and the 
        *   values of the start and end offsets.
        */
      //@{
         /**
           *   Set the bool that describe if the end offset should be ignored
           *   or not.
           *   @param   ignore   The new value of the "ignoreEndOffset" 
           *                     variable. This is true if the end offset
           *                     should be ignored, false otherwise.
           */
         inline void setIgnoreEndOffset( bool ignore );
         
         /**
           *   Find out if the end offset should be ignored or not.
           *   @return  True if the end offset should be ignored, false
           *            otherwise.
           */
         inline bool ignoreEndOffset() const;
         
         /**
           *   Set the bool that describe if the start offset should be 
           *   ignored or not.
           *   @param   ignore   The new value of the "ignoreStartOffset" 
           *                     variable. This is true if the start offset
           *                     should be ignored, false otherwise.
           */
         inline void setIgnoreStartOffset( bool ignore );

         /**
           *   Find out if the start offset should be ignored or not.
           *   @return True if the startOffset should be ignored and set to
           *           0 or 1 depending on which way we are heading. All of
           *           the segment should be used.
           */
         inline bool ignoreStartOffset( void ) const;
      
         /**
           *   Set the offset that should be used for the start-segment.
           *   @param   startOffset The new value of the start offset.
           */
         void setStartOffset( uint16 startOffset );

         /**
           *   Get the offset that should be used for the start-segment.
           *   @return The new value of the start offset.
           */
         uint16 getStartOffset() const;
         
         /**
           *   Set the offset that should be used for the end-segment (for
           *   the desitnation).
           *   @param   endOffset   The new value of the end offset.
           */
         void setEndOffset( uint16 endOffset );

         /**
           *   Get the offset that should be used for the destination-segment.
           *   @return The new value of the end offset.
           */
         uint16 getEndOffset() const;
      //@}
      
      /**
        *   Set whether all names should be printed in the turndescription.
        *   @param allNames   If set to true, all names will be printed,
        *                     otherwise only one (or zero).
        */
      inline void setPrintAllNames(bool allNames);
      
      /**
        *   Get whether all names should be printed in the turndescription.
        *   @return True if all names should be printed,
        *           otherwise only one (or zero).
        */
      inline bool getPrintAllNames() const;

      /**  Sets removeAheadIfNameDiffer. Is set if you want to remove the
       *   street name in the route if you have an ahead-turn and the street
       *   changes name.
       */
      inline void setRemoveAheadIfNameDiffer(bool remove);

      void setNoConcatinate(bool noConcatinate);
      
      /**
       *   Returns removeAheadIfNameDiffer.
       *   @return  True if you want to remove the street name in the route
       *   if you have an ahead-turn and the street changes name.
       */
      inline bool getRemoveAheadIfNameDiffer() const;


      bool getNoConcatinate() const;


      void nameChangeAsTurn(bool nameChangeAsTurn);

      bool getNameChangeAsTurn() const;

      
      /**
       *   Returns the startdirection.
       *   @return The start direction of the route.
       */
      bool getStartDirection();

      /**
        *   Set the ordinal number of this packet. The packets within 
        *   the same route are numberd from the start (number 0) to the 
        *   destination so that they can be concatinated in the server.
        *   @param   ordinal  The ordinal number of this packet.
        */
      void setOrdinal(uint16 ordinal);

      /**
       *    Get the ordinal number of this packet.
       *    @return The ordinal number for the packet in a split
       *            expansion.
       */
      uint16 getOrdinal() const;
      
      /**
        *   Add one node ID to this packet.
        *   @param   nodeID   The ID of the node that should be added to 
        *                     this packet.
        */
      void addNodeID( uint32 nodeID );
      
      /**
       *    Get all route items (node ids).
       *    @param   theMap   The map that the route passes through.
       *    @param   nodeIDs  Out parameter. List of node ids.
       *    @return  The number of route items.
       */
      uint32 getRouteItems( const GenericMap* theMap,
                            list<uint32>& nodeIDs ) const;
      
      /**
       *   Set the requested language for the route description.
       *   @return The requested language.
       */
      void setLanguage(StringTable::languageCode language);

      /**
       *   Get the requested language for the route description.
       *   @return The requested language.
       */
      uint32 getLanguage() const;
      
      /**
       *    @name Route type.
       *    Get and set methods for the route type(s) to return.
       *    The type describes the format of the route that should 
       *    be returned. Valid values are ROUTE_TYPE_STRING, 
       *    ROUTE_TYPE_GFX and ROUTE_TYPE_STRING_COORD. It is 
       *    possible to combine ("or") all values to get the route 
       *    in different formats.
       */
      //@{
         /**
          *    Get the type of route(s) that should be returned.
          *    @return  Wanted format(s) of the route.
          */
         byte getType() const;

         /**
          *    Set the type of route(s) that should be returned.
          *    @param   t  Wanted format(s) of the route.
          */
         void setType(byte t);
      //@}
    
      /**
        *   Find out if landmarks should be included in the 
        *   route requested or not.
        */
      bool includeLandmarks() const;

      /**
        *   Set whether landmarks should be included in the 
        *   route or not.
        *   @param inlcudeLandmarks True if landmarks should 
        *                           be included, false if not.
        */
      void setIncludeLandmarks(bool includeLandmarks);

      /**
        *   Find out if AddionalCost should be used in the 
        *   route requested or not.
        */
      bool useAddCost() const;
      
      /**
        *   Set whether Additional costs should be used in the 
        *   route or not.
        *   @param ignoreAddCost True if additional cost statesshould be
        *                        ignored .
        */
      void setUseAddCost(bool useAddCost);

      /**
        *   Find out how many streets before a turn should be
        *   presented as a LandMark
        *   @return The max number of streets to include in landmark.
        */
      byte getNbrPassStreets() const;

      
      /**
        *   Set the number of streets before a turn to be
        *   presented as a LandMark
        *   @param  nbrOfStreets The max number of streets to include
        *           as a landmark.
        */
      void setNbrPassStreets(byte nbrOfStreets);
      
      /**
       *    @name Last ID.
       *    Get and set methods for the last ID in the route.
       */
      //@{
         /**
           *   Set the ID of the last item in the route.
           *   @param   ID    The id of the last item.
           */
         void setLastID(uint32 ID);

         /**
           *   Get the ID of the last item in the route.
           *   @return  The id of the last item.
           */
         uint32 getLastID(void) const;
         
         /**
           *   Set the ID of the map where the last item in the route
           *   is located.
           *   @param   mapID The ID of the map for the last item.
           */
         void setLastMapID(uint32 mapID);

         /**
           *   Get the ID of the map where the last item in the route
           *   is located.
           *   @return  The ID of the map for the last item.
           */
         uint32 getLastMapID(void) const;

         /**
          * Set the countryID of the last map in preceding route part.
          *
          * @param countryID The countryID of the map for the last item.
          */
         void setLastCountry( uint32 countryID );

         /**
          * Get the countryID of the last map in preceding route part.
          *
          * @return The countryID of the map for the last item.
          */
         uint32 getLastCountry() const;
      //@}

      /**
       *    @name Map ID to country ID lookup table.
       *    Get and set methods for the map ID to country ID lookup table.
       *    This is only needed for overview maps.
       */
      //@{
         /**
          * Set the mapID to countryID table, please note that this
          * method only can be called before any addNode call.
          * 
          * @param mapID2CountryID The mapID to countryID table.
          * @return True if mapID to countryID table was set, false if not.
          */
         bool setMapIDToCountryID( map<uint32,uint32>& mapID2CountryID );


         /**
          * Get the mapID to countryID table.
          *
          * @param mapID2CountryID Set to add the mapID to countryID 
          *                        table to.
          */
         void getMapIDToCountryID( 
            map<uint32,uint32>& mapID2CountryID ) const;


         /**
          * Get the number of items in the mapID to countryID table.
          *
          * @return The number of mapID to countryID entries.
          */
         uint32 getNbrMapIDToCountryID() const;
      //@}
         
         /**
          *   Return the number of disturbances in the packet.
          *   @return The number of disturbances in the packet.
          */
         uint16 getNbrTrafficInfo() const;

         /**
          *   Set the number of disturbances in the packet.
          *   @param The new number of disturbances in the packet.
          */
         void setNbrTrafficInfo(uint16 nbr);
         
         /**
          * Add one disturbance to the packet.
          * Only use this when the whole route is in the packet.
          * 
          * If this is a detour splitID and/or mergeID is set to the nodes
          * where the route detours/joins, if these points are on this map.
          * 
          * @param nodeID The node where the disturbance is located
          * @param splitID If not detour MAX_UINT32
          * @param distNodeID node of a detours disturbance. MAX_UINT32 if on
          *               another map.(same as nodeID if not detour)
          * @param mergeID If not detour MAX_UINT32
          * @param distID The ID of the disturbance
          * @param type The type of disturbance
          * @param text The disturbance text.
          */
         void addTrafficInfo(uint32 nodeID,
                             bool   split,
                             uint32 distNodeID,
                             bool   merge,
                             uint32 distID,
                             TrafficDataTypes::disturbanceType type,
                             const char* text);

         /**
          * Add the disturbance affecting the route on this map to the packet.
          * Disturbances & detour points not on this map are ignored.
          * Only use this when the whole route is in the packet.
          *
          * @param dVect The vector with the routes disturbance descriptions.
          * @return The number of disturbances added.
          */
         int addTrafficInfo( const vector<DisturbanceDescription>& dVect);
         

         /**
          * Retrieve one disturbance
          * If this is a detour splitID and/or mergeID is set to the nodes
          * where the route detours/joins, if these points are on this map.
          *
          * @param pos The position in the packet to read at. Set to 0 to start
          *            at first disturbance.
          * @param nodeID The node where the disturbance is located
          * @param split True if detour split point.
          * @param distNodeID node of a detours disturbance. MAX_UINT32 if 
          *            on another map. (same as nodeID if not detour)
          * @param mergeID True if detour merge point.
          * @param distID The ID of the disturbance
          * @param type The type of disturbance
          * @param text The disturbance text.
          * @return -1 if fail otherwise the next position in the packet. 
          */
         int getTrafficInfo( int &pos,
                             uint32 &nodeID,
                             bool   &split,
                             uint32 &distNodeID,
                             bool   &merge,
                             uint32 &distID,
                             TrafficDataTypes::disturbanceType &type,
                             char* &text) const;
            
      
  private:
      /**
       * Set the nbr mapID to countryID entries to 0.
       * Used in constructor and setMapIDToCountryID.
       *
       * @param nbr The number of mapID to countryID entries.
       */
      void setNbrMapIDToCountryID( uint32 nbr );
};

#define EXPAND_REPLY_ROUTE_TYPE              0
#define EXPAND_REPLY_HEADING                 1
#define EXPAND_REPLY_NBRSTRINGS              2
#define EXPAND_REPLY_STRINGSIZE              4
#define EXPAND_REPLY_NBRITEMS                8
#define EXPAND_REPLY_NBRSTRINGSPERITEM      10
#define EXPAND_REPLY_ITEMSSIZE              12
#define EXPAND_REPLY_TOT_DIST               16
#define EXPAND_REPLY_TOT_TIME               20
#define EXPAND_REPLY_TOT_STANDSTILL         24
#define EXPAND_REPLY_ORDINAL                28
#define EXPAND_REPLY_LAST_LEFT_STREETCOUNT  30
#define EXPAND_REPLY_LAST_RIGHT_STREETCOUNT 31
#define EXPAND_REPLY_START_OFFSET           32
#define EXPAND_REPLY_END_OFFSET             34

#define EXPAND_REPLY_UTURN                  36
#define EXPAND_REPLY_START_DIR_NBR          37
#define EXPAND_REPLY_START_DIR_ODDEVEN      38

#define EXPAND_REPLY_LASTITEM_LAT           41
#define EXPAND_REPLY_LASTITEM_LON           45
#define EXPAND_REPLY_SUB_HEADER_SIZE        49

/**
 *    Packet sent as reply to the ExpandRouteRequestPacket, containing
 *    the route in the wanted format (string or graphical). After the
 *    normal ReplyPacket-header the ExpandRouteReplyPacket contains:
 *    Table begins at REPLY_HEADER_SIZE
 *    @packetdesc
 *       @row  0 @sep 1 bytes @sep Route type                        @endrow
 *       @row  1 @sep 1 bytes @sep Heading                           @endrow
 *       @row  2 @sep 2 bytes @sep Start angle                       @endrow
 *       @row  2 @sep 2 bytes @sep Number of strings in stringtable  @endrow
 *       @row  4 @sep 4 bytes @sep Size of the stringtable           @endrow
 *       @row  8 @sep 2 bytes @sep Number of items                   @endrow
 *       @row 10 @sep 2 bytes @sep Number of nbr of items per string @endrow
 *       @row 12 @sep 4 bytes @sep size of item data                 @endrow
 *       @row 16 @sep 4 bytes @sep Total distance                    @endrow
 *       @row 20 @sep 4 bytes @sep Total time                        @endrow
 *       @row 24 @sep 4 bytes @sep Total standstill time             @endrow
 *       @row 28 @sep 2 bytes @sep Ordinal number of the reply       @endrow
 *       @row 30 @sep 1 bytes @sep Last segment's left street count  @endrow
 *       @row 31 @sep 1 bytes @sep Last segment's right street count @endrow
 *       @row 32 @sep 2 bytes @sep Start offset                      @endrow
 *       @row 34 @sep 2 bytes @sep End offset                        @endrow
 *       @row 36 @sep 1 bit   @sep U-turn                            @endrow
 *       @row 37 @sep x bytes @sep Latitude of destination           @endrow
 *       @row 41 @sep x bytes @sep Longitude of destination          @endrow
 *       @row 45 @sep x bytes @sep String data starts here           @endrow
 *    @endpacketdesc
 *
 *    The format of string data follows:
 *    @packetdesc
 *       @row  0 @sep 4 bytes @sep StringCode  @endrow
 *       @row  4 @sep 4 bytes @sep Distance    @endrow
 *       @row  8 @sep 4 bytes @sep Time        @endrow
 *       @row 12 @sep 4 bytes @sep Lat         @endrow
 *       @row 16 @sep 4 bytes @sep Lon         @endrow
 *       @row 20 @sep 1 bytes @sep transport   @endrow
 *       @row 21 @sep 1 bytes @sep nametype    @endrow
 *       @row 22 @sep 1 byte  @sep turn number @endrow
 *       @row 23 @sep 1 byte  @sep crossing kind @endrow
 *       @row 24 @sep Objects @sep Lanes @endrow
 *       @row  x @sep Objects @sep SignPosts @endrow
 *       @row  x @sep list    @sep Landmarks   @endrow
 *    @endpacketdesc
 *
 *   
 */
class ExpandRouteReplyPacket : public ReplyPacket {
   public:
      /**
        *   Creates an empty ExpandRouteReplyPacket.
        */
      ExpandRouteReplyPacket(int size = MAX_PACKET_SIZE);

      /**
        *   Creates a ExpandRouteReplyPacket as a reply to a given
        *   ExpandRouteRequestPacket.
        *   @param   p  The corresponding request packet.
        */
      ExpandRouteReplyPacket(const ExpandRouteRequestPacket* p);

      /**
       *    Get the type of route that is returned in this packet.
       *    @return  The type of route wanted by the requester.
       */
      inline byte getRouteType(void) const;
      
      /**
       *    Set the type of route in this packet.
       */
      inline void setRouteType(byte t);
  
      /**
       *   @name Directions.
       *   Methods to get/set the start and end directions.
       */
      //@{
         /** 
          *    Get the start direction of the route.
          *    @return True if start towards node 0, false otherwise.
          */
         bool getStartDir() const;
         
         /** 
          *    Set the start direction of the route.
          *    @param   towards0 True if start towards node 0, false 
          *                      otherwise.
          */
         void setStartDir(bool towards0);
         
         /** 
          *    Get the end direction.
          *    @return True if driving towards node 0 at destination, 
          *            false otherwise.
          */
         bool getEndDir() const;
         
         /** 
          *    Set the end direction.
          *    @param   towards0 True if destination is towards node 0, 
          *                      false otherwise.
          */
         void setEndDir(bool towards0);

         /**
          *    Returns true if we have an U-turn.
          *    @return  Returns true if we have an U-turn.
          */
         bool getUturn() const;             
         
         /**
          *    Set the U-turn parameter.
          *    @param   U-turn True if we have an U-turn.
          */
         void setUturn(bool uTurn);
      //@}

      /**
       *    Get the number of items in the string data.
       *    @return  The number of string data.
       */
      inline uint16 getNumStringData() const;
      
      /**
       *    Set the number of items in the string data.
       *    @param   num   The number of string data.
       */
      inline void setNumStringData( uint16 num );
     
      /**
       *    Get the size of the string data.
       *    @return  The size of the string data.
       */
      inline uint32 getSizeStringData() const;

      /**
       *    Set the size of the strings to a given value.
       *    @param   size  The size of the string data.
       */
      inline void setSizeStringData( uint32 size);
       
      /**
       *    Get the number of items in the data.
       *    @return  The number of item data.
       */
      inline uint16 getNumItemData() const;

      /**
       *    Set the number of items in the data.
       *    @param   num   The number of item data.
       */
      inline void setNumItemData( uint16 num );
       
      /**
       *    Get the size of the data-items.
       *    @return  The size of the data-items.
       */
      inline uint32 getSizeItemsData() const;

      /**
       *    Set the size of the data-items.
       *    @param   size  The size of the data-items.
       */
      inline void setSizeItemsData(uint32 size);

      /**
       *    Get the number of items per string.
       *    @return The number of positions with nbr of items per 
       *            string.
       */
      inline uint16 getNbrItemsPerString() const;
      
      /**
       *    Get the number of items per string.
       *    @param   nbr   The positions with nbr of items per string.
       */
      inline void setNbrItemsPerString( uint16 nbr );

      /**
       *    Set the ordinal number of this packet. The packets within 
       *    the same route are numberd from the start (number 0) to the 
       *    destination so that they can be concatinated in the server.
       *    @param   ordinal  The ordinal number of this packet.
       */
      void setOrdinal(uint16 ordinal);

      /**
       *    Get the ordinal number of this packet.
       *    @return The ordinal number for the packet in a split
       *            expansion.
       */
      uint16 getOrdinal() const;

      /**
       *    Get the total distance of the route.
       *    @return  The total distance of the route (meters).
       */
      uint32 getTotalDist() const;

      /**
       *    Set the total distance of the route.
       *    @param   dist  The estimated total driving distance 
       *                   (meters).
       */
      void setTotalDist(uint32 dist);

      /**
       *    Get the total traversal time for the route.
       *    @return  The total time of the route. (seconds)
       */
      uint32 getTotalTime() const;

      /**
       *    Set the total traversal time for the route.
       *    @param   time  The estimated total time. (seconds)
       */
      void setTotalTime(uint32 time);

      /**
       *    Get the number of seconds for the standstill time.
       *    @return  The standstill time (seconds).
       */
      uint32 getStandStillTime() const;

      /**
       *    Set the number of seconds for the standstill time.
       *    @param time The estimated standstill time.
       */
      void setStandStillTime(uint32 time);

      /**
       *    Get the number of streets to the left for the last segment.
       *    @return  The number of streets to the left for the last
       *             segment.
       */
      byte getLastSegmentsLeftStreetCount() const;
      
      /**
       *    Set the number of streets to the left for the last segment.
       *    @param   count   The number of streets to the left for the last
       *                     segment.
       */
      void setLastSegmentsLeftStreetCount(byte count);
      
      /**
       *    Get the number of streets to the right for the last segment.
       *    @return  The number of streets to the right for the last
       *             segment.
       */
      byte getLastSegmentsRightStreetCount() const;
      
      /**
       *    Set the number of streets to the right for the last segment.
       *    @param   count   The number of streets to the right for the last
       *                     segment.
       */
      void setLastSegmentsRightStreetCount(byte count);
      
      /**
       *    Set the offset on the start segmenet.
       *    @param   offset   The offset to use at the start segment.
       */
      void setStartOffset(uint16 offset);

      /**
       *    Get the offset on the start segmenet.
       *    @return The offset to use at the start segment.
       */
      uint16 getStartOffset() const;

      /**
       *    Set the offset on the last segmenet.
       *    @param   offset   The offset to use at the last segment.
       */
      void setEndOffset(uint16 offset);

      /**
       *    Get the offset on the last segmenet.
       *    @return The offset to use at the last segment.
       */
      uint16 getEndOffset() const;

      /**
       *    @name Start direction.
       *    Methods to get the start direction of the route in different
       *    formats.
       */
      //@{
         /**
          *    Set the start direction of the route in terms of 
          *    increasing of decreasing housenumbers.
          *    @param x    The value of the direction.
          */
         void setStartDirectionHousenumber(ItemTypes::routedir_nbr_t x);

         /**
          *    Get the start direction of the route in terms of 
          *    increasing of decreasing housenumbers.
          *    @return The value of the direction.
          */
         ItemTypes::routedir_nbr_t getStartDirectionHousenumber() const;

         /**
          *    Set the start direction of the route in terms of 
          *    odd on the left or right side.
          *    @param x    The value of the direction.
          */
         void setStartDirectionOddEven(ItemTypes::routedir_oddeven_t x);

         /**
          *    Get the start direction of the route in terms of 
          *    odd on the left or right side.
          *    @return The value of the direction.
          */
         ItemTypes::routedir_oddeven_t getStartDirectionOddEven() const;
      //@}

      /**
       *    @name Destination position.
       *    Methods to get and set the coordinate of the destination.
       *    This will be set to be able to draw the route properly
       *    in the graphical client.
       */
      //@{
         /**
          *    Set the coordinates of the destination.
          *    @param   lat   The latitude part of the coordinate.
          *    @param   lon   The longitude part of the coordinate.
          */
         inline void setLastItemPosition(int32 lat, int32 lon);
         
         /**
          *    Get the latitude part of the destination position.
          *    @return  The latitude part of the coordinate for the 
          *             destination.
          */
         inline int32 getLastItemLat() const;
         
         /**
          *    Get the longitude part of the destination position.
          *    @return  The longitude part of the coordinate for the 
          *             destination.
          */
         inline int32 getLastItemLon() const;
      //@}
      
      /**
       *    @name Add data
       *    Methods used to add data to the packet.
       *    @see Add NAVIGATOR-data
       */
      //@{
         /**
          *    Add one stringdata to this packet.
          *    @warning All strings must be added (by using this method) 
          *             before you call the other methods that adds data, 
          *             e.g. setItemData() / setNavItemData().
          *
          *    @param   str         The string.
          *    @param   stringCode  The turndesc etc. to be displayed
          *                         next to the string. NB! The most
          *                         significant (leftmost) byte describes
          *                         the exitcount.
          *    @param   transport   The type of transoprtation (e.g walk, 
          *                         drive).
          *    @param   dist        The dist to be displayed if 0 no dist 
          *                         will be displayed.
          *    @param   time        The time to be displayed if 0 no time 
          *                         will be displayed.
          *    @param   lat         The latitude of the turn. (Default 0).
          *    @param   lon         The longitude of the turn.(Default 0).
          *    @param   nameType    The type of the name ( Default 0 ).
          *    @param   turnNumber  The number of the turn, eg. 2 nd left.
          *    @param   lanes       The lanes.
          *    @param   signPosts   The signposts.
          *    @param   landmarks   The landmarks belonging to the string data.
          */ 
         bool setStringData( const char* str,
                             uint32 stringCode,
                             uint32 dist,
                             uint32 time,
                             ItemTypes::transportation_t transport,
                             int32 lat,
                             int32 lon,
                             uint8  nameType,
                             byte turnNumber,
                             ItemTypes::crossingkind_t crossingKind,
                             uint32 nbrPossTurns,
                             uint32* possTurns,
                             const ExpandStringLanesCont& lanes,
                             const ExpandStringSignPosts& signPosts,
                             LandmarkHead* landmarks );

         /**
          *    Add one data-item to this packet, <b>without</b> coordinates. 
          *    @warning Before you call this function make sure you have 
          *             added all the string data you want.}
          *    @warning Do not use this function when the receiver expects 
          *             coordinates.
          *
          *    @param   mapID    The upper part of the itemID.
          *    @param   itemID   The lower part of the itemID.
          */
         void setItemData(uint32 mapID, uint32 itemID);

         /**
          *   Add one data-item to this packet, <B>with</B> one pair of
          *   coordinates. 
          *   @warning Before you call this function make sure you have 
          *            added all the string data you want.}
          *   @warning Do not use this function when routeType implies 
          *            that one pair of latitude and longitude should not 
          *            be used.
          *  
          *   @param mapID     The upper part of the itemID.
          *   @param itemID    The lower part of the itemID.
          *   @param lat       The longitude of the item.
          *   @param lon       The latitude of the item.
          *
          */
         void setItemData( uint32 mapID, uint32 itemID,
                           int32 lat, int32 lon);
      //@}
         
      /**
       *    @name Add NAVIGATOR-data
       *    Add data when the route type is ROUTE_TYPE_NAVIGATOR.
       *    The information stored is the map id, item id, 
       *    speedlimit, attributes (motorway flag)
       *    and all coordinates for that item. These methods 
       *    @b must be called in the correct order.
       *   
       *    First call the setNavItemData method, which will store the 
       *    item and map id, as well as the speedlimit. Store the 
       *    return value of this method and use that when adding the 
       *    coordinates.
       *
       *    Then add the coordinates for that item by calling
       *    addCoordToItemData(lat, lon, nbrCoordPos) for each
       *    coordinate that should be added. The parameter nbrCoordPos 
       *    should be set to the return value of setNavItemData().
       */
      //@{
      
         /**
          *    Static method to create a bitfield containing
          *    additional attributes.
          *    @param   freeway  Whether this road is a freeway or not.
          *    @param   ramp     Whether this road is a ramp or not.
          *    @param driveOnRightSide Whether this road is right side
          *                            traffic.
          *    @return  A bitfield representing the attributes. To 
          *             pass into the packet.
          */
         inline static byte createAttributes( bool freeway, bool ramp,
                                              bool driveOnRightSide );

         /**
          *    Static method to extract the additional attributes 
          *    from the attribute bitfield (from the packet). 
          *    @param   attributes  Bitfield representing the attributes.
          *    @param   freeway     Outparameter. Whether this road is
          *                         a freeway or not.
          *    @param   ramp        Outparameter. Whether this road is
          *                         a ramp or not.
          *    @param driveOnRightSide Whether this road is right side
          *                            traffic or not.
          */
         inline static void extractAttributes( byte attributes, 
                                               bool& freeway,
                                               bool& ramp,
                                               bool& driveOnRightSide );
         
          /**
           *   Add one data-item to this packet, @b with speedlimit
           *   and <B>with all</B> coordinates (added later by method
           *   addCoordToItemData).
           *   This method must be used before setting the coordinates
           *   with addCoordToItemData().
           *   Use only this method when the route type is 
           *   ROUTE_TYPE_NAVIGATOR.
           *   @remark Before you call this function make sure
           *           you have added all the string data you want.
           *
           *   @param   mapID       The upper part of the itemID.
           *   @param   itemID      The lower part of the itemID.
           *   @param   speedLimit  The speedlimit for this item.
           *   @param   attributes  Additional attributes for this item.
           *                        @see extractAttributes.
           *   @return  The position in the packet for indicating the
           *            number of coordinates. This value should be
           *            passed on to the addCoordToItemData() method.
           */
         uint32 setNavItemData( uint32 mapID, uint32 itemID, 
                                byte speedLimit, byte attributes );

         /**
          *    Adds one coordinate (can be called several times) to
          *    a data-item. To be used after setNavItemData() which
          *    specifies which item that the coordinates belong to.
          *    This method should only be used when the route type
          *    is ROUTE_TYPE_NAVIGATOR.
          *    @param   lat         The latitude coordinate.
          *    @param   lon         The longitude coordinate.
          *    @param   nbrCoordPos The position in the packet for
          *                         indicating the number of coordinates
          *                         added to the item. This parameter
          *                         should come from the returnvalue
          *                         of setItemData().
          */
         void addCoordToNavItemData(int32 lat, int32 lon,
                                    uint32 nbrCoordPos);
      //@}
      
      /**
       *    Get the string-items in the route.
       *    @warning The vector is alloced here but must be deleted 
       *             elsewhere.
       * 
       *    @return  A vector of size getNumStringData() containing 
       *             StringRouteItem.
       */
      ExpandStringItem** getStringDataItem() const;

      /**
       *    Get the data-items in the route.
       *    @warning The vector is alloced here but must be deleted 
       *             elsewhere.
       *
       *    @return The vector of vectors of gitemsID.
       */
      ExpandItemID* getItemID() const;

      /**
       *    Print some data from the buffer in this packet.
       *
       *    @param   headerOnly    If true only print header and 
       *                            subheader.
       */
      void dump2( bool headerOnly = false ) const;

      /**
       *    Set the number of items per string.
       *    @param nbr  The nbr of items per string.
       */
      void addItemsPerString(uint32 nbr);

      /**
       *    Get the number of items per sting for a given index. Valid
       *    values are 0 < getNbrItems().
       *    @param index   The cardinal number of the string that the
       *                   number of items should be returnde for.
       *    @return The number of items per string at given index.
       */
      uint32 getItemsPerString(uint32 index) const;

      /**
       *    Get the ID of the group that the item at a given position
       *    belongs to.
       *    @param   index The cardinal number for the item that the
       *                   group ID should be returned.
       *    @return  The group ID that the item at position index belongs 
       *             to.
       */
      uint32 getGroupID( uint32 index ) const;
      
      /**
       *    @name Route summary
        *   Methods for getting route summary information as strings.
        *   The desired language, whether the text should be wap formatted 
        *   and distance format is read from the route description 
        *   properties bitfield.
        *   @see  ExpandStringItem For how to set the route description
        *         properties (createRouteDescProps()).
        */
      //@{
         /**
           *   Get a string with the total distance of the route.
           *   For instance: "Total distance 58 km".
           *   @param descProps  Route description properties (bitfield).
           *                     The language, wap format and distance
           *                     format information are used.
           *   @param buf        Preallocated buffer which will be filled
           *                     with the total distance of the route.
           *   @param maxLength  The length of buf.
           *   @param nbrBytesWritten  The number of bytes written into buf.
           *   @return True if the data did fit in the
           *           buffer, false otherwise.
           */
         bool getTotalDistStr(DescProps descProps,
                              char* buf,
                              uint32 maxLength,
                              uint32 nbrBytesWritten) const;
    
         /**
           *   Get a string with either the total time of the route or
           *   the total standstill time of the route.
           *   For instance: "Total time 20:10".
           *   @param descProps  Route description properties (bitfield).
           *                     The language and wap format information
           *                     are used.
           *   @param buf        Preallocated buffer which will be filled
           *                     with the total / standstill time.
           *   @param maxLength  The length of buf.
           *   @param nbrBytesWritten  The number of bytes written into buf.
           *   @param standStillTime   If this is set to true, the total
           *                           standstill time is used, otherwise
           *                           the total time.
           *   @return True if the data did fit in the
           *           buffer, false otherwise.
           */
         bool getTimeStr(DescProps descProps,
                         char* buf,
                         uint32 maxLength,
                         uint32 nbrBytesWritten,
                         bool standStillTime = false) const;
      
         /**
           *   Get a string with the average speed of the route.
           *   For instance: "Average speed 85 km/h".
           *   @param descProps  Route description properties (bitfield).
           *                     The language, wap format information
           *                     are used.
           *   @param buf        Preallocated buffer which will be filled
           *                     with the average speed of the route.
           *   @param maxLength  The length of buf.
           *   @param nbrBytesWritten  The number of bytes written into buf.
           *   @return True if the data did fit in the
           *           buffer, false otherwise.
           */
         bool getAverageSpeedStr(DescProps descProps,
                                 char* buf,
                                 uint32 maxLength,
                                 uint32 nbrBytesWritten) const;
      //@}

   private:
      /**
        *   Increases the number of stringdata in the buffer.
        */
      inline void incNumStringData();

      /**
        *   Increases the number of itemdata in the buffer.
        */
      inline void incNumItemData();
};

// =======================================================================
//                                     Implementation of inlined methods =


inline uint16 
ExpandRouteRequestPacket::getNbrItems()  const
{
   return readShort( REPLY_HEADER_SIZE );
}

inline void 
ExpandRouteRequestPacket::setNbrItems(uint16 num ) {
   writeShort( REPLY_HEADER_SIZE, num);
}

inline bool  
ExpandRouteRequestPacket::getStartDir() const
{
   return readBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 7 );
}


inline void  
ExpandRouteRequestPacket::setStartDir(bool startDir) 
{
   writeBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 7, startDir);
}

inline bool  
ExpandRouteRequestPacket::getEndDir() const
{
   return readBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 6 );
}

inline void 
ExpandRouteRequestPacket::setEndDir(bool endDir) 
{
   writeBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 6, endDir);
}

inline bool
ExpandRouteRequestPacket::ignoreEndOffset( void ) const
{
   return readBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 5 );
}

inline void
ExpandRouteRequestPacket::setIgnoreEndOffset( bool ignore ) 
{
   writeBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 5, ignore);
}

inline bool
ExpandRouteRequestPacket::ignoreStartOffset( void ) const
{
   return readBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 4 );
}

inline void
ExpandRouteRequestPacket::setIgnoreStartOffset( bool ignore ) 
{
   writeBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 4, ignore);
}

bool
ExpandRouteRequestPacket::getPrintAllNames() const
{
   return readBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 3 );
}

void
ExpandRouteRequestPacket::setPrintAllNames(bool allNames)
{
   writeBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 3, allNames);
}

inline bool
ExpandRouteRequestPacket::getRemoveAheadIfNameDiffer() const
{
   return readBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 2 );
}


inline void
ExpandRouteRequestPacket::setRemoveAheadIfNameDiffer(bool remove)
{
   writeBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 2, remove);
}


inline bool  
ExpandRouteRequestPacket::getUturn() const
{
   return readBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 1 );
}

inline void  
ExpandRouteRequestPacket::setUturn(bool uTurn) 
{
   writeBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 1, uTurn );
}

inline bool  
ExpandRouteRequestPacket::getAbbreviate() const
{
   return readBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 0 );
}

inline void  
ExpandRouteRequestPacket::setAbbreviate(bool abbreviate) 
{
   writeBit( REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE, 0, abbreviate);
}


// - - - - - - - - - - - - - - - - - - - - - - - ExpandRouteReplyPacket
inline byte 
ExpandRouteReplyPacket::getRouteType(void) const {
   return readByte(REPLY_HEADER_SIZE+EXPAND_REPLY_ROUTE_TYPE);
}

inline void 
ExpandRouteReplyPacket::setRouteType(byte t) {
   writeByte(REPLY_HEADER_SIZE + EXPAND_REPLY_ROUTE_TYPE, t);
}


inline uint16 
ExpandRouteReplyPacket::getNumStringData() const
{
   return readShort(REPLY_HEADER_SIZE + EXPAND_REPLY_NBRSTRINGS);
}

inline void  
ExpandRouteReplyPacket::setNumStringData( uint16 num )
{
   writeShort(REPLY_HEADER_SIZE+EXPAND_REPLY_NBRSTRINGS, num);
}

inline uint32  
ExpandRouteReplyPacket::getSizeStringData() const
{
   return readLong(REPLY_HEADER_SIZE + EXPAND_REPLY_STRINGSIZE);
}

inline void  
ExpandRouteReplyPacket::setSizeStringData( uint32 size)
{
   writeLong(REPLY_HEADER_SIZE + EXPAND_REPLY_STRINGSIZE, size);
}

inline uint16  
ExpandRouteReplyPacket::getNumItemData() const
{
   return readShort(REPLY_HEADER_SIZE+EXPAND_REPLY_NBRITEMS);
}

inline void  
ExpandRouteReplyPacket::setNumItemData( uint16 num )
{
   writeShort(REPLY_HEADER_SIZE+EXPAND_REPLY_NBRITEMS, num);
}
 
inline uint16  
ExpandRouteReplyPacket::getNbrItemsPerString() const
{
   return readShort( REPLY_HEADER_SIZE +
                     EXPAND_REPLY_NBRSTRINGSPERITEM ); 
}

inline void  
ExpandRouteReplyPacket::setNbrItemsPerString( uint16 nbr )
{
   writeShort( REPLY_HEADER_SIZE + EXPAND_REPLY_NBRSTRINGSPERITEM, nbr );
}

inline uint32  
ExpandRouteReplyPacket::getSizeItemsData() const
{
   return readLong(REPLY_HEADER_SIZE + EXPAND_REPLY_ITEMSSIZE);
}

inline void  
ExpandRouteReplyPacket::setSizeItemsData( uint32 size )
{
   writeLong(REPLY_HEADER_SIZE + EXPAND_REPLY_ITEMSSIZE, size );
}

inline void  
ExpandRouteReplyPacket::incNumStringData()
{
   writeShort(REPLY_HEADER_SIZE+EXPAND_REPLY_NBRSTRINGS,
             getNumStringData()+1);
}

inline void  
ExpandRouteReplyPacket::incNumItemData()
{
   setNumItemData(getNumItemData()+1);
}

inline void 
ExpandRouteReplyPacket::setLastItemPosition(int32 lat, int32 lon)
{
   writeLong(REPLY_HEADER_SIZE + EXPAND_REPLY_LASTITEM_LAT, lat);
   writeLong(REPLY_HEADER_SIZE + EXPAND_REPLY_LASTITEM_LON, lon);
}

inline int32 
ExpandRouteReplyPacket::getLastItemLat() const
{
   return (readLong(REPLY_HEADER_SIZE + EXPAND_REPLY_LASTITEM_LAT));
}

inline int32 
ExpandRouteReplyPacket::getLastItemLon() const
{
   return (readLong(REPLY_HEADER_SIZE + EXPAND_REPLY_LASTITEM_LON));
}

inline byte
ExpandRouteReplyPacket::createAttributes( bool freeway, bool ramp,
                                          bool driveOnRightSide )
{
   // Bit 0 (LSB) : freeway
   // Bit 1       : ramp
   // Bit 2       : driveOnRightSide
   byte attributes = 0;
   if (freeway) {
      attributes |= 0x1;
   }
   
   if (ramp) {
      attributes |= 0x2;
   }

   if ( driveOnRightSide ) {
      attributes |= 0x4;
   }
   
   return (attributes);
}

inline void 
ExpandRouteReplyPacket::extractAttributes( byte attributes, 
                                           bool& freeway,
                                           bool& ramp,
                                           bool& driveOnRightSide )
{
   // Bit 0 (LSB) : freeway
   // Bit 1       : ramp
   // Bit 2       : driveOnRightSide
   if (attributes & 0x1) {
      freeway = true;
   } else {
      freeway = false;
   }
   
   if (attributes & 0x2) {
      ramp = true;
   } else {
      ramp = false;
   }

   if ( attributes & 0x4 ) {
      driveOnRightSide = true;
   } else {
      driveOnRightSide = false;
   }
}


#endif




