/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXFEATURE_H
#define GFXFEATURE_H

#include "config.h"

#include <vector>

#include "ItemTypes.h"
#include "TrafficDataTypes.h"
#include "Vector.h"
#include "StringTable.h"
#include "MC2SimpleString.h"
#include "MC2Coordinate.h"
#include "LangTypes.h"

class DataBuffer;
class GfxPolygon;
class GfxRoadPolygon;
class MC2BoundingBox;

typedef vector<GfxPolygon*> gfxPolygonVector;


/**
 * Describes a graphical item.
 *
 */
class GfxFeature : public VectorElement {
public:
   typedef uint32 SizeType;
      /**
       * The types of features.
       */
      enum gfxFeatureType {
         /// Main class road.
         STREET_MAIN                     = 0,
         /// First class road.
         STREET_FIRST                    = 1,
         /// Second class road.
         STREET_SECOND                   = 2,
         /// Third class road.
         STREET_THIRD                    = 3,
         /// Fourth class road.
         STREET_FOURTH                   = 4,
         
         /// Builtup area
         BUILTUP_AREA                    = 5,
         /// Park (cityPark)
         PARK                            = 6,
         /// Forest
         FOREST                          = 7,
         /// General building
         BUILDING                        = 8,
         /// Water
         WATER                           = 9,
         /// Island
         ISLAND                          = 10,
         /// Pedestrian area
         PEDESTRIANAREA                  = 11,
         /// Aircraft road
         AIRCRAFTROAD                    = 12,
         /// Land
         LAND                            = 13,
         /// Builtup area, square-symbol
         BUILTUP_AREA_SQUARE             = 14,
         /// Builtup area, small symbol
         BUILTUP_AREA_SMALL              = 15,
         /// Water represented as a line
         WATER_LINE                      = 16,
         /// Ferry
         FERRY                           = 17,
         /// Railway
         RAILWAY                         = 18,
         /// Individual building items
         INDIVIDUALBUILDING              = 19,
         /// Park (nationalPark)
         NATIONALPARK                    = 20,
         /// Ocean (water below land)
         OCEAN                           = 21,
         /// Country border
         BORDER                          = 22,
         /// Airport item
         AIRPORTGROUND                   = 23,
         /**
          * Contains some cartographic types, that are considered to be
          * green areas, similar to park items, where you can sit down
          * and have "picnic"
          */
         CARTOGRAPHIC_GREEN_AREA         = 24,
         /**
          * Most cartographic types = "all other types", that are not 
          * included in any specific cartographic feature type 
          * (like the green area)
          */
         CARTOGRAPHIC_GROUND             = 25,
         
         /**
          * ROUTE.
          * This indicates that the start of a new routesegment.
          * Necessary to know if a certain routesegment should be 
          * highlighted.
          */
         ROUTE                           = 100,
         /** 
          * ROUTE_CONTINUATION.
          * Continuation of a route, ie. this feature does not indicate
          * the start of a new routesegment. It is simply a continuation of
          * a previous ROUTE feature. Necessary to know in order to 
          * higlight a specific route segment.
          */
         ROUTE_CONTINUATION              = 101,
         /// PARK CAR, and walk
         PARK_CAR                        = 102,
         /// EMPTY                           
         EMPTY                           = 150,

         /// SYMBOL
         SYMBOL                          = 151,
         
         /// Traffic Info                
         TRAFFIC_INFO                    = 152,
         
         /// ROUTE_ORIGIN
         ROUTE_ORIGIN                    = 237,
         /// ROUTE_DESTINATION
         ROUTE_DESTINATION               = 238,
         
         /// PointOfInterest
         POI                             = 254,
         /// Event
         EVENT                           = 256,
         /// Walkway
         WALKWAY                         = 257,
         /// Water in park, same as water but drawn after park.
         WATER_IN_PARK                   = 258,
         /// Island in bua, same as island but with the bua color.
         ISLAND_IN_BUA                   = 259,
         /// Island in water in park, same as island but with park color.
         ISLAND_IN_WATER_IN_PARK         = 260,
         /// Island in water in park in BUA, same as island but with BUA color.
         ISLAND_IN_WATER_IN_PARK_BUA     = 261,
         /// Island in water in park outside BUA, same as island.
         ISLAND_IN_WATER_IN_PARK_ISLAND  = 262,
         /// BUA on island
         BUA_ON_ISLAND                   = 263,
         /// The number of feature-types
         NBR_GFXFEATURES
      };


      /**
       * Constructor.
       * @param type The type of the feature.
       * @param name The name of the feature, is copied.
       */
      GfxFeature( gfxFeatureType type,
                  const char* name,
                  bool displayText = false, int fontSize = 10,
                  int32 textLat = 0, int32 textLon = 0, 
                  int drawTextStart = 90 );

      /**
       * Constructs a GfxFeature with a boundingbox as geometry.
       * @param type The type of the feature.
       * @param name The name of the feature, is copied.
       */
      GfxFeature( gfxFeatureType type,
                  const MC2BoundingBox& bbox,
                  const char* name = NULL );
      
      /**
       * Constructor.
       * @param type The type of the feature.
       */
      GfxFeature( gfxFeatureType type );

      /**
       *    Clone method. 
       */
      GfxFeature* clone() const;

      /**
       *    Clones the feature and only keeps the specified polygon.
       *    @param   poly  The polygon index to keep.
       *    @return  A new GfxFeature with only the specified polygon.
       *             NULL if poly is out of bounds.
       */
      GfxFeature* clonePolygon( uint16 poly ) const;

      /**
       * Destructor.
       */
      virtual ~GfxFeature();

      /**
       * Create a new feature with the correct dynamic type from a 
       * databuffer.
       *
       * @param   data  The buffer that contains data for the feature.
       * @return  The new feature.
       */
      static GfxFeature* createNewFeature( DataBuffer* data );
      

      /**
       * Create a new feature with the correct dynamic type given
       * the feature type.
       *
       * @param   type  The feature type.
       * @param   name  The name of the feature, is copied.
       * @return  The new feature.
       */
      static GfxFeature* createNewFeature( GfxFeature::gfxFeatureType,
                                           const char* name = NULL );
      
      
      /**
       * The type of feature.
       */
      inline gfxFeatureType getType() const;


      /**
       * The name of feature.
       */
      inline const char* getName() const;
      
      /**
       * Set the name of the feature.
       * @param   name  The name. The string is copied to the feature.
       */
      void setName(const char* name);

      /**
       * Put the feature into a DataBuffer. Should write getSize bytes.
       */
      virtual bool save( DataBuffer* data ) const;

      
      /**
       * The size of the feature when put into a DataBuffer, in bytes.
       */
      virtual SizeType getSize() const;

      
      /**
       * The number of polygons.
       */
      inline uint32 getNbrPolygons() const;


      /**
       * The i'th polygon or NULL if no such polygon.
       * @param i The index of the poltgon to get.
       * @return A polygon or NULL.
       */
      inline GfxPolygon* getPolygon( uint32 i ) const;

      
      /**
       * Add a new polygon.
       * @param coordinate16Bits Whether 16 bit relative coordinates or
       *                         32 bit absolute coordinates should be
       *                         used.
       * @param startSize The initial size of the polygon, default 16.
       */
      virtual void addNewPolygon( bool coordinate16Bits,
                                  uint32 startSize = 16 );


      /**
       * Add new coordinates without specifying the previous ones.
       * O(1) complexity if the coordinates internally are represented by
       * by absolute coordinates.
       * O(n) complexity if the coordinates internally are represented
       * by relative coordinates.
       * n is the number of coordinates added so far.
       * Use with care! Rather use the other version of addCoordinate().
       *
       * @param   lat      Latitude to add.
       * @param   lon      Longitude to add.
       */
      void addCoordinateToLast( int32 lat, int32 lon );

      /// @see above
      inline void addCoordinateToLast( const MC2Coordinate& coord ) {
         addCoordinateToLast( coord.lat, coord.lon );
      }

      /**
       * Add new coordinates to the last polygon. 
       * O(1) complexity regardless of the
       * internal representation of the coordinates.
       * Prefer this method to addCoordinate( int32 lat, int32 lon ).
       *
       * @param   lat      Latitude to add.
       * @param   lon      Longitude to add.
       * @param   prevLat  Previous latitude. Ignored if no previous
       *                   coordinates exist.
       * @param   prevLon  Previous longitude. Ignored if no previous
       *                   coordinates exist.
       */
      void addCoordinateToLast( int32 lat, int32 lon, 
                                int32 prevLat, int32 prevLon );


      /**
       * Dump information about the feature to stdout.
       */
      virtual void dump(int verboseLevel = 1) const;


      /**
       * The scalelevel of the feature.
       */
      inline uint32 getScaleLevel() const;


      /**
       * Set the scalelevel of the feature.
       */
      inline void setScaleLevel( uint32 scaleLevel );

      /**
       * Check if the supplied feature is of the same type as 
       * this one. Include typechecks for subclasses of Feature.
       * @param feature The feature to check.
       * @return True if the features are of the same type,
       *         false otherwise.
       */
      virtual bool equalsType( const GfxFeature* feature ) const;
      

      inline int32 getTextLat() const;
      inline int32 getTextLon() const;
      inline void setTextLat( int32 lat );
      inline void setTextLon( int32 lon );
      inline void setTextCoord( const MC2Coordinate& coord );

      inline bool getDisplayText() const;
      inline void setDisplayText( bool displayText );
      inline int getFontSize() const;
      inline void setFontSize( int fontSize );
      
      /**
       *  This information will overwrite LangType and NameIsAbbreviated,
       *  and vice verca.
       */
      inline int getDrawTextStart() const;
      inline void setDrawTextStart( int drawTextStart );

      
      /// @return If the name is already abbreviated.
      inline bool getNameIsAbbreviated() const;

      /// @return The language of the name.
      inline LangTypes::language_t getLangType() const;

      /**
       *  Set the language of the name, and if the name is
       *  already abbreviated.
       *
       *  This information will overwrite the DrawTextStart 
       *  info, and vice verca.
       */
      inline void setLangType( LangTypes::language_t langType,
                               bool nameIsAbbreviated );

      /// @return The country where the GfxFeature is located.
      inline StringTable::countryCode getCountryCode() const;

      /// Sets the country code, where the GfxFeature is located.
      inline void setCountryCode( StringTable::countryCode country );

      /**
       * Resize number of polygons to the new size.
       * Delete polygons if size is less than getNbrPolygons.
       */
      void resizeNbrPolygons( uint32 size, 
                              bool coordinate16Bits = false,
                              uint32 startSize = 1 );
   /// @return name of the feature type
   static const char* getFeatureTypeAsString( const GfxFeature& feature ) {
      return getFeatureTypeAsString( feature.getType() );
   }

   /// @return name of the feature type
   static const char* getFeatureTypeAsString( uint32 type );

   void setBasename( const char* name ) {
      m_basename = name;
   }

   const MC2String& getBasename() const {
      return m_basename;
   }

   void addNewPolygon( GfxPolygon* poly );

protected:
      /**
       * Load the feature from a DataBuffer.
       *
       * @param data DataBuffer with the GfxFeature data to read.
       */
      virtual bool createFromDataBuffer( DataBuffer* data );


      /**
       * Reads the header from Databuffer and returns the number of 
       * polygons.
       *
       * @param data DataBuffer with the GfxFeature data to read.
       * @return The number of polygons.
       */
      virtual uint32 readHeader( DataBuffer* data );

      
      /**
       * Reads the polygons from Databuffer.
       *
       * @param data DataBuffer with the GfxFeature data to read.
       * @param nbrPolys The number of polygons to read.
       */
      virtual void readPolygons( DataBuffer* data, uint32 nbrPolys );

      
      /**
       * Writes the header into Databuffer.
       */
      virtual void writeHeader( DataBuffer* data ) const;

      
      /**
       * Writes the polygons into Databuffer.
       */
      void writePolygons( DataBuffer* data ) const;

            
      /**
       * Reads a polygon from databuffer.
       */
      virtual GfxPolygon* readPolygon( DataBuffer* data ) const;

      
      /**
       * The type of GfxMapFeature.
       */
      gfxFeatureType m_type;


      /**
       * The scaleLevel of the feature.
       */
      uint32 m_scaleLevel;


      /**
       * The name of the feature.
       */
      char* m_name;

      
      /**
       * The polygons.
       */
      gfxPolygonVector m_polys;


      /**
       * If the text should be displayed. Not automatically initialized.
       */
      bool m_displayText;

      
      /**
       * The size of the text in points. Not automatically initialized.
       */
      int m_fontSize;

      
      /**
       * The latitude-value of the text. Not automatically initialized.
       */
      int32 m_textLat;

      
      /**
       * The longitude-value of the text. Not automatically initialized.
       */
      int32 m_textLon;

      
      /**
       * HACK:
       * 
       * When tilemaps are generated, this member will be used to store
       * the language type (16 lower bits), as well as if the 
       * name is already abbreviated (MSB).
       *
       * The angle of the text, in degrees,  or if negative it 
       * indicates the text to follow the polygon at 
       * index (-drawTextStart)+1, ei. -1 is index 0.
       */
      int m_drawTextStart;

      // The country where the GfxFeature is located
      StringTable::countryCode m_countryCode;

private:
   /**
    * Basename is used for T-tiles when filtering and merging
    * streets. This contains the same name as the G-tiles name.
    * This is because some streets might have mismatching names between
    * languages. For example in taiwan there were two street items that had
    * the same name in english but two different names in chinese which caused
    * CRC-mismatch between T- and G-tiles.
    */
   MC2String m_basename;
};


/**
 * A GfxRoadFeature.
 * Represensts a road.
 */
class GfxRoadFeature : public GfxFeature {
    public:
      /**
       * Constructor.
       * 
       * @param type The type of feature, any ROAD type.
       */
      GfxRoadFeature( gfxFeatureType type );


      /**
       * Constructor.
       * @param type The type of the feature.
       * @param name The name of the feature, is copied.
       */
      GfxRoadFeature( gfxFeatureType type,
                      const char* name );
            
      
      /**
       * Destructor.
       */
      virtual ~GfxRoadFeature();


      /**
       * The size of the GfxRoadFeature when put into a 
       * DataBuffer, in bytes.
       */
      virtual SizeType getSize() const;


      /**
       * Add a new polygon.
       * @param coordinate16Bits Whether 16 bit relative coordinates or
       *                         32 bit absolute coordinates should be
       *                         used.
       * @param startSize The initial size of the polygon, default 16.
       */
      virtual void addNewPolygon( bool coordinate16Bits,
                                  uint32 startSize = 16 );
      /**
       * Add new road polygon.
       * This is somewhat strange, the normal GfxFeature::addNewPolygon does
       * not seem work with gcc "3.4.6 20060404 (Red Hat 3.4.6-10)",
       * seems to be some strange virtual lookup failure. It was discovered in
       * GfxFeature::recalculateLevelAndSplitRoads .
       */
      void addNewPolygon( GfxRoadPolygon* poly );

      /**
       * Dump information about the feature to stdout.
       */
      virtual void dump(int verboseLevel = 1) const;

   protected:
      /**
       * Reads a polygon from databuffer.
       */
      virtual GfxPolygon* readPolygon( DataBuffer* data ) const;


   private:

};


/**
 * A GfxPOIFeature.
 * Represensts a point of interest.
 */
class GfxPOIFeature : public GfxFeature {
public:
   typedef uint16 CategoryID;
   typedef vector<CategoryID> Categories;
      /**
       * Constructor.
       * @param   name  The name of the feature, is copied.
       */
      GfxPOIFeature(const char* name = NULL);


      /**
       * Constructor.
       * 
       * @param poiType The poi type.
       */
      GfxPOIFeature( ItemTypes::pointOfInterest_t poiType );


      /**
       * Constructor.
       * @param poiType The poi type of the feature.
       * @param name    The name of the feature, is copied.
       */
      GfxPOIFeature( ItemTypes::pointOfInterest_t poiType,
                     const char* name );
      
      
      /**
       * Destructor.
       */
      virtual ~GfxPOIFeature();


      /**
       * Put the feature into a DataBuffer. Should write getSize bytes.
       */
      bool save( DataBuffer* data ) const;


       /**
       * The size of the GfxPOIFeature when put into a 
       * DataBuffer, in bytes.
       */
      SizeType getSize() const;


      /**
       * @return  The poi-type.
       */
      inline ItemTypes::pointOfInterest_t getPOIType() const;


      /**
       * @param   The poi-type.
       */
      inline void setPOIType(ItemTypes::pointOfInterest_t poiType);
         

      /**
       * Dump information about the feature to stdout.
       */
      virtual void dump(int verboseLevel = 1) const;

      /**
       * Check if the supplied feature is of the same type as 
       * this one. Include typechecks for subclasses of Feature.
       * @param feature The feature to check.
       * @return True if the features are of the same type,
       *         false otherwise.
       */
      virtual bool equalsType( const GfxFeature* feature ) const;

      /**
       * Set the extra info, that can mean different things depending
       * on the poi type.
       * For citycentre, extra info is display class.
       * @param   extraInfo   The extra info.
       */
      inline void setExtraInfo( byte extraInfo );

      /**
       * Get the extra info, that can mean different things depending
       * on the poi type. 
       * For citycentre, extra info is display class.
       * If m_customImageName is set to a value, then
       * the extra info is the tilemap scalelevel where the poi should
       * be shown.
       * @return The extra info.
       */
      inline byte getExtraInfo() const;

      /**
       * Set the custom image name of the poi.
       */
      void setCustomImageName( const MC2SimpleString& customName );
      
      /**
       * Get the custom image name of the poi.
       */
      const MC2SimpleString& getCustomImageName() const;

   const Categories& getCategories() const { return m_categories; }

   /// @param newCategories this will be swapped with the old ones
   void swapCategories( Categories& newCategories ) {
      m_categories.swap( newCategories );
   }

private:
      /**
       * Load the feature from a DataBuffer.
       *
       * @param data DataBuffer with the GfxFeature data to read.
       */
      bool createFromDataBuffer( DataBuffer* data );

      
      /**
       * The poi-type.
       */
      ItemTypes::pointOfInterest_t m_poiType;

      /**
       * The extra info, that can mean different things depending
       * on the poi type.
       * For citycentre, extra info is display class.
       * If m_customImageName is set to a value, then
       * the extra info is the tilemap scalelevel where the poi should
       * be shown.
       */
      byte m_extraInfo;

      /**
       * The custom image name of the POI. Set this if not the standard
       * symbol for the poi should be used.
       */
      MC2SimpleString m_customImageName;
   /// the category ids that this poi belongs to
   Categories m_categories;
};

class ExtendedTileString;

/**
 * Describes an Event gfx feature.
 */
class GfxEventFeature: public GfxFeature {
public:
   typedef uint32 Date;
   typedef uint32 IDType;
   static const IDType INVALID_ID = MAX_UINT32;
   /**
    * Light weight version of ExtendedTileString.
    * @see ExtendedTileString
    */
   class StringType {
   public:
      typedef uint16 Type;

      StringType( Type type,
                  const MC2String& string ):
         m_type( type ),
         m_string( string ) {
      }

      /// @return string data type, @see ExtendedTileString
      Type getType() const {
         return m_type;
      }

      /// @return string data
      const MC2String& getString() const {
         return m_string;
      }
      bool operator == ( const StringType& other ) const {
         return
            getType() == other.getType() &&
            getString() == other.getString();
      }
   private:
      Type m_type; ///< String type, @see ExtendedTileString
      MC2String m_string; ///< The string data.
   };

   typedef GfxPOIFeature::Categories Categories;
   typedef vector< StringType > Strings;

   explicit GfxEventFeature( const char* name = NULL );

   /// Adds a category this event
   void addCategory( Categories::value_type category ) {
      m_categories.push_back( category );
   }

   /// Adds a string.
   void addString( const StringType& string ) {
      m_strings.push_back( string );
   }

   /// @see GfxFeature
   bool save( DataBuffer* data ) const;

   /// @see GfxFeature
   virtual bool createFromDataBuffer( DataBuffer* data );

   /// @return extended strings for tile maps
   const Strings& getStrings() const {
      return m_strings;
   }

   /// @return the categories this feature belongs to.
   const Categories& getCategories() const {
      return m_categories;
   }

   /// @see GfxFeature
   SizeType getSize() const;

   /// @return start time in UTC for this event.
   Date getDate() const {
      return m_date;
   }

   /// Set start time in UTC for this event.
   void setDate( Date date ) {
      m_date = date;
   }

   void setDuration( Date duration ) {
      m_duration = duration;
   }

   Date getDuration() const {
      return m_duration;
   }

   /// Sets external ID for this event.
   void setID( uint32 id ) {
      m_id = id;
   }

   /// @return external ID for this event.
   IDType getID() const {
      return m_id;
   }

private:
   Strings m_strings; ///< Extended strings.
   Categories m_categories; ///< Category IDs.
   Date m_date; ///< time in UTC.
   Date m_duration; ///< Duration in 5 minut resolution.
   IDType m_id; ///< Unique external ID.

};


/**
 * A GfxSymbolFeature.
 * Represents a symbol.
 */
class GfxSymbolFeature : public GfxFeature {
   public:
      /**
       * The types of symbols.
       */
      enum symbols {
         /// PIN, ordinary map pin.
         PIN = 0,
         /// USER_DEFINED, uses symbol image name.
         USER_DEFINED = 1,
         
         /// The number of symbols, not a real symbol.
         NBR_SYMBOLS = 3
      };


      /**
       * Constructor.
       * 
       * @param type The type of feature, any symbol type.
       */
      GfxSymbolFeature( gfxFeatureType type );


      /**
       * Constructor.
       * @param type The type of the feature.
       * @param name The name of the feature, is copied.
       * @param symbol The type of symbol.
       * @param symbolImage The image of the feature used if USER_DEFINED
       *                    symbol, is copied. Default "".
       */
      GfxSymbolFeature( gfxFeatureType type,
                        const char* name,
                        symbols symbol,
                        const char* symbolImage = "" );
      
      
      /**
       * Destructor.
       */
      virtual ~GfxSymbolFeature();


      /**
       * The size of the GfxSymbolFeature when put into a 
       * DataBuffer, in bytes.
       */
      virtual SizeType getSize() const;


      /**
       * Dump information about the feature to stdout.
       */
      virtual void dump(int verboseLevel = 1) const;


      /**
       * The type of symbol.
       * @return The type of symbol.
       */
      symbols getSymbol() const;

      
      /**
       * The symbolImage.
       * @return The symbolImage.
       */
      const char* getSymbolImage() const;


      /**
       * Prints the essential data into a string that can be used in a 
       * URL.
       *
       * @param target The string to print into.
       * @return The number of chars written into target.
       */
      uint32 toStringBuf( char* target ) const;

      
      /**
       * Prints the essential GfxSymbolFeature data into a string that 
       * can be used in an URL.
       *
       * @param target The string to print into.
       * @param symbol The type of symbol.
       * @param name The name of the feature.
       * @param symbolImage The image of the feature used if USER_DEFINED
       *                    symbol.
       * @return The number of chars written into target.
       */
      static uint32 writeSymbolString( char* target,
                                       symbols symbol,
                                       const char* name,
                                       int32 lat,
                                       int32 lon,
                                       const char* symbolImage );

      
      /**
       * Reads the essential GfxSymbolFeature data from a string that 
       * can be part of an URL.
       *
       * @param data The string to read data from.
       * @param symbol Set to she type of symbol.
       * @param name Set to new string with the name of the feature.
       * @param symbolImage Set to new string with the image of the 
       *                    feature used if USER_DEFIND symbol.
       * @return If data contains valid GfxSymbolFeature data.
       */
      static bool readSymbolString( const char* data,
                                    symbols& symbol,
                                    char*& name,
                                    int32& lat,
                                    int32& lon,
                                    char*& symbolImage );

   void setSymbol( symbols sym ) {
      m_symbol = sym;
   }

   void setSymbolImage( const char* image );

   protected:
      /**
       * Reads the header from Databuffer and returns the number of 
       * polygons.
       *
       * @param data DataBuffer with the GfxFeature data to read.
       * @return The number of polygons.
       */
      virtual uint32 readHeader( DataBuffer* data );


      /**
       * Writes the header into Databuffer.
       */
      virtual void writeHeader( DataBuffer* data ) const;


   private:
      /**
       * The type of symbol.
       */
      symbols m_symbol;


      /**
       * The symbolImage.
       */
      char* m_symbolImage;
};

/**
 *    A GfxTrafficInfoFeature.
 *    Represents traffic information (such as an accident, road work etc.)
 */
class GfxTrafficInfoFeature : public GfxFeature {
   public:

      /**
       * Simple constructor.
       * @param   name  Free text describing the traffic info.
       */
      GfxTrafficInfoFeature(const char* name = NULL);

      /**
       * Constructor with parameters.
       * 
       * @param   name              Free text describing the traffic info.
       * @param   trafficInfoType   The traffic info type.
       * @param   angle             The angle in degrees of the direction
       *                            of the traffic info.
       * @param   bothDirections    True if the traffic info is valid in
       *                            both directions, false if it's only
       *                            valid in the angles direction.
       * @param   startTime         Start time for the traffic info, 
       *                            the time since the 
       *                            Epoch (00:00:00 UTC, January 1, 1970), 
       *                            measured in seconds.
       * @param   endTime           Start time for the traffic info, 
       *                            the time since the 
       *                            Epoch (00:00:00 UTC, January 1, 1970), 
       *                            measured in seconds.
       */
      GfxTrafficInfoFeature(
            const char* name, 
            TrafficDataTypes::disturbanceType trafficInfoType,
            uint16 angle,
            bool bothDirections,
            uint32 startTime,
            uint32 endTime );

      
      /**
       * Destructor.
       */
      virtual ~GfxTrafficInfoFeature();


      /**
       * The size of the GfxTrafficFeature when put into a 
       * DataBuffer, in bytes.
       */
      virtual SizeType getSize() const;


      /**
       * Dump information about the feature to stdout.
       */
      virtual void dump(int verboseLevel = 1) const;

      
      /**
       * @name Get and set methods for the members.
       */
      //@{
         /**
          * @return  The traffic info type.
          */
         inline TrafficDataTypes::disturbanceType getTrafficInfoType() const;

         /**
          * @param   type  The traffic info type.
          */
         inline void setTrafficInfoType(
                            TrafficDataTypes::disturbanceType type);
         
         /**
          * @return  Whether the traffic info is valid in both directions
          *          or only the direction specified by the angle.
          */
         inline bool isValidInBothDirections() const;

         /**
          * @param   bothDirections True if the traffic info is valid in
          *                         both directions, false if only the
          *                         direction specified by the angle.
          */
         inline void setValidInBothDirections(bool bothDirections);
         
         /**
          * @return  The angle in degrees counted clockwise
          *          from north of the traffic info.
          */
         inline uint16 getAngle() const;

         /**
          * @param   angle  The angle in degrees counted clockwise
          *                 from north of the traffic info.
          */
         inline void setAngle(uint16 angle);
         
         /**
          * @return  Start time for the traffic info, the time since the 
          *          Epoch (00:00:00 UTC, January 1, 1970), 
          *          measured in seconds.
          */
         inline uint32 getStartTime() const;

         /**
          * @param   startTime   Start time for the traffic info, 
          *                      the time since the 
          *                      Epoch (00:00:00 UTC, January 1, 1970), 
          *                      measured in seconds.
          */
         inline void setStartTime(uint32 startTime);
         
         /**
          * @return  End time for the traffic info, the time since the 
          *          Epoch (00:00:00 UTC, January 1, 1970), 
          *          measured in seconds.
          */
         inline uint32 getEndTime() const;

         /**
          * @param   endTime     End time for the traffic info, 
          *                      the time since the 
          *                      Epoch (00:00:00 UTC, January 1, 1970), 
          *                      measured in seconds.
          */
         inline void setEndTime(uint32 endTime);

      //@}
      
   protected:
      /**
       * Reads the header from Databuffer and returns the number of 
       * polygons.
       *
       * @param data DataBuffer with the GfxFeature data to read.
       * @return The number of polygons.
       */
      virtual uint32 readHeader( DataBuffer* data );


      /**
       * Writes the header into Databuffer.
       */
      virtual void writeHeader( DataBuffer* data ) const;


   private:
      /**
       * The type of traffic info.
       */
      TrafficDataTypes::disturbanceType m_trafficInfoType;

      /**
       * Whether the disturbance is valid in both directions or only
       * in the direction specified by the angle.
       */
      bool m_validInBothDirections;

      /**
       * Start time for the traffic info, the time since the 
       * Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.
       */
      uint32 m_startTime;

      /**
       * End time for the traffic info, the time since the 
       * Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.
       */
      uint32 m_endTime;

      /**
       * The angle of the traffic info. Represented as parts of 255 in
       * degrees, ie. angle_degree = m_angle / 255. Counted clockwise
       * from north.
       */
      byte m_angle;

};


// =====================================================================
//                                    Implementation of inlined methds =


inline GfxFeature::gfxFeatureType 
GfxFeature::getType() const
{
   return m_type;
}


inline const char* 
GfxFeature::getName() const
{
   return m_name;
}


inline uint32 
GfxFeature::getNbrPolygons() const
{
   return m_polys.size();
}


inline GfxPolygon* 
GfxFeature::getPolygon( uint32 i ) const
{
   return m_polys[i];
}


inline uint32 
GfxFeature::getScaleLevel() const {
   return m_scaleLevel;
}


inline void 
GfxFeature::setScaleLevel( uint32 scaleLevel ) {
   m_scaleLevel = scaleLevel;
}


inline int32 
GfxFeature::getTextLat() const
{
   return m_textLat;
}


inline int32 
GfxFeature::getTextLon() const
{
   return m_textLon;
}

inline void 
GfxFeature::setTextLat( int32 lat ) {
   m_textLat = lat;
}


inline void 
GfxFeature::setTextLon( int32 lon ) {
   m_textLon = lon;  
}

inline void 
GfxFeature::setTextCoord( const MC2Coordinate& coord ) {
   setTextLat( coord.lat );
   setTextLon( coord.lon );
}

inline bool 
GfxFeature::getDisplayText() const
{
   return m_displayText;
}


inline void 
GfxFeature::setDisplayText( bool displayText ) {
   m_displayText = displayText; 
}


inline int 
GfxFeature::getFontSize() const
{
   return m_fontSize;
}


inline void 
GfxFeature::setFontSize( int fontSize ) {
   m_fontSize = fontSize;
}


inline int32 
GfxFeature::getDrawTextStart() const
{
   return m_drawTextStart;
}


inline void 
GfxFeature::setDrawTextStart( int drawTextStart ) {
   m_drawTextStart = drawTextStart;
}

inline bool 
GfxFeature::getNameIsAbbreviated() const
{
   return (m_drawTextStart & 0x8000000) == 0;
}

inline LangTypes::language_t 
GfxFeature::getLangType() const
{
   return LangTypes::language_t( m_drawTextStart & 0xffff );
}

inline void 
GfxFeature::setLangType( LangTypes::language_t langType,
                         bool nameIsAbbreviated )
{
   if ( ! nameIsAbbreviated ) {
      // Set the high bit.
      m_drawTextStart = 0x8000000;
   }
   m_drawTextStart |= (int) langType;
}

inline StringTable::countryCode
GfxFeature::getCountryCode() const
{
   return m_countryCode;
}

inline void
GfxFeature::setCountryCode( StringTable::countryCode country )
{
   m_countryCode = country;
}

inline ItemTypes::pointOfInterest_t 
GfxPOIFeature::getPOIType() const
{
   return (m_poiType);
}


inline void
GfxPOIFeature::setPOIType( ItemTypes::pointOfInterest_t poiType )
{
   m_poiType = poiType;
}

inline void 
GfxPOIFeature::setExtraInfo( byte extraInfo )
{
   m_extraInfo = extraInfo;
}
   
inline byte 
GfxPOIFeature::getExtraInfo() const
{
   return m_extraInfo;
}

inline TrafficDataTypes::disturbanceType 
GfxTrafficInfoFeature::getTrafficInfoType() const
{
   return (m_trafficInfoType);
}


inline void
GfxTrafficInfoFeature::setTrafficInfoType(TrafficDataTypes::disturbanceType type) 
{
   m_trafficInfoType = type;
}


inline bool 
GfxTrafficInfoFeature::isValidInBothDirections() const
{
   return (m_validInBothDirections);
}


inline void 
GfxTrafficInfoFeature::setValidInBothDirections(bool bothDirections)
{
   m_validInBothDirections = bothDirections;
}


inline uint16 
GfxTrafficInfoFeature::getAngle() const
{
   // Convert from 0 - 255 to 0 - 359 degrees.
   return (uint16( m_angle/float64(255)*360 ));
}


inline void 
GfxTrafficInfoFeature::setAngle(uint16 angle)
{
   // Convert from 0 - 359 degrees to 0 - 255.
   m_angle = byte( (angle%360)/float64(360)*255 );
}


inline uint32 
GfxTrafficInfoFeature::getStartTime() const
{
   return (m_startTime);
}


inline void 
GfxTrafficInfoFeature::setStartTime(uint32 startTime)
{
   m_startTime = startTime;
}


inline uint32 
GfxTrafficInfoFeature::getEndTime() const
{
   return (m_endTime);
}


inline void 
GfxTrafficInfoFeature::setEndTime(uint32 endTime)
{
   m_endTime = endTime;
}

#endif // GFXFEATURE_H
