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

#include "IDPairVector.h"
#include "UnexpandedRoute.h"
#include "Packet.h"
#include "SubRouteVector.h"
#include "DriverPref.h"
#include "ItemTypes.h"
#include "DisturbanceDescription.h"
#include "NodeBits.h"

UnexpandedRoute::UnexpandedRoute(const SubRouteVector& srVect,
                                 const DriverPref& driverPref )
{
   m_driverPref = new DriverPref(driverPref);
   // Put the route in our vector.
   putRouteInVector( m_ids, srVect );
}

UnexpandedRoute::~UnexpandedRoute()
{
   delete m_driverPref;
}

void
UnexpandedRoute::putRouteInVector(vector<IDPair_t>& vect,
                                  const SubRouteVector& srVect)
{
   for ( uint i = 0; i < srVect.size(); ++i ) {
      // Add origin
      vect.push_back(IDPair_t(srVect[i]->getThisMapID(),
                              srVect[i]->getOrigNodeID() ) );
      uint32 mapID = srVect[i]->getThisMapID();
      for ( uint j = 0; j < srVect[i]->size(); ++j ) {
         vect.push_back(IDPair_t(mapID, (*srVect[i])[j] ) );
         mc2dbg8 << "[UER]: Node [" << (vect.size() - 1)
                 << "] = " << vect.back() << endl;
      }
   }
   // Add last destination.
   SubRoute* lastSubRoute = srVect.back();
   vect.push_back(IDPair_t( lastSubRoute->getNextMapID(),
                            lastSubRoute->getDestNodeID() ) );
}

pair<int,int>
UnexpandedRoute::findFirstCommonNode( const vector<IDPair_t>& first,
                                      const vector<IDPair_t>& second,
                                      int firstStart,
                                      int secondStart )
{
   // FIXME: We should probably skip special time nodes here.
   for ( uint32 i = firstStart; i < first.size(); ++i ) {
      if(GET_ADDITIONAL_COST_STATE(first[i].getItemID())){
         continue;
      }
      for( uint32 j = secondStart; j < second.size(); ++j ) {
               // ignore addcost nodes.
         
         if(GET_ADDITIONAL_COST_STATE(second[j].getItemID())){
            continue;
         }

         if ( first[i] == second[j] ) {
            return make_pair(i,j);
         }
      }
   }
   return make_pair(-1,-1);
}

pair<int,int>
UnexpandedRoute::findFirstDifference( const vector<IDPair_t>& first,
                                      const vector<IDPair_t>& second,
                                      int firstStart,
                                      int secondStart )
{
   // Get indeces of first common node.
   pair<int, int> idx = findFirstCommonNode( first, second,
                                             firstStart, secondStart);
   
   if ( idx.first < 0 ) {
      mc2dbg4 << "[UER]: No common nodes at all." << endl;
      // How to describe the difference?
      return make_pair( 0, 0 );
   }
   
   mc2dbg << "[UER]: first common: " << idx.first << ":" << idx.second
          << endl;
   
   // Start at the first common node and find the first uncommon
   int firstSize = first.size();
   int secondSize = second.size();
   int i = idx.first;
   int j = idx.second;
   while ( ( i < firstSize ) && ( j < secondSize ) ) {
      // ignore addcost nodes.
      if(GET_ADDITIONAL_COST_STATE(first[i].getItemID())){
         i++;
      }
      
      if(GET_ADDITIONAL_COST_STATE(second[j].getItemID())){
         j++;
      }
      
      
      if ( first[i] != second[j] ) {
         return make_pair( i, j );
      }      
      ++i;
      ++j;
   }
   return make_pair(-1, -1);
}

int
UnexpandedRoute::findDisturbance(const vector<IDPair_t>& routeNodes,
                                 int startPos )
{
   const int routeSize = routeNodes.size();
   for ( int i = startPos ; i < routeSize; ++i ) {
      const uint32 curNode = routeNodes[i].getItemID();
      if ( GET_ADDITIONAL_COST_STATE(curNode) ) {
         mc2dbg4 << "[UER]: Found the first disturbance at "
                << i << endl;
         return i;
      }
   }
   return -1;
}

bool
UnexpandedRoute::compareToOriginal(const UnexpandedRoute& other)
{
   // Start by finding the first node that is the same.
   vector<IDPair_t>& thisRoute(m_ids);
   const vector<IDPair_t>& otherRoute(other.m_ids);

   // Find the idx of the first node that is different
   pair<int,int> firstDiff = findFirstDifference(thisRoute, otherRoute);

   mc2dbg8 << "[UER]: First difference at ("
           << firstDiff.first << "," << firstDiff.second << ")" << endl;
   
   if ( ( firstDiff.first < 0 ) || (firstDiff.second < 0 ) ) {
      // No difference found - you may return now.
      return false;
   }

   // Now we will try to find the place where there are disturbances
   int distIdx = findDisturbance(thisRoute);
   mc2dbg4 << "[UER]: One route goes from " << thisRoute[firstDiff.first-1]
          << " -> " << thisRoute[firstDiff.first] << " instead of "
          << otherRoute[firstDiff.second-1] << " -> "
          << otherRoute[firstDiff.second] << endl;
   mc2dbg4 << "[UER]: A disturbance on node "
          << thisRoute[distIdx+1] << endl;
   mc2dbg4 << "[UER]: Destinations are the same = "
          << (MapBits::nodeItemID(thisRoute.back().getItemID()) ==
              MapBits::nodeItemID(otherRoute.back().getItemID()))
          << endl;
   
   return true;
}

int
UnexpandedRoute::getDetourNodes(vector<DisturbanceDescription> &result,
                                UnexpandedRoute& ref)
{
   int thisIndex  = 0;
   int refIndex   = 0;
   int detAdded = 0;
  
      
   
   pair<int,int> searchPair;
   //uint32 detNode;
   //uint32 distNode;
   searchPair = findFirstCommonNode(ref.getIDPairs(), m_ids
                                    ,refIndex, thisIndex);
   refIndex  = searchPair.first;
   thisIndex = searchPair.second;
   
   if((thisIndex == -1) || (refIndex == -1))
      return detAdded;
   
   // started with detour?
   if(refIndex != 0){
      // find a disturbance that caused this.
      IDPair_t distNode = findDisturbedNode(ref.getIDPairs(), refIndex, 0);
      result.push_back( DisturbanceDescription(
                           m_ids[ 0 ],
                           distNode,
                           m_ids[ thisIndex ] ) );
      detAdded++;
      // Add join lm. at this point
   }
   
   
   bool inDetour     = false;
   int refSplitindex = -1;
   IDPair_t splitPoint  = IDPair_t(MAX_UINT32, MAX_UINT32);
   IDPair_t distPoint   = IDPair_t(MAX_UINT32, MAX_UINT32);
   
   do {
      // Search for split
      if((thisIndex != -1) && (refIndex != -1)){
         // Common node found searching for detour
         searchPair = findFirstDifference(ref.getIDPairs(), m_ids,
                                          refIndex, thisIndex);
         refIndex  = searchPair.first;
         thisIndex = searchPair.second;
         if((thisIndex != -1) && (refIndex != -1)){
            // Add lm detour node.
            refSplitindex = refIndex;
            splitPoint    = m_ids[thisIndex];
            mc2dbg << "[UER]: split point[" << thisIndex << "] = "
                   << m_ids[thisIndex] << endl;
            inDetour = true;
         }
         
      }
      
      if((thisIndex != -1) && (refIndex != -1)){
         // Split found searching for common node
         searchPair = findFirstCommonNode(ref.getIDPairs(), m_ids,
                                          refIndex, thisIndex);
         refIndex  = searchPair.first;
         thisIndex = searchPair.second;
         if((thisIndex != -1) && (refIndex != -1)){
            // Add lm detour node.
            // search for joining
            distPoint = findDisturbedNode(ref.getIDPairs(), refSplitindex,
                                         refIndex);
            
            mc2dbg << "[UER]: join point[" << thisIndex << "] = "
                   << m_ids[thisIndex] << endl;
            // Add 
            result.push_back( 
               DisturbanceDescription( splitPoint,
                                       distPoint,
                                       m_ids[ thisIndex ] ) );
            detAdded++;
            inDetour = false;
         }
         
      }
   } while ((thisIndex != -1) && (refIndex != -1));

   if(inDetour){
      // Ends in detour, search for the disturbance.
      distPoint = findDisturbedNode(ref.getIDPairs(), refSplitindex,
                                   ref.size()-1);
      result.push_back( DisturbanceDescription(
                           splitPoint,
                           distPoint,
                           IDPair_t(MAX_UINT32, MAX_UINT32) ) );
      detAdded++;
   }
   return detAdded;
}



int
UnexpandedRoute::getDisturbedNodes(vector<DisturbanceDescription> &result)
{
   
   int distAdded = 0;
   int startPos = 0;
   do{
      startPos = findDisturbance(m_ids, startPos);
      if(startPos != -1) {
         DisturbanceDescription desc(m_ids[(startPos++)-1]);
         result.push_back(desc);
         distAdded++;
      }
   } while( startPos != -1);
   return distAdded ;
}


vector<IDPair_t>
UnexpandedRoute::getIDPairs() {
   return m_ids;
}

IDPair_t
UnexpandedRoute::findDisturbedNode(const vector<IDPair_t>& routeNodes,
                                   int startIndex, int endIndex)
{
   int step = 1;
   if(startIndex > endIndex)
      step = -1;
   
   uint32 maxCost = 0;
   IDPair_t worstNode =  IDPair_t(MAX_UINT32, MAX_UINT32);
   
   while(startIndex != endIndex){
      const uint32 curNode = routeNodes[startIndex].getItemID();
      uint32 cost = GET_ADDITIONAL_COST_STATE(curNode);
      if ((startIndex != 0) &&  cost ) {
         if(cost > maxCost){
            worstNode = routeNodes[startIndex-1];
            maxCost = cost;
         }
         
      }
      startIndex += step;
   }
   return worstNode;
}

uint32
UnexpandedRoute::getSizeAsBytes() const {
   uint32 s = 0;
   // size
   s += 1*4;

   // m_ids
   s += size()*4*2;

   return s;
}

void
UnexpandedRoute::load( Packet* p, int& pos ) {

}


uint32
UnexpandedRoute::save( Packet* p, int& pos ) const {
   uint32 startOffset = p->getLength();

   p->incWriteLong( pos, size() );
   for ( uint32 i = 0 ; i < size() ; ++i ) {
      p->incWriteLong( pos, m_ids[ i ].getMapID() );
      p->incWriteLong( pos, m_ids[ i ].getItemID() );
   }

   p->setLength(pos); 
   return (p->getLength() - startOffset);
}
