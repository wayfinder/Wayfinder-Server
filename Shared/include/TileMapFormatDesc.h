/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_MAP_FORMAT_DESC_H
#define TILE_MAP_FORMAT_DESC_H

#include "TileMapConfig.h"
#include "TileFeatureArg.h"
#include "TileFeature.h"
#include "MC2SimpleString.h"
#include "TileImportanceTable.h"
#include "LangTypes.h"
#include "TileCategory.h"
#include "TileMapLayerInfo.h"
#include "CopyrightHolder.h"
#include "NotCopyable.h"

#include <vector>
#include <utility>
#include <set>

class MapProjection;
class MC2Coordinate;
class MC2BoundingBox;
class TileMapParams;
class RouteID;
struct ParamsNotice;

class TileMapCategoryNotice : public TileCategory {
public:   
   /**
    *   Creates a new TileMapPOICategoryNotice.
    *   @param catName  Name of category in utf-8. Will be copied.
    *   @param id       The id of the category.
    *   @param nbrTypes The number of types in the category.
    *   @param poiTypes poiTypes of the category. Will not be copied.
    */
   TileMapCategoryNotice(const char* catName,
                         int id,
                         int enabled,
                         int nbrTypes,
                         const TileFeature::tileFeature_t* poiTypes) :
      TileCategory( catName, id, enabled ),
      m_nbrTypes(nbrTypes),
      m_poiTypes(poiTypes) {
      // All is well that ends well.
   }


   /// Sets if the category should be enabled or not.
   void setEnabled( int enabled ) { m_enabled = enabled; }
   /// Returns the number of types in this category.   
   int getNbrTypes() const { return m_nbrTypes; }   
   /// Returns a type in this category.
   TileFeature::tileFeature_t getType(int idx) const {
      return m_poiTypes[idx];
   }
   void putInSet( set<int32>& theSet ) const {
      theSet.insert( m_poiTypes, m_poiTypes + getNbrTypes() );
   }
   
private:
   /// Name of the category in utf-8   
   MC2SimpleString m_catName;
   /// Number of poiTypes in the category
   int m_nbrTypes;
   /// The poi-types of the category.
   const TileFeature::tileFeature_t* m_poiTypes;   
};

/// Holds lat and lon index.
class TileIndex {
public:
   typedef int ValueType;

   TileIndex( ValueType latIdx, ValueType lonIdx ):
      m_latIdx( latIdx ), m_lonIdx( lonIdx ) {
   }

   /// @return latitiude index.
   ValueType getLatIdx() const {
      return m_latIdx;
   }

   /// @return longitude index.
   ValueType getLonIdx() const {
      return m_lonIdx;
   }

   /// Less than compare another TileIndex
   bool operator < ( const TileIndex& other ) const {
      if ( getLatIdx() != other.getLatIdx() ) {
         return getLatIdx() < other.getLatIdx();
      }
      return getLonIdx() < other.getLonIdx();
   }

   /// Equal compare another TileIndex
   bool operator == ( const TileIndex& other ) const {
      return
         getLatIdx() == other.getLatIdx() &&
         getLonIdx() == other.getLonIdx();
   }

   bool operator != ( const TileIndex& other ) const {
      return ! ( *this == other );
   }

private:
   ValueType m_latIdx; ///< longitude index
   ValueType m_lonIdx; ///< latitude index
};

inline ostream& operator << ( ostream& ostr, const TileIndex& index ) {
   ostr << index.getLatIdx() << ", " << index.getLonIdx();
   return ostr;
}

class TileMapFormatDesc: private NotCopyable {
public:

   typedef int16 featureInt;

   /// Gets the language from the paramString.
   static LangTypes::language_t getLanguageFromParamString(
      const MC2SimpleString& paramString );
   /// @return true if paramString is a night mode param
   static bool isNightMode( const MC2SimpleString& paramString );
   /**
    *   Returns a paramstring.
    *   @param lang              The requested language
    *   @param clientTypeString  A string identifying the client type.
    *   @param randChars         Optional random characters.
    */
   static MC2SimpleString createParamString( LangTypes::language_t lang,
                                             const char* clientTypeString,
                                             const char* randChars = NULL );
                                      

   /**
    *   Returns the language used .
    */
   LangTypes::language_t getLanguage() const {
      return m_lang;
   }

   /**
    *   Returns true if the language of this TileMapFormatDesc
    *   is ok for the TileMapHandler.
    */
   bool okLanguage( LangTypes::language_t tmhLang ) const {
      // For old format descs.
      return ( getLanguage() == LangTypes::invalidLanguage) ||
         // And for new format descs.
         ( getLanguage() == tmhLang );
   }
   
   /**
    *   Returns the minimum existing level. (Height)
    */
   int getMinLevel() const {
      return 0;
   }

   /**
    *   Returns the maximum existing level. (Height)
    */
   int getMaxLevel() const {
      return 15;
   }

   /**
    *   Returns the vector of categories.
    *   To be used via TileMapHandler for the clients.
    */
   const vector<const TileCategory*>& getCategories() const;

   /**
    *   Enable or disable a category with the specified id.
    *   To be used via the TileMapHandler for the clients.
    *   @return True if the category id was found.
    */
   bool setCategoryEnabled( int id, bool enabled );

   /**
    *   Put all the disabled feature types in the set.
    *   To be used by TileMapHandler internally.
    */
   void getFeaturesDisabledByCategories(
      set<int32>& features) const;
   
   /**
    *   Contstructor.
    */
   TileMapFormatDesc();

   /**
    *   Destructor.
    */
   virtual ~TileMapFormatDesc();

   /**
    *  Loads the data from file.
    */
   virtual void load( const char* fileName = "" );
   
   /**
    *  Loads the data from disk.
    */
   virtual void load( BitBuffer& buf,
                      TileMapFormatDesc* previouslyUsed = NULL );

   /**
    *   Puts parameters in a vector based on the view projection.
    *   @param paramsByLayerNbr  [IN/OUT] ParamsNotices for each layer.
    *                            Will be updated.
    *   @param paramVectorOut The vector to be filled with parameters.
    *   @param reserveParams  The vector of reserve params.
    *   @param projection     Current projection.
    *   @param useGzip        True if gzip compression can be requested.
    *   @param layers         The layers that the client wants to display.
    *   @param routeID        [Optional] If route should be displayed
    *                         a route id is necessary.
    *   @param detailLevel    [Optional] If not specified, the detaillevel
    *                         is calculated and used. Otherwise the
    *                         specified detaillevel is used.
    *   @return Number of parameters.
    */
   bool createParams(map<uint32, ParamsNotice>& paramsByLayerNbr,
                                vector<TileMapParams>& paramVectorOut,
                                vector<TileMapParams>& reserveParams,
                                const MapProjection& projection,
                                int useGzip,
                                LangTypes::language_t language,
                                const set<int>& layers,
                                const RouteID* routeID = NULL,
                                int detailLevel = MAX_INT32 );

   /**
    *   Returns all parameters needed for the supplied bbox
    *   on all levels. It is a dangerous function to use, since
    *   there can be very many parameters when much zoomed out.
    *   @param allParams The parameters will be put here.
    *   @param bbox      The bounding box to get the params for.
    *   @param layers    The layers to get the params for.
    *   @param useGzip   True if gzip should be used.
    *   @param language  The language to use ( for the string maps ).
    *   @param minScale  The minimum scale allowed.
    */
   void getAllParams( set<TileMapParams>& allParams,
                      const MC2BoundingBox& bbox,
                      const set<int>& layers,
                      int useGzip,
                      LangTypes::language_t language,
                      uint32 minScale ) const;

   /**
    *   Returns all parameters needed for the supplied bbox
    *   on all levels. It is a dangerous function to use, since
    *   there can be very many parameters when much zoomed out.
    *   @param outBuf      Parameters will be written to this buffer.
    *   @param bbox        The bounding box to get the params for.
    *   @param layers      The layers to get the params for.
    *   @param extraParams Extra parameters to add to the buffer.
    *   @param useGzip     True if gzip should be used.
    *   @param language    The language to use ( for the string maps ).
    *   @param minScale    The minimum scale allowed.
    *   @return outBuf
    */   
   SharedBuffer* getAllParams( const MC2BoundingBox& bbox,
                               const set<int>& layers,
                               const set<MC2SimpleString>& extraParams,
                               int useGzip,
                               LangTypes::language_t language,
                               uint32 minScale ) const;

  /**
   *   Returns all parameters needed for the supplied coordinates
   *   on all levels. It is a dangerous function to use, since
   *   there can be a lot of parameters.
   *
   *
   *   @param allParams    The parameters will be put here.
   *   @param coordsBegin  The begin iterator for the coordinates.
   *   @param coordsEnd    The end iterator for the coordinates.
   *   @param extraPixels  How many pixels extra that should be added 
   *                       around each coordinate for each detaillevel.
   *                       This should correspond to the screen size 
   *                       of the device that is used to show the
   *                       vector map.
   *   @param layers       The layers to get the params for.
   *   @param useGzip      True if gzip should be used.
   *   @param language     The language to use ( for the string maps ).
   *   @param minScale     The minimum scale allowed.
   *   @param maxScale     The maximum scale allowed.
   *   @param routeID      The route id, in case a route should be present.
   */
  void getAllParams( set<MC2SimpleString>& allParams,
                     vector<MC2Coordinate>::const_iterator coordsBegin,
                     vector<MC2Coordinate>::const_iterator coordsEnd,
                     uint32 extraPixels,
                     const set<int>& layers,
                     int useGzip,
                     LangTypes::language_t language,
                     uint32 minScale,
                     uint32 maxScale,
                     const RouteID* routeID ) const;
 
   /**
    *    Get the scale range for the param.
    *    Results is a pair where first is minScale and second is maxScale.
    *    @param   param The param.
    *    @return  The scalerange. first is minScale and second is maxScale.
    */
   pair<uint16, uint16> getScaleRange( const TileMapParams& param ) const;
   
   /**
    *   Returns a new list of default arguments for a certain type of
    *   feature, i.e. the values are filled in according to the 
    *   description format. To be called after the 
    *   getArgsForFeatureType are filled in.
    */
   void addDefaultArgsForFeatureType(featureInt type,
                                     vector<TileFeatureArg*>& args) const;
   
   /**
    *   Returns the number of primitive features that can be created
    *   for the complexFeature complexFeature.
    */
   int countFeaturesForFeature( TileFeature* complexFeature );
   
   /**
    *   Adds the primitives for the feature to the vector.
    *   The primitives do not own any of the arguments, they are
    *   owned by the parent feature.
    */
   void getFeaturesForFeature(vector<TilePrimitiveFeature*>& primitives,
                              TileFeature* complexFeature) const;

   /**
    *   Calculates the boundingbox based on the detail level and
    *   tile index.
    *   @param layerID     The layer ID.
    *   @param bbox        [OUT] The resulting boundingbox.
    *   @param detailLevel The detail level.
    *   @param tileLatIdx  Tile latitude index.
    *   @param tileLonIdx  Tile longitude index.
    */
   void getBBoxFromTileIndex( uint32 layerID,
                              MC2BoundingBox& bbox,
                              int detailLevel,
                              int tileLatIdx,
                              int tileLonIdx ) const;
   /// See above.
   void getBBoxFromTileIndex( uint32 layerID,
                              MC2BoundingBox& bbox,
                              int detailLevel,
                              const TileIndex& index ) const {
      getBBoxFromTileIndex( layerID, bbox,
                            detailLevel,
                            index.getLatIdx(), index.getLonIdx() );
   }
      
   /**
    *   @param layerID     The layer ID.
    *   @param detailLevel The detail level.
    *   @param tileLatIdx  Tile latitude index.
    *   @param tileLonIdx  Tile longitude index.
    *   @param coord       [OUT] A coordinate for the tile.
    *   @return   The scale (meters per pixel).
    *
    */
   uint16 getCoordAndScaleForTile( uint32 layerID,
                                   int detailLevel,
                                   int tileLatIdx,
                                   int tileLonIdx,
                                   TileMapCoord& coord ) const;

 
   /**
    *   Returns the default background color in
    *   24-bits RGB R is in bits 23-16.
    */
   uint32 getDefaultBackgroundColor() const;

   /**
    *    Get the importance notice as specified by the parameter.
    *    @param   param The param.
    *    @return  The importance notice.
    */
   const TileImportanceNotice* getImportanceNbr(
                                    const TileMapParams& param ) const;

   /**
    *   Returns a reference to the vector of layer id:s and descriptions.
    */
   const vector<pair<uint32, MC2SimpleString> >&
      getLayerIDsAndDescriptions() const;

   /**
    *   Returns the number of detail levels for a layerID.
    */
   int getNbrDetailLevelsForLayerID( uint32 layerID ) const;
   
   /**
    *   Get the detail level for the specified scale.
    *   @param layerID The layer ID.
    *   @param scale The scale.
    *   @return The detail level for the scale.
    */
   int getDetailLevelFromLayerID( uint32 layerID, uint16 scale ) const;

   /**
    *    Get the number of scale indexes.
    *    @return  The number of scale indexes.
    */   
   uint32 getNbrScaleIndexes() const {
      return m_scaleIndexByLevel.size();
   }
   
   /**
    *   Returns a new list of arguments for a certain type of feature.
    *   The arguments must be filled from a stream or bytes before
    *   using. These are the arguments that are transferred, i.e. does
    *   not contain the default arguments.
    */
   void getArgsForFeatureType(featureInt type,
                              vector<TileFeatureArg*>& args) const {
      if ( type < m_argsByTileFeatureTypeArraySize ) {
         
         const vector<TileFeatureArg*>& curVect =
            m_argsByTileFeatureTypeArray [type];
         
         args.reserve( curVect.size() );
         for ( vector<TileFeatureArg*>::const_iterator jt = 
                  curVect.begin(); jt != curVect.end(); ++jt ) {
            args.push_back( (*jt)->clone() );
         }
      }
   }
   

   /**
    *    Get the scale index from the specified scale.
    *    @param   scale The scale.
    *    @return  The scale index.
    */
   uint32 getScaleIndexFromScale( uint32 scale ) const {
      scaleIndexByLevel_t::const_iterator findIt = 
         m_scaleIndexByLevel.upper_bound( scale );
      // MAX_UINT32 should always be present in the table.
      MC2_ASSERT( findIt != m_scaleIndexByLevel.end() );
      return findIt->second;
   }

   /**
    *   Puts the number of features for each level in the
    *   vector levelCounts.
    */
   void countNbrFeaturesPerLevel( vector<int>& levelCounts,
                                  const vector<TileFeature>& features ) const;
   /**
    *   Calculates the scale.
    *   @param meters   Map meters.
    *   @param pixels   Screen pixels.
    *   @return The scale.
    */
   static float64 getScale( float64 meters, int pixels ); 
   /**
    * Cap the latitude index to a valid range.
    * @param layerNbr the layer of the tile
    * @param detailLevel the detail of the tile
    * @param minLat minimum latitude, i.e start lat
    * @param maxLat maximum latitude, i.e end lat
    */
   void capLatIdx( int layerNbr, int detailLevel,
                   int& minLat, int& maxLat ) const;

   /**
    *   Gets the tile index for the specified detail level and 
    *   coordinate.
    *   @param layerID        The layer number.
    *   @param detailLevel    The detail level.
    *   @param lat            Latitude.
    *   @param lon            Longitude.
    *   @param tileLatIdx     [OUT] Tile latitude index.
    *   @param tileLonIdx     [OUT] Tile longitude index.
    */
   void getTileIndex( uint32 layerNbr,
                      int detailLevel, 
                      int32 lat, int32 lon, 
                      int& tileLatIdx, 
                      int& tileLonIdx ) const;

   /// See above.
   void getTileIndex( uint32 layerNbr,
                      int detailLevel,
                      const MC2Coordinate& coord,
                      TileIndex& tileIndex ) const {
      int latIdx, lonIdx;
      getTileIndex( layerNbr, detailLevel,
                    coord.lat, coord.lon,
                    latIdx, lonIdx );
      tileIndex = TileIndex( latIdx, lonIdx );
   }

   /**
    *   Get the layer number for the specified layer ID.
    *   @param layerID  The layer ID.
    *   @return   The layer (ordinal) number for the layer ID.
    */
   inline uint32 getLayerNbrFromID( uint32 layerID ) const;

   /**
    *   Returns the CRC computed by the server.
    */
   uint32 getCRC() const;

   /**
    *   Returns the timestamp for the creation of the desc in the
    *   server.
    */
   uint32 getTimeStamp() const;

   /**
    *   Returns true if the params are allowed to be stored on disk.
    *   @param params TileMapParams for a map.
    */
   int allowedOnDisk( const TileMapParams& params ) const;
  
   /**
    *   Get if this is a valid param.
    */
   bool valid( const TileMapParams& params ) const;
   /**
    * Validates the layer index for a params.
    * @return true if the layer ID is valid.
    */
   inline bool isValidLayerID( uint32 layerID ) const;

   /**
    * Validates the importance number. This function assumes that the 
    * layer number is valid in the params.
    * @return true if the importance nbr is valid for the params.
    */
   bool isValidImportance( const TileMapParams& params ) const;

   /**
    * Returns mc2UnitsPerTile for a specific layers detail level.
    * @param layerNbr LayerNbr for wich you want the mc2UnitsPerTile
    * @param detailLevel Detail level for wich you want the mc2UnitsPerTile
    * @return Mc2UnitsPerTile for the specified layer and the specified detail level.
    */
   int getMc2UnitsPerTile( uint32 layerNbr, int detailLevel );

   inline const CopyrightHolder& getCopyrightHolder() const;

protected:

   /**
    *   Get the scale for the specified detail level.
    *   @param layerNbr The layer number.
    *   @param scale The detail level.
    *   @return The scale for the detail level.
    */
   uint16 getScaleForDetailLevel( uint32 layerNbr, int detailLevel ) const;

   /**
    *   Reads the categories from the supplied buffer.
    *   @param buf The buffer to read from.
    */
   void readCategories( SharedBuffer& buf,
                        const TileMapFormatDesc* previouslyUsed );
   
   /**
    *   Dumps the categories to mc2dbg
    */
#ifdef __unix__
   void dumpCategories() const;
#else
   inline void dumpCategories() const {}
#endif
   
   /**
    *   Returns the number of layers available.
    */
   inline uint32 getNbrLayers() const;
   
   /**
    *   Get the detail level for the specified scale.
    *   @param layerNbr The layer number.
    *   @param scale The scale.
    *   @return The detail level for the scale.
    */
   int getDetailLevel( uint32 layerNbr, uint16 scale ) const;
 
   /**
    *   Initializes the tile sizes for the specified layer number.
    *   NB! Assumes that m_tileScaleByLayer is already large enough.
    *   
    *   @param layerNbr    The layer number that the tile sizes
    *                      apply to.
    *   @param meters      Length in meters for the smallest tile.
    *   @param pixels      Number pixels that looks good for the 
    *                      smallest tile.
    *   @param dpi         DPI for the screen.
    *   @param zoomFactor  How much larger each tile should be on
    *                      the less detailed level.
    *   @param exchangeTileFactor   When to exchange from a less detailed
    *                               tile to the next tile: 
    *                               scalelevel( i ) + exchangeFactor*
    *                               ( scalelevel( i + 1) - scalelevel( i ) )
    *                               I.e:
    *                               1.0 means change
    *                               to the new tile when the scalelevel is
    *                               the same as the scalelevel for the 
    *                               next tile. 0.5 means change halways
    *                               between the less detailed tile and
    *                               the next tile.
    *   @param detailLevels   Number of detail levels.
    */
   void initTileSizesForLayer( uint32 layerNbr,
                               int meters,
                               int pixels,
                               int dpi,
                               float64 zoomFactor,
                               float64 exchangeTileFactor,
                               int detailLevels );

   /**
    *   Creates parameters.
    *
    *   @param reserveParams     If set to NULL, then no reserve maps will
    *                            be added.
    */
   bool innerCreateParams(map<uint32, ParamsNotice>& paramsByLayerNbr,
                          vector<TileMapParams>& paramVectorOut,
                          vector<TileMapParams>* reserveParams,
                          const MC2BoundingBox& bbox,
                          const MC2Coordinate& center,
                          uint32 inScale,
                          int useGzip,
                          LangTypes::language_t language,
                          const set<int>& layers,
                          const RouteID* routeID,
                          int inDetailLevel ) const;   

   /**
    *    Type of map to use when transferring arguments from
    *    the complex feature to the primitives.
    *    First is the name of the argument in the complex feature.
    *    Second.first is the index of the primitive in the vector of
    *    primitives for that kind of complex feature and second.second
    *    is the name of the argument in the primitive.
    */
   typedef multimap<TileArgNames::tileArgName_t,
                    pair<int, TileArgNames::tileArgName_t> > argTransferMap_t;
   
   /**
    *    Vector of primitives to be put in the map below.
    *    Will probably contain non-primitives due to the
    *    destructor.
    */
   typedef vector<TilePrimitiveFeature*> primitiveVector_t;
//   typedef vector<TileFeature*> primitiveVector_t;
   
   /**
    *    Map for mapping the complex features into primitive  
    */
   typedef primitiveVector_t primitiveDefaultMap_t;
   
   /**
    *    Map containing the primitive prototypes for each
    *    type of complex feature.
    */
   primitiveDefaultMap_t* m_primitiveDefaultMap;

   /// Size of the m_primitiveDefaultMap
   int m_primitiveDefaultMapSize;

   /**
    *    Scale level is key and scale index is value.
    */
   typedef map<uint32, uint32> scaleIndexByLevel_t;

   /**
    *    Map to go from scale level to discreet scale index.
    *    Scale level is key and scale index is value.
    */
   scaleIndexByLevel_t m_scaleIndexByLevel;
   
   /**
    *    The string prefix. Is 4 bits.
    */
   uint16 m_serverPrefix;

   /// Array of arguments to each type of feature.
   vector<TileFeatureArg*>* m_argsByTileFeatureTypeArray;

   /// The number of elements in m_argsByTileFeatureTypeArray
   int m_argsByTileFeatureTypeArraySize;
   
   /**
    *    The tile importance tables.
    *    Each layer has it's own, located at the layer ordinal number.
    *    NB! Not layer ID!
    */
   vector<TileImportanceTable*> m_importanceTables;

   /**
    *    Container of the default arguments for the primitives.
    */
   TileArgContainer m_defaultArgs;

   /**
    *    Vector containing pairs of ( mc2units, maxScaleLevel ) 
    *    for each detail level. The index in the vector corresponds
    *    to the detail level.
    */
   typedef vector< pair<int, uint16> > tileScale_t;
   
   /**
    *    Tilescales for each layer. The index in the vector corresponds
    *    to the layer number.
    */
   vector< tileScale_t* > m_tileScaleByLayer;

   /**
    *    Vector containing the available layer IDs and their
    *    description. For backwards compatibility.
    */
   vector<pair<uint32, MC2SimpleString> > m_layerIDsAndDescForComp;
   
   /**
    *    Vector containing the available layer IDs and their
    *    description.
    */
   TileMapLayerInfoVector m_layerIDsAndDesc;

   /**
    *    Map with layer id as key and layer number as value.
    */
   map<uint32, uint32> m_layerNbrByID;

   /**
    *    Vector of categorynotices which contain the name
    *    of the category and the types of the pois in it.
    */
   vector<TileMapCategoryNotice> m_categories;

   /**
    *    Vector of pointers to the categories to
    *    give to the client.
    */
   vector<const TileCategory*> m_clientCategories;

   /**
    *    Array of poi-types for the categories.
    *    The notices use them without copying.
    */
   TileFeature::tileFeature_t* m_categoryPoiTypes;

   /**
    *    The language of this object.
    */
   LangTypes::language_t m_lang;
   
   /// Detail level for the reserve maps
   int m_reserveDetailLevel;
   /// Number of maps around center to requsest for reserve maps.
   int m_extraTilesForReserve;
   /// CRC for the saved buffer
   uint32 m_crc;
   /// Timestamp for the desc
   uint32 m_timeStamp;

private:
   
   /**
    *   Puts the primitives for the featuretype into the vector
    *   with the default parameters set.
    */
   void getFeaturePrimitivesDefault(vector<TilePrimitiveFeature*>& primitives,
                                    featureInt featureType)
      const;
   
   /**
    *    Empty map to return when the feature type is not
    *    found in m_argsPerType. Should be static but Symbian...
    */
   const argTransferMap_t m_emptyTransferMap;

   /**
    *    Type of array containing the argTransferMap_t per
    *    item type.
    */
   typedef argTransferMap_t* argTransferPerTypeVector_t;

   /**
    *    Array replacing the old map. The index in the array
    *    corresponds to the tile feature type.
    */
   argTransferPerTypeVector_t m_argsPerTypeArray;

   /// Size of the m_argsPerTypeVector
   int m_argsPerTypeArraySize;
   
   /**
    *    Returns the argTransferMap for the supplied feature type.
    */
   const argTransferMap_t& getArgMap(featureInt featureType) const;
   
   /**
    *    The copyright string.
    */
   char* m_copyrightString;
  
protected:
   /**
    *    The copyright holder, containing boxes of map suppliers.
    */
   CopyrightHolder m_copyrightHolder;
  
   /// The default background color
   uint32 m_backgroundColor;
};

// --- Implementation of inlined methods. ---

inline const CopyrightHolder& 
TileMapFormatDesc::getCopyrightHolder() const 
{
   return m_copyrightHolder;
}

inline bool 
TileMapFormatDesc::isValidLayerID( uint32 layerID ) const {
   return m_layerNbrByID.find( layerID ) != m_layerNbrByID.end();
}

inline uint32 
TileMapFormatDesc::getLayerNbrFromID( uint32 layerID ) const
{
   map<uint32, uint32>::const_iterator it = m_layerNbrByID.find( layerID );
   MC2_ASSERT( it != m_layerNbrByID.end() );
   return it->second;
}

inline uint32
TileMapFormatDesc::getNbrLayers() const
{
   return m_layerIDsAndDesc.size();
}

// -- Class TileMapFormatCRC -----------------------------------

/// Class that keeps the crc for a TileMapFormatDesc
class TileMapFormatDescCRC {
public:

   /// Creates a new TileMapFormatDescCRC from a crc
   TileMapFormatDescCRC( uint32 crc );

   /// Saves the CRC to a buffer.
   bool save( SharedBuffer& buf ) const;
   /// Loads the CRC from a buffer.
   bool load( SharedBuffer& buf );
   /// Returns the CRC
   uint32 getCRC() const;
   
   
private:
   /// The crc
   uint32 m_crc;
};

#endif // TILE_MAP_FORMAT_DESC_H
