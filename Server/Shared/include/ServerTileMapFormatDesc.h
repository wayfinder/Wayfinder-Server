/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERVER_TILE_MAP_FORMAT_DESC_H
#define SERVER_TILE_MAP_FORMAT_DESC_H

#include "TileMapConfig.h"
#include "TileMapFormatDesc.h"
#include "GfxFeature.h"
#include "TileArgNames.h"
#include "ServerTileCategories.h"
#include "TileMapParams.h"
#include "GDColor.h"
#include "MapRights.h"
#include "ImageTable.h"

class Packet;
class TileCollectionNotice;

/**
 *
 */
struct TileDrawSetting {

   /**
    *    Constructor that resets everything.
    */
   TileDrawSetting() {
      reset();
   }
   
   /**
    *    Reset all values to default values.
    */
   void reset() {
      m_color = MAX_UINT32;
      m_borderColor = MAX_UINT32;
      m_width = MAX_UINT32;
      m_widthMeters = MAX_UINT32;
      m_borderWidth = MAX_UINT32;
      m_nameType = MAX_UINT32;
      m_level = MAX_UINT32;
      m_level1 = MAX_UINT32;
      m_fontType = MAX_UINT32;
      m_fontSize = MAX_UINT32;
      m_imageName = "";
   }


      
      
   // MAX_UINT32 means invalid argument.
   
   uint32 m_color;

   uint32 m_borderColor;

   uint32 m_width;

   uint32 m_widthMeters;

   uint32 m_borderWidth;

   uint32 m_nameType;

   uint32 m_level;
   
   uint32 m_level1;
   
   uint32 m_fontType;
   
   uint32 m_fontSize;

   MC2SimpleString m_imageName;

};

class TileDrawSettings : public map<int, TileDrawSetting>
{
   public:
      TilePrimitiveFeature::tileFeature_t m_primitive;
};

/**
 *    Params to be used when creating the ServerTileMapFormatDesc.
 *    I.e. the ones that must be compared when caching the
 *    descs.
 */
class STMFDParams {
public:
   static const uint32 DEFAULT_LAYERS;
   static const uint32 DEFAULT_SERVER_PREFIX;
   static const uint32 HIGH_END_SERVER_PREFIX;

   /// used when params are not important. 
   STMFDParams(): 
      m_lang( LangTypes::english ), 
      m_nightMode( false ),
      m_layers( DEFAULT_LAYERS ),
      m_drawSettingVersion( 0 ),
      m_imageSet( ImageTable::DEFAULT ),
      m_serverPrefix( DEFAULT_SERVER_PREFIX )
   {}

   STMFDParams( LangTypes::language_t language, 
                bool nightMode ):
      m_lang( language ),
      m_nightMode( nightMode ),
      m_layers( DEFAULT_LAYERS ),
      m_drawSettingVersion( 0 ),
      m_imageSet( ImageTable::DEFAULT ),
      m_serverPrefix( DEFAULT_SERVER_PREFIX ) {
   }

   STMFDParams( LangTypes::language_t language, 
                bool nightMode,
                uint32 layers,
                uint32 drawSettingVersion,
                ImageTable::ImageSet imageSet,
                uint32 serverPrefix = DEFAULT_SERVER_PREFIX ):
      m_lang( language ),
      m_nightMode( nightMode ),
      m_layers( layers ),
      m_drawSettingVersion( drawSettingVersion ),
      m_imageSet( imageSet ),
      m_serverPrefix( serverPrefix ) {
   }
   
   int operator<( const STMFDParams& other ) const {
      if ( m_lang != other.m_lang ) {
         return m_lang < other.m_lang;
      } else if ( m_nightMode != other.m_nightMode ) {
         return m_nightMode < other.m_nightMode;
      } else if ( m_layers != other.m_layers ) {
         return m_layers < other.m_layers;
      } else if ( m_drawSettingVersion != other.m_drawSettingVersion ) {
         return m_drawSettingVersion < other.m_drawSettingVersion;
      } else if ( m_serverPrefix != other.m_serverPrefix ) {
         return m_serverPrefix < other.m_serverPrefix;
      } else {
         return m_imageSet < other.m_imageSet;
      }
   }

   /// Save to packet.
   int save( Packet* packet, int& pos ) const;
      
   /// Load from packet.
   int load( const Packet* packet, int& pos );
      
   friend ostream& operator<<( ostream& str, const STMFDParams& me ) {
      str << LangTypes::getLanguageAsString( me.m_lang );
      return str << " nightmode: " << me.m_nightMode 
                 << " drawSettingVersion:  " << me.m_drawSettingVersion
                 << " imageSet: " << me.m_imageSet
                 << " layers: 0x" << hex << me.m_layers << dec
                 << " server prefix: " << me.m_serverPrefix;
   }

   /// @return language
   LangTypes::language_t getLanguage() const {
      return m_lang;
   }

   /// @return whether to use night mode or not
   bool useNightMode() const { 
      return m_nightMode;
   }

   /// @return Draw setting version.
   uint32 getDrawSettingVersion() const {
      return m_drawSettingVersion;
   }

   /// @return Layer mask.
   uint32 getLayers() const {
      return m_layers;
   }

   /// @return the image set to use.
   ImageTable::ImageSet getImageSet() const {
      return m_imageSet;
   }

   /// @return Server "version".
   uint32 getServerPrefix() const {
      return m_serverPrefix;
   }

   /// @return true if both instances are equal.
   bool operator == ( const STMFDParams& other ) const {
      return
         m_nightMode == other.m_nightMode &&
         m_lang == other.m_lang &&
         m_layers == other.m_layers &&
         m_drawSettingVersion == other.m_drawSettingVersion &&
         m_imageSet == other.m_imageSet &&
         m_serverPrefix == other.m_serverPrefix;
   }

   /// The language
   LangTypes::language_t m_lang;
   /// whether or not to use night mode
   bool m_nightMode;

private:

   uint32 m_layers; ///< bit field for layers
   uint32 m_drawSettingVersion; ///< version for draw setting
   ImageTable::ImageSet m_imageSet; ///< the image set to use
   uint32 m_serverPrefix; ///< Server "version".
};

/**
 *    TileMapFormatDesc with extra functionality useful 
 *    when generating maps.
 */
class ServerTileMapFormatDesc : public TileMapFormatDesc {   
public:
   typedef set<MC2SimpleString> paramSet_t;
   static const byte SPECIAL_CUSTOM_POI_MASK = 0x80;

      /// Image name by tilefeature type.
      typedef map<int, MC2SimpleString> imageByType_t;
      
      explicit ServerTileMapFormatDesc( const STMFDParams& settings );

      /**
       *    Constructor.
       */
      ServerTileMapFormatDesc( const STMFDParams& settings,
                               const CopyrightHolder& copyrights );

      /**
       *    Destructor
       */
      ~ServerTileMapFormatDesc();


      /**
       *  Saves the data to a databuffer and calculates the crc.
       *  @param buf          Buffer to save the data into
       *  @param timeStampPos The position of the timeStamp is saved here.
       */
      void save( BitBuffer& buf, uint32* timeStampPos = NULL );

      /**
       *  Updates the timeStamp in the buffer where a desc was
       *  saved.
       *  @param buf          The buffer.
       *  @param timeStampPos Position from a previous save.
       */
      static void updateTimeStamp( BitBuffer& buf, uint32 timeStampPos );

      /**
       *    Get the first importance notice of the specified type.
       *    @param   layerID  The layer ID.
       *    @param   type     The type.
       *    @return  The importance notice or NULL if not found.
       */
      const TileImportanceNotice* getImportanceOfType( uint32 layerID,
                                                       uint16 type ) const;


      /**
       *  Sets the hardcoded data.
       */
      virtual void setData();

      /**
       *  Get the server prefix.
       */
      inline uint32 getServerPrefix() const;

      /**
       *    Get the tilefeature type for the gfxfeature.
       *    @param   gfxFeature  The gfx feature.
       *    @return  The tilefeature type (positive value) or a negative
       *             value if no corresponding tilefeature type could
       *             be found.
       */
      int16 getTileFeatureTypeFromGfxFeature(
                     const GfxFeature* gfxFeature ) const;

      /**
       *    Returns true if the feature type should
       *    be represented as a closed polygon.
       */
      bool closedFeatureType( int16 type ) const;

      /**
       *    Returns the poi names by type. Used by
       *    SimplePoiMap.
       */
      const imageByType_t& getUsedPoiNamesByType() const {
         return m_usedImageByTileFeatureType;
      }
   
      /**
       *    Checks if the specified GfxFeature is of the importance.
       *    @param   gfxFeature     The GfxFeature to check.
       *    @param   importance     The importance to check against.
       *    @param   sqMeterToSqPixelFactor  Square meter to sq. pixel
       *                                     factor.
       *    @param   param          The TileMapParam for the importance.
       *    @return  If gfxFeature is part of importance.
       */
      bool isGfxOfImportance( 
                           const GfxFeature* gfxFeature,
                           const TileImportanceNotice* importance,
                           float64 sqMeterToSqPixelFactor,
                           const TileMapParams& param ) const;

      /**
       *    Checks if the feature of the specified type contains a name.
       *    @param   type  The feature type.
       *    @return  If the feature contains names.
       */
      bool hasName( uint16 type ) const;

      /**
       *    Returns true if the type of feature has the arg argName.
       */
      bool hasArg( featureInt type, 
                   TileArgNames::tileArgName_t argName ) const {
         argsByTypeAndName_t::const_iterator it = 
            m_argsByTypeAndName.find( type );
         if ( it != m_argsByTypeAndName.end() ) {
            argsByName_t::const_iterator jt =  it->second->find( argName );
            if ( jt != it->second->end() ) {
               return true;
            }
         }
         return false;
      }

      /**
       *    Private importance type enum. Contains the importance types
       *    that groups together several GfxFeature types.
       */
      enum importanceType_t {
         ALL_AREA_FEATURES = GfxFeature::NBR_GFXFEATURES + 1,
         ALL_OTHER_FEATURES,
         ALL_ROUTE_FEATURES,
         LAND_AND_MAJOR_CITYCENTRES,
         // Place holders for old sorting order of streets.
         // This order lead to that small streets were drawn on top
         // of big ones.
         PLACE_HOLDER_STREET_MAIN_AND_MEDIUM_CITYCENTRES_AND_AIRPORTS,
         PLACE_HOLDER_STREET_FIRST_AND_MINOR_CITYCENTRES,
         PLACE_HOLDER_STREET_SECOND_AND_FERRY,
         PLACE_HOLDER_STREET_FOURTH_AND_RAILWAY,
         PLACE_HOLDER_POIS,
         // New sorting order of streets.
         // Draw big streets above small ones.
         STREET_FOURTH_AND_RAILWAY,
         STREET_THIRD_AND_MORE,
         STREET_SECOND_AND_FERRY,
         STREET_FIRST_AND_MINOR_CITYCENTRES,
         STREET_MAIN_AND_MEDIUM_CITYCENTRES_AND_AIRPORTS,
         POIS,
         TRAFFIC,
      };

      /**
       *    @return The number of importances for the tile described
       *            by the submitted param.
       */
      inline uint32 getNbrImportances( const TileMapParams& param ) const;
      inline uint32 getNbrImportances( uint32 layerNbr,
                                       uint32 scale,
                                       uint32 detailLevel ) const;

      /**
       *    @return The number of pixels for the tile described
       *            by the submitted param.
       */
      inline uint32 getNbrPixels( const TileMapParams& param ) const;
     
      /**
       *    The min, default and max level of the road features.
       */
      static const int c_minRoadFeatureLevel;
      static const int c_defaultRoadFeatureLevel;
      static const int c_maxRoadFeatureLevel;
     
      /**
       *    Get the road level including offset for the tilemap.
       *    @param   levelWithoutOffset Level of the road without offset,
       *                                i.e. positive means above ground.
       */
      static int getRoadLevelWithOffset( int levelWithoutOffset );
    
      /**
       *    Get all the bitmap names present for a given image set.
       *    @param   bitmaps     Will be filled with all bitmap names.
       *    @param   suffix      The suffix, for instance ".png" that will
       *                         be added to the bitmap names.
       *    @param   imageSet    The image set to use.
       */
      static void getBitmapNames( set<MC2SimpleString>& bitmaps,
                                  const MC2SimpleString& suffix,
                                  ImageTable::ImageSet imageSet );

      /// Returns the names of the needed bitmaps without suffix
      static void getNeededBitmapNames( set<MC2String>& names, 
                                        ImageTable::ImageSet imageSet );
   
      /**
       *    @return The maximum scale where there are any features
       *            for the layer id.
       */
      inline uint32 getMaxScale( uint32 layerID ) const;
    
      /**
       *    Add all importances available for the specified param
       *    to the vector.
       *    @param   param    The param.
       *    @param   params   [OUT] The vector will be filled with
       *                      the params of all available importances
       *                      for param. The first element in the vector
       *                      will be param.
       */
      void getAllImportances( const MC2SimpleString& param,
                              vector<MC2SimpleString>& params ) const;
 
      /**
       *    Get the STMFDParams representing this object.
       */
      inline STMFDParams getSTMFDParams() const;

      /**
       *    Check if the tile settings are the same for the two
       *    layers.
       */
      bool sameTileSettingsForLayerID( int layerID, 
                                       int otherLayerID ) const;
   /**  
    *   Generates a TileCollectionNotice needed for the supplied bbox
    *   on all levels. 
    *   @param collection the tile collection to be filled in
    *   @param otherParams this will be filled with parameters that are not map tiles
    *   @param bbox      The bounding box to get the params for.
    *   @param layers    The layers to get the params for.
    *   @param useGzip   True if gzip should be used.
    *   @param language  The language to use ( for the string maps ).
    *   @param minScale  The minimum scale allowed.
    *   @param extraParams additional params to be added to collection or otherParams
    */
   void getAllNotices( TileCollectionNotice& collection,
                       paramSet_t& otherParams,
                       const MC2BoundingBox& bbox,
                       const set<int>& layers,
                       int useGzip, 
                       LangTypes::language_t language,
                       uint32 minScale,
                       const paramSet_t* extraParams ) const;

   /**
    * @param poi the poi type that wants an image name
    * @return image name, not including file extension. Just plain name which
    *         can be used with TMap
    */
   const char* getPOIImageName( ItemTypes::pointOfInterest_t poi ) const;

   /**
    * @param tileFeatureType The tileFeatureType for the poi to look for.
    * @return The bitmap name of the poi.
    */
    MC2String getPOIImageName( int tileFeatureType ) const;

   /**
    * @param itemType the item type that wants an image name
    * @return image name, not including file extension. Just plain name which
    *         can be used with TMap
    */
   MC2String getItemImageName( ItemTypes::itemType itemType ) const;
   
   /**
    * Check if the feature type is within the scale range for this parameter.
    * @param   tileType The tilefeature type.
    * @param   param    The tilemap param.
    * @return  True if the feature is within the scale range, false othwerwise.
    */
   bool checkFeatScaleRange( int tileType, const TileMapParams& param ) const;
   /**
    * Determines tile feature type from poi type.
    * @return tile feature type. Returns TileFeature::nbr_tile_features on failure.
    */
   TileFeature::tileFeature_t 
   getTileFeatureForPOI( ItemTypes::pointOfInterest_t poiType ) const;

   /// @return true if tile feature type is special custom poi
   inline static bool isSpecialCustomPOI( int tileFeatureType );

   /// @return whether this ServerTileMapFormatDesc is for a high end device.
   bool highEndDevice() const { 
      return getServerPrefix() == STMFDParams::HIGH_END_SERVER_PREFIX; 
   }
   
private:
   /// initialize member variables
   void init( const STMFDParams& settings );

      /**
       *    Build the array used in TileMapFormatDesc for
       *    getting the arguments for Features.
       */
      void buildArgsByTypeArray();
      
      /**
       *    Create internal tables for bitmaps etc.
       */
      void createPOITables(); 
 
      /**
       *    Create argvector for tile feature type.
       *    @param   type  The tile feature type.
       */
      void createArgVectorForType( int type );
      
      /**
       *    Add arguments containing default scale range to the
       *    arg vector if there should be any default scale range
       *    arguments for the specified feature type.
       *    @param   type     The tile feature type.
       *    @param   args     [IN/OUT] Arg vector that might have
       *                      default scale range args added to it.
       */
      void addDefaultScaleRange( 
                        int16 type, vector<TileFeatureArg*>& args );
      
      /**   
       *    Adds feature with thresholds to the importance table.
       *    The threshold is defined as the number of square pixels
       *    on the screen the feature must have it should be included. 
       *    The detaillevels may be divided into several steps, i.e.
       *    it is checked for each step 
       *    (detaillevel 1.0, 1.25, 1.5, 1.75 etc.) that the features
       *    are above the threshold to be included.
       *
       *    @param   layerID                 The layer.
       *    @param   zoomFactor              The zoom factor between the 
       *                                     different detaillevels.
       *    @param   nbrStepsPerDetailLevel  The number of steps for 
       *                                     each detaillevel.
       *    @param   typeAndSqPixelThreshold Vector of feature type
       *                                     and corresponding threshold
       *                                     (square pixels on the screen)
       *                                     needed for inclusion of the
       *                                     feature.
       */      
      void addThresholdDataToImportanceTable( 
                        uint32 layerID,
                        float64 zoomFactor,
                        int nbrStepsPerDetailLevel,
                  const vector<pair<int, int> >& typeAndSqPixelThreshold );

      /**
       *    Adds static features to the importance table. I.e. features 
       *    that always should be displayed on the same scalelevel.
       *    @param   layerID           The layer.
       *    @param   typeAndScaleLevel typeAndScaleLevel vector of
       *                               feature types and the scalelevel
       *                               that they should be included at.
       */
      void addStaticDataToImportanceTable(
                  uint32 layerID,
                  const vector<pair<int, uint16> >& typeAndScaleLevel );
   
      /**
       *    Add a new importance table.
       *    @param   fetchStrings     True if the strings should be fetched
       *                              without clicks from the user.
       *    @param   layerID          The layer ID.
       *    @param   descrStr         A string describing the layer.
       *    @param   updatePeriodMin  Default update period for client in
       *                              minuts. 0 means no update.
       *    @param   optional         True if the layer is optional for the
       *                              client.
       *    @param   visibleByDefault True if the layer should be visible
       *                              when added the first time.
       *    @param   affectedByACPMode If the ACP mode setting should apply
       *                               for this layer.
       *    @param   fetchLayerWhenACPEnabled   Only valid if 
       *                                        affectedByACPMode is true!
       *                                        If the layer should be fetched 
       *                                        when ACP mode is enabled. This 
       *                                        setting should override any 
       *                                        other visibility settings.
       *
       *    @param   fetchLayerWhenACPDisabled   Only valid if 
       *                                         affectedByACPMode is true!
       *                                         If the layer should be fetched 
       *                                         when ACP mode is disabled. This 
       *                                         setting should override any 
       *                                         other visibility settings.
       */
      void addImportanceTable( bool fetchStringsByDefault,
                               uint32 layerID,
                               const MC2SimpleString& descrStr,
                               uint32 updatePeriodMin = 0,
                               bool optional = false,
                               bool visibleByDefault = true,
                               bool affectedByACPMode = false,
                               bool fetchLayerWhenACPEnabled = false,
                               bool fetchLayerWhenACPDisabled = false );

      /**
       *    Initializes the tiles and importances.
       */
      void initTilesAndImportances();

      /**
       *    Get the tilefeature type for the street gfxfeature.
       *    @param   gfxFeature  The gfx feature (street).
       *    @param   roadClass   The road class of the gfx feature.
       *    @return  The tilefeature type.
       */
      int16 getStreetTileTypeFromGfxFeature( const GfxFeature* gfxFeature,
                                             byte roadClass ) const;

      /**
       *    Creates the categories.
       */
      void createCategories( LangTypes::language_t lang );
    
      // Forward.
      struct tileSettings_t;
 
      /**
       *    Initializes the tile sizes for the specified layer number.
       *    This method will resize m_tileScaleByLayer if needed.
       *    @param layerNbr     The layer number that the tile sizes
       *                        apply to.
       *    @param tileSettings The tile settings.
       *    
       */
      void initTileSizesForLayer( uint32 layerNbr,
                                  const tileSettings_t& tileSettings );

   /** 
    * Decides whether or not area threshold should be used when
    * choosing if a feature type should be included in an importance.
    */
   bool useAreaThresholdForType( GfxFeature::gfxFeatureType featureType ) const;
      
      /**
       *    Coordinates needed so that the coords arg won't crash.
       */
      TileMapCoords m_coords;
                  
      /// Image name by tilefeature type.
      imageByType_t m_imageByTileFeatureType;
   
      /// Image name by tilefeature type. Removed the unused ones.
      imageByType_t m_usedImageByTileFeatureType;

      /// Poitype by tilefeature type.
      typedef map<ItemTypes::pointOfInterest_t, int>  typeByPOI_t;
      typeByPOI_t m_tileFeatureTypeByPOIType;
      
      /// Scale range. First is minscale and second is maxscale.
      typedef pair<int, int> scaleRange_t;

      /**
       * Default scale range indexed by tilefeature type. 
       * Not all tilefeature will have a default scale range.
       */
      typedef map<int, scaleRange_t> scaleRangeByType_t;     
      scaleRangeByType_t m_scaleRangeByType;
      
      typedef map<featureInt, 
         vector<TileFeatureArg*> > argsByType_t;

      /**
       *    The arguments for each feature type.
       */
      argsByType_t m_argsByTileFeatureType;
       
      /**
       *    Type of map containing the argTransferMap_t per item
       *    type.
       */
      typedef map<featureInt,
         argTransferMap_t*> argTransferPerType_t;
            
      /**
       *    Map containing the arguments that should be transferred
       *    down to the primitives keyed by complex feature type.
       */
      argTransferPerType_t m_argsPerType;
      
      typedef map<TileArgNames::tileArgName_t, TileFeatureArg*> argsByName_t;
      
      /// Maps feature type to argTransferMap_t
      typedef map<featureInt, argsByName_t*> argsByTypeAndName_t;
      
      /**
       *    Arguments sorted by tilefeature type.
       */
      argsByTypeAndName_t m_argsByTypeAndName;
      
      /// POI circles.
      //set<ItemTypes::pointOfInterest_t> m_circlePOIs;

      /**
       *    Temporary struct, used to build the tables
       *    m_imageByTileFeatureType and m_tileFeatureTypeByPOIType.
       *    Implementation of constructor last in ServerTileMapFormatDesc
       *    to avoid long compilation times with MC2SimpleString and
       *    gcc-2.96
       */
      struct TileNotice {
         /**
          *    Constructor.
          *    @param   tilefeatureType   The tilefeature type.
          *    @param   imageName         The image name for the 
          *                               tilefeature.
          *    @param   poiType           [Optional] Set to the poi-type
          *                               if there's a one-to-one 
          *                               relationship between tilefeature
          *                               type and poi-type. 
          */
         TileNotice( ServerTileCategories::category_t category,
                     int tilefeatureType,
                     ImageTable::ImageCode image,
                     ItemTypes::pointOfInterest_t poiType = 
                     ItemTypes::nbr_pointOfInterest );
         
         /// Image name.
         ImageTable::ImageCode m_image;
         
         /// Tilefeature type.
         int m_tileFeatureType;

         /**
          * Poi-type. Set to ItemTypes::nbr_pointOfInterest if there's
          * no one-to-one relationship between 
          * tilefeature type and poi-type. 
          */
         ItemTypes::pointOfInterest_t m_poiType;

         /// Category of the feature
         ServerTileCategories::category_t m_category;
         /// Level
         int m_level;
      };

      /// Size of m_categoryPoiTypes
      int m_nbrCategoryPoiTypes;
      
      /**
       *   Static vector of tile notices used by createPOITables and
       *   createCategories
       */
      static vector<TileNotice> c_tileNotices;

      /**
       *   Static function that creates the tile notices.
       */
      static vector<TileNotice> createTileNotices();
      
      /**
       *    Create the default settings for all features.
       */
      void createDefaultSettings();

      /**
       * Create old settings for old clients.
       */
      void createOldDefaultSettings();

      /**
       *    Adds arguments that should be transferred to the primitives.
       *    @param   featureType    The feature type.
       *    @param   primitiveType  The primitive type.
       */
      void addArgTransferMap( int featureType, int primitiveType );

      /**
       *    Writes the categories to the format desc.
       *    @param latin1 If true, the categories will be written in
       *           latin1. This is done once, then it is written using
       *           utf-8 due to a bug in older WF-clients.
       */
      void writeCategories( SharedBuffer& buf, bool latin1 );
      
      /**
       *    Help method to createDefaultSettings.
       *    Checks if the specified argument vector contains
       *    all invalid arguments or not. If all args are the same, 
       *    then the vector is resized to only contain one element.
       *    
       *    @param   vec            Vector with arguments.
       *    @param   invalidVector  Vector with invalid arguments.
       *    @return True if there were any valid arguments.
       */
      template <class T> bool
      checkAndUpdateArgVector( T& vec, T& invalidVector ) 
      {
         // Check if invalid arguments.
         if ( ! ( vec == invalidVector ) ) {
            // Contains valid arguments.
            
            // Check if all arguments are the same.
            T allSame;
            allSame.resize( vec.size(), vec.front() );

            if ( vec == allSame ) {
               // All arguments are same.
               // Reduce to only contain one.
               vec.resize( 1 );
            }
            return true;
         } else {
            // All arguments invalid.
            return false;
         }
      }

      /**
       *    Struct containing the settings for the tile sizes for
       *    each layer.
       */
      struct tileSettings_t {
         /**
          *    How many meter the most detailed tile is.
          */
         int m_meters;

         /**
          *    How many pixels the default client has.
          */
         int m_pixels;

         /**
          *    The DPI of the default client.
          */
         int m_dpi;

         /**
          *    The zoom factor between the different detaillevels.
          */
         float64 m_zoomFactor;

         /**
          *   When to exchange from a less detailed tile to the next tile: 
          *   scalelevel( i ) + 
          *   exchangeFactor* ( scalelevel( i + 1) - scalelevel( i ) )
          *   I.e:
          *   1.0 means change to the new tile when the scalelevel is
          *   the same as the scalelevel for the next tile. 
          *   0.5 means change halways between the less detailed tile and
          *   the next tile.
          */
         float64 m_exchangeTileFactor;

         /**
          *    The total number of tile detail levels.
          */
         int m_detailLevels;

         /**
          *   Check if the two tilesettings are the same.
          */
         bool operator == (const tileSettings_t& other ) const;
      };

      /**
       *    Contains tilesettings indexed by layer number.
       */
      vector<tileSettings_t> m_tileSettingsByLayerNbr;

      inline uint32 makeColor (GDUtils::Color::imageColor dayColor, 
                               GDUtils::Color::imageColor nightColor) const;
      inline uint32 makeColor(uint32 dayRed, uint32 dayGreen, uint32 dayBlue, 
                              uint32 nightRed, 
                              uint32 nightGreen, 
                              uint32 nightBlue) const;
      inline uint32 makeColor(GDUtils::Color::imageColor dayColor, 
                              uint32 nightRed, 
                              uint32 nightGreen, 
                              uint32 nightBlue) const;


   bool m_nightMode; ///< whether the settings should be for night mode or not.
   uint32 m_drawSettingVersion; ///< draw setting version 
   uint32 m_layers; ///< layer field
   ImageTable::ImageSet m_imageSet; ///< the image set to use.
   /// Text color.
   uint32 m_textColor;
   /// Top horizon color.
   uint32 m_topHorizonColor;
   /// Bottom horizon color.
   uint32 m_bottomHorizonColor;
}; // class ServerTileMapFormatDesc

// Inlines
inline bool 
ServerTileMapFormatDesc::isSpecialCustomPOI( int tileFeatureType ) {
   return 
      tileFeatureType == TileFeature::special_custom_poi_2 ||
      tileFeatureType == TileFeature::special_custom_poi_4 ||
      tileFeatureType == TileFeature::special_custom_poi_7 ||
      tileFeatureType == TileFeature::special_custom_poi_10 ||
      tileFeatureType == TileFeature::special_custom_poi_15 ||
      tileFeatureType == TileFeature::special_custom_poi_20 ||
      tileFeatureType == TileFeature::special_custom_poi_30;
}

inline uint32 
ServerTileMapFormatDesc::getServerPrefix() const
{
   return m_serverPrefix;
}

inline uint32 
ServerTileMapFormatDesc::getNbrImportances( uint32 layerNbr,
                                            uint32 scale,
                                            uint32 detailLevel ) const
{
   return m_importanceTables[ layerNbr ]
            ->getNbrImportanceNbrs( scale, detailLevel );
}
                                           
inline uint32 
ServerTileMapFormatDesc::getNbrImportances( 
                              const TileMapParams& param ) const
{
   return m_importanceTables[ getLayerNbrFromID( param.getLayer() ) ]
            ->getNbrImportanceNbrs( 0, param.getDetailLevel() );
}

inline uint32 
ServerTileMapFormatDesc::getNbrPixels( 
                              const TileMapParams& param ) const
{
   return m_tileSettingsByLayerNbr[ 
      getLayerNbrFromID( param.getLayer() ) ].m_pixels;
}
      
inline uint32 
ServerTileMapFormatDesc::getMaxScale( uint32 layerID ) const
{
   const tileScale_t& tileScale = 
      *m_tileScaleByLayer[ getLayerNbrFromID( layerID ) ];
   return tileScale.back().second;
}

inline STMFDParams 
ServerTileMapFormatDesc::getSTMFDParams() const
{
   return STMFDParams( m_lang, m_nightMode, m_layers, m_drawSettingVersion,
                       m_imageSet, getServerPrefix() );
}

bool
inline ServerTileMapFormatDesc::tileSettings_t::operator == (const tileSettings_t& other ) const 
{
   return m_meters == other.m_meters &&
      m_pixels == other.m_pixels &&
      m_dpi == other.m_dpi &&
      fabs( m_zoomFactor - other.m_zoomFactor ) < 0.001 &&
      fabs( m_exchangeTileFactor - 
            other.m_exchangeTileFactor ) < 0.001 &&
      m_detailLevels == other.m_detailLevels;
}


#endif // SERVER_TILE_MAP_FORMAT_DESC_H

