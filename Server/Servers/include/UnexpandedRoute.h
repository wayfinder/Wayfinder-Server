/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UNEXPANDEDROUTE_H
#define UNEXPANDEDROUTE_H

#include "config.h"

#include <vector>
class SubRouteVector;
class DriverPref;
class DisturbanceDescription;
class Packet;

/**
 *    Class representing an unexpanded route.
 *    <br />
 *    Contains functions for comparing routes.
 */
class UnexpandedRoute {
public:
   /**
    *   Constructor.
    *   @param srVect     The route. Will be copied.
    *   @param driverPref The driver preferences. Will be copied.
    */   
   UnexpandedRoute(const SubRouteVector& srVect,
                   const DriverPref& driverPref );

   /**
    *   Destructor
    */
   virtual ~UnexpandedRoute();
   
   /**
    *   Compares the unexpanded route to the original.
    *   This route is assumed to be one with e.g. disturbances.
    *   Should probably return something later.
    *   @param disturbanceFree The disturbance free route.
    *   @return false if the routes are equal, true if they differ.
    */
   bool compareToOriginal(const UnexpandedRoute& disturbanceFree);
   

   /**
    *   Get a vector containing the nodes where the route splitups or
    *   rejoins and the node with the disturbance that caused the detour.
    *   The node that will be the disturbance node is one of the nodes of the
    *   most expensive disturbance.
    *   @param ref The route without disturbance avoidance.
    *   @return Vector with pairs of nodeIDs. The detour node and the
    *                  disturbed node. 
    */
   int getDetourNodes(vector<DisturbanceDescription> &result,
                      UnexpandedRoute& ref);
   
   /**
    *   Get a vector with the nodes with disturbances.
    *   @return  Vector with the nodes that have disturbances.
    */
   int getDisturbedNodes(vector<DisturbanceDescription> &result);
   
   vector<IDPair_t> getIDPairs();

   uint32 size() const
      { return m_ids.size(); }

   /**
    * Write this UnexpandedRoute as a byte buffer.
    *
    * @param p    The packet where the UnexpandedRoute will be saved.
    * @param pos  Position in p, will be updated when calling this 
    *             method.
    */
   uint32 save( Packet* p, int& pos ) const;

   /**
    * Get the size of this when stored in a byte buffer.
    *
    * @return  The number of bytes this will use when stored 
    *          in a byte buffer.
    */
   uint32 getSizeAsBytes() const;

   /**
    * Set all members with data from the buffer.
    *
    * @param p    The packet where the UnexpandedRoute will be loaded 
    *             from.
    * @param pos  Position in p, will be updated when calling this 
    *             method.
    */
   void load( Packet* p, int& pos );
   
private:

   /**
    *   Make a RouteReplyPacket, sortof.
    */
   static void putRouteInVector(vector<IDPair_t>& vect,
                                const SubRouteVector& srVect);

   /**
    *   Find the first node in <code>first</code> that also exists
    *   in <code>second</code>.
    *   @param first  First route to check.
    *   @param second Second route to check.
    *   @return A pair with index to first in first and index in
    *           second in second or -1,-1 if not found.
    */
   static pair<int,int> findFirstCommonNode( const vector<IDPair_t>& first,
                                             const vector<IDPair_t>& second,
                                             int firstStart = 0,
                                             int secondStart = 0);

   /**
    *   Starts by calling findFirstCommonNode. If there is no common node
    *   it will return 0,0. If there is a common node, the function
    *   continues comparing nodes from that point on. The index of the
    *   first node that differs will be returned, or -1,-1 if equal.
    */
   static pair<int,int> findFirstDifference( const vector<IDPair_t>& first,
                                             const vector<IDPair_t>& second,
                                             int firstStart = 0,
                                             int secondStart = 0);

   /**
    *   Looks for disturbances from the <code>startPos</code> and forward.
    *   @return Found position or -1 if no disturbance found.
    */
   static int findDisturbance( const vector<IDPair_t>& routeNodes,
                               int startPos = 0);

   /**
    *   Method for finding a disturbance on a detour.
    *   Searches for the most costly (time) disturbance on a route between
    *   two indexes. If the start index is higer than the end index, the
    *   search will be backwards.
    *   @return ID of the most disturbed node. MAX_UINT32 if no disturbance
    *           was found.
    */
   
   static IDPair_t findDisturbedNode(const vector<IDPair_t>& route,
                                     int startIndex, int endIndex);
   
   /**
    *   Vector containing the route ids.
    */
   vector<IDPair_t> m_ids;

   /**
    *   Driver preferences.
    */
   DriverPref* m_driverPref;
   
};

#endif
