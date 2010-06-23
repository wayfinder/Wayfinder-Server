/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileCollectionIterator.h"

#include "TileCollectionNotice.h"

TileCollectionIterator::
TileCollectionIterator(const TileCollectionIterator& other,
                       bool firstImportanceOnly ) {
   *this = TileCollectionIterator( other );
   m_firstImportanceOnly = firstImportanceOnly;

}

TileCollectionIterator::
TileCollectionIterator( const TileCollectionNotice& notices,
                        uint32 serverPrefix,
                        bool gzip,
                        LangTypes::language_t lang,
                        bool firstImportanceOnly ):
   m_notices( &notices ),
   m_layerIdx( 0 ),
   m_detailIdx( notices.getTilesForAllDetails()[ 0 ].getStartDetail() ),
   m_tileIdx( 0 ),
   m_importanceNbr( 0 ),
   m_serverPrefix( serverPrefix ),
   m_gzip( gzip ),
   m_lang( lang ),
   m_data( true ),
   m_firstImportanceOnly( firstImportanceOnly ) {

   MC2_ASSERT( ! notices.getTilesForAllDetails().empty() );

   goToStart();
}

TileMapParams TileCollectionIterator::operator *() const {
   MC2_ASSERT( ! end() );

   int layer = m_notices->
      getTilesForAllDetails()[ m_tileIdx ].
      getTilesForDetail( m_detailIdx )->
      getLayerID( m_layerIdx );
   return TileMapParams( m_serverPrefix, 
                         (int)m_gzip, 
                         layer,                           
                         m_data ? 
                         TileMapTypes::tileMapData :
                         TileMapTypes::tileMapStrings ,
                         m_importanceNbr, 
                         // if data use swedish lang
                         m_data ? LangTypes::swedish : m_lang ,
                         m_latIdx, m_lonIdx,
                         m_detailIdx );
}

bool TileCollectionIterator::end() const {
   return m_tileIdx == m_notices->getTilesForAllDetails().size();
}

TileCollectionIterator& TileCollectionIterator::operator ++() {
   //
   // The order of the iteration is as follows:
   // 1) language and type 
   //   1.1) if type == data then lang =swedish, else lang = real lang
   // 2) Importance
   // 3) layer
   // 4) lon
   // 5) lat
   // 6) detail 
   // 7) tile
   //

   // toggle tileMap type
   m_data = ! m_data;

   if ( ! m_data ) {
      return *this;
   }

   if ( ! m_firstImportanceOnly ) {
      if ( m_importanceNbr < m_notices->
           getTilesForAllDetails()[ m_tileIdx ].
           getTilesForDetail( m_detailIdx )->getLastImportanceNbr( m_layerIdx ) ) {
         m_importanceNbr++;
         return *this;
      }
   }

   if ( m_layerIdx != m_notices->
        getTilesForAllDetails()[ m_tileIdx  ].
        getTilesForDetail( m_detailIdx )->getNbrLayers() - 1 ) {

      m_layerIdx++;

      m_importanceNbr = m_notices->
         getTilesForAllDetails()[ m_tileIdx  ].
         getTilesForDetail( m_detailIdx )->getFirstImportanceNbr( m_layerIdx );

      return *this;
   } else {
      m_layerIdx = 0;
   }

   // restart importance number and increase lon
   m_importanceNbr = m_notices->
      getTilesForAllDetails()[ m_tileIdx  ].
      getTilesForDetail( m_detailIdx )->getFirstImportanceNbr( m_layerIdx );


   m_lonIdx++;

   if ( m_lonIdx < m_endLonIdx ) {
      return *this;
   }

   // restart lon and increase lat
   m_lonIdx = m_notices->
      getTilesForAllDetails()[ m_tileIdx ].
      getTilesForDetail( m_detailIdx )->getStartLonIdx();

   m_latIdx++;

   if ( m_latIdx < m_endLatIdx ) {
      return *this;
   }

   // restart lat and increase detail level
   m_latIdx =  m_notices->
      getTilesForAllDetails()[ m_tileIdx ].
      getTilesForDetail( m_detailIdx )->getStartLatIdx();

   //
   // Detail Index note:
   // Max details = tile notices size + start detail, so 
   // substract start detail idx from current detail idx
   // 

   if ( m_detailIdx - m_notices->
        getTilesForAllDetails()[ m_tileIdx  ].getStartDetail()
        != m_notices->
        getTilesForAllDetails()[ m_tileIdx  ].
        getTilesNotices().size() ) {
      m_detailIdx++;
   }

   // restart detail and increase tile idx
   // if detail idx - start = size of details
   if ( m_detailIdx - m_notices->
        getTilesForAllDetails()[ m_tileIdx  ].getStartDetail()
        >= m_notices-> getTilesForAllDetails()[ m_tileIdx  ].
        getTilesNotices().size()  ) {

            
      if ( m_tileIdx != m_notices->getTilesForAllDetails().size() ) {
         m_tileIdx++;
      }

      if ( ! end() ) {
         m_detailIdx = m_notices->getTilesForAllDetails()[ m_tileIdx ].
            getStartDetail();
      }
   }
   // if we are not finished iterating update new start point
   // for lat/lon and importance nbr
   if ( ! end() ) {
      const TilesNotice& notice = *m_notices->
         getTilesForAllDetails()[ m_tileIdx ].
         getTilesForDetail( m_detailIdx );

      m_latIdx = notice.getStartLatIdx();
      m_lonIdx = notice.getStartLonIdx();
      m_endLatIdx = notice.getEndLatIdx() + 1;
      m_endLonIdx = notice.getEndLonIdx() + 1;

      m_importanceNbr = notice.getFirstImportanceNbr( m_layerIdx );

   }

   return *this;
}

void TileCollectionIterator::goToStart() {
   m_layerIdx = 0;
   m_tileIdx = 0;
   m_importanceNbr =  0;
   m_detailIdx = m_notices->getTilesForAllDetails()[ 0 ].getStartDetail();

   m_data = true;

   // setup start point
   const TilesNotice &notice = *m_notices->
      getTilesForAllDetails()[ m_tileIdx ].
      getTilesForDetail( m_detailIdx );

   m_importanceNbr = notice.getFirstImportanceNbr( 0 );
   m_latIdx = notice.getStartLatIdx();
   m_lonIdx = notice.getStartLonIdx();
   m_endLatIdx = notice.getEndLatIdx() + 1;
   m_endLonIdx = notice.getEndLonIdx() + 1;   

}

void TileCollectionIterator::goToEnd() {
   m_tileIdx = m_notices->getTilesForAllDetails().size();
}

bool TileCollectionIterator::
operator == ( const TileCollectionIterator& other ) const {
   if ( this == &other )
      return true;

   // if both have the same notice ptr and 
   // one of them have end() == true then 
   // we could do a quicker comparsion.
   //
   // The comparsion with an end iterator is done
   // more frequently than a full comparison.
   //
   if ( other.m_notices == m_notices ) { 
      if ( other.end() || end() ) {
         return other.end() && end();
      }
   }

   // else compare everything

   // TODO: optimize order
#define HELPCMP( x ) x == other.x   
   return 
      HELPCMP( m_notices ) &&
      HELPCMP( m_importanceNbr ) &&
      HELPCMP( m_latIdx ) &&
      HELPCMP( m_lonIdx ) &&
      HELPCMP( m_layerIdx ) &&
      HELPCMP( m_detailIdx ) &&
      HELPCMP( m_tileIdx ) &&
      HELPCMP( m_endLatIdx ) &&
      HELPCMP( m_endLonIdx ) &&
      HELPCMP( m_serverPrefix ) &&
      HELPCMP( m_gzip ) &&
      HELPCMP( m_lang ) &&
      HELPCMP( m_data );
}


std::ostream& operator << (ostream& out, const TileCollectionIterator& it ) {
   out << "notice ptr: " << it.m_notices << endl
       << "importanceNbr: " << it.m_importanceNbr << endl
       << "layer: " << it.m_layerIdx << endl
       << "lon/lat: " << it.m_lonIdx << "/" << it.m_latIdx << endl
       << "detail: " << it.m_detailIdx << endl
       << "tile: " << it.m_tileIdx << endl
       << "---" << endl
       << "endLatIdx/endLonIdx: " << it.m_endLatIdx << "/" << it.m_endLonIdx 
       << endl
       << "serverPrefix: " << it.m_serverPrefix << endl
       << "gzip: " << it.m_gzip << endl
       << "lang: " << it.m_lang << endl
       << "data: " << it.m_data << endl
       << "firstImportanceOnly: " << it.m_firstImportanceOnly << endl
       << "tilesForAllDetails.Size: " 
       << it.m_notices->getTilesForAllDetails().size() << endl
       << "notice ptr: " 
       << it.m_notices->getTilesForAllDetails()[ it.m_tileIdx ].
      getTilesForDetail( it.m_detailIdx ) << endl;
   
   return out;
}
