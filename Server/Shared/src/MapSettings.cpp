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

#include "StringUtility.h"
#include "MapSettings.h"
#include <algorithm>
#include "MapUtility.h"
#include "Packet.h"
#include "RedLineSettings.h"
#include "DrawingProjection.h"
#include "MC2Point.h"


// Used for searching in a MapSettingsVector.
struct lessThanMapSetting:
   public binary_function<const MapSetting*, 
                          const MapSetting*, bool> {
   bool operator () ( const MapSetting* x, 
                      const MapSetting* y ) const
      {
         if ( x->m_featureType < y->m_featureType ) {
            return true;
         } else if ( x->m_featureType > y->m_featureType ) {
            return false;
         } else if ( (x->m_highScaleLevel >= y->m_highScaleLevel &&
                      x->m_highScaleLevel <= y->m_lowScaleLevel ) ||
                     (x->m_lowScaleLevel >= y->m_highScaleLevel &&
                      x->m_lowScaleLevel <= y->m_lowScaleLevel ) )
         {
            return false;
         } else {
            if ( x->m_highScaleLevel > y->m_highScaleLevel ) {
               return true;
            } else {
               return false;
            }
         }
      }
};
namespace {
const uint32 DEFAULT_SHOW_TYPE_MASK =
   MapSettings::MAP |
   MapSettings::TOPOGRAPH_MAP |
   MapSettings::ROUTE |
   MapSettings::CITY_CENTRE ;
}

MapSettings::MapSettings() 
      : m_poiBits( 256, true ),
        m_showTypeMask( ::DEFAULT_SHOW_TYPE_MASK ),
        m_maxOneCoordPerPixelForMap( true ),
        m_maxOneCoordPerPixelForRoute( true ),
        m_navigatorCrossingMap( false ),
        m_settings(),
        m_searchSetting(),
        m_size( 0 ),
        m_mapRights(),
        m_imageSet( ImageTable::DEFAULT )
{
   m_redLineSettings = new RedLineSettings;
   m_drawingProjection = NULL;
   m_drawingProjIncluded = false;
   // Header: nbr mapsettings, 
   // [showMap,showTopographMap,showPOI,showRoute,drawScale]
   // Body: poisettings:
   //       MapSettings: Added in addSettingFor()
   // 4 for the length.
   m_size = 4 + 4 + 4 + m_poiBits.size()/8 + 4 + 4 + 4;
   m_size += m_redLineSettings->getSizeInPacket();

   // Make sure companies are not shown in the map.
   setPOI(ItemTypes::company, false);
   
}

MapSettings::MapSettings( const MapSettings& other )
    : m_poiBits( 256, true ),
      m_showTypeMask( ::DEFAULT_SHOW_TYPE_MASK ),
      m_maxOneCoordPerPixelForMap( true ),
      m_maxOneCoordPerPixelForRoute( true ),
      m_navigatorCrossingMap( false ),
      m_settings(),
      m_searchSetting(),
      m_size( 0 ),
      m_imageSet( ImageTable::DEFAULT )
{
   m_redLineSettings = new RedLineSettings;
   m_drawingProjection = NULL;
   m_drawingProjIncluded = false;
   m_size = 4 + 4 + 4 + m_poiBits.size()/8 + 4 + 4 + 4;
   m_size += m_redLineSettings->getSizeInPacket();

   // Make sure companies are not shown in the map.
   setPOI(ItemTypes::company, false);
   Packet p( other.getSize() * 2 );
   int pos = REQUEST_HEADER_SIZE;
   other.saveToPacket( &p, pos );
   int newpos = REQUEST_HEADER_SIZE;
   this->loadFromPacket( &p, newpos, true );
   m_size = newpos - REQUEST_HEADER_SIZE;
   MC2_ASSERT( pos == newpos );
}

MapSettings::~MapSettings() {
   for ( uint32 i = 0; i < m_settings.size() ; i++ ) {
      delete m_settings[ i ];
   }
   delete m_redLineSettings;
   delete m_drawingProjection;
}

void MapSettings::setRedLineSettings( const RedLineSettings& settings )
{
   m_size -= m_redLineSettings->getSizeInPacket();
   *m_redLineSettings = settings;
   m_size += m_redLineSettings->getSizeInPacket();
}

void
MapSettings::setDrawingProjection( DrawingProjection* projection )
{
   delete m_drawingProjection;
   m_drawingProjection = projection;
   m_size += m_drawingProjection->getSizeInPacket();
   m_drawingProjIncluded = true;
}

void 
MapSettings::addSettingFor( GfxFeature::gfxFeatureType featureType,
                            uint32 highScaleLevel,
                            uint32 lowScaleLevel,
                            bool onMap,
                            bool borderOnMap,
                            bool textOnMap,
                            DrawSettings::drawstyle_t drawStyle,
                            DrawSettings::symbol_t drawSymbol,
                            int lineWidth,
                            GDUtils::Color::CoolColor drawColor,
                            int borderLineWidth,
                            GDUtils::Color::CoolColor borderColor,
                            int fontSize,
                            GDUtils::Color::CoolColor fontColor,
                            const char* fontName,
                            bool copyString )
{
   MapSetting* setting = new MapSetting( featureType, highScaleLevel,
                                         lowScaleLevel, onMap, borderOnMap,
                                         textOnMap,
                                         drawStyle, drawSymbol,
                                         lineWidth, drawColor,
                                         borderLineWidth, borderColor,
                                         fontSize, fontColor,
                                         fontName, copyString );
   m_settings.push_back( setting );
   m_size += setting->getSize();
}


bool 
MapSettings::getSettingFor( GfxFeature::gfxFeatureType featureType,
                            uint32 scaleLevel,
                            bool& onMap,
                            bool& borderOnMap,
                            bool& textOnMap,
                            DrawSettings::drawstyle_t& drawStyle,
                            DrawSettings::symbol_t& drawSymbol,
                            int& lineWidth,
                            GDUtils::Color::CoolColor& drawColor,
                            int& borderLineWidth,
                            GDUtils::Color::CoolColor& borderColor,
                            int& fontSize,
                            GDUtils::Color::CoolColor& fontColor,
                            const char*& fontName,
                            bool copyString )
{
   // search
   MapSetting* res = getSettingFor( featureType, scaleLevel, false );

   if ( res != NULL ) {
      onMap = res->m_onMap;
      borderOnMap = res->m_borderOnMap;
      textOnMap = res->m_textOnMap;
      drawStyle = res->m_drawStyle;
      drawSymbol = res->m_drawSymbol;
      lineWidth = res->m_lineWidth;
      drawColor = res->m_drawColor;
      borderLineWidth = res->m_borderLineWidth;
      borderColor = res->m_borderColor;
      fontSize = res->m_fontSize;
      fontColor = res->m_fontColor;
      if ( copyString ) {
         fontName= StringUtility::newStrDup( res->m_fontName );
      } else {
         fontName= res->m_fontName;
      }
      
      return true;
   } else {
      return false;
   }
}


MapSetting* 
MapSettings::getSettingFor( GfxFeature::gfxFeatureType featureType,
                            uint32 scaleLevel,
                            bool copySetting )
{
   // search
   m_searchSetting.m_featureType = featureType;
   m_searchSetting.m_highScaleLevel = scaleLevel;
   m_searchSetting.m_lowScaleLevel = scaleLevel;

   MapSettingVector::const_iterator it = 
      lower_bound( m_settings.begin(), 
                   m_settings.end(), 
                   &m_searchSetting,
                   lessThanMapSetting() );

   if ( it != m_settings.end() &&
        lessThanMapSetting()( (*it), &m_searchSetting ) ==
        lessThanMapSetting()( &m_searchSetting, (*it) ) ) 
   {
      MapSetting* res = (*it);
      if ( copySetting ) {
         res = new MapSetting( *(*it), true );
      }

      return res;
   } else {
      DEBUG2(mc2log << warn << "No setting found for featureType " 
             << int(featureType) << " scaleLevel " << scaleLevel << endl;);
      return NULL;
   }
}

      
void 
MapSettings::setPOI( ItemTypes::pointOfInterest_t poiType, bool onMap )
{
   m_poiBits[poiType] = onMap;
}


void 
MapSettings::saveToPacket( Packet* p, int& pos ) const
{   
   // Header
   int startPos = pos;

   // LengthHelper lh(pos);

   // nbr mapsettings
   p->incWriteLong( pos, m_settings.size() );

    
   // Bools
   uint32 bools = (m_maxOneCoordPerPixelForMap << 26 |
                   m_maxOneCoordPerPixelForRoute << 25 |
                   m_navigatorCrossingMap << 24 |
                   1 << 22 );   // Include length

   p->incWriteLong( pos, bools );
   p->incWriteLong( pos, m_showTypeMask );
   p->incWriteLong( pos, static_cast<uint32>( m_imageSet ) );

   // Body
   // poisettings
   uint32 index = 0;
   while ( index + 7 < m_poiBits.size() ) {
      // Write hole unsinged char
      uint8 bits = 0;
      for ( uint32 i = 0 ; i < 8 ; i++ ) {
         bits |= (m_poiBits[index+i] << (7-i));
      }
      p->incWriteByte( pos, bits );
      index += 8;
   }

   // MapRights
   m_mapRights.save( p, pos );

   // MapSettings
   for ( uint32 i = 0 ; i < m_settings.size() ; i++ ) {
      m_settings[i]->saveToPacket( p, pos );
   }

   int lengthPos = pos;
   // Make room for the length
   p->incWriteLong( pos, 0 );

   // lh.allocLength( pos )
   
   // Write new data.
   m_redLineSettings->save( p, pos );

   // Include projection
   if( m_drawingProjIncluded ) {
      p->incWriteLong( pos, 1 );
   } else {
      p->incWriteLong( pos, 0 );
   }
   
   // Write drawing projection
   if( m_drawingProjIncluded ) {
      m_drawingProjection->save( p, pos );
   }
   
   p->incWriteString( pos, m_tileMapParamStr.c_str() );

   // End new data.
   // Write back length
   uint32 length = pos - startPos;
   p->incWriteLong( lengthPos, length );

   // lh.updateLength( pos )
}


void 
MapSettings::loadFromPacket( const Packet* p, int& pos, bool copyString )
{
   int startPos = pos;
   // Header
   
   // nbr mapsettings
   uint32 nbrMapSettings = p->incReadLong( pos );
   m_settings.reserve( nbrMapSettings );

   uint32 bools = p->incReadLong( pos );
   m_showTypeMask = p->incReadLong( pos );
   m_imageSet = static_cast<ImageTable::ImageSet>( p->incReadLong( pos ) );

   m_maxOneCoordPerPixelForMap = (bools>>26) & 1;
   m_maxOneCoordPerPixelForRoute = (bools>>25) & 1;
   m_navigatorCrossingMap = (bools>>24) & 1;
   bool includeLength = ( bools >> 22 ) & 1;
        
   // Body
   // poisettings
   uint32 index = 0;
   while ( index + 7 < m_poiBits.size() ) {
      // Read hole unsinged char
      uint8 bits = p->incReadByte( pos );
      for ( uint32 i = 0 ; i < 8 ; i++ ) {
         m_poiBits[index+i] = (bits >>(7-i))&0x1;
      }
      index += 8;
   }

   // MapRights
   m_mapRights.load( p, pos );

   // MapSettings
   for ( uint32 i = 0 ; i < nbrMapSettings ; i++ ) {
      MapSetting* mapSetting = new MapSetting();
      mapSetting->loadFromPacket( p, pos, copyString );
      m_settings.push_back( mapSetting );
   }

   sort();

   if ( ! includeLength ) {
      mc2dbg8 << "[MSettings]: Length is NOT included" << endl;
      // Old version of the stuff.
      return;
   } else {
      mc2dbg8 << "[MSettings]: Length is included" << endl;
   }

   // Has length. Read it.
   uint32 length = p->incReadLong( pos );
   // Do special new stuff.
   m_redLineSettings->load( p, pos );

   // Include Drawing projection
   int proj = p->incReadLong( pos );
   if( proj == 0 ) {
      m_drawingProjIncluded = false;
   } else {
      m_drawingProjIncluded = true;
   }
   
   // Load drawing projection
   if( m_drawingProjIncluded ) {
      m_drawingProjection = DrawingProjection::create( p, pos );
   } else {
      m_drawingProjection = NULL;
   }

   // nothing more to read
   if ( pos == startPos + (int)length ) {
      m_tileMapParamStr = "";
      return;
   }

   char* str = NULL;
   p->incReadString( pos, str );
   m_tileMapParamStr = str ? str : "";

   // Skip if necessary
   if ( uint32(pos) != startPos + length ) {
      mc2dbg << "[MSettings]: Skipping unknown data" << endl;
   }
   // Increase pos to skip unknown data.
   pos = startPos + length;
}


bool getImageShowSetting( struct MapSettingsTypes::ImageSettings&
                          imageSettings,
                          GfxFeature::gfxFeatureType type )
{
   switch ( type ) {
      case GfxFeature::STREET_MAIN :
         return imageSettings.m_image_show_street_main;
         break;
      case GfxFeature::STREET_FIRST :
         return imageSettings.m_image_show_street_first;
         break;
      case GfxFeature::STREET_SECOND :
         return imageSettings.m_image_show_street_second;
         break;
      case GfxFeature::STREET_THIRD :
         return imageSettings.m_image_show_street_third;
         break;
      case GfxFeature::STREET_FOURTH :
         return imageSettings.m_image_show_street_fourth;
         break;
      case GfxFeature::BUILTUP_AREA :
         return imageSettings.m_image_show_builtup_area;
         break;
      case GfxFeature::PARK :
         return imageSettings.m_image_show_park;
         break;
      case GfxFeature::FOREST :
         return imageSettings.m_image_show_forest;
         break;
      case GfxFeature::BUILDING :
         return imageSettings.m_image_show_building;
         break;
      case GfxFeature::WATER :
         return imageSettings.m_image_show_water;
         break;
      case GfxFeature::ISLAND :
         return imageSettings.m_image_show_island;
         break;
      case GfxFeature::PEDESTRIANAREA :
         return imageSettings.m_image_show_pedestrianarea;
         break;
      case GfxFeature::AIRCRAFTROAD :
         return imageSettings.m_image_show_aircraftroad;
         break;
      case GfxFeature::LAND :
         return imageSettings.m_image_show_land;
         break;
      case GfxFeature::BUILTUP_AREA_SQUARE :
         return imageSettings.m_image_show_builtup_area;
         break;
      case GfxFeature::BUILTUP_AREA_SMALL :
         return imageSettings.m_image_show_builtup_area;
         break;
      default:
         return true;
         break;
   }
}


void 
MapSettings::mergeImageSettings( struct MapSettingsTypes::ImageSettings&
                                 imageSettings ) 
{
   // Handle all booleans
   for ( GfxFeature::gfxFeatureType type = GfxFeature::STREET_MAIN ;
         type <= GfxFeature::BUILTUP_AREA_SMALL ;
         type = GfxFeature::gfxFeatureType( type + 1 ) ) 
   {
      if ( ! getImageShowSetting( imageSettings, type ) ) {
         for ( int i = CONTINENT_LEVEL ; 
               i <= DETAILED_STREET_LEVEL ; 
               i++ ) 
         {
            MapSetting* setting = getSettingFor( type, i );
            if ( setting != NULL ) {
               setting->m_onMap = false;
               setting->m_borderOnMap = false;
            }
         }
      }
   }
}


void
MapSettings::sort() {
   ::sort( m_settings.begin(), m_settings.end(), 
           lessThanMapSetting() );
}


MapSettings* 
MapSettings::createDefaultMapSettings( 
   MapSettingsTypes::defaultMapSetting setting ) 
{
   MapSettings* res = NULL;

   switch ( setting ) {
      case MapSettingsTypes::MAP_SETTING_STD:
         res = createDefaultSTDMapSettings();
         break;
      case MapSettingsTypes::MAP_SETTING_WAP :
         res = createDefaultWAPMapSettings();
         break;
   }

   return res;
}


//**********************************************************************
// createDefaultSTDMapSettings
//**********************************************************************


MapSettings* 
MapSettings::createDefaultSTDMapSettings() {
   MapSettings* res = new MapSettings(); 
   // LAND
   res->addSettingFor( GfxFeature::LAND,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::YELLOWISH ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::YELLOWISH ),
                       16,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // BUILTUP_AREA
   res->addSettingFor( GfxFeature::BUILTUP_AREA,
                       CONTINENT_LEVEL,
                       SMALL_COUNTY_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::SALMON ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::SALMON ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::BUILTUP_AREA,
                       MUNICIPAL_LEVEL,
                       MUNICIPAL_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSALMON ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSALMON ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );


   res->addSettingFor( GfxFeature::BUILTUP_AREA,
                       CITY_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PEACHPUFF ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PEACHPUFF ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // BUILTUP_AREA_SQUARE
   res->addSettingFor( GfxFeature::BUILTUP_AREA_SQUARE,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::SYMBOL,
                       DrawSettings::SQUARE_3D_SYMBOL,
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // BUILTUP_AREA_SMALL
   res->addSettingFor( GfxFeature::BUILTUP_AREA_SMALL,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::SYMBOL,
                       DrawSettings::SMALL_CITY_SYMBOL,
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // WATER
   res->addSettingFor( GfxFeature::WATER,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSLATEBLUE ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSLATEBLUE ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::NEWDARKBLUE ),
                       "VeraBI.ttf" );
   
   // ISLAND
   res->addSettingFor( GfxFeature::ISLAND,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::YELLOWISH ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::YELLOWISH ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "VeraBI.ttf" );

   // PARK
   res->addSettingFor( GfxFeature::PARK,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::NEWDARKGREEN ),
                       "VeraBI.ttf" );
   
   // NATIONALPARK
   res->addSettingFor( GfxFeature::NATIONALPARK,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::CLOSED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::NEWDARKGREEN ),
                       "VeraBI.ttf" );
   
   // FOREST
   res->addSettingFor( GfxFeature::FOREST,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::NEWDARKGREEN ),
                       "VeraBI.ttf" );
   
   // ROUTE
   res->addSettingFor( GfxFeature::ROUTE,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::RED ), // Not relevant 
                       3, //  Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::RED ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // ROUTE_ORIGIN
   res->addSettingFor( GfxFeature::ROUTE_ORIGIN,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::SYMBOL,
                       DrawSettings::ROUTE_ORIGIN_SYMBOL,
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );
   
   // ROUTE_DESTINATION
   res->addSettingFor( GfxFeature::ROUTE_DESTINATION,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::SYMBOL,
                       DrawSettings::ROUTE_DESTINATION_SYMBOL,
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );
   
   // STREET_MAIN
   res->addSettingFor( GfxFeature::STREET_MAIN,
                       CONTINENT_LEVEL,
                       CONTINENT_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,// Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,// Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),// Not relevant
                       1, //  Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ), // Not relevant 
                       10,// Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),// Not relevant
                       "Vera.ttf" );// Not relevant

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       COUNTRY_LEVEL,
                       COUNTY_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       SMALL_COUNTY_LEVEL,
                       CITY_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       SMALL_CITY_LEVEL,
                       DISTRICT_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       9,
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       11,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       15,
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // STREET_FIRST
   res->addSettingFor( GfxFeature::STREET_FIRST,
                       CONTINENT_LEVEL,
                       COUNTY_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE, // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTCORAL ), // Not relevant
                       10, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" ); // Not relevant

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       SMALL_COUNTY_LEVEL,
                       SMALL_COUNTY_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ), // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTCORAL ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       MUNICIPAL_LEVEL,
                       CITY_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ), // Not relevant
                       2,
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTCORAL ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       SMALL_CITY_LEVEL,
                       DISTRICT_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       BLOCK_LEVEL,
                       BLOCK_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       PART_OF_BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       7,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       9,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       11,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 
   
   // STREET_SECOND
   res->addSettingFor( GfxFeature::STREET_SECOND,
                       CONTINENT_LEVEL,
                       COUNTY_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,  // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ), // Not relevant
                       8, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" );  // Not relevant

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       SMALL_COUNTY_LEVEL,
                       MUNICIPAL_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ), // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       CITY_LEVEL,
                       CITY_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ), // Not relevant
                       2,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       SMALL_CITY_LEVEL,
                       DISTRICT_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ),
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ),
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       7,
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ),
                       9,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );
   
   // STREET_THIRD
   res->addSettingFor( GfxFeature::STREET_THIRD,
                       CONTINENT_LEVEL,
                       MUNICIPAL_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,  // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGOLDENRODYELLOW ),// Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ), // Not relevant
                       8, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" );  // Not relevant

   res->addSettingFor( GfxFeature::STREET_THIRD,
                       CITY_LEVEL,
                       SMALL_CITY_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGOLDENRODYELLOW ),// Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_THIRD,
                       DISTRICT_LEVEL,
                       BLOCK_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGOLDENRODYELLOW ),
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_THIRD,
                       PART_OF_BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGOLDENRODYELLOW ),
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::GREY ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_THIRD,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       7,
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGOLDENRODYELLOW ),
                       9,
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSLATEGREY ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // STREET_FOURTH
   res->addSettingFor( GfxFeature::STREET_FOURTH,
                       CONTINENT_LEVEL,
                       SMALL_CITY_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,  // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ), // Not relevant
                       8, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" );  // Not relevant

   res->addSettingFor( GfxFeature::STREET_FOURTH,
                       DISTRICT_LEVEL,
                       DISTRICT_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),// Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_FOURTH,
                       BLOCK_LEVEL,
                       BLOCK_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ), // Not relevant
                       2,
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_FOURTH,
                       PART_OF_BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::GREY ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_FOURTH,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       7,
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),
                       9,
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSLATEGREY ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // Add the poi:s that should be shown
   // Feel free to change this, everything is currently set to be shown
   // except for companies...
   res->setPOI(ItemTypes::company, false);
   res->setPOI(ItemTypes::airport, true);
   res->setPOI(ItemTypes::amusementPark, true);
   res->setPOI(ItemTypes::atm, true);
   res->setPOI(ItemTypes::automobileDealership, true);
   res->setPOI(ItemTypes::bank, true);
   res->setPOI(ItemTypes::bowlingCentre, true);
   res->setPOI(ItemTypes::busStation, true);
   res->setPOI(ItemTypes::businessFacility, true);
   res->setPOI(ItemTypes::cafe, true);
   res->setPOI(ItemTypes::casino, true);
   res->setPOI(ItemTypes::cinema, true);
   res->setPOI(ItemTypes::cityCentre, true);
   res->setPOI(ItemTypes::cityHall, true);
   res->setPOI(ItemTypes::communityCentre, true);
   res->setPOI(ItemTypes::commuterRailStation, true);
   res->setPOI(ItemTypes::courtHouse, true);
   res->setPOI(ItemTypes::exhibitionOrConferenceCentre, true);
   res->setPOI(ItemTypes::ferryTerminal, true);
   res->setPOI(ItemTypes::frontierCrossing, true);
   res->setPOI(ItemTypes::golfCourse, true);
   res->setPOI(ItemTypes::groceryStore, true);
   res->setPOI(ItemTypes::historicalMonument, true);
   res->setPOI(ItemTypes::hospital, true);
   res->setPOI(ItemTypes::hotel, true);
   res->setPOI(ItemTypes::iceSkatingRink, true);
   res->setPOI(ItemTypes::library, true);
   res->setPOI(ItemTypes::marina, true);
   res->setPOI(ItemTypes::motoringOrganisationOffice, true);
   res->setPOI(ItemTypes::museum, true);
   res->setPOI(ItemTypes::nightlife, true);
   res->setPOI(ItemTypes::openParkingArea, true);
   res->setPOI(ItemTypes::parkAndRide, true);
   res->setPOI(ItemTypes::parkingGarage, true);
   res->setPOI(ItemTypes::petrolStation, true);
   res->setPOI(ItemTypes::policeStation, true);
   res->setPOI(ItemTypes::publicSportAirport, true);
   res->setPOI(ItemTypes::railwayStation, true);
   res->setPOI(ItemTypes::recreationFacility, true);
   res->setPOI(ItemTypes::rentACarFacility, true);
   res->setPOI(ItemTypes::restArea, true);
   res->setPOI(ItemTypes::restaurant, true);
   res->setPOI(ItemTypes::tollRoad, true);
   res->setPOI(ItemTypes::school, true);
   res->setPOI(ItemTypes::shoppingCentre, true);
   res->setPOI(ItemTypes::skiResort, true);
   res->setPOI(ItemTypes::sportsActivity, true);
   res->setPOI(ItemTypes::sportsCentre, true);
   res->setPOI(ItemTypes::subwayStation, true);
   res->setPOI(ItemTypes::theatre, true);
   res->setPOI(ItemTypes::touristAttraction, true);
   res->setPOI(ItemTypes::touristOffice, true);
   res->setPOI(ItemTypes::university, true);
   res->setPOI(ItemTypes::vehicleRepairFacility, true);
   res->setPOI(ItemTypes::winery, true);
   res->setPOI(ItemTypes::unknownType, true);
   res->setPOI(ItemTypes::postOffice, true);
   res->setPOI(ItemTypes::tramStation, true);
   
   res->sort();

   return res;
}


//**********************************************************************
// createDefaultWAPMapSettings
//**********************************************************************


MapSettings* 
MapSettings::createDefaultWAPMapSettings() {
   MapSettings* res = new MapSettings();
   // LAND
   res->addSettingFor( GfxFeature::LAND,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::YELLOWISH ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::YELLOWISH ),
                       16,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // BUILTUP_AREA
   res->addSettingFor( GfxFeature::BUILTUP_AREA,
                       CONTINENT_LEVEL,
                       SMALL_COUNTY_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::SALMON ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::SALMON ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::BUILTUP_AREA,
                       MUNICIPAL_LEVEL,
                       MUNICIPAL_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSALMON ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSALMON ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::BUILTUP_AREA,
                       CITY_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PEACHPUFF ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PEACHPUFF ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // BUILTUP_AREA_SQUARE
   res->addSettingFor( GfxFeature::BUILTUP_AREA_SQUARE,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::SYMBOL,
                       DrawSettings::SQUARE_3D_SYMBOL,
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // BUILTUP_AREA_SMALL
   res->addSettingFor( GfxFeature::BUILTUP_AREA_SMALL,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::SYMBOL,
                       DrawSettings::SMALL_CITY_SYMBOL,
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // WATER
   res->addSettingFor( GfxFeature::WATER,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSLATEBLUE ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSLATEBLUE ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::NEWDARKBLUE ),
                       "VeraBI.ttf" );
   
   // ISLAND
   res->addSettingFor( GfxFeature::ISLAND,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::YELLOWISH ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::YELLOWISH ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "VeraBI.ttf" );

   // PARK
   res->addSettingFor( GfxFeature::PARK,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::NEWDARKGREEN ),
                       "VeraBI.ttf" );
   
   // NATIONALPARK
   res->addSettingFor( GfxFeature::NATIONALPARK,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::CLOSED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::NEWDARKGREEN ),
                       "VeraBI.ttf" );
   
   // FOREST
   res->addSettingFor( GfxFeature::FOREST,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::NEWDARKGREEN ),
                       "VeraBI.ttf" );
   
   // ROUTE
   res->addSettingFor( GfxFeature::ROUTE,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant 
                       3, //  Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // ROUTE_ORIGIN
   res->addSettingFor( GfxFeature::ROUTE_ORIGIN,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::SYMBOL,
                       DrawSettings::ROUTE_ORIGIN_SYMBOL,
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );
   
   // ROUTE_DESTINATION
   res->addSettingFor( GfxFeature::ROUTE_DESTINATION,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::SYMBOL,
                       DrawSettings::ROUTE_DESTINATION_SYMBOL,
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );
   
   // STREET_MAIN
   res->addSettingFor( GfxFeature::STREET_MAIN,
                       CONTINENT_LEVEL,
                       CONTINENT_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,// Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,// Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),// Not relevant
                       1, //  Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),// Not relevant 
                       10,// Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),// Not relevant
                       "Vera.ttf" );// Not relevant

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       COUNTRY_LEVEL,
                       COUNTY_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ), // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       SMALL_COUNTY_LEVEL,
                       CITY_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       SMALL_CITY_LEVEL,
                       DISTRICT_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),
                       9,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       11,
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),
                       15,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // STREET_FIRST
   res->addSettingFor( GfxFeature::STREET_FIRST,
                       CONTINENT_LEVEL,
                       COUNTY_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE, // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::INDIANRED ), // Not relevant
                       10, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" ); // Not relevant

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       SMALL_COUNTY_LEVEL,
                       SMALL_COUNTY_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ), // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       MUNICIPAL_LEVEL,
                       CITY_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ), // Not relevant
                       2,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       SMALL_CITY_LEVEL,
                       DISTRICT_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       BLOCK_LEVEL,
                       BLOCK_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       PART_OF_BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),
                       7,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       9,
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),
                       11,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 
   
   // STREET_SECOND
   res->addSettingFor( GfxFeature::STREET_SECOND,
                       CONTINENT_LEVEL,
                       SMALL_COUNTY_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,  // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGREY ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       8, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" );  // Not relevant

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       MUNICIPAL_LEVEL,
                       CITY_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGREY ), // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       SMALL_CITY_LEVEL,
                       DISTRICT_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGREY ),
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGREY ),
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       7,
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGREY ),
                       9,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );
   
   // STREET_THIRD
   res->addSettingFor( GfxFeature::STREET_THIRD,
                       CONTINENT_LEVEL,
                       CITY_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,  // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       8, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" );  // Not relevant

   res->addSettingFor( GfxFeature::STREET_THIRD,
                       SMALL_CITY_LEVEL,
                       SMALL_CITY_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ), // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_THIRD,
                       DISTRICT_LEVEL,
                       BLOCK_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_THIRD,
                       PART_OF_BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_THIRD,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       7,
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),
                       9,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // STREET_FOURTH
   res->addSettingFor( GfxFeature::STREET_FOURTH,
                       CONTINENT_LEVEL,
                       SMALL_CITY_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,  // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGREY ), // Not relevant
                       8, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" );  // Not relevant

   res->addSettingFor( GfxFeature::STREET_FOURTH,
                       DISTRICT_LEVEL,
                       DISTRICT_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),// Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_FOURTH,
                       BLOCK_LEVEL,
                       BLOCK_LEVEL,
                       false,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ), // Not relevant
                       2,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_FOURTH,
                       PART_OF_BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,//3,
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),
                       3,//5,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_FOURTH,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       false,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       5,//7,
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),
                       7,//9,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // Add the poi:s that should be shown.
   // Only the most important POI:s are shown.
   // Feel free to change this if you like...
   res->setPOI(ItemTypes::company, false);
   res->setPOI(ItemTypes::airport, true);
   res->setPOI(ItemTypes::amusementPark, false);
   res->setPOI(ItemTypes::atm, false);
   res->setPOI(ItemTypes::automobileDealership, false);
   res->setPOI(ItemTypes::bank, false);
   res->setPOI(ItemTypes::bowlingCentre, false);
   res->setPOI(ItemTypes::busStation, false);
   res->setPOI(ItemTypes::businessFacility, false);
   res->setPOI(ItemTypes::cafe, false);
   res->setPOI(ItemTypes::casino, false);
   res->setPOI(ItemTypes::cinema, false);
   res->setPOI(ItemTypes::cityCentre, false);
   res->setPOI(ItemTypes::cityHall, false);
   res->setPOI(ItemTypes::communityCentre, false);
   res->setPOI(ItemTypes::commuterRailStation, true);
   res->setPOI(ItemTypes::courtHouse, false);
   res->setPOI(ItemTypes::exhibitionOrConferenceCentre, false);
   res->setPOI(ItemTypes::ferryTerminal, false);
   res->setPOI(ItemTypes::frontierCrossing, false);
   res->setPOI(ItemTypes::golfCourse, false);
   res->setPOI(ItemTypes::groceryStore, false);
   res->setPOI(ItemTypes::historicalMonument, false);
   res->setPOI(ItemTypes::hospital, false);
   res->setPOI(ItemTypes::hotel, true);
   res->setPOI(ItemTypes::iceSkatingRink, false);
   res->setPOI(ItemTypes::library, false);
   res->setPOI(ItemTypes::marina, false);
   res->setPOI(ItemTypes::motoringOrganisationOffice, false);
   res->setPOI(ItemTypes::museum, false);
   res->setPOI(ItemTypes::nightlife, true);
   res->setPOI(ItemTypes::openParkingArea, false);
   res->setPOI(ItemTypes::parkAndRide, false);
   res->setPOI(ItemTypes::parkingGarage, true);
   res->setPOI(ItemTypes::petrolStation, false);
   res->setPOI(ItemTypes::policeStation, false);
   res->setPOI(ItemTypes::publicSportAirport, false);
   res->setPOI(ItemTypes::railwayStation, false);
   res->setPOI(ItemTypes::recreationFacility, false);
   res->setPOI(ItemTypes::rentACarFacility, false);
   res->setPOI(ItemTypes::restArea, false);
   res->setPOI(ItemTypes::restaurant, true);
   res->setPOI(ItemTypes::tollRoad, true);
   res->setPOI(ItemTypes::school, false);
   res->setPOI(ItemTypes::shoppingCentre, false);
   res->setPOI(ItemTypes::skiResort, false);
   res->setPOI(ItemTypes::sportsActivity, false);
   res->setPOI(ItemTypes::sportsCentre, false);
   res->setPOI(ItemTypes::subwayStation, false);
   res->setPOI(ItemTypes::theatre, false);
   res->setPOI(ItemTypes::touristAttraction, true);
   res->setPOI(ItemTypes::touristOffice, false);
   res->setPOI(ItemTypes::university, false);
   res->setPOI(ItemTypes::vehicleRepairFacility, false);
   res->setPOI(ItemTypes::winery, false);
   res->setPOI(ItemTypes::unknownType, false);
   res->setPOI(ItemTypes::postOffice, false);
   res->setPOI(ItemTypes::tramStation, false);
 
   res->sort();
   
   return res;
}

//**********************************************************************
// createDefaultTileMapSettings
//**********************************************************************


MapSettings* 
MapSettings::createDefaultTileMapSettings( bool highEndDevice ) {

   // These settings are used in 
   // GfxTileFeatureMapProcessor::includeFeatureType
   // to determine if a type should be included in the tile map

   MapSettings* res = new MapSettings(); 
   // LAND
   res->addSettingFor( GfxFeature::LAND,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::YELLOWISH ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::YELLOWISH ),
                       16,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // BUILTUP_AREA
   res->addSettingFor( GfxFeature::BUILTUP_AREA,
                       CONTINENT_LEVEL,
                       SMALL_COUNTY_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::SALMON ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::SALMON ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::BUILTUP_AREA,
                       MUNICIPAL_LEVEL,
                       MUNICIPAL_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSALMON ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSALMON ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );


   res->addSettingFor( GfxFeature::BUILTUP_AREA,
                       CITY_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PEACHPUFF ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PEACHPUFF ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // BUILTUP_AREA_SQUARE
   res->addSettingFor( GfxFeature::BUILTUP_AREA_SQUARE,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::SYMBOL,
                       DrawSettings::SQUARE_3D_SYMBOL,
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // BUILTUP_AREA_SMALL
   res->addSettingFor( GfxFeature::BUILTUP_AREA_SMALL,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::SYMBOL,
                       DrawSettings::SMALL_CITY_SYMBOL,
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // WATER
   res->addSettingFor( GfxFeature::WATER,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSLATEBLUE ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTSLATEBLUE ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::NEWDARKBLUE ),
                       "VeraBI.ttf" );
   
   // ISLAND
   res->addSettingFor( GfxFeature::ISLAND,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::YELLOWISH ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::YELLOWISH ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "VeraBI.ttf" );

   // PARK
   res->addSettingFor( GfxFeature::PARK,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::NEWDARKGREEN ),
                       "VeraBI.ttf" );
   
   // NATIONALPARK
   res->addSettingFor( GfxFeature::NATIONALPARK,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::CLOSED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::NEWDARKGREEN ),
                       "VeraBI.ttf" );
   
   // FOREST
   res->addSettingFor( GfxFeature::FOREST,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::FILLED,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::DARKSEAGREEN ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::NEWDARKGREEN ),
                       "VeraBI.ttf" );
   
   // ROUTE
   res->addSettingFor( GfxFeature::ROUTE,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::RED ), // Not relevant 
                       3, //  Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::RED ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // ROUTE_ORIGIN
   res->addSettingFor( GfxFeature::ROUTE_ORIGIN,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::SYMBOL,
                       DrawSettings::ROUTE_ORIGIN_SYMBOL,
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );
   
   // ROUTE_DESTINATION
   res->addSettingFor( GfxFeature::ROUTE_DESTINATION,
                       CONTINENT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::SYMBOL,
                       DrawSettings::ROUTE_DESTINATION_SYMBOL,
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );
   
   // STREET_MAIN
   res->addSettingFor( GfxFeature::STREET_MAIN,
                       CONTINENT_LEVEL,
                       CONTINENT_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,// Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,// Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),// Not relevant
                       1, //  Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ), // Not relevant 
                       10,// Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),// Not relevant
                       "Vera.ttf" );// Not relevant

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       COUNTRY_LEVEL,
                       COUNTY_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       SMALL_COUNTY_LEVEL,
                       CITY_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       SMALL_CITY_LEVEL,
                       DISTRICT_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       9,
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_MAIN,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       11,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       15,
                       GDUtils::Color::makeColor( GDUtils::Color::DARKGREY ),
                       12,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // STREET_FIRST
   res->addSettingFor( GfxFeature::STREET_FIRST,
                       CONTINENT_LEVEL,
                       COUNTY_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE, // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTCORAL ), // Not relevant
                       10, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" ); // Not relevant

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       SMALL_COUNTY_LEVEL,
                       SMALL_COUNTY_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ), // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTCORAL ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       MUNICIPAL_LEVEL,
                       CITY_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ), // Not relevant
                       2,
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTCORAL ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       SMALL_CITY_LEVEL,
                       DISTRICT_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       BLOCK_LEVEL,
                       BLOCK_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       PART_OF_BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       7,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 

   res->addSettingFor( GfxFeature::STREET_FIRST,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE, 
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       9,
                       GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                       11,
                       GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                       10,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" ); 
   
   // STREET_SECOND

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       CONTINENT_LEVEL,
                       COUNTY_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,  // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ), // Not relevant
                       8, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" );  // Not relevant

   if ( false /*highEndDevice*/ ) {

      res->addSettingFor( GfxFeature::STREET_SECOND,
                          MUNICIPAL_LEVEL,
                          CITY_LEVEL,
                          false,
                          true,
                          true,
                          DrawSettings::LINE, 
                          DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                          1, // Not relevant
                          GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ), // Not relevant
                          2,
                          GDUtils::Color::makeColor( GDUtils::Color::LIGHTCORAL ),
                          10,
                          GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                          "Vera.ttf" ); 

      /* not
         res->addSettingFor( GfxFeature::STREET_SECOND,
         SMALL_COUNTY_LEVEL,
         SMALL_COUNTY_LEVEL,
         false,
         true,
         true,
         DrawSettings::LINE, 
         DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
         1, // Not relevant
         GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ), // Not relevant
         1,
         GDUtils::Color::makeColor( GDUtils::Color::LIGHTCORAL ),
         10,
         GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
         "Vera.ttf" ); 
      */

      res->addSettingFor( GfxFeature::STREET_SECOND,
                          COUNTRY_LEVEL,
                          COUNTY_LEVEL,
                          false,
                          true,
                          true,
                          DrawSettings::LINE,
                          DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                          1,
                          GDUtils::Color::makeColor( GDUtils::Color::GHOSTWHITE ),
                          1,
                          GDUtils::Color::makeColor( GDUtils::Color::TOMATO ),
                          12,
                          GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                          "Vera.ttf" );
   }

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       SMALL_COUNTY_LEVEL,
                       MUNICIPAL_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ), // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       CITY_LEVEL,
                       CITY_LEVEL,
                       false,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ), // Not relevant
                       2,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       SMALL_CITY_LEVEL,
                       DISTRICT_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ),
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       BLOCK_LEVEL,
                       PART_OF_BLOCK_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       3,
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ),
                       5,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   res->addSettingFor( GfxFeature::STREET_SECOND,
                       DETAILED_STREET_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       7,
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ),
                       9,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // STREET_THIRD
   if ( false /*highEndDevice*/ ) {
   res->addSettingFor( GfxFeature::STREET_THIRD,
                       CONTINENT_LEVEL,
                       COUNTY_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,  // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::PALEGOLDENROD ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ), // Not relevant
                       8, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" );  // Not relevant
   }

   res->addSettingFor( GfxFeature::STREET_THIRD,
                       CONTINENT_LEVEL,
                       SMALL_CITY_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,  // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGOLDENRODYELLOW ),// Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ), // Not relevant
                       8, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" );  // Not relevant

   res->addSettingFor( GfxFeature::STREET_THIRD,
                       /*highEndDevice ? BLOCK_LEVEL :*/ DISTRICT_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::LIGHTGOLDENRODYELLOW ),// Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::GOLDENROD ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );

   // STREET_FOURTH
   res->addSettingFor( GfxFeature::STREET_FOURTH,
                       CONTINENT_LEVEL,
                       /*highEndDevice ? SMALL_CITY_LEVEL :*/ DISTRICT_LEVEL,
                       false,
                       false,
                       false,
                       DrawSettings::LINE,  // Not relevant
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ), // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ), // Not relevant
                       8, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ), // Not relevant
                       "Vera.ttf" );  // Not relevant

   res->addSettingFor( GfxFeature::STREET_FOURTH,
                       BLOCK_LEVEL,
                       DETAILED_STREET_LEVEL,
                       true,
                       true,
                       true,
                       DrawSettings::LINE,
                       DrawSettings::SQUARE_3D_SYMBOL, // Not relevant
                       1, // Not relevant
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),// Not relevant
                       1,
                       GDUtils::Color::makeColor( GDUtils::Color::WHITESMOKE ),
                       8,
                       GDUtils::Color::makeColor( GDUtils::Color::BLACK ),
                       "Vera.ttf" );


   // Add the poi:s that should be shown
   res->setPOI(ItemTypes::company, true);
   res->setPOI(ItemTypes::airport, true);
   res->setPOI(ItemTypes::amusementPark, true);
   res->setPOI(ItemTypes::atm, true);
   res->setPOI(ItemTypes::automobileDealership, true);
   res->setPOI(ItemTypes::bank, true);
   res->setPOI(ItemTypes::bowlingCentre, true);
   res->setPOI(ItemTypes::busStation, true);
   res->setPOI(ItemTypes::businessFacility, true);
   res->setPOI(ItemTypes::cafe, true);
   res->setPOI(ItemTypes::casino, true);
   res->setPOI(ItemTypes::cinema, true);
   res->setPOI(ItemTypes::cityCentre, true);
   res->setPOI(ItemTypes::cityHall, true);
   res->setPOI(ItemTypes::communityCentre, true);
   res->setPOI(ItemTypes::commuterRailStation, true);
   res->setPOI(ItemTypes::courtHouse, true);
   res->setPOI(ItemTypes::exhibitionOrConferenceCentre, true);
   res->setPOI(ItemTypes::ferryTerminal, true);
   res->setPOI(ItemTypes::frontierCrossing, true);
   res->setPOI(ItemTypes::golfCourse, true);
   res->setPOI(ItemTypes::groceryStore, true);
   res->setPOI(ItemTypes::historicalMonument, true);
   res->setPOI(ItemTypes::hospital, true);
   res->setPOI(ItemTypes::hotel, true);
   res->setPOI(ItemTypes::iceSkatingRink, true);
   res->setPOI(ItemTypes::library, true);
   res->setPOI(ItemTypes::marina, true);
   res->setPOI(ItemTypes::motoringOrganisationOffice, true);
   res->setPOI(ItemTypes::museum, true);
   res->setPOI(ItemTypes::nightlife, true);
   res->setPOI(ItemTypes::openParkingArea, true);
   res->setPOI(ItemTypes::parkAndRide, true);
   res->setPOI(ItemTypes::parkingGarage, true);
   res->setPOI(ItemTypes::petrolStation, true);
   res->setPOI(ItemTypes::policeStation, true);
   res->setPOI(ItemTypes::publicSportAirport, true);
   res->setPOI(ItemTypes::railwayStation, true);
   res->setPOI(ItemTypes::recreationFacility, true);
   res->setPOI(ItemTypes::rentACarFacility, true);
   res->setPOI(ItemTypes::restArea, true);
   res->setPOI(ItemTypes::restaurant, true);
   res->setPOI(ItemTypes::tollRoad, true);
   res->setPOI(ItemTypes::school, true);
   res->setPOI(ItemTypes::shoppingCentre, true);
   res->setPOI(ItemTypes::skiResort, true);
   res->setPOI(ItemTypes::sportsActivity, true);
   res->setPOI(ItemTypes::sportsCentre, true);
   res->setPOI(ItemTypes::subwayStation, true);
   res->setPOI(ItemTypes::theatre, true);
   res->setPOI(ItemTypes::touristAttraction, true);
   res->setPOI(ItemTypes::touristOffice, true);
   res->setPOI(ItemTypes::university, true);
   res->setPOI(ItemTypes::vehicleRepairFacility, true);
   res->setPOI(ItemTypes::winery, true);
   res->setPOI(ItemTypes::wlan, true);
   res->setPOI(ItemTypes::unknownType, true);
   res->setPOI(ItemTypes::postOffice, true);
   res->setPOI(ItemTypes::tramStation, true);
   
   res->sort();

   return res;
}


const char* 
MapSettings::defaultMapSettingToString( 
   MapSettingsTypes::defaultMapSetting setting )
{
   switch ( setting ) {
      case MapSettingsTypes::MAP_SETTING_WAP :
         return "wap";
         break;
      case MapSettingsTypes::MAP_SETTING_STD :
      default:
         return "std";
         break;
   } 
}


MapSettingsTypes::defaultMapSetting 
MapSettings::defaultMapSettingFromString( 
   const char* str, MapSettingsTypes::defaultMapSetting defaultSetting ) 
{
   if ( StringUtility::strcasecmp( str, "WAP" ) == 0 ) {
      return MapSettingsTypes::MAP_SETTING_WAP;
   } else if ( StringUtility::strcasecmp( str, "STD" ) == 0 ) {
      return MapSettingsTypes::MAP_SETTING_STD;
   } else { // other
      return defaultSetting;
   }
}


int 
MapSettings::imageSettingsToString( 
   char* target, struct MapSettingsTypes::ImageSettings& imageSettings )
{
   byte tmpStr[ 6 ]; // 14 bits used(2) + padding(3) + reserve(1)
   int pos = 0;

   tmpStr[ pos++ ] = ( (imageSettings.m_image_show_street_main<<7) |
                       (imageSettings.m_image_show_street_first<<6) |
                       (imageSettings.m_image_show_street_second<<5) |
                       (imageSettings.m_image_show_street_third<<4) |
                       (imageSettings.m_image_show_street_fourth<<3) |
                       (imageSettings.m_image_show_builtup_area<<2) |
                       (imageSettings.m_image_show_park<<1) |
                       (imageSettings.m_image_show_forest) );
   tmpStr[ pos++ ] = ( (imageSettings.m_image_show_building<<7) |
                       (imageSettings.m_image_show_water<<6) |
                       (imageSettings.m_image_show_island<<5) |
                       (imageSettings.m_image_show_pedestrianarea<<4) |
                       (imageSettings.m_image_show_aircraftroad<<3) |
                       (imageSettings.m_image_show_land<<2) );

   float64 rem = float64(pos) / 3;
   while ( rem != rint( rem ) ) {
      tmpStr[ pos ] = char( pos );
      pos++;
      rem = float64(pos) / 3;
   }

   int length = StringUtility::URLEncode( target, (char*)tmpStr , pos );
   target[ length ] = '\0';

   return length;
}


bool 
MapSettings::imageSettingsFromString( 
   const MC2String& str, 
   struct MapSettingsTypes::ImageSettings& imageSettings )
{
   byte data[ str.size() + 1 ];

   int length = StringUtility::URLDecode( data, str.c_str() );
   if ( length != -1 ) {
      if ( length > 2 ) {
         int pos = 0;
         uint8 dataByte = data[ pos++ ];
         imageSettings.m_image_show_street_main = ( dataByte>>7 & 0x1 );
         imageSettings.m_image_show_street_first = ( dataByte>>6 & 0x1 );
         imageSettings.m_image_show_street_second = ( dataByte>>5 & 0x1 );
         imageSettings.m_image_show_street_third = ( dataByte>>4 & 0x1 );
         imageSettings.m_image_show_street_fourth = ( dataByte>>3 & 0x1 );
         imageSettings.m_image_show_builtup_area = ( dataByte>>2 & 0x1 );
         imageSettings.m_image_show_park = ( dataByte>>1 & 0x1 );
         imageSettings.m_image_show_forest = ( dataByte & 0x1 );
         dataByte = data[ pos++ ];
         imageSettings.m_image_show_building = ( dataByte>>7 & 0x1 );
         imageSettings.m_image_show_water = ( dataByte>>6 & 0x1 );
         imageSettings.m_image_show_island = ( dataByte>>5 & 0x1 );
         imageSettings.m_image_show_pedestrianarea = ( dataByte>>4 & 0x1 );
         imageSettings.m_image_show_aircraftroad = ( dataByte>>3 & 0x1 );
         imageSettings.m_image_show_land = ( dataByte>>2 & 0x1 );

         return true;
      } else {
         mc2dbg4 << "MapSettings::imageSettingsFromString  data too short "
                 << " length " << length  << " = " << data<< endl;
         return false;
      }
   } else {
      DEBUG4(mc2log << error << "MapSetting::imageSettingsFromString "
                    "URLDecode failed" << endl;);
      return false;
   }
}

void MapSettings::addShowMask( uint32 mask ) {
   m_showTypeMask |= mask;
}

void MapSettings::setImageSet( ImageTable::ImageSet imageSet ) {
   m_imageSet = imageSet;
}

ostream& 
MapSettings::dump( ostream& out ) {
   out << "MapSettings" << endl
       << " poiBits" << endl;
   uint32 j = 0;
   for ( uint32 i = 0 ; i < 256 ; i++ ) {
      if ( j == 0 ) {
         out << "      ";
      }
      out << m_poiBits[i];
      j++;
      if ( j >= 20 ) {
         j = 0;
         out << endl;
      } else {
         out << ", ";
      }
   }
   out << endl;
   out << " showMap " << getShowMap() << endl
       << " showTopographMap " << getShowTopographMap() << endl
       << " showPOI " << getShowPOI() << endl
       << " showRoute " << getShowRoute() << endl
       << " showTraffic " << getShowTraffic() << endl
       << " drawScale " << getDrawScale() << endl
       << " showCityCentres " << getShowCityCentres() << endl
       << " maxOneCoordPerPixelForMap " << m_maxOneCoordPerPixelForMap << endl
       << " maxOneCoordPerPixelForRoute " << m_maxOneCoordPerPixelForRoute
       << endl
       << " navigatorCrossingMap " << m_navigatorCrossingMap << endl;
   out << " MapSettingVector size " << m_settings.size() << endl;
   for ( MapSettingVector::iterator it = m_settings.begin() ; 
         it != m_settings.end() ; ++it )
   {
      (*it)->dump( out );
   }
   

   return out;
}
