/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ACPTILEMAPREQUEST_H
#define ACPTILEMAPREQUEST_H

#include "TileMapRequest.h"
#include "MC2String.h"

#include <vector>

class ParserTileHandler;
class MultiRequest;
class ParserThread;

/**
 * Holds tile map request data for ACP tile map request.
 */
class ACPRequestHolder {
public:
   explicit ACPRequestHolder( const TileMapRequest* request );
   ACPRequestHolder( const TileMapParams& params,
                     const TileMapRequest::TileBuffers& buffers ):
      m_params( params ),
      m_buffers( buffers ) {
   }

   const TileMapParams& getTileMapParams() const {
      return m_params;
   }

   const TileMapRequest::TileBuffers& getTileMapBufferHolders() {
      return m_buffers;
   }

private:
   TileMapParams m_params;
   TileMapRequest::TileBuffers m_buffers;
};

/**
 * Access Controlled POI Tile Map Request.
 * Composes a subset of TileMapRequests with specific map rights.
 * Handles most of the ACP functions such as tile map cache names, tile
 * requests and merging of each tile map requests data buffer holder.
 *
 */
class ACPTileMapRequest: public TileMapRequest {
public:
   /// holds information about request
   typedef vector< TileMapRequest* > RequestVector;
   typedef vector< ACPRequestHolder* > RequestInfoVector;
   /// holds rights
   typedef vector< MapRights::Rights > RightsVector;
   /// type for cache strings returned in getCacheNames. @see getCacheNames
   typedef vector< pair< MC2String, MapRights::Rights > > CacheStrings;

   /** maps param string to request.
    * The cache param string, i.e it will have map rights included.
    */
   typedef vector< pair< MC2String, const TileMapRequest::TileBuffers*> > ParamRequests;

   ACPTileMapRequest( const RequestData& reqOrID,
                      const ServerTileMapFormatDesc& mapDesc,
                      const TileMapParams& params,
                      const TopRegionRequest* topReq,
                      const MapRights& rights,
                      const RequestVector& doneRequests,
                      RightsVector& doneRights );

   /// Same as above, but with information passed as ACPRequestHolder
   ACPTileMapRequest( const RequestData& reqOrID,
                      const ServerTileMapFormatDesc& mapDesc,
                      const TileMapParams& params,
                      const TopRegionRequest* topReq,
                      const MapRights& rights,
                      RequestInfoVector& doneRequests,
                      RightsVector& doneRights );

   virtual ~ACPTileMapRequest();


   /// @see RequestWithStatus
   void processPacket( PacketContainer* pack );
   /// @see Request
   PacketContainer* getAnswer();
   /// @see Request
   PacketContainer* getNextPacket();

   /// @see TileMapRequest
   void getTileMapBuffers( vector<TileMapBufferHolder>& holder ) const;

   /// @param requests maps param string ( including map right ) to tilemap request
   void getTileMapRequests( ParamRequests& requests ) const;

   /**
    * @param inParam tilemap parameter to use
    * @param rights the map rights that the strings should represent.
    * @param cacheNames returns cache names for each tilemap that the request
    *                   would require to generate upon a request.
    */
   static void getCacheNames( const TileMapParams& inParam, const MapRights& rights,
                              CacheStrings& cacheNames );

   static bool isACPParam( const TileMapParams& inParam );

   static void createTileMapRequests( ParserThread& thread,
                                      const ServerTileMapFormatDesc& mapDesc,
                                      const TileMapParams& params,
                                      const TopRegionRequest* topReq,
                                      const MapRights& rights,
                                      vector<TileMapRequest*>& requests,
                                      vector<MapRights::Rights>& rightsVector );
   
private:
   /// compose the real request
   void composeRequest( const RequestData& reqOrID );

   PacketContainer* m_request; ///< the one that does the request work
   RequestInfoVector m_tilerequests; ///< information about requests
   vector<MapRights::Rights> m_rights; ///< single rights for each request.
   PacketContainer* m_answer; ///< final answer
};



#endif
