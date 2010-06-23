/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSER_TILE_HANDLER_H
#define PARSER_TILE_HANDLER_H

#include "config.h"

#include "ParserHandler.h"
#include "LangTypes.h"
#include "ACPTileMapRequest.h"
#include "NotCopyable.h"

#include <set>

class ExpandedRoute;
class MC2BoundingBox;
class ParserThreadFileDBufRequester;
class RouteID;
class TileMapBufferHolder;
class TileMapParams;
class TileMapQuery;
class ServerTileMapFormatDesc;
class RequestWithStatus;
class TileMapRequest;
class RouteReplyPacket;
class DataBuffer;
class MC2SimpleString;
class BitBuffer;
class SharedBuffer;


class ParserTileHandler : public ParserHandler, private NotCopyable {
public:
   ParserTileHandler( ParserThread* thread,
                      ParserThreadGroup* group );

   /// Destructor
   ~ParserTileHandler();

   /**
    *    Gets the tilemaps that the query wants.
    *    @param query Query to fill in.
    */
   void getTileMaps( TileMapQuery& query );

   /**
    *    Gets many tilemaps on the format:
    *    <br />
    *    Zero-terminated string with params.<br />
    *    4 bytes big endian length <br />
    *    length bytes of data <br />
    *    <br />
    *    @param params A vector of parameters.
    *    @param startOffset The start offset in bytes.
    *    @param maxBytes    The approx maximum number of bytes to send.
    */
   DataBuffer* getTileMaps( const vector<MC2SimpleString>& params,
                            uint32 startOffset,
                            uint32 maxBytes );

   
   /**
    *   Precaches all tilemaps in the boundingbox down to the
    *   specified scalelevel. The cache can then be used by
    *   the clients. The cache is tar:ed and written into
    *   the supplied vector<byte>.
    *
    *   @param singleFile  True if an .sfd cache is desired, else tar.
    *   @param debug       True if a debug version of .sfd cache is wanted.
    *   @param bbox        The boundingbox for the area to cache.
    *   @param scale       The minimum scale to cache down to.
    *   @param lang        The language.
    *   @param layers      The layers to cache.
    *   @param cacheName   The suffix of the resulting cache dir.
    *   @param cacheMaxSize The max size in bytes of the cache.
    *   @param useGzip     Whether to use gzip or not. Default true.
    */
   void precacheClientTileMaps( bool singleFile,
                                bool debug,
                                vector<byte>& outBuf,
                                const MC2BoundingBox& bbox,
                                uint32 scale,
                                LangTypes::language_t lang,
                                const set<int>& layers,
                                const char* cacheName,
                                uint32 cacheMaxSize,
                                bool useGzip = true );
   
   /**
    *   Precaches the tilemaps along the route.
    *   Will look at the speedlimit of the different streets
    *   in the route to select an appropriate detaillevel.
    *   The cache can then be used by the clients. 
    *   The cache is tar:ed and written into
    *   the supplied vector<byte>.
    *
    *   @param outBuf      [OUT] Vector of bytes that will be
    *                            filled with the tar-file
    *                            containing the resulting cache.
    *   @param expRoute    The expanded route.
    *   @param routeID     The route id.
    *   @param lang        The language.
    *   @param layers      The ids of the layers to cache.
    *   @param cacheName   The suffix of the resulting cache dir.
    *   @param cacheMaxSize The max size in bytes of the cache.
    *   @param useGzip     Whether to use gzip or not. Default true.
    */
   void getRouteTileMapCache( vector<byte>& outBuf,
                              const ExpandedRoute* expRoute,
                              const RouteID& routeID,
                              LangTypes::language_t lang,
                              const set<int>& layerIDs,
                              const char* cacheName,
                              uint32 cacheMaxSize,
                              bool useGzip );
   
   /**
    *   Precaches the tilemaps along the route.
    * 
    *   This version will fetch the route from the database
    *   and perform the route expansion.
    *   
    *   Will look at the speedlimit of the different streets
    *   in the route to select an appropriate detaillevel.
    *   The cache can then be used by the clients. 
    *   The cache is tar:ed and written into
    *   the supplied vector<byte>.
    *
    *   @param outBuf      [OUT] Vector of bytes that will be
    *                            filled with the tar-file
    *                            containing the resulting cache.
    *   @param routeID     The route id.
    *   @param lang        The language.
    *   @param layers      The ids of the layers to cache.
    *   @param cacheName   The suffix of the resulting cache dir.
    *   @param cacheMaxSize The max size in bytes of the cache.
    *   @param useGzip     Whether to use gzip or not. Default true.
    */
   void getRouteTileMapCache( vector<byte>& outBuf,
                              const RouteID& routeID,
                              LangTypes::language_t lang,
                              const set<int>& layerIDs,
                              const char* cacheName,
                              uint32 cacheMaxSize,
                              bool useGzip );
   
private:

   /**
    * The internal version of getBitMap that is called after getBitMap has
    * updated the paramStr.
    *
    * @param paramStr The bitmap tile param.
    * @param xFactor The scale factor for the bitmap.
    */
   BitBuffer* getRealBitMap( const char* paramStr, int xFactor );

   /**
    * Returns a server tilemap format desc, determined by the current user.
    * @param paramString a param string.
    */
   STMFDParams getDefaultSTMFDParams( const char* paramString = NULL ) const;
   /**
    * Requests cache, this one checks the ACP params too.
    * @param param the cache key
    * @param retVector return vector that contains all small buffers in cache
    * @return pointer to cache buffer
    */
   BitBuffer* requestCache( const MC2SimpleString& param, 
                            std::vector<TileMapBufferHolder>& retVector );
   /**
    * Requests cache without checking for ACP params
    * @param param the cache key
    * @param retVector return vector that contains all small buffers in cache
    * @return pointer to cache buffer
    */
   BitBuffer*
   requestCacheDirect( const MC2SimpleString& inParam, 
                       vector<TileMapBufferHolder>& retVector );

   /**
    * Stores parameter to cache on disc
    * @param param the tile map param string
    * @param tileMapBuffers tile map data
    * @return true if the cache was written to disc
    */
   bool storeCache( const MC2SimpleString& param,
                    const std::vector<TileMapBufferHolder>& tileMapBuffers );
   /**
    * Stores parameter to cache on disc, same as the function above but with
    * request as the parameter. This function might store more than one buffer
    * to disc. For example ACP tile maps.
    * @param param the tile map param string
    * @param tileMapBuffers tile map data
    * @return true if the cache was written to disc
    */
   bool storeCache( const MC2SimpleString& param,
                    const TileMapRequest* request );

   /**
    *   Gets a non-tileMap, currently desc, desc CRC and bitmap.
    */
   BitBuffer* getNonTileMap( const MC2SimpleString& paramStr );
   
   /**
    *   Gets a non-tileMap, currently desc, desc CRC and bitmap.
    */
   BitBuffer* getNonTileMap( const char* paramStr );

   /**
    *   Gets a bitmap. Converts it to the correct format if necessary.
    *   @return NULL if impossible.
    */
   BitBuffer* getBitMap( const char* paramStr );
   
   /**
    *   Precaches all the tilemaps present in allParams.
    *   The cache can then be used by the clients. 
    *   The cache is tar:ed and written into
    *   the supplied vector<byte>.
    *
    *   @param outBuf      [OUT] Vector of bytes that will be
    *                            filled with the tar-file
    *                            containing the resulting cache.
    *   @param allParams   All the params of the tilemap to include
    *                      in the cache.
    *   @param lang        The language.
    *   @param layers      The layers to cache.
    *   @param cacheName   The suffix of the resulting cache dir.
    *   @param cacheMaxSize The max size in bytes of the cache.
    *   @param useGzip     Whether to use gzip or not. Default true.
    *   @param routeID     [Optional] Set to the route id in case 
    *                      route tilemaps are included.
    */
   void precacheClientTileMaps( vector<byte>& outBuf,
                                const SharedBuffer& allParams,
                                LangTypes::language_t lang,
                                const char* cacheName,
                                uint32 cacheMaxSize,
                                bool useGzip = true,
                                const RouteID* routeID = NULL );

   /**
    *   Precaches tilemaps for a single file cache.
    *   @outBuf [OUT] Vector of bytes that will be
    *   @param cacheName the cache name
    *   @param debug If to include debug strings in SFD data.
    *   @param bbox bounding box for the area
    *   @param layers
    *   @param extraParams extra parameters
    *   @param useGzip if the data should be packed
    *   @param lang      The language.
    *   @param scale
    */
   void precacheClientTileMapsSFD( vector<byte>& outBuf,
                                   const char* cacheName,
                                   bool debug,
                                   const MC2BoundingBox& bbox,
                                   const set<int>& layers,
                                   const set<MC2SimpleString>& extraParams,
                                   bool useGzip,
                                   LangTypes::language_t lang,
                                   uint32 scale );
   
   /**
    *    Will calculate all the tilemap params that are along
    *    the specified route.
    *    XXX: This method is VERY slow.
    *
    *    @param   expRoute    The expanded route.
    *    @param   routeID     The route id.
    *    @param   lang        The language.
    *    @param   layerIDs    The ids of the layers to include along
    *                         the route.
    *    @param   routeParams [OUT] Will be filled with the tilemap
    *                         params along the route.
    */
   void getRouteTileMapParams( const ExpandedRoute* expRoute,
                               const RouteID& routeID,
                               LangTypes::language_t lang,
                               const set<int>& layerIDs,
                               set<MC2SimpleString>& routeParams );
   
   /**
    *    Erases all string and data maps for this param.
    */
   void eraseMapsFromCache( TileMapParams params );
   
   /**
    *    Get route tilemaps.
    */
   DataBuffer* getRouteTileMaps( const ExpandedRoute* expRoute,
                                 const RouteID& routeID,
                                 LangTypes::language_t lang,
                                 const set<int>& layerIDs,
                                 uint32 startOffset,
                                 uint32 maxBytes );
      
   /**
    * Remove a TileMap from the tile cache.
    *
    * @param paramStr The paramstring to be removed.
    */
   void removeTileMapBUffer( const char* paramStr );
      
   
   /**
    *    Checks and possibly corrects the tile map CRCs.
    *    <br />
    *    If a string tilemap has a different CRC than its
    *    corresponding tilemap, it is probably because of
    *    the fact that the maps are created using Swedish
    *    strings and  thus  the  concatenation of streets
    *    with the same name  can  be different from other
    *    languages. It will then try to exchange the text
    *    map to one created with Swedish as language.
    *    <br />
    *    If the CRC still differs,  it can be because the
    *    database has been down while creating one of the
    *    maps. In this case all the maps in all languages
    *    are deleted.
    *    <br />
    *    Will also put correct buffers into the cache,
    *    @param buffers Buffers to check and correct.
    */
   void correctCRCsAndPutInCache( const TileMapRequest* request );
   /**
    * Same as function above, this one does all the work.
    * @see correctCRCsAndPutInCache
    * @param shouldAssert true if the function should assert if crc could
    *        not be corrected
    * @return true if crc was ok or it were successfully recovered
    */
   bool correctCRCsAndPutInCache( const TileMapRequest* request, 
                                  bool shouldAssert );
   /**
    *    Replaces the string with a DXXX if the server prefix is wrong.
    *    Shouldn't happen very often, since we have concluded that it
    *    isn't a very good idea to change server prefix.
    */
   MC2SimpleString checkServerPrefixAndReplace( const MC2SimpleString& param );
    
   /**
    *    Will MC2_ASSERT that the cached tile maps have correct CRC.
    *    This method should not be called in the production cluster.
    */
   void assertCachedCRC( const char* param, 
                         const std::vector<TileMapBufferHolder>& crcBuffers,
                         const ServerTileMapFormatDesc* desc );
   /**
    * Sends tile map requests.
    * @param reqs RequestWithStatus vector, this will be destroyed.
    * @param tilereqs TileMapRequests vector, this will be destroyed.
    * @param routePackets will be destroyed
    * @param query the TileMapQuery to add buffers to.
    * @return number of buffers added to query
    */
   int requestTiles( vector<RequestWithStatus*>& reqs, 
                     vector<TileMapRequest*>& tilereqs,
                     vector<RouteReplyPacket*>& routePackets,
                     TileMapQuery& query );

   /**
    *    Will create empty TileMapBufferHolders for the param(s).
    *    @param params Must be saved since the strings in retVector
    *                  point into them
    */
   void createEmptyMaps( const MC2SimpleString& inParam,
                         vector<TileMapBufferHolder>& retVector,
                         vector<MC2SimpleString>& params,
                         const ServerTileMapFormatDesc& tmfd );

   TileMapRequest*
   createTileMapRequest( const ServerTileMapFormatDesc& mapDesc,
                         const TileMapParams& params,
                         const RouteReplyPacket* routeReplyPack );

   /**
    * Fetches ACP tile map data and stores them in tilereqs.
    * @param cacheStrings the names of the data to fetch
    * @param cacheBuffers buffers from cache, must be deleted
    * @param tilereqs the return requests to be used with ACPTileMapRequest
    * @param rights the rights for each request in tilereqs
    * @return true on success, else false
    */
   bool getACPTileRequests( const ACPTileMapRequest::
                            CacheStrings& cacheStrings,
                            vector<BitBuffer*>& cacheBuffers, 
                            ACPTileMapRequest::RequestInfoVector& tilereqs,
                            ACPTileMapRequest::RightsVector& rights );

   /**
    * Compose ACP tilemap from cache.
    * @param param the tilemap param to compose ACP buffer from
    * @return a composed bit buffer or NULL if there is no cache to 
    *         compose from or tile map request.
    */
   BitBuffer* composeACP( const TileMapParams& param,
                          vector<TileMapBufferHolder>& buffers );

   /**
    * Determins whether the image size is correct.
    * @param str Determin image size from this string
    * @return true if the image size is ok.
    */
   bool isValidUserImageSize( const MC2String& str );

   /**
    *    Cache of tilemaps and other tile-stuff.
    */
   ParserThreadFileDBufRequester* m_tileMapCache;

   /**
    * Whether we are in acp mode or not.
    */
   bool m_acpMode;

   /**
    * Describes a valid user image size.
    * User image size in this cases refers to
    * the client specified image size.
    */
   struct ImageSize {
      ImageSize( uint32 width, uint32 height ):
         m_width( width ),
         m_height( height ) {
      }

      bool operator < ( const ImageSize& other ) const {
         if ( m_width != other.m_width ) {
            return m_width < other.m_width;
         }
         return m_height < other.m_height;
      }

      uint32 m_width; ///< in pixels.
      uint32 m_height; ///< in pixels.
   };

   typedef std::set< ImageSize > ImageSizes;

   /// Contains all valid image sizes
   ImageSizes m_validImageSizes;
};


#endif
