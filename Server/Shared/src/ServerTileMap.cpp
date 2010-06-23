/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapConfig.h"
#include "TileMap.h"
#include "ServerTileMap.h"
#include "TileFeature.h"
#include "TileMapParams.h"
#include "TileMapTypes.h"
#include "TileMapFormatDesc.h"
#include "MC2BoundingBox.h"
#include "GfxConstants.h"

WritableTileFeature* 
WritableTileMap::getCurrentNewFeature(int32 type)
{
   WritableTileFeature* feat =
      static_cast<WritableTileFeature*>(&m_features[m_lastFeature]);
   feat->setType( type );
   return feat;
}

WritableTileMap::WritableTileMap(const TileMapParams& params, 
                                 const TileMapFormatDesc& desc,
                                 uint16 importanceType,
                                 int nbrFeatures ):
   m_lastFeature( 0 ) 
{
   m_features.resize( nbrFeatures );

   // These two are really only used in the client
   m_pixelBoxes.resize( nbrFeatures );
   m_vectorProxies.resize( nbrFeatures );

   m_params = new TileMapParams(params.getAsString().c_str());
   m_mc2Scale = int( desc.getCoordAndScaleForTile( params.getLayer(),
                                                   params.getDetailLevel(),
                                                   params.getTileIndexLat(),
                                                   params.getTileIndexLon(),
                                                   m_referenceCoord ) *
      GfxConstants::METER_TO_MC2SCALE );
   // Adjust the reference coord so it is a multiple of the mc2scale.
   snapCoordToPixel( m_referenceCoord );

}

WritableTileMap::~WritableTileMap() {

}


void WritableTileMap::merge( TileMap& other ) {
   if ( other.empty() ) {
      return;
   }

   // copy all features
   // blarghh! stupid copy constructor in class TileFeature
   // so we have to do assignment from old vector after
   // resizing the m_features
   {
      vector<TileFeature> features;
      features.swap( m_features );
   
      m_features.resize( features.size() + 
                         distance( other.begin(), other.end() ) );
      for ( uint32 i = 0; i < features.size(); ++i ) {
         m_features[ i ] = features[ i ];
         features[ i ].clearArguments();
      }
   }

   TileMap::iterator featureIt = other.begin();
   TileMap::iterator featureItEnd = other.end();
   for ( uint32 featureIdx = 0;
         featureIt != featureItEnd;
         ++featureIt, ++m_lastFeature, ++featureIdx ) {
      // copy feature
      m_features[ m_lastFeature ] = *featureIt;

      // this should is not really required here, the strings are
      // located in another tilemap
      const MC2SimpleString* str = other.getStringForFeature( featureIdx );
      if ( str != NULL ) {
         addName( m_lastFeature, str->c_str() );
      } 
      featureIt->clearArguments();

   }



   // if there is no features, this one could contain strings 
   // and categories
   if ( other.nbrFeatures() == 0 &&
        other.getNbrFeaturesWithText() != 0 ) {
      // this tile contains strings and categories, add them

      // copy the feature index in text order
      uint32 firstStringPos = m_strings.size();
      for ( int32 i = 0; i < other.getNbrFeaturesWithText(); ++i ) {
         int textOrder = other.getFeatureIdxInTextOrder( i );
         if ( other.getStringForFeature( textOrder ) ) {
            m_featureIdxInTextOrder.push_back( firstStringPos + textOrder );
         }
      }
      // merge the strings in normal order
      m_strings.insert( m_strings.end(),
                        other.getStrings().begin(), other.getStrings().end() );

      // now we need to offset the "string index by feature index" and merge
      // it too
      const uint32 endOffset = other.getStrIdxByFeatureIdx().size();
      const uint32 startOffset = m_strIdxByFeatureIdx.size();
      m_strIdxByFeatureIdx.resize( m_strIdxByFeatureIdx.size() +
                                   endOffset );
      for ( uint32 i = 0; i < endOffset; ++i ) {
         m_strIdxByFeatureIdx[ startOffset + i ] =
            firstStringPos + other.getStrIdxByFeatureIdx()[ i ];
      }
      // finaly copy the category map
      uint32 poiCategoryMapSizeBefore = m_poiCategoryMap.size();
      m_poiCategoryMap.reserve( poiCategoryMapSizeBefore + 
                                other.getPOICategories().size() );
      for ( uint32 i = 0; i < other.getPOICategories().size(); ++i ) {
         TileMap::poiCategoryMap_t::value_type otherValue = 
            other.getPOICategories()[ i ];
         otherValue.first += poiCategoryMapSizeBefore;
         m_poiCategoryMap.push_back( otherValue );
      }
   }

   // bitwise-and empty importances mask since both tiles must
   // have empty importances on the same places for it to
   // be a real empty importances
   m_emptyImportances &= other.getEmptyImportances();
}
