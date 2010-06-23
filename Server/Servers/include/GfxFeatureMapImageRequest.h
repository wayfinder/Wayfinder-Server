/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXFEATUREMAPIMAGEREQUEST_H
#define GFXFEATUREMAPIMAGEREQUEST_H

#include "config.h"

#include "Request.h"

#include "RouteArrowType.h"
#include "ImageDrawConfig.h"
#include "MapRights.h"

#include "MapUtility.h"
#include "ScreenSize.h"

#include "MC2BoundingBox.h"

class RouteReplyPacket;
class MC2BoundingBox;
class MapSettings;
class PacketContainer;
class TopRegionRequest;
class GFMDataHolder;
class BBoxReplyPacket;
class UserItem;

/** 
 *   A request that asks for and receives a gfx feature map.
 *
 */
class GfxFeatureMapImageRequest : public RequestWithStatus {
   public:
      /// Makes the GfxFeatureMapImageRequest own the MapSettings.
      void ownMapSettings();

      /**
       *    Create a GfxFeatureMapImageRequest.
       *    
       *    @param   idOrParent   Unique request ID or parent request.
       *    @param   bboxLatLon   The requested bbox.
       *    @param   screenX      Width of requesters area, 0 is inf.
       *    @param   screenY      Height of requesters area, 0 is inf.
       *    @param   language     The prefered language of names.
       *    @param   p            The RouteReplyPacket.
       *    @param   image        Is set to true if you want an image in 
       *                          return. If it's set to false you get a 
       *                          GfxFeatureMap in return.
       *    @param   exportFormat Is set to true if you want a
       *                          GfxExportFeatureMap in return. If you
       *                          want a GfxFeatureMap or an image in
       *                          return it is set to false.
       *    @param   drawCopyRight  If we want a copyright in the image.
       *    @param   format       The Image format.
       *    @param   sizeParam    The size parameter from the server.
       *    @param   mapSettings  The MapSettings to use.
       *    @param   topReq       Pointer to valid TopRegionRequest with data
       *    @param   maxScale     The maximum (most detailed) scalelevel
       *                          that is requested. If INVALID_SCALE_LEVEL, 
       *                          the screensize will be used to determine
       *                          this.
       *    @param   minScale     The minimum (least detailed) scalelevel
       *                          that is requested. If not specified,
       *                          CONTINENT_LEVEL (the least detailed 
       *                          scalelevel) will be used.
       *    @param   filtScale    The scalelevel used when filtering features.
       *                          If INVALID_SCALE_LEVEL, the maximum (most
       *                          detailed) scalelevel will be used.
       *    @param   extractForTileMaps   If the gfxfeaturemap should be
       *                                  used for creating tilemaps or not.
       *    @param   zoom Level of zoom for some projections.
       */
      GfxFeatureMapImageRequest (const RequestData& idOrParent,
                                 const MC2BoundingBox* bboxLatLon,
                                 uint16 screenX,
                                 uint16 screenY,
                                 LangTypes::language_t language,
                                 const RouteReplyPacket* p,
                                 bool image,
                                 bool exportFormat,
                                 bool drawCopyRight,
                                 ImageDrawConfig::imageFormat format,
                                 uint16 sizeParam,
                                 MapSettings* mapSettings,
                                 const TopRegionRequest* topReq,
                                 const MC2BoundingBox& ccBBox,
                                 uint32 maxScale = INVALID_SCALE_LEVEL,
                                 uint32 minScale = CONTINENT_LEVEL,
                                 uint32 filtScale = INVALID_SCALE_LEVEL,
                                 bool extractForTileMaps = false,
                                 uint32 zoom = MAX_UINT32 );
      
      /**
       *    Destructor.
       */
      virtual ~GfxFeatureMapImageRequest();

      /** 
       *   Process the answer from the mapmodule.
       *   @param ans   The answer from the mapmodule.
       */
      virtual void processPacket(PacketContainer* ans);

      /**
       *   In case an image or the gfxfeature export format is requested
       *   then the answer will be in the form of a replypacket.
       *   
       *   In case a gfxfeaturemap is requested, then the answer should
       *   be retrieved by getGFMDataHolder() instead. In that case
       *   this method will return NULL.
       *
       *   Note that the caller of this method must delete the answer
       *   afterwards.
       *   
       */
      virtual PacketContainer* getAnswer();

      /**
       *   Get the GFMDataHolder, containing the necessary information
       *   to construct the gfxfeaturemap. 
       */
      GFMDataHolder* getGFMDataHolder() const;
      
      /**
       *    @return The status of the request when it is done.
       */
      StringTable::stringCode getStatus() const;

      /**
       * Sets the route turn to draw on the image. This requires a route.
       * 
       * @param beforeTurn The index of the feature before the turn.
       *                   
       *                   NB.
       *                   This is the index counting from the first rotue
       *                   feature, i.e. if the empty features should be
       *                   used to convert this id to the feature id in the
       *                   GfxFeatureMap, 1 must be added in order to
       *                   compensate for the route origin.
       *
       * @param afterTurn The index of the feature after the turn.
       *                   
       *                   NB.
       *                   This is the index counting from the first rotue
       *                   feature, i.e. if the empty features should be
       *                   used to convert this id to the feature id in the
       *                   GfxFeatureMap, 1 must be added in order to
       *                   compensate for the route origin.
       *
       * @param arrow The arrow type used to indicate the turn.
       */
      void setRouteTurn( uint32 beforeTurn, uint32 afterTurn,
                         RouteArrowType::arrowType arrow
                         = RouteArrowType::TURN_ARROW );

      /**
       * Set the rotation for the map.
       *
       * @param rotationAngle Rotation in degrees. Valid range [0 - 360],
       *                      0 => north up, clocwise rotation is positive.
       */
      void setMapRotation( int16 rotationAngle );

      /**
       * Adds a symbol to the final map.
       *       
       * @param lat The latitude of the symbol.
       * @param lon The longitude of the symbol.
       * @param name The name of the symbol.
       * @param symbol The type of symbol.
       * @param symbolImage The image of the feature used if USER_DEFINED
       *                    symbol, is copied. Default "".
       */
      void addSymbolToMap( int32 lat, int32 lon,
                           const char* name,
                           GfxSymbolFeature::symbols symbol,
                           const char* symbolImage = "" );


      /**
       * Adds a symbol map to the request.
       * 
       * @param map The map to mergeInto the request.
       */
      void addSymbolMap( GfxFeatureMap* map );

      /**
       * @name Methods for getting the coordinates for the
       *       requested map area.
       */
      //@{
         /**
          * @return  Upper left latitude.
          */
         inline int32 getUpperLeftLat() const;

         /**
          * @return  Upper left longitude.
          */
         inline int32 getUpperLeftLon() const;

         /**
          * @return  Lower right latitude.
          */
         inline int32 getLowerRightLat() const;

         /**
          * @return  Lower right longitude.
          */
         inline int32 getLowerRightLon() const;

         /**
          *  @return  The MC2BoundingBox for city centres.
          */
         inline MC2BoundingBox& getCityCentresBoundingBox() const;
                  
      //@}

      /**
       * Set the allowed maps.
       *
       * @param maps The allowed maps. NULL means all maps are allowed.
       */
      inline void setAllowedMaps( set< uint32 >* maps );

   void setCopyright( const MC2String& copyright ) { m_copyright = copyright; }

      /**
       * Set the user for the request.
       */
      void setUser( const UserItem* user );

private:

      /**
       *    Create and enqueue the alive packet request to the InfoModule.
       */
      void makeInfoAliveRequest();

      /**
       *    Creates and enqueues initial GfxFeatureMapImageRequestPacket(s).
       */
      void makeInitialMapRequests();

     /**
      *     Creates and enqueues the initial requests for the supplied
      *     bounding box.
      */
      void makeInitialMapRequests( const MC2BoundingBox& box,
                                   uint32& count );

      /**
       *    Creates and enqueues a new GfxFeatureMapImageRequestPacket.
       *    @param  mapID       The mapid.
       *    @param  nbrPackets  The number of parts the packet consists of.
       *    @param  includeCountryPolygon If the country polygon should be
       *                        included/drawn from the map with id mapID
       *                        (if map is country map).
       */
      void makeGfxFeatureMapRequestPacket(uint32 mapID,
                                          const MC2BoundingBox& bbox,
                                          bool includeCountryPolygon,
                                          uint32 requestTagToUse );
      
      /**
       *    If InfoModule is available, create and enqueue requests for all
       *    pending map IDs.
       */
      void makeGfxFeatureMapRequestPacketsForInfoModule(
         const vector<uint32>& mapIDs,
         const MC2BoundingBox& bbox,
         uint32 origRequestTag );
     /**
      * Creates packets for the module that creates events data.
      */
      void makeRequestPacketsForEvents( const vector<uint32>& mapIDs,
                                        const MC2BoundingBox& bbox,
                                        uint32 origRequestTag );
   
      /**
       *    Enqueue GfxFeatureMapImageRequestPackets corresponding to the
       *    RouteReplyPacket passed
       *    @param   p  The RouteReplyPacket which has the 
       *                routing information
       */
      void makeGfxFeatureMapRequestPackets(RouteReplyPacket* p);

      /**
       *    Creates and enqueue a GfxFeatureMapImageRequestPacket.
       *    @param  size  The size of the GfxFeatureMap.
       *    @param  data  The GfxFeatureMap data.
       *    @param  copyright The copyright string for the map data.
       */
      void makeGfxFeatureMapImageRequestPacket( uint32 size, DataBuffer* data,
                                                const char* copyright);

      /**
       *    Concatenates the answers.
       */
      void concatenateMapAnswers();

      /**
       *    Handles the bbox reply(ies)
       */
      void handleBBoxReply( const BBoxReplyPacket* packet );

      /**
       * Check if a mapID is allowed.
       * 
       * @param mapID The mapID to check if it is allowed.
       * @return True if mapID is allowed false if not.
       */
      bool checkAllowedMapID( uint32 mapID ) const;

      /**
       *   Create the speical allowed maps for Tre in Italy hack.
       */
      set<uint32>* getAllowedGfxMapIDs() const;
   
      /**
       *    The requestpacket.
       */
      PacketContainer* m_request;

      /**
       *    The answer in case an image or export format is requested.
       */
      PacketContainer* m_answer;

      /**
       *    The answer in case a gfxfeaturemap is requested.
       */
      GFMDataHolder* m_gfmDataHolder;
      
      /**
       *    The answers from the slimmmap requests.
       */
      vector<PacketContainer*> m_mapReplies;
      
      /**
       *    @name The information from the original request packet.
       */
      //@{
      
         ///   Upper left latitude.
         int32 m_upperLeftLat;
         
         ///   Upper left longitude.
         int32 m_upperLeftLon;

         ///   Lower right latitude.
         int32 m_lowerRightLat;

         ///   Lower right longitude.
         int32 m_lowerRightLon;

         /// The MC2BoundingBox for city centres
         MC2BoundingBox m_ccBBox;

         /// size of screen in pixels.
         ScreenSize m_screenSize;

         /// The number of colors in the map.
         byte m_nbrColors;

         /// The most detailed scale-level (meters/pixel)
         uint32 m_maxScaleLevel;

         /// The least detailed scale-level (meters/pixel)
         uint32 m_minScaleLevel;

         /// The scale-level (meters/pixel) used when filtering
         uint32 m_filtScaleLevel;

         /// True if the overview contents should be drawn. 
         bool m_drawOverviewContents;
         
         /// The language.
         LangTypes::language_t m_language;
         
         /// Origin IP (of the requester).
         uint32 m_originIP;

         /// Origin port (of the requester).
         uint16 m_originPort;

         /// The route reply packet
         const RouteReplyPacket* m_routeReplyPacket;

         bool m_drawImage;
         bool m_exportFormat;


         /// If to draw copyright on image.
         bool m_drawCopyRight;

    
         /// The format of the image to be returned
         ImageDrawConfig::imageFormat m_imageFormat;

         /// The size parameter from the server
         uint16 m_sizeParam;

         /// MapSettings to delete
         MapSettings* m_mapSettingsToDelete;

         /// The MapSettings
         MapSettings* m_mapSettings;
         
         /// Determines what type of arrow to draw, if any.
         RouteArrowType::arrowType m_arrowType;

         /// The beforeturn.
         uint32 m_beforeTurn;

         /// The afterturn.
         uint32 m_afterTurn;

         /**
          * The rotation angle used for among others corssing maps 
          * expressed in degrees [ 0 - 360 ].
          */ 
         int16 m_mapRotation;
      //@}


      /**
       * Indicates if a scalable map should be generated. Is 
       * set to true if the maxScale parameter in the
       * constructor is INVALID_SCALE_LEVEL.
       */         
       bool m_scalable;

      /**
       * The map with all the userdefined symbols.
       */
      GfxFeatureMap* m_symbolMap;

      /**
       * If traffic information should be included on the map.
       */
      bool m_includeTrafficInfo;

      /**
       * Whether to use events or not.
       */
      bool m_useEvents;

      /**
       * The maps handled, used to retrieve traffic information
       */ 
      set< uint32 > m_mapsHandled;

      /**
       * The allowed maps, may be NULL.
       */ 
      set< uint32 >* m_allowedMaps;

      /**
       *   The set of special hack allowed maps only for certain
       *   topregions controlled by SERVER_ALLOWED_GFX_REGIONS
       */
      set<uint32>* m_specialAllowedMaps;
   
      /**
       * If the gfxfeaturemap should be used for creating tilemaps.
       */
      bool m_extractForTileMaps;

      /// pointer to valid topregionrequest *with* data
      const TopRegionRequest* m_topReq;

      /**
       *   True if only the route map should be included.
       */
      bool m_onlyRoute;

      /**
       *   The status of the request.
       */
      StringTable::stringCode m_status;
   /// For overriding the copyright string, mainly used with image maps
   MC2String m_copyright; 

      /// The user of the request, may be NULL. Set in setUser.
      const UserItem* m_user;
};


// ========================================================================
//                                       Implementation ofinlined methods =


inline int32
GfxFeatureMapImageRequest::getUpperLeftLat() const
{
   return (m_upperLeftLat);
}

inline int32
GfxFeatureMapImageRequest::getUpperLeftLon() const
{
   return (m_upperLeftLon);
}


inline int32
GfxFeatureMapImageRequest::getLowerRightLat() const
{
   return (m_lowerRightLat);
}


inline int32
GfxFeatureMapImageRequest::getLowerRightLon() const
{
   return (m_lowerRightLon);
}

inline void 
GfxFeatureMapImageRequest::setAllowedMaps( set< uint32 >* maps ) {
   m_allowedMaps = maps; 
}

inline void
GfxFeatureMapImageRequest::setUser( const UserItem* user ) {
   m_user = user;
}

#endif

