/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <algorithm>
#include <set>

#include "ServerSubRouteVector.h"
#include "SubRoute.h"
#include "OverviewMap.h"
#include "IDPairVector.h"

// Temporary function for testing

void
ServerSubRouteVector::getSubRoutesToMap(uint32 mapID,
                                       map<uint32, SubRoute*>& subRoutes )
{
   // Loop through all the SubRoutes. Later we should replace this
   // linear search with a multimap or something.

   // FIXME: This function will not always return the cheapest route.
   
   const_iterator it = begin();
   
   while ( it != end() ) {
      // Get the subroute in the list for convenience.
      SubRoute* listSubRoute = *it;
      
      if ( listSubRoute->getNextMapID() == mapID ) {
         map<uint32, SubRoute*>::const_iterator find_it =
            subRoutes.find(listSubRoute->getDestNodeID());
         if ( find_it == subRoutes.end() ) {
            // Not inserted yet 
            subRoutes.insert(make_pair(listSubRoute->getDestNodeID(),
                                       listSubRoute));
         } else {
            // Already inserted one route with that ID.
            SubRoute* foundRoute = find_it->second;
            if ( foundRoute->getCost() >= listSubRoute->getCost() ) {
               // Overwrite the old one.
               subRoutes[listSubRoute->getDestNodeID()] = listSubRoute;
            } else {
               mc2dbg8 << "SSRV: Cheaper route already inserted" << endl;
            }
         }
      }
      ++it;
   }
}

void
ServerSubRouteVector::initDests(uint32 nbrDest)
{
   if ( nbrDest == 0 ) {
      m_destIndexArray = NULL;
   } else {
      // Create array with one extra MAX_UINT32
      m_destIndexArray = new uint32[nbrDest+1];
      uint32 index;
      for ( index = 0; index < nbrDest+1; ++index ) {
         m_destIndexArray[index] = MAX_UINT32;
      }
   }
   m_nbrDest = nbrDest;
   m_maxNbrDest = nbrDest;
}

ServerSubRouteVector::ServerSubRouteVector(uint32 nbrDest) :
      SubRouteVector()
{
   initDests(nbrDest);
   m_prevSubRouteVectorIndex = MAX_UINT32;
   m_externalCutOff = MAX_UINT32;
}

ServerSubRouteVector::~ServerSubRouteVector()
{
   // Set the SubRoutes that are not owned by this
   // to NULL so that SubRouteVector won't delete them
   for(uint32 i=0; i < (*this).size(); ++i ) {
      if ( m_ownedByMe[i] == false ) {
         (*this)[i] = NULL;
         mc2dbg8 << "SSRV: Number " << i << " is not owned by me " << endl;
      }
   }
   // Deleting the m_destIndexArray.
   delete [] m_destIndexArray;
}

void
ServerSubRouteVector::insertDestSubRoute(uint32 index)
{
   uint32 indexCounter = 0;

   // If we have more than one destination, we have to check
   if ( m_nbrDest > 1) {

      // Look for a free position in the vector or
      // a position that has a SubRoute to the same
      // map and node.
      // The last position in the array should not be used.
      while (m_destIndexArray[indexCounter] != MAX_UINT32 &&
             ((*this)[index]->getNextMapID() !=
              (*this)[m_destIndexArray[indexCounter]]->getNextMapID() ||
              (*this)[index]->getDestNodeID() !=
              (*this)[m_destIndexArray[indexCounter]]->getDestNodeID() ) ) {
         
         indexCounter++;
      }

      // Since the number of wanted destinations can differ from
      // the number of real destinations (not only in the case of
      // one wanted destination) we must look for the most expensive
      // one if we didn't find a free spot or a route to the same place
      if ( indexCounter >= m_maxNbrDest ) {
         mc2dbg << "Destination array full - looking for the most "
                   "expensive one" << endl;
         int maxIndex = -1; 
         uint32 maxCost = 0;
         for(int i=0; i < (int)m_nbrDest; ++i ) {
            uint32 curCost = (*this)[m_destIndexArray[i]]->getCost();
            if (  curCost > maxCost){
               maxIndex = i;
               maxCost = curCost;
            }
         }
         MC2_ASSERT( maxIndex >= 0 );
         indexCounter = maxIndex;
      } else {
         // The number of destinations may have been decreased earlier
         // Increase the number of destinations if MAX_UINT32 was found
         // inside the vector.
         m_nbrDest = MAX(m_nbrDest, indexCounter + 1);
      }
   }

   if ( m_destIndexArray[indexCounter] == MAX_UINT32 ) {
      mc2dbg4 << "Setting a new destination! pos = " << indexCounter
              <<  "***" << endl;
      mc2dbg4 << "Cost is " << (*this)[index]->getCost() << endl;
      m_destIndexArray[indexCounter] = index;
      
   } else if ( (*this)[m_destIndexArray[indexCounter]]->getCost() >
               (*this)[index]->getCost() ) {
      mc2dbg4 << "Changing a destination! pos = " << indexCounter << "***"
              << endl;
      mc2dbg4 << "Cost is " << (*this)[index]->getCost() << endl;
      m_destIndexArray[indexCounter] = index;
   }
}

void
ServerSubRouteVector::insertSubRoute(SubRoute* pSubRoute,
                                     bool isDest)
{
   if ( pSubRoute->getSubRouteID() != MAX_UINT32 ) {
      mc2dbg8 << "[SSRV]: Route with id " << pSubRoute->getSubRouteID()
             << " inserted " << endl;
   }
   pSubRoute->setSubRouteID( size() );
   SubRouteVector::insertSubRoute(pSubRoute);
   // The SubRoute should be deleted.
   m_ownedByMe.push_back(true);
   
   if ( isDest ) {
      insertDestSubRoute( size() - 1);
   }
}


uint32 
ServerSubRouteVector::getCutOff() const
{
   
   if ( m_destIndexArray[m_nbrDest - 1] == MAX_UINT32 ) {      
      // BEGIN Cheating
      int nbr = 0;
      for( uint32 i=0; i < m_nbrDest; ++i ) {
         if ( m_destIndexArray[i] == MAX_UINT32 ) {
            nbr = i; 
            break;
         }
      }
     // Fake cutoff if half of the dests are found.
      if ( m_nbrDest > 20 && (nbr > ((int)m_nbrDest / 2))  ) {
         mc2dbg << "[SSRV]: Cheating - only " << nbr
                << " of " << m_nbrDest << " found"
                << endl;
         //int tempNbr = m_nbrDest;
         ((ServerSubRouteVector*)(this))->m_nbrDest = nbr;
         uint32 retVal = getCutOff();
         // Set the number of dests back. Probably not good.
         //((ServerSubRouteVector*)(this))->m_nbrDest = tempNbr;
         return retVal;
      } else {
         mc2dbg8 << "[SRVV]: Not cheating "
                 << nbr << " of " << m_nbrDest << " found" << endl;
      }
      // END Cheating
      
      // m_destIndexArray is not complete yet      
      mc2dbg8 << "[SSRV]: External cutoff used. "
              << nbr << " of " << m_nbrDest << " dests found" << endl;
      return m_externalCutOff;
   } else {
      // Must check all SubRoutes in the m_destIndexArray.
      uint32 cutOff = 0;
      uint32 arrayCounter = 0;
      uint32 expensiveIdx = 0;
      
      while ( arrayCounter < m_nbrDest ) {
         // Get the cost from the SubRoute pointed to by the array.
         uint32 curCost = (*this)[m_destIndexArray[arrayCounter]]->getCost();
         if ( curCost > cutOff ) {          
            cutOff = curCost;
            expensiveIdx = arrayCounter;
         }
         arrayCounter++;         
      }

      if ( cutOff < m_externalCutOff ) {
         mc2dbg8 << "[SSRV] : Most expensive route is "
                 << (*this)[m_destIndexArray[expensiveIdx]]->getNextMapID()
                 << ":" << hex
                 << (*this)[m_destIndexArray[expensiveIdx]]->getDestNodeID()
                 << dec << endl;
         return cutOff;
      } else {
         return m_externalCutOff;
      }
   }         
}

void
ServerSubRouteVector::resetIndex(uint32 index)
{
   m_ownedByMe[index] = false;
}

void
ServerSubRouteVector::resetAll( )
{

   for (uint32 index = 0; index < (*this).size(); ++index ) {
      // Just to be safe
      ServerSubRouteVector::resetIndex(index);
   }
}

// FIXME: Avoid copies if possible.
ServerSubRouteVector*
ServerSubRouteVector::getResultVector(uint32 index)
{
   uint32 currentSubRouteIndex = getDestIndexArray()[index];
   
   ServerSubRouteVector* returnVector = new ServerSubRouteVector;
   if ( currentSubRouteIndex != MAX_UINT32 ) {

      SubRoute* currentSubRoute = (*this)[currentSubRouteIndex];      
      
      while ( currentSubRoute->getThisMapID() != MAX_UINT32 ) {
         // FIXME: We don't have to copy all the time
         currentSubRoute = new SubRoute(*currentSubRoute);

         // Reset index is not used now.
         // uint32 resetIdx = currentSubRouteIndex;
         // SubRouteID is destroyed here. Is it OK if we don't copy?
         returnVector->insertSubRoute( currentSubRoute );
         currentSubRouteIndex = currentSubRoute->getPrevSubRouteID();
         mc2dbg8 << "[SSRV]: MapID is " << currentSubRoute->getThisMapID()
                 << endl;
         mc2dbg8 << "[SSRV]: Previndex is " << currentSubRouteIndex << endl;
         //ServerSubRouteVector::resetIndex( resetIdx );
         currentSubRoute = (*this)[currentSubRouteIndex];
      }
      reverse( returnVector->begin(), returnVector->end() );
      returnVector->front()->setPrevSubRouteID(
         currentSubRoute->getPrevSubRouteID() );
      returnVector->front()->setOrigOffset( currentSubRoute->getDestOffset() );
      mc2dbg8 << "SRVV costs (back)"
              <<  returnVector->back()->getCostASum() << ':'
              <<  returnVector->back()->getCostBSum() << ':'
              <<  returnVector->back()->getCostCSum() << endl;
      mc2dbg8 << "SRVV costs (front)"
              <<  returnVector->front()->getCostASum() << ':'
              <<  returnVector->front()->getCostBSum() << ':'
              <<  returnVector->front()->getCostCSum() << endl;
   } else {
      mc2dbg8 << "Empty vector will be returned " << endl;
      mc2dbg8 << "Size of return vector is now "
              << returnVector->size() << endl;
   }
   return returnVector;
}

const SubRoute*
ServerSubRouteVector::containsCheaperThan(const SubRoute* subRoute) const
{
   // Loop through all the SubRoutes. Later we should replace this
   // linear search with a multimap or something.
   const_iterator it = begin();
   
   while ( it != end() ) {
      const SubRoute* listSubRoute = *it;
      // Start by comparing id:s. It should be equal more seldomly than
      // map id.
      if ( listSubRoute->getDestNodeID() == subRoute->getDestNodeID() &&
           listSubRoute->getNextMapID() == subRoute->getNextMapID() &&
           listSubRoute->getCost() <= subRoute->getCost() ) {
         // This one was the same, but cheaper.
         return listSubRoute;
      }
      ++it;
   }
   // No cheaper SubRoute found.
   return NULL;
}

int
ServerSubRouteVector::countNodesOnLevel(int level) const
{
   set<IDPair_t> alreadyUsed;
   int nbr = 0;
   for ( const_iterator it = begin();
         it != end();
         ++it ) {
      IDPair_t id((*it)->getNextMapID(), (*it)->getDestNodeID());
      if ( alreadyUsed.find(id) == alreadyUsed.end()) { 
         if ( OverviewMap::existsOnLevel(level, (*it)->getDestNodeID())) {
            ++nbr;
         }
         alreadyUsed.insert(id);
      }
   }
   return nbr;
}

SubRoute*
ServerSubRouteVector::merge( ServerSubRouteVector* sourceVector )
{
   if ( sourceVector == NULL || sourceVector->getSize() == 0 ) {
      return NULL;
   }
   
   ServerSubRouteVector::iterator it = sourceVector->begin();

   int nbrMerged = 0;
   // The first one should not be used, methinks.
   while ( (*it) != sourceVector->back() ) {
      insertSubRoute( *it );
      uint32 prevIndex = (*it)->getSubRouteID();
      // Set the next to prev.
      (*(++it))->setPrevSubRouteID( prevIndex );
      ++nbrMerged;
   }
   mc2dbg8 << "[SSRV]: Merging added " << nbrMerged << " vectors" << endl;
   mc2dbg8 << "[SSRV]: Source vector size was " << sourceVector->getSize()
           << endl;
   return (*it);
}


ServerSubRouteVectorVector::~ServerSubRouteVectorVector()
{
   iterator it = begin();
   while ( it != end() ) {
      delete *it;
      it = erase( it );
   }
}
