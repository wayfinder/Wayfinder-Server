/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTINGCONNECTION_H
#define ROUTINGCONNECTION_H

#include "config.h"
#include "DataBuffer.h"
#include "RouteConstants.h"

class RoutingNode; // The connection has a pointer to a node.

/**
 * Contains information about the connections. The costs to transit the
 * connections and the allowed vehicle types are stored in this class.
 *
 */
class RoutingConnectionData
{
  public:
   
   /**
    *  Constructs a RoutingConnectionData object.
    * 
    *  @param buff A DataBuffer containg connection data
    *              according to the protocol described in grammars.tex.
    */
   inline RoutingConnectionData() {};

   /**
    *  Sets the connection data from the supplied buffer.
    *  @param buff A DataBuffer containg connection data
    *              according to the protocol described in grammars.tex.    
    */
   inline void readFromBuffer(DataBuffer* buff);
   
   /**
    * @name Get- and set functions for the cost member variables.
    */
   //@{
      /**
       * Gets cost A.
       * 
       *   @param vehicleRest Used to check for tollroads.
       *   @return Cost A (= distance). 
       */
      inline uint32 getCostA(uint32 vehicleRest) const;

      /**
       *   Sets cost A.
       *   @param cost The new cost.
       */
      inline void setCostA(uint32 cost);
      
      /**
       * Get cost B.
       * 
       *   @param vehicleRest Used to check for tollroads.
       *   @return Cost B (= time). 
       */
      inline uint32 getCostB(uint32 vehicleRest) const;

      /**
       *   Sets cost B.
       *   @param cost The new cost.
       */
      inline void setCostB(uint32 cost);
      
      /**
       *   Get cost C.
       *   @param vehicleRest Used to check for tollroads.
       *   @return Cost C (= time with disturbances).
       */
      inline uint32 getCostC(uint32 vehicleRest) const;

      /**
       * Sets cost C. This is used when TM sends costs updates.
       * 
       * @param costC The cost to set as costC.
       */
      inline void setCostC(uint32 costC);
   
      /**
       *   Get cost D. This cost should always be zero and is not yet
       *   defined.
       * 
       * @return Cost D.
       */
      inline uint32 getCostD() const;
   
   //@}
      
   /**
    * Get the vehicle restriction.
    * @param costC True if we are routing and using disturbances.
    * @return The vehicle restriction.
    */
   inline uint32 getVehicleRestriction(bool costC) const;

   /**
    *   Set the Vehicle restrictions.
    *   @param vehicleRest
    */
   inline void setVehicleRestriction(uint32 vehicleRest);

   /**
    * Trace the data to cout.
    */
   void dump();
   
  private:
   
   /**
    * @name Member variables containing the transit cost.
    */
   //@{
      /**
       * Cost A, the distance cost.
       */
      uint32 m_costA;

      /**
       * Cost B, the time cost.
       */
      uint32 m_costB;

      /**
       * Cost C, the time cost with possible disturbances. Normally, cost
       * B and C are equal, that is no disturbances apply to this node.
       */
      uint32 m_costC;
   
   //@}
   
      /**
       * A bitfield that contains information about what vehicle that may
       * pass the connection.
       */
      uint32 m_vehicleRestriction;
      
}; // RoutingConnectionData


/**
 * Contains indexpointers to the connections.
 * FIXME: This class should contain real pointers instead of indices.
 *
 */
class RoutingConnection
{
  public:

   /**
    *   Initializes the connection with the supplied data.
    *   @param index The index of the connection node.
    *   @param A pointer to the connecting data.
    *
    */
   inline void initConnection(RoutingNode* node,
                              RoutingConnectionData* data);
   
   /**
    *   Creates a new connection.
    * 
    *   @param index The index of the connection node. Must be
    *                inited with a sane value if not set here.
    *   @param A pointer to the connecting data. Must be inited
    *          with a sane value later if not set here.
    */
   inline RoutingConnection(RoutingNode* node,
                            RoutingConnectionData* data);

   /**
    *   Creates a deep copy of the RoutingConnection including
    *   connectiondata. Used for storing old connection data
    *   after a connectioncost has changed.
    */
   RoutingConnection(const RoutingConnection& other);

   /**
    *   Creates an empty RoutingConnection.
    */
   RoutingConnection() : m_data(NULL), m_next(NULL), m_node(NULL)
      {}
   
   /**
    *
    */
   //virtual ~RoutingConnection()  {} ;
   
   /**
    * @name Get- and set functions for the list of connections.
    */
   //@{
      /**
       * Get the next connection in the list of connections from a node.
       * 
       * @return The next connection in the list.
       */
      inline RoutingConnection* getNext() const;

      /**
       * Set the next connection to follow this connection.
       * 
       * @param con The connection to be inserted after this one in the
       *            list.
       */
      inline void setNext(RoutingConnection* con);
   //@}

   /**
    * Get the index of the node that this connection points to.
    * 
    * @return The index of the node which the connection is connecting to.
    */
   inline RoutingNode* getNode() const;

   /**
    * Get the connection data.
    * 
    * @return A pointer to the connection data. 
    */
   inline RoutingConnectionData* getData() const;

   /**
    *   Sets new connection data for the connection.
    *   @param newData The data to copy over the old data.
    */
   inline void setData(const RoutingConnectionData& newData);

   
   /**
    * Dump the information in the connection to cout.
    */
   void dump();
      
  protected:
   
   /**
    * Pointer to a RoutingConnectionData object containing the
    * transit cost.
    */
   RoutingConnectionData* m_data;
      
   /**
    * The next connection in the list.
    */
   RoutingConnection* m_next;

   /**
    * Index in the nodevector (in the map) to the next node. 
    */
   RoutingNode* m_node;
   
}; // RoutingConnection


/**
 * Contains information for an external connection.
 *
 * Warning! Has the external node ID where index normally is stored
 *          in a RoutingConnection.
 *
 */
class ExternalRoutingConnection : public RoutingConnection
{
  public:

   /**
    *   Empty constructor.
    */
   inline ExternalRoutingConnection() {}
   
   /**
    * Constructs an ExternalRoutingConnection object.
    * 
    * @param mapID The ID of the map to which the connection leads.
    * @param nodeID The node ID to which the connection leads.
    * @param data A pointer to the transit costs and restrictions.
    */
   inline ExternalRoutingConnection(uint32 mapID,
                                    uint32 nodeID,
                                    RoutingConnectionData* data);

   /**
    *   Fills in the data for an ExternalRoutingConnection.
    *   @param mapID The ID of the map to which the connection leads.
    *   @param nodeID The node ID to which the connection leads (other
    *                 map).
    *   @param data   A pointer to the cost and restrictions for the conn.
    */
   inline void initConnection(uint32 mapID,
                              uint32 nodeID,
                              RoutingConnectionData* data);
   
   /**
    * Get the map ID.
    * 
    * @return The mapID.
    */
   inline uint32 getMapID() const;

   /**
    * Get the node ID.
    * 
    * @return the nodeID (stored in m_nodeIndex to save space).
    */
   inline uint32 getNodeID() const;
   
  private:
   
   /**
    * The mapID of the map the connection leads to.
    */   
   uint32 m_mapID;

   /**
    *  The node ID of the node that the connection leads to.
    */
   uint32 m_nodeID;
   
}; // ExternalRoutingConnection

///////////////////////////////////////////////////////////////
// Inlined methods for RoutingConnectionData
///////////////////////////////////////////////////////////////

inline void
RoutingConnectionData::readFromBuffer(DataBuffer* buff)
{
   m_costA = buff->readNextLongAligned();
   m_costB = buff->readNextLongAligned();  
   m_costC = buff->readNextLongAligned();
   buff->readNextLongAligned(); // Cost D - now removed
   m_vehicleRestriction = buff->readNextLong();
#if 0
   if ( m_costA > MAX_UINT16 ) {
      mc2dbg << "[RCD]: m_costA > MAX_UINT16" << endl;
   }
   if ( m_costB > MAX_UINT16 ) {
      mc2dbg << "[RCD]: m_costB > MAX_UINT16" << endl;
   }
   if ( m_costC > MAX_UINT16 ) {
      mc2dbg << "[RCD]: m_costC > MAX_UINT16" << endl;
   }
   if ( costD > MAX_UINT16 ) {
      mc2dbg << "[RCD]: m_costD > MAX_UINT16" << endl;
   }
#endif
}



inline uint32
RoutingConnectionData::getCostA(uint32 vehicleRest) const
{
   static const uint32 specialVehicles
      = ItemTypes::avoidTollRoad | ItemTypes::avoidHighway;
   if ( MC2_UNLIKELY ( vehicleRest & m_vehicleRestriction &
                       specialVehicles ) ) {
      uint32 curCost = m_costA;
      if ( vehicleRest & m_vehicleRestriction & ItemTypes::avoidHighway ) {
         curCost = uint32(curCost * Connection::highwayDefaultPenaltyFactor );
      }
      if ( vehicleRest & m_vehicleRestriction & ItemTypes::avoidTollRoad ) {
         curCost += Connection::metersToDistCost(
            Connection::tollRoadDistanceDefaultPenalty_m);
      }
      return curCost;
   } else {
      return m_costA;
   }
}

inline uint32
RoutingConnectionData::getCostB(uint32 vehicleRest) const
{
   static const uint32 specialVehicles
      = ItemTypes::avoidTollRoad | ItemTypes::avoidHighway;
   if ( MC2_UNLIKELY ( vehicleRest & m_vehicleRestriction &
                       specialVehicles ) ) {
      uint32 curCost = m_costB;
      if ( vehicleRest & m_vehicleRestriction & ItemTypes::avoidHighway ) {
         curCost = uint32(curCost * Connection::highwayDefaultPenaltyFactor );
      }
      if ( vehicleRest & m_vehicleRestriction & ItemTypes::avoidTollRoad ) {
         curCost += Connection::secToTimeCost(
            Connection::tollRoadTimeDefaultPenalty_s);
      }
      return curCost;
   } else {
      return m_costB;
   }
}

inline uint32
RoutingConnectionData::getCostC(uint32 vehicleRest) const
{
   static const uint32 specialVehicles
      = ItemTypes::avoidTollRoad | ItemTypes::avoidHighway;
   if ( MC2_UNLIKELY ( vehicleRest & m_vehicleRestriction &
                       specialVehicles ) ) {
      uint32 curCost = m_costC;
      if ( vehicleRest & m_vehicleRestriction & ItemTypes::avoidHighway ) {
         curCost = uint32(curCost * Connection::highwayDefaultPenaltyFactor );
      }
      if ( vehicleRest & m_vehicleRestriction & ItemTypes::avoidTollRoad ) {
         curCost += Connection::secToTimeCost(
            Connection::tollRoadTimeDefaultPenalty_s);
      }
      return curCost;
   } else {
      return m_costC;
   }
}

inline uint32
RoutingConnectionData::getCostD() const
{
   return 1;
}

inline void
RoutingConnectionData::setCostA(uint32 cost)
{
   m_costA = cost;
}

inline void
RoutingConnectionData::setCostB(uint32 cost)
{
   m_costB = cost;
}

inline void
RoutingConnectionData::setCostC(uint32 costC)
{
   m_costC = costC;
}

inline uint32
RoutingConnectionData::getVehicleRestriction(bool costC) const
{
   if ( MC2_UNLIKELY( costC && MC2_UNLIKELY( m_costC == MAX_UINT32 ) ) ) {
      return m_vehicleRestriction & ItemTypes::pedestrian;
   } else {
      return m_vehicleRestriction;
   }   
}

inline void
RoutingConnectionData::setVehicleRestriction(uint32 vehicleRest)
{
   m_vehicleRestriction = vehicleRest;
}

///////////////////////////////////////////////////////////////
// Inlined methods for RoutingConnection
///////////////////////////////////////////////////////////////

inline void
RoutingConnection::initConnection(RoutingNode* node,
                                  RoutingConnectionData* data)
{
   m_data = data;
   m_node = node;
   m_next = NULL;
}

inline
RoutingConnection::RoutingConnection(RoutingNode* node, 
                                     RoutingConnectionData* data)

{
   initConnection(node, data);
}


inline RoutingConnection*
RoutingConnection::getNext() const
{
   return m_next;
}

inline void
RoutingConnection::setNext( RoutingConnection* con )
{
   m_next = con;
}

inline RoutingNode*
RoutingConnection::getNode() const
{
   return m_node;
}


inline RoutingConnectionData*
RoutingConnection::getData() const
{
   return m_data;
}

inline void
RoutingConnection::setData(const RoutingConnectionData& newData)
{
   *m_data = newData;
}

///////////////////////////////////////////////////////////////
// Inlined methods for ExternalRoutingConnection
///////////////////////////////////////////////////////////////

inline void
ExternalRoutingConnection::initConnection(uint32 mapID,
                                          uint32 nodeID,
                                          RoutingConnectionData* data)
{
   RoutingConnection::initConnection(NULL, data);
   m_mapID  = mapID;
   m_nodeID = nodeID;
}

inline
ExternalRoutingConnection::ExternalRoutingConnection(
   uint32 mapID,
   uint32 nodeID,
   RoutingConnectionData* data)
      : RoutingConnection(NULL, data)
{
   m_mapID  = mapID;
   m_nodeID = nodeID;
}


inline uint32
ExternalRoutingConnection::getMapID() const
{
   return m_mapID;
}

inline uint32
ExternalRoutingConnection::getNodeID() const
{
   return m_nodeID;
}

#endif // ROUTINGCONNECTION_H

