/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef RMTYPES_H_
#define RMTYPES_H_

#include "config.h"
#include <utility>

/**
 *   Contains some shared RouteModule types.
 */

/// Type of cost used when routing
typedef uint32 nodeCost_t;

/// Unvisited nodes should have this cost.
static const nodeCost_t INVALID_COST = MAX_UINT32;

/**
 *    Represents an expanded node and it's index in
 *    the m_nbrMultiConnections table.
 *    @see RoutingMap
 *    @see DisturbanceStorage
 */
class nodeIndex_t : public pair<uint32, uint32> {
public:
   /**
    *    Less than operator. Needed for binary search.
    *    Should be replaced by functor, because now
    *    operator < and operator> aren't opposits.
    */
   bool operator < ( const nodeIndex_t& otherNodeIndex ) const {
      return first < otherNodeIndex.first;
   }
}; 
   
/**
 *    Represents from and to node of a multi connection.
 *    @see RoutingMap
 *    @see DisturbanceStorage
 */
class fromToNode_t : public pair<uint32, uint32> {
public:
   fromToNode_t(uint32 from, uint32 to) : pair<uint32, uint32>(from,to){}
   fromToNode_t() : pair<uint32, uint32>(MAX_UINT32,MAX_UINT32) {}
   uint32 getStartNode() const { return first; }
   uint32 getEndNode() const { return second; }
   /// Prints the pair on a stream.
   inline friend ostream& operator<<( ostream& stream,
                                      const fromToNode_t& nodes);
};

inline ostream&
operator<<( ostream& stream, const fromToNode_t& nodes )
{
   return stream << "0x" << hex << nodes.first << "->0x" << nodes.second
                 << dec;
}

#endif
