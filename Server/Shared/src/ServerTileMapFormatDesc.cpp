/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ServerTileMapFormatDesc.h"
#include "GfxFeature.h"
#include "TileMapParams.h"
#include "UTF8Util.h"
#include "MC2CRC32.h"
#include "GfxPolygon.h"
#include "GfxRoadPolygon.h"

#include "Packet.h"
#include "STLUtility.h"
#include "TileCollectionNotice.h"
#include "TileMapParamTypes.h"
#include "ArrayTools.h"
#include "Math.h"
#include "TileColorSettings.h"
#include "Properties.h"
#include "TileFeatureScaleRange.h"
#include "ParamsNotice.h"

#define MAX_TILE_SCALELEVEL MAX_UINT16

//#define ROUTE_FACTOR 37
#define ROUTE_FACTOR 10

// Number of detail levels in the map layer. BUGGED clients will believe
// that all layers have this many detail levels :(
#define NBR_MAP_DETAILS 9


inline
uint32 makeColorRGB( uint32 r, uint32 g, uint32 b ) {
   return ( (r << 16) | (g << 8) | (b) );
}
   

inline
uint32 makeColorGD(GDUtils::Color::imageColor color) 
{
   return ::makeColorRGB(GDUtils::Color::getRed(color),
                         GDUtils::Color::getGreen(color),
                         GDUtils::Color::getBlue(color));
}

uint32 
ServerTileMapFormatDesc::makeColor (GDUtils::Color::imageColor dayColor, 
                                    GDUtils::Color::imageColor nightColor)
const
{
   if ( !m_nightMode ){
      return ::makeColorGD(dayColor);
   }
   else {
      return ::makeColorGD(nightColor);
   }
}


inline uint32 
ServerTileMapFormatDesc::makeColor(uint32 dayRed, 
                                   uint32 dayGreen, 
                                   uint32 dayBlue, 
                                   uint32 nightRed, 
                                   uint32 nightGreen, 
                                   uint32 nightBlue) const
{
   if ( !m_nightMode ){
      return ::makeColorRGB(dayRed, dayGreen, dayBlue);
   }
   else {
      return ::makeColorRGB(nightRed, nightGreen, nightBlue);
   }
}

inline uint32 
ServerTileMapFormatDesc::makeColor(GDUtils::Color::imageColor dayColor, 
                                   uint32 nightRed, 
                                   uint32 nightGreen, 
                                   uint32 nightBlue) const
{
   if ( !m_nightMode ){
      return ::makeColorGD(dayColor);
   }
   else {
      return ::makeColorRGB(nightRed, nightGreen, nightBlue);
   }
}

const uint32 STMFDParams::DEFAULT_LAYERS =  
( 1 << TileMapTypes::c_poiLayer ) |
( 1 << TileMapTypes::c_mapLayer ) |
( 1 << TileMapTypes::c_trafficLayer ) |
( 1 << TileMapTypes::c_routeLayer );

// Version number. Must fit inside 5 bits, i.e. < 32, otherwise
// everything will blow up. Don't change this ever please.
const uint32 STMFDParams::DEFAULT_SERVER_PREFIX = 10;
// Server prefix for devices that can handle some more content
const uint32 STMFDParams::HIGH_END_SERVER_PREFIX = 11;

int 
STMFDParams::save( Packet* packet, int& pos ) const 
{
   int startPos = pos;
   packet->incWriteLong( pos, 0 ); // Length, filled in later.
  
   // The language.
   packet->incWriteLong( pos, (int) m_lang );
   packet->incWriteByte( pos, m_nightMode );
   packet->incWriteByte( pos, m_serverPrefix );
   packet->incWriteLong( pos, m_layers );
   packet->incWriteLong( pos, m_imageSet );
   packet->incWriteLong( pos, m_drawSettingVersion );

   int nbrBytes = pos - startPos;
   packet->incWriteLong( startPos, nbrBytes );

   return nbrBytes;
}

int 
STMFDParams::load( const Packet* packet, int& pos ) {
   int startPos = pos;
   int sizeInPacket = packet->incReadLong( pos );
  
   m_lang = LangTypes::language_t( packet->incReadLong( pos ) );
   m_nightMode = packet->incReadByte( pos );
   m_serverPrefix = static_cast< uint32 >( packet->incReadByte( pos ) );
   m_layers = packet->incReadLong( pos );
   m_imageSet = static_cast< ImageTable::ImageSet >
      ( packet->incReadLong( pos ) );
   m_drawSettingVersion = packet->incReadLong( pos );

   pos = startPos + sizeInPacket;
   return sizeInPacket;
}

ServerTileMapFormatDesc::
ServerTileMapFormatDesc( const STMFDParams& settings ):
   TileMapFormatDesc(),
   m_drawSettingVersion( 0 ),
   m_layers( 0 ),
   m_imageSet( ImageTable::DEFAULT ) {
   init( settings );
}

ServerTileMapFormatDesc::
ServerTileMapFormatDesc( const STMFDParams& settings,
                         const CopyrightHolder& copyrights ):
   TileMapFormatDesc(),
   m_drawSettingVersion( 0 ),
   m_layers( 0 ),
   m_imageSet( ImageTable::DEFAULT ) {
   m_copyrightHolder = copyrights;
   init( settings );
}

void ServerTileMapFormatDesc::init( const STMFDParams& settings ) {
   m_nightMode = settings.m_nightMode;
   m_lang = settings.m_lang;
   m_layers = settings.getLayers();
   m_drawSettingVersion = settings.getDrawSettingVersion();
   m_imageSet = settings.getImageSet();
   m_reserveDetailLevel   = 6;
   m_extraTilesForReserve = 3;
   m_serverPrefix = settings.getServerPrefix();
   m_textColor = 0x000000;
   m_topHorizonColor = 0x000000;
   m_bottomHorizonColor = 0x000000;
}

ServerTileMapFormatDesc::~ServerTileMapFormatDesc()
{
   // Remove the m_argsPerType
   {
      for ( argTransferPerType_t::iterator it = m_argsPerType.begin();
            it != m_argsPerType.end(); ++it ) {
         delete (*it).second;
      }
      m_argsPerType.clear();
   }
   // Remove the args
   {
      for ( argsByType_t::iterator it = m_argsByTileFeatureType.begin();
            it != m_argsByTileFeatureType.end(); ++it ) {
         STLUtility::deleteValues( it->second );
      }

      for ( argsByTypeAndName_t::const_iterator it = 
               m_argsByTypeAndName.begin(); it != m_argsByTypeAndName.end() ; ++it ) {
         delete it->second;
      }
      m_argsByTypeAndName.clear();
   }
   // This vector holds the same args as in m_argsByTileFeatureType
   // so we must not delete the args inside in TileMapFormatDesc.
   delete [] m_argsByTileFeatureTypeArray;
   m_argsByTileFeatureTypeArray = NULL;
}

void
ServerTileMapFormatDesc::writeCategories( SharedBuffer& buf, bool latin1)
{
   if ( latin1 ) {
      // Have to create categories in english since some clients
      // correctly convert from utf-8 and some don't
      createCategories( LangTypes::english );
   }
   int nbrPoiCategories = m_categories.size(); 
   buf.writeNextBALong( nbrPoiCategories );
   buf.writeNextBALong( m_nbrCategoryPoiTypes );     
   for ( int i = 0; i < nbrPoiCategories; ++i ) {
      if ( latin1 ) {
         buf.writeNextString( UTF8Util::utf8ToIso(
                                 m_categories[i].getName().c_str() ).c_str() );
      } else {
         buf.writeNextString( m_categories[i].getName().c_str() );
      }
      buf.writeNextBAShort( m_categories[i].getID() );
      buf.writeNextBAByte( m_categories[i].isEnabled() );
      buf.writeNextBALong( m_categories[i].getNbrTypes() );
      for ( int j = 0; j < m_categories[i].getNbrTypes(); ++j ) {
         buf.writeNextBALong( m_categories[i].getType(j) );
      }
   }
   if ( latin1 ) {
      // Create the correct categories again.
      createCategories( m_lang );
   }
}

void 
ServerTileMapFormatDesc::save( BitBuffer& buf, uint32* timeStampPos )
{
   buf.fillWithZeros();
   // Save the general tile information.
   buf.alignToByte();

   // Write the server prefix (version number) as a string to
   // be compatible.
   char tmpStr[128];
   sprintf(tmpStr, "%u", (unsigned int)m_serverPrefix);
   buf.writeNextString( tmpStr );

   uint32 nbrLayers = getNbrLayers();
   MC2_ASSERT( nbrLayers == m_tileSettingsByLayerNbr.size() );
   
   // Nbr layers.
   buf.writeNextBAByte( nbrLayers );
  
   for ( uint32 i = 0; i < nbrLayers; ++i ) {

      // Tile data for the layer.
      const tileSettings_t& setting = m_tileSettingsByLayerNbr[ i ];

      buf.writeNextBAShort( setting.m_meters );
      buf.writeNextBAShort( setting.m_pixels );
      buf.writeNextBAShort( setting.m_dpi );
      buf.writeNextBALong( uint32( setting.m_zoomFactor * 1000 ) );
      buf.writeNextBALong( uint32( setting.m_exchangeTileFactor * 1000 ) );

      // INFO: FIX DUE TO BUGGED CLIENTS
      // Due to a bug in old clients, all number of layers must be the same
      // as for layer 0, i.e. the map. 
      // Therefore send the number of detaillevels in the maplayer for
      // all layers, because otherwise the old clients will crash.
      // Note, that we resend the correct information to the new clients
      // a little below here.
      buf.writeNextBAByte( NBR_MAP_DETAILS ); 
//      buf.writeNextBAByte( setting.m_detailLevels );

      buf.writeNextBAByte( m_layerIDsAndDesc[ i ].getID() );
      buf.writeNextString( m_layerIDsAndDesc[ i ].getName().c_str() );

      // Save the importance table.
      m_importanceTables[ i ]->save( buf );
   }
  
   // Save m_argsByTileFeatureType. 
   // These are the arguments to be transferred, i.e. they do not contain
   // the default arguments.
//   typedef map<int16, 
//      vector<TileFeatureArg*> > argsByType_t;
  
   mc2dbg << "[STMFD] save : m_argsByTileFeatureType.size() "
          << m_argsByTileFeatureType.size() << endl;   
   {
      MC2_ASSERT( m_argsByTileFeatureTypeArray );
      // Write the size
      buf.writeNextBAShort( m_argsByTileFeatureTypeArraySize );
      // Write the contents
      for ( int x = 0; x < m_argsByTileFeatureTypeArraySize; ++x ) {
         // Key. Not necessary, really, but I will keep it until
         // it works.
         buf.writeNextBAShort( x );
         // Value. vector<TileFeatureArg*>.
         const vector<TileFeatureArg*>* vec = &m_argsByTileFeatureTypeArray[x];
         // Size of vector
         buf.writeNextBAByte( vec->size() );
         // The args of the vector.
         for ( vector<TileFeatureArg*>::const_iterator jt = vec->begin();
               jt != vec->end(); ++jt ) {
            // Write the arg.
            (*jt)->saveFullArg( buf );
         }
         buf.alignToByte();
      }
   }
  
   // Save m_scaleIndexByLevel.
   buf.writeNextBAByte( m_scaleIndexByLevel.size() );
   for ( scaleIndexByLevel_t::const_iterator it = 
            m_scaleIndexByLevel.begin(); it != m_scaleIndexByLevel.end(); 
         ++it ) {
      buf.writeNextBALong( it->first );
   }
   
   // Save the default arguments for the primitives.
   m_defaultArgs.save( buf );
   buf.alignToByte();

   // Save m_primitiveDefaultMap.
   // typedef map<int16, primitiveVector_t>
   //    primitiveDefaultMap_t;
   // typedef vector<TileFeature*> primitiveVector_t;
   
   mc2dbg8 << "[STMFD] save : m_primitiveDefaultMap.size() " 
           << m_primitiveDefaultMapSize << endl;
   // Size of the map.
   buf.writeNextBAShort( m_primitiveDefaultMapSize );
   for ( int i = 0 ; i < m_primitiveDefaultMapSize; ++i ) {
      // Key.
      buf.writeNextBAShort( i );
      // Value. 
      // vector<TileFeature*>
      const primitiveVector_t& vec = m_primitiveDefaultMap[i];
      // Size of vector.
      buf.writeNextBAByte( vec.size() );
      for ( primitiveVector_t::const_iterator jt = vec.begin();
            jt != vec.end(); ++jt ) {
         //  Type
         buf.writeNextBAShort( (*jt)->getType() );
         // Nbr args.
         buf.writeNextBAByte( (*jt)->getNbrArgs() );
         // The args.
         for ( TilePrimitiveFeature::arg_iterator kt = (*jt)->arg_begin();
               kt != (*jt)->arg_end(); ++kt ) {
            buf.writeNextBAShort( m_defaultArgs.getArgIndex( *kt ) ); 
            //(*kt)->saveFullArg( buf ); 
         }
         buf.alignToByte();
      }
   }

   // Save m_argsPerType.
   // typedef map<int16,
   //    argTransferMap_t*> argTransferPerType_t;

   mc2dbg8 << "[STMFD] save : m_argsPerType.size() " 
           << m_argsPerType.size() << endl;

   // Important! argTransferMap_t:s for all feature types must be sent
   // because the client keeps them in a vector, not a map.

   {
      // Get the last feature type used.
      featureInt lastFeatureType = (*m_argsPerType.rbegin()).first;
      // Size of the array
      featureInt size = lastFeatureType + 1;
      buf.writeNextBAShort( size );
      
      static const argTransferMap_t emptyMap;
      for ( featureInt x = 0; x < size; ++x ) {
         // Find the current feature int.
         argTransferPerType_t::const_iterator it = m_argsPerType.find( x );
         // Default to the empty map.
         const argTransferMap_t* transfMap = &emptyMap;
         if ( it != m_argsPerType.end() ) {
            transfMap = it->second;
         }
         // Key.
         buf.writeNextBAShort( x );
         // Value.
         //   typedef multimap<TileArgNames::tileArgName_t,
         //           pair<int, TileArgNames::tileArgName_t> >
         //      argTransferMap_t;
         
         // Size of the multimap.
         buf.writeNextBAShort( transfMap->size() );
         for ( argTransferMap_t::const_iterator jt = transfMap->begin();
               jt != transfMap->end(); ++jt ) {
            // Key, TileArgNames::tileArgName_t
            buf.writeNextBAByte( (*jt).first );
            // Value, pair<int, TileArgNames::tileArgName_t>.
            // int.
            buf.writeNextBAShort( (*jt).second.first );
            // MC2SimpleString.
            buf.writeNextBAByte( (*jt).second.second );
         }
      }
   }

   // Background color.
   buf.writeNextBALong( m_backgroundColor );

   // Write the detail level of the reserve maps
   buf.writeNextBALong( m_reserveDetailLevel );
   // Write the number of extra tiles around center for reserve maps
   buf.writeNextBALong( m_extraTilesForReserve );
   
   /// Write the language
   buf.writeNextBALong( m_lang );

   // Also write the categories once in latin1 for old buggy S60 WF.
   // These will be in English since UIQ client wasn't buggy.
   writeCategories( buf, true );
   
   //dumpCategories();

   uint32 crcPOS = buf.getCurrentOffset();
   // Write a zero for the crc calculation.
   buf.writeNextBALong( 0 );
   // Write a zero for the time stamp so that it is not included in crc
   buf.writeNextBALong( 0 );

   // Resend the tile information, correct this time.
   MC2_ASSERT( nbrLayers == m_tileScaleByLayer.size() );
   for ( uint32 i = 0; i < m_tileScaleByLayer.size(); ++i ) {
      const tileScale_t& tileScale = *m_tileScaleByLayer[ i ];
      // Nbr detail levels.
      buf.writeNextBAByte( tileScale.size() );
      for ( uint32 j = 0; j < tileScale.size(); ++j ) {
         // mc2UnitsForTile
         buf.writeNextBALong( tileScale[ j ].first );
         // scale
         buf.writeNextBAShort( tileScale[ j ].second );
      }
   }

   // Send the copyright string.
   buf.writeNextString( "©2010" );

   // Write categories again, this time in utf-8 as intended.
   writeCategories( buf, false );

   // Write more information about the layers
   m_layerIDsAndDesc.save( buf );
  
   // The color of the text and horizon.

   buf.writeNextBALong( m_textColor );
   buf.writeNextBALong( m_topHorizonColor );
   buf.writeNextBALong( m_bottomHorizonColor );
  
   // Save the copyright holder.
   m_copyrightHolder.save( buf, m_drawSettingVersion );

   // Then more stuff can be written here if necessary   
   
   // This should be last
   {
      uint32 endPos = buf.getCurrentOffset();
      
      // Calculate CRC
      m_crc = MC2CRC32::crc32( buf.getBufferAddress(),
                               endPos );
      // Write in the CRC and other stuff that should not be included
      // in it.
      buf.reset();
      buf.readPastBytes( crcPOS );
      buf.writeNextBALong( m_crc );
      mc2dbg8 << "[STMFD]: Wrote crc " << MC2HEX( m_crc ) << " @ "
             << uint32(buf.getCurrentOffset() - 4) << endl;
      if ( timeStampPos != NULL ) {
         // Save the position of the time stamp for future updates.
         *timeStampPos = buf.getCurrentOffset();
      }
      buf.writeNextBALong( TimeUtility::getRealTime() / 60 );

      // And set the old offset.
      buf.reset();
      buf.readPastBytes( endPos );
   }
}

void
ServerTileMapFormatDesc::updateTimeStamp( BitBuffer& buf,
                                          uint32 timeStampPos )
{
   uint32 oldPos = buf.getCurrentOffset();

   // Set new position
   buf.reset();
   buf.readPastBytes( timeStampPos );
   
   buf.writeNextBALong( TimeUtility::getRealTime() / 60 );
   // Set back the position
   buf.reset();
   buf.readPastBytes( oldPos );
}

void
ServerTileMapFormatDesc::addThresholdDataToImportanceTable(
                  uint32 layerID,
                  float64 zoomFactor,
                  int nbrStepsPerDetailLevel,
                  const vector<pair<int, int> >& typeAndSqPixelThreshold )
{
   uint32 layerNbr = getLayerNbrFromID(  layerID );
   tileScale_t& tileScale = *m_tileScaleByLayer[ layerNbr ];
   TileImportanceTable* importanceTable = m_importanceTables[ layerNbr ];
   
   int detailLevel = 0;
   // Square zoomfactor
   float64 sqZoomFactor= zoomFactor * zoomFactor;
  
   // Allow everything when fully covering detaillevel 0.
   for ( vector<pair<int, int> >::const_iterator jt = 
            typeAndSqPixelThreshold.begin(); jt != 
         typeAndSqPixelThreshold.end(); ++jt ) {
      uint16 maxScale = tileScale.front().second;
      importanceTable->insert( make_pair( maxScale, 
               TileImportanceNotice( 0, // detaillevel,
                                     maxScale,
                                     jt->first, // feature type
                                     0 ) ) );     // threshold
   }
  
   vector< pair<int, uint16> >::iterator it = tileScale.begin();
   MC2_ASSERT( it != tileScale.end() );
   vector< pair<int, uint16> >::iterator nextIt = it;
   ++nextIt;
   
   while ( nextIt != tileScale.end() ) {

      for ( int i = 1; i <= nbrStepsPerDetailLevel; ++i ) {
         
         float64 percentIntoNextDetailLevel = 
            i / float64( nbrStepsPerDetailLevel  );
         uint16 scaleLevel = uint16( it->second +  
            (nextIt->second - it->second) * percentIntoNextDetailLevel );

         for ( vector<pair<int, int> >::const_iterator jt = 
                  typeAndSqPixelThreshold.begin(); jt != 
               typeAndSqPixelThreshold.end(); ++jt ) {
            
            // Square pixels on the screen.
            uint32 threshold = jt->second + 
               uint32( jt->second * ( sqZoomFactor - 1 ) * 
                percentIntoNextDetailLevel );
            
               importanceTable->insert( make_pair( scaleLevel,  
                  TileImportanceNotice( detailLevel,
                                        scaleLevel,
                                        jt->first, // feature type
                                        threshold ) ) );
         }
      }
      ++detailLevel;
      it = nextIt;
      ++nextIt;
   }
}

void
ServerTileMapFormatDesc::addStaticDataToImportanceTable(
               uint32 layerID,
               const vector<pair<int, uint16> >& typeAndScaleLevel )
{
   uint32 layerNbr = getLayerNbrFromID( layerID );
   TileImportanceTable* importanceTable = m_importanceTables[ layerNbr ];
   for ( vector<pair<int, uint16> >::const_iterator it = 
         typeAndScaleLevel.begin(); it != typeAndScaleLevel.end(); ++it ) {
      uint16 scale = it->second;
      int detailLevel = getDetailLevel( layerNbr, scale );
      // INFO: FIX DUE TO BUGGED CLIENTS.
      detailLevel = MAX( detailLevel, NBR_MAP_DETAILS ); 
      importanceTable->insert( make_pair( scale,
            TileImportanceNotice( detailLevel,
                                  scale,
                                  it->first,        // feature type
                                  MAX_UINT32 ) ) ); // no threshold 
   }
}

void
ServerTileMapFormatDesc::addImportanceTable( bool fetchStrings,
                                             uint32 layerID,
                                             const MC2SimpleString& descrStr,
                                             uint32 updatePeriodMin,
                                             bool optional,
                                             bool visibleByDefault,
                                             bool affectedByACPMode,
                                             bool fetchLayerWhenACPEnabled,
                                             bool fetchLayerWhenACPDisabled ) 
{
   m_importanceTables.push_back( new TileImportanceTable() );
   m_layerNbrByID[ layerID ] = getNbrLayers();
   // For compatibility. Probably not used in the server.
   m_layerIDsAndDescForComp.push_back( make_pair(layerID, descrStr) );
   // For new use.
   m_layerIDsAndDesc.push_back( TileMapLayerInfo( layerID,
                                                  descrStr,
                                                  updatePeriodMin,
                                                  optional,
                                                  visibleByDefault,
                                                  fetchStrings,
                                                  affectedByACPMode,
                                                  fetchLayerWhenACPEnabled,
                                                  fetchLayerWhenACPDisabled ) );
}

namespace {

/// Returns the area threshold for the ALL_AREA_FEATURES importance.
int allAreaThreshold( uint32 serverPrefix ) {
   return ( serverPrefix == STMFDParams::HIGH_END_SERVER_PREFIX ) ? 10 : 100;
}

}

void
ServerTileMapFormatDesc::initTilesAndImportances()
{ 
   // -- Add route layer and set the traffic settings
   tileSettings_t tileSettings;
   tileSettings_t trafficSettings;
   
   {
      tileSettings.m_meters = 420 * ROUTE_FACTOR;
      tileSettings.m_pixels = 200 * ROUTE_FACTOR;
      tileSettings.m_dpi = 72;
      tileSettings.m_zoomFactor = 3.0;
      tileSettings.m_exchangeTileFactor = 0.0;
      tileSettings.m_detailLevels = 7;
      trafficSettings = tileSettings;
      initTileSizesForLayer( m_tileSettingsByLayerNbr.size(),
                             tileSettings );
      m_tileSettingsByLayerNbr.push_back( tileSettings );
      
      vector<pair<int, uint16> > typeAndScaleLevel;    
      addImportanceTable( false, // fetchStrings by default
                          TileMapTypes::c_routeLayer, 
                          "The route",
                          0, // minutes
                          false, // not optional
                          // visible if the layer said so
                          m_layers & ( 1 << TileMapTypes::c_routeLayer ) );
      typeAndScaleLevel.clear();
      typeAndScaleLevel.push_back( 
         make_pair( ALL_ROUTE_FEATURES, MAX_TILE_SCALELEVEL ) ); 
      
      addStaticDataToImportanceTable( TileMapTypes::c_routeLayer,
                                      typeAndScaleLevel );
      // Build the matrix.
      m_importanceTables.back()->buildMatrix();
   }
      // -- End of route layer. 

   // default settings
   
   // We hope 15 bits will be enough for detail 0.
   tileSettings.m_meters = 420;    
   tileSettings.m_pixels = 200;     // Ideal on a 200 pixel screen
   tileSettings.m_dpi = 72;         // And 72 dpi.
   tileSettings.m_zoomFactor = 3.0; // Factor to next level of detail
   tileSettings.m_exchangeTileFactor = 0.0;
   tileSettings.m_detailLevels = NBR_MAP_DETAILS;

   // -- Add map layer.
   {
      initTileSizesForLayer( m_tileSettingsByLayerNbr.size(),
                             tileSettings );
      m_tileSettingsByLayerNbr.push_back( tileSettings );

      vector<pair<int, int> > typeAndSqPixelThreshold;
      vector<pair<int, uint16> > typeAndScaleLevel;
      addImportanceTable( true, // fetchStrings by default
                          TileMapTypes::c_mapLayer, 
                          "The map",
                          0, // minutes
                          false, // not optional
                          // visible if the layer said so
                          m_layers & ( 1 << TileMapTypes::c_mapLayer ) );

      typeAndSqPixelThreshold.push_back( 
         make_pair( ALL_AREA_FEATURES, 
                    allAreaThreshold( getServerPrefix() ) ) );
      
      addThresholdDataToImportanceTable(TileMapTypes::c_mapLayer,
                                        tileSettings.m_zoomFactor,
                                        2, // nbrStepsPerDetailLevel
                                        typeAndSqPixelThreshold );

      
//        typeAndScaleLevel.push_back( 
//            make_pair( ALL_OTHER_FEATURES, MAX_TILE_SCALELEVEL ) ); // Always
      
      typeAndScaleLevel.push_back( 
                                  make_pair( LAND_AND_MAJOR_CITYCENTRES, MAX_TILE_SCALELEVEL ) ); // Always
      typeAndScaleLevel.push_back( 
         make_pair( STREET_MAIN_AND_MEDIUM_CITYCENTRES_AND_AIRPORTS, 1400 ) ); // Changed from 1000
      typeAndScaleLevel.push_back( 
         make_pair( STREET_FIRST_AND_MINOR_CITYCENTRES, 130 ) ); // was 110
      typeAndScaleLevel.push_back( 
         make_pair( STREET_SECOND_AND_FERRY, 44 ) );  // Changed from 50
      typeAndScaleLevel.push_back( 
         make_pair( STREET_THIRD_AND_MORE, 18 ) ); // Changed from 33
//   typeAndScaleLevel.push_back( 
//         make_pair( GfxFeature::RAILWAY, 12 ) );
      typeAndScaleLevel.push_back( 
         make_pair( STREET_FOURTH_AND_RAILWAY, 6 ) ); // Changed from 10.
      
      addStaticDataToImportanceTable( TileMapTypes::c_mapLayer,
                                      typeAndScaleLevel );
      // Build the matrix.
      m_importanceTables.back()->buildMatrix();
   }
   // -- End of map layer

   
   // -- Add poi layer.
   {
      // Poi layer contains same settings as the map layer.
      initTileSizesForLayer( m_tileSettingsByLayerNbr.size(),
                             tileSettings );
      m_tileSettingsByLayerNbr.push_back( tileSettings );
      
      vector<pair<int, uint16> > typeAndScaleLevel;
      // If the user has such rights so that the pois
      // should be visible when acp enabled. (I.e. ACP doesn't replace POIs).
      bool poisVisibleAlsoWhenACPIsEnabled = m_layers & ( 1 << TileMapTypes::c_poiLayer);
      addImportanceTable( false, // fetchStrings by default
                          TileMapTypes::c_poiLayer, 
                          "The POI:s",
                          0, // minutes
                          false, // not optional
                          poisVisibleAlsoWhenACPIsEnabled, // Visible if ACP doesn't replace POIs 
                          true, // affectedByACPMode
                          poisVisibleAlsoWhenACPIsEnabled, // fetchLayerWhenACPEnabled
                          true ); // fetchLayerWhenACPDisabled
      typeAndScaleLevel.clear();
      typeAndScaleLevel.push_back( 
         make_pair( POIS, 41 ) ); 
      
      addStaticDataToImportanceTable( TileMapTypes::c_poiLayer,
                                      typeAndScaleLevel );
      // Build the matrix.
      m_importanceTables.back()->buildMatrix();
   }
   // -- End of poi layer
   
   // -- Add traffic layer.
   {
      // Poi layer contains same settings as the map layer.
      initTileSizesForLayer( m_tileSettingsByLayerNbr.size(),
                             trafficSettings );
      m_tileSettingsByLayerNbr.push_back( trafficSettings );
      
      vector<pair<int, uint16> > typeAndScaleLevel;
      addImportanceTable( false, // fetchStrings by default
                          TileMapTypes::c_trafficLayer,
                          "Traffic",
                          10,      // minutes
                          true,    // optional
                          // visible if the layer said so
                          m_layers & ( 1 << TileMapTypes::c_trafficLayer ) ); 
      typeAndScaleLevel.clear();
      typeAndScaleLevel.push_back( 
         make_pair( TRAFFIC, 41 ) ); 
      
      addStaticDataToImportanceTable( TileMapTypes::c_trafficLayer,
                                      typeAndScaleLevel );
      // Build the matrix.
      m_importanceTables.back()->buildMatrix();
   }
   // -- End of normal poi layer

   // -- Add ACP poi layer. Same settings as normal POI layer
   {
      // Poi layer contains same settings as the map layer.
      initTileSizesForLayer( m_tileSettingsByLayerNbr.size(),
                             tileSettings );
      m_tileSettingsByLayerNbr.push_back( tileSettings );
      
      vector<pair<int, uint16> > typeAndScaleLevel;
      bool visible = m_layers & ( 1 << TileMapTypes::c_acpLayer );
      addImportanceTable( false, // fetchStrings by default
                          TileMapTypes::c_acpLayer, 
                          "The ACP POI:s",
                          0,
                          true, // optional
                          // visible if the layer said so
                          visible,
                          true, // affectedByACPMode
                          visible, // fetchLayerWhenACPEnabled
                          false ); // fetchLayerWhenACPDisabled
      typeAndScaleLevel.clear();
      typeAndScaleLevel.push_back( 
         make_pair( POIS, 41 ) ); 
      
      addStaticDataToImportanceTable( TileMapTypes::c_acpLayer,
                                      typeAndScaleLevel );
      // Build the matrix.
      m_importanceTables.back()->buildMatrix();
   }
   // -- End of ACP poi layer

   // Setup Event layer
   {
      tileSettings.m_meters = 420 * ROUTE_FACTOR;
      tileSettings.m_pixels = 200 * ROUTE_FACTOR;
      // Event Poi layer contains same settings as the map layer.
      initTileSizesForLayer( m_tileSettingsByLayerNbr.size(),
                             tileSettings );
      m_tileSettingsByLayerNbr.push_back( tileSettings );

      vector<pair<int, uint16> > typeAndScaleLevel;
      bool visible = m_layers & ( 1 << TileMapTypes::c_eventLayer );
      addImportanceTable( false, // fetchStrings by default
                          TileMapTypes::c_eventLayer,
                          "Events",
                          120, // 2 hours update time
                          true, // optional
                          // visible if the layer said so
                          visible,
                          false, // not affected by acp mode
                          visible, // fetchLayerWhenACPEnabled
                          false ); // fetchLayerWhenACPDisabled
      typeAndScaleLevel.clear();
      typeAndScaleLevel.push_back(
         make_pair( POIS, 41 ) );

      addStaticDataToImportanceTable( TileMapTypes::c_eventLayer,
                                      typeAndScaleLevel );
      // Build the matrix.
      m_importanceTables.back()->buildMatrix();
   }
   // -- End of EventLayer poi layer
}

void 
ServerTileMapFormatDesc::addDefaultScaleRange( int16 type, 
                                        vector<TileFeatureArg*>& args )
{
   scaleRangeByType_t::const_iterator it = m_scaleRangeByType.find( type );
   if ( it != m_scaleRangeByType.end() ) {
      // There's a default scale range for this feature type.
      const scaleRange_t& range = it->second;

      // Min scale.
      SimpleArg* minScale = new SimpleArg(TileArgNames::min_scale, 16);
      minScale->setValue( range.first );
      args.push_back( minScale );
      
      // Max scale.
      SimpleArg* maxScale = new SimpleArg(TileArgNames::max_scale, 16);
      maxScale->setValue( range.second );
      args.push_back( maxScale );
   }
}

void
ServerTileMapFormatDesc::buildArgsByTypeArray()
{
   // Get largest feature type.
   const featureInt lastFeatureType =
      (*m_argsByTileFeatureType.rbegin()).first;      
   // The size
   const featureInt size = lastFeatureType + 1;
   
   // Allocate the arrays
   delete [] m_argsByTileFeatureTypeArray;
   m_argsByTileFeatureTypeArraySize = size;
   m_argsByTileFeatureTypeArray =
      new vector<TileFeatureArg*>[ m_argsByTileFeatureTypeArraySize ];
   
   // Build the vector used when getting args for features.
   for ( argsByType_t::const_iterator it = m_argsByTileFeatureType.begin();
         it != m_argsByTileFeatureType.end();
         ++it ) {
      m_argsByTileFeatureTypeArray[ it->first ] = it->second;
   }
}


void 
ServerTileMapFormatDesc::setData()
{  

   initTilesAndImportances();
   
   m_argsPerType.clear();

   delete [] m_primitiveDefaultMap;
   m_primitiveDefaultMapSize = 0;
   
//   // Set background colour to land-color.
   if ( m_drawSettingVersion == 0 ) {
      m_backgroundColor = 0xfcefaa;
   } else {
      m_backgroundColor = ::makeColorRGB( 225, 225, 218 );
   }
   if ( m_nightMode ) {
      m_backgroundColor = 0x000000;
   }
   // Create the POI tables.
   createPOITables();
 
   // Create the settings for all features.
   createDefaultSettings();
   
   // Build m_defaultArgs.
   // Go through all args for the primitives and add them to 
   // m_defaultArgs and replace them with the arg from m_defaultArgs.
   for ( int i = 0; i < m_primitiveDefaultMapSize; ++i ) {
      
      primitiveVector_t& vec = m_primitiveDefaultMap[i];
      for ( primitiveVector_t::iterator jt = vec.begin();
            jt != vec.end(); ++jt ) {
         for ( TilePrimitiveFeature::arg_iterator kt = (*jt)->arg_begin();
               kt != (*jt)->arg_end(); ++kt ) {
            // Add to m_defaultArgs.
            int argIndex = m_defaultArgs.addArg( *kt );
            // Set the new argument from m_defaultArgs.
            *kt = m_defaultArgs.getArg( argIndex );
         }
      }
   }

   buildArgsByTypeArray();

   createCategories( m_lang );

}

const TileImportanceNotice* 
ServerTileMapFormatDesc::getImportanceOfType( uint32 layerID,
                                              uint16 type ) const
{ 
   uint32 layerNbr = getLayerNbrFromID( layerID );
   return m_importanceTables[ layerNbr ]->getFirstOfType( type );
}

void
ServerTileMapFormatDesc::createArgVectorForType( int type)
{
   vector<TileFeatureArg*> argVector;
   // Coord type.
   switch ( type ) {
      case ( TileFeature::street_class_0 ) :
      case ( TileFeature::street_class_0_ramp ) :
      case ( TileFeature::street_class_0_level_0 ) :
      case ( TileFeature::street_class_1 ) :
      case ( TileFeature::street_class_1_ramp ) :
      case ( TileFeature::street_class_1_level_0 ) :
      case ( TileFeature::street_class_2 ) :
      case ( TileFeature::street_class_2_ramp ) :
      case ( TileFeature::street_class_2_level_0 ) :
      case ( TileFeature::street_class_3 ) :
      case ( TileFeature::street_class_3_level_0 ) :
      case ( TileFeature::street_class_4 ) :
      case ( TileFeature::street_class_4_level_0 ) :
      case ( TileFeature::route ) :
      case ( TileFeature::water ) :
      case ( TileFeature::ocean ) :
      case ( TileFeature::park ) :
      case ( TileFeature::land ) :
      case ( TileFeature::island ) :
      case ( TileFeature::building_area ) :
      case ( TileFeature::building_outline ) :
      case ( TileFeature::bua ) :
      case ( TileFeature::railway ) :
      case ( TileFeature::border ) :
      case ( TileFeature::airport_ground ) :
      case ( TileFeature::cartographic_green_area ) :
      case ( TileFeature::cartographic_ground ) :
      case ( TileFeature::forest ) :
      case ( TileFeature::aircraftroad ) :
      case ( TileFeature::walkway ) :
      case ( TileFeature::water_in_park ) :
      case ( TileFeature::island_in_bua ) :
      case ( TileFeature::island_in_water_in_park ) :
      case ( TileFeature::bua_on_island ) :
         // We cannot add coordinates to these args, since map is null.
         argVector.push_back( new CoordsArg( &m_coords,
                                             TileArgNames::coords ) );
         break;

      default: 
         break;
   }

   // Check if it contains an image and therefore should have a coord arg.
   imageByType_t::const_iterator it = m_imageByTileFeatureType.find( type );
   if ( it != m_imageByTileFeatureType.end() ) {
      argVector.push_back( new CoordArg( TileArgNames::coord ) );
      // If no valid image name is present, then the image name should
      // be added.
      if ( it->second == "" &&
           // Event features should not have any images
           type != TileFeature::event ) {
         StringArg* stringArg = new StringArg( TileArgNames::image_name );
         stringArg->setValue( "" ); // saveFullArgs requires a value.
         argVector.push_back( stringArg );
      }
   }
   
   // Special for ssi, and special custom pois
   switch ( type ) {
      case TileFeature::street_class_0:
      case TileFeature::street_class_0_ramp:
      case TileFeature::street_class_1:
      case TileFeature::street_class_1_ramp:
      case TileFeature::street_class_2:
      case TileFeature::street_class_2_ramp:
      case TileFeature::street_class_3:
      case TileFeature::street_class_4:
      case TileFeature::walkway: {
         // Level 0
         SimpleArg* arg = new SimpleArg( TileArgNames::level, 4 );
         arg->setValue( 0 ); // saveFullArgs requires a value.
         argVector.push_back( arg );
         // Level 1
         arg = new SimpleArg( TileArgNames::level_1, 4 );
         arg->setValue( 0 ); // saveFullArgs requires a value.
         argVector.push_back( arg );
         } break;
      case TileFeature::event: {
         // Event features has time and external id arguments.
         SimpleArg* arg = new SimpleArg( TileArgNames::time, 31 );
         argVector.push_back( arg );
         arg = new SimpleArg( TileArgNames::ext_id, 16 );
         argVector.push_back( arg );
         arg = new SimpleArg( TileArgNames::duration, 16 );
         argVector.push_back( arg );
      } break;
      case TileFeature::special_custom_poi_2:
      case TileFeature::special_custom_poi_4:         
      case TileFeature::special_custom_poi_7:
      case TileFeature::special_custom_poi_10:
      case TileFeature::special_custom_poi_15:         
      case TileFeature::special_custom_poi_20:
      case TileFeature::special_custom_poi_30: {
         // real_feature_type for special custom pois
         SimpleArg* arg = new SimpleArg( TileArgNames::real_feature_type, 8 );
         arg->setValue( TileFeature::nbr_tile_features );
         argVector.push_back( arg );
      }  break;
      default:
         break;
   }

   
   m_argsByTileFeatureType.insert( make_pair( type, argVector ) );
   argsByName_t* argsByName = new argsByName_t();
   for ( vector<TileFeatureArg*>::const_iterator jt = 
            argVector.begin(); jt != argVector.end(); ++jt ) {
         argsByName->insert( make_pair( (*jt)->getName(), *jt ) );
   }  
   m_argsByTypeAndName.insert( make_pair( type, argsByName ) );
}


int16 
ServerTileMapFormatDesc::getStreetTileTypeFromGfxFeature( 
                                       const GfxFeature* gfxFeature,
                                       byte roadClass ) const 
{
   int16 type = -1;
   GfxRoadPolygon* poly = 
      static_cast<GfxRoadPolygon*> ( gfxFeature->getPolygon( 0 ) );

   bool useDefaultLevel = true;
   if ( ( poly->getLevel0() == 0 ) &&
         ( poly->getLevel1() == 0 ) ) {
      useDefaultLevel = false;
   }
   bool isRamp = poly->isRamp();

   switch ( roadClass ) {
      case ( 0 ) :
         if ( isRamp ) {
            type = TileFeature::street_class_0_ramp;
         }
         else if ( useDefaultLevel ) {
            type = TileFeature::street_class_0;
         } else {
            type = TileFeature::street_class_0_level_0;
         }
         break;
      case ( 1 ) :
         if ( isRamp ) {
            type = TileFeature::street_class_1_ramp;
         }
         else if ( useDefaultLevel ) {
            type = TileFeature::street_class_1;
         } else {
            type = TileFeature::street_class_1_level_0;
         }
         break;
      case ( 2 ) :
         if ( isRamp ) {
            type = TileFeature::street_class_2_ramp;
         }
         else if ( useDefaultLevel ) {
            type = TileFeature::street_class_2;
         } else {
            type = TileFeature::street_class_2_level_0;
         }
         break;
      case ( 3 ) :
         if ( useDefaultLevel ) {
            type = TileFeature::street_class_3;
         } else {
            type = TileFeature::street_class_3_level_0;
         }
         break;
      case ( 4 ) :
         if ( useDefaultLevel ) {
            type = TileFeature::street_class_4;
         } else {
            type = TileFeature::street_class_4_level_0;
         }
         break;
      default :
         break;
   }
   return type;
}

int16 tileFeatureTypeFromPOI( const GfxPOIFeature* poiFeature,
                              const map<ItemTypes::pointOfInterest_t, int>&
                              tileFeatureTypeByPOIType ) {
   typedef map<ItemTypes::pointOfInterest_t, int>  typeByPOI_t;       
   if ( ! poiFeature->getCustomImageName().empty() ) {

      byte scaleLevel = poiFeature->getExtraInfo();

      // ugly hack starts here
      if ( scaleLevel & ServerTileMapFormatDesc::SPECIAL_CUSTOM_POI_MASK ) {
         // strip the special bit
         scaleLevel ^= ServerTileMapFormatDesc::SPECIAL_CUSTOM_POI_MASK;
         if ( scaleLevel <= 2 ) {
            return TileFeature::special_custom_poi_2;
         } else if ( scaleLevel <= 4 ) {
            return TileFeature::special_custom_poi_4;
         } else if ( scaleLevel <= 7 ) {
            return TileFeature::special_custom_poi_7;
         } else if ( scaleLevel <= 10 ) {
            return TileFeature::special_custom_poi_10;
         } else if ( scaleLevel <= 15 ) {
            return TileFeature::special_custom_poi_15;
         } else if ( scaleLevel <= 20 ) {
            return TileFeature::special_custom_poi_20;
         } else {
            return TileFeature::special_custom_poi_30;
         }
      }
      
      // Old Custom poi.
      // Check the scale level that is stored in the extra info.
      
      if ( scaleLevel <= 2 ) {
         return TileFeature::custom_poi_2;
      } else if ( scaleLevel <= 4 ) {
         return TileFeature::custom_poi_4;
      } else if ( scaleLevel <= 7 ) {
         return TileFeature::custom_poi_7;
      } else if ( scaleLevel <= 10 ) {
         return TileFeature::custom_poi_10;
      } else if ( scaleLevel <= 15 ) {
         return TileFeature::custom_poi_15;
      } else if ( scaleLevel <= 20 ) {
         return TileFeature::custom_poi_20;
      } else {
         return TileFeature::custom_poi_30;
      }
   }
   
   // 
   if ( ( poiFeature->getPOIType() == ItemTypes::hospital ) &&
        ( poiFeature->getExtraInfo() == 1 ) ) {
      // Special for Turkey
      return TileFeature::turkish_hospital;
   }
   
   typeByPOI_t::const_iterator it = 
      tileFeatureTypeByPOIType.find( poiFeature->getPOIType() );
   
   if ( it != tileFeatureTypeByPOIType.end() ) {
      return it->second;
   } else {
      // It wasn't a POI that can be 
      // directly translated to tilefeature type.
      if ( poiFeature->getPOIType() == ItemTypes::cityCentre ) {
         switch ( poiFeature->getExtraInfo() ) {
         case ( 2 ) : 
            return TileFeature::city_centre_2;
         case ( 3 ) : // US. 2-5 mill.
         case ( 4 ) :
            return TileFeature::city_centre_4;
         case ( 5 ) :
         case ( 6 ) : // US.
            return TileFeature::city_centre_5;
         case ( 7 ) :
            return TileFeature::city_centre_7;
         case ( 8 ) :
            return TileFeature::city_centre_8;
         case ( 9 ) : // US.
         case ( 10 ) :
            return TileFeature::city_centre_10;
         case ( 11 ) :
            return TileFeature::city_centre_11;
         case ( 12 ) :
            return TileFeature::city_centre_12;
         default:
            break;
         }
      } else if ( poiFeature->getPOIType() == ItemTypes::petrolStation ){
         return TileFeature::petrol_station;
      }
   }
   // unknown poi
   return -1;
}

int16 
ServerTileMapFormatDesc::getTileFeatureTypeFromGfxFeature( 
                                     const GfxFeature* gfxFeature ) const
{
   // This function is also used when creating the SimplePoiMaps.
   int16 tileFeatureType = -1;

   // Require geometry.
   if ( gfxFeature->getNbrPolygons() == 0 ) {
      return tileFeatureType;
   }

   switch ( gfxFeature->getType() ) {

      case ( GfxFeature::STREET_MAIN ) :
         return getStreetTileTypeFromGfxFeature( gfxFeature, 0 );
         break;

      case ( GfxFeature::STREET_FIRST ) :
         return getStreetTileTypeFromGfxFeature( gfxFeature, 1 );
         break;

      case ( GfxFeature::STREET_SECOND ) :
         return getStreetTileTypeFromGfxFeature( gfxFeature, 2 );
         break;

      case ( GfxFeature::STREET_THIRD ) :
         return getStreetTileTypeFromGfxFeature( gfxFeature, 3 );
         break;

      case ( GfxFeature::STREET_FOURTH ) :
         return getStreetTileTypeFromGfxFeature( gfxFeature, 4 );
         break;

      case ( GfxFeature::ROUTE ) :
         return TileFeature::route;
         break;

      case ( GfxFeature::ROUTE_ORIGIN ) :
         return TileFeature::route_origin;
         break;

      case ( GfxFeature::ROUTE_DESTINATION ) :
         return TileFeature::route_destination;
         break;

      case ( GfxFeature::PARK_CAR ) :
         return TileFeature::route_park_car;
         break;

      case ( GfxFeature::POI ) :
         tileFeatureType =
            tileFeatureTypeFromPOI( static_cast< const GfxPOIFeature* >
                                    ( gfxFeature ),
                                    m_tileFeatureTypeByPOIType );
         break;

      case ( GfxFeature::EVENT ):
         tileFeatureType = TileFeature::event;
         break;
      case ( GfxFeature::WATER ) :
         //         case ( GfxFeature::WATER_LINE ) :
         tileFeatureType = TileFeature::water;
         break;

      case ( GfxFeature::PARK ) :
         tileFeatureType = TileFeature::park;
         break;

      case ( GfxFeature::LAND ) :
         tileFeatureType = TileFeature::land;
         break;

      case ( GfxFeature::ISLAND ) :
         tileFeatureType = TileFeature::island;
         break;

      case ( GfxFeature::BUILDING ) :
         tileFeatureType = TileFeature::building_area;
         break;
      case ( GfxFeature::INDIVIDUALBUILDING ) :
         tileFeatureType = TileFeature::building_outline;
         break;

      case ( GfxFeature::BUILTUP_AREA ) :
         tileFeatureType = TileFeature::bua;
         break;

      case ( GfxFeature::RAILWAY ) :
         tileFeatureType = TileFeature::railway;
         break;

      case ( GfxFeature::OCEAN ) :
         tileFeatureType = TileFeature::ocean;
         break;

      case ( GfxFeature::BORDER ) :
         tileFeatureType = TileFeature::border;
         break;
         
      case ( GfxFeature::AIRPORTGROUND ) :
         tileFeatureType = TileFeature::airport_ground;
         break;
         
      case ( GfxFeature::CARTOGRAPHIC_GREEN_AREA ) :
         tileFeatureType = TileFeature::cartographic_green_area;
         break;
         
      case ( GfxFeature::CARTOGRAPHIC_GROUND ) :
         tileFeatureType = TileFeature::cartographic_ground;
         break;
         
      case ( GfxFeature::FOREST ) :
         tileFeatureType = TileFeature::forest;
         break;
         
      case ( GfxFeature::AIRCRAFTROAD ) :
         tileFeatureType = TileFeature::aircraftroad;
         break;

      case GfxFeature::WALKWAY:
         tileFeatureType = TileFeature::walkway;
         break;

      case GfxFeature::WATER_IN_PARK:
         tileFeatureType = TileFeature::water_in_park;
         break;

      case GfxFeature::ISLAND_IN_BUA:
         tileFeatureType = TileFeature::island_in_bua;
         break;

      case GfxFeature::ISLAND_IN_WATER_IN_PARK:
      case GfxFeature::ISLAND_IN_WATER_IN_PARK_ISLAND:
      case GfxFeature::ISLAND_IN_WATER_IN_PARK_BUA:
         tileFeatureType = TileFeature::island_in_water_in_park;
         break;
         
      case GfxFeature::BUA_ON_ISLAND:
         tileFeatureType = TileFeature::bua_on_island;
         break;
         
      case GfxFeature::TRAFFIC_INFO: {
         const GfxTrafficInfoFeature* trafficFeat =
            static_cast<const GfxTrafficInfoFeature*>(gfxFeature);
         // Add more types here.
         switch ( trafficFeat->getTrafficInfoType() ) {
#if 1
            case TrafficDataTypes::Camera:
               tileFeatureType = TileFeature::speed_camera;
               break;
            case TrafficDataTypes::UserDefinedCamera:
               tileFeatureType = TileFeature::user_defined_speed_camera;
               break;
            case TrafficDataTypes::RoadWorks:
               tileFeatureType = TileFeature::roadwork;
               break;
            case TrafficDataTypes::MaintenanceWorks:
               tileFeatureType = TileFeature::roadwork;
               break;
            case TrafficDataTypes::ConstructionWorks:
               tileFeatureType = TileFeature::roadwork;
               break;
            case TrafficDataTypes::SpeedTrap:
               tileFeatureType = TileFeature::speed_trap;
               break;
#endif
            default:
               tileFeatureType = TileFeature::traffic_default;
               break;
         }
      }
      break;

      default:
         break;

   }

   return tileFeatureType;
}

bool ServerTileMapFormatDesc::useAreaThresholdForType( 
   GfxFeature::gfxFeatureType featureType ) const {
   return true;
   // currently used for all types regardless of device
/*   if ( !highEndDevice() ) {
      return true;
   }
   else {
      return featureType != GfxFeature::BUILDING &&
             featureType != GfxFeature::INDIVIDUALBUILDING;
             }*/
}

bool 
ServerTileMapFormatDesc::isGfxOfImportance( 
                           const GfxFeature* gfxFeature,
                           const TileImportanceNotice* importance,
                           float64 sqMeterToSqPixelFactor,
                           const TileMapParams& param ) const
{
   bool sameType = false;
   uint16 baseImportance = importance->getType();
   
   if ( baseImportance < GfxFeature::NBR_GFXFEATURES ) {
      if ( baseImportance == gfxFeature->getType() ) {
         sameType = true;
      }
   } else {
     
      switch ( baseImportance ) {
         case ( ALL_AREA_FEATURES ) :
            switch ( gfxFeature->getType() ) {
               case ( GfxFeature::BUILTUP_AREA ) :
               case ( GfxFeature::ISLAND ) :
               case ( GfxFeature::WATER ) :
               case ( GfxFeature::PARK ) :
               case ( GfxFeature::BUILDING ) :
               case ( GfxFeature::INDIVIDUALBUILDING ) :
               case ( GfxFeature::AIRPORTGROUND ) :
               case ( GfxFeature::CARTOGRAPHIC_GREEN_AREA ) :
               case ( GfxFeature::CARTOGRAPHIC_GROUND ) :
               case ( GfxFeature::FOREST ) :
               case ( GfxFeature::AIRCRAFTROAD ) :
               case GfxFeature::WATER_IN_PARK:
               case GfxFeature::ISLAND_IN_BUA:
               case GfxFeature::WALKWAY:
               case GfxFeature::RAILWAY:
               case GfxFeature::ISLAND_IN_WATER_IN_PARK:
               case GfxFeature::ISLAND_IN_WATER_IN_PARK_BUA:
               case GfxFeature::ISLAND_IN_WATER_IN_PARK_ISLAND:
               case GfxFeature::BUA_ON_ISLAND:
                  sameType = true;
                  break;
               default:
                  break;
            } break;
         case ( ALL_OTHER_FEATURES ) :
            switch ( gfxFeature->getType() ) {
               case ( GfxFeature::LAND ) :
               case ( GfxFeature::STREET_MAIN ) : 
               case ( GfxFeature::STREET_FIRST ) :
               case ( GfxFeature::STREET_SECOND ) :
               case ( GfxFeature::STREET_THIRD ) :
               case ( GfxFeature::STREET_FOURTH ) :
               case ( GfxFeature::BORDER ) :
               case ( GfxFeature::WALKWAY ) :
               case GfxFeature::RAILWAY:
                  sameType = true;
                  break;
               default:
                  break;
            } break;
         
         case ( STREET_SECOND_AND_FERRY ) :
            switch ( gfxFeature->getType() ) {
               case ( GfxFeature::STREET_SECOND ) :
               case ( GfxFeature::FERRY ) :
               case GfxFeature::RAILWAY:
                  sameType = true;
                  break;
/*               case GfxFeature::STREET_FOURTH:
                 sameType = highEndDevice();*/
               default:
                  break;
            } break;
         
         case ( STREET_FOURTH_AND_RAILWAY ) :
            switch ( gfxFeature->getType() ) {
               case ( GfxFeature::STREET_FOURTH ) :
               case ( GfxFeature::RAILWAY ) :
               case GfxFeature::WALKWAY:
                  sameType = true;
                  break;
               default:
                  break;
            } break;
         
         case ( ALL_ROUTE_FEATURES ) :
            switch ( gfxFeature->getType() ) {
               case ( GfxFeature::ROUTE ) :
               case ( GfxFeature::ROUTE_CONTINUATION ) :
               case ( GfxFeature::ROUTE_ORIGIN ) :
               case ( GfxFeature::ROUTE_DESTINATION ) :
               case ( GfxFeature::PARK_CAR ) :
               case ( GfxFeature::STREET_MAIN ) :
               case ( GfxFeature::STREET_FIRST ) :
               case ( GfxFeature::STREET_SECOND ) :
               case ( GfxFeature::STREET_THIRD ) :
               case ( GfxFeature::STREET_FOURTH ) :
                  sameType = true;
                  break;
               default:
                  break;
            } break;
         
         case ( LAND_AND_MAJOR_CITYCENTRES ) :
            switch ( gfxFeature->getType() ) {
               // Land will be added anyway.
//               case ( GfxFeature::LAND ) :
//                  sameType = true;
//                  break;
               case GfxFeature::RAILWAY:
               case ( GfxFeature::BORDER ) :
                  sameType = true;
                  break;
               case ( GfxFeature::POI ) : {
                  const GfxPOIFeature* gfxPOI = 
                     static_cast<const GfxPOIFeature*> ( gfxFeature );
                  if ( gfxPOI->getPOIType() == ItemTypes::cityCentre ) {
                     if ( gfxPOI->getExtraInfo() < 8 ) {
                        sameType = true;
                     }
                  }
               } break;
               default:
                  break;
            }
            break;

         case ( STREET_MAIN_AND_MEDIUM_CITYCENTRES_AND_AIRPORTS ) :
            switch ( gfxFeature->getType() ) {
               case GfxFeature::RAILWAY:
               case ( GfxFeature::STREET_MAIN ) :
                  sameType = true;
                  break; 
               case ( GfxFeature::STREET_FIRST ) :
                  sameType = true;
                  break;
               case ( GfxFeature::POI ) : {
                  const GfxPOIFeature* gfxPOI = 
                     static_cast<const GfxPOIFeature*> ( gfxFeature );
                  if ( gfxPOI->getPOIType() == ItemTypes::cityCentre ) {
                     if ( gfxPOI->getExtraInfo() >= 8 &&
                          gfxPOI->getExtraInfo() < 11 ) {
                        sameType = true;
                     } 
                  } else if ( gfxPOI->getPOIType() == ItemTypes::airport ) {
                     sameType = true;
                  }
               } break;
               default:
                  break;
            }
            break;
         
         case ( STREET_FIRST_AND_MINOR_CITYCENTRES ) :
            switch ( gfxFeature->getType() ) {
            case ( GfxFeature::RAILWAY ) :
               sameType = true;
               break;

               /* case ( GfxFeature::STREET_FIRST ) :
                  sameType = true;
                  break;*/
               case ( GfxFeature::POI ) : {
                  const GfxPOIFeature* gfxPOI = 
                     static_cast<const GfxPOIFeature*> ( gfxFeature );
                  if ( gfxPOI->getPOIType() == ItemTypes::cityCentre ) {
                     if ( gfxPOI->getExtraInfo() >= 11 ) {
                        sameType = true;
                     } 
                  }
               } break;
               default:
                  break;
            }
            break;
         case ( POIS ) :
            // Events should be shown
            if ( gfxFeature->getType() == GfxFeature::EVENT ) {
               sameType = true;
            }

            if ( gfxFeature->getType() == GfxFeature::POI ) {
               const GfxPOIFeature* gfxPOI = 
                  static_cast<const GfxPOIFeature*> ( gfxFeature );
               switch ( gfxPOI->getPOIType() ) {
                  case ( ItemTypes::ferryTerminal ) :
                  case ( ItemTypes::skiResort ) :
                  case ( ItemTypes::railwayStation ) :
                  case ( ItemTypes::busStation ) :
                  case ( ItemTypes::tramStation ) :
                  case ( ItemTypes::commuterRailStation ) :
                  case ( ItemTypes::subwayStation ) :
                  case ( ItemTypes::golfCourse ) :
                  case ( ItemTypes::restaurant ) :
                  case ( ItemTypes::nightlife ) :
                  case ( ItemTypes::cafe ) :
                  case ( ItemTypes::cinema ) :
                  case ( ItemTypes::parkingGarage ) :
                  case ( ItemTypes::openParkingArea ) :
                  case ( ItemTypes::parkAndRide ) :
                  case ( ItemTypes::petrolStation ) :
                  case ( ItemTypes::historicalMonument ) :
                  case ( ItemTypes::theatre ) :
                  case ( ItemTypes::touristAttraction ) :
                  case ( ItemTypes::touristOffice ) :
                  case ( ItemTypes::casino ) :
//                  case ( ItemTypes::placeOfWorship ) : // add again if
//                                                       // we have a general
//                                                       // pow-image
                  case ( ItemTypes::wlan ) :
                  case ( ItemTypes::church ) :
                  case ( ItemTypes::mosque ) :
                  case ( ItemTypes::synagogue ) :
                  case ( ItemTypes::hinduTemple ) :
                  case ( ItemTypes::buddhistSite ) :
                     sameType = true;
                     break;
                  case ( ItemTypes::postOffice ): // post office
                  case ( ItemTypes::company ) :
                  case ( ItemTypes::shop ) :
                     sameType =
                        getTileFeatureTypeFromGfxFeature( gfxFeature ) != -1;
                     break;
                  case ( ItemTypes::hospital ) :
                     // There are hospitals and turkish hospitals
                     sameType =
                        getTileFeatureTypeFromGfxFeature( gfxFeature ) != -1;
                     break;
                  case ( ItemTypes::hotel ) :
                     sameType =
                        getTileFeatureTypeFromGfxFeature( gfxFeature ) != -1;
                     break;
                  default:
                     break;
               }
            }
            break;
            
         case TRAFFIC:
            // First check if it is traffic
            sameType = gfxFeature->getType() == GfxFeature::TRAFFIC_INFO ;
            // Then check if it is a traffic that we have a symbol for.
            sameType = sameType &&
               getTileFeatureTypeFromGfxFeature( gfxFeature ) != -1;
            break;
         default:
            break;
      }
   }

   if ( sameType ) {
     
      
      // Check tile type.
      int tileType = getTileFeatureTypeFromGfxFeature( gfxFeature );
      if ( tileType < 0 ) {
         // Did not result in a desired tile feature type.
         // This probably means that something strange has occured.
         // (such as no display class for a city center.)
         mc2dbg8 << warn << "[STMFD]:isGfxOfImportance:  skipping " 
                 << gfxFeature->getName() << endl;
         return false;
      }

      // Check scale range.
      if ( ! checkFeatScaleRange( tileType, param  ) ) {
         // If the scale is not OK,
         // then the feature is not of this importance.
         return false;
      }

      if ( useAreaThresholdForType( gfxFeature->getType() ) &&
           importance->getThreshold() != MAX_UINT32 ) {
         // Calculate threshold for gfxFeature.
         if ( gfxFeature->getNbrPolygons() > 0 ) {
            float64 area = 0;
            // Sum up the area for all polygons. They should be for the
            // same item, e.g. London.
            for ( int i = 0; i < int(gfxFeature->getNbrPolygons()); ++i ) {
               GfxPolygon* poly = gfxFeature->getPolygon( i );            
               area += poly->getArea();
            }
            mc2dbg8 << "FI: area = " << area << endl;
            // Convert area from mc2^2 to square pixels.
            double pixelArea = area * sqMeterToSqPixelFactor;
            mc2dbg8 << "FI: pixelArea = " << pixelArea 
                    << endl;
            
            mc2dbg8 << "FI: threshold = " << importance->getThreshold() 
                    << endl; 
            
            // Check if any other thresholds are present in this 
            // detaillevel. Use this as maxThreshold in that case.
            uint32 maxThreshold = MAX_UINT32;
            uint32 layerNbr = getLayerNbrFromID( param.getLayer() );
            for ( int i = 0; i < param.getImportanceNbr(); ++i ) {
               const TileImportanceNotice* notice = 
                  m_importanceTables[ layerNbr ]->getImportanceNbr( 
                        i, param.getDetailLevel() );
               if ( ! notice->fixedScale() ) {
                  // Update maxthreshold.
                  maxThreshold = notice->getThreshold();
               }
            }

            if ( (uint32) pixelArea >= importance->getThreshold() &&
                 (uint32) pixelArea < maxThreshold ) {
               return true;
            } else {
               if ( (uint32) pixelArea >= maxThreshold ) {
                  mc2dbg8 << "FI: Feature removed. Should have been added "
                     << "before.  pixelArea >= maxThreshold : " << pixelArea
                     << " >= " << maxThreshold 
                     << ", and importance->getThreshold() == " 
                     << importance->getThreshold() << endl;
               }
               // Remove
               mc2dbg8 << "FI: area = " << pixelArea
                       << " removing feature!" << endl;               
               return false;
            }
         } else {
            return false;
         }
            
      } else {
         // No threshold.
         return true;
      }
   } else {
      // Not of same type.
      return false;
   }
}

bool 
ServerTileMapFormatDesc::hasName( uint16 type ) const
{
   return true;
}



vector<ServerTileMapFormatDesc::TileNotice>
ServerTileMapFormatDesc::c_tileNotices = createTileNotices();

void
ServerTileMapFormatDesc::getBitmapNames( set<MC2SimpleString>& bitmaps,
                                         const MC2SimpleString& suffix,
                                         ImageTable::ImageSet imageSet )
{
   for ( vector<ServerTileMapFormatDesc::TileNotice>::const_iterator it =
            c_tileNotices.begin(); it != c_tileNotices.end(); ++it ) {
      MC2SimpleString imageName( ImageTable::getImage( it->m_image,
                                                       imageSet ) );
      if ( imageName != MC2SimpleString( "" ) ) {
         MC2String fullName( "b" );
         fullName += imageName.c_str();
         fullName += suffix.c_str();

         bitmaps.insert( MC2SimpleString( fullName.c_str() ) );
      }
   }
}

void
ServerTileMapFormatDesc::getNeededBitmapNames( set<MC2String>& names,
                                               ImageTable::ImageSet imageSet )
{
   // Set of unused images.
   set<ImageTable::ImageCode> notused;
   static ImageTable::ImageCode notused2[] = {
      ImageTable::ICE_SKATING_RINK,
      ImageTable::MARINA,
      ImageTable::BOWLING_CENTRE,
      ImageTable::CITYHALL,
      ImageTable::COURT_HOUSE,
      ImageTable::MUSEUM,
      ImageTable::RECREATION_FACILITY,
      ImageTable::REST_AREA,
      ImageTable::SPORTS_ACTIVITY,
      ImageTable::UNIVERSITY,
      ImageTable::WINERY,
      ImageTable::LIBRARY,
      ImageTable::SCHOOL,
      ImageTable::TOLL_ROAD,
      ImageTable::NOIMAGE,
   };

   notused.insert( &notused2[0],
                   &notused2[sizeof(notused2)/sizeof(notused2[0])] );
       
   for ( vector<ServerTileMapFormatDesc::TileNotice>::const_iterator it =
            c_tileNotices.begin(); it != c_tileNotices.end(); ++it ) {
      if ( notused.find( it->m_image ) == notused.end() ) {
         names.insert( ImageTable::getImage( it->m_image, imageSet ) );
      }
   }

   mc2dbg8 << STLUtility::co_dump(names, ", " ) << endl;
}


vector<ServerTileMapFormatDesc::TileNotice>
ServerTileMapFormatDesc::createTileNotices()
{
   // To be added to the vector of notices.
   const TileNotice notices [] = {
      // Route symbols.
      TileNotice( ServerTileCategories::OTHER,
                  TileFeature::route_origin,
                  ImageTable::ROUTE_ORIG ),
      TileNotice( ServerTileCategories::OTHER,
                  TileFeature::route_destination,
                  ImageTable::ROUTE_DEST ),
      TileNotice( ServerTileCategories::OTHER,
                  TileFeature::route_park_car,
                  ImageTable::PARK_AND_WALK ),
      
      // Special POI:s.   
      TileNotice( ServerTileCategories::OTHER,
                  TileFeature::city_centre_2,
                  ImageTable::CITY_CENTRE_SQUARE ),
      TileNotice( ServerTileCategories::OTHER,
                  TileFeature::city_centre_4,
                  ImageTable::CITY_CENTRE ),
      TileNotice( ServerTileCategories::OTHER,
                  TileFeature::city_centre_5,
                  ImageTable::CITY_CENTRE ),
      TileNotice( ServerTileCategories::OTHER,
                  TileFeature::city_centre_7,
                  ImageTable::CITY_CENTRE_SMALL_RED ),
      TileNotice( ServerTileCategories::OTHER,
                  TileFeature::city_centre_8,
                  ImageTable::CITY_CENTRE_SMALL_PINK ),
      TileNotice( ServerTileCategories::OTHER,
                  TileFeature::city_centre_10,
                  ImageTable::CITY_CENTRE_POINT ),
      TileNotice( ServerTileCategories::OTHER,
                  TileFeature::city_centre_11,
                  ImageTable::CITY_CENTRE_POINT ),
      TileNotice( ServerTileCategories::OTHER,
                  TileFeature::city_centre_12,
                  ImageTable::CITY_CENTRE_POINT_SMALL ),
      
      // Ordinary POI:s.
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::atm,
                  ImageTable::ATM,
                  ItemTypes::atm ),
      TileNotice( ServerTileCategories::LEISURE,
                  TileFeature::golf_course,
                  ImageTable::GOLF_COURSE,
                  ItemTypes::golfCourse ),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::ice_skating_rink,
                  ImageTable::ICE_SKATING_RINK,
                  ItemTypes::iceSkatingRink),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::marina,
                  ImageTable::MARINA,
                  ItemTypes::marina),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::vehicle_repair_facility,
                  ImageTable::CAR_REPAIR,
                  ItemTypes::vehicleRepairFacility),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::bowling_centre,
                  ImageTable::BOWLING_CENTRE,
                  ItemTypes::bowlingCentre),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::bank,
                  ImageTable::BANK,
                  ItemTypes::bank),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::casino,
                  ImageTable::CASINO,
                  ItemTypes::casino),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::city_hall,
                  ImageTable::CITYHALL,
                  ItemTypes::cityHall),
      TileNotice( ServerTileCategories::TRAVEL_TERMINAL,
                  TileFeature::commuter_railstation,
                  ImageTable::COMMUTER_RAIL_STATION,
                  ItemTypes::commuterRailStation),
      TileNotice( ServerTileCategories::TRAVEL_TERMINAL,
                  TileFeature::subway_station,
                  ImageTable::SUBWAY_STATION,
                  ItemTypes::subwayStation),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::courthouse,
                  ImageTable::COURT_HOUSE,
                  ItemTypes::courtHouse),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::historical_monument,
                  ImageTable::HISTORICAL_MONUMENT,
                  ItemTypes::historicalMonument),
      TileNotice( ServerTileCategories::TOURISM,
                  TileFeature::museum,
                  ImageTable::MUSEUM,
                  ItemTypes::museum),
      TileNotice( ServerTileCategories::ENTERTAINMENT,
                  TileFeature::nightlife,
                  ImageTable::NIGHTLIFE,
                  ItemTypes::nightlife),
      TileNotice( ServerTileCategories::TOURISM,
                  TileFeature::church,
                  ImageTable::CHURCH_CHRISTIAN,
                  ItemTypes::church),
      TileNotice( ServerTileCategories::TOURISM,
                  TileFeature::mosque,
                  ImageTable::CHURCH_MOSQUE,
                  ItemTypes::mosque),
      TileNotice( ServerTileCategories::TOURISM,
                  TileFeature::synagogue,
                  ImageTable::CHURCH_SYNAGOGUE,
                  ItemTypes::synagogue),
      TileNotice( ServerTileCategories::TOURISM,
                  TileFeature::hindu_temple,
                  ImageTable::HINDU_TEMPLE,
                  ItemTypes::hinduTemple),
      TileNotice( ServerTileCategories::TOURISM,
                  TileFeature::buddhist_site,
                  ImageTable::BUDDHIST_TEMPLE,
                  ItemTypes::buddhistSite),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::post_office,
                  ImageTable::POST_OFFICE,
                  ItemTypes::postOffice),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::recreation_facility,
                  ImageTable::RECREATION_FACILITY,
                  ItemTypes::recreationFacility),
      TileNotice( ServerTileCategories::RENT_A_CAR,
                  TileFeature::rent_a_car_facility,
                  ImageTable::RENTACAR_FACILITY,
                  ItemTypes::rentACarFacility),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::rest_area,
                  ImageTable::REST_AREA,
                  ItemTypes::restArea),
      TileNotice( ServerTileCategories::LEISURE,
                  TileFeature::ski_resort,
                  ImageTable::SKI_RESORT,
                  ItemTypes::skiResort),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::sports_activity,
                  ImageTable::SPORTS_ACTIVITY,
                  ItemTypes::sportsActivity),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::shop,
                  ImageTable::SHOP,
                  ItemTypes::shop ),
      TileNotice( ServerTileCategories::ENTERTAINMENT,
                  TileFeature::theatre,
                  ImageTable::THEATRE,
                  ItemTypes::theatre),
      TileNotice( ServerTileCategories::TOURISM,
                  TileFeature::tourist_attraction,
                  ImageTable::TOURIST_ATTRACTION,
                  ItemTypes::touristAttraction),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::university,
                  ImageTable::UNIVERSITY,
                  ItemTypes::university),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::winery,
                  ImageTable::WINERY,
                  ItemTypes::winery),
      TileNotice( ServerTileCategories::PARKING,
                  TileFeature::parking_garage,
                  ImageTable::PARKING_GARAGE,
                  ItemTypes::parkingGarage),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::park_and_ride,
                  ImageTable::PARK_AND_RIDE,
                  ItemTypes::parkAndRide),
      TileNotice( ServerTileCategories::PARKING,
                  TileFeature::open_parking_area,
                  ImageTable::OPEN_PARKING_AREA,
                  ItemTypes::openParkingArea),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::amusement_park,
                  ImageTable::AMUSEMENT_PARK,
                  ItemTypes::amusementPark),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::library,
                  ImageTable::LIBRARY,
                  ItemTypes::library),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::school,
                  ImageTable::SCHOOL,
                  ItemTypes::school),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::grocery_store,
                  ImageTable::GROCERY_STORE,
                  ItemTypes::groceryStore),
      TileNotice( ServerTileCategories::PETROL_STATION,
                  TileFeature::petrol_station,
                  ImageTable::PETROL_STATION,
                  ItemTypes::petrolStation ),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::tram_station,
                  ImageTable::TRAMWAY,
                  ItemTypes::tramStation),
      TileNotice( ServerTileCategories::TRAVEL_TERMINAL,
                  TileFeature::ferry_terminal,
                  ImageTable::FERRY_TERMINAL,
                  ItemTypes::ferryTerminal),
      TileNotice( ServerTileCategories::ENTERTAINMENT,
                  TileFeature::cinema,
                  ImageTable::CINEMA,
                  ItemTypes::cinema),
      TileNotice( ServerTileCategories::TRAVEL_TERMINAL,
                  TileFeature::bus_station,
                  ImageTable::BUS_STATION,
                  ItemTypes::busStation),
      TileNotice( ServerTileCategories::TRAVEL_TERMINAL,
                  TileFeature::railway_station,
                  ImageTable::RAILWAY_STATION,
                  ItemTypes::railwayStation),
      TileNotice( ServerTileCategories::TRAVEL_TERMINAL,
                  TileFeature::airport,
                  ImageTable::AIRPORT,
                  ItemTypes::airport),
      TileNotice( ServerTileCategories::RESTAURANT,
                  TileFeature::restaurant,
                  ImageTable::RESTAURANT,
                  ItemTypes::restaurant),
      TileNotice( ServerTileCategories::RESTAURANT,
                  TileFeature::cafe,
                  ImageTable::CAFE,
                  ItemTypes::cafe),
      TileNotice( ServerTileCategories::HOTEL,
                  TileFeature::hotel,
                  ImageTable::HOTEL,
                  ItemTypes::hotel),
      TileNotice( ServerTileCategories::TOURISM,
                  TileFeature::tourist_office,
                  ImageTable::TOURIST_OFFICE,
                  ItemTypes::touristOffice),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::police_station,
                  ImageTable::POLICE_STATION,
                  ItemTypes::policeStation),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::hospital,
                  ImageTable::HOSPITAL,
                  ItemTypes::hospital),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::toll_road,
                  ImageTable::TOLL_ROAD,
                  ItemTypes::tollRoad),  
      TileNotice( ServerTileCategories::WIFI_SPOT,
                  TileFeature::wlan,
                  ImageTable::WIFI_HOTSPOT,
                  ItemTypes::wlan),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::custom_poi_2,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::custom_poi_4,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::custom_poi_7,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::custom_poi_10,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::custom_poi_15,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::custom_poi_20,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::custom_poi_30,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::special_custom_poi_2,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::special_custom_poi_4,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::special_custom_poi_7,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::special_custom_poi_10,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::special_custom_poi_15,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::special_custom_poi_20,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::special_custom_poi_30,
                  ImageTable::NOIMAGE ), // image is a transfered parameter
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::event,
                  ImageTable::NOIMAGE ),
      TileNotice( ServerTileCategories::DISPLAY_OTHERS,
                  TileFeature::turkish_hospital,
                  ImageTable::TURKISH_HOSPITAL ),
   };

   // Now insert the static notices.
   vector<TileNotice> tmpVec( &notices[0],
                              &notices[sizeof(notices)/sizeof(notices[0])] );
   
   tmpVec.push_back( TileNotice( ServerTileCategories::OTHER,
                                 TileFeature::traffic_default,
                                 ImageTable::TRAFFIC_INFORMATION ) );
   tmpVec.back().m_level = 14; // Set to be displayed before other pois
   
   tmpVec.push_back( TileNotice( ServerTileCategories::OTHER,
                                 TileFeature::roadwork,
                                 ImageTable::ROAD_WORK ) );
   tmpVec.back().m_level = 14; // Set to be displayed before other pois
   
   tmpVec.push_back( TileNotice( ServerTileCategories::OTHER,
                                 TileFeature::speed_camera,
                                 ImageTable::SPEED_CAMERA ) );
   tmpVec.push_back( TileNotice( ServerTileCategories::OTHER,
                                 TileFeature::user_defined_speed_camera,
                                 ImageTable::USER_DEFINED_SPEED_CAMERA ) );
   tmpVec.back().m_level = 14; // Set to be displayed before other pois
   
   tmpVec.push_back( TileNotice( ServerTileCategories::OTHER,
                                 TileFeature::speed_trap,
                                 ImageTable::SPEED_TRAP ) );
   tmpVec.back().m_level = 14; // Set to be displayed before other pois
   
   return tmpVec;
}

void
ServerTileMapFormatDesc::createPOITables()
{
   set<MC2String> neededImages;
   getNeededBitmapNames( neededImages, m_imageSet );
   for ( uint32 i = 0; i < c_tileNotices.size(); ++i ) {
      TileNotice& notice = c_tileNotices[ i ];
      const char* imageName = ImageTable::getImage( notice.m_image, 
                                                    m_imageSet );
      // Add to to m_imageByTileFeature.
      m_imageByTileFeatureType[ notice.m_tileFeatureType ] = imageName;
      // Also insert in the other table if it is one of used
      // ones
      if ( neededImages.find( imageName ) != neededImages.end() ) {
         m_usedImageByTileFeatureType[ notice.m_tileFeatureType ] = imageName;
      }
      // Add to m_tileFeatureTypeByPOIType if poitype is valid.
      if ( notice.m_poiType != ItemTypes::nbr_pointOfInterest ) {
         m_tileFeatureTypeByPOIType[ notice.m_poiType ] = 
                                                notice.m_tileFeatureType;
      }
   }

   MC2String poiScaleRangeFilename =
      Properties::getProperty( "POI_SCALE_RANGE_FILE",
                               "poi_scale_ranges.xml" );

   mc2dbg << "[STMFD] Loading poi scale range file: "
          << poiScaleRangeFilename << endl;
   if ( ! TileFeatureScaleRange::load( poiScaleRangeFilename,
                                       m_scaleRangeByType ) ) {
      mc2dbg << warn << "[STMFD] Failed to load poi scale range file." << endl;
      mc2dbg << warn << "[STMFD] Going down." << endl;
      MC2_ASSERT( false );
   }

}

void
ServerTileMapFormatDesc::createDefaultSettings()
{   
   if ( m_drawSettingVersion == 0 ) {
      createOldDefaultSettings();
      return;
   }

   map<int, TileDrawSettings> settingsByType;
   MC2String tileMapColorFilename;
   if ( m_nightMode ) {
      if ( m_drawSettingVersion == 3 ) {
         tileMapColorFilename = Properties::
            getProperty( "TILE_MAP_NIGHT_COLOR_FILE_V3", "tile_map_night_colors.xml" );
      } else {
         tileMapColorFilename = Properties::
            getProperty( "TILE_MAP_NIGHT_COLOR_FILE_V2", "tile_map_night_colors_v2.xml" );
      }
   } else {
      if ( m_drawSettingVersion == 3 ) {
         tileMapColorFilename = Properties::
            getProperty( "TILE_MAP_COLOR_FILE_V3", "tile_map_colors.xml" );
      } else {
         tileMapColorFilename = Properties::
            getProperty( "TILE_MAP_COLOR_FILE_V2", "tile_map_colors_v2.xml" );
      }

   }

   mc2dbg << "[STMFD] Loading tile map colors from: "
          << tileMapColorFilename << endl;
   TileColorSettings::BasicSetting basicColorSetting;
   if ( ! TileColorSettings::
        loadSettings( tileMapColorFilename.c_str(),
                      settingsByType, basicColorSetting ) ) {
      mc2dbg << "[STMFD] Failed to tile map colors from: \""
             << tileMapColorFilename << "\"" << endl;
      MC2_ASSERT( false );
   }

   m_topHorizonColor = basicColorSetting.m_topHorizonColor;
   m_bottomHorizonColor = basicColorSetting.m_bottomHorizonColor;
   m_textColor = basicColorSetting.m_textColor;

   TileDrawSettings settings;
   TileDrawSetting setting;

   // The POI:s -----------------------------------------------------------


   settings.m_primitive    = TilePrimitiveFeature::bitmap;

   for ( imageByType_t::const_iterator it = 
            m_imageByTileFeatureType.begin(); 
         it != m_imageByTileFeatureType.end(); ++it ) {
      setting.reset();
      setting.m_imageName = it->second;

      // Get level from the vector of notices
      // Default is 15
      setting.m_level     = 15;

      for ( vector<TileNotice>::const_iterator vt = c_tileNotices.begin();
            vt != c_tileNotices.end();
            ++vt ) {
         if ( vt->m_tileFeatureType == it->first ) {
            if ( vt->m_level != -1 ) {
               setting.m_level = vt->m_level;
            }
            // Ok we have found it.
            break;
         }
      }

      // Special settings for citycentre.
      switch ( it->first ) {
         case ( TileFeature::city_centre_2 ) :
         case ( TileFeature::city_centre_4 ) :
         case ( TileFeature::city_centre_5 ) :
         case ( TileFeature::city_centre_7 ) :
         case ( TileFeature::city_centre_8 ) :
         case ( TileFeature::city_centre_10 ) :
         case ( TileFeature::city_centre_11 ) :
         case ( TileFeature::city_centre_12 ) :
            setting.m_nameType = TileMapNameSettings::horizontal;
            setting.m_fontSize = 12;
            setting.m_fontType = 0;
            break;
         default:
            // Reset.
            break;
      }
      settings[ MAX_TILE_SCALELEVEL ] = setting;      
      settingsByType.insert( make_pair( it->first, settings ) );
   }
   settings.clear();


   // --------------------------------------------------------------------- 

   // Count how many scaleIndex we need.
   m_scaleIndexByLevel.clear();
   
   for ( map<int, TileDrawSettings>::const_iterator it = 
            settingsByType.begin(); it != settingsByType.end(); ++it ) {
      for ( TileDrawSettings::const_iterator jt = it->second.begin();
            jt != it->second.end(); ++jt ) {
         m_scaleIndexByLevel.insert( make_pair( jt->first, -1 ) );
      }
   }

   // Set the scale index now.
   int index = 0;
   for ( scaleIndexByLevel_t::iterator it = m_scaleIndexByLevel.begin();
         it != m_scaleIndexByLevel.end(); ++it ) {
      it->second = index;
      ++index;
   }


   int nbrScaleIndex = m_scaleIndexByLevel.size();

   for ( map<int, TileDrawSettings>::const_iterator it = 
            settingsByType.begin(); it != settingsByType.end(); ++it ) {
      
      
      int featureType = it->first;      
      const TileDrawSettings& settings = it->second;
      // Create new settings which are keyed by scale index instead
      // of scale level.
      TileDrawSettings settingsByIndex;
      
      for ( TileDrawSettings::const_iterator jt = settings.begin();
            jt != settings.end(); ++jt ) {
         settingsByIndex[ m_scaleIndexByLevel[ jt->first ] ] = jt->second;
      }

      MC2_ASSERT( ! settingsByIndex.empty() );

      vector<uint32> color( nbrScaleIndex );
      vector<uint32> borderColor( nbrScaleIndex );
      vector<uint32> width( nbrScaleIndex );
      vector<uint32> widthMeters( nbrScaleIndex );
      vector<uint32> borderWidth( nbrScaleIndex );
      vector<uint32> nameType( nbrScaleIndex );
      vector<uint32> level( nbrScaleIndex );
      vector<uint32> level1( nbrScaleIndex );
      vector<uint32> fontSize( nbrScaleIndex );
      vector<uint32> fontType( nbrScaleIndex );
      vector<MC2SimpleString> imageName( nbrScaleIndex );
      
      const TileDrawSetting* prevSetting = NULL;
      
      for ( int i = nbrScaleIndex - 1; i >= 0; --i ) {
         
         TileDrawSettings::const_iterator findIt = 
            settingsByIndex.find( i );

         const TileDrawSetting* setting = NULL;
         if ( findIt != settingsByIndex.end() ) {

            setting = &(findIt->second);

            prevSetting = setting;
         } else {
            MC2_ASSERT( prevSetting != NULL );
            setting = prevSetting;
         }
         
         color[ i ] = setting->m_color;
         borderColor[ i ] = setting->m_borderColor;
         width[ i ] = setting->m_width;
         widthMeters[ i ] = setting->m_widthMeters;
         borderWidth[ i ] = setting->m_borderWidth;
         nameType[ i ] = setting->m_nameType;
         level[ i ] = setting->m_level;
         level1[ i ] = setting->m_level1;
         fontSize[ i ] = setting->m_fontSize;
         fontType[ i ] = setting->m_fontType;
         imageName[ i ] = setting->m_imageName;
      }

      // Create the invalid vectors.
      vector<uint32> invalidVecUint32( nbrScaleIndex, MAX_UINT32 );
      vector<MC2SimpleString> invalidVecString( nbrScaleIndex, "" );
      
      vector<TileFeatureArg*> args;
      
      // Color.
      if ( checkAndUpdateArgVector( color, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::color, 25 );
         arg->setValue( color );
         args.push_back( arg );
      }
      
      if ( checkAndUpdateArgVector( borderColor, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::border_color, 25 );
         arg->setValue( borderColor );
         args.push_back( arg );
      }
      
      // Width.
      if ( checkAndUpdateArgVector( width, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::width, 8 );
         arg->setValue( width );
         args.push_back( arg );
      }

      // Width in meters
      if ( checkAndUpdateArgVector( widthMeters, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::width_meters, 8 );
         arg->setValue( widthMeters );
         args.push_back( arg );
      }

      // Border width.
      if ( checkAndUpdateArgVector( borderWidth, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::border_width, 8 );
         arg->setValue( borderWidth );
         args.push_back( arg );
      }

      // Name type.
      if ( checkAndUpdateArgVector( nameType, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::name_type, 8 );
         arg->setValue( nameType );
         args.push_back( arg );
      }

      // Level.
      if ( checkAndUpdateArgVector( level, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::level, 8 );
         arg->setValue( level );
         args.push_back( arg );
      }

      // Level 1.
      if ( checkAndUpdateArgVector( level1, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::level_1, 8 );
         arg->setValue( level1 );
         args.push_back( arg );
      }

      // Font type.
      if ( checkAndUpdateArgVector( fontType, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::font_type, 8 );
         arg->setValue( fontType );
         args.push_back( arg );
      }
      
      // Font size.
      if ( checkAndUpdateArgVector( fontSize, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::font_size, 8 );
         arg->setValue( fontSize );
         args.push_back( arg );
      }
      
      // Image name.
      if ( checkAndUpdateArgVector( imageName, invalidVecString ) ) {
         StringArg* arg = new StringArg( TileArgNames::image_name );
         arg->setValue( imageName );
         args.push_back( arg );
      }
     
      // Add min and max scale if present.
      addDefaultScaleRange( featureType, args );
     
      WritableTileFeature* feature = 
         new WritableTileFeature( settings.m_primitive );
      feature->setArgs( args );

      primitiveVector_t prims;
      prims.push_back( feature );

      if ( m_primitiveDefaultMapSize < ( featureType + 1 ) ) {
         m_primitiveDefaultMap =
            ArrayTool::reallocate( m_primitiveDefaultMap,
                                 m_primitiveDefaultMapSize,
                                 featureType + 1,
                                 m_primitiveDefaultMapSize );
      }
      m_primitiveDefaultMap[ featureType ] = prims;

      // Add to arg transfer map.
      addArgTransferMap( featureType, settings.m_primitive );
     
      // Create the transfer args.
      createArgVectorForType( featureType );
   }
} 

void
ServerTileMapFormatDesc::createOldDefaultSettings()
{   
   map<int, TileDrawSettings> settingsByType;

   TileDrawSettings settings;
   TileDrawSetting setting;

   m_textColor = makeColor( 0x17, 0x17, 0x17,
                            0xFF, 0xFF, 0xFF );
   m_topHorizonColor = makeColor( 0xB2, 0xAD, 0xF4,
                                  0x00, 0x00, 0x00 );
   m_bottomHorizonColor = makeColor( 0xFF, 0xFF, 0xFF,
                                     0x6E, 0x6E, 0x6E );

   // Road class 0 --------------------------------------------------------
   // orange, border bright red
   setting.m_color         = makeColor( 240, 180, 50, 180, 125, 125 );
   setting.m_borderColor   = makeColor( 224, 0, 0, 180, 80, 110 );
   setting.m_width         = 14;
   setting.m_widthMeters   = 22;
   //setting.m_borderWidth   = 3;
   setting.m_nameType      = TileMapNameSettings::on_line;
   setting.m_fontType      = 0;
   setting.m_fontSize      = 7;
   
   settings.m_primitive    = TilePrimitiveFeature::line;
   settings[ 1 ] = setting;
   
   setting.m_width         = 10;
   settings[ 2 ] = setting;
   
   setting.m_width         = 8;
   setting.m_widthMeters   = MAX_UINT32;  // use normal width on zoom 2.0-n
   settings[ 4 ] = setting;
   
   setting.m_width         = 6;
   settings[ 12 ] = setting;
   
   setting.m_width         = 4;
   settings[ 30 ] = setting;
   
   setting.m_width         = 3; 
   settings[ 100 ] = setting;

   // border light red (ssi_1 goes light brown)
   setting.m_borderColor   = makeColor( 255, 80, 55, 150, 80, 100 );
   setting.m_width         = 2; 
   settings[ 600 ] = setting;
   
   // light brown / brownish pink, no border
   setting.m_color         = makeColor( 195, 140, 75, 160, 130, 130 );
   setting.m_borderColor   = MAX_UINT32;
   setting.m_width         = 2;
   settings[ 1000 ] = setting;
   
   setting.m_width         = 1;
   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::street_class_0,
                                     settings ) );

   // ramp settings, less width
   TileDrawSettings rampsettings = settings;
   for ( TileDrawSettings::iterator it = rampsettings.begin(); 
         it != rampsettings.end(); ++it ) {
      uint32 origwidth = it->second.m_width;
      uint32 origwidthMeters = it->second.m_widthMeters;
      if ( (origwidthMeters > 0) && (origwidthMeters != MAX_UINT32)) {
         uint32 newwidthMeters = MAX( int(0.5 * origwidthMeters), 1);
         it->second.m_widthMeters = newwidthMeters;
      }
      if ( origwidth > 0 ) {
         uint32 newwidth = MAX( int(0.5 * origwidth), 1);
         it->second.m_width = newwidth;
      }
   }
   settingsByType.insert( make_pair( TileFeature::street_class_0_ramp,
                                     rampsettings ) );
   rampsettings.clear();
   
   // Copy the other settings and only set the level.
   for ( TileDrawSettings::iterator it = settings.begin(); 
         it != settings.end(); ++it ) {
      it->second.m_level = c_defaultRoadFeatureLevel; 
      it->second.m_level1 = c_defaultRoadFeatureLevel; 
   }
   settingsByType.insert( make_pair( TileFeature::street_class_0_level_0,
                                     settings ) );
   settings.clear();
  
   // Road class 1 --------------------------------------------------------
   setting.reset();
   // Yellow-brown-grey
   setting.m_color         = makeColor( 225, 205, 110, 170, 130, 130 );
   setting.m_borderColor   = makeColor( 255, 80, 55, 150, 85, 100 );
   setting.m_width         = 12;
   setting.m_widthMeters   = 22;
   //setting.m_borderWidth   = 3;
   setting.m_nameType      = TileMapNameSettings::on_line;
   setting.m_fontType      = 0;
   setting.m_fontSize      = 7;
   
   settings.m_primitive    = TilePrimitiveFeature::line;
   settings[ 1 ] = setting;
   
   setting.m_width         = 9;
   settings[ 2 ] = setting;
   
   setting.m_width         = 7;
   setting.m_widthMeters   = MAX_UINT32;  // use normal width on zoom 2.0-n
   settings[ 3 ] = setting;
   
   setting.m_width         = 6;
   settings[ 4 ] = setting;
   
   setting.m_width         = 5;
   settings[ 5 ] = setting;
   
   setting.m_width         = 4;
   settings[ 12 ] = setting;
   
   setting.m_width         = 3;
   settings[ 30 ] = setting;
   
   setting.m_width         = 2;
   settings[ 100 ] = setting;

   // light brown
   setting.m_borderColor   = makeColor( 195, 140, 75, 140, 80, 90 );
   setting.m_width         = 1; 
   settings[ MAX_TILE_SCALELEVEL ] = setting;

   settingsByType.insert( make_pair( TileFeature::street_class_1,
                                     settings ) );
   // ramp settings, less width
   rampsettings = settings;
   for ( TileDrawSettings::iterator it = rampsettings.begin(); 
         it != rampsettings.end(); ++it ) {
      uint32 origwidth = it->second.m_width;
      uint32 origwidthMeters = it->second.m_widthMeters;
      if ( (origwidthMeters > 0) && (origwidthMeters != MAX_UINT32)) {
         uint32 newwidthMeters = MAX( int(0.5 * origwidthMeters), 1);
         it->second.m_widthMeters = newwidthMeters;
      }
      if ( origwidth > 0 ) {
         uint32 newwidth = MAX( int(0.5 * origwidth), 1);
         it->second.m_width = newwidth;
      }
   }
   settingsByType.insert( make_pair( TileFeature::street_class_1_ramp,
                                     rampsettings ) );
   rampsettings.clear();

   // Copy the other settings and only set the level.
   for ( TileDrawSettings::iterator it = settings.begin(); 
         it != settings.end(); ++it ) {
      it->second.m_level = c_defaultRoadFeatureLevel; 
      it->second.m_level1 = c_defaultRoadFeatureLevel; 
   }
   settingsByType.insert( make_pair( TileFeature::street_class_1_level_0,
                                     settings ) );
   settings.clear();

   // Road class 2 --------------------------------------------------------
   setting.reset();
   // pale yellow
   setting.m_color         = makeColor( 248, 248, 205, 185, 160, 160 );
   // light red
   setting.m_borderColor   = makeColor( 255, 80, 55, 160, 110, 140 );
   setting.m_width         = 11;
   setting.m_widthMeters   = 18;
   //setting.m_borderWidth   = 3;
   setting.m_nameType      = TileMapNameSettings::on_line;
   setting.m_fontType      = 0;
   setting.m_fontSize      = 7;
   
   settings.m_primitive    = TilePrimitiveFeature::line;   
   settings[ 1 ] = setting;
   
   setting.m_width         = 8;
   settings[ 2 ] = setting;
   
   setting.m_width         = 7;
   setting.m_widthMeters   = MAX_UINT32;  // use normal width on zoom 2.0-n
   settings[ 3 ] = setting;
   
   setting.m_width         = 6;
   settings[ 4 ] = setting;
   
   setting.m_width         = 5;
   settings[ 5 ] = setting;
   
   setting.m_width         = 4;
   settings[ 12 ] = setting;
   
   setting.m_width         = 3;
   settings[ 30 ] = setting;
   
   setting.m_width         = 2;
   settings[ 60 ] = setting;
   
   // light brown
   setting.m_borderColor   = makeColor( 195, 140, 75, 160, 110, 140 );
   setting.m_width         = 1;
   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::street_class_2,
                                     settings ) );
   // ramp settings, less width
   rampsettings = settings;
   for ( TileDrawSettings::iterator it = rampsettings.begin(); 
         it != rampsettings.end(); ++it ) {
      uint32 origwidth = it->second.m_width;
      uint32 origwidthMeters = it->second.m_widthMeters;
      if ( (origwidthMeters > 0) && (origwidthMeters != MAX_UINT32)) {
         uint32 newwidthMeters = MAX( int(0.5 * origwidthMeters), 1);
         it->second.m_widthMeters = newwidthMeters;
      }
      if ( origwidth > 0 ) {
         uint32 newwidth = MAX( int(0.5 * origwidth), 1);
         it->second.m_width = newwidth;
      }
   }
   settingsByType.insert( make_pair( TileFeature::street_class_2_ramp,
                                     rampsettings ) );
   rampsettings.clear();

   // Copy the other settings and only set the level.
   for ( TileDrawSettings::iterator it = settings.begin(); 
         it != settings.end(); ++it ) {
      it->second.m_level = c_defaultRoadFeatureLevel; 
      it->second.m_level1 = c_defaultRoadFeatureLevel; 
   }
   settingsByType.insert( make_pair( TileFeature::street_class_2_level_0,
                                     settings ) );
   settings.clear();

   // Road class 3 --------------------------------------------------------
   setting.reset();
   // Ghostwhite
   setting.m_color         = makeColor( 248, 248, 255, 195, 185, 185);
   // light brown
   setting.m_borderColor   = makeColor( 195, 140, 75, 200, 160, 180);
   setting.m_width         = 10;
   setting.m_widthMeters   = 18;
   //setting.m_borderWidth   = 3;
   setting.m_nameType      = TileMapNameSettings::on_line;
   setting.m_fontType      = 0;
   setting.m_fontSize      = 7;
   
   settings.m_primitive    = TilePrimitiveFeature::line;   
   settings[ 1 ] = setting;
  
   setting.m_width         = 8;
   settings[ 2 ] = setting;
   
   setting.m_width         = 7;
   setting.m_widthMeters   = MAX_UINT32;  // use normal width on zoom 2.0-n
   settings[ 3 ] = setting;
   
   setting.m_width         = 6;
   settings[ 4 ] = setting;
   
   setting.m_width         = 5;
   settings[ 5 ] = setting;
   
   setting.m_width         = 4;
   settings[ 7 ] = setting;
   
   setting.m_width         = 3;
   settings[ 12 ] = setting;
   
   setting.m_width         = 2;
   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::street_class_3,
                                     settings ) );

   // Copy the other settings and only set the level.
   for ( TileDrawSettings::iterator it = settings.begin(); 
         it != settings.end(); ++it ) {
      it->second.m_level = c_defaultRoadFeatureLevel; 
      it->second.m_level1 = c_defaultRoadFeatureLevel; 
   }
   settingsByType.insert( make_pair( TileFeature::street_class_3_level_0,
                                     settings ) );
   settings.clear();

   // Road class 4 --------------------------------------------------------
   setting.reset();
   // Ghostwhite
   setting.m_color         = makeColor( 248, 248, 255, 195, 185, 185);
   // light brown
   setting.m_borderColor   = makeColor( 195, 140, 75, 200, 160, 180);
   setting.m_width         = 10;
   setting.m_widthMeters   = 16;
   //setting.m_borderWidth   = 3;
   setting.m_nameType      = TileMapNameSettings::on_line;
   setting.m_fontType      = 0;
   setting.m_fontSize      = 7;
   
   settings.m_primitive    = TilePrimitiveFeature::line;   
   settings[ 1 ] = setting;
  
   setting.m_width         = 8;
   settings[ 2 ] = setting;
   
   setting.m_width         = 7;
   setting.m_widthMeters   = MAX_UINT32;  // use normal width on zoom 2.0-n
   settings[ 3 ] = setting;
   
   setting.m_width         = 5;
   settings[ 4 ] = setting;
   
   setting.m_width         = 4;
   settings[ 5 ] = setting;
   
   setting.m_width         = 3;
   settings[ 12 ] = setting;
   
   setting.m_width         = 2;
   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::street_class_4,
                                     settings ) );

   // Copy the other settings and only set the level.
   for ( TileDrawSettings::iterator it = settings.begin(); 
         it != settings.end(); ++it ) {
      it->second.m_level = c_defaultRoadFeatureLevel; 
      it->second.m_level1 = c_defaultRoadFeatureLevel; 
   }
   settingsByType.insert( make_pair( TileFeature::street_class_4_level_0,
                                     settings ) );
   settings.clear();

   // Route ---------------------------------------------------------------
   setting.reset();
   //setting.m_color         = 0xff0000; // red
   setting.m_color         = makeColor( 0xff, 0x0, 0x0, 0xee, 0x0, 0x0); // red
   setting.m_width         = 9;
   //setting.m_borderWidth   = 4;
   setting.m_level          = 14;
   
   settings.m_primitive    = TilePrimitiveFeature::line;
   settings[ 2 ] = setting;
   
   setting.m_width = 8;
   settings[ 3 ] = setting;
   
   setting.m_width = 7;
   settings[ 4 ] = setting;
   
   setting.m_width = 6;
   settings[ 12 ] = setting;
   
   setting.m_width = 4;
   settings[ 100 ] = setting;
   
   setting.m_width = 3;
   settings[ 1000 ] = setting;
   
   setting.m_width = 2;
   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::route, settings ) );
   
   settings.clear();

   // Railway -------------------------------------------------------------
   setting.reset();
   setting.m_color         = makeColor(0x77, 0x77, 0x77, 0x66, 0x53, 0x53);
   setting.m_borderColor   = MAX_UINT32;
   setting.m_width         = 3;
   //setting.m_borderWidth   = 3;
   setting.m_level         = 7;  // Changed from 4
   
   settings.m_primitive    = TilePrimitiveFeature::line;
   settings[ 4 ] = setting;
   
   setting.m_width         = 2;
   settings[ 8 ] = setting;
   
   setting.m_width         = 1;
   settings[ MAX_TILE_SCALELEVEL ] = setting;

   settingsByType.insert( make_pair( TileFeature::railway, settings ) );
   settings.clear();

   // Building area ------------------------------------------------------
   setting.reset();
   // color same as airport ground
   setting.m_color         = makeColor( 0x99, 0x99, 0x99, 0x80, 0x79, 0x79);
   setting.m_level         = 5;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::building_area,
                                     settings ) );
   settings.clear();

   // Building outline ----------------------------------------------------
   setting.reset();
   //setting.m_color         = 0x704030;    // ok brown, but too dark (texts).
   setting.m_color         = makeColor( 0xb0, 0x80, 0x90, 0x78, 0x5c, 0x5c);
   setting.m_level         = 6;  // Changed from 4
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::building_outline,
                                     settings ) );
   settings.clear();

   // Park ----------------------------------------------------------------
   setting.reset();
   // Black at night
   setting.m_color         = makeColor( 0x8f, 0xbc, 0x8f, 0x19, 0x50, 0x19 );
   setting.m_level         = 5;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::park,
                                     settings ) );
   settings.clear();

   // Water ---------------------------------------------------------------
   setting.reset();
   // color same as ocean
   setting.m_color         = makeColor( 0x84, 0x70, 0xff, 0x40, 0x40, 0x90);
   setting.m_level         = 3;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::water,
                                     settings ) );
   settings.clear();

   // Bua -----------------------------------------------------------------
   setting.reset();
   //setting.m_color         = makeColor( 245, 185, 150,  245, 185, 150); // A bit too dark
   
   //setting.m_color         = makeColor( 245, 190, 160, 166, 138, 138); //nm:greypink
   setting.m_color         = makeColor( 245, 190, 160, 85, 60, 65); //nm:darkgreyred
   setting.m_level         = 2;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ 100 ] = setting;

   //setting.m_color         = makeColor( 254, 175, 130,  254, 175, 130);
   setting.m_color         = makeColor( 245, 185, 150, 85, 60, 65);
   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::bua,
                                     settings ) );
   settings.clear();

   // Land ----------------------------------------------------------------
   setting.reset();
   setting.m_color         = m_backgroundColor;
   setting.m_level         = 1;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::land,
                                     settings ) );
   settings.clear();

   // Border --------------------------------------------------------------
   setting.reset();
   setting.m_color         = makeColor( 180, 180, 160, 85, 80, 80 ); // grey
   setting.m_borderColor   = MAX_UINT32;
   setting.m_width         = 1;
   setting.m_level         = 1;
   settings.m_primitive    = TilePrimitiveFeature::line;

   settings[ 20000 ] = setting;
   
   setting.m_color         = m_backgroundColor; // not shown
   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::border, settings ) );
   settings.clear();

   // Airport ground (airport items) -------------------------------------
   setting.reset();
   // color same as building area
   setting.m_color         = makeColor( 0x99, 0x99, 0x99, 0x80, 0x79, 0x79);
   setting.m_level         = 5;        // same as building area, park
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::airport_ground,
                                     settings ) );
   settings.clear();

   // Cartographic green areas --------------------------------------------
   setting.reset();
   setting.m_color         = makeColor( 162, 198, 156,
                                        //[L]0x8f, 0xbc, 0x8f, 
                                        0x19, 0x50, 0x19 );
   setting.m_level         = 5;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::cartographic_green_area,
                                     settings ) );
   settings.clear();

   // Cartographic ground -------------------------------------------------
   setting.reset();
   setting.m_color         = makeColor( 212, 212, 212,
                                        //[L] 0x99, 0x99, 0x99, 
                                        0x80, 0x79, 0x79 );
   setting.m_level         = 5;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::cartographic_ground,
                                     settings ) );
   settings.clear();

   // Forest --------------------------------------------------------------
   setting.reset();
   setting.m_color         = makeColor(  162, 198, 156,
                                         //[L]0x8f, 0xbc, 0x8f, 
                                         0x19, 0x50, 0x19 );
   setting.m_level         = 1;// 5;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::forest,
                                     settings ) );
   settings.clear();

   // Aircraftroads -------------------------------------------------------
   setting.reset();
   setting.m_color         = makeColor( 0xef, 0xef, 0xef, 0xbf, 0xbf, 0xbf );
   setting.m_level         = 6;  // on top of airport_ground
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::aircraftroad,
                                     settings ) );
   settings.clear();

   // Island ----------------------------------------------------------------
   setting.reset();
   setting.m_color         = m_backgroundColor;
   setting.m_level         = 4;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::island,
                                     settings ) );
   settings.clear();

   // Ocean ---------------------------------------------------------------
   setting.reset();
   // color same as water
   setting.m_color         = makeColor( 0x84, 0x70, 0xff, 0x40, 0x40, 0x90);
   setting.m_level         = 0;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::ocean,
                                     settings ) );
   settings.clear();

   // Walkway -------------------------------------------------------------
   setting.reset();
   // Ghostwhite
   setting.m_color         = makeColor( 248, 248, 255, 195, 185, 185);
   // light brown
   setting.m_borderColor   = makeColor( 195, 140, 75, 200, 160, 180);
   setting.m_level         = 7;
   setting.m_width         = 10;
   setting.m_widthMeters   = 16;
   //setting.m_borderWidth   = 3;
   setting.m_nameType      = TileMapNameSettings::on_line;
   setting.m_fontType      = 0;
   setting.m_fontSize      = 7;
   
   settings.m_primitive    = TilePrimitiveFeature::line;   
   settings[ 1 ] = setting;
  
   setting.m_width         = 8;
   settings[ 2 ] = setting;
   
   setting.m_width         = 7;
   setting.m_widthMeters   = MAX_UINT32;  // use normal width on zoom 2.0-n
   settings[ 3 ] = setting;
   
   setting.m_width         = 5;
   settings[ 4 ] = setting;
   
   setting.m_width         = 4;
   settings[ 5 ] = setting;
   
   setting.m_width         = 3;
   settings[ 12 ] = setting;
   
   setting.m_width         = 2;
   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::walkway,
                                     settings ) );

   settings.clear();

   // Water in park -------------------------------------------------------
   setting.reset();
   setting.m_color         = makeColor( 0x84, 0x70, 0xff, 0x40, 0x40, 0x90);
   setting.m_level         = 6;
   settings.m_primitive    = TilePrimitiveFeature::polygon;
   
   settings[ MAX_TILE_SCALELEVEL ] = setting;
   
   settingsByType.insert( make_pair( TileFeature::water_in_park,
                                     settings ) );
   settings.clear();

   // Island in BUA ------------------------------------------------------
   setting.reset();
   setting.m_color         = makeColor( 245, 190, 160, 85, 60, 65);
   setting.m_level         = 4;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;

   settingsByType.insert( make_pair( TileFeature::island_in_bua,
                                     settings ) );
   settings.clear();

   // Island in water in park ---------------------------------------------
   setting.reset();
   setting.m_color         = m_backgroundColor;
   setting.m_level         = 7;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;

   settingsByType.insert( make_pair( TileFeature::island_in_water_in_park,
                                     settings ) );
   settings.clear();

   // BUA on island ------------- -----------------------------------------
   setting.reset();
   setting.m_color         = makeColor( 245, 190, 160, 85, 60, 65);
   setting.m_level         = 5;
   settings.m_primitive    = TilePrimitiveFeature::polygon;

   settings[ MAX_TILE_SCALELEVEL ] = setting;

   settingsByType.insert( make_pair( TileFeature::bua_on_island,
                                     settings ) );
   settings.clear();

   // The POI:s -----------------------------------------------------------
   
   settings.m_primitive    = TilePrimitiveFeature::bitmap;
   
   for ( imageByType_t::const_iterator it = 
            m_imageByTileFeatureType.begin(); 
         it != m_imageByTileFeatureType.end(); ++it ) {
      setting.reset();
      setting.m_imageName = it->second;

      // Get level from the vector of notices
      // Default is 15
      setting.m_level     = 15;

      for ( vector<TileNotice>::const_iterator vt = c_tileNotices.begin();
            vt != c_tileNotices.end();
            ++vt ) {
         if ( vt->m_tileFeatureType == it->first ) {
            if ( vt->m_level != -1 ) {
               setting.m_level = vt->m_level;
            }
            // Ok we have found it.
            break;
         }
      }
      
      // Special settings for citycentre.
      switch ( it->first ) {
         case ( TileFeature::city_centre_2 ) :
         case ( TileFeature::city_centre_4 ) :
         case ( TileFeature::city_centre_5 ) :
         case ( TileFeature::city_centre_7 ) :
         case ( TileFeature::city_centre_8 ) :
         case ( TileFeature::city_centre_10 ) :
         case ( TileFeature::city_centre_11 ) :
         case ( TileFeature::city_centre_12 ) :
            setting.m_nameType = TileMapNameSettings::horizontal;
            setting.m_fontSize = 12;
            setting.m_fontType = 0;
            break;
         default:
            // Reset.
            break;
      }
      settings[ MAX_TILE_SCALELEVEL ] = setting;      
      settingsByType.insert( make_pair( it->first, settings ) );
   }
   settings.clear();
   
   // --------------------------------------------------------------------- 

   // Count how many scaleIndex we need.
   m_scaleIndexByLevel.clear();
   
   for ( map<int, TileDrawSettings>::const_iterator it = 
            settingsByType.begin(); it != settingsByType.end(); ++it ) {
      for ( TileDrawSettings::const_iterator jt = it->second.begin();
            jt != it->second.end(); ++jt ) {
         m_scaleIndexByLevel.insert( make_pair( jt->first, -1 ) );
      }
   }

   // Set the scale index now.
   int index = 0;
   for ( scaleIndexByLevel_t::iterator it = m_scaleIndexByLevel.begin();
         it != m_scaleIndexByLevel.end(); ++it ) {
      it->second = index;
      ++index;
   }

   int nbrScaleIndex = m_scaleIndexByLevel.size();

   for ( map<int, TileDrawSettings>::const_iterator it = 
            settingsByType.begin(); it != settingsByType.end(); ++it ) {
      
      
      int featureType = it->first;      
      const TileDrawSettings& settings = it->second;
      // Create new settings which are keyed by scale index instead
      // of scale level.
      TileDrawSettings settingsByIndex;
      
      for ( TileDrawSettings::const_iterator jt = settings.begin();
            jt != settings.end(); ++jt ) {
         settingsByIndex[ m_scaleIndexByLevel[ jt->first ] ] = jt->second;
      }

      MC2_ASSERT( ! settingsByIndex.empty() );

      vector<uint32> color( nbrScaleIndex );
      vector<uint32> borderColor( nbrScaleIndex );
      vector<uint32> width( nbrScaleIndex );
      vector<uint32> widthMeters( nbrScaleIndex );
      vector<uint32> borderWidth( nbrScaleIndex );
      vector<uint32> nameType( nbrScaleIndex );
      vector<uint32> level( nbrScaleIndex );
      vector<uint32> level1( nbrScaleIndex );
      vector<uint32> fontSize( nbrScaleIndex );
      vector<uint32> fontType( nbrScaleIndex );
      vector<MC2SimpleString> imageName( nbrScaleIndex );
      
      const TileDrawSetting* prevSetting = NULL;
      
      for ( int i = nbrScaleIndex - 1; i >= 0; --i ) {
         
         TileDrawSettings::const_iterator findIt = 
            settingsByIndex.find( i );

         const TileDrawSetting* setting = NULL;
         if ( findIt != settingsByIndex.end() ) {

            setting = &(findIt->second);

            prevSetting = setting;
         } else {
            MC2_ASSERT( prevSetting != NULL );
            setting = prevSetting;
         }
         
         color[ i ] = setting->m_color;
         borderColor[ i ] = setting->m_borderColor;
         width[ i ] = setting->m_width;
         widthMeters[ i ] = setting->m_widthMeters;
         borderWidth[ i ] = setting->m_borderWidth;
         nameType[ i ] = setting->m_nameType;
         level[ i ] = setting->m_level;
         level1[ i ] = setting->m_level1;
         fontSize[ i ] = setting->m_fontSize;
         fontType[ i ] = setting->m_fontType;
         imageName[ i ] = setting->m_imageName;
      }

      // Create the invalid vectors.
      vector<uint32> invalidVecUint32( nbrScaleIndex, MAX_UINT32 );
      vector<MC2SimpleString> invalidVecString( nbrScaleIndex, "" );
      
      vector<TileFeatureArg*> args;
      
      // Color.
      if ( checkAndUpdateArgVector( color, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::color, 25 );
         arg->setValue( color );
         args.push_back( arg );
      }
      
      if ( checkAndUpdateArgVector( borderColor, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::border_color, 25 );
         arg->setValue( borderColor );
         args.push_back( arg );
      }
      
      // Width.
      if ( checkAndUpdateArgVector( width, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::width, 8 );
         arg->setValue( width );
         args.push_back( arg );
      }

      // Width in meters
      if ( checkAndUpdateArgVector( widthMeters, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::width_meters, 8 );
         arg->setValue( widthMeters );
         args.push_back( arg );
      }

      // Border width.
      if ( checkAndUpdateArgVector( borderWidth, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::border_width, 8 );
         arg->setValue( borderWidth );
         args.push_back( arg );
      }

      // Name type.
      if ( checkAndUpdateArgVector( nameType, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::name_type, 8 );
         arg->setValue( nameType );
         args.push_back( arg );
      }

      // Level.
      if ( checkAndUpdateArgVector( level, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::level, 8 );
         arg->setValue( level );
         args.push_back( arg );
      }

      // Level 1.
      if ( checkAndUpdateArgVector( level1, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::level_1, 8 );
         arg->setValue( level1 );
         args.push_back( arg );
      }

      // Font type.
      if ( checkAndUpdateArgVector( fontType, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::font_type, 8 );
         arg->setValue( fontType );
         args.push_back( arg );
      }
      
      // Font size.
      if ( checkAndUpdateArgVector( fontSize, invalidVecUint32 ) ) {
         SimpleArg* arg = new SimpleArg( TileArgNames::font_size, 8 );
         arg->setValue( fontSize );
         args.push_back( arg );
      }
      
      // Image name.
      if ( checkAndUpdateArgVector( imageName, invalidVecString ) ) {
         StringArg* arg = new StringArg( TileArgNames::image_name );
         arg->setValue( imageName );
         args.push_back( arg );
      }
     
      // Add min and max scale if present.
      addDefaultScaleRange( featureType, args );
     
      WritableTileFeature* feature = 
         new WritableTileFeature( settings.m_primitive );
      feature->setArgs( args );

      primitiveVector_t prims;
      prims.push_back( feature );

      if ( m_primitiveDefaultMapSize < ( featureType + 1 ) ) {
         m_primitiveDefaultMap =
            ArrayTool::reallocate( m_primitiveDefaultMap,
                                 m_primitiveDefaultMapSize,
                                 featureType + 1,
                                 m_primitiveDefaultMapSize );
      }
      m_primitiveDefaultMap[ featureType ] = prims;

      // Add to arg transfer map.
      addArgTransferMap( featureType, settings.m_primitive );
     
      // Create the transfer args.
      createArgVectorForType( featureType );
   }
} 

bool
ServerTileMapFormatDesc::closedFeatureType( int16 type ) const
{
   MC2_ASSERT( type < m_primitiveDefaultMapSize );
   MC2_ASSERT( type >= 0 );
      
   // Only supports one primitive per type, which is what be have
   // in reality.
   const primitiveVector_t& vec = m_primitiveDefaultMap[ type ];
   
   MC2_ASSERT( vec.size() == 1 );
   
   int16 primType = vec.front()->getType();
   switch ( primType ) {
      case TilePrimitiveFeature::polygon:
         // Dar var visst inte fler.
         return true;
      default:
         return false;
   }
   return false;
}

void 
ServerTileMapFormatDesc::addArgTransferMap( int featureType, 
                                            int primitiveType ) {

   argTransferMap_t* argMap = new argTransferMap_t;

   switch ( primitiveType ) {
      case ( TilePrimitiveFeature::bitmap ) :
      case ( TilePrimitiveFeature::circle ) :
         argMap->insert( make_pair( TileArgNames::coord,
                  make_pair(0, TileArgNames::coord) ) );
         break;
      case ( TilePrimitiveFeature::line ) :
      case ( TilePrimitiveFeature::polygon ) :
         argMap->insert( make_pair( TileArgNames::coords,
                  make_pair(0, TileArgNames::coords) ) );
         break;
      default:
         break;
   }  

   // Check if it's one of the custom pois that has explicit images.
   // Also check if it's one of the features that has explicit levels.
   switch ( featureType ) {
      case ( TileFeature::custom_poi_2 ) :
      case ( TileFeature::custom_poi_4 ) :
      case ( TileFeature::custom_poi_7 ) :
      case ( TileFeature::custom_poi_10 ) :
      case ( TileFeature::custom_poi_15 ) :
      case ( TileFeature::custom_poi_20 ) :
      case ( TileFeature::custom_poi_30 ) :
      case TileFeature::special_custom_poi_2:
      case TileFeature::special_custom_poi_4:
      case TileFeature::special_custom_poi_7:
      case TileFeature::special_custom_poi_10:
      case TileFeature::special_custom_poi_15:
      case TileFeature::special_custom_poi_20:
      case TileFeature::special_custom_poi_30:
         argMap->
            insert( make_pair( TileArgNames::image_name,
                               make_pair(0, TileArgNames::image_name) ) );
         break;
      case ( TileFeature::street_class_0 ) :
      case ( TileFeature::street_class_0_ramp ) :
      case ( TileFeature::street_class_1 ) :
      case ( TileFeature::street_class_1_ramp ) :
      case ( TileFeature::street_class_2 ) :
      case ( TileFeature::street_class_2_ramp ) :
      case ( TileFeature::street_class_3 ) :
      case ( TileFeature::street_class_4 ) :
         argMap->insert( make_pair( TileArgNames::level,
                  make_pair(0, TileArgNames::level) ) );
         argMap->insert( make_pair( TileArgNames::level_1,
                  make_pair(0, TileArgNames::level_1) ) );
         break;
      case TileFeature::event:
         argMap->insert( make_pair( TileArgNames::time,
                                    make_pair( 0, TileArgNames::time ) ) );
         argMap->insert( make_pair( TileArgNames::ext_id,
                                    make_pair( 0, TileArgNames::ext_id ) ) );
         argMap->insert( make_pair( TileArgNames::duration,
                                    make_pair( 0, TileArgNames::duration ) ) );
         break;
      default:
         break;
   }
   
   m_argsPerType.insert( make_pair( featureType, argMap ) );
}

void
ServerTileMapFormatDesc::createCategories(LangTypes::language_t lang)
{
   ServerTileCategories categoryList;
   typedef multimap<ServerTileCategories::category_t, TileFeature::tileFeature_t>
      catMultiMap_t ;   
   catMultiMap_t categories;
   
   for ( uint32 i = 0; i < c_tileNotices.size(); ++i ) {
      categories.insert( make_pair( c_tileNotices[i].m_category,
                                    TileFeature::tileFeature_t(
                                       c_tileNotices[i].m_tileFeatureType )) );
   }
   // Other should not be there. We will think about that.
   categories.erase( ServerTileCategories::OTHER );

   // Put the unique keys into a set.
   typedef set<ServerTileCategories::category_t> categorySet_t;
   categorySet_t uniqueCategories;
   for( catMultiMap_t::const_iterator it = categories.begin();
        it != categories.end();
        ++it ) {
      uniqueCategories.insert( it->first );
   }

   // Create the stuff that is going to be saved.
   m_nbrCategoryPoiTypes = categories.size();
   delete [] m_categoryPoiTypes;
   m_categoryPoiTypes =
      new TileFeature::tileFeature_t[ m_nbrCategoryPoiTypes ];
   m_categories.clear();
   m_categories.reserve( uniqueCategories.size() );
   int currentPoiTypeIdx = 0;
   
   for( categorySet_t::const_iterator it = uniqueCategories.begin();
        it != uniqueCategories.end();
        ++it ) {
      pair<catMultiMap_t::const_iterator, catMultiMap_t::const_iterator>
         range = categories.equal_range( *it );
      // Get the category name.
      MC2String curCatName (
         categoryList.getCategoryEntry( *it ).getName( lang ) );
      // Convert from map string to utf-8.
      curCatName = UTF8Util::mc2ToUtf8( curCatName );
      int curCatID = categoryList.getCategoryEntry( *it ).getID();
      const TileFeature::tileFeature_t* firstCat =
         m_categoryPoiTypes + currentPoiTypeIdx;
      int nbrPoiTypes = std::distance( range.first, range.second );
      bool enabled = categoryList.getCategoryEntry( *it ).enabledByDefault();
      for ( catMultiMap_t::const_iterator jt = range.first;
            jt != range.second;
            ++jt ) {
         m_categoryPoiTypes[currentPoiTypeIdx++] = jt->second;
      }
      m_categories.push_back( TileMapCategoryNotice(curCatName.c_str(),
                                                    curCatID,
                                                    enabled,
                                                    nbrPoiTypes,
                                                    firstCat) );
   }
}

void
ServerTileMapFormatDesc::initTileSizesForLayer( 
                                     uint32 layerNbr,
                                     const tileSettings_t& tileSettings )
{
   // Make sure that m_tileScaleByLayer is large enough to be indexed by
   // layerNbr. Sorry about this, but initTileSizesForLayer in TMFD makes
   // this assumption.
   m_tileScaleByLayer.resize( 
         MAX( m_tileScaleByLayer.size(), layerNbr + 1 ), NULL );

   // Call the method in TileMapFormatDesc.
   TileMapFormatDesc::initTileSizesForLayer( layerNbr,
         tileSettings.m_meters,
         tileSettings.m_pixels,
         tileSettings.m_dpi,
         tileSettings.m_zoomFactor,
         tileSettings.m_exchangeTileFactor,
         tileSettings.m_detailLevels );
}

// Min level for roads should be above buas.
const int ServerTileMapFormatDesc::c_minRoadFeatureLevel     = 3;
const int ServerTileMapFormatDesc::c_defaultRoadFeatureLevel = 8;
// Max level for roads should be below the route.
const int ServerTileMapFormatDesc::c_maxRoadFeatureLevel     = 13;

int 
ServerTileMapFormatDesc::getRoadLevelWithOffset( int levelWithoutOffset ) 
{
   int level = c_defaultRoadFeatureLevel + levelWithoutOffset;
   if ( level < c_minRoadFeatureLevel ) {
      level = c_minRoadFeatureLevel;
   }
   if ( level > c_maxRoadFeatureLevel ) {
      level = c_maxRoadFeatureLevel;
   }
   return level;
}


void
ServerTileMapFormatDesc::getAllImportances( 
         const MC2SimpleString& param,
         vector<MC2SimpleString>& params ) const
{

   int nbrImp = getNbrImportances( param );

   params.reserve( nbrImp );
   // First add the specified param, then the others.
   params.push_back( param );
   for ( int i = 0; i < nbrImp; ++i ) {
      TileMapParams curParam( param );
      if ( i != curParam.getImportanceNbr() ) {
         curParam.setImportanceNbr( i );
         params.push_back( curParam.getAsString() );
      }
   }
}      

bool 
ServerTileMapFormatDesc::sameTileSettingsForLayerID( int layerID, 
                                                     int otherLayerID ) const {
   return m_tileSettingsByLayerNbr[ 
      getLayerNbrFromID( layerID ) ] ==
      m_tileSettingsByLayerNbr[ 
      getLayerNbrFromID( otherLayerID ) ];
}


ServerTileMapFormatDesc::
TileNotice::TileNotice(ServerTileCategories::category_t category,
                       int tilefeatureType,
                       ImageTable::ImageCode image,
                       ItemTypes::pointOfInterest_t poiType )
      : m_image( image ),
        m_tileFeatureType( tilefeatureType ),
        m_poiType( poiType ),
        m_category( category ),
        m_level ( -1 ) {
}      







class TilesNoticeContructor {
public:
typedef set<MC2SimpleString> ParamSet;

   void addToCollection( const ServerTileMapFormatDesc& mapDesc, 
                         TileCollectionNotice& collection,
                         ParamSet& otherParams,
                         const char* paramStr ) {

      if ( ! TileMapParamTypes::isMap( paramStr ) ) {
         otherParams.insert( paramStr );
         return;
      }


      // Tile map. Add to data structure.
      TileMapParams param( paramStr );

      map<int,int>::const_iterator findit = 
         collection.m_indexByLayerID.find( param.getLayer() );
      if ( findit == collection.m_indexByLayerID.end() ) {
         // Layer not yet added.
         // Check if we can reuse an existing layer.
         bool added = false;
         for ( map<int,int>::const_iterator jt = 
                  collection.m_indexByLayerID.begin();
               jt != collection.m_indexByLayerID.end(); ++jt ) {
            // Check if this is a layer with same properties as the
            // one we want to add.
            if ( mapDesc.sameTileSettingsForLayerID( param.getLayer(),
                                                     jt->first ) ) {
               // Same settings for these two layers. 
               // Use the same index in m_tilesForAllDetails then.
               collection.m_indexByLayerID[ param.getLayer() ] = jt->second;
               added = true;
            }
         }
         if ( ! added ) {
            collection.m_indexByLayerID[ param.getLayer() ] = 
               collection.m_tilesForAllDetails.size();
            collection.m_tilesForAllDetails.
               push_back( TilesForAllDetailsNotice() );
         } 
      }

      // Now the layer should be added.
      findit = collection.m_indexByLayerID.find( param.getLayer() );
      MC2_ASSERT( findit != collection.m_indexByLayerID.end() );

      // Get the TilesForAllDetailsNotice.
      TilesForAllDetailsNotice& allDetails = 
         collection.m_tilesForAllDetails[ findit->second ];

      
      // Add new notices if needed.
      if ( (int) allDetails.m_tilesNotice.size() <= 
           param.getDetailLevel() ) {
         allDetails.m_tilesNotice.resize( param.getDetailLevel() + 1 );   
      }
     
      TilesNotice& notice = 
         allDetails.m_tilesNotice[ param.getDetailLevel() ];

      notice.updateWithParam( param );
   }

   template <typename T>
   void
   addToCollection( const ServerTileMapFormatDesc& desc, 
                    TileCollectionNotice& collection,
                    ParamSet& otherParams,
                    const T& begin, const T& end )
   {
      for ( T it = begin; it != end; ++it ) {
         addToCollection( desc,  collection, otherParams,
                          (*it).getAsString().c_str() );
      }
   }

};


void ServerTileMapFormatDesc::
getAllNotices( TileCollectionNotice& collection,
               paramSet_t& otherParams,
               const MC2BoundingBox& bbox,
               const set<int>& layers,
               int useGzip, 
               LangTypes::language_t language,
               uint32 minScale,
               const paramSet_t* extraParams ) const 
{
   TilesNoticeContructor constr;
   // Yes, This function looks almost like TileMapFormatDesc::getAllParams
   // at least for now.
   // It will be optimized for TileCollectionNotice later.

   if ( extraParams != NULL ) {
      paramSet_t::const_iterator it = extraParams->begin();
      paramSet_t::const_iterator itEnd = extraParams->end();
      for (; it != itEnd; ++it ) {
         constr.addToCollection( *this, collection, otherParams,
                                 (*it).c_str() );
      }
   }

   // Divide the bounding box into many (slow)   
   const int64 maxArea = SQUARE( int64(704074)*int64(16) );

   if ( bbox.getArea() > maxArea ) {
      mc2dbg << "[TFMD]: Dividing bounding box" << endl;
      // Divide the bounding box into 4 new ones
      vector<MC2BoundingBox> bboxVec;
      bboxVec.resize( 4, bbox );
      int32 halfLat = bbox.getMinLat() + bbox.getHeight() / 2;
      int32 halfLon = bbox.getMinLon() + bbox.getLonDiff() / 2;
      bboxVec[0].setMinLat(halfLat);
      bboxVec[1].setMinLat(halfLat);
      bboxVec[2].setMaxLat(halfLat);
      bboxVec[3].setMaxLat(halfLat);
      bboxVec[0].setMaxLon(halfLon);
      bboxVec[1].setMinLon(halfLon);
      bboxVec[2].setMinLon(halfLon);
      bboxVec[3].setMaxLon(halfLon);
      for ( int i = 0; i < 4; ++i ) {
         getAllNotices( collection, otherParams,
                        bboxVec[i], layers, useGzip, language,
                        minScale, NULL );
      }
      return;
   }
   
   // FIXME: Do not use the scale in the for loop.
   set<int>::const_iterator it = layers.begin();
   set<int>::const_iterator itEnd = layers.end();
   for ( ; it != itEnd; ++it ) {
      set<int> tmpLayers;
      tmpLayers.insert(*it);
      uint32 layerNbr = getLayerNbrFromID( *it );
     
      // Get the interesting scales.
      vector<uint32> interestingScales;
      interestingScales.push_back( minScale );
      uint32 prevScale = 0;
      uint32 lastScale = prevScale;
      for ( uint32 i = 0; i < m_tileScaleByLayer[ layerNbr ]->size(); 
            ++i ) {
         if ( prevScale > minScale ) {
            interestingScales.push_back( prevScale );
         }
         prevScale = (*m_tileScaleByLayer[ layerNbr ])[ i ].second + 1;
         lastScale = prevScale;
      }
      
      for ( uint32 scaleIdx = 0; scaleIdx < interestingScales.size(); 
            ++scaleIdx ) {

         // Enlarge the boundingbox with 400 pixels.
         MC2BoundingBox extraBBox( bbox );

         int nextScale = lastScale;
         if ( scaleIdx + 1 < interestingScales.size() ) {
            nextScale = interestingScales[ scaleIdx + 1 ];
         }

         map<uint32, ParamsNotice> paramsByLayerNbr;
         vector<TileMapParams> paramVectorOut;
         vector<TileMapParams> reserveParams;
         MC2Coordinate center ( bbox.getCenter() );
         innerCreateParams( paramsByLayerNbr,
                            paramVectorOut,
                            &reserveParams,
                            extraBBox,
                            center,                        
                            interestingScales[ scaleIdx ],
                            useGzip,
                            language,
                            tmpLayers,
                            (const RouteID*)NULL, // No need to cache RouteID
                            MAX_INT32 );
         constr.addToCollection( *this, collection, otherParams,
                                 paramVectorOut.begin(), paramVectorOut.end() );
         constr.addToCollection( *this, collection, otherParams,
                                 reserveParams.begin(), reserveParams.end() );

         // Also add the strings of the reserve params.
         for ( vector<TileMapParams>::const_iterator jt = 
                  reserveParams.begin(); jt != reserveParams.end();
               ++jt ) {
            TileMapParams tmpParam( *jt );
            tmpParam.setTileMapType( TileMapTypes::tileMapStrings );
            tmpParam.setLanguageType( language );
            constr.addToCollection( *this, collection, otherParams,
                                    tmpParam.getAsString().c_str() );
         }
      }
   }
}

const char* ServerTileMapFormatDesc::
getPOIImageName( ItemTypes::pointOfInterest_t poi ) const
{
   // first find TileFeature type then find image name from tile feature type

   typeByPOI_t::const_iterator it = 
      m_tileFeatureTypeByPOIType.find( poi );
   if ( it == m_tileFeatureTypeByPOIType.end() ) {
      return "";
   }

   imageByType_t::const_iterator image = 
      m_usedImageByTileFeatureType.find( (*it).second );
   if ( image == m_usedImageByTileFeatureType.end() ) {
      return "";
   }

   return (*image).second.c_str();
} 

MC2String ServerTileMapFormatDesc::
getItemImageName( ItemTypes::itemType itemType ) const
{
   return "";
}

MC2String ServerTileMapFormatDesc::
getPOIImageName( int tileFeatureType ) const 
{
   imageByType_t::const_iterator it = m_imageByTileFeatureType.find( tileFeatureType );
   if ( it == m_imageByTileFeatureType.end() ) {   
      return "";
   }
   return (*it).second.c_str();
}

bool 
ServerTileMapFormatDesc::checkFeatScaleRange( int tileType, 
                                        const TileMapParams& param ) const
{
   // Check scale range.
   scaleRangeByType_t::const_iterator it = m_scaleRangeByType.find( tileType );
   if ( it != m_scaleRangeByType.end() ) {
      // Found the scale range.
      
      // There's a default scale range for this feature type.
      const scaleRange_t& featureRange = it->second;

      // Get the max and min scale for this tile param.
      pair<uint16,uint16> paramRange = getScaleRange( param );
     
      // Don't include if:
      // tile.min > feature.max || feature.min > tile.max
      if ( paramRange.first > featureRange.second ||
           featureRange.first > paramRange.second ) {
         // Feature is not inside parameter scale range.
         mc2dbg8 << "[STMFD]::checkFeatScaleRange( " 
                 << tileType << ", " 
                 << param.getAsString() << ")"
                 << " returns false, since tile (" 
                 << paramRange.first << ", " 
                 << paramRange.second << "), feature ("
                 << featureRange.first << ", " 
                 << featureRange.second << ")" << endl;
         return false;
      } else {
         return true;
      }
      
   } else {
      // No scale range means that the scale is OK.
      return true;
   }
}

TileFeature::tileFeature_t 
ServerTileMapFormatDesc::
getTileFeatureForPOI( ItemTypes::pointOfInterest_t poiType ) const {

   typeByPOI_t::const_iterator it = 
      m_tileFeatureTypeByPOIType.find( poiType );
   if ( it == m_tileFeatureTypeByPOIType.end() ) {
      return TileFeature::nbr_tile_features;
   }
   return static_cast<TileFeature::tileFeature_t>( it->second );
}

