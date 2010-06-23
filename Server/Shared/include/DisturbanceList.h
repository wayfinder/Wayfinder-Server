/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DISTURBANCELIST_4711_H
#define DISTURBANCELIST_4711_H

#include<vector>
#include<map>

#include "config.h"
#include "IDPairVector.h"

class DisturbanceList;
class DisturbanceListElement;


/**
 *    Vector of disturbance-pointers.
 */
class DisturbanceVector : public vector<DisturbanceListElement*> {
};

/**
 *   Objects of this class describes temporary disturbances
 *   to be sent to the RouteModule together with the route.
 *
 *   Used to inherits from OrigDestInfo for easier translation from
 *   coordinates when OrigDestLookup is done. Not used. Inherits
 *   from IDPair_t instead now.
 */
class DisturbanceListElement : public IDPair_t {
public:

   /**
    *   Creates a DisturbanceListElement with the supplied
    *   id and extra cost in seconds.
    *   @param id The node id to add the disturbance for.
    *   @param extraCostSec Extra cost in seconds.
    */
   DisturbanceListElement( const IDPair_t& id,
                           uint32 extraCostSec );

   /**
    *   Return true if the node should be avoided.
    */
   bool shouldAvoid() const;

   /**
    *   Return the factor to punish with. Only valid if the
    *   above shouldAvoid() returns false.
    */
   float getFactor() const;

   /**
    *   Return the factor as is. Only for use by SubRoutePacket
    *   and RouteTrafficCostPacket.
    */
   uint32 getRawFactor() const;

   /**
    *   Returns true if the factor should be used. False if the
    *   time should be used.
    */
   bool getUseFactor() const;
   
   /**
    *   Returns the time to be added to the node if getUseFactor
    *   returns false.
    */
   uint32 getTime() const;

   /**
    *   Returns the mapid of the dist.
    */
   uint32 getMapID() const;

   /**
    *   Returns the node id of the dist.
    */
   uint32 getNodeID() const;

   /**
    *   Returns the raw factor from the floating point factor.
    */
   inline static uint32 floatToRaw( float factor );

   /**
    *   Converts an InfoModule factor to the raw factor needed here.
    */
   inline static uint32 infoModuleFactorToRaw( uint32 infoModuleFactor );
   
private:

   /// To avoid that any other uses the factor method.
   friend class DisturbanceList;
   friend class RMSubRouteRequestPacket;
   friend class RouteTrafficCostReplyPacket;

   /**
    *   Creates a disturbance element which means that
    *   the cost of the connections from the supplied
    *   node should be multiplied by the supplied factor.
    *   @param mapID  The mapID of the node.
    *   @param nodeID The nodeID. Highest bit is important.
    *   @param penaltyFactor Multiplicative factor for the costs.
    */
   DisturbanceListElement(uint32 mapID,
                          uint32 nodeID, 
                          float penaltyFactor);

   /**
    *   Creates a disturbance element with the supplied mapID
    *   and nodeID and rawFactor. To be used by SubRoutePacket.
    *   @param mapID  The mapID of the disturbance.
    *   @param nodeID The nodeID of the disturbance.
    *   @param rawFactor The raw factor of the disturbance.
    *   @param dummy     Dummy.
    */
   DisturbanceListElement(uint32 mapID,
                          uint32 nodeID,
                          uint32 rawFactor,
                          bool useFactor,
                          uint32 timeSec);
   
   /**
    *   Creates a disturbance element which means that
    *   the supplied node should be avoided.
    *   @param mapID  The mapID of the node.
    *   @param nodeID The nodeID. Highest bit is important.
    */
   DisturbanceListElement(uint32 mapID,
                          uint32 nodeID);

   
   /**
    *   Factor to multiply existing costs in the RM with.
    *   MAX_UINT32 is a special value which means that the
    *   node should be avoided.
    */
   uint32 m_factor;

   /**
    *   True if the factor should be used. False if the
    *   time should be used.
    */
   bool m_useFactor;

   /**
    *   Time in seconds to be added to the disturbed node.
    */
   uint32 m_time;
   
};

/**
 *   List containing (temporary) disturbances to send to
 *   the RouteModule.
 */
class DisturbanceList {

public:

   /**
    *    Creates a new empty DisturbanceList.
    */
   DisturbanceList();

   /**
    *    Copy constructor.
    */
   DisturbanceList(const DisturbanceList& orig);
   
   /**
    *    Deletes the DisturbanceList and all the
    *    elements in it.
    */
   ~DisturbanceList();

   /**
    *    Takes the disturbances from the vector <code>vect</code>
    *    and adds them into the list.
    *    @param vect  Vector to add disturbances from. Will be cleared.
    *    @param mapID If set, all the disturbances are assumed to be from
    *                 that mapID. If MAX_UINT32 the mapids of the
    *                 disturbances will be checked.
    */
   void takeDisturbances( DisturbanceVector& vect,
                          uint32 mapID = MAX_UINT32 );

   
   /**
    *    Adds a disturbance to the list. The DisturbanceListElement
    *    will be deleted when the list is deleted.
    */
   void addDisturbance(DisturbanceListElement* dist);

   /**
    *    Adds a disturbance which means that the affected node(s)
    *    should be completely avoided when routing.
    */
   void avoidNode(uint32 mapID,
                  uint32 nodeID,
                  bool avoidOtherNodeToo = false);

   /**
    *    Adds a multiplicative penalty to a node. The connections
    *    from this node will penalized using the supplied factor.
    *    @param mapID  The map id of the affected node.
    *    @param itemID The item id of the affected node.
    *    @param penaltyFactor The factor to multiply the old costs with.
    *    @param avoidOtherNodeToo If true there will be a disturbance in
    *                             the other direction too.
    */
   void addPenalty(uint32 mapID,
                   uint32 nodeID,
                   float penaltyFactor,                   
                   bool avoidOtherNodeToo = false);

   /**
    *    Adds a time cost to the node.
    *    @param mapID  The map id of the affected node.
    *    @param itemID The item id of the affected node.
    *    @param timeSec The time in seconds to add to the node.
    *    @param avoidOtherNodeToo If true there will be a disturbance in
    *                             the other direction too.
    */
   void addDelayTime(uint32 mapID,
                     uint32 nodeID,
                     uint32 timeSec,
                     bool avoidOtherNodeToo = false);

   /**
    *    Returns a pointer to the disturbancevector for the wanted
    *    map. Shouldn't be deleted by the caller, belongs to the list.
    *    If mapID = MAX_UINT32 all disturbances will be returned.
    *    @param mapID The mapID to get disturbance for. If mapID =
    *                 MAX_UINT32 all disturbances on all maps will
    *                 be returned.
    */
   const DisturbanceVector* getDisturbances(uint32 mapID = MAX_UINT32) const;

   /**
    *    Adds the disturbances for the supplied map to the vector
    *    distVect. The disturbances should not be deleted, they belong
    *    to the list.
    *    @param distVect Vector to add the disturbances to.
    *    @param mapID    Map id or MAX_UINT32 for all maps.
    */
   int getDisturbances( DisturbanceVector& distVect,
                        uint32 mapID ) const;

   
private:

   /**
    *    Pair suitable for putting into DisturbanceMap_t.
    */
   typedef pair<uint32, DisturbanceVector*> DistVectPair_t;
   
   /**
    *    Disturbances sorted by mapID.
    */
   typedef map<uint32, DisturbanceVector*> DisturbanceMap_t;

   /** 
    *    The disturbances are kept here, sorted by mapid.
    */ 
   DisturbanceMap_t m_distMap;

   /**
    *    Vector containing all the disturbances.
    *    The disturbances in this vector will not be
    *    deleted since they are also in the other
    *    vectors. The opposite would be simpler, though.
    */
   DisturbanceVector m_allDistVect;

   /**
    *    Deletes all the disturbances in the vector vect.
    */
   void deleteVectorContents(DisturbanceVector& vect);
   
};

//----------------------------------------------------------------
//  Implementation of inlines
//----------------------------------------------------------------

inline bool
DisturbanceListElement::shouldAvoid() const
{
   return m_factor == MAX_UINT32 || m_time == MAX_UINT32;
}

inline uint32
DisturbanceListElement::getMapID() const
{
   return first;
}

inline uint32
DisturbanceListElement::getNodeID() const
{
   return second;
}

inline uint32
DisturbanceListElement::floatToRaw( float factor )
{
   // Fix-point
   float tempFactor = MAX_UINT16 * factor;
   if ( tempFactor > MAX_UINT32 ) {
      // Remove the connections
      return MAX_UINT32; 
   } else {
      return uint32(tempFactor);
   }
}

inline uint32
DisturbanceListElement::infoModuleFactorToRaw( uint32 infoModuleFactor )
{
   if ( infoModuleFactor == MAX_UINT32 ) {
      return MAX_UINT32;
   } else {
      return floatToRaw( float(infoModuleFactor) / float(1000) );
   }
}

#endif
