/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTINGNODE_H
#define ROUTINGNODE_H

class RoutingMap;

#include "config.h"
#include "RoutingConnection.h"
#include "RouteConstants.h"
#include "ItemTypes.h"
#include "FileDebugCalcRoute.h"

#include <stdio.h>

#define LEVEL_MASK 0x78000000

/**
 * Contains a node used for routing.
 *
 */
class RoutingNode
{

private:
     /**
       *   Creates a RoutingNode. Should only be used by RoutingMap.
       */
      inline RoutingNode();

      /// To stop others from using the empty constructor.
      friend class RoutingMap;
   
public:
      /**
       *   Constructor for use in OrigDestNode.
       */
      inline RoutingNode( bool dummy );
      
      /**
       *   Sets the infinity of the node.
       *   Should be in OrigDestNode.
       */
      inline void setInfinity(uint8 inf);
      
      /**
       * Adds a connection last in the connection list.
       *
       * @param data     A pointer to the connection.
       * @param forward  Tells if the connection connects in the forward or
       *                 backward direction.
       */
      inline void addConnection(RoutingConnection* data,
                                bool forward);

      /**
       * @name Get- and set functions for the itemID.
       */
      //@{
         /**
          * Get the itemID.
          * 
          * @return The itemID of this node.
          */
         inline uint32 getItemID() const;
         
         /**
          * Set the itemID of this node.
          * 
          * @param itemID The itemID to set.
          */
         inline void setItemID(uint32 itemID);
      //@}

   /**     
    * @returns the index in the node vector      
    */
   inline uint32 getIndex() const;

   /**     
    * @param index the index in the vector      
    */
   inline void setIndex(uint32 index);

   /**
    * @returns the cost to drive from origin to this node.
    */
   inline uint32 getRealCost(const RoutingMap* theMap) const;

   /**     
    * @param cost the cost to drive from origin to this node.
    */
   inline void setRealCost(RoutingMap* theMap, uint32 cost);
  
   /**
    * @returns the estimated cost between origin and destination.
    */
   inline uint32 getEstCost(const RoutingMap* theMap) const;

   /**     
    * @param cost the estimated cost between origin and destination.
    */
   inline void setEstCost(RoutingMap* theMap, uint32 cost);

   /** 
    * @returns the gradient (the node that are in the closest
    * path from orig).
    */
   inline RoutingNode* getGradient(const RoutingMap* theMap) const;

   /**     
    * @param gradient the gradient.
    */
   inline void setGradient(RoutingMap* theMap, RoutingNode* gradient);

   /**
    *   @param dest is true if this node is a destination node,
    *   used for evaluating the cut off value
    */
   inline void setDest(RoutingMap* theMap, bool dest);

   /**
    *  @return true if this node is a destination
    */
   inline byte isDest(const RoutingMap* theMap) const;

   /**
    *   O(n), so don't use in CalcRoute loops.
    *   @return the number of connections of this node
    */
   inline uint32 getNbrConnections() const;
   
   /**
    *   @param   forward  tells if its the first forward
    *            or the first backward connection that will
    *            be returned.
    *   @return  Pointer to the first connection.
    */
   inline RoutingConnection* getFirstConnection( bool forward = true ) const;

   /**
    *   Sets the first connection to a new one. 
    *   @param newFirst The first connection (list).
    *   @param forward  True if the forward connection should be set.
    *   @return The old connection.
    */ 
   inline RoutingConnection* setFirstConnection( RoutingConnection* newFirst,
                                                 bool forward);
   
   /**
    *  Returns the connection matching the index looking in the direction
    *  of the forward parameter.
    *  @param   index is the index of the connection to be returned
    *  @param   forward tells if its in the forward or backward direction
    *  @return  the connection if found else NULL
    */
   RoutingConnection* getConnection( uint32 index, bool forward ) const;

   RoutingConnection* getConnection( RoutingNode* node, bool forward ) const;

   /**
    *  @param lat is the latitude of the node
    */
   inline void setLat( int32 lat );

   /**
    *  @return the latitude of the node
    */
   inline int32 getLat() const;
      
   /**
    *  @param lon is the longitude of the node
    */
   inline void setLong( int32 lon );

   /**
    *  @return the longitude of the node
    */
   inline int32 getLong() const;

   /**
    *   Sets lots of stuff at once.
    */
   inline void setMuch( uint32 itemID,
                        uint32 restrictions,
                        uint32 index,
                        uint32 lat,
                        uint32 lon );
             
   /**
    *   Resets the node data. Must be applied before doing a 
    *   new routing
    */
   inline void reset(RoutingMap* theMap);
   
   /**
    * XXX: To speed the calculation up a bit this information should probably
    *      be moved to RoutingConnection
    * @param res is the restriction into this node @see databaseTypes.
    */
   inline void setRestrictions( uint32 res );

   /**
    * @param lineID is the ID of the line e.g. busline, ferryline etc.
    */
   inline void setLineID( uint16 lineID );

   /**
    * @return the id of the  line.
    */
   inline uint16 getLineID() const;

   /**
    * XXX: To speed the calculation up a bit this information should probably
    *      be a bitfield instead.
    * @return the restriction into this node @see databaseTypes.
    */   
   inline byte getRestriction() const;
      
   /**
    * XXX: May be obsolete
    * @param visited is true if this node is an origin that comes
    * from an externalconnection
    */
   inline void setVisited( RoutingMap* theMap, bool visited );

   /**
    * XXX: May be obsolete
    * @return true if this node is an origin from an external connection
    */
   inline bool isVisited( const RoutingMap* theMap );
   
   /**
    * Used for debug. Traces data to log.
    */
   void dump();

protected:
      
   /// First connection in forward direction.
   RoutingConnection* m_forwardConn;

   ///   A connection in the backward direction
   RoutingConnection* m_backwardConn;

   ///   The latitude of the node
   int32 m_lat;

   /// The longitude of the node
   int32 m_lon;

   ///  The index in the vector
   uint32 m_nodeIndex;

   /// The item ID
   uint32 m_itemID;

   /// The estimated cost
   uint32 m_estimatedCost;

   /// The real cost
   uint32 m_cost;

   /// The gradient
   RoutingNode* m_gradient;

   /// True if the node is a destination
   bool m_dest : 1;

   /// True if the node has been visited
   bool m_isVisited : 1;
      
   /**   
    *   Restrictions to the road, used together with entryRestrictions 
    *   in RoutingConnection.
    *   @see RoutingConnection
    */
   //ItemTypes::entryrestriction_t m_restriction : 2;   
   byte m_restriction;

   /// Current infinity.
   uint8 m_infinity;
   
   /// The ID of a busline etc. 0 if this is a street. Not used.
   uint16 m_lineID : 1;   
};

#include "RoutingMap.h"

inline
RoutingNode::RoutingNode()
{   
}

inline 
RoutingNode::RoutingNode(bool dummy)
{
   m_forwardConn = NULL;
   m_backwardConn = NULL;
   m_lineID = 0; // Must zero this one.
   
   m_infinity    = MAX_UINT8;
   m_restriction = ItemTypes::noRestrictions;
   //reset(NULL);
}

inline void
RoutingNode::reset(RoutingMap* theMap)
{
   // These will become nothing if FILE_DEBUGGING is off.
#ifdef FILE_DEBUGGING
   FileDebugCalcRoute::writeCostToFile('R',
                                       m_itemID,
                                       m_cost,
                                       MAX_UINT32,
                                       true,
                                       m_dest,
                                       "Resetting");
   FileDebugCalcRoute::writeCostToFile('R',
                                       m_itemID,
                                       m_estimatedCost,
                                       MAX_UINT32,
                                       false,
                                       m_dest,
                                       "Resetting");
#endif
   m_cost          = MAX_UINT32;
   m_estimatedCost = MAX_UINT32;
   m_gradient      = NULL;
   m_dest          = false;
   m_isVisited     = false;

   m_infinity = theMap->getInfinity();
   
}



inline uint32
RoutingNode::getItemID() const
{
   return m_itemID;
}

inline void
RoutingNode::setItemID(uint32 itemID)
{
   m_itemID = itemID;
}


inline uint32
RoutingNode::getIndex() const
{
   return m_nodeIndex;
}

inline void
RoutingNode::setIndex( uint32 index )
{
   m_nodeIndex = index;
}

inline uint32
RoutingNode::getRealCost(const RoutingMap* theMap) const
{
   if ( theMap->getInfinity() == m_infinity ) {
      return m_cost;
   } else {
      return MAX_UINT32;
   }
}

inline void
RoutingNode::setRealCost( RoutingMap* theMap, uint32 cost )
{
#ifdef FILE_DEBUGGING
   // DEBUG. Will dissapear if FILE_DEBUGGING is undefined
   FileDebugCalcRoute::writeCostToFile('C',
                                       m_itemID,
                                       m_cost,
                                       cost,
                                       true,
                                       m_dest,
                                       "");
#endif
   if ( theMap->getInfinity() != m_infinity ) {
      reset(theMap);
   }
   m_cost = cost;
}
  
inline uint32
RoutingNode::getEstCost(const RoutingMap* theMap) const
{
   if ( theMap->getInfinity() == m_infinity ) {
      return m_estimatedCost;
   } else {
      return MAX_UINT32;
   }
}

inline void
RoutingNode::setEstCost(RoutingMap* theMap, uint32 cost)
{
#ifdef FILE_DEBUGGING
   FileDebugCalcRoute::writeCostToFile('C',
                                       m_itemID,
                                       m_estimatedCost,
                                       cost,
                                       false,
                                       m_dest,
                                       "");
#endif
   if ( theMap->getInfinity() != m_infinity ) {
      reset(theMap);
   }
   m_estimatedCost = cost;
}

inline void
RoutingNode::setInfinity(uint8 inf)
{
   m_infinity = inf;
}

inline void
RoutingNode::setRestrictions( uint32 res )
{
   //m_restriction = ItemTypes::entryrestriction_t(res);
   m_restriction = byte(res);
}

inline void 
RoutingNode::setLineID( uint16 lineID )
{
   if(lineID != 0 )
      mc2dbg4 << "Setting lineID " << lineID << endl;
   m_lineID = lineID;
}

inline uint16 
RoutingNode::getLineID() const
{
   return m_lineID;
}

inline byte
RoutingNode::getRestriction() const
{
   return m_restriction;
}
      
inline RoutingNode*
RoutingNode::getGradient(const RoutingMap* theMap) const
{
   if ( theMap->getInfinity() == m_infinity ) {
      return m_gradient;
   } else {
      return NULL;
   }
}

inline void
RoutingNode::setGradient( RoutingMap* theMap,
                          RoutingNode* gradient )
{
   if ( theMap->getInfinity() != m_infinity ) {
      reset(theMap);
   }
   m_gradient = gradient;
}

inline void
RoutingNode::setDest( RoutingMap* theMap, bool dest )
{
   if ( theMap->getInfinity() != m_infinity ) {
      reset(theMap);
   } 
   m_dest = dest;
}

inline byte
RoutingNode::isDest( const RoutingMap* theMap ) const
{
   if ( theMap->getInfinity() == m_infinity ) {
      return m_dest;
   } else {
      return false;
   }
}

inline void
RoutingNode::setVisited( RoutingMap* theMap, bool visited )
{
   if ( theMap->getInfinity() != m_infinity ) {
      reset(theMap);
   }
   m_isVisited = visited;
}

inline bool
RoutingNode::isVisited(const RoutingMap* theMap)
{
   if ( theMap->getInfinity() == m_infinity ) {
      return m_isVisited;
   } else {
      return false;
   }
}
      

inline RoutingConnection*
RoutingNode::getFirstConnection( bool forward ) const
{
   if (forward) {
      return m_forwardConn;
   } else {
      return m_backwardConn;
   }
}

inline RoutingConnection*
RoutingNode::setFirstConnection( RoutingConnection* newFirst,
                                 bool forward )
{
   RoutingConnection* retVal = getFirstConnection( forward );
   if ( forward ) {
      m_forwardConn = newFirst;
   } else {
      m_backwardConn = newFirst;
   }
   return retVal;
}


inline uint32
RoutingNode::getNbrConnections() const
{
   uint32 nbrConnections = 0;
   
   RoutingConnection *currentConnection = getFirstConnection();
   while (currentConnection) {
      nbrConnections++;
      currentConnection = currentConnection->getNext();
   }
   return nbrConnections;
}


inline void
RoutingNode::setLat( int32 lat )
{
   m_lat = lat;
}

inline int32
RoutingNode::getLat() const
{
   return m_lat;
}
      
inline void
RoutingNode::setLong( int32 lon )
{
   m_lon = lon;
}

inline int32
RoutingNode::getLong() const
{
   return m_lon;
}

inline
void RoutingNode::addConnection( RoutingConnection* data, bool forward )
{   
   if ( forward ) {
      data->setNext(m_forwardConn);
      m_forwardConn = data;
   } else {
      data->setNext(m_backwardConn);
      m_backwardConn = data;
   }
}

inline void
RoutingNode::setMuch( uint32 itemID,
                      uint32 restrictions,
                      uint32 idx,
                      uint32 lat,
                      uint32 lon )
{
   // From the old constructor
   m_forwardConn = NULL;
   m_backwardConn = NULL;
   
   m_infinity    = MAX_UINT8;

   // Now set stuff.
   m_lat = lat;
   m_lon = lon;
   m_itemID = itemID;
   m_nodeIndex = idx;
   m_restriction = restrictions;
}


#endif



