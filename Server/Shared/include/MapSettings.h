/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPSETTINGS_H
#define MAPSETTINGS_H

#include "config.h"
#include "ItemTypes.h"
#include "DrawSettings.h"
#include "GfxFeature.h"
#include "MapSetting.h"
#include "MapSettingsTypes.h"
#include "GDColor.h"
#include "MapRights.h"
#include "ImageTable.h"
#include <vector>

class Packet;
class RedLineSettings;
class DrawingProjection;

/**
 * Settings for a map. The settings include what to draw and how to draw
 * it.
 *   
 */
class MapSettings {
public:
   /// Types to be shown in the final gfx map
   enum ShowType {
      /// Map items
      MAP             = ( 1 << 0 ),
      /// Topographical information
      TOPOGRAPH_MAP   = ( 1 << 1 ),
      /// Point of Interest items
      POI             = ( 1 << 2 ),
      /// Routes
      ROUTE           = ( 1 << 3 ),
      /// Traffic information POIs
      TRAFFIC         = ( 1 << 4 ),
      /// City centres
      CITY_CENTRE     = ( 1 << 5 ),
      /// Whether or not to draw scale
      SCALE           = ( 1 << 6 ),
      /// Event POIs
      EVENT           = ( 1 << 7 ),
   };

      /**
       * Constructor, creates empty but valid MapSettings.
       */
      MapSettings();
      
      /**
       * Destructor deletes all members.
       */
      ~MapSettings();

      /**
       *    Copy constructor.
       */
      MapSettings( const MapSettings& other );
      
      /**
       * Setting for a GfxFeatureType.
       * Note that most default values really do NOT make any sense
       * to use, except if the settings is for a feature that should
       * not present on the map.
       * 
       * @param featureType The type of feature to set MapSettings for.
       * @param highScaleLevel The highest scaleLevel the setting is for.
       * @param lowScaleLevel The lowest scaleLevel the setting is for.
       * @param onMap If featuretype should be on map.
       * @param borderOnMap If featuretypes border should be on map.
       * @param textOnMap If featuretypes text should be on map.
       * @param drawStyle Type of drawing, filled, line etc..
       * @param drawSymbol If drawStyle if symbol then this indicates
       *                   which symbol.
       * @param lineWidth Linewidth in pixels.
       * @parm drawColor Color of the line.
       * @param fontSize The fonts size for this featureType.
       * @param fontName Name of the font to use.
       * @param copyString If string should be copied, default false.
       */
      void addSettingFor( GfxFeature::gfxFeatureType featureType,
                          uint32 highScaleLevel,
                          uint32 lowScaleLevel,
                          bool onMap,
                          bool borderOnMap,
                          bool textOnMap = false,
                          DrawSettings::drawstyle_t drawStyle = 
                              DrawSettings::LINE,
                          DrawSettings::symbol_t drawSymbol = 
                              DrawSettings::SQUARE_3D_SYMBOL,
                          int lineWidth = 1,
                          GDUtils::Color::CoolColor drawColor = 
                          static_cast< GDUtils::Color::CoolColor >
                          ( 0x000000 ),
                          int borderLineWidth = 1,
                          GDUtils::Color::CoolColor borderColor = 
                          static_cast< GDUtils::Color::CoolColor >
                          ( 0x000000 ),
                          int fontSize = 10,
                          GDUtils::Color::CoolColor fontColor = 
                          static_cast< GDUtils::Color::CoolColor >
                          ( 0x000000 ),
                          const char* fontName = "Vera.ttf",
                          bool copyString = false );
                       
      /**
       * Get settings for a GfxFeatureType.
       *
       * @param featureType The type of feature to get MapSettings for.
       * @param scaleLevel The scaleLevel the settings should be for.
       * @param onMap If featuretype should be on map at all.
       * @param borderOnMap If featuretypes border should be on map.
       * @param textOnMap If featuretypes text should be on map.
       * @param drawStyle Type of drawing, filled, line etc..
       * @param drawSymbol If drawStyle if symbol then this indicates
       *                   which symbol.
       * @param lineWidth Linewidth in pixels.
       * @parm drawColor Color of the line.
       * @param fontSize The fonts size for this featureType.
       * @param fontName Name of the font to use.
       * @param copyString If strings should be copied, default false.
       * @return True if there is a setting for the specified featureType.
       */ 
      bool getSettingFor( GfxFeature::gfxFeatureType featureType,
                          uint32 scaleLevel,
                          bool& onMap,
                          bool& borderOnMap,
                          bool& texOnMap,
                          DrawSettings::drawstyle_t& drawStyle,
                          DrawSettings::symbol_t& drawSymbol,
                          int& lineWidth,
                          GDUtils::Color::CoolColor& drawColor,
                          int& borderLineWidth,
                          GDUtils::Color::CoolColor& borderColor,
                          int& fontSize,
                          GDUtils::Color::CoolColor& fontColor,
                          const char*& fontName,
                          bool copyString = false );


      /**
       * Get settings for a GfxFeatureType.
       *
       * @param featureType The type of feature to get MapSettings for.
       * @param scaleLevel The scaleLevel the settings should be for.
       * @param copySetting If MapSetting should be copid, default false.
       * @return MapSetting for the specified featureType and scaleLevel,
       *         or NULL if no such setting.
       */ 
      MapSetting* getSettingFor( GfxFeature::gfxFeatureType featureType,
                                 uint32 scaleLevel,
                                 bool copySetting = false );


      /**
       * Sets if poiType POIs should be on map.
       *
       * @param poiType The type of POI to set on map for.
       * @param onMap If true then POI should be on map, false not.
       */
      void setPOI( ItemTypes::pointOfInterest_t poiType, bool onMap );
      
      
      /**
       * Gets if poiType POIs should be on map.
       *
       * @param poiType The type of POI to get on map for.
       * @return True if POI should be on map, false if not.
       */
      inline bool getPOI( ItemTypes::pointOfInterest_t poiType ) const;

      
      /**
       * Set the map content bools.
       *
       * @param showMap If map data should be present.
       * @param showTopographMap If topographical backgroup should be used.
       * @param showPOI If POIs should be in map.
       * @param showRoute If route should be in map.
       * @param showCityCentres If the city centres should be in map.
       */
      inline void setMapContent( bool showMap, bool showTopographMap,
                                 bool showPOI, bool showRoute );


      /**
       * If map data should be present.
       */
      inline bool getShowMap() const;


      /**
       * If topographical backgroup should be used.
       */
      inline bool getShowTopographMap() const;


      /**
       * If POIs should be in map.
       */
      inline bool getShowPOI() const;

      
      /**
       * If route should be in map.
       */
      inline bool getShowRoute() const;


      /**
       * If traffic information should be in map.
       */
      inline bool getShowTraffic() const;

      /**
       * Whether or not to show event pois.
       * @return Whether or not to show event pois.
       */
      inline bool getShowEvents() const;

      /**
       * Set if traffic information should be in map.
       *
       * @param showTraffic The new value of showTraffic.
       */
      inline void setShowTraffic( bool showTraffic );

      
      /**
       * Set if scale should be drawn.
       */
      inline void setDrawScale( bool drawScale );


      /**
       * If scale should be drawn.
       */
      inline bool getDrawScale() const;

      /**
       * Set if city centres should be drawn.
       */
      inline void setShowCityCentres( bool showCityCentres );

      /**
       * If city centres should be drawn.
       */
      inline bool getShowCityCentres() const;

      /**
       * Set if maximum one coordinate per pixel should be allowed
       * for the map. Only valid if map is present.
       */
      inline void setMaxOneCoordPerPixelForMap( bool value );
      
      
      /**
       * If maximum one coordinate per pixel should be allowed
       * for the map. Only valid if map is present.
       */
      inline bool getMaxOneCoordPerPixelForMap() const;
      
      
      /**
       * Set if maximum one coordinate per pixel should be allowed
       * for the route. Only valid if route is present.
       */
      inline void setMaxOneCoordPerPixelForRoute( bool value );
      
      
      /**
       * If maximum one coordinate per pixel should be allowed
       * for the route. Only valid if route is present.
       */
      inline bool getMaxOneCoordPerPixelForRoute() const;

      
      /**
       * Set if the map is a navigator crossing map.
       */
      inline void setNavigatorCrossingMap( bool value );
      
      
      /**
       * If the map is a navigator crossing map.
       */
      inline bool getNavigatorCrossingMap() const;

      
      /**
       * Writes the current content of the MapSettings to packet.
       * @param p The packet to write to.
       * @param pos The position in p to start at.
       */
      void saveToPacket( Packet* p, int& pos ) const;
      
      
      /**
       * Loads the content of the packet.
       * @param p The packet to read from.
       * @param pos The position in p to start at.
       * @param copyString If strings from packet should be copied, default
       *        false.
       */
      void loadFromPacket( const Packet* p, int& pos, bool copyString = false );


      /**
       * The size of the MapSettings when packed into a Packet.
       * @return The number of bytes the MapSettings take in a Packet.
       */
      inline uint32 getSize() const;

      
      /**
       * Sets the settings from imageSettings in this MapSettings.
       * @param imageSettings The settings to merge into this MapSettings.
       */
      void mergeImageSettings( 
         struct MapSettingsTypes::ImageSettings& imageSettings );

      
      /**
       * Sorts the settings. Call after adding settings if settings should
       * se searched.
       */
      void sort();


      /**
       * Makes a MapSettigns with the default settings.
       * @param setting The type of default setting.
       * @return A new MapSettings with the apropriate settings.
       */
      static MapSettings* createDefaultMapSettings(
         MapSettingsTypes::defaultMapSetting setting );


      /**
       * Makes a MapSettigns with the default standard settings.
       * @return A new MapSettings with the apropriate settings.
       */
      static MapSettings* createDefaultSTDMapSettings();


      /**
       * Makes a MapSettigns with the default settings for a WAP device.
       * @return A new MapSettings with the apropriate settings.
       */
      static MapSettings* createDefaultWAPMapSettings();
      
      
      /**
       * Makes a MapSettigns with the default settings for Tile maps.
       *
       * Only used for creating a GfxFeatureMap. The colors, line widths
       * etc. are not used in the final map drawing.
       *
       * @param highEndDevice See STMFDParams
       *
       * @return A new MapSettings with the apropriate settings.
       */
      static MapSettings* createDefaultTileMapSettings( bool highEndDevice );
      

      /**
       * Returns a string representing the defaultMapSetting input.
       * @param setting The defaultMapSetting.
       * @return A string representing the defaultMapSetting.
       */
      static const char* defaultMapSettingToString( 
         MapSettingsTypes::defaultMapSetting setting );

      
      /**
       * Returns the defaultMapSetting that the string represents.
       * @param str The string representing a defaultMapSetting.
       * @return The defaultMapSetting that the string represents.
       */
      static MapSettingsTypes::defaultMapSetting
         defaultMapSettingFromString( 
            const char* str, 
            MapSettingsTypes::defaultMapSetting defaultSetting = 
            MapSettingsTypes::MAP_SETTING_STD );


      /**
       * Prints the imageSetting into target.
       * @param target The string to print into, length must at least be
       *        3*( ceil( (#image_show-vars)/8 ) ).
       * @param imageSettings The ImageSettings to print into target.
       * @return The number of chars written into target.
       */
      static int imageSettingsToString( 
         char* target, 
         struct MapSettingsTypes::ImageSettings& imageSettings );

      
      /**
       * Inserts the ImageSettings from str into imageSettings.
       * @param str The string with the image settings.
       * @param imageSettings The ImageSettings to set.
       * @return True if imageSettings was set ok, false if not.
       */
      static bool imageSettingsFromString( 
         const MC2String& str, 
         struct MapSettingsTypes::ImageSettings& imageSettings );


      /**
       * Prints the content to out.
       */
      ostream& dump( ostream& out );

      /**
       *   Returns the redline settings.
       */
      const RedLineSettings& getRedLineSettings() const {
         return *m_redLineSettings;
      }

      /**
       *   Sets the redline settings.
       */
      void setRedLineSettings( const RedLineSettings& settings );

      /**
       *  Returns the drawing projection
       */
      const DrawingProjection* getDrawingProjection() const {
         return m_drawingProjection;
      }

      /**
       *   Sets the drawing projection.
       */
      void setDrawingProjection( DrawingProjection* projection );
      /// sets the tilemap param string
      void setTileMapParamStr( const MC2SimpleString& str ) {
         m_tileMapParamStr = str;
      }
      /// @return tile map param str
      const MC2SimpleString& getTileMapParamStr() const { 
         return m_tileMapParamStr;
      }

      /**
       * Set the user's map rights, used for poi selection.
       */
      void setMapRights( const MapRights& m );

      /**
       * Get the user's map rights, used for poi selection.
       */
      const MapRights& getMapRights() const;
      /**
       * Adds a mask to the current show mask.
       * @param mask the mask to be added to the current show mask
       */
      void addShowMask( uint32 mask );

   /**
    * Gets the image set to use when drawing maps.
    * @return the image set.
    */
   ImageTable::ImageSet getImageSet() const { 
      return m_imageSet; 
   }

   /**
    * Sets which image set to use when drawing maps.
    * @param imageSet The image set to use.
    */
   void setImageSet( ImageTable::ImageSet imageSet );
private:
      /**
       * The POI on map bools in a bitset.
       * NB! bitset is not implemented in current STL(egcs-2.91.66), using
       *     bit_vector.
       * NB! vector<bool> is a specialized variant of vector which will
       *     become bit_vector which has been removed.
       */
      //bitset<256> poiBits;
      //bit_vector m_poiBits;
      vector<bool> m_poiBits;

      /// Mask of what types to show, @see ShowTypes
      uint32 m_showTypeMask;

      /**
       * If a Drawing Projection is included
       */
      bool m_drawingProjIncluded;
      
      /**
       * Whether maximum one coordinate per pixel should be allowed
       * for the map. Only valid if map is present.
       */
      bool m_maxOneCoordPerPixelForMap;
       
      
      /**
       * Whether maximum one coordinate per pixel should be allowed
       * for the route. Only valid if route is present.
       */
      bool m_maxOneCoordPerPixelForRoute;
       
      
      /**
       * If this map will be a navigator crossing map.
       */
      bool m_navigatorCrossingMap;
      
      MC2SimpleString m_tileMapParamStr;

      typedef vector<MapSetting*> MapSettingVector;
      MapSettingVector m_settings;
      MapSetting m_searchSetting;
      uint32 m_size;
      /// RedLineSettings are used for java simple maps
      RedLineSettings* m_redLineSettings;

      /// Drawing projection are used for drawing GfxFeatureMaps.
      DrawingProjection* m_drawingProjection;

      /// The MapRights default constructor used, set in setMapRights.
      MapRights m_mapRights;

   /// The image set to use when drawing the maps.
   ImageTable::ImageSet m_imageSet;

   /**
    * Sets show type to be shown or not.
    * @param type The type to be assigned the "value".
    * @param value Whether to set the type to be shown or not.
    */
   inline void setShowType( ShowType type, bool value );

   /**
    * Whether or not a specific type should be shown.
    * @param type
    * @return true if the type should be shown else false.
    */
   inline bool shouldShow( ShowType type ) const;
};


// =======================================================================
//                                     Implementation of inlined methods =
inline void
MapSettings::setShowType( ShowType type, bool use ) {
   if ( use ) {
      m_showTypeMask |= type;
   } else if ( shouldShow( type ) ) {
      m_showTypeMask ^= type;
   }
}

inline bool
MapSettings::shouldShow( ShowType type ) const {
   return m_showTypeMask & type;
}

uint32 
MapSettings::getSize() const {
   return m_size;
}


inline void 
MapSettings::setMapContent( bool showMap, bool showTopographMap,
                            bool showPOI, bool showRoute )
{
   setShowType( MapSettings::MAP, showMap );
   setShowType( MapSettings::TOPOGRAPH_MAP, showTopographMap );
   setShowType( MapSettings::POI, showPOI );
   setShowType( MapSettings::ROUTE, showRoute );
}


inline bool
MapSettings::getShowEvents() const {
   return shouldShow( MapSettings::EVENT );
}
inline bool
MapSettings::getShowMap() const {
   return shouldShow( MapSettings::MAP );
}


inline bool 
MapSettings::getShowTopographMap() const {
   return shouldShow( MapSettings::TOPOGRAPH_MAP );
}


inline bool
MapSettings::getShowPOI() const {
   return shouldShow( MapSettings::POI );
}


inline bool 
MapSettings::getShowRoute() const {
   return shouldShow( MapSettings::ROUTE );
}


inline bool 
MapSettings::getShowTraffic() const {
   return shouldShow( MapSettings::TRAFFIC );
}


inline void
MapSettings::setShowTraffic( bool showTraffic ) {
   setShowType( MapSettings::TRAFFIC, showTraffic );
}


inline void 
MapSettings::setDrawScale( bool drawScale ) {
   setShowType( MapSettings::SCALE, drawScale );
}


inline bool 
MapSettings::getDrawScale() const {
   return shouldShow( MapSettings::SCALE );
}

inline void
MapSettings::setShowCityCentres( bool showCityCentres ) {
   setShowType( MapSettings::CITY_CENTRE, showCityCentres );
}

inline bool
MapSettings::getShowCityCentres() const {
   return shouldShow( MapSettings::CITY_CENTRE );
}

      /**
       * Set if maximum one coordinate per pixel should be allowed
       * for the map. Only valid if map is present.
       */
inline void 
MapSettings::setMaxOneCoordPerPixelForMap( bool value )
{
   m_maxOneCoordPerPixelForMap = value;
}
      
      
inline bool 
MapSettings::getMaxOneCoordPerPixelForMap() const
{
   return (m_maxOneCoordPerPixelForMap);
}
      
      
inline void 
MapSettings::setMaxOneCoordPerPixelForRoute( bool value )
{
   m_maxOneCoordPerPixelForRoute = value;
}
      
      
inline bool 
MapSettings::getMaxOneCoordPerPixelForRoute() const
{
   return (m_maxOneCoordPerPixelForRoute);
}

   
inline void 
MapSettings::setNavigatorCrossingMap( bool value )
{
   m_navigatorCrossingMap = value;
}
      
      
inline bool 
MapSettings::getNavigatorCrossingMap() const
{
   return (m_navigatorCrossingMap);
}


inline bool 
MapSettings::getPOI( ItemTypes::pointOfInterest_t poiType ) const
{
   return (m_poiBits[poiType]);
}

inline void
MapSettings::setMapRights( const MapRights& m ) {
   m_mapRights = m;
}

inline const MapRights&
MapSettings::getMapRights() const {
   return m_mapRights;
}

#endif // MAPSETTINGS_H

