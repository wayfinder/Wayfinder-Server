/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERVER_TILE_MAP_H
#define SERVER_TILE_MAP_H

#include "TileMapConfig.h"
#include "TileMapTypes.h"
#include "MC2SimpleString.h"
#include "TileMap.h"
#include "ExtendedTileString.h"

#include <algorithm>
#include <map>

class TileFeature;
class TilePrimitiveFeature;
class TileMapFormatDesc;

/**
 *    Class that helps sorting the features indices.
 */
class FeatureIndexSorter {
   public:
      /**
       *   The constructor needs the tilemap. 
       */
      FeatureIndexSorter( const TileMap* theMap ) : m_map( theMap ) {}
       
      /**
       *    The most important feature gets the lowest number.
       *    @return The importance for the feature type. 
       */
      static int getFeatureImportance( int featureType ) {
         switch ( featureType ) {
            // Route features.
            case ( TileFeature::route_origin ) :
               return -3;
            case ( TileFeature::route_destination ) :
               return -2;
            case ( TileFeature::route_park_car ) :
               return -1;

            // The least important road features should be drawn below
            // the more important ones.
            case ( TileFeature::street_class_4_level_0 ) :
               return 20;
            case ( TileFeature::street_class_4 ) :
               return 21;
            case ( TileFeature::street_class_3_level_0 ) :
               return 22;
            case ( TileFeature::street_class_3 ) :
               return 23;
            case ( TileFeature::street_class_2_level_0 ) :
               return 24;
            case ( TileFeature::street_class_2_ramp ) :
               return 25;
            case ( TileFeature::street_class_2 ) :
               return 26;
            case ( TileFeature::street_class_1_level_0 ) :
               return 27;
            case ( TileFeature::street_class_1_ramp ) :
               return 28;
            case ( TileFeature::street_class_1 ) :
               return 29;
            case ( TileFeature::street_class_0_level_0 ) :
               return 30;
            case ( TileFeature::street_class_0_ramp ) :
               return 31;
            case ( TileFeature::street_class_0 ) :
               return 32;
               
            // Citycenters.
            case ( TileFeature::city_centre_2 ) :
               return 50;
            case ( TileFeature::city_centre_4 ) :
               return 51;
            case ( TileFeature::city_centre_5 ) :
               return 52;
            case ( TileFeature::city_centre_7 ) :
               return 53;
            case ( TileFeature::city_centre_8 ) :
               return 54;
            case ( TileFeature::city_centre_10 ) :
               return 55;
            case ( TileFeature::city_centre_11 ) :
               return 56;
            case ( TileFeature::city_centre_12 ) :
               return 57;
            // Other POI:s
            case ( TileFeature::airport ) :
               return 60;
            case ( TileFeature::church ) :
               return 61;
            case ( TileFeature::tourist_attraction ) :
               return 62;
            case ( TileFeature::railway_station ) :
               return 63;
            case ( TileFeature::commuter_railstation ) :
               return 64;
            case ( TileFeature::bus_station ) :
               return 65;
            case ( TileFeature::petrol_station ) :
               return 66;
            case ( TileFeature::tourist_office ) :
               return 67;
            case ( TileFeature::hospital ) :
               return 68;
            // The rest.
            default :
               // We want to sort the features on feature type so that
               // the transferred tilemaps will be smaller.
               return featureType + 1000;
         }
      }
      
      /**
       *    @return True if feature type a has a more important text than b.
       */
      bool operator() ( int a, int b ) const {
         // Check that everything is in order.
         MC2_ASSERT( m_map != NULL );
         MC2_ASSERT( m_map->getFeature( a ) != NULL );
         MC2_ASSERT( m_map->getFeature( b ) != NULL );
         
         // Get the features.   
         const TileFeature* aFeature = m_map->getFeature( a );
         const TileFeature* bFeature = m_map->getFeature( b );
         
         // Get the feature types.
         int aType = aFeature->getType();
         int bType = bFeature->getType();
        
         if ( aType != bType ) {
            mc2dbg8 << "Feature types: " << aType << ":" << bType << endl;
            return (getFeatureImportance( aType ) < 
                   getFeatureImportance( bType ));
         } 

         // Same type. Sort on image_name argument if present. 
         const StringArg* aImageName = 
            static_cast<const StringArg*> (
               aFeature->getArg( TileArgNames::image_name ) );
         const StringArg* bImageName = 
            static_cast<const StringArg*> (
               bFeature->getArg( TileArgNames::image_name ) );
         if ( aImageName != NULL && bImageName != NULL ) {
            return aImageName->getValue() < bImageName->getValue();
         }
         
         // Did not contain an image_name arg. 
         // The features are considered equal.
         return false;
      }
      
   private:
      /**
       *    The map.
       */
      const TileMap* m_map;
};

class WritableTileMap : public TileMap {
   
public:
   
   /**
    *    Creates a new WritableTileMap.
    *    @param params         The parameters used to request the map.
    *    @param mapDesc        The map description.
    *    @param importanceType The importance type.
    *    @param nbrFeatures    The number of features to allocate.
    */
   WritableTileMap(const TileMapParams& params, 
                   const TileMapFormatDesc& mapDesc,
                   uint16 importanceType,
                   int nbrFeatures );

   /**
    *
    */
   virtual ~WritableTileMap();
   
   /**
    *    Adds a feature to the map.
    */
   inline uint32 add( TileFeature* feature );

   /**
    *    Adds a name to the feature with specified feature index.
    */
   inline void addName( uint32 featureIndex, const char* name );
   /**
    * Adds a category id to the feature
    */
   inline void addCategory( uint32 featureIndex, uint16 category );

   /**
    * Add extended tile string to tile map.
    */
   inline void addExtendedString( const ExtendedTileString& string );

   /**
    *    Method to be called after everything is added to the map.
    * @param sortFeature If the feature index should be sorted.
    */
   inline void complete( bool sortFeatures = true );

   /**
    *    Returns a pointer to the current feature to be added
    *    to the map.
    */
   WritableTileFeature* getCurrentNewFeature(int32 type);

   /**
    * Merge another tilemap into this one
    */
   void merge( TileMap& other );

   /**
    * The number of not added features, call before complete.
    */
   void nbrNotAddedFeatures( int nbrNotAddedFeatures );

private:

   /**
    *    Last allocated feature index.
    */
   int m_lastFeature;

   /**
    * Temporary map used to build the vector m_strings.
    * String is key and string index is value.
    */
   map<MC2SimpleString, uint32> m_strIdxByStrMap;
   
   /**
    * Temporary map used to build the vector m_strIdxByFeatureIdx.
    * Feature index is key and string index value.
    */
   map<uint32, uint32> m_strIdxByFeatureIdxMap;
   /**
    * Temporary map for feature index to category id map.
    * Used for building real category map later.
    */
   map<uint32, uint16> m_categoryByFeatureIdxMap;
   /**
    * Temporary map for feature index to extended tile strings
    */
   map< uint32, TileMap::ExtendedStringTable > m_writableExtendedStrings;

};

// --- Inlined methods ---

inline uint32 
WritableTileMap::add( TileFeature* feature ) 
{
   MC2_ASSERT( feature == &m_features[ m_lastFeature ] );
   return m_lastFeature++;
}

inline void 
WritableTileMap::addName( uint32 featureIndex, const char* name ) 
{    
   MC2SimpleString str( name );

   uint32 strIdx = 0;
   map<MC2SimpleString, uint32>::const_iterator it = 
      m_strIdxByStrMap.find( str );
   if ( it != m_strIdxByStrMap.end() ) {
      // Already present among the strings.
      strIdx = it->second;
   } else {
      // Not present, so add it.
      strIdx = m_strings.size();
      m_strIdxByStrMap.insert( make_pair( str, strIdx ) );
      m_strings.push_back( str );
   }
   
   m_strIdxByFeatureIdxMap.insert( make_pair( featureIndex, strIdx ) );
   
}

inline void
WritableTileMap::addCategory( uint32 featureIndex, uint16 category ) {

   m_categoryByFeatureIdxMap.
      insert( make_pair( featureIndex, category ) );
}
inline void
WritableTileMap::addExtendedString( const ExtendedTileString& string ) {
   m_writableExtendedStrings[ string.getFeatureIndex() ].
      push_back( string );
}

inline void
WritableTileMap::complete( bool sort )
{
   // Everything is added to the map now.

   // Resize the vector and set the default string index to -1 (no string).

   m_strIdxByFeatureIdx.resize( m_features.size(), -1 );
   
   // Sort the features and strings.
   vector<int> featureIndexVec( m_features.size() );
   for ( uint32 i = 0; i < featureIndexVec.size(); ++i ) {
      featureIndexVec[ i ] = i;
   }
   if ( sort ) {
      FeatureIndexSorter featureIndexSorter( this );
       std::sort( featureIndexVec.begin(), featureIndexVec.end(),
                  featureIndexSorter );
   }
   
   vector<TileFeature> oldFeatures( m_features.size() );
   m_features.swap( oldFeatures );
   m_poiCategoryMap.clear();
   for ( uint32 i = 0; i < featureIndexVec.size(); ++i ) {
      // i is the new index.
      const int oldIndex = featureIndexVec[ i ];
      // Check if feature i has a string.
      map<uint32,uint32>::const_iterator findIt = 
         m_strIdxByFeatureIdxMap.find( oldIndex );
      if ( findIt != m_strIdxByFeatureIdxMap.end() ) {
         int strIdx = findIt->second;
         m_strIdxByFeatureIdx[ i ] = strIdx;
         m_featureIdxInTextOrder.push_back( i );
      }
      map<uint32, uint16>::const_iterator catIt = 
         m_categoryByFeatureIdxMap.find( oldIndex );
      if ( catIt != m_categoryByFeatureIdxMap.end() ) {
         m_poiCategoryMap.push_back( make_pair( i, catIt->second ) );
      }
      m_features[ i ] = oldFeatures[ oldIndex ];
      // Avoid that the args are deleted when the oldfeatures are 
      // destructed since the assignment operator 
      // only copies the arg pointers.
      oldFeatures[ oldIndex ].m_args.clear();

      // setup extended string table

      map<uint32, ExtendedStringTable >::const_iterator
         extStringIt = m_writableExtendedStrings.find( oldIndex );
      if ( extStringIt != m_writableExtendedStrings.end() ) {
         ExtendedStringTable::const_iterator estrIdx =
            extStringIt->second.begin();
         for ( ; estrIdx != extStringIt->second.end(); ++estrIdx ) {
            m_extendedStrings.
               push_back( ExtendedTileString( i, // real feature index
                                              estrIdx->getType(),
                                              estrIdx->getData() ) );
         }
      }
   }
}

inline void
WritableTileMap::nbrNotAddedFeatures( int nbrNotAddedFeatures ) {
   m_features.resize( m_features.size() - nbrNotAddedFeatures );
}

#endif // SERVER_TILE_MAP_H

