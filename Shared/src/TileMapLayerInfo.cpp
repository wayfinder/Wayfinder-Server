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

#include "TileMapLayerInfo.h"
#include "BitBuffer.h"

template<class X, class Y> 
static bool setReportChanged( X& a, const Y& b ) {
   if ( a != b ) {
      a = b;
      return true;
   } else {
      return false;
   }
}

TileMapLayerInfo::TileMapLayerInfo()   
{
   m_id = MAX_UINT32;
   m_updatePeriod = 0;
   m_isOptional = false;
}

void
TileMapLayerInfo::setIDAndName( uint32 id,
                                const MC2SimpleString& name )
{
   m_name = name;
   m_id   = id;
}

bool
TileMapLayerInfo::setVisible( bool visible )
{
   return setReportChanged( m_visible, visible );
}

bool
TileMapLayerInfo::setUpdatePeriodMinutes( uint32 period )
{
   return setReportChanged( m_updatePeriod, period );
}

bool
TileMapLayerInfo::setOptional( int optional )
{
   return setReportChanged( m_isOptional, optional );
}

bool
TileMapLayerInfo::setServerOverrideNumber( uint8 nbr )
{
   return setReportChanged( m_serverOverride, nbr );
}

bool
TileMapLayerInfo::setPresent( bool present )
{
   return setReportChanged( m_presentInTileMapFormatDesc, present );
}

int
TileMapLayerInfo::getSizeInDataBuffer() const
{
   return 4 + 4 + m_name.length() + 1 + 4 + 1 + 1 + 1 + 1 + 2;
}

int
TileMapLayerInfo::save( BitBuffer& buf ) const
{
   int length = 0;
   {      
      // Create temporary buffer for easier skipping.
      BitBuffer buf2( buf.getCurrentOffsetAddress(),
                      getSizeInDataBuffer() );
      
      buf2.writeNextBALong( 0 ); // Length, fill in later
      int startPos = buf2.getCurrentOffset();
      
      buf2.writeNextBALong( m_id );
      buf2.writeNextString( m_name );
      buf2.writeNextBALong( m_updatePeriod );
      buf2.writeNextBits( m_transient,1 );
      buf2.writeNextBits( m_isOptional, 1 );
      buf2.writeNextBits( m_serverOverride, 8 );
      buf2.writeNextBits( m_presentInTileMapFormatDesc, 1 );
      buf2.writeNextBits( m_visible, 1 );
      buf2.writeNextBits( m_alwaysFetchStrings, 1 );
      buf2.writeNextBits( 3, 1 );
      buf2.alignToByte();

      buf2.writeNextBits( m_affectedByACPMode, 1 );
      buf2.writeNextBits( m_fetchLayerWhenACPEnabled, 1 );
      buf2.writeNextBits( m_fetchLayerWhenACPDisabled, 1 );
      // Write place holders for the rest of the byte.
      buf2.writeNextBits( 5, 1 );
      buf2.alignToByte();
     
      length = buf2.getCurrentOffset() - startPos;
   }
   buf.writeNextBALong( length );
   buf.readPastBytes( length );
   
   return length + 4;
}

int
TileMapLayerInfo::load( BitBuffer& buf )
{
   int nbrBytes = buf.readNextBALong();
   {
      // Create temporary buffer for easier skipping.
      BitBuffer buf2( buf.getCurrentOffsetAddress(), nbrBytes );
      
      buf2.readNextBALong( m_id );
      buf2.readNextString( m_name );
      buf2.readNextBALong( m_updatePeriod );
      buf2.readNextBits( m_transient,1 );
      buf2.readNextBits( m_isOptional, 1 );
      buf2.readNextBits( m_serverOverride, 8 );
      buf2.readNextBits( m_presentInTileMapFormatDesc, 1 );
      buf2.readNextBits( m_visible, 1 );
      buf2.readNextBits( m_alwaysFetchStrings, 1 );
      buf2.alignToByte();
      if ( buf2.getNbrBytesLeft() >= 1 ) {
         buf2.readNextBits( m_affectedByACPMode, 1 );
         buf2.readNextBits( m_fetchLayerWhenACPEnabled, 1 );
         buf2.readNextBits( m_fetchLayerWhenACPDisabled, 1 );
      } else {
         // Default setting is to ignore the ACP mode.
         m_affectedByACPMode = false;
         m_fetchLayerWhenACPEnabled = false;
         m_fetchLayerWhenACPDisabled = false;
      }
   }
   buf.readPastBytes( nbrBytes );
   return nbrBytes + 4;
}

void 
TileMapLayerInfo::setAffectedByACPMode( bool affected )
{
   m_affectedByACPMode = affected;
}

bool 
TileMapLayerInfo::isAffectedByACPMode() const
{
   return m_affectedByACPMode;
}

void 
TileMapLayerInfo::setFetchLayerWhenACPEnabled( bool fetch )
{
   m_fetchLayerWhenACPEnabled = fetch;
}

bool 
TileMapLayerInfo::getFetchLayerWhenACPEnabled() const
{
   return m_fetchLayerWhenACPEnabled;
}

void 
TileMapLayerInfo::setFetchLayerWhenACPDisabled( bool fetch )
{
   m_fetchLayerWhenACPDisabled = fetch;
}

bool 
TileMapLayerInfo::getFetchLayerWhenACPDisabled() const
{
   return m_fetchLayerWhenACPDisabled;
}

int
TileMapLayerInfoVector::getSizeInDataBuffer() const
{
   int sum = 4 + 4; // Length + version
   for ( const_iterator it = begin();
         it != end();
         ++it ) {
      sum += it->getSizeInDataBuffer();
   }
   return sum;
}

int
TileMapLayerInfoVector::save( BitBuffer& buf ) const
{
   buf.writeNextBALong( 0 ); // version
   buf.writeNextBALong( size() );
   for ( const_iterator it = begin();
         it != end();
         ++it ) {
      it->save( buf );
   }
   return getSizeInDataBuffer();
}


int
TileMapLayerInfoVector::load( BitBuffer& buf )
{
   clear();   
   buf.readNextBALong(); // version
   int length = buf.readNextBALong();
   resize( length );
   for ( iterator it = begin();
         it != end();
         ++it ) {
      it->load( buf );
   }
   return getSizeInDataBuffer();
}

const TileMapLayerInfo*
TileMapLayerInfoVector::getLayerInfo( uint32 layerID ) const
{
   for ( const_iterator it = begin();
         it != end();
         ++it ) {
      if ( (*it).getID() == layerID ) {
         return &(*it);
      }
   }
   return NULL;
}


bool
TileMapLayerInfoVector::updateLayers( TileMapLayerInfoVector& clientVector,
                                      const set<int>& existinLayers )
{
   // The clientVector will be the target of this operation.
   // Afterwards the vector is copied.
   int changed = 0;

   // Add changes from the client vector to our settings.
   for( TileMapLayerInfoVector::iterator it = clientVector.begin();
        it != clientVector.end();
        ++it ) {
      TileMapLayerInfo* myInfo = NULL;
      // Find the info from the server.
      for ( TileMapLayerInfoVector::iterator jt = this->begin();
            jt != this->end();
            ++jt ) {
         if ( jt->getID() == it->getID() ) {
            myInfo = &(*jt);
            break;
         }
      }
      changed += it->setPresent( myInfo &&
                ( existinLayers.find( it->getID() ) != existinLayers.end() ) );
      if ( ! myInfo ) {
         continue;
      }
      // These are always correct from the server by definition
      changed += it->setOptional( myInfo->isOptional() );
      // Non-optional layers can not be turned off.
      if ( ! it->isOptional() ) {
         changed += it->setVisible( true );
      }
      if ( it->getServerOverrideNumber() !=
           myInfo->getServerOverrideNumber() ) {
         // Force the update into the vector.
         changed +=
            it->setUpdatePeriodMinutes( myInfo->getUpdatePeriodMinutes() );
         changed += it->setVisible( myInfo->isVisible() );
         // But only do it once.
         it->setServerOverrideNumber( myInfo->getServerOverrideNumber() );
      } else {
         // Keep the info that the client entered
      }
   }
   // Add layers to the vector that are not present yet.
   for ( iterator jt = begin();
         jt != end();
         ++jt ) {
      bool found = false;
      for ( iterator it = clientVector.begin();
            it != clientVector.end();
            ++it ) {
         if ( jt->getID() == it->getID() ) {
            found = true;
            break;
         }
      }
      if ( ! found ) {
         // Insert that into the client vector.
         ++changed;
         clientVector.push_back( *jt );
      }
   }

   // This vector will now be the same as the other one.
   *this = clientVector;
   
   return changed;
}

bool
TileMapLayerInfoVector::updateLayersToDisplay( set<int>& layerIDs ) const
{
   set<int> newLayers;
   for( const_iterator jt = begin();
        jt != end();
        ++jt ) {
      if ( jt->isVisible() && jt->isPresent() ) {
         newLayers.insert( jt->getID() );
      }
   }
   newLayers.swap( layerIDs );
   return newLayers != layerIDs;
}
