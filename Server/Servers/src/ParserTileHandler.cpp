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

#include "DBufRequester.h"
#include "BitBuffer.h"
#include "DataBuffer.h"
#include "DataBufferUtil.h"
#include "FileDBufRequester.h"
#include "UnixMFDBufRequester.h"

#include "MC2String.h"
#include "MC2SimpleString.h"
#include "Properties.h"
#include "STLStringUtility.h"
#include "TimeUtility.h"

#include "ParserTileHandler.h"
#include "TileMapQuery.h"
#include "TileMapBufferQuery.h"
#include "TileMapClientPrecacheQuery.h"
#include "TileMapClientSFDQuery.h"
#include "TileMapRequest.h"
#include "TileMapParams.h"
#include "TileMap.h"
#include "EmptyTileMap.h"
#include "ServerTileMapFormatDesc.h"
#include "ParamsNotice.h"
#include "TileMapParamTypes.h"
#include "TileMapParamTypes.h"
#include "TileMapBufferHolder.h"
#include "TileCollectionNotice.h"
#include "FixedSizeString.h"
#include "ClientSettings.h"
#include "TileMapCreator.h"
#include "SFDHolder.h"

#include "UserData.h"

#include "FilePtr.h"
#include "File.h"
#include "ParserThreadGroup.h"
#include "DebugClock.h"

#include "ACPTileMapRequest.h"
#include "MultiRequest.h"

#include "DeleteHelpers.h"
#include "POISetProperties.h"

// For fstat and friends
#include "stat_inc.h"

// For the route handling.
#include "RoutePacket.h"
#include "ExpandRouteRequest.h"
#include "ExpandedRoute.h"
#include "ExpandRoutePacket.h"

#include "ServerTileMapFormatDesc.h"
#include "TileMapParamTypes.h"
#include "NullTileMapQuery.h"

#include "ServerTileMap.h"
#include "TempDirectory.h"

#include "ImageTools.h"
#include "ParserThreadFileDBufRequester.h"
#include "ImageTable.h"

#include "UserImage.h"
#include "STLUtility.h"
#include "SVGImage.h"

#include <memory>
#include <iterator>

namespace {
// converts string to new cache string (inserts "N" )
inline MC2SimpleString cacheString( const MC2SimpleString& str ) {
   MC2String newStr("N");
   newStr += str.c_str();
   return newStr.c_str();
}

/**
 * Sets the ACP MapRight at the end of the new param and returns it
 * Assumes originalStr length is > 0.
 * @param originalStr the original param string
 * @param newParam the modified parameter from the original
 */
MC2String setACPMapRightFromOriginal( const MC2SimpleString& originalStr,
                                      const TileMapParams& newParam ) {
   // find last of "."
   uint32 pos = originalStr.length() - 1;
   for ( ; pos != 0; --pos ) {
      if ( originalStr[ pos ] == '.' ) {
         break;
      }
   }
   // nothing to copy, this should not happen.
   if ( pos == 0 ) {
      return newParam.getAsString().c_str();
   }

   // compose new param string
   MC2String newParamStr = newParam.getAsString().c_str();
   // copy mapright string
   while ( pos < originalStr.length() ) {
      newParamStr += originalStr[ pos++ ];
   }
   mc2dbg << "[ParserTileHandler] in: " << originalStr << " out: " << newParamStr << endl;
   return newParamStr;
}

}


static inline bool isMapFormatDesc( const char* paramsStr )
{
   return TileMapParamTypes::isMapFormatDesc( paramsStr );
}

static inline bool isMap( const char* paramsStr )
{
   return TileMapParamTypes::isMap( paramsStr );
}

static inline bool isBitmap( const char* paramsStr )
{
   return TileMapParamTypes::isBitmap( paramsStr );
}

static inline bool isMapFormatDesc( const MC2SimpleString& paramsStr )
{
   return TileMapParamTypes::isMapFormatDesc( paramsStr.c_str() );
}

static inline bool isMap( const MC2SimpleString& paramsStr )
{
   return TileMapParamTypes::isMap( paramsStr.c_str() );
}

static inline bool isBitmap( const MC2SimpleString& paramsStr )
{
   return TileMapParamTypes::isBitmap( paramsStr.c_str() );
}


static inline MC2String fixCachePath( MC2String cachePath ) 
{
   const char* specReg = 
            Properties::getProperty( "SERVER_ALLOWED_GFX_REGIONS" );
   if ( specReg != NULL ) {
      char* replacedToDel = StringUtility::replaceString( specReg,
                                                          ";", "_och_" );
      specReg = replacedToDel ? replacedToDel : specReg;
      cachePath = STLStringUtility::addPathComponent(cachePath,specReg);
      delete [] replacedToDel;
      
   }
   return cachePath;
}

static inline void assertFileExists( const MC2String& fileName )
{
   FileUtils::FilePtr file( fopen( fileName.c_str(), "r" ) );
   if ( file.get() == NULL ) {
      mc2dbg2 << "[PTileH]: File " << MC2CITE(fileName) << " does not exist"
             << endl;
   } else {
      mc2dbg8 << "[PTileH]: File " << MC2CITE(fileName) << " exists" << endl;
   }
}

ParserTileHandler::ParserTileHandler( ParserThread* thread,
                                      ParserThreadGroup* group )
   : ParserHandler( thread, group ),
     m_acpMode( false )
{
   /// Create the tileMapCache.
   const char* cachePath =
      Properties::getProperty("TILE_MAP_CACHE_PATH");
   
   if ( cachePath && cachePath[0] != '\0' ) {
      // Replace the cache path if special regions are used
      MC2String newPath = fixCachePath( cachePath );
      cachePath = newPath.c_str();
      mc2dbg2 << "[ParserThread]: Will cache TMaps in "
             << MC2CITE(cachePath) << endl;
      m_tileMapCache =
         new ParserThreadFileDBufRequester( cachePath );
   } else {
      mc2log << warn << "[PTH]: No tile map cache!!" << endl;
      // No cache.
      m_tileMapCache = NULL;
   }
   // Check that the needed bitmaps are present.
   {
      const char* imagesPath = Properties::getProperty( "IMAGES_PATH" );
      if ( imagesPath == NULL ) {
         imagesPath = "./";
      }

      MC2String im = imagesPath;
      
      ServerTileMapFormatDesc tmp_desc( STMFDParams( LangTypes::english, 
                                                     false ) );
      tmp_desc.setData();
      set<MC2String> neededBmps;
      
      for ( int i = ImageTable::DEFAULT; i < ImageTable::NBR_IMAGE_SETS; ++i ) {
         tmp_desc.getNeededBitmapNames( neededBmps,
                                        static_cast<ImageTable::ImageSet>(i) );
      }
      mc2dbg2 << "[PTileH]: neededBmps.size() = " << neededBmps.size()
             << endl;
      for ( set<MC2String>::const_iterator it = neededBmps.begin();
            it != neededBmps.end();
            ++it ) {
         MC2String start = STLStringUtility::addPathComponent(im, *it);
         assertFileExists( start + ".png" );
         assertFileExists( start + ".svg" );
         assertFileExists( start + ".mif" );      
      }
   }
}

ParserTileHandler::~ParserTileHandler()
{
   delete m_tileMapCache;
}

void
ParserTileHandler::
precacheClientTileMapsSFD( vector<byte>& outBuf,
                           const char* cacheName,
                           bool debug,
                           const MC2BoundingBox& bbox,
                           const set<int>& layers,
                           const set<MC2SimpleString>& extraParams,
                           bool useGzip,
                           LangTypes::language_t lang,
                           uint32 scale )

{

   int count = 0;
   TileMapClientSFDQuery query1( m_group->
                                 getTileMapFormatDesc( STMFDParams( lang, 
                                                                    false ),
                                                       m_thread ),
                                 cacheName,
                                 debug,
                                 bbox, 
                                 layers,
                                 extraParams,
                                 useGzip,
                                 lang, 
                                 scale );
   

   
   do {
      MC2_ASSERT( count < 99 );
      getTileMaps( query1 );
   } while ( query1.reputFailed() && count++ < 100 );

   mc2dbg << "nbrAdded: " << query1.getNbrAdded() << endl;
   mc2dbg << "nbrWanted: " << query1.getNbrWanted() << endl;
   mc2dbg << "[PTH]: count: " << count << endl;
   mc2dbg << "[PTH]: Starting createBuffer" << endl;
   const SharedBuffer* buf = query1.getResult();
   mc2dbg << "[PTH]: Done createBuffer" << endl;

   // Put the result in the outbuffer.
   outBuf.clear();
   outBuf.insert( outBuf.end(),
                  buf->getBufferAddress(),
                  buf->getBufferAddress() + buf->getBufferSize() );

}

void
ParserTileHandler::
precacheClientTileMaps( vector<byte>& outBuf,
                        const SharedBuffer& allParams,
                        LangTypes::language_t lang,
                        const char* cacheName,
                        uint32 cacheMaxSize,
                        bool useGzip,
                        const RouteID* routeID )
try {
   FileUtils::TempDirectory tmpDir( "/tmp/precache_" );
   
   MC2String basepath;
   basepath += tmpDir.getPath();
   basepath += "/";
   basepath += cacheName;
   
   if ( mkdir( basepath.c_str(),
               S_IRUSR | S_IWUSR | S_IXUSR |
               S_IRGRP | S_IXGRP |
               S_IROTH | S_IXOTH ) == -1 ) {
      throw FileUtils::FileException( MC2String("Failed to create directory: ")
                                      + basepath );
   }

   // Create the multifile cache to store the data to.
   UnixMFDBufRequester cache( NULL, // No parent
                              basepath.c_str(),
                              cacheMaxSize,
                              1 ); // Only one file for the precache.
   

   // New
   // Start with all parameters in the first round and then
   // retry the missing ones in the subsequent rounds.
   int count = 0;

   TileMapClientPrecacheQuery 
      query( allParams, cache,
             m_group->
             getTileMapFormatDesc( STMFDParams( lang, false ), m_thread  ) );

   mc2dbg << __FUNCTION__ << endl;
   do {
      getTileMaps( query );
   } while ( query.reputFailed() && count++ < 100 );
   
   mc2dbg << __FUNCTION__ << endl;
   mc2dbg << "count = " << count << endl;

   // The maps should now be in the cache.

   // Count which params that are present in the cache.
   // TODO: Move to it's own method, or as output from getAllParams.
   bool containsBitmaps = false;
   paramsByLayerAndDetail_t paramsByLayerAndDetail;

   SharedBuffer tmpBuf( allParams.getBufferAddress(),
                        allParams.getBufferSize() );
   if ( tmpBuf.getBufferSize() == 0 ) {
      mc2dbg << warn << "[PTH]" << __FUNCTION__
             << ": No params to read." << endl;
      return;
   }

   for ( const char* it = tmpBuf.readNextString();
         ! tmpBuf.bufferFilled();
         it = tmpBuf.readNextString() ) {
      if ( isMap( it ) ) {
         
         TileMapParams tmpParam( it );
         // Update the map with this param.
         paramsByLayerAndDetail.update( tmpParam );
      } else if ( isBitmap( it ) ) {
         // Bitmap detected.
         containsBitmaps = true;
      }
   }
   
   // Create the file to indicate which languages that are present
   // in the tilemapcache.
   SharedBuffer infoBuf( 1024 );
   // Length of data.
   infoBuf.writeNextBALong( 0 ); // To be filled in.
   // Nbr languages.
   infoBuf.writeNextBALong( 1 ); // Only one language now.
   infoBuf.writeNextBALong( (uint32) lang );
  
   // Write the param info.
   // Nbr paramnotices.
   infoBuf.writeNextBALong( paramsByLayerAndDetail.size() );
   for ( map<layerAndDetail_t, ParamsNotice>::const_iterator it = 
            paramsByLayerAndDetail.begin(); 
         it != paramsByLayerAndDetail.end(); ++it ) {
      it->second.save( infoBuf );
   }
  
   // If the cache contains bitmaps. 
   infoBuf.writeNextBAByte( containsBitmaps );
   
   // Write the route id (if present).
   if ( routeID != NULL ) {
      // Nbr routes.
      infoBuf.writeNextBALong( 1 ); // One route present.
      infoBuf.writeNextString( routeID->toString().c_str() );
   } else {
      // Not a cache containing a route.
      infoBuf.writeNextBALong( 0 ); // No routes present.
   }
      
   uint32 infoBufSize = infoBuf.getCurrentOffset();
   // Write rest of the length of infofile. (i.e. total size - 4)
   infoBuf.reset();
   infoBuf.writeNextBALong( infoBufSize - 4 );
   
   MC2String infofileName = basepath;
   infofileName += "/a/cacheinfo";
   // Write the language file.
   int infofile = creat( infofileName.c_str(), 
                         S_IRUSR | S_IWUSR | 
                         S_IRGRP | S_IROTH  );

   write( infofile, infoBuf.getBufferAddress(), infoBufSize );
   close( infofile );

   // Tar the created cache directory and insert it into outBuf.
   MC2String tarCmd = "tar -C ";
   tarCmd += tmpDir.getPath();
   tarCmd += " -cf - ";
   tarCmd += cacheName;
   FILE* tarFile = popen( tarCmd.c_str(), "r" ); 

   if ( tarFile != NULL ) {
      static const int bufSize = 4096;
      byte buf[ bufSize ];

      while (! feof( tarFile ) && !ferror( tarFile ) ) {
         size_t size = fread( buf, 1, bufSize, tarFile );
         if ( !ferror( tarFile ) ) {
            outBuf.insert( outBuf.end(), buf, buf + size );
         }
      }
      pclose( tarFile );
   }


} catch ( const FileUtils::FileException& e ) {
   mc2dbg << error << "[PTH]: " << e.what() << endl;
}


void
ParserTileHandler::precacheClientTileMaps( bool singleFile,
                                           bool debug,
                                           vector<byte>& outBuf,
                                           const MC2BoundingBox& bbox,
                                           uint32 scale,
                                           LangTypes::language_t lang,
                                           const set<int>& layers,
                                           const char* cacheName,
                                           uint32 cacheMaxSize,
                                           bool useGzip )
{
   const ServerTileMapFormatDesc* desc = 
      m_group->getTileMapFormatDesc( STMFDParams( lang, false ), m_thread );

   set<MC2SimpleString> extraParams;
   extraParams.insert( "DYYY" );
   extraParams.insert( "dYYY" ); // for night mode

   DebugClock clock;
   
   mc2dbg << info << "[PTH]: Time for getAllParams" << clock << endl;

   if ( ! singleFile ) {
      SharedBuffer* allParams = desc->getAllParams( bbox,
                                                 layers,
                                                 extraParams,
                                                 useGzip,
                                                 lang,
                                                 scale );
      mc2dbg << "[PTH]: Tar file" << endl;
      precacheClientTileMaps( outBuf,
                              *allParams,
                              lang,
                              cacheName,
                              cacheMaxSize,
                              useGzip );

      delete allParams;

   } else {
      mc2dbg << "[PTH]: Single file " << cacheName << endl;
      precacheClientTileMapsSFD( outBuf,
                                 //*allParams,
                                 cacheName,
                                 debug,
                                 bbox,
                                 layers,
                                 extraParams,
                                 useGzip,
                                 lang,
                                 scale );

   }
   
}

void
ParserTileHandler::
correctCRCsAndPutInCache( const TileMapRequest* request )  {
   // only assert on the second round
   for ( uint32 i = 0; i < 2; ++i ) {
      if ( correctCRCsAndPutInCache( request, i != 0 ) ) {
         // end loop if crc was ok or successfully recovered
         break;
      }
   }
}

bool
ParserTileHandler::
correctCRCsAndPutInCache( const TileMapRequest* request, 
                          bool shouldAssert ) {
   if ( dynamic_cast<const ACPTileMapRequest*>( request ) ) {
      storeCache( request->getTileMapParams(),
                  request );
      return true;
   }
   vector<TileMapBufferHolder> buffers;
   request->getTileMapBuffers( buffers );

   // we cant do anything with empty buffer
   if ( buffers.empty() ) {
      mc2dbg
         << "[PTH]: correctCRCsAndPutInCache: no buffers to work on." << endl;
      return true;
   }

   TileMapParams param( buffers[ 0 ].getDesc() );

   // nothing to do if we dont have strings
   if ( param.getTileMapType() != TileMapTypes::tileMapStrings ) {
      // add to cache
      param.setImportanceNbr( 0 );
      // ACP mode, now lets restore map rights to the string,
      //  shall we ol' chap.
      if ( m_acpMode ) {
         storeCache( ::setACPMapRightFromOriginal( buffers[ 0 ].getDesc(),
                                                   param ).c_str(),
                     buffers );
      } else {
         storeCache( param, buffers );
      }

      return true;
   }

   //
   // We have the following text buffer:
   // [T1 T2 ... Tn ]
   //
   // We want to get the corresponding
   // Data buffer:
   // [G1 G2 ... Gn ]
   //
   // and compare each item in them with CRC
   //
   //
   // 1) Get the data map by using the importance 0
   //    value from an item in "buffers".
   //    (Each item in buffers differs only by importance)
   //   a) Try to get it from cache
   //   b) if cache fails send a TileMapRequest and
   //      store the respons in cache
   //   c) if both step a) and b) fails, return.
   // 
   // 2) Compare the CRCs
   //    a) if CRC differs send a TileMapRequest for
   //       the swedish string map
   //    b) still bad then;
   //       Create empty tilemap and send it to client
   //       but without saving it to cache.
   //
   // 3) store correct cache
   //



   //
   // Step 1:
   // Get data map
   //
   MC2String dataMapImp0Str;

   // setup data map imp0 str, special cases for ACP param
   {
      TileMapParams dataMapImp0( buffers[ 0 ].getDesc() );
      dataMapImp0.setTileMapType( TileMapTypes::tileMapData );
      // all data tile maps use swedish as language
      dataMapImp0.setLanguageType( LangTypes::swedish );
      dataMapImp0.setImportanceNbr( 0 );
      if ( m_acpMode ) {
         dataMapImp0Str = ::setACPMapRightFromOriginal( buffers[ 0 ].getDesc(),
                                                        dataMapImp0 );

      } else {
         dataMapImp0Str = dataMapImp0.getAsString().c_str();
      }
   }

   std::vector<TileMapBufferHolder> dataMapCache;
   // request from cache
   BitBuffer* dataMapBuffer( requestCache( dataMapImp0Str.c_str(), 
                                           dataMapCache ) );

   const ServerTileMapFormatDesc* desc =
      m_group->getTileMapFormatDesc( getDefaultSTMFDParams(), 
                                     m_thread );
   

   auto_ptr<TileMapRequest> req;

   if ( dataMapBuffer == NULL ) {
      mc2dbg << "[PTH]: Not found in cache: " << dataMapImp0Str << endl;
      //
      // did not find any data so we send a request
      // and save it to cache
      //
      auto_ptr<RouteReplyPacket> routeReply;
      if ( param.getRouteID() != NULL ) {
         routeReply.reset( m_thread->getStoredRoute( *param.getRouteID() ) );
      }


      int safety_counter = 0;
      do {
         req.reset( createTileMapRequest( *desc,
                                          dataMapImp0Str.c_str(),
                                          routeReply.get() ) );

         m_thread->putRequest( req.get() );
         mc2dbg << "[PTH]: correctCRCsAndPutInCache(): dataMapBuffers, "
                << " Put Request done. " << endl;
         req->getTileMapBuffers( dataMapCache );
      }  while ( dataMapCache.size() != buffers.size() &&
                 ++safety_counter < 100 );

      MC2_ASSERT( dataMapCache.size() == buffers.size() );

      // save to cache
      storeCache( dataMapImp0Str.c_str(), req.get() );
   }

   // at this point we must have same numbers of strings and data maps
   MC2_ASSERT( dataMapCache.size() == buffers.size() );

   bool dontSave = false; // whether the tilemap should be saved to cache

   //
   // Step 2:
   // Compare and try to fix CRC if needed.
   //
   for ( int i = 0, n = buffers.size(); i < n; ++i ) {
      // is crc good?
      if ( dataMapCache[ i ].getCRC() == buffers[ i ].getCRC() ) {
         buffers[ i ].setGood( true );
         continue;
      }

      buffers[ i ].setGood( false );

      mc2dbg << "[PTH]: ------- " << endl
             << "CRC differs for "
             << MC2CITE( buffers[ i ].getDesc() ) << "( "
             << MC2HEX( buffers[ i ].getCRC() ) << " ) != "
             << MC2CITE( dataMapCache[ i ].getDesc() ) << "( " 
             << MC2HEX( dataMapCache[ i ].getCRC() ) << " ) i = " << i << endl;

      MC2_ASSERT( isMap( buffers[i].getDesc() ) );
      MC2_ASSERT( TileMapParams( buffers[i].getDesc() ).getTileMapType() ==
                  TileMapTypes::tileMapStrings );

      // Get the string map in Swedish and store it as
      // original lang
      MC2String swedishParamStr;
      auto_ptr<RouteReplyPacket> routeReply;

      //
      // create routeReply and swedish param string
      //
      {
         // first create some test params, and test swedish params
         // then setup route id, and finaly set the new swedishParamStr

         TileMapParams swedishParams( buffers[ i ].getDesc() );
         swedishParams.setLanguageType( LangTypes::swedish );

         TileMapParams testParams( swedishParams );
         testParams.setTileMapType( TileMapTypes::tileMapData );     

         MC2_ASSERT( testParams.getAsString() == 
                     TileMapParams(dataMapCache[ i ].getDesc()).getAsString());

         // Get route packet if needed.
         TileMapParams params( swedishParams );
         
         if ( params.getRouteID() != NULL ) {
            routeReply.reset( m_thread->
                              getStoredRoute( *params.getRouteID() ) );
         }

         // set param string
         if ( m_acpMode ) {
            swedishParamStr = 
               ::setACPMapRightFromOriginal( buffers[ i ].getDesc(),
                                             swedishParams );
         } else {
            swedishParamStr = swedishParams.getAsString().c_str();
         }
      }

      // get swedish replacement maps, 
      // resend the request until we get a nonempty answer or
      // until resend count exceeds 100
      auto_ptr<TileMapRequest> req;
      vector<TileMapBufferHolder> swebuffers;
      int safetyCounter = 0;
      do {
         req.reset( createTileMapRequest( *desc,
                                          swedishParamStr.c_str(),
                                          routeReply.get() ) );
         m_thread->putRequest( req.get() );
         req->getTileMapBuffers( swebuffers );

         if ( swebuffers.empty() ) {
            mc2dbg << "[PTH]: Swedish replacement buffer empty!" 
                   << " retry count: " << safetyCounter << endl;
            continue;
         }
         
      } while ( swebuffers.empty() && ++safetyCounter < 100 );

      // still empty after all the retries...?
      if ( swebuffers.empty() ) {
         mc2dbg << warn << "[PTH]: Failed to get replacement map (timeout?)"
                << endl;
         continue;
      }

      TileMapBufferHolder& sweholder = swebuffers.front();

      MC2_ASSERT( MC2SimpleString( sweholder.getDesc() ) ==
                  swedishParamStr.c_str() );
      // Check CRC again
      if ( sweholder.getCRC() == dataMapCache[ i ].getCRC() ) {

         mc2dbg << "[PTH]: Swedish map CRC OK - replacing "
                << MC2CITE( buffers[i].getDesc() ) << " -> "
                << MC2CITE( sweholder.getDesc() ) << " in cache" << endl;
         buffers[ i ].copy( sweholder, true ); // copy, but keep description

      } else {

         mc2dbg << "[PTH]: Swedish map CRC not ok. " << endl;
         mc2dbg << "[PTH]: Erasing maps" << endl;
         mc2dbg << "[PTH]: CRCS really bad for "
                << MC2CITE( sweholder.getDesc() ) << "( "
                << MC2HEX( sweholder.getCRC() ) << " ) != "
                << MC2CITE( dataMapCache[ i ].getDesc() ) << "( " 
                << MC2HEX( dataMapCache[ i ].getCRC() )
                << " ) i = " << i << endl;

         // Erase entire cache
         {
            TileMapParams imp0( buffers[0].getDesc() );
            imp0.setImportanceNbr( 0 );
            if ( m_acpMode ) {
               eraseMapsFromCache(::setACPMapRightFromOriginal(buffers[ 0 ].
                                                               getDesc(),
                                                               imp0).c_str());
            } else {
               eraseMapsFromCache( imp0 );
            }
         }

         
         // should assert means this is the final round
         // and there will be no more tries to fetch a tile with 
         // correct CRC.
         //
         if ( shouldAssert ) {
            // Failed to fetch a tile with correct CRC;
            // Create an empty tile and send it, without writing
            // to cache.
            WritableTileMap emptyTileMap( buffers[ i ].getDesc(), 
                                          *desc,
                                          0, // unused
                                          0 ); // nbr polys.
      
            BitBuffer* holderBuffer = new BitBuffer( 1024 );
            emptyTileMap.save( *holderBuffer );
            holderBuffer->setSizeToOffset();
            holderBuffer->reset();
      
            TileMapBufferHolder tmp( buffers[ i ].getDesc(), 
                                     holderBuffer, 
                                     emptyTileMap.getCRC(), 
                                     true ); 

            // copy all data without changing the description
            buffers[ i ].copy( tmp, true );
         }
         // dont save to cache from now on
         dontSave = true;
      }
   }

   // if we should save to cache then return
   // true if it should not try again else 
   // false if it should give the tilemap another try
   if ( dontSave ) { 
      mc2dbg << "[PTH] Dont save cache." << endl;
      delete dataMapBuffer; // Don't forget to clean up before early exit
      return false; // failed to fetch tile with correct crc
   }

   //
   // Step 3:
   // Store the correct cache
   //

   // assign a unique key to the cache (just take on from the array )
   MC2String imp0Str;
   {
      TileMapParams imp0( buffers[0].getDesc() );
      imp0.setImportanceNbr( 0 );
      if ( m_acpMode ) {
         imp0Str = ::setACPMapRightFromOriginal( buffers[ 0 ].getDesc(),
                                                 imp0 );
      } else {
         imp0Str = imp0.getAsString().c_str();
      }

   }

   storeCache( imp0Str.c_str(), buffers );

   m_tileMapCache->releaseCached( cacheString( dataMapImp0Str.c_str() ),
                                  dataMapBuffer );
   return true;
}

void
ParserTileHandler::eraseMapsFromCache( TileMapParams params )
{
   int end = LangTypes::nbrLanguages;
   params.setTileMapType( TileMapTypes::tileMapStrings );
   params.setImportanceNbr( 0 );
   for ( int i = 0; i < end; ++i ) {
      LangTypes::language_t lang = LangTypes::language_t(i);
      params.setLanguageType( lang );
      // Add "N" for new cache
      m_tileMapCache->removeBuffer( cacheString(params.getAsString()) );
   }
   // Remove the data map too
   params.setLanguageType( LangTypes::swedish );
   params.setTileMapType( TileMapTypes::tileMapData );
   // Add "N" for new cache
   m_tileMapCache->removeBuffer( cacheString( params.getAsString() ) );
}

BitBuffer*
ParserTileHandler::getNonTileMap( const MC2SimpleString& paramStr )
{
   return getNonTileMap( paramStr.c_str() );
}

namespace { 

MC2String stripFileEnding( const MC2String& file ) {
   MC2String::size_type pos = file.find_last_of( "." );
   if ( pos == MC2String::npos ) {
      return file;
   }
   return file.substr( 0, pos );
}
}

BitBuffer*
ParserTileHandler::getBitMap( const char* paramStr )
{
   if ( paramStr == NULL || strlen( paramStr ) == 0 ) {
      return NULL;
   }

   MC2String paramString( paramStr );
   // "Settings"
   // The size factor compared to normal (20x20 icon, without transparency)
   int xFactor = 1;
   // Examples Btat_restaurant.png, b;tat_restaurant.png
   // Check for ';' in paramStr and use that for bitmap settings
   size_t semiColonPos = paramString.find( ';' );
   if ( semiColonPos != MC2String::npos ) {
      // Extract settings and put new paramString together without settings
      MC2String settings( paramString.c_str() + 1 );
      settings.erase( semiColonPos - 1 );
      mc2dbg4 << "Settings " << settings << endl;
      // Parse settings
      // When having settings the default for xFactor changes
      xFactor = 2;
      // For now there is no settings...
      
      // Remove settings from bitmap name
      paramString.erase( 1, semiColonPos );
      mc2dbg4 << " New paramString " << paramString << endl;
   }

   MC2String filename = m_group->
      getPOIImage( ::stripFileEnding( paramString.c_str() + 1 ), m_thread );
   if ( filename.empty() ) {
      // did not find the image in the table, lets continue 
      // to load it in the normal way
      return getRealBitMap( paramString.c_str(), xFactor );
   }

   // readd file ending
   filename += ".";
   filename += STLStringUtility::fileExtension( paramString );

   // prepend first 'b' or 'B' to the filename
   return getRealBitMap( ( MC2String( 1, paramString[ 0 ] ) + 
                           filename ).c_str(), xFactor );
}


void replaceFileExtension( MC2String& in, const MC2String& ext ) {
   MC2String::size_type findPos = in.rfind( '.' ); // Last .
   
   if ( findPos != MC2String::npos ) {
      in.erase( findPos + 1 );
      in.append( ext );
   }
}

BitBuffer* getFileBUffer( const MC2String& path, const MC2String& file ) {
   BitBuffer* dataBuf = NULL;

   FixedSizeString filePath( path.size() + 1 + file.size() + 1 );

   sprintf( filePath, "%s/%s", path.c_str(), file.c_str() );
   // File from path directory
   vector<byte> buff;
   if ( File::readFile( filePath.c_str(), buff ) > 0 ) {
      // Ok copy to a BitBuffer
      dataBuf = new BitBuffer( buff.size() );
      dataBuf->writeNextByteArray( &buff.front(), buff.size() );
      // Everything went fine.
      return dataBuf;
   }

   return dataBuf;
}

namespace {

/**
 * Parses sizes from a set of strings in the format widthxheight,
 * For example: 28x20, 40x32 etc.
 * @param sizes input sizes
 * @return sizes as width height pairs
 */
template < typename ImageSizes >
ImageSizes parseUserSizes( const vector< MC2String >& sizes ) {

   typedef typename ImageSizes::value_type ValueType;
   ImageSizes parsedSizes;

   for ( uint32 i = 0; i < sizes.size(); ++i ) {
      int32 width = -1;
      int32 height = -1;
      if ( sscanf( sizes[ i ].c_str(),
                   "%dx%d", &width, &height ) == 2 ) {
         parsedSizes.insert( ValueType( width, height ) );
      } else if ( ! sizes[ i ].empty() ) {
         mc2dbg << warn << "[PT] "
                << sizes[ i ]
                << " is not a valid size format." << endl;
      }
   }

   return parsedSizes;
}

} // anonymous

bool ParserTileHandler::isValidUserImageSize( const MC2String& str ) {

   // parse and validate user size from param
   int32 width = -1;
   int32 height = -1;
   if ( ! UserImage::getFilename( str, width, height ).empty() ) {

      // get valid sizes from config

      if ( m_validImageSizes.empty() ) {
         vector< MC2String > sizes = STLStringUtility::
            explode( ",",
                     Properties::
                     getProperty( "USER_IMAGE_SIZES", "40x40" ) );
         m_validImageSizes = ::parseUserSizes< ImageSizes >( sizes );
      }

      // validate user input size
      return STLUtility::has( m_validImageSizes,
                              ImageSize( width, height ) );
   }

   return false;
}

BitBuffer* 
ParserTileHandler::getRealBitMap( const char* paramsStr, int xFactor ) {
   mc2dbg4 << "getRealBitMap( " << paramsStr << ", " << xFactor<< " )" << endl;

   // Bitmap
   BitBuffer* dataBuf = NULL;
   const char* imagesPath = Properties::getProperty( "IMAGES_PATH" );
   if ( imagesPath == NULL ) {
      imagesPath = "./";
   }
   
   if ( paramsStr[ 0 ] == '\0' ) {
      // Empty string
      return NULL;
   }

   if ( strstr(  paramsStr + 1, ".." ) != NULL ) {
      // .. in file name
      return NULL;
   }

   const char* paramsStrToLoad = paramsStr;

   vector<MC2String> paramNames;
   //paramNames.push_back( paramsStr );
   // xFactor != 1 adds _dpi[xFactor] to file name
   bool magnify = false;
   MC2String xFactorStr( paramsStrToLoad );
   if ( xFactor != 1 ) {
      magnify = true;
      xFactorStr = paramsStrToLoad;
      // Find the last '.' before the suffix 
      // and squeeze in "_dpi[xFactor]" before.
      char dpiStr[ 20 ];
      sprintf( dpiStr, "_dpi%u", xFactor );
      MC2String::size_type pos = xFactorStr.rfind( "." );
      if ( pos != MC2String::npos ) {
         xFactorStr.insert( pos, dpiStr );
      }
      paramsStrToLoad = xFactorStr.c_str();
   }

   bool oldBitmap = TileMapParamTypes::isOldBitmap( paramsStrToLoad );
   MC2String oldBitmapStr;
   if ( oldBitmap ) {
      // Old bitmap.
      oldBitmapStr = paramsStrToLoad;

      // Find the last '.' before the suffix 
      // and squeeze in "_old" before.
      MC2String::size_type pos = oldBitmapStr.rfind( "." );
      if ( pos != MC2String::npos ) {
         oldBitmapStr.insert( pos, "_old" );
      }
      paramsStrToLoad = oldBitmapStr.c_str();
   }

   paramNames.push_back( paramsStrToLoad );

   MC2String fileExtForOriginal = STLStringUtility::fileExtension( 
      paramsStrToLoad, true );
   bool isVectorFormat = false;
   if ( fileExtForOriginal == "mif" || fileExtForOriginal == "svg" ) {
      // Can not convert vectorised map image formats, but something is done
      isVectorFormat = true;
   }

   if ( !isVectorFormat && fileExtForOriginal != "png" ) {
      MC2String pngImage( paramsStrToLoad );
      size_t findPos = pngImage.rfind( '.' );
      if ( findPos != MC2String::npos ) {
         pngImage.erase( findPos );
         pngImage.append( ".png" );
      }
      paramNames.push_back( pngImage );
   }


   for ( uint32 i = 0; i < paramNames.size(); ++i ) {
      paramsStrToLoad = paramNames[ paramNames.size() - 1 - i ].c_str();
      MC2String fileExt( STLStringUtility::fileExtension( 
                            paramsStrToLoad, true ) );
      bool otherFormat = false;
      if ( i + 1 != paramNames.size() ) {
         otherFormat = true;
      }

      // If this is request is custom image size request, then
      // generate image from svg
      if ( TileMapParamTypes::isCustomImageSize( paramsStrToLoad ) ) {
         bool wantCropped = TileMapParamTypes::
            isCustomImageSizeCropped( paramsStrToLoad );
         // validate input
         if ( ! isValidUserImageSize( &paramsStrToLoad[ 1 ] ) ) {
            return NULL;
         }

         return UserImage::
            createCustomImageSize( imagesPath,
                                   &paramsStrToLoad[ 1 ],
                                   wantCropped ).release();
      }

      int conversionTried = 0;
      while ( conversionTried < 2 ) {
         ++conversionTried;
                                      
         FixedSizeString filePath( strlen( imagesPath ) + 1 + 
                                   strlen( paramsStrToLoad + 1 ) + 1 );

         sprintf( filePath, "%s/%s", imagesPath, paramsStrToLoad + 1 );

         // Image from Images directory
         vector<byte> buff;
         if ( !otherFormat && File::readFile( filePath.c_str(), buff ) > 0 ) {
            // Ok copy to a BitBuffer
            delete dataBuf; // While loop makes this necessary.
            dataBuf = new BitBuffer( buff.size() );
            dataBuf->writeNextByteArray( &buff.front(), buff.size() );
            // Everything went fine.
            return dataBuf;
         } else {

            bool converted = false;
            if ( !isVectorFormat && conversionTried == 1 && 
                 fileExt != "png" ) {
               // Try to convert the bitmap from png.
               mc2dbg << "[PTH]: Trying to convert file from png"
                      << endl;
               converted = ImageTools::convert( filePath.c_str(), "png" );
            } else {
               mc2log
                  << warn << "ParserTileHandler::requestTileMap Failed to "
                  << "open BitMap: " << MC2CITE( filePath.c_str() ) << " from "
                  << MC2CITE( paramsStrToLoad + 1 ) << endl;
            }

            if ( !isVectorFormat && !converted && magnify && 
                 conversionTried == 1 ) {
               MC2String xFactorImage( imagesPath );
               xFactorImage.append( "/" );
               MC2String inputImage( xFactorImage ); // Starts with path
               xFactorImage.append( xFactorStr.c_str() + 1 );
               inputImage.append( paramsStr + 1 );
               if ( otherFormat ) {
                  replaceFileExtension( inputImage, fileExt );
                  replaceFileExtension( xFactorImage, fileExt );
               }
               mc2dbg4 << "Making magnified image: " << inputImage << " to "
                       << xFactorImage << " using magnification " 
                       << xFactor << endl;
               if ( !File::fileExist( xFactorImage ) ) {
                  // Make it
                  if ( !UserImage::magnify( inputImage, xFactorImage, 
                                            xFactor, oldBitmap ) ) {
                     mc2log << warn << "[PTH]::getRealBitMap "
                            << "failed to magnify " << inputImage << " to "
                            << xFactorImage << " using magnification " 
                            << xFactor << endl;
                  } else {
                     mc2dbg4 << "Made " << xFactorImage << endl;
                  }
               } else {
                  mc2dbg4 << "Have " << xFactorImage << endl;
               }
            }

            if ( !converted && oldBitmap && conversionTried == 1 ) {
               // ok, we have old bitmap and read failed on the first run,
               // so we are going to try to convert the original file to
               // old file by removing transparent on each side
               // of the original image.
               MC2String path( imagesPath );
               path += "/";
               // + 1 to skip the first character
               MC2String realOrigFile = xFactorStr.c_str() + 1;
               MC2String realDestFile = paramsStrToLoad + 1;
               if ( otherFormat ) {
                  replaceFileExtension( realOrigFile, fileExt );
                  replaceFileExtension( realDestFile, fileExt );
               }
               mc2dbg4 << "Crop " << path + realOrigFile << " to " 
                       << path + realDestFile << endl;
               if ( GDUtils::cropTransparent( path + realOrigFile, 
                                              path + realDestFile ) ) {
                  // crop was successfull now lets try with _old.png again.
                  conversionTried = 1; 
               }
            }

         }

         if ( otherFormat ) {
            // No need to try if not the right format
            break;
         }
      }
   }
   
   // Create the png from the svg
   if ( dataBuf == NULL && fileExtForOriginal == "png" ) {
      MC2String svgFilePath( paramsStr );
      svgFilePath = svgFilePath.substr( 1 );
      //MC2String pngFilePath( MC2String( imagesPath ) + "/" + svgFilePath);
      STLStringUtility::replaceString( svgFilePath, "png", "svg" );
      svgFilePath = MC2String( imagesPath ) + "/" + svgFilePath;
      
      int size = xFactor * 20;
      auto_ptr< DataBuffer > buf = 
         ImageTools::createPNGFromSVG( svgFilePath, size, size );
      if (buf.get() != NULL ) {
         dataBuf = DataBufferUtil::convertToBitBuffer( *buf ).release();
         
         //mc2log << "[PTH]: Cacheing: " << paramNames << endl;
         //File::writeFile( paramNames, dataBuf->getBufferAddress(), 
         //                 dataBuf->getBufferSize() );
      }
      
   }

   if ( dataBuf == NULL ) {
      // Conversion failed try the original image name
      dataBuf = getFileBUffer( imagesPath, paramsStr + 1 );
      if ( dataBuf != NULL ) {
         mc2log << info << "[PTH]: using the non converted image: "
                << (paramsStr + 1) << " for " << paramsStr << endl;
      } else {
         mc2log << warn << "[PTH]: " << (paramsStr+1) << " does not exist"
                << endl;
      }
   }   

   if ( dataBuf == NULL ) {
      // The file could not be found, send cityCenter to avoid 301
      const char* defaultImage = "cityCentre_point_small";
      MC2String param = "b";
      param += defaultImage + MC2String(".");
      param += STLStringUtility::fileExtension(paramsStr, true);
      if ( param == paramsStr ) {
         // Do not loop forever.
         mc2log << error << "[PTH]: " << defaultImage << " does not exist"
                << endl;
         return NULL;
      }
      // Could not load the file.
      // Send cityCentre
      mc2log << warn << "[PTH]: Missing bitmap "
             << MC2CITE( paramsStr + 1 ) << " - will send "
             << MC2CITE( defaultImage ) << endl;
      return getRealBitMap( param.c_str(), xFactor );
   }
   
   return dataBuf;
}


BitBuffer*
ParserTileHandler::getNonTileMap( const char* paramsStr )
{
   // 1. Try the cache
   {
      BitBuffer* tmpBuf = m_tileMapCache->requestCached( paramsStr );
      if ( tmpBuf ) {
         return tmpBuf;
      }
   }
   // 2. Create the requested non-map
   TileMapParamTypes::param_t paramType =
      TileMapParamTypes::getParamType( paramsStr );
   
   switch ( paramType ) {
      case TileMapParamTypes::TILE:
         MC2_ASSERT( false );
         break;
      case TileMapParamTypes::BITMAP:
         return getBitMap( paramsStr );
         break;
      case TileMapParamTypes::FORMAT_DESC_CRC:
      case TileMapParamTypes::FORMAT_DESC: {
         // Description format.
         STMFDParams param = getDefaultSTMFDParams( paramsStr );
         if ( paramType == TileMapParamTypes::FORMAT_DESC ) {
            // Get the format desc
            return m_group->newTileMapFormatDescBuffer( param, m_thread );
         } else {
            // Get the CRC buf.
            return m_group->newTileMapFormatDescCRCBuffer( param, m_thread );
         }
         break;
      }
      case TileMapParamTypes::UNKNOWN:
         mc2log << error << "[PTH]:getNonTileMap paramType is "
                << "TileMapParamTypes::UNKNOWN!" << endl;
         break;
      case TileMapParamTypes::BUFFER_HOLDER:
         mc2log << error << "[PTH]:getNonTileMap paramType is "
                << "TileMapParamTypes::BUFFER_HOLDER!" << endl;
         break;
   }
   return NULL;
}

DataBuffer*
ParserTileHandler::getTileMaps(const vector<MC2SimpleString>& params,
                          uint32 startOffset,
                          uint32 maxBytes)
{
   TileMapBufferQuery query( params, maxBytes );
   getTileMaps( query );

   const SharedBuffer* buf = query.getMultiBuffer();
#if 0
   // Test loading all maps in outgoing buffer.
   if ( buf != NULL ) {
      SharedBuffer rb( *buf );
      rb.reset();
      while ( rb.getCurrentOffset() < rb.getBufferSize() ) {
         MC2SimpleString desc = rb.readNextString();
         int buflen = rb.readNextBALong();
         if ( buflen != 0 ) {
            BitBuffer* mapBuf =
               new BitBuffer(
                  const_cast<uint8*>(rb.readNextByteArray( buflen )),
                  buflen );
            if ( isMap( desc.c_str() ) ) {
               mc2dbg << "[PTH]: Loading map" << desc << endl;
               TileMap tmap;
               tmap.load( *mapBuf,
                          *m_group->getTileMapFormatDesc( LangTypes::english ),
                          desc );
            }
            delete mapBuf;
         }
      }
   }
#endif
   
   if ( buf->getBufferSize() == 0 ) {
      return NULL;
   } else {
      return new DataBuffer( *buf );
   }
}

MC2SimpleString
ParserTileHandler::checkServerPrefixAndReplace( const MC2SimpleString& param )
{
   // Send DXXX if incompatible serverstring.
   // This probably happens very seldomly.
   if ( isMap( param.c_str() ) ) {
      // Check for correct version of the parameters.
      TileMapParams tmpParams( param.c_str() );
      bool nightMode = TileMapFormatDesc::isNightMode( param );
      // Create desc
      const ServerTileMapFormatDesc* desc =
         m_group->
         getTileMapFormatDesc( STMFDParams( tmpParams.getLanguageType(),
                                            nightMode,
                                            STMFDParams::DEFAULT_LAYERS,
                                            0,
                                            ImageTable::DEFAULT,
                                            tmpParams.getServerPrefix() ),
                               m_thread );
      // Wrong version -> return the format desc.
      if ( tmpParams.getServerPrefix() !=
           desc->getServerPrefix() ) {
         mc2log << warn << "[PTH]: Got request for wrong version of map "
                << tmpParams.getServerPrefix() << " should be "
                << desc->getServerPrefix() << endl;
         
         return desc->createParamString( tmpParams.getLanguageType(),
                                         "mc2server");
      }
   }

   return param;
}

void
ParserTileHandler::
assertCachedCRC( const char* inParam, 
                 const std::vector<TileMapBufferHolder>& crcBuffers,
                 const ServerTileMapFormatDesc* desc ) {

   if ( ! isMap( inParam ) ) {
      return;
   }
   MC2String cacheParamStr;

   // setup param string
   {

      TileMapParams tmpParam( inParam );
      if ( tmpParam.getTileMapType() == TileMapTypes::tileMapData ) {
         return;
      }

      tmpParam.setTileMapType( TileMapTypes::tileMapData );
      tmpParam.setLanguageType( LangTypes::swedish ); 
      tmpParam.setImportanceNbr( 0 );

      // Check for the corresponding data map which should be there.
      mc2dbg8 << "[PTH::asserCachedCRC]: requesting cache " << endl;

      // In ACP mode we must convert the cache request string to
      // the correct ACP string from the original
      if ( m_acpMode ) {
         cacheParamStr = ::setACPMapRightFromOriginal( inParam, tmpParam );
      } else {
         // no acp mode, then lets get the normal string
         cacheParamStr = tmpParam.getAsString().c_str();
      }
   }

   vector<TileMapBufferHolder> dataHolders;
   BitBuffer* tmpDataBuf = requestCache( cacheParamStr.c_str(), dataHolders );


 
   // There should not be any string tilemaps in the cache
   // without the data tilemap.

   if ( tmpDataBuf == NULL ) {
      // Clean up before we crash
      mc2log << fatal << "[PTH]: No data map "
             << MC2CITE( cacheParamStr ) << " for string map "
             << MC2CITE( inParam ) << endl;
      mc2log << fatal << "[PTH]: Cleaning cache before crash" << endl;
      eraseMapsFromCache( inParam );
      mc2log << error << "Assert was here!" << endl;
      return;
   }

   if ( crcBuffers.size() != dataHolders.size() ) {
      mc2log << fatal << "[PTH]: in assertCachedCRC" << endl;
      eraseMapsFromCache( inParam );
      MC2_ASSERT( crcBuffers.size() == dataHolders.size() );
   }

   // Now check that the crcs are matching.
   for ( uint32 i = 0; i < dataHolders.size(); ++i ) {
      if ( dataHolders[ i ].getCRC() != crcBuffers[ i ].getCRC() ) {
         mc2log << fatal << "[PTH]: crc bad!" << endl;
         mc2log << fatal << "Tile: " 
                << dataHolders[ i ].getDesc()
                << " (" << MC2HEX( dataHolders[ i ].getCRC() ) << ") != " 
                << crcBuffers[ i ].getDesc() 
                << " (" << MC2HEX( crcBuffers[ i ].getCRC() ) << ")"
                << " i = " << i << endl;
         eraseMapsFromCache( inParam );
         MC2_ASSERT( dataHolders[ i ].getCRC() ==
                     crcBuffers[ i ].getCRC() );
      }

   }

   m_tileMapCache->releaseCached( cacheString( cacheParamStr.c_str() ) , 
                                  tmpDataBuf );

}

/**
 * Releases cache buffer when it goes out of scope.
 * Similar to auto_ptr.
 */
class CachePtr {
public:
   CachePtr():
      m_cache( NULL ),
      m_buffer( NULL ),
      m_name() {
   }

   CachePtr( ParserThreadFileDBufRequester* cache,
             BitBuffer* buffer,
             const MC2SimpleString& name ):
      m_cache( cache ),
      m_buffer( buffer ),
      m_name( name ) {
   }

   ~CachePtr() {
      destroy();
   }

   void reset( ParserThreadFileDBufRequester* cache,
               BitBuffer* buffer,
               const MC2SimpleString& name ) {
      destroy();
      m_cache = cache;
      m_buffer = buffer;
      m_name = name;
   }

private:
   void destroy() {
      if ( m_cache ) {
         m_cache->release( m_name, m_buffer );
      }
   }

   ParserThreadFileDBufRequester* m_cache;
   BitBuffer* m_buffer;
   MC2SimpleString m_name;
};

int
ParserTileHandler::requestTiles( vector<RequestWithStatus*>& reqs,
                                 vector<TileMapRequest*>& tilereqs,
                                 vector<RouteReplyPacket*>& routePackets,
                                 TileMapQuery& query ) 
{
   int nbrAdded = 0;

   if ( ! reqs.empty() ) {
      m_thread->putRequests( reqs );
   }
      
   vector<TileMapRequest*>::const_iterator rt = tilereqs.begin();
   vector<TileMapRequest*>::const_iterator rt_end = tilereqs.end();
   for ( ; rt != rt_end; ++rt ) {

      if ( (*rt)->getStatus() != StringTable::OK ) {
         mc2dbg << "[PTH]: Status of TMR is "
                << MC2CITE( StringTable::getString( (*rt)->getStatus(),
                                                    StringTable::ENGLISH) )
                << endl;
       
         mc2dbg << "StringTable status not ok!" << endl;

         continue;
      }

      CachePtr m_cachePtr;
      vector<TileMapBufferHolder> buffers;

      // Check CRC and add to cache
      if ( m_tileMapCache ) {
         // If crc differs for gfx and string tile crcOKForTile
         // will try to request the tile in Swedish and put it
         // into the cache. If it is still incorrect, it will
         // erase all the maps in all languages including
         // the gfx map.
         correctCRCsAndPutInCache( *rt );
         // Now request it from cache, since it could've been
         // recovered.
         // TODO: This is a place for some improvements.
         //       If it wasn't recovered, then it could've
         //       been grabbed from the request directly.
         TileMapParams imp0( (*rt)->getTileMapParams() );
         imp0.setImportanceNbr( 0 );
         m_cachePtr.reset( m_tileMapCache,
                           requestCache( imp0, buffers ),
                           cacheString( imp0.getAsString().c_str() ) );
      }

      // if fetch from cache failed or we didn't have
      // any cache, get buffers from request
      if ( buffers.empty() ) {
         (*rt)->getTileMapBuffers( buffers );
      }

      if ( buffers.empty() ) {
         mc2log << "[PTH]: No buffers in received request" << endl;
      } else {
         mc2dbg8 << "[PTH] Adding from request: " << buffers[ 0 ].getDesc()
                << endl;
         mc2dbg8 << "[PTH] and the size is  " << buffers.size() << endl;
      }
      // Put the buffers that are not marked as non-good into the
      // query.
      nbrAdded += query.addBuffers( buffers );
   }

   reqs.clear();

   STLUtility::deleteValues( tilereqs );
   STLUtility::deleteValues( routePackets );

   routePackets.clear();

   return nbrAdded;
}
namespace {
inline bool isFormatDescOrCRC( const TileMapParamTypes::param_t type ) {
   return 
      type == TileMapParamTypes::FORMAT_DESC ||
      type == TileMapParamTypes::FORMAT_DESC_CRC;
}

}

STMFDParams ParserTileHandler::getDefaultSTMFDParams( const char* paramString ) const {

   uint32 layers = STMFDParams::DEFAULT_LAYERS;
   uint32 drawSettingVersion = 0;
   const TileMapParamTypes::param_t paramType = 
      TileMapParamTypes::getParamType( paramString ? paramString : "" );

   bool isVicinityVersion1 = false;
   bool useEventLayer = false;
   // the client check for draw setting is only needed for 
   // tile map format descriptor tile param. 
   if ( ::isFormatDescOrCRC( paramType ) ) {
      isVicinityVersion1 = paramString ?
         strstr( paramString, "jv\\1\\vicinity" ) != 0 : false;
      useEventLayer = paramString ?
         strstr( paramString, "jv\\1\\eventfinder" ) != 0 : false;
   }

   // vicinity version 1 client, return layer poi and acp only
   if ( isVicinityVersion1 ) {
      layers = 0;
      BitUtility::setBit( layers, TileMapTypes::c_poiLayer, true );
   }
   
   const UserItem* user = m_thread->getCurrentUser();
   const ClientSetting* clientSetting = NULL;
   // if we have a valid user, then setup poi and acp layer
   if ( user != NULL ) {
      clientSetting = m_thread->getClientSetting();
      MapRights rights = user->getUser()->
         getAllMapRights( TimeUtility::getRealTime() );

      // have any of the required acp layer rights?
      if ( rights & POISetProperties::getACPLayerRights() ) {
         layers |= ( 1 << TileMapTypes::c_acpLayer );
      } else if ( layers & ( 1 << TileMapTypes::c_acpLayer ) ) {
         layers ^= ( 1 << TileMapTypes::c_acpLayer );
      }

      // disable poi layer if client has this special layer
      if ( rights & MapRights( MapRights::DISABLE_POI_LAYER ) &&
           layers & ( 1 << TileMapTypes::c_poiLayer ) ) {
         layers ^= ( 1 << TileMapTypes::c_poiLayer );
      }

      if ( clientSetting != NULL ) {
         drawSettingVersion = clientSetting->getDrawVersion();
         // Check for special clients that has a hardcoded set of layers
         uint32 tmpLayers = 0;
         if ( clientSetting->getSpecificTMapLayers( tmpLayers ) ) {
            layers = tmpLayers;
         }
      }
   }
   // set draw version 2 for active X component
   if ( ::isFormatDescOrCRC( paramType ) &&
        drawSettingVersion == 0 &&
        strstr( paramString, "WinClientConn" ) != 0 ) {
      drawSettingVersion = 2;
   }


   if ( useEventLayer ) {
      layers = 0;
      BitUtility::setBit( layers, TileMapTypes::c_eventLayer, true );
      // special case for MapClient testing
      if ( strstr( paramString, "gtk-jv\\1\\eventfinder" ) != 0 ) {
         BitUtility::setBit( layers, TileMapTypes::c_mapLayer, true );
      }
   }

   bool nightMode = paramString ? TileMapFormatDesc::
      isNightMode( paramString ) : false;

   LangTypes::language_t lang = paramString ? 
      TileMapFormatDesc::getLanguageFromParamString( paramString ) :
      LangTypes::english;

   ImageTable::ImageSet imageSet = ImageTable::DEFAULT;
   if ( clientSetting != NULL ) {
      imageSet = clientSetting->getImageSet();
   }

   uint32 serverPrefix = STMFDParams::DEFAULT_SERVER_PREFIX;

   if ( clientSetting != NULL ) {
      serverPrefix = clientSetting->getServerPrefix();
   }

   return STMFDParams( lang, nightMode, layers, drawSettingVersion, 
                       imageSet, serverPrefix );
}

void
ParserTileHandler::getTileMaps( TileMapQuery& query ) {
   MC2_ASSERT( m_tileMapCache != NULL );

   const ServerTileMapFormatDesc* desc =
      m_group->
      getTileMapFormatDesc( getDefaultSTMFDParams(),
                            m_thread );
   
   int nbrAdded = 0;
   // When the time limit has been exceeded, only cached maps
   // will be considered.
   uint32 startTime = TimeUtility::getCurrentTime();

   int nbrCached = 0;
   int nbrNonMaps = 0;

   mc2dbg8 << "[PTH]: Part 1 and 2 - Cache and non-cached tiles" << endl;

   // PART 1 and 2 - Send the requests.

   // Put back the ones that has not been in cache.
   // Not needed since we don't tell the query about the cached maps.
   // query.reputFailed(); 
   
   // Now it is time to send the requests.

   TileMapQuery::paramVect_t params;   
   int nbrParams = 0;
   while ( query.getNextParams( params, 10 ) ) {

      vector<TileMapRequest*> tilereqs;
      vector<RequestWithStatus*> reqs;
      vector<RouteReplyPacket*> routePackets;

      nbrParams += params.size();

      int nbrAddedThisTime = 0;
      TileMapQuery::paramVect_t::const_iterator pit = params.begin();
      TileMapQuery::paramVect_t::const_iterator pit_end = params.end();
      for (; pit != pit_end; ++pit ) {
         // Check if valid input
         mc2dbg8 << "PTH param " << *pit << endl;

         if ( isMap( *pit ) ) {
            TileMapParams tileParams( *pit );
            // If tile map not valid or the tmfd dont think its valid
            // or if we are not in acp mode and the tilemap string don't 
            // match the input string ( which would be strange... )
            if ( !tileParams.getValid() || !desc->valid( tileParams ) || 
                 ( !m_acpMode && (*pit) != tileParams.getAsString() ) ) {
               mc2dbg <<
                  "------------------------------------------------------" 
                      << endl;
               mc2log << warn << "[PTH]: Param " << *pit << " is not valid."
                      << endl;
               mc2dbg << "tileparam valid? " << tileParams.getValid() 
                      << "; according to desc valid? " 
                      << desc->valid(tileParams )
                      << "; " << (*pit) << " =? " << tileParams.getAsString()
                      << endl;
               mc2dbg << tileParams << endl;

               ++nbrNonMaps;
               auto_ptr<TileMapBufferHolder>
                  ocean( TileMapCreator::createOceanTileMap( *pit, *desc ) );

               nbrAddedThisTime = query.addOneBuffer( *pit, 
                                                      ocean->getBuffer() );
               continue;
            }
         }

         // Replaces the param with a DXXX if the prefix is wrong.         
         MC2SimpleString param = checkServerPrefixAndReplace( *pit );
         
         if ( param != *pit ) {
            // Should have returned in the first loop.
            mc2dbg << "[PTH]: " << param
                   << " != " << *pit << " - stopping " << endl;
            break;
         }
         

         if ( isMap( param ) ) {
            MC2String cacheParamStr;

            // setup cache string
            {
               TileMapParams cacheParams( param );
               // skip assert if in acp mode with acp param
               if ( ACPTileMapRequest::isACPParam( param ) && 
                    ! m_acpMode ) {
                  MC2_ASSERT( param == cacheParams.getAsString() );
               }

            
               cacheParams.setImportanceNbr( 0 );
               if ( m_acpMode ) {
                  cacheParamStr = 
                     ::setACPMapRightFromOriginal( param, cacheParams );
               } else {
                  cacheParamStr = cacheParams.getAsString().c_str();
               }

            }
            
            vector<TileMapBufferHolder> cache;
            BitBuffer* cacheBuf = requestCache( cacheParamStr.c_str(), cache );
            if ( cacheBuf != NULL  && ! cache.empty() ) {

               // TODO: MC2_ASSERT that the crc:s are correct in the
               //       tilemap cache. This can be removed once we know that
               //       the crc:s are ok.
               assertCachedCRC( param.c_str(), cache, desc );

               nbrAddedThisTime = query.addBuffers( cache );
               nbrCached += nbrAddedThisTime;
               // append "N" for new cache
               m_tileMapCache->releaseCached( cacheString( cacheParamStr.c_str() ),  
                                              cacheBuf );
               continue;
            }
         } else {
            // Note:
            // No need to requestTiles for non-maps, they
            // are generated by the SFD Query one by one 
            //

            // Then take the non-maps
            ++nbrNonMaps;
            BitBuffer* buf = getNonTileMap( param );
            nbrAddedThisTime = query.addOneBuffer( param, buf );
            nbrAdded += nbrAddedThisTime;
            m_tileMapCache->release( param, buf );
            continue;
         }

         TileMapParams tp( param );

         const RouteID* routeID = tp.getRouteID();
         RouteReplyPacket* routePack = NULL;
         
         if ( routeID ) {
            routePack = m_thread->getStoredRoute( *routeID );
            routePackets.push_back( routePack );
         }

         tp.setImportanceNbr( 0 );
         tilereqs.push_back( createTileMapRequest( *desc,
                                                   tp.getAsString(),
                                                   routePack ) );

         reqs.push_back( tilereqs.back() );
      } // End for all params
      
      nbrAddedThisTime += requestTiles( reqs, tilereqs, routePackets, query );
      nbrAdded += nbrAddedThisTime;

      params.clear();

      // The client may have moved since it took so long.
      // If the time limit is exceeded, only cached maps will be
      // sent.
      const uint32 timeLimit = query.getTimeLimit();
      if ( ( TimeUtility::getCurrentTime() - startTime ) > timeLimit ) {  
         mc2dbg << warn << "[PTH]: Time limit "
                << timeLimit 
                << " ms exceeded when creating "
                << "TileMaps - not adding any more"
                << endl;
         // Add no more maps.
         break;
      }
   } // End while getNextParams

   
   mc2dbg << "[PTH]: " << query.getNbrAdded()
          << " maps of " << query.getNbrWanted()          
          << " sent" << ", "
          << nbrAdded << " added, "
          << nbrCached << " cached, "
          << nbrNonMaps << " non-maps, "
          << nbrParams << " nbrParams, "
          << query.getNbrFailed() << " failed."          
          << endl;
}

static inline void getScaleFromSpeedKph( uint32 speedKph, 
                                         uint32& minScale,
                                         uint32& maxScale )
{
#if 0
   // This is taken from series 60.
   if( speedKph < 30 ){
      minScale = 2;
   } else if( speedKph < 55 ){
      minScale = 4;
   } else if( speedKph < 75 ){
      minScale = 10;
   } else if( speedKph < 95 ){
      minScale = 20;
   } else {
      minScale = 40;
   }

   maxScale = minScale + 20;
   maxScale = minScale+1;
#else
   minScale = 1;
   maxScale = 24000;

#endif
}

void
ParserTileHandler::getRouteTileMapParams( const ExpandedRoute* expRoute,
                                     const RouteID& routeID,
                                     LangTypes::language_t lang,
                                     const set<int>& layerIDs,
                                     set<MC2SimpleString>& routeParams )
{
   DebugClock functionClock;
   // Create desc
   const ServerTileMapFormatDesc* desc = m_group->
      getTileMapFormatDesc( STMFDParams( lang, false ),
                            m_thread );

   // Get all route items (turn descriptions)
   for ( uint32 i = 0; i < expRoute->getNbrExpandedRouteItems(); ++i ) {
      const ExpandedRouteItem* routeItem = 
         expRoute->getExpandedRouteItem( i );
      MC2_ASSERT( routeItem != NULL );
      
      // Get all roads for the turn description.
      for ( uint32 j = 0; j < routeItem->getNbrExpandedRouteRoadItems();
            ++j ) {
         const ExpandedRouteRoadItem* routeRoadItem = 
            routeItem->getExpandedRouteRoadItem( j );
         MC2_ASSERT( routeRoadItem != NULL );
         
         uint32 scale = 0;
         uint32 maxScale = 0;

         // The speed limit. 
         uint32 speedLimit = routeRoadItem->getPosSpeedLimit();
         // Get the suitable scale range for this speed.
         getScaleFromSpeedKph( speedLimit, scale, maxScale );
   
         desc->getAllParams( routeParams, 
                             routeRoadItem->coordsBegin(),
                             routeRoadItem->coordsEnd(),
                             250, // Extra pixels, FIXME: Don't hardcode
                             layerIDs,
                             true, // Use gzip
                             lang,
                             scale,
                             maxScale,
                             &routeID );
      }
   }

   // Also add DYYY.
   routeParams.insert( "DYYY" );

   // Add the poi icons in case they poi layer is present.
   if ( layerIDs.find( TileMapTypes::c_poiLayer ) != layerIDs.end() ) {
      for ( int i = ImageTable::DEFAULT; i < ImageTable::NBR_IMAGE_SETS; ++i ) {
         // Most clients use png.
         ServerTileMapFormatDesc::getBitmapNames( 
            routeParams, ".png", static_cast<ImageTable::ImageSet>(i) );
         // but S80 uses gif.
         ServerTileMapFormatDesc::getBitmapNames( 
            routeParams, ".gif", static_cast<ImageTable::ImageSet>(i) );
#if 0
         // And gtk client uses xpm.
         ServerTileMapFormatDesc::getBitmapNames( 
            routeParams, ".xpm", static_cast<ImageTable::ImageSet>(i) );
#endif      
      }
   }
     
   mc2dbg << "[PTH]: Took " << functionClock
          << " to create " << routeParams.size() 
          << " route tilemap params."
          << endl;
}

void
ParserTileHandler::getRouteTileMapCache( vector<byte>& outBuf,
                                    const ExpandedRoute* expRoute,
                                    const RouteID& routeID,
                                    LangTypes::language_t lang,
                                    const set<int>& layerIDs,
                                    const char* cacheName,
                                    uint32 cacheMaxSize,
                                    bool useGzip )
{
   DebugClock functionClock;
   // Create the params.
   set<MC2SimpleString> routeParams;
   getRouteTileMapParams( expRoute,
                          routeID,
                          lang,
                          layerIDs,
                          routeParams );

   // FIXME: Move this to getRouteTileMapParams
   vector<char> buf;
   for ( set<MC2SimpleString>::const_iterator it = routeParams.begin();
         it != routeParams.end();
         ++it ) {
      buf.insert( buf.end(), it->c_str(), it->c_str() + it->length() + 1 );
   }

   SharedBuffer paramBuf( reinterpret_cast<uint8*>( &buf.front() ),
                          buf.size() );
   
   // Create the cache.   
   precacheClientTileMaps( outBuf,
                           paramBuf,
                           lang,
                           cacheName,
                           cacheMaxSize,
                           useGzip,
                           &routeID );
   mc2dbg << "[PTH]: Took " << functionClock
          << " to create a cache containing " 
          << routeParams.size() << " route tilemaps"
          << endl;
}

void
ParserTileHandler::getRouteTileMapCache( vector<byte>& outBuf,
                                    const RouteID& routeID,
                                    LangTypes::language_t lang,
                                    const set<int>& layerIDs,
                                    const char* cacheName,
                                    uint32 cacheMaxSize,
                                    bool useGzip )
{

   StringTable::languageCode langCode = 
      ItemTypes::getLanguageTypeAsLanguageCode( lang );

   // Get Route
   ExpandRouteRequest* expReq = NULL;
   PacketContainer* expandRouteCont = NULL;
   RouteReplyPacket* routePack = NULL;
   if ( routeID.isValid() ) {
      uint32 startTime = TimeUtility::getCurrentTime();
      uint32 expandType = (ROUTE_TYPE_STRING | ROUTE_TYPE_NAVIGATOR |
                           ROUTE_TYPE_ITEM_STRING | ROUTE_TYPE_GFX);
      routePack = m_thread->getStoredRouteAndExpand(
         routeID, expandType, langCode, 
         false/*abbreviate*/, false/*landmarks*/, true/*removeAheadIfDiff*/,
         false/*nameChangeAsWP*/, expReq, expandRouteCont );
      if ( routePack == NULL || expReq == NULL || expandRouteCont == NULL )
      {
         mc2log << warn 
                << "getRouteTileMapCache: getStoredRouteAndExpand failed ";
         if ( TimeUtility::getCurrentTime() - startTime > 3000 ) {
            mc2log << "Timeout";
         } else {
            // No such route?
            mc2log << "Error";
         }
         mc2log << endl;
      }
   } // End if ok to get stored route


   if ( expandRouteCont != NULL && expandRouteCont->getPacket() != NULL ) {
      // Get the ExpandRouteReplyPacket.
      ExpandRouteReplyPacket* errp = static_cast<ExpandRouteReplyPacket*> (
            expandRouteCont->getPacket() );
      ExpandedRoute expRoute( errp );
  
      getRouteTileMapCache( outBuf,
                            &expRoute,
                            routeID,
                            lang,
                            layerIDs,
                            cacheName,
                            cacheMaxSize,
                            useGzip );
   } 
   
   delete expReq;
   delete expandRouteCont;
   delete routePack;
   
}

DataBuffer*
ParserTileHandler::getRouteTileMaps( const ExpandedRoute* expRoute,
                                const RouteID& routeID,
                                LangTypes::language_t lang,
                                const set<int>& layerIDs,
                                uint32 startOffset,
                                uint32 maxBytes )
{

   // Create the params.
   set<MC2SimpleString> routeParams;
   getRouteTileMapParams( expRoute,
                          routeID,
                          lang,
                          layerIDs,
                          routeParams );

   // Add everything from the set to the vector.
   vector<MC2SimpleString> routeParamVector;
   routeParamVector.reserve( routeParams.size() );
   routeParamVector.insert( routeParamVector.end(), 
                            routeParams.begin(), 
                            routeParams.end() );

   // Dump the route params.
   mc2dbg << "[PTH]: getRouteTileMaps:" << endl;
   for ( uint32 i = 0; i < routeParamVector.size(); ++i ) {
      mc2dbg << "[PTH]: " << routeParamVector[ i ] << endl;
   }
   
   // Get the tilemaps.
   return getTileMaps( routeParamVector,
                       startOffset, maxBytes );
}

void
ParserTileHandler::removeTileMapBUffer( const char* paramStr ) {
   m_tileMapCache->removeBuffer( paramStr );
}

void
ParserTileHandler::createEmptyMaps( const MC2SimpleString& inParam,
                                    vector<TileMapBufferHolder>& retVector,
                                    vector<MC2SimpleString>& params,
                                    const ServerTileMapFormatDesc& tmfd ) {
   // No good. So make empty.
   mc2dbg << "[PTH]: Empty map created for invalid param " 
          << inParam << endl;

   // Find out which params that should be extracted.
   tmfd.getAllImportances( inParam, params );

   for ( uint32 i = 0; i < params.size(); ++i ) {
      // create tile map buffer holder and add it to our return vector
      auto_ptr<TileMapBufferHolder>
         ocean( TileMapCreator::createOceanTileMap( params[ i ], tmfd ) );
      retVector.push_back( *ocean );
   }
}


BitBuffer*
ParserTileHandler::requestCache( const MC2SimpleString& inParam, 
                                 vector<TileMapBufferHolder>& retVector )
{
   MC2_ASSERT( m_tileMapCache );
   // In ACP mode we don't want to go through TileMapParams::getAsString(),
   // since that will break it
   if ( m_acpMode ) {
      return requestCacheDirect( inParam, retVector );
   }

   if ( !isMap( inParam ) ) {
      mc2dbg << "[PTH] requestCache: Not a map" << endl;
      return NULL;
   }
   TileMapParams realParam( inParam );
   MC2_ASSERT( realParam.getImportanceNbr() == 0 );

   // if not in acp request mode and this is a acp param; check acp 
   if ( ACPTileMapRequest::isACPParam( realParam ) ) {
      // lets see if we have any ACP cached for this param.
      // if we do not have any cached; then it will return NULL and let
      // the caller take care of the request.
      return composeACP( inParam, retVector );
   }

   return requestCacheDirect( inParam, retVector );
}

BitBuffer*
ParserTileHandler::requestCacheDirect( const MC2SimpleString& inParam, 
                                       vector<TileMapBufferHolder>& retVector ) {

   const ServerTileMapFormatDesc* tmfd =
      m_group->
      getTileMapFormatDesc( getDefaultSTMFDParams( inParam.c_str() ), 
                            m_thread );

   BitBuffer* allBuffers = NULL;
   // insert "N" to mark New cache file
   MC2SimpleString param( cacheString( inParam ) );

   // Try Premade files
   // TODO: What to do it we get crc missmatch (G from premade and T from MM)
   allBuffers = m_group->getSFDHolder() != NULL ? 
      m_group->getSFDHolder()->requestCached( inParam.c_str(), *tmfd ) : NULL;
   // TODO: A feature is that the allBuffers is returned to 
   //       m_tileMapCache->release. Not good looking.

   if ( allBuffers == NULL ) {

      mc2dbg8 << "[PTH]: REQUEST CACHE, param = " << param << endl;

      allBuffers = m_tileMapCache->requestCached( param.c_str() );
   }

   if ( allBuffers == NULL ) {
      if ( ! tmfd->valid( inParam ) ) {
         vector<MC2SimpleString> params;
         createEmptyMaps( inParam, retVector, params, *tmfd );
         bool ok = storeCache( inParam, retVector );
         
         // Now the empty maps are in the cache. ( maybe: )
         //
         // Trafic layer and invalid params are not written to cache
         // and if we try to call this method again we will fail to 
         // write to cache again and ..yes, will be a huge-stack-of-death
         // due to the recursive calls.
         // ( Example: G+thGgCE )
         retVector.clear();
         if ( ok ) {
            // Call this method again if they were written to file cache.
            return requestCache( inParam, retVector ); 
         } else {
            // ok, the cache was not written, so dont even 
            // think about trying to write to cache again or
            // a horde of angry stack bashing monkeys will rain on
            // your parade. No, Im just kidding;
            // there will be only one ingenious monkey with a time machine.
            return NULL;
         }

      } else {
         // Ordinary tile that was not found in cache.
         // Handle as usual.
         return NULL;
      }
   }
         

   /* 
    *  Cache buffer format:
    *  4 bytes [long] ; nbr of buffers
    *  --- buffer ---
    *  4 bytes [long] ; size of buffer
    *  4 bytes [long] ; crc
    *  string         ; description
    *  1 byte  [bool] ; if empty buffer
    *  n bytes        ; buffer
    * ---------------
    */

   mc2dbg4 << "param: " << param << endl;
   mc2dbg4 << "buffer size: " << allBuffers->getBufferSize() << endl;

   uint32 nbrBuffers = allBuffers->readNextBALong();

   mc2dbg4 << "Loading cache: " << nbrBuffers << " number of buffers. " << endl;

   // load all small buffers and add them to return vector

   for ( uint32 i = 0; i < nbrBuffers; ++i ) {
      uint32 buffSize = allBuffers->readNextBALong();
      uint32 crc = allBuffers->readNextBALong();
      const char * const desc = allBuffers->readNextString();
      bool empty = allBuffers->readNextByte();

      // create a new buffer and copy data since the big buffer will
      // be deleted
      BitBuffer *holderBuffer = NULL;
      if ( buffSize != 0 ) {
         mc2dbg8 << "Creating holder buffer with size: " << buffSize << endl;
         holderBuffer = new BitBuffer( allBuffers->getCurrentOffsetAddress(),
                                       buffSize );
         
         // skip the holderBuffer bytes
         allBuffers->readPastBytes( buffSize );
      } else {
         mc2dbg << "No holder buffer" << endl;
      }

      // create tile map buffer holder and add it to our return vector
      TileMapBufferHolder tmp( desc, 
                               holderBuffer, 
                               crc, 
                               empty );
      retVector.push_back( tmp );
   }

   // make sure the impotance number is the only thing
   // that differs
   for ( uint32 i = 0; i < retVector.size(); ++i ) {
      TileMapParams imp0( retVector[ i ].getDesc() );
      imp0.setImportanceNbr( 0 );
      if ( imp0.getAsString() != inParam ){
         mc2dbg << "imp0: " << imp0.getAsString() << " inParam: " 
                << inParam << endl;
         // could be acp here....
         //         MC2_ASSERT( imp0.getAsString() == inParam );
      }
   }

   for ( int i = 0, n= retVector.size(); i < n; ++i ) {
      if ( TileMapParams( retVector[i].getDesc()).getImportanceNbr() != i ) {
         mc2log << "[PTH]: Error in cache - removing "
                << MC2CITE(retVector[i].getDesc()) 
                << " imp " 
                << TileMapParams( retVector[i].getDesc() ).getImportanceNbr()
                << " != " << i << endl;
         // TODO: But param is NG-... so it can't be used, perhaps inParam?
         eraseMapsFromCache( param );
      }
      MC2_ASSERT( TileMapParams(
         retVector[i].getDesc()).getImportanceNbr() == i );
   }

   // this cache file is b0rk   
   if ( retVector.empty() ) {
      mc2dbg << "[PTH] Cache: " << MC2CITE( param ) << " is empty." << endl;
      m_tileMapCache->release( param.c_str(), allBuffers );
      allBuffers = NULL;
   }
   return allBuffers;
}
namespace {

/**
 * Creates a cache buffer from a set of tilemap buffer holders.
 * @param tileMapBuffers the buffers to create a bit buffer from
 * @param newTileMapBuffers an array of buffers that the descriptor string
 *        should be updated on. Default none. This must have the same number
 *        of items as the tileMapBuffers.
 * @return the complete buffer with data.
 */
BitBuffer* 
createCacheBuffer( const TileMapRequest::TileBuffers& tileMapBuffers,
                   TileMapRequest::TileBuffers* newTileMapBuffers = NULL ) {
   // for buffer format see ParserTileHandler::requestCache
      
   // calculate buffer size
   uint32 bufferSize = 4; // 4 for number of buffers
   // accumulate variable size for each buffer
   for ( uint32 i = 0; i < tileMapBuffers.size(); ++i ) {
      bufferSize += strlen( tileMapBuffers[ i ].getDesc() ) + 1;
      if ( tileMapBuffers[ i ].getBuffer() != NULL ) {
         bufferSize += tileMapBuffers[ i ].getBuffer()->
            getBufferSize();
      }
   }

   // add fixed size for each buffer times nbr of items
   bufferSize += 9 * tileMapBuffers.size();


   mc2dbg4 << "Caching: " << tileMapBuffers.size() << " number of buffers. " << endl;
   mc2dbg4 << "Total buffer size: " << bufferSize << " bytes" << endl;

   BitBuffer *cacheBuffer = new BitBuffer( bufferSize );

   // write data
   cacheBuffer->writeNextBALong( tileMapBuffers.size()  );
   for ( uint32 i = 0; i < tileMapBuffers.size(); ++i ) {
      const TileMapBufferHolder& buff = tileMapBuffers[ i ];

      if ( buff.getBuffer() != NULL ) {
         cacheBuffer->writeNextBALong( buff.getBuffer()->getBufferSize() );
      } else {
         cacheBuffer->writeNextBALong( 0 );
      }

      cacheBuffer->writeNextBALong( buff.getCRC() ); 
      // save desc before trying to set it, 
      // the buffer we are going to set might be the same as the one we
      // are going to save.
      const char* desc = buff.getDesc();
      // set new descriptor string if we have tile map buffers that
      // needs updating
      if ( newTileMapBuffers ) {
         (*newTileMapBuffers)[ i ].
            setDesc( (const char*)cacheBuffer->getCurrentOffsetAddress() );
      }
      cacheBuffer->writeNextString( desc );

      cacheBuffer->writeNextByte( buff.getEmpty() );

      if ( buff.getBuffer() != NULL ) {
         cacheBuffer->writeNextByteArray( buff.getBuffer()->getBufferAddress(),
                                          buff.getBuffer()->getBufferSize() );
      }

   }
   return cacheBuffer;
}

}

bool
ParserTileHandler::
storeCache( const MC2SimpleString& inParam,
            const std::vector<TileMapBufferHolder>& tileMapBuffers ) {
   if ( tileMapBuffers.empty() ) {
      mc2dbg8 << "[PTH] Store cache, buffer empty for param: " << inParam << endl;
      return false;
   }

   if ( ! isMap( inParam ) ) {
      mc2dbg8 << "[PTH]: storeCache not a map" << endl;
   }

   // Must have importance 0 to store in cache.
   if ( TileMapParams( inParam ).getImportanceNbr() != 0 ) {
      return false;
   }

   /* This will not work with ACP
   for ( int i = 0, n= tileMapBuffers.size(); i < n; ++i ) {
      MC2_ASSERT( TileMapParams(
         tileMapBuffers[i].getDesc()).getImportanceNbr() == i );
      
      TileMapParams imp0( tileMapBuffers[ i ].getDesc() );
      imp0.setImportanceNbr( 0 );
      if ( imp0.getAsString() != inParam ){
         mc2dbg << "imp0: " << imp0.getAsString() << " inParam: " 
                << inParam << endl;
         MC2_ASSERT( imp0.getAsString() == inParam );
      }      

   }
   */

   // insert "N" to mark New cache
   MC2SimpleString param( cacheString( inParam ) );

   mc2dbg8 << "[PTH]: STORE CACHE" << endl;
   mc2dbg8 << "param: " << param << endl;

   BitBuffer* cacheBuffer = ::createCacheBuffer( tileMapBuffers );

   bool wasAllowed = m_tileMapCache->allowed( param, cacheBuffer );
   m_tileMapCache->release( param.c_str(), cacheBuffer );

   return wasAllowed;
}

bool
ParserTileHandler::
storeCache( const MC2SimpleString& inParam,
            const TileMapRequest* request ) {
   const ACPTileMapRequest *acp = dynamic_cast<const ACPTileMapRequest*>( request );

   if ( acp ) {
      mc2dbg << "Store as ACP." << endl;
      // store each request from this one in cache.
      // Name them as "param.mapright", ex: T+34234.64
      // where 64 is from UserEnums::userRightService

      ACPTileMapRequest::ParamRequests cacheMap;
      acp->getTileMapRequests( cacheMap );
      typedef ACPTileMapRequest::ParamRequests::const_iterator const_iterator;
      const_iterator it = cacheMap.begin();
      const_iterator itEnd = cacheMap.end();
      for ( ; it != itEnd; ++it ) {
         mc2dbg << "ACP store cache: " << it->first << endl;
         storeCache( it->first.c_str(), *it->second );
      }

      return true;
   } else {
      mc2dbg4 << "Store as normal." << endl;
      // store as normal
      TileMapRequest::TileBuffers buffers;
      request->getTileMapBuffers( buffers );
      return storeCache( inParam, buffers );
   }
}

TileMapRequest* 
ParserTileHandler::
createTileMapRequest( const ServerTileMapFormatDesc& mapDesc,
                      const TileMapParams& params,
                      const RouteReplyPacket* routeReplyPack ) {
   // Determine what kind of tiles we are going to get.
   if ( ACPTileMapRequest::isACPParam( params ) ) {

      MapRights userMapRights;
      if ( m_thread->getCurrentUser() ) {
         userMapRights =
            m_thread->getCurrentUser()->getUser()->
            getAllMapRights( TimeUtility::getRealTime() );
      }

      mc2dbg8 << "[PTH] createTileMapRequest: Requesting with rights: " << userMapRights << endl;

      ACPTileMapRequest::RequestVector tilereqs;
      ACPTileMapRequest::RightsVector tilerights;

      // fetch ALL map rights requests, 
      // this way the cache fetching will be easier for ACP with
      // other rights than normal.

      ACPTileMapRequest::
         createTileMapRequests( *m_thread,
                                mapDesc, 
                                params,
                                m_thread->getTopRegionRequest(),
                                userMapRights, //~MapRights(),
                                tilereqs, tilerights );

      MultiRequest mrequest( m_thread->getNextRequestID(),
                             vector<RequestWithStatus*>(tilereqs.begin(),
                                                        tilereqs.end()) );
      m_thread->putRequest( &mrequest );

      mc2dbg8 << "[PTH] createTileMapRequest: Requesting a set of tiles: "
             << tilereqs.size() << endl;
      for ( uint32 i = 0; i < tilereqs.size(); ++i ) {
         // We can not set offset unless we have a valid buffer
         if ( tilereqs[ i ]->getStatus() != StringTable::OK ) {
            continue;
         }
         tilereqs[ i ]->getTileMapBuffer().getBuffer()->setOffsetToSize();
      }
      // lets fetch ACP tiles
      return new ACPTileMapRequest( m_thread->getNextRequestID(),
                                    mapDesc,
                                    params, 
                                    m_thread->getTopRegionRequest(),
                                    userMapRights,
                                    tilereqs, tilerights );
   } else {
      // oh, just plain boring tiles....
      return new TileMapRequest( m_thread->getNextRequestID(),
                                 mapDesc,
                                 params,
                                 routeReplyPack,
                                 m_thread->getTopRegionRequest() );
   }
   
}

// for debug
ostream& operator << ( ostream& ostr, const
                       ACPTileMapRequest::CacheStrings::value_type cache ) {
   ostr << cache.first << ", " << cache.second;
   return ostr;
}

bool ParserTileHandler::
getACPTileRequests( const ACPTileMapRequest::CacheStrings& cacheStrings,
                    vector<BitBuffer*>& cacheBuffers,
                    ACPTileMapRequest::RequestInfoVector& tilereqs,
                    ACPTileMapRequest::RightsVector& tilerights ) {

   // for each cache string, featch cache tile map,
   // if tilemap is not in cache then request it and try cache again
   // Each buffer holder from the cache is stored in tilereqs
   typedef ACPTileMapRequest::CacheStrings::const_iterator const_iterator;
   const_iterator it = cacheStrings.begin();
   const_iterator itEnd = cacheStrings.end();
   for ( ; it != itEnd; ++it ) {
      TileMapRequest::TileBuffers holder;
      BitBuffer* buf = requestCacheDirect( it->first.c_str(), holder );
      if ( buf == NULL ) {
         mc2dbg4 << "[PTH] getACPTileRequests: Cache is empty!" << endl;

         // go into acp mode section, so we don't fetch special acp again
         m_acpMode = true;
         // fetch missing param and ignore buffer and get it from the cache
         delete m_thread->getTileMap( it->first.c_str() );
         m_acpMode = false;
         buf = requestCacheDirect( it->first.c_str(), holder );

         // argh, this is so wrong, one buffer is missing
         if ( buf == NULL ) {
            STLUtility::deleteValues( tilereqs );
            return false;
         }

      }
      // keep the cache buffer alive until the entire ACP request is done.
      cacheBuffers.push_back( buf );

      for ( uint32 i = 0; i < holder.size(); ++i ) {
         // set to holder to end offset, so we can use it in the request
         holder[ i ].getBuffer()->setOffsetToSize();
      }

      // assign info to ACPTileMapRequest params
      tilereqs.push_back( new ACPRequestHolder::
                          ACPRequestHolder( it->first.c_str(), holder ) );
      tilerights.push_back( it->second );
   }

   return true;
}

BitBuffer*
ParserTileHandler::composeACP( const TileMapParams& params,
                               vector<TileMapBufferHolder>& buffers ) {
   mc2dbg8 << "[PTH] " << __FUNCTION__ << endl;
   /*
    * This is how it goes:
    *
    * 1 ) Get ACP cache strings for this param 
    * 2 ) Check cache against these strings
    * 2.1 ) Have all cached -> step 3
    * 2.2 ) Some cached, this should not happen -> return no cache
    * 2.3 ) No cache -> return no cache
    * 3) merge tilemaps
    *
    */

   // get user rights
   MapRights userMapRights;
   if ( m_thread->getCurrentUser() ) {
      userMapRights = m_thread->getCurrentUser()->
         getUser()->getAllMapRights( TimeUtility::getRealTime() );
   }

   mc2dbg << "[PTH] composeACP: Requesting with rights: " << userMapRights << endl;

   // get strings for parameters to fetch from cache
   ACPTileMapRequest::CacheStrings cacheStrings;
   // holds data strings if the request is a string-map
   ACPTileMapRequest::CacheStrings dataStrings;

   //   
   // Get Cache strings
   //

   { 
      // cache is stored as importance 0 name
      TileMapParams imp0Param( params );
      imp0Param.setImportanceNbr( 0 );

      ACPTileMapRequest::getCacheNames( imp0Param, userMapRights,
                                        cacheStrings );
      if ( imp0Param.getTileMapType() == TileMapTypes::tileMapStrings ) {
         imp0Param.setTileMapType( TileMapTypes::tileMapData );
         ACPTileMapRequest::getCacheNames( imp0Param, userMapRights,
                                           dataStrings );
      }

      // debug
      /*
      mc2dbg4 << "[PTH] ACP params: ";
      copy( cacheStrings.begin(), cacheStrings.end(),
            ostream_iterator<ACPTileMapRequest::CacheStrings::value_type>
            ( mc2dbg, ", " ) );
      */
   }

   // if we do not have any cache strings
   // then it means that we probably do not have ACP rights!
   // The client should not even request them, but since they can...
   // (and some old clients do ) we must check for this here and
   // create empty tile maps, so the client do not keep requesting them
   // and get "301" errors.
   if ( cacheStrings.empty() ) {
      const ServerTileMapFormatDesc* desc =
         m_group->
         getTileMapFormatDesc( getDefaultSTMFDParams(), 
                               m_thread );
      WritableTileMap emptyTileMap( params,
                                    *desc,
                                    0, // unused
                                    0 ); // nbr polys.
      
      BitBuffer* holderBuffer = new BitBuffer( 1024 );
      emptyTileMap.save( *holderBuffer );

      holderBuffer->setSizeToOffset();
      holderBuffer->reset();

      TileMapBufferHolder tmp( params.getAsString().c_str(), 
                               holderBuffer, 
                               emptyTileMap.getCRC(), true ); 
      buffers.push_back( tmp );

      return ::createCacheBuffer( buffers, &buffers );

   }

   //
   // Do we have anything cached?, setup request info for cached info.
   //

   // info for ACPTileMapRequest
   ACPTileMapRequest::RequestInfoVector tilereqs;
   ACPTileMapRequest::RightsVector tilerights;
   tilerights.reserve( cacheStrings.size() );


   // cache buffers, will be deleted once we exit this function
   STLUtility::AutoContainer< vector<BitBuffer*> > cacheBuffers;

   if ( ! getACPTileRequests( cacheStrings, cacheBuffers,
                              tilereqs, tilerights ) ) {
      STLUtility::deleteValues( tilereqs );
      return NULL;
   }

   //
   // compose the merge request and send it.
   //

   const ServerTileMapFormatDesc* desc =
      m_group->
      getTileMapFormatDesc( getDefaultSTMFDParams(), 
                            m_thread );
   
   // tilereqs and rights are now owned by this request
   ACPTileMapRequest acp( m_thread->getNextRequestID(),
                          *desc,
                          params, 
                          m_thread->getTopRegionRequest(),
                          userMapRights,
                          tilereqs, tilerights );
   m_thread->putRequest( &acp );

   if ( acp.getStatus() != StringTable::OK ) {
      mc2dbg << "[PTH] composeACP: Merge failed!" << endl;
      // merge request failed
      return NULL;
   }

   //
   // request done successfully, create a bit buffer from the holders
   // and return it.
   //
   acp.getTileMapBuffers( buffers );

   return ::createCacheBuffer( buffers, &buffers );
}
