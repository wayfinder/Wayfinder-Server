/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RMSubRoute.h"

// =======================================================================
//                                                            RMSubRoute =
// =======================================================================

RMSubRoute::RMSubRoute() : m_extMapID( 1, 1 ),
                           m_extNodeID( 1, 1 ),
                           m_extCost( 1, 1 ),
                           m_extEstimatedCost( 1, 1 ),
                           m_lats( 1, 1 ),
                           m_lons( 1, 1 )
                           
{
   m_subRouteID = MAX_UINT32;
   m_prevSubRouteID = MAX_UINT32;
   m_sucSubRouteID = MAX_UINT32;
   m_visited = false;
   m_routeComplete = false;
   m_forwardRoute = true;
   m_nbrMapsVisited = 0;
}

RMSubRoute::RMSubRoute( RMSubRoute* subRoute, bool copyVisitedAndComplete )
      : m_extMapID( subRoute->getNbrConnections(), 1 ),
        m_extNodeID( subRoute->getNbrConnections(), 1 ),
        m_extCost( subRoute->getNbrConnections(), 1 ),
        m_extEstimatedCost( subRoute->getNbrConnections(), 1 ),
        m_lats( subRoute->getNbrConnections(), 1 ),
        m_lons( subRoute->getNbrConnections(), 1 ),
        m_costsA( subRoute->getNbrConnections(), 1),
        m_costsB( subRoute->getNbrConnections(), 1),
        m_costsC( subRoute->getNbrConnections(), 1)
        
{   
   m_subRouteID = subRoute->getSubRouteID();
   m_prevSubRouteID = subRoute->getPrevSubRouteID();
   m_sucSubRouteID = subRoute->getSucSubRouteID();
   m_forwardRoute = subRoute->isForward();
   
   for( uint32 i = 0; i < subRoute->getNbrConnections(); i++ ) {
      m_extMapID.addLast( subRoute->m_extMapID[i] );
      m_extNodeID.addLast( subRoute->m_extNodeID[i] );
      m_extCost.addLast( subRoute->m_extCost[i] );
      m_extEstimatedCost.addLast( subRoute->m_extEstimatedCost[i] );
      m_lats.addLast( subRoute->m_lats[i] );
      m_lons.addLast( subRoute->m_lons[i] );
      m_costsA.addLast( subRoute->m_costsA[i] );
      m_costsB.addLast( subRoute->m_costsB[i] );
      m_costsC.addLast( subRoute->m_costsC[i] );
      
   }
   if ( ! copyVisitedAndComplete ) {
      m_visited        = false;
      m_routeComplete  = false;
      m_nbrMapsVisited = 0;
   } else {     
      m_visited        = subRoute->m_visited;
      m_routeComplete  = subRoute->m_routeComplete;
      m_nbrMapsVisited = subRoute->m_nbrMapsVisited;
      m_endToNode0     = subRoute->m_endToNode0;
      m_startToNode0   = subRoute->m_startToNode0;
      m_startOffset    = subRoute->m_startOffset;
      m_endOffset      = subRoute->m_endOffset;
   }
}


void RMSubRoute::addExternal( uint32 mapID,
                              uint32 nodeID,
                              uint32 cost,
                              uint32 estimatedCost,
                              int32 lat,
                              int32 lon,
                              uint32 costAsum,
                              uint32 costBsum,
                              uint32 costCsum)
{
   m_extMapID.addLast(mapID);
   m_extNodeID.addLast(nodeID);
   m_extCost.addLast(cost);
   m_extEstimatedCost.addLast(estimatedCost);
   m_lats.addLast(lat);
   m_lons.addLast(lon);
   m_costsA.addLast(costAsum);
   m_costsB.addLast(costBsum);
   m_costsC.addLast(costCsum);
}


void RMSubRoute::setCost( uint32 index,
                        uint32 cost,
                        uint32 estimatedCost )
{
   if( index < m_extCost.getSize()){
      m_extCost[index] = cost;
      m_extEstimatedCost[index] = estimatedCost;
   }
}


bool 
RMSubRoute::findExternal(uint32 mapID, uint32 nodeID) const
{
   for (uint32 i = 0; i < getNbrConnections(); i++) {
      if ((mapID == m_extMapID[i]) && (nodeID == m_extNodeID[i])) {
          return true;
      }
   }
   return false;
}


bool
RMSubRoute::externalEquals(const RMSubRoute& subRoute) const
{
   if (subRoute.getNbrConnections() != getNbrConnections()) {
      return false;
   }

   // Now scan all the external connections and see if all are found.
   for (uint32 i = 0; i < getNbrConnections(); i++) {
      if (!findExternal(subRoute.m_extMapID[i], subRoute.m_extNodeID[i])) {
         return false;
      }
   }

   return true;
}

bool RMSubRoute::checkExternalMapID( uint32 mapID )
{
   for( uint32 i=0;i < m_extMapID.getSize(); i++){
      if( m_extMapID[i] == mapID )
         return true;
   }
   return false;
}


void RMSubRoute::dump( bool simple )
{
   cout << "m_extMapID         = ";
   m_extMapID.dump(true);
   cout << "m_extNodeID (hex)  = " << hex;
   m_extNodeID.dump(true);
   cout << "m_extNodeID (dec)  = " << dec;
   m_extNodeID.dump(true);
   cout << "m_extCost          = ";
   m_extCost.dump(true);
   cout << "m_extEstimatedCost = ";
   m_extEstimatedCost.dump(true);
   cout << "m_subRouteID       = " << m_subRouteID << endl;
   cout << "m_prevSubRouteID   = " << m_prevSubRouteID << endl;
   cout << "m_sucSubRouteID    = " << m_sucSubRouteID <<  endl;
   if (m_visited) {
      cout << "m_visited          = true" << endl;
   }
   else {
      cout << "m_visited          = false" << endl;
   }

   cout << "m_nbrMapsVisited   = " << m_nbrMapsVisited << endl;
   
   if (m_routeComplete) {
      cout << "m_routeComplete    = true" << endl;
   }
   else {
      cout << "m_routeComplete    = false" << endl;
   }
/*
   if (m_forwardRoute) {
      cout << "m_forwardRoute     = true" << endl;
   }
   else {
      cout << "m_forwardRoute     = false" << endl;
   }
*/
   if (m_forwardRoute) {
      cout << "Route direction    = FORWARD" << endl;
   }
   else {
      cout << "Route direction    = BACKWARD" << endl;
   }
} // dump


// =======================================================================
//                                                    CompleteRMSubRoute =
// =======================================================================


CompleteRMSubRoute::CompleteRMSubRoute()
    : m_nodeIDs(1, 65536)
{
   setMapID( MAX_UINT32 );
}


CompleteRMSubRoute::CompleteRMSubRoute( uint32 mapID )
      : m_nodeIDs(1, 65536)
{
   setMapID( mapID );
}


CompleteRMSubRoute::CompleteRMSubRoute( RMSubRoute* subRoute, bool completeCopy )
      : RMSubRoute( subRoute, completeCopy )
{
}


CompleteRMSubRoute::CompleteRMSubRoute( CompleteRMSubRoute* completeSubRoute,
                                    bool completeCopy )
      : RMSubRoute( completeSubRoute, completeCopy ),
        m_nodeIDs(completeSubRoute->getNbrNodesInRoute(), 10)
{
   for( uint32 i = 0; i < completeSubRoute->getNbrNodesInRoute(); i++ ) {
      m_nodeIDs.addLast( completeSubRoute->m_nodeIDs[i] );
   }

   setMapID( completeSubRoute->m_mapID );
}

/*
void CompleteSubRoute::makeSubRouteFromRoute(RoutingMap* map, Head* result) {
   RoutingNode* nodeInRoute = (RoutingNode*)(result->last());
   uint32 gradient = nodeInRoute->getGradient();

   while (gradient != MAX_UINT32) {
      this->addEndNode(nodeInRoute->getItemID());
      nodeInRoute = map->getNode(gradient);
      gradient = nodeInRoute->getGradient();
   }
   m_routeComplete = true;
}
*/

void CompleteRMSubRoute::dump(bool simple)
{
   cout << "m_mapID            = " << m_mapID << endl;
   
   if( !simple ) {
       /*
      cout << "Nbr = " <<  m_nodeIDs.getSize() << endl;
      int32 j = 0;
      int32 stop = int32( m_nodeIDs.getSize() ) - 4;
     
      
      if( stop <= 0 )
         m_nodeIDs.dump(true);
      else { 
         while( j < stop ) {
            cout << hex << m_nodeIDs[j  ] << ", "
                 << m_nodeIDs[j+1] << ", "
                 << m_nodeIDs[j+2] << ", "
                 << m_nodeIDs[j+3] << endl;
            j += 4;
         }
         
         for( uint32 i = j; i < m_nodeIDs.getSize()-1; i++ )
            cout << m_nodeIDs[i] << ", ";
         cout << m_nodeIDs[ m_nodeIDs.getSize()-1 ] << dec << endl;
      }
      */
   }
   
   RMSubRoute::dump();
}

// =======================================================================
//                                           CompleteProximityRMSubRoute =
// =======================================================================

CompleteProximityRMSubRoute::CompleteProximityRMSubRoute() :
      CompleteRMSubRoute()
{

}


CompleteProximityRMSubRoute::CompleteProximityRMSubRoute(
   CompleteRMSubRoute* completeRoute,
   bool completeCopy)
      : CompleteRMSubRoute(completeRoute,
                         completeCopy)
{

}


CompleteProximityRMSubRoute::~CompleteProximityRMSubRoute()
{

}


