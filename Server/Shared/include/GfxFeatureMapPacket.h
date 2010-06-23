/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXFEATUREMAPPACKET_H
#define GFXFEATUREMAPPACKET_H

#include "config.h"
#include "Packet.h"
#include "LangTypes.h"
#include "ScreenSize.h"

#include <list>

class MC2BoundingBox;
class DataBuffer;
class MapSettings;
class GenericMap;
class UserRightsMapInfo;
class UserUser;

#define GFXFEATUREMAP_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define GFXFEATUREMAP_REPLY_PRIO   DEFAULT_PACKET_PRIO

/**
  *   Packet used for requesting a gfx-feature map over either a 
  *   specified area, or a route.
  *
  *   It is optional to add node id:s to this packet. In case any node ids
  *   are present then the GfxFeatureMap will cover a route. 
  *   If no node ids are
  *   present, then the GfxFeatureMap will cover the area specified by the
  *   supplied boundingbox. The methods for getting startoffset and
  *   endoffset of the route does only make sense when the number of
  *   node ids are more than zero.
  *
  *   After the normal header this packet contains:
  *   Table begins at REQUEST_HEADER_SIZE
  *   \begin{tabular}{lll}
  *      Pos & Size    & \\ \hline
  *      0        & 4 bytes & Upper left corner latitude for requested map \\
  *      4        & 4 bytes & Upper left corner longitude for requested map \\
  *      8        & 4 bytes & Lower right corner latitude for requested map \\
  *      12       & 4 bytes & Lower right corner longitude for requested map \\
  *      16       & 2 bytes & Screen size x \\
  *      18       & 2 bytes & Screen size y \\
  *      20       & 4 bytes & Max (most detailed) scale-level\\
  *      24       & 4 bytes & Min (least detailed) scale-level\\
  *      28       & 4 bytes & Scale-level used when filtering\\
  *      32       & 4 bytes & language\\
  *      36       & 2 bytes & Start offset \\
  *      38       & 2 bytes & End offset \\
  *      40:7     & 1 bit   & Is this the start of the route
  *                           (1 == use startoffset) \\ 
  *      40:6     & 1 bit   & Is this the end of the route 
  *                           (1 == use endoffset) \\
  *      40:5     & 1 bit   & Draw overview content?
  *                           (1 == yes) \\
  *      40:4     & 1 bit   & Draw the country polygon? 
  *                           (1 == yes) \\
  *      40:3     & 1 bit   & Extract for tilemaps? 
  *                           (1 == yes) \\
  *      41       & 1 byte  & The number of requestpackets (one for each
  *                           map that should be extracted.)
  *      42       & 2 bytes & Nbr of node ids (=nbrNodeIDs) to follow \\
  *      44       & 4 bytes & Length of map settings (=msLen).
  *      48       & msLen   & Map settings.
  *      xx       & 4 bytes & Node id in route. (Repeated nbrNodeIDs times) \\
  *   \end{tabular}
  *
  */
class GfxFeatureMapRequestPacket : public RequestPacket {
public:
      
      /**
        *   Creates a GfxFeatureMapRequestPacket with all parameters.
        *   @param   reqID          The RequestID.
        *   @param   packetID       The PacketID.
        *   @param   upperLeftLat   Upper left latitude of requested map.
        *   @param   upperLeftLon   Upper left longitude of requested map.
        *   @param   lowerRightLat  Lower right latitude of requested map.
        *   @param   lowerRightLon  Lower right longitude of requested map.
        *   @param   screenSizeX    Screen size x.
        *   @param   screenSizeY    Screen size y.
        *   @param   maxScaleLevel  The maximum (most detailed) scale-level 
        *                           to use when extracting
        *                           and creating the map.
        *   @param   minScaleLevel  The minimum (least detailed) scale-level 
        *                           to use when extracting
        *                           and creating the map.
        *   @param   filtScaleLevel The scale-level to use when filtering
        *                           the features of the map.
        *   @param   language       The prefered language of names.
        *   @param   mapSettings    MapSettings.
        *   @param   drawOverviewContents True if the contents of the overview
        *                                 should be drawn.
        *   @param   extractForTileMaps Extract for tilemaps. Default false.
        */
      GfxFeatureMapRequestPacket(uint32 mapID,
                                 const UserUser* user,
                                 uint16 reqID,
                                 uint16 packetID,
                                 int32  upperLeftLat,
                                 int32  upperLeftLon,
                                 int32  lowerRightLat,
                                 int32  lowerRightLon,
                                 uint16 screenSizeX,
                                 uint16 screenSizeY,
                                 uint32 maxScaleLevel,
                                 uint32 minScaleLevel,
                                 uint32 filtScaleLevel,
                                 LangTypes::language_t language,
                                 const MapSettings* mapSettings,
                                 bool   ignoreStartOffset = true,
                                 bool   ignoreEndOffset = true,
                                 uint16 startOffset = 0,
                                 uint16 endOffset = 0,
                                 bool   drawOverviewContents = false,
                                 bool   extractForTileMaps = false );

      /**
        *   Get the coordinates encapsulating the requested map.
        *   All parameters are output parameters.
        *   @param   upperLeftLat   Upper left latitude of requested map.
        *   @param   upperLeftLon   Upper left longitude of requested map.
        *   @param   lowerRightLat  Lower right latitude of requested map.
        *   @param   lowerRightLon  Lower right longitude of requested map.
        */
      void getBoundingCoordinates(int32&  upperLeftLat,
                                  int32&  upperLeftLon,
                                  int32&  lowerRightLat,
                                  int32&  lowerRightLon) const;
      
      /**
        *   Get the coordinates encapsulating the requested map in the
        *   form of a bounding box.
        *   @param   bbox  Preallocated boundingbox.
        */
      void getMC2BoundingBox(MC2BoundingBox* bbox) const;
      
      /**
        *   Set the coordinates encapsulating the requested map.
        *   @param   upperLeftLat   Upper left latitude of requested map.
        *   @param   upperLeftLon   Upper left longitude of requested map.
        *   @param   lowerRightLat  Lower right latitude of requested map.
        *   @param   lowerRightLon  Lower right longitude of requested map.
        */
      void setBoundingCoordinates(int32  upperLeftLat,
                                  int32  upperLeftLon,
                                  int32  lowerRightLat,
                                  int32  lowerRightLon);

      /**
        *   Get screen size.
        *   All parameters are output parameters.
        *   @param   screenSizeX    Screen size x.
        *   @param   screenSizeY    Screen size y.
        */
      void getScreenSize(uint16& screenSizeX, uint16& screenSizeY) const;
      /// @return Screen size, see above.
      ScreenSize getScreenSize() const;
      /**
        *   Set screen size.
        *   Note that the GfxFeatureMap will use 16 bit coordinates if either
        *   screen size x or y is higher than 256, otherwise 8 bit 
        *   coordinates will be used.
        *   @param   screenSizeX    Screen size x.
        *   @param   screenSizeY    Screen size y.
        */
      void setScreenSize(uint16 screenSizeX, uint16 screenSizeY);

      
      /**
        *   Get the maximum (most detailed) scale-level to 
        *   use when extracting and creating the map.
        *   @return The scale-level.
        */
      uint32 getMaxScaleLevel() const;
     
      
      /**
        *   Set the maximum (most detailed) scale-level to 
        *   use when extracting and creating the map.
        *   @param scaleLevel The scale-level.
        */
      void setMaxScaleLevel(uint32 scaleLevel);

      
      /**
        *   Get the minimum (least detailed) scale-level to 
        *   use when extracting and creating the map.
        *   @return The scale-level.
        */
      uint32 getMinScaleLevel() const;
      
      
      /**
        *   Set the minimum (least detailed) scale-level to 
        *   use when extracting and creating the map.
        *   @param scaleLevel The scale-level.
        */
      void setMinScaleLevel(uint32 scaleLevel);

      
      /**
        *   Get the scale-level to use when filtering the features
        *   of the map.
        *   @return The scale-level.
        */
      uint32 getFiltScaleLevel() const;
     
      
       /**
        *   Set the scale-level to use when filtering the features
        *   of the map.
        *   @param scaleLevel The scale-level.
        */
      void setFiltScaleLevel(uint32 scaleLevel);


      /**
       * Get the language to use when creating the map.
       * @return The preferred language.
       */
      LangTypes::language_t getLanguage() const;
      

      /**
        *   Set the language to use when extracting and creating the 
        *   map.
        *   @param language The language for this map.
        */
      void setLanguage( LangTypes::language_t language );

      
      /**
       *    Get the mapsettings.
       *
       *    @param   mapSettings The MapSettings. (Preallocated)
       */
      void getMapSettings( MapSettings* mapSettings ) const;
      
      /**
       *    Get the mapsettings.
       *
       *    @param   mapSettings The MapSettings. (Preallocated)
       *    @param   rights      The user rights.
       */
      void getMapSettingsAndRights( MapSettings& mapSettings,
                                    UserRightsMapInfo& rights) const;
     
      
      /**
       *    Use this method to add node ids in case you want a 
       *    GfxFeatureMap covering a route instead of an area.
       *    Add one node id to the packet.
       *    @param nodeID  The node id to add.
       */
      void addNodeID(uint32 nodeID);

      /**
       *    Checks if the packet is containing any route or not.
       *    @return  True if the packet contains nodes of a route.
       */
      inline bool containsRoute() const;
      
      /**
       *    Get all route items (node ids).
       *    @param   theMap   The map that the route passes through.
       *    @param   nodeIDs  [Out parameter]. List of node ids.
       *    @return  The number of route items.
       */
      uint32 getNodeIDs( const GenericMap* theMap,
                         list<uint32>& nodeIDs ) const; 
      
      /**
       * @name Methods to get information about the start and end
       *       of the route. This information does only make sense
       *       in case the number of nodes > 0, ie. that the GfxFeatureMap
       *       that we request should consist of a route instead of
       *       an area.
       */
      //@{
         /**
          * @return True if the this packet does not include the start
          *         of the route and that the startoffset therefore
          *         can be ignored. False otherwise.
          */
         bool getIgnoreStartOffset() const;
         
         
         /**
          * @param ignore  Whether the start offset should be ignored
          *                or not. (true == ignore).
          */
         void setIgnoreStartOffset(bool ignore);
         
         
         /**
          * @return True if the this packet does not include the end
          *         of the route and that the endoffset therefore
          *         can be ignored. False otherwise.
          */
         bool getIgnoreEndOffset() const;

         
         /**
          * @param ignore  Whether the end offset should be ignored
          *                or not. (true == ignore).
          */
         void setIgnoreEndOffset(bool ignore);
         
         
         /**
          * @return  The start offset.
          */
         uint16 getStartOffset() const;


         /**
          * @param  offset The start offset.
          */
         void setStartOffset(uint16 offset);

         
         /**
          * @return  The end offset.
          */
         uint16 getEndOffset() const;
         
         
         /**
          * @param  offset The end offset.
          */
         void setEndOffset(uint16 offset);

      //@}
   
      /**
        *   Get the number of requestpackets (one for each map) that 
        *   the extraction of this GfxFeatureMap involves.
        *   @return  The number of req packets.
        */
      byte getNbrReqPackets() const;

      /**
       *   Set the number of requestpackets (one for each map) that 
       *   the extraction of this GfxFeatureMap involves.
       *   @param   nbr   The number of req packets.
       */
      void setNbrReqPackets(byte nbr);

      /**
       *   Returns true if the contents of the overview maps should be
       *   drawn.
       *   @return True if more than the LAND should be drawn in the
       *           country/overview maps.
       */
      bool getDrawOverviewContents() const;

      /**
       *   Set if the overview contents should be drawn.
       *   @param drawOverview If true, the contents of the overview
       *                       should be drawn.
       */
      void setDrawOverviewContents(bool drawOverview);
   
      /**
       *   Returns true if the country polygon of the country map 
       *   should be included/drawn.
       *   @return True if the LAND should be included/drawn in the
       *           country/overview maps.
       */
      bool getIncludeCountryPolygon() const;

      /**
       *   Set if the country polygon should be included/drawn.
       *   @param includeCountryPolygon   If true, the country polygon 
       *                      from this country map should be included/drawn.
       */
      void setIncludeCountryPolygon(bool includeCountryPolygon);

      /**
       *    Return if should be used for extracting tilemaps.
       */
      bool extractForTileMaps() const;
      
      /**
       *   Print information about the packet to stdout.
       */
      void printPacket() const;

private:
      /**
        *   Default constructor, declared private to avoid use.
        */
      GfxFeatureMapRequestPacket();

      /**
       *    Get the number of node ids of this part of the route..
       *    If the number of node ids are zero, that means that this
       *    map will not contain a route, but instead contain the area
       *    specified by the supplied bounding box.
       *    
       *    @return The number of node ids. If the return value is equal
       *            to zero, that means the route will not contain a 
       *            route, but an area instead.
       */
      uint16 getNbrNodeIDs() const;
      
      /**
       * Increment the number of node ids.
       * @return  The position to write the new node id.
       */
      inline int incNbrNodeIDs();
      
      /**
       * Get the position for a certain node id.
       * @param   i  Node id index.
       * @return  The postion for node id i.
       */
      inline int getNodeIDPos(uint32 i) const;
 
      /**
       * The positions of the things with static locations in the packet.
       */
      enum positions {
         upperLeftLat_POS  = REQUEST_HEADER_SIZE,
         upperLeftLon_POS  = upperLeftLat_POS + 4,
         lowerRightLat_POS = upperLeftLon_POS + 4,
         lowerRightLon_POS = lowerRightLat_POS + 4,
         screenSizeX_POS   = lowerRightLon_POS + 4,
         screenSizeY_POS   = screenSizeX_POS + 2,
         maxScaleLevel_POS = screenSizeY_POS + 2,
         minScaleLevel_POS = maxScaleLevel_POS + 4,
         filtScaleLevel_POS= minScaleLevel_POS + 4,
         language_POS      = filtScaleLevel_POS + 4,
         startOffset_POS   = language_POS + 4,
         endOffset_POS     = startOffset_POS + 2,
         nbrColors_POS     = endOffset_POS + 2,
         isStartAndEnd_POS = nbrColors_POS + 1, // Also drawOverview and
                                                // drawCountryPolygon
         nbrReqPackets_POS = isStartAndEnd_POS + 1,
         target_POS        = nbrReqPackets_POS + 1,
         nbrNodeIDs_POS    = target_POS + 1,
         mapSettingsLen_POS= nbrNodeIDs_POS + 2,
         
         endStatic_POS     = mapSettingsLen_POS + 4
      };

   /** Bit position for ignore start offset */
   static const int IGN_START_BIT_POS      = 7;
   /** Bit position for ignore end offset */
   static const int IGN_END_BIT_POS        = 6;
   /** Bit position for "draw overview contents" */
   static const int DRAW_OVERVIEW_BIT_POS  = 5;
   /** Bit position for draw countrymaps' country polygon */
   static const int DRAW_COUNTRYPOLYGON_BIT_POS = 4;
   /// Bit position for extract for tilemap
   static const int EXTRACT_FOR_TILEMAPS_BIT_POS = 3;
};



/**
  *   Packet that is sent as a reply to a GfxFeatureMapRequestPacket.
  *
  *   After the normal header this packet contains:
  *   Table begins at REPLY_HEADER_SIZE
  *   \begin{tabular}{lll}
  *      Pos  & Size    & \\ \hline
  *       0   & 1 bytes & Flags \\
  *       0:5 & 1 bit   & Bit set to 1 if the contents of the overview
  *                       maps should be drawn in the following requests.\\
  *       1   & 3 bytes   & Unused. \\
  *       4   & 4 bytes   & MapID \\
  *       8   & 4 bytes   & Number of requests (i) that need to be resent
  *       12  & 2 bytes   & Length of copyright string\\
  *       14  & string    & The Copyright \\
  *       xxx & 4 bytes & i instances of the map ids of the requests to 
  *                       resend \\
  *       xxx & 4 bytes & i instances of bools about including the country 
  *                       polygon or not for each map id of the requests to 
  *                       resend \\
  *       xxx & 4 bytes & Number of bytes of the GfxFeatureMap data \\
  *       xxx & xxx     & The GfxFeatureMap data
  *   \end{tabular}
  *   
  *   Note that the number of requests to resend can be 0, and it is
  *   only in that case that we will find more than 0 bytes of 
  *   GfxFeatureMap data in the reply.
  *
  *   When creating the replypacket, these methods must be called in
  *   the following order:
  *   setCopyRight() (optional)
  *   addMapIDforResending() (optional)
  *   addNbrFeaturesInNextLayer() (optional)
  *   setGfxFeatureMapData() (optional)
  *
  */
class GfxFeatureMapReplyPacket : public ReplyPacket {
   
public:
      
      /**
        *   Default constructor.
        */
      GfxFeatureMapReplyPacket(uint32 size = MAX_PACKET_SIZE);

      /**
        *   Creates a GfxFeatureMapReplyPacket by filling in the
        *   parameters from the GfxFeatureMapRequestPacket.
        *   @param   p  The GfxFeatureMapRequestPacket.
        */
      GfxFeatureMapReplyPacket(const GfxFeatureMapRequestPacket* p);

      /**
       * Set the copyright string for the map data.
       * {\it {\bf NB!} Note that this method must not be called after 
       * addMapIDForResending(), setGfxFeatureMapData() or 
       * addNbrFeaturesInNextLayer()}.
       *
       * @param copyright The copyright string.
       */
      void setCopyright( const char* copyright );

      /**
       * Get the copyright string for the map data.
       *
       * @return The copyright string for the map data.
       */
      const char* getCopyright() const;

      /**
        *   Set the GfxFeatureMap to the reply.
        *   {\it {\bf NB!} Note that this method must not be called before 
        *   addMapIDForResending() or addNbrFeaturesInNextLayer.}
        *   It does not make any sense to add a non empty GfxFeatureMap
        *   if there are requests to resend, since the GfxFeatureMap is not
        *   extracted yet at that time.
        *   @param   size  The number of bytes of the GfxFeatureMap data.
        *   @param   buf   Databuffer containing the GfxFeatureMap data.
        */
      void setGfxFeatureMapData(uint32 size, DataBuffer* data,
                                bool zipped = false );
      
      /**
       *   Get the GfxFeatureMap as a zipped or unzipped DataBuffer,
       *   as indicated by the zipped outparamter.
       *
       *   @return A DataBuffer containing the requested GfxFeatureMap.
       *           Please note that the DataBuffer is created inside
       *           this method and must be deleted by the caller.
       *           Also note that the buffer is pointing into the packet,
       *           so the packet must not be deleted during the
       *           lifetime of the DataBuffer.
       */
      DataBuffer* getGfxFeatureMapData( bool& zipped );

      /**
        *   Get the GfxFeatureMap as an unzipped DataBuffer.
        *    
        *   {\it {\bf NB!} The DataBuffer returned by this method is 
        *   created in this method but {\bf must} be deleted by the 
        *   caller!}
        *   
        *   @return A DataBuffer containing the requested GfxFeatureMap.
        *           Please note that the DataBuffer is created inside
        *           this method and must be deleted by the caller.
        *           Also note that the buffer is pointing into the packet,
        *           so the packet must not be deleted during the
        *           lifetime of the DataBuffer.
        */
      DataBuffer* getGfxFeatureMapData();

      /**
       *   Returns true if the contents of the overview maps should be
       *   drawn.
       *   @return True if more than the LAND should be drawn in the
       *           country/overview maps.
       */
      bool getDrawOverviewContents() const;

      /**
       *   Set if the overview contents should be drawn.
       *   @param drawOverview If true, the contents of the overview
       *                       should be drawn.
       */
      void setDrawOverviewContents(bool drawOverview);

      /**
       *   Returns true if the country polygon of the country map 
       *   should be included/drawn.
       *   @return True if the LAND should be included/drawn in the
       *           country/overview maps.
       */
      bool getIncludeCountryPolygon() const;

      /**
       *   Set if the country polygon should be included/drawn.
       *   @param includeCountryPolygon   If true, the country polygon 
       *                      from this country map should be included/drawn.
       */
      void setIncludeCountryPolygon(bool includeCountryPolygon);

      /**
        *   Prints part of the packet to stdout. (Not the GfxFeatureMap)
        */
      void printPacket();


      /**
       * Sets the mapID of the packet. This is the single mapID of the
       * Packet if getNbrRequestsToResend is zero (0).
       */
      void setMapID( uint32 mapID );


      /**
       * Gets the mapID of the packet. This is the single mapID of the
       * Packet if getNbrRequestsToResend is zero (0).
       */
      uint32 getMapID() const;


private:
      /**
       *    Set if gfxfeaturemap data is zipped or not.
       */
      void setZippedGfxFeatureMapData(bool zipped);

      /**
       *    Get if the gfxfeaturemap data is zipped or not.
       */
      bool getZippedGfxFeatureMapData() const;

      /**
        *   Initializes an empty reply.
        */
      void init();

      /**
        *   Increase the number of requests to resend.
        */
      inline void incNbrRequestsToResend();

      /**
        *   Add a map id of a request that needs to be resent to
        *   the mapmodule. This implies that the GfxFeatureMap is not yet
        *   extracted, ie. that the GfxFeatureMap data contains 0 bytes.
        *   {\it {\bf NB!} Note that this method must not be called after 
        *   setGfxFeatureMapData() or addNbrFeaturesInNextLayer().}
        *   @param   mapID    The map id.
        *   @param   includeCountryPolygon   If the country polygon should 
        *                     be included from this map (if country map).
        *                     Default true.
        */
      void addMapIDForResending(uint32 mapID,
                                bool includeCountryPolygon = true);

      /**
       *    @return The start position for the gfxData.
       */
      inline int getGfxStartPos() const;

   // -- Deprecated methods
   
      /**
        *   Get the number of requests that need to be resent.
        *   @return The nbr of request to resend.
        */
      inline uint32 getNbrRequestsToResend() const;

      /**
        *   Get the map id for a the request to resend.
        *   @param   i  The index of the request to resend. Note that
        *               the index must be less than 
        *               getNbrRequestsToResend().
        *   @return  The map id for request i to resend. MAX_UINT32 if
        *            the index i was invalid.
        */
      uint32 getMapIDToResend(uint32 i) const;

      /**
        *   Get info about country polygons should be included for 
        *   the map id of the request to resend.
        *   @param   i  The index of the request to resend. Note that
        *               the index must be less than 
        *               getNbrRequestsToResend().
        *   @return  Whether the country polygon should be included for
        *            the map id of request i to resend. False if
        *            the index i was invalid.
        */
      bool  getIncludeCountryPolygonForMapToResend(uint32 i) const;

   // -- 
   
      /** Position in the packet for flags */
      static const int FLAGS_POS = REPLY_HEADER_SIZE;
      /** Position in the packet for mapID */
      static const int MAPID_POS = FLAGS_POS + 4;
      /** Position in the packet for the number of requests to send */
      static const int NBR_REQUESTS_POS = FLAGS_POS + 8;
      /** Position in the packet for the length of copyright */
      static const int LENGTH_COPYRIGHT_POS = NBR_REQUESTS_POS + 4;
      /** Position in the packet for the copyright */
      static const int COPYRIGHT_POS = LENGTH_COPYRIGHT_POS + 2;
      /** Position for start of map id:s not counting copyright */
      static const int MAP_ID_START_POS = LENGTH_COPYRIGHT_POS + 2;
   
      /** Bit position among the flags for draw countrymaps' country polygon */
      static const int DRAW_COUNTRYPOLYGON_BIT_POS = 6;
      /** Bit position among the flags for draw countrymaps contents */
      static const int DRAW_OVERVIEW_BIT_POS = 5;
      /** Bit position among the flags for zipped gfxfeaturmap */
      static const int ZIPPED_GFXFEATUREMAPDATA_POS = 4;
};


// =======================================================================
//                                     Implementation of inlined methods =

// ---------- GfxFeatureMapReplyPacket ----------------

inline uint32 
GfxFeatureMapReplyPacket::getNbrRequestsToResend() const
{
   return (readLong(NBR_REQUESTS_POS));
}

inline void
GfxFeatureMapReplyPacket::incNbrRequestsToResend()
{
   writeLong(NBR_REQUESTS_POS, getNbrRequestsToResend() + 1);
}

inline int
GfxFeatureMapReplyPacket::getGfxStartPos() const
{
   // The GfxData starts at MAP_ID_START_POS + mapids*8 + length copyright
   return MAP_ID_START_POS + getNbrRequestsToResend() * 8 + 
      readShort( LENGTH_COPYRIGHT_POS );
}

// ---------- GfxFeatureMapRequestPacket ----------------

inline int
GfxFeatureMapRequestPacket::incNbrNodeIDs()
{
   uint32 nbrNodeIDs = getNbrNodeIDs();
   writeShort( nbrNodeIDs_POS, nbrNodeIDs + 1);
   mc2dbg8 << "   Increased nbr nodes to " << getNbrNodeIDs() << endl;
   return (getNodeIDPos(nbrNodeIDs));
}

inline int 
GfxFeatureMapRequestPacket::getNodeIDPos(uint32 i) const
{
   return (readLong( mapSettingsLen_POS ) + endStatic_POS + i * 4);
}

inline bool
GfxFeatureMapRequestPacket::containsRoute() const 
{
   return getNbrNodeIDs() > 0;
}

#endif

