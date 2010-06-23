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

#include "TileMapQuery.h"

#include "BitBuffer.h"
#include "MC2SimpleString.h"
#include "SharedBuffer.h"
#include "TileMapParams.h"
#include <algorithm>
#include "TileMapParamTypes.h"
#include "TimeUtility.h"
#include "STLStringUtility.h"

#include "TileMapBufferHolder.h"

// Sorts the params on importance nbr.
struct TileMapParamSorter
{
   // Sorts the params on importance nbr.
   bool operator() ( const char* paramStr1, 
                     const char* paramStr2 ) const
   {      
      TileMapParams p1( paramStr1, true/*Might be invalid map, like bitmap*/ );
      TileMapParams p2( paramStr2, true/*Might be invalid map, like bitmap*/ );
   
      if ( p1.getLayer() != p2.getLayer() ) {
         return p1.getLayer() < p2.getLayer();
      } else if ( p1.getTileIndexLat() != p2.getTileIndexLat() ) {
         return p1.getTileIndexLat() < p2.getTileIndexLat();
      } else if ( p1.getTileIndexLon() != p2.getTileIndexLon() ) {
         return p1.getTileIndexLon() < p2.getTileIndexLon();
      } else if ( p1.getDetailLevel() != p2.getDetailLevel() ) {
         return p1.getDetailLevel() < p2.getDetailLevel();
      } else if ( p1.getLanguageType() != p2.getLanguageType() ) {
         return p1.getLanguageType() < p2.getLanguageType();
      } else if ( p1.getTileMapType() != p2.getTileMapType() ) {
         return p1.getTileMapType() < p2.getTileMapType();
      // The important thing here is that the importance nbr is
      // compared last, so that the importances for a tile can be merged.
      } else if ( p1.getImportanceNbr() != p2.getImportanceNbr() ) {
         return p1.getImportanceNbr() < p2.getImportanceNbr();
      } else {
         return strcmp(paramStr1,paramStr2) < 0;
      }
   }
};

void
TileMapQuery::updateStillWanted()
{
   internalParamSet_t tmpWanted( m_wanted.begin(), m_wanted.end() );
   m_stillWanted.swap( tmpWanted );
}

void
TileMapQuery::sortParams()
{
   m_startTime = TimeUtility::getCurrentTime();
   MC2_ASSERT( m_wanted.empty() );
   
   m_wanted.reserve( 1024 );
   
   mc2dbg8 << "[TMQ]: Filling m_wanted" << endl;
   // FIXME: Calc size first.
   {
      SharedBuffer tmp( m_paramBuf->getBufferAddress(),
                        m_paramBuf->getBufferSize() );
      while ( ! tmp.bufferFilled() ) {
         m_wanted.push_back( tmp.readNextString() );
      }
   }
   
   // This is also called by reputFailed
   mc2dbg8 << "[TMQ]: Done filling m_wanted" << endl;
   if ( m_wanted.size() < 10000 ) {
      mc2dbg8 << "[TMQ]: Sorting " << m_wanted.size() << " params" << endl;   
      std::sort( m_wanted.begin(), m_wanted.end(), TileMapParamSorter() );
      mc2dbg8 << "[TMQ]: Params sorted" << endl;
   } else {
      mc2dbg8 << "[TMQ]: Not sorting " << m_wanted.size() << " params" << endl;
   }

   m_nbrAdded = 0;
   m_nbrFailed = 0;
   m_lastPrintTime = 0;
   
   m_nextWantedIt = m_wanted.begin();
   updateStillWanted();
}

template<class CONTAINER> SharedBuffer* makeBuffer( const CONTAINER& cont )
{
   // Calculate the size
   uint32 stringSize = 0;
   for ( typename CONTAINER::const_iterator it = cont.begin();
         it != cont.end();
         ++it ) {
      stringSize += it->length() + 1;
   }
   SharedBuffer* retBuf = new SharedBuffer( stringSize );
   typename CONTAINER::const_iterator it_end = cont.end();
   for ( typename CONTAINER::const_iterator it = cont.begin();
         it != it_end;
         ++it ) {
      retBuf->writeNextString( *it );
   }

   mc2dbg8 << "[TMQ]: Bufsize = " << retBuf->getCurrentOffset() << endl;
   
   return retBuf;
}

TileMapQuery::TileMapQuery( const paramVect_t& wanted )
      : m_nbrAdded( 0 ),
        m_paramBufToDelete( makeBuffer( wanted ) )
        
{
   m_statusPrints = false;
   m_paramBuf = m_paramBufToDelete;
   // Sort the wanted maps according to importance
   sortParams();
   // Default to no time limit
   m_timeLimit = MAX_UINT32;
}

TileMapQuery::TileMapQuery( const paramSet_t& wanted )
      : m_nbrAdded( 0 ),
        m_paramBufToDelete( makeBuffer( wanted ) )
{
   m_statusPrints = false;
   m_paramBuf = m_paramBufToDelete;
   // Sort the wanted maps according to importance
   sortParams();
   // Default to no time limit
   m_timeLimit = MAX_UINT32;
   
}

TileMapQuery::TileMapQuery( const SharedBuffer& buf )
      : m_nbrAdded(0)
{
   m_statusPrints = false;
   m_paramBufToDelete = new SharedBuffer( buf.getBufferAddress(),
                                          buf.getBufferSize() );
   m_paramBuf = m_paramBufToDelete;
   // Sort the wanted maps according to importance
   sortParams();
   // Default to no time limit
   m_timeLimit = MAX_UINT32;
}

bool
TileMapQuery::reputFailed()
{
   if ( m_stillWanted.empty() ) {
      return false;
   }

   m_nextWantedIt = m_wanted.begin();
   mc2dbg << "[TileMapQuery]::reputFailed rewinds the loop." << endl;

   return true;
}

TileMapQuery::~TileMapQuery()
{
   delete m_paramBufToDelete;
}

int
TileMapQuery::getNbrWanted() const
{
   return m_wanted.size();
}

int
TileMapQuery::getNbrAdded() const
{
   return m_nbrAdded;
}

int
TileMapQuery::getNbrFailed() const
{
   return m_nbrFailed;
}

void
TileMapQuery::realGetNextParams( vector<MC2SimpleString>& params,
                                 int maxNbr )
{
   if ( isDone() ) {
      return;
   }
   uint32 startSize = 0;
   while( int( params.size() - startSize ) < maxNbr ) {
      if ( m_nextWantedIt == m_wanted.end() ) {
         return;
      }

      if ( m_stillWanted.find( *m_nextWantedIt ) != m_stillWanted.end() ) {
         // Add param.
         params.push_back( *m_nextWantedIt );
         
         // First check if we added a tilemap, so that we can do some 
         // tricks.
         if ( ! TileMapParamTypes::isMap( *m_nextWantedIt ) ) {
            // Not a tilemap, so it's not possible to eat up the following
            // params.
            ++m_nextWantedIt;
            continue;
         }
               
         // Now skip all subsequent importances since the TileMapRequest
         // will return all importances for the requested tile.
         TileMapParams curParam( *m_nextWantedIt );
         bool onlyImportanceDiffers = true;
            
         while ( onlyImportanceDiffers &&
                 ++m_nextWantedIt != m_wanted.end() &&
                 TileMapParamTypes::isMap( *m_nextWantedIt ) ) {
            TileMapParams nextParam( *m_nextWantedIt );
            nextParam.setImportanceNbr( curParam.getImportanceNbr() );
            onlyImportanceDiffers = 
               curParam.getAsString() == nextParam.getAsString();
            
            // Debug.
            if ( onlyImportanceDiffers ) {
               mc2dbg8 << "[TMQ]: Skipping " << *m_nextWantedIt
                       << " since " << curParam.getAsString()
                       << " is already to be requested." << endl;
            }
         }
      } else {
         // Next param.
         ++m_nextWantedIt;
      }
   }
}

int
TileMapQuery::getNextParams( vector<MC2SimpleString>& params,
                             int maxNbr )
{
   int paramOrigSize = params.size();
   realGetNextParams( params, maxNbr );
   return params.size() - paramOrigSize;
}

int
TileMapQuery::addOneBuffer( const MC2SimpleString& param,
                            SharedBuffer* buf )
{
   bufVect_t oneEntryVector( 1, bufVect_t::value_type( param, buf ) );
   return addBuffers( oneEntryVector );
}

uint32
TileMapQuery::getNbrDone() const
{
   return m_wanted.size() - m_stillWanted.size();
}

uint32
TileMapQuery::getNbrStillWanted() const
{
   return m_stillWanted.size();
}

int
TileMapQuery::addBuffers( const bufVect_t& bufs )
{
   if ( isDone() ) {
      mc2log << warn
             << "[TMQ]: addBuffers(..) already done not taking more" << endl;
      return 0;
   }
   // Result vector.
   bufVect_t toAdd;
   toAdd.reserve( bufs.size() );

   const bufVect_t::const_iterator it_end = bufs.end();
   // Check to see that the buffers haven't been sent once before.
   for ( bufVect_t::const_iterator it = bufs.begin();
         it != it_end;
         ++it ) {
      // Param set contains const char*
      internalParamSet_t::iterator findIt =
         m_stillWanted.find( it->first.c_str() );
      
      if ( findIt == m_stillWanted.end() ) {
         mc2dbg8 << "[TMQ]: " << MC2CITE( it->first ) << " not wanted" << endl;
         // Not wanted.
         continue;
      }

      if ( it->second == NULL ) {
         // Failed map
         mc2dbg << "[TMQ]: " << MC2CITE( it->first ) << " failed" << endl;
         ++m_nbrFailed;
      } else {
         toAdd.push_back( *it );
         // Never request it again.
         m_stillWanted.erase( findIt );
      }
   }

   printStatus();

   int nbrAddedThisTime = internalAddBuffers( toAdd );
   m_nbrAdded += nbrAddedThisTime;
   
   return nbrAddedThisTime;
}

int
TileMapQuery::addBuffers( const vector<TileMapBufferHolder>& buffers )
{
   bufVect_t tmpVect(buffers.size());
   // FIXME: Do not copy this many times
   for ( uint32 i = 0, n = buffers.size(); i < n; ++i ) {
      if ( buffers[i].isGood() ) {
         tmpVect[i].first  = buffers[i].getDesc();
         tmpVect[i].second = buffers[i].getBuffer();
      }
   }
   return addBuffers( tmpVect );
}

void TileMapQuery::printStatus() {

   if ( ( ( TimeUtility::getCurrentTime() - m_lastPrintTime ) > 1000 ) ||
        (  getNbrStillWanted() < 100 ) ) {
      if ( m_statusPrints ) {
         mc2dbg << "[TMQ]: " << getNbrStillWanted() << " buffers to go "
                << m_nbrFailed << " failed" << endl;
         uint32 timeElapsed_s = (TimeUtility::getCurrentTime() - m_startTime)/1000;
         double eta_s = 1e99;
         uint32 nbrDone = getNbrDone();
         double speed = nbrDone / double(timeElapsed_s);
         if ( speed != 0 ) {
            eta_s = ( getNbrStillWanted() + nbrDone ) / speed;
            eta_s -= timeElapsed_s;
         }
         mc2dbg << "[TMQ]: Time elapsed "
                << STLStringUtility::splitSeconds(timeElapsed_s) << " ETA: "
                << STLStringUtility::splitSeconds(uint32(eta_s)) << endl;
         mc2dbg << "[TMQ]: " << speed
                << " params/s" << endl;
      }
      m_lastPrintTime = TimeUtility::getCurrentTime();
//       // WARNING ! PROFILING !
//       if ( m_nbrAdded > 10000 ) {
//          mc2log << info << "[TMQ]: Profiling exit" << endl;
//          exit(0);
//       }
   }
}
