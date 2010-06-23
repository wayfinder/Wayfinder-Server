/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SubRouteList.h"
#include "OrigDestNodes.h"
#include "DebugRM.h"
#include "RouteMapNotice.h"

bool
SubRouteList::printVersion()
{
   mc2log << info
          << endl;
   return true;
}

bool
SubRouteList::versionPrinted = SubRouteList::printVersion();


SubRouteList::SubRouteList()
{
   mc2dbg4 << "SubRouteList::SubRouteList" << endl;
   m_subRouteVect.reserve( 800 );
   m_routeID = MAX_UINT32;
   m_cutOff = MAX_UINT32;
   m_listType = SubRouteListTypes::LOWER_LEVEL;
}


SubRouteList::~SubRouteList()
{
} 

void
SubRouteList::deleteAllSubRoutes()
{
   STLUtility::deleteValues( m_subRouteVect );
}


void
SubRouteList::copyListHeader(const SubRouteList& subRouteList)
{
   m_routeID  = subRouteList.getRouteID();
   m_cutOff   = subRouteList.getCutOff();
   m_listType = subRouteList.getListType();
}


inline uint32
SubRouteList::findSubRouteWithSameExternals(const RMSubRoute& subRoute) const
{
   // NB! Searching backwards here. Used to be forwards
   // The reason for changing this was that I think that the least
   // expensive routes should be last since addSubRoute function
   // only add routes if they are less expensive.
   for (int i = m_subRouteVect.size() - 1; i >= 0; --i) {
      RMSubRoute* sRoute = m_subRouteVect[ i ];
      
      if (sRoute->externalEquals(subRoute)) {
         return i;
      }
   }
   return MAX_UINT32;
}


uint32
SubRouteList::addSubRoute(RMSubRoute* subRoute)
{
   mc2dbg4 << "SubRouteList::addSubRoute" << endl;
   
   uint32 index = findSubRouteWithSameExternals(*subRoute);

   if ( subRoute->m_extCost[0] == MAX_UINT32 )
      mc2dbg2 << "\007MAX_UINT32 in addSubRoute" << endl;
   
   // Item wasn't found
   if (index == MAX_UINT32) {
      m_subRouteVect.push_back(subRoute);
      return m_subRouteVect.size() - 1;
   } else {
      mc2dbg8 << "Adding a SubRoute with same externals as a stored "
              << "one, index " << index << endl;
      // Remove l8er
      // Item found check cost to se if we should update.
      RMSubRoute* sRoute = m_subRouteVect[ index ];
      
      // All the external costs should be the same since we are in the
      // same place for all connections.
      if (sRoute->m_extEstimatedCost[0] >
           subRoute->m_extEstimatedCost[0]) { // &&              // NEW
//          (sRoute->isForward() == subRoute->isForward())) { // NEW
         
#if 1
         //mc2dbg4<< "Deleting subRoute at " << index << endl;
         RMSubRoute* routeToRemove = m_subRouteVect[ index ];
         m_subRouteVect[ index ] = subRoute;
         delete routeToRemove;
         return index;
#else
         m_subRouteVect.push_back(subRoute);
         uint32 newIndex = m_subRouteVect.size() - 1;
         mc2dbg8 << ", index of new subRoute " << newIndex << endl;
         return newIndex;
#endif
      } else {
         // Functions should delete SubRoute themselves if this function
         // returns MAX_UINT32         
         return MAX_UINT32;
      }
   }
   mc2log << info << "SubRouteList::addSubRoute   This will never be seen!!"
          << endl;
} // addSubRoute

bool
SubRouteList::operator == (const VectorElement& elm) const
{
   return ((SubRouteList*)&elm)->m_routeID == m_routeID;
}


bool
SubRouteList::operator != (const VectorElement& elm) const
{
   return ((SubRouteList*)&elm)->m_routeID != m_routeID;
}


bool
SubRouteList::operator > (const VectorElement& elm) const
{
   return m_routeID > ((SubRouteList*)&elm)->m_routeID;
}


bool
SubRouteList::operator < (const VectorElement& elm) const
{
   return m_routeID < ((SubRouteList*)&elm)->m_routeID;
}


bool
SubRouteList::exists(RMSubRoute* subRoute)
{
   RMSubRoute* sRoute;
   bool bExists = false;
   
   for (uint32 i = 0; i < m_subRouteVect.size(); i++) {
      sRoute = m_subRouteVect[ i ];
      if (sRoute->getNbrConnections() == subRoute->getNbrConnections()) {

         //Scan the externals
         bExists = false;
         for (uint32 j = 0; j < sRoute->getNbrConnections(); j++) {
            if( ( sRoute->m_extNodeID[j] == subRoute->m_extNodeID[j] ) &&
                ( sRoute->m_extMapID[j] == subRoute->m_extMapID[j] ) ){
               if( sRoute->m_extEstimatedCost[j] > 
                   subRoute->m_extEstimatedCost[j] ) {
                  bExists = true;
                  break;
               }
            }
         }
         if (bExists) {
            break;
         }
      }
   }
   return bExists;
} 

RMSubRoute*
SubRouteList::findExternalNodeSubRoute(uint32 externalID)
{
   for (uint32 i = 0; i < m_subRouteVect.size(); i++) {
      RMSubRoute* subRoute = m_subRouteVect[ i ];
      for (uint32 j = 0; j < subRoute->getNbrConnections(); j++) {
         uint32 mapID, nodeID, cost, estimatedCost;
         subRoute->getExternal(j, mapID, nodeID, cost, estimatedCost);
         if (nodeID == externalID) {
            return subRoute;
         }
      }
   }
   return NULL;
}

uint32
SubRouteList::findExternalNode(uint32 externalID)
{
   RMSubRoute* subRoute = findExternalNodeSubRoute(externalID);
   if ( subRoute == NULL )
      return MAX_UINT32;
   else
      return subRoute->getSubRouteID();
}



void
SubRouteList::dump()
{
   mc2log << info << "RouteID:  " << m_routeID  << endl
          << "Listtype: " << m_listType << endl
          << "Cut off:  " << m_cutOff   << endl
          << "Number of stored subroutes: " << m_subRouteVect.size()
          << endl;
   
   for( uint32 i = 0; i < m_subRouteVect.size(); i++ ) {
      mc2log << "========== RMSubRoute " << i << " ==========" << endl;
      if ( m_subRouteVect[ i ] != NULL ) {
         m_subRouteVect[ i ]->dump(false);
      } else {
         mc2log << "NULL" << endl;
      }
   }

   mc2log << "===============================" << endl;
}

