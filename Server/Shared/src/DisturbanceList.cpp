/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DisturbanceList.h"

#include "NonStdStl.h"

#include <functional>
#include <algorithm>

//-------------------------------------------------------------------
//  DisturbanceListElement
//-------------------------------------------------------------------


DisturbanceListElement::DisturbanceListElement( const IDPair_t& id,
                                                uint32 costSec )
      : IDPair_t( id ),
        m_factor( 65536 ), // will be 1
        m_useFactor( false ),
        m_time( costSec )
{
}

DisturbanceListElement::DisturbanceListElement(uint32 mapID,
                                               uint32 nodeID,
                                               uint32 rawFactor,
                                               bool useFactor,
                                               uint32 timeSec) :
      IDPair_t(mapID, nodeID),
      m_factor(rawFactor),
      m_useFactor(useFactor),
      m_time(timeSec)
{
}

DisturbanceListElement::DisturbanceListElement(uint32 mapID,
                                               uint32 nodeID) :
      IDPair_t(mapID, nodeID),
      m_factor(MAX_UINT32),
      m_useFactor(true),
      m_time( MAX_UINT32 )
{
}

DisturbanceListElement::DisturbanceListElement(uint32 mapID,
                                               uint32 nodeID,
                                               float factor) :
      IDPair_t(mapID, nodeID)
{   
   // Fix-point
   m_factor = floatToRaw( factor );
   m_useFactor = true;
   m_time = 0;
}


float
DisturbanceListElement::getFactor() const
{
   return float(m_factor) / float(MAX_UINT16);
}

uint32
DisturbanceListElement::getRawFactor() const
{
   return m_factor;
}

bool
DisturbanceListElement::getUseFactor() const
{
   return m_useFactor;
}

uint32
DisturbanceListElement::getTime() const
{
   return m_time;
}


//-------------------------------------------------------------------
//  DisturbanceList
//-------------------------------------------------------------------

DisturbanceList::DisturbanceList()
{
   // Nothing
}

void
DisturbanceList::deleteVectorContents(DisturbanceVector& vect)
{
   DisturbanceVector::iterator it(vect.begin());
   while (it != vect.end() ) {
      delete *it;
      ++it;
   }
}

DisturbanceList::~DisturbanceList()
{
   DisturbanceMap_t::iterator it(m_distMap.begin());
   while ( it != m_distMap.end() ) {
      DisturbanceVector* vect = it->second;
      deleteVectorContents(*vect);
      delete vect;
      ++it;
   }
}

void
DisturbanceList::addDisturbance(DisturbanceListElement* dist)
{
   DisturbanceMap_t::iterator it(m_distMap.find(dist->getMapID()));
   if ( it == m_distMap.end() ) {
      // No vector exists for this mapid.
      DisturbanceVector* vect = new DisturbanceVector;
      vect->push_back(dist);
      m_distMap.insert(DistVectPair_t(dist->getMapID(), vect));
   } else {
      // Vector exists.
      it->second->push_back(dist);
   }
   m_allDistVect.push_back(dist);
}

void
DisturbanceList::takeDisturbances( DisturbanceVector& vect,
                                   uint32 mapID )
{
   if ( mapID == MAX_UINT32 ) {
      for( DisturbanceVector::const_iterator it = vect.begin();
           it != vect.end();
           ++it ) {
         addDisturbance( *it );
      }
   } else {
      // Now all the mapids must be the same as mapID.
      MC2_ASSERT( vect.end() ==
                  find_if( vect.begin(),
                           vect.end(),
                           not1(compose1(
                              bind1st(
                                 equal_to<uint32>(), mapID),
                              mem_fun(&DisturbanceListElement::getMapID)))));
      
      DisturbanceMap_t::iterator it(m_distMap.find(mapID));
      if ( it == m_distMap.end() ) {
         DisturbanceVector* mapVect = new DisturbanceVector;         
         m_distMap.insert(DistVectPair_t(mapID, mapVect) );
         // Nice. Now we know that the vector in the map is
         // empty, so we can just swap the vectors and we're done
         mapVect->swap( vect );
      } else {
         // Add the dists from our vector to the one in the map.
         it->second->insert( it->second->end(),
                             vect.begin(),
                             vect.end() );
      }
      // Also insert into the alldistvect.
      m_allDistVect.insert( m_allDistVect.end(),
                            vect.begin(),
                            vect.end() );
   }
   vect.clear();
}


void
DisturbanceList::avoidNode(uint32 mapID,
                           uint32 nodeID,
                           bool avoidOtherNodeToo)
{
   addDisturbance(new DisturbanceListElement(mapID, nodeID));
   
   if ( avoidOtherNodeToo ) {
      avoidNode(mapID, nodeID ^ 0x80000000, false);
   }
}

void
DisturbanceList::addPenalty(uint32 mapID,
                            uint32 nodeID,
                            float penaltyFactor,
                            bool otherNodeToo)
{
   addDisturbance(new DisturbanceListElement(mapID, nodeID, penaltyFactor));
   
   if ( otherNodeToo ) {
      addPenalty( mapID, nodeID ^ 0x80000000, penaltyFactor, false );
   }
}

void
DisturbanceList::addDelayTime(uint32 mapID,
                              uint32 nodeID,
                              uint32 timeSec,
                              bool avoidOtherNodeToo)
{
   addDisturbance(new DisturbanceListElement(mapID, nodeID, 65536, false,
                                         timeSec));
   if ( avoidOtherNodeToo ) {
      addDelayTime(mapID, nodeID ^ 0x80000000, timeSec, false);
   }
}

const DisturbanceVector*
DisturbanceList::getDisturbances(uint32 mapID) const
{
   if ( mapID != MAX_UINT32 ) {
      DisturbanceMap_t::const_iterator it(m_distMap.find(mapID));
      if ( it != m_distMap.end() ) {
         return it->second; 
      } else {
         return NULL;
      }
   } else {
      return &m_allDistVect;
   }
}

int
DisturbanceList::getDisturbances( DisturbanceVector& vect,
                                  uint32 mapID ) const
{
   const DisturbanceVector* toAdd = getDisturbances( mapID );
   if ( toAdd ) {
      vect.insert( vect.end(), toAdd->begin(), toAdd->end() );
      return toAdd->size();
   } else {
      return 0;
   }
}

