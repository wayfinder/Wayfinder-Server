/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARAMSNOTICE_H
#define PARAMSNOTICE_H

#include "SharedBuffer.h"
#include "TileMapParams.h"

// ----------------------- ParamsNotice -------------------------

/**
 *    A notice describing a collection of TileMapParams.
 */
struct ParamsNotice {
  
   /**
    *    @return If the two notices are equal.
    */
   bool operator == ( const ParamsNotice& other ) const {
      return m_startLatIdx    == other.m_startLatIdx &&
             m_endLatIdx      == other.m_endLatIdx &&
             m_startLonIdx    == other.m_startLonIdx &&
             m_endLonIdx      == other.m_endLonIdx &&
             m_detailLevel    == other.m_detailLevel &&
             m_nbrImportances == other.m_nbrImportances &&
             m_layerID        == other.m_layerID;
   }
  
   /**
    *    @return True if the param is inside the ParamsNotice.
    */
   bool inside( const TileMapParams& param ) const { 
      return m_detailLevel == (int)param.getDetailLevel() &&
             m_layerID == (uint32) param.getLayer() &&
             m_startLatIdx <= param.getTileIndexLat() &&
             m_endLatIdx >= param.getTileIndexLat() &&
             m_startLonIdx <= param.getTileIndexLon() &&
             m_endLonIdx >= param.getTileIndexLon(); 
      // importance not used.
   }

   /**
    *    Load from buffer.
    */
   void load( SharedBuffer& buf ) {
      m_layerID = buf.readNextBALong();
      m_detailLevel = (int) buf.readNextBALong();
      m_nbrImportances = (int) buf.readNextBALong();
      m_startLatIdx = (int)  buf.readNextBALong();
      m_endLatIdx = (int)  buf.readNextBALong();
      m_startLonIdx = (int)  buf.readNextBALong();
      m_endLonIdx = (int)  buf.readNextBALong();
   }
   
   /**
    *    Save to buffer.
    */
   void save( SharedBuffer& buf ) const {
      buf.writeNextBALong( m_layerID );
      buf.writeNextBALong( m_detailLevel );
      buf.writeNextBALong( m_nbrImportances );
      buf.writeNextBALong( m_startLatIdx );
      buf.writeNextBALong( m_endLatIdx );
      buf.writeNextBALong( m_startLonIdx );
      buf.writeNextBALong( m_endLonIdx );
   }

   void dump( ostream& str ) const {
      str << "ParamsNotice: m_layerID = " << m_layerID 
          << ", m_detailLevel = " << m_detailLevel
          << ", m_startLatIdx = " << m_startLatIdx
          << ", m_endLatIdx = " << m_endLatIdx
          << ", m_startLonIdx = " << m_startLonIdx
          << ", m_endLonIdx = " << m_endLonIdx << endl;
   }

   /// Start lat index.
   int m_startLatIdx;

   /// End lat index.
   int m_endLatIdx;

   /// Start lon index.
   int m_startLonIdx;

   /// End lon index.
   int m_endLonIdx;

   /// The detail level.
   int m_detailLevel;

   /// Number importances.
   int m_nbrImportances;

   /// The layer id.
   uint32 m_layerID;
  
};

/// first is layerid and second is detaillevel.
typedef pair<int,int> layerAndDetail_t;

/// Map, containing ParamsNotices, keyed by layer and detail.
struct paramsByLayerAndDetail_t : public map<layerAndDetail_t, ParamsNotice>
{
   /// Update the map with the param.
   void update( const TileMapParams& param ) {

      layerAndDetail_t ld( param.getLayer(), 
                           param.getDetailLevel() );
      
      iterator findit = find( ld );
      if ( findit == end() ) {
         // New layer and detaillevel. Add notice.
         ParamsNotice notice;
         notice.m_startLatIdx = 
            notice.m_endLatIdx = param.getTileIndexLat();
         notice.m_startLonIdx = 
            notice.m_endLonIdx = param.getTileIndexLon();
         notice.m_detailLevel = param.getDetailLevel();
         notice.m_layerID = param.getLayer();
         // Add the notice.
         insert( make_pair( ld, notice ) );
      } else {
         // Update existing notice.
         ParamsNotice& notice = (*findit).second;
         if ( notice.m_startLatIdx > param.getTileIndexLat() ) {
            notice.m_startLatIdx = param.getTileIndexLat();
         }
         if ( notice.m_endLatIdx < param.getTileIndexLat() ) {
            notice.m_endLatIdx = param.getTileIndexLat();
         }
         if ( notice.m_startLonIdx > param.getTileIndexLon() ) {
            notice.m_startLonIdx = param.getTileIndexLon();
         }
         if ( notice.m_endLonIdx < param.getTileIndexLon() ) {
            notice.m_endLonIdx = param.getTileIndexLon();
         }
      }
   }
};

#endif
