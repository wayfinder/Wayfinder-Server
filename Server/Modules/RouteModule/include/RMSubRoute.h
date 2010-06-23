/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SUBROUTE_RM_H_
#define SUBROUTE_RM_H_

#include "config.h"
#include "RouteConstants.h"

/**
 * Class containing compact subroutes. Each subroute has an id that
 * identifies it in a list.
 *
 */
class RMSubRoute
{
   friend class SubRouteList;
   friend class LeaderSubRouteList;
   
   public:
      /**
       * Creates an empty RMSubRoute. The type is SUBROUTELIST.
       */
      RMSubRoute();

      /**
       *   Creates a copy of another subroute. The type is SUBROUTELIST.
       *
       *   @param subRoute the subroute to copy.
       *   @param copyVisitedAndComplete False if visited and complete
       *                                 should be set to false as in the old
       *                                 "copyconstructor".
       */
      RMSubRoute( RMSubRoute* subRoute, bool copyVisitedAndComplete = true );

      /**
       * Deletes the RMSubRoute and releases the used memory.
       */
      virtual ~RMSubRoute() {}
         
      /**
       * Adds info about a connection that leads to another map.
       *
       * @param mapID wich map the connection leads to.
       * @param nodeID id of the node the connection leads to.
       * @param cost what it costs to reach the node.
       * @param estimatedCost the estimated cost to reach a destination.
       */
      void addExternal(uint32 mapID,
                       uint32 nodeID,
                       uint32 cost,
                       uint32 estimatedCost,
                       int32 lat,
                       int32 lon,
                       uint32 costAsum,
                       uint32 costBsum,
                       uint32 costCsum);
           

      /**
       * Gets info about a connection that leads to another map.
       *
       * @param index         The index in the connectionvector.
       * @param mapID         The map that the connection leads to
       *                      (outparameter).
       * @param nodeID        The id of the node the connection leads to
       *                      (outparameter).
       * @param cost          The cost if reaching the node (outparameter).
       * @param estimatedCost The estimated cost to reach a destination
       *                      (outparameter).
       * @param lat           The latitude of the (prev) node.
       * @param lon           The longitude of the (prev) node.
       */
      inline void getExternal(uint32 index,
                              uint32 &mapID,
                              uint32 &nodeID,
                              uint32 &cost,
                              uint32 &estimatedCost,
                              int32& lat,
                              int32& lon,
                              uint32& costASum,
                              uint32& costBSum,
                              uint32& costCsum);

      /**
       *   Returns the cost-sums for the costs a-c. (For sortdist).
       *   @param index The index in the connectionvector.
       *   @param costA The sum for cost A.
       *   @param costB The sum for cost B.
       *   @param costC The sum for cost C.
       */
      inline void getCostSums(uint32 index,
                              uint32& costA,
                              uint32& costB,
                              uint32& costC);
      
      /**
       * Gets info about a connection that leads to another map.
       *
       * @param index         The index in the connectionvector.
       * @param mapID         The map that the connection leads to
       *                      (outparameter).
       * @param nodeID        The id of the node the connection leads to
       *                      (outparameter).
       * @param cost          The cost if reaching the node (outparameter).
       * @param estimatedCost The estimated cost to reach a destination
       *                      (outparameter).
       */
      void getExternal(uint32 index,
                       uint32 &mapID,
                       uint32 &nodeID,
                       uint32 &cost,
                       uint32 &estimatedCost);
         
      /**
       * Updates the cost variables!!!
       *
       * @param index index in the connectionvector.
       * @param cost what it costs to reach the node.
       * @param estimatedCost the estimated cost to reach a destination.
       */
      void setCost( uint32 index,
                    uint32 cost,
                    uint32 estimatedCost );
      /**
       * Get number of connections in a bordernode.
       * 
       * @return The number of connections in a bordernode.
       */
      inline uint32 getNbrConnections() const;

      /**
       *    Set the ID of this sub route.
       *    @param   id  The new ID of this subroute.
       */
      inline void setSubRouteID( uint32 id );

      /**
       *    Get the ID of this sub route.
       *    @return  The ID of this subroute.
       */
      inline uint32 getSubRouteID();

      /**
       *    Set the ID of the RMSubRoute prevoius to this one in the 
       *    calculated route.
       *    @param   id The ID of the previous RMSubRoute.
       */
      inline void setPrevSubRouteID( uint32 id );

      /**
       *    Get the ID of the RMSubRoute previous to this one in the 
       *    calculated route.
       *    @return  The ID of the previous RMSubRoute.
       */
      inline uint32 getPrevSubRouteID();

      /**
       * @name Get- and set functions for the sucSubRouteID.
       * @memo Get and set the sucSucRouteID.
       * @doc  Get and set the sucSubRouteID.
       */
      //@{
         /**
          * Get the ID of the RMSubRoute sucessive to this one in the
          * calculated route.
          *
          * @return The value of sucSubRouteID.
          */
         inline uint32 getSucSubRouteID();

         /**
          * Set the ID of the RMSubRoute sucessive to this one in the
          * calculated route.
          *
          * @param The new value of sucSubRouteID.
          */
         inline void setSucSubRouteID(uint32 subRouteID);
      //@}
      
      /**
       *    Find out of this RMSubRoute has been developed for further 
       *    routing.
       *    @return True if this RMSubRoute has been developed on other maps.
       */
      inline bool isVisited();

      /**
       *    Set the value of m_visited.
       *    @param visited True if this RMSubRoute is evaluated, false
       *                   otherwise.
       */
      inline void setVisited( bool visited );

      /**
       *    @name Offsets.
       *    @memo Get- and Set-methods for the offsets.
       *    @doc  Some methods to get and set the values of the start
       *          and end offset for this route.
       */
      //@{
         /**
          *    Set the start offset for this route.
          *    @param   offset   The new value of the start offset.
          */
         inline void setStartOffset( uint16 offset );
         
         /**
          *    Get the start offset for this route.
          *    @return The value of the start offset.
          */
         inline uint16 getStartOffset();

         /**
          *    Set the end offset for this route.
          *    @param   offset   The new value of the start offset.
          */
         inline void setEndOffset( uint16 offset );
         
         /**
          *    Get the end offset for this route.
          *    @return The value of the end offset.
          */
         inline uint16 getEndOffset();
      //@}

      /**
       *    @name Directions
       *    @memo Get- and Set-methods for the directions.
       *    @doc  Some methods to get and set the values of the start
       *          and end directions for this route.
       */
      //@{
         /**
          *    Set the start direction for this route.
          *    @param   toNode0  True if starting towards node 0, false
          *                      otherwise.
          */
         inline void setStartToNode0( bool toNode0 );

         /**
          *    Get the start direction for this route.
          *    @return True if starting towards node 0, false otherwise.
          */
         inline bool getStartToNode0();
      
         /**
          *    Set the end direction for this route.
          *    @param   toNode0  True if ending towards node 0, false
          *                      otherwise.
          */
         inline void setEndToNode0( bool toNode0 );

         /**
          *    Get the end direction for this route.
          *    @return True if ending towards node 0, false otherwise.
          */
         inline bool getEndToNode0();
      //@}
      
      /**
       *    Print data about this subroute to standard out.
       *    @param   simple   Optional parameter that if set to false
       *                      gives more output.
       */
      virtual void dump(bool simple = true);
      
      /**
       */
      bool checkExternalMapID( uint32 mapID );

      /**
       *    Set the routeComplete.
       *    @param   routeComplete  The new value of the route-compleete 
       *                            attribute.
       *    @return  True if the rout is a complete route on one map.
       */
      inline void setRouteComplete(bool complete);

      /**
       *    Get the status of routeComplete.
       *    @return True if the CompleteRMSubRoute is indeed a complete 
       *            route.
       */
      inline bool getRouteComplete();

      /**
       */
      void updateFinishedRoutes();
      
      /**
       */
      inline bool isForward();

      /**
       */
      inline void setForward( bool forward );
   
      /**
       */
      inline bool isNodeOnLowLevel( uint32 index );

      /**
       *    Get the size of this subroute when saving it in a packet.
       *    @return  The size (in bytes) of this sub route.
       */
      virtual inline uint32 getPacketSize();

      /**
       * @name Get- and set functions for the m_nbrMapsVisited.
       */
      //@{
         /**
          * Get the number of maps visited by this route (this and
          * previous subroutes) so far.
          * 
          * @return The number of maps visited.
          */
         inline uint16 getNbrMapsVisited();
         
         /**
          * Set the number of maps visited so far by the route (this and 
          * previous subroutes).
          * 
          * @param nbrMaps The new value of number of maps visited.
          */
         inline void setNbrMapsVisited(uint16 nbrMaps);
      //@}

      /**
       * Find out if this RMSubRoute has an external connection to the
       * specified segment.
       *
       * @param mapID  The ID of the map to check if any connection leads
       *               to.
       * @param nodeID The ID of the node to check if any connection leads
       *               to.
       * @return       True if an external connection leads to the segment
       *               with given mapID and nodeID.
       */
      bool findExternal(uint32 mapID, uint32 nodeID) const;

      /**
       * Find out if this RMSubRoute has the same external connections as
       * the one given as parameter.
       *
       * @param  subRoute The RMSubRoute to match external connections with.
       * @return          True if all external connections
       *                  (and their number) are equal, false otherwise.
       */
      bool externalEquals(const RMSubRoute& subRoute) const;
         
  protected:
      /**
       *    @name External nodes
       *    @memo Vectors with info about the nodes on other maps.
       *    @doc  Vectors with info about the nodes on other maps.
       */
      //@{
         /**
          *    The mapIDs that the nodes belongs to.
          */
         Vector m_extMapID;

         /**
          *    The ID of the node of the external nodes.
          */
         Vector m_extNodeID;

         /**
          *    The cost of getting to one of the external nodes.
          */
         Vector m_extCost;

         /**
          * The estimated cost of getting to one of the external nodes.
          */
         Vector m_extEstimatedCost;

         /** 
          *    The latitudes.
          */
         Vector m_lats;

         /**
          *    The longitudes.
          */
         Vector m_lons;

         /**
          *    Sums for costA.
          */
         Vector m_costsA;
         
         /**
          *    Sums for costB.
          */
         Vector m_costsB;

         /**
          *    Sums for costC.
          */
         Vector m_costsC;

      //@}

      /**
       *    ID of a subroute, in reality this is a vectorindex.
       */
      uint32 m_subRouteID;
      
      /**
       *    ID of the subroute's ancestor.
       */
      uint32 m_prevSubRouteID;

      /**
       * ID of the RMSubRoute's sucessor in the route. Will only be set when
       * a route is calculated and is to be sent.
       */
      uint32 m_sucSubRouteID;

      /**
       *    True if this subroute is evaluated, false otherwise.
       */
      bool m_visited;

      /**
       *    @name Directions.
       *    @memo The start and end directions.
       *    @doc  Members to store the directions at the start and end
       *          of this subroute.
       */
      //@{
         /**
          *    True if starting towards node0.
          */
         bool m_endToNode0;

         /**
          *    True if ending towards node0.
          */
         bool m_startToNode0;
      //@}

      /**
       *    @name Offsets
       *    @memo The start and end offsets.
       *    @doc  Members to store the offsets at the start and end
       *          of this subroute. Offset is the way to describe the
       *          exact point on the segment in terms of parts of 
       *          $2^16$.
       */
      //@{
         /**
          *    The start offset.
          */
         uint16 m_startOffset;

         /**
          *    The end offset.
          */
         uint16 m_endOffset;
      //@}

      /**
       * The number of maps visited by this subroute and all the previous 
       * subroutes. (The number of maps the route has passed so forth.)
       */
      uint16 m_nbrMapsVisited;
      
     /**
      */
      bool m_routeComplete;
      
      /**
       */
      bool m_forwardRoute;
}; // RMSubRoute


/**
 * Class containing the calculated subroute. Use when sending 
 * replypackets to the leader.
 *
 */
class CompleteRMSubRoute : public RMSubRoute
{
   ///Instances will always be stored in a SubRouteList
   friend class SubRouteList;
   
   public:
      /**
       * Default constructor. Sets members to default values.
       */
      CompleteRMSubRoute();
      
      /**
       * Constructor. Sets the mapID.
       */
      CompleteRMSubRoute( uint32 mapID );
      
      /**
       * Constructor. Copies the members from parameter instance.
       *
       * @param subRoute instance to copy.
       * @param completeCopy False if we are supposed to use the old
       *                     "copy"-constructor in RMSubRoute.
       */
      CompleteRMSubRoute( RMSubRoute* subRoute,
                        bool completeCopy = true);
      
      /**
       * Constructor. Copies the members from parameter instance.
       *
       * @param CompactRMSubRoute instance to copy.
       * @param completeCopy False if we are supposed to use the old
       *                     "copy"-constructor in RMSubRoute.
       */
      CompleteRMSubRoute( CompleteRMSubRoute* completeSubRoute,
                        bool completeCopy );

      /**
       * Destructor. Doesn't do anything at the moment.
       */            
      virtual ~CompleteRMSubRoute() {};

      /**
       * Functions that converts an ordinary result list to
       * a CompleteRMSubRoute.
       * @see CalcRoute
       * @param map the map where the route lies
       * @param result the result list that is to be converted
       */
//      void makeRMSubRouteFromRoute( RoutingMap* map, Head* result );
      
      /**
       * Retrives the itemID of the subroute's origin. Declared as inline.
       */
      inline Vector* getNodeIDs();

      /**
       *    Make sure that the size of the internal node array is at
       *    least as big as a given value.
       *    @param   n  The minimum of positions in the internal node 
       *                array.
       */
      inline void setAllocSize(int n);

      /**
       *    Set the element at a given position to a given value.
       *    {\it {\bf NB!} The internal array must be large enough
       *                   before calling this method!}
       *    
       *    @param   value The value to insert into node array.
       *    @param   index The position of value in node array.
       */
      inline void setElementAt( uint32 value, uint32 index );
 
      /**
       * Adds value last.
       */
      inline void addLast( uint32 value );

      /**
       * Retrives the mapID. Declared as inline.
       */
      inline uint32 getMapID();
      
      /**
       * Retrieve number of nodes in the subroute. Declared as inline.
       *
       * @return number of nodes in the subroute (so far).
       */
      inline uint32 getNbrNodesInRoute();
      
      /**
       * Adds one node to end this sub route. Declared as inline.
       *
       * @param nodeID Id of the new node.
       */
      inline void addEndNode( uint32 nodeID );

      /**
       *   Returns the last node in the CompleteRMSubRoute.
       */
      inline uint32 getLast() const;
      
      /**
       * Sets the mapID. Declared as inline.
       */
      inline void setMapID( uint32 mapID );
      
      /**
       *    Pront data about this compleete sub route to standard
       *    out.
       *    @param   simple   Optional parameter, that if set to false
       *                      give more outdata.
       */
      virtual void dump(bool simple = true);

      /**
       *    Get the size of this subroute when storing in a packet.
       *    @return  The size of this subroute in bytes.
       */
      virtual inline uint32 getPacketSize();

   private:
      /**
       *    The subroute as itemIDs.
       */
      Vector m_nodeIDs;
      
      /**
       *    The id of the subroute's map.
       */
      uint32 m_mapID;
}; // CompleteRMSubRoute


/**
 * Class that describes a complete proximity route with offset for all
 * the nodes in the route. Most offsets will be MAX\_UINT16 (one can reach
 * the entire segment within the specified cutoff).
 *
 */
class CompleteProximityRMSubRoute : public CompleteRMSubRoute
{
  public:

   /**
    * Default constructor.
    */
   CompleteProximityRMSubRoute();
   
   /**
    * Constructor that constructs a CompleteRMProximitySubRoute
    * from a CompleteRMSubRoute.
    *
    * @param completeRoute A CompleteRoute to copy and give some extra
    *        fields.
    * @param completeCopy False if the old "copy"-constructor for RMSubRoute
    *                     should be used.
    */
   CompleteProximityRMSubRoute(CompleteRMSubRoute* completeRoute,
                             bool completeCopy);

   /**
    * Destructor for this class.
    */
   ~CompleteProximityRMSubRoute();

   /**
    * Gets the total size this subroute will take in a packet.
    *
    * @return The total size of this subroute (in bytes).
    */
   virtual inline uint32 getPacketSize();

   /**
    * Get the offsets of the nodes in the route.
    *
    * @return A Vector with the offsets in the route.
    */
   inline Vector& getOffsets();

   /** 
    * Get the cost vector.
    * 
    * @return A Vector with the costs to reach the respective nodes in 
    *         the route.
    */
   inline Vector& getNodeCosts();

  private:
   
// ========================================================================
//                                                       Member variables =
// ========================================================================

   /**
    * A vector with the offsets of the nodes in the route.
    */
   Vector m_offsets;

   /**
    * A vector containing the costs to reach the respective node. The cost
    * for the origin node is zero.
    */
   Vector m_nodeCosts;

}; // CompleteProximityRMSubRoute

// =======================================================================
//                                     Implementation of inlined methods =


// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//                                                      CompleteRMSubRoute =
inline uint32 
CompleteRMSubRoute::getPacketSize()
{
   return RMSubRoute::getPacketSize() + 8 + m_nodeIDs.getSize()*4;
}

inline Vector* CompleteRMSubRoute::getNodeIDs() 
{
   return &m_nodeIDs;
}

inline void  
CompleteRMSubRoute::setAllocSize(int n) 
{
   m_nodeIDs.setAllocSize(n);
}

inline void  
CompleteRMSubRoute::setElementAt( uint32 value, uint32 index ) 
{
   m_nodeIDs.setElementAt( index, value);
}

inline void  
CompleteRMSubRoute::addLast( uint32 value ) 
{
   m_nodeIDs.addLast( value ); 
}

inline uint32  
CompleteRMSubRoute::getMapID() 
{
   return m_mapID;
}

inline uint32  
CompleteRMSubRoute::getNbrNodesInRoute() 
{
   return m_nodeIDs.getSize();
}

inline void  
CompleteRMSubRoute::addEndNode( uint32 nodeID ) 
{
   m_nodeIDs.addLast(nodeID);         
}

inline uint32
CompleteRMSubRoute::getLast() const
{
   return m_nodeIDs[m_nodeIDs.getSize() - 1];
}

inline void  
CompleteRMSubRoute::setMapID( uint32 mapID ) 
{
   m_mapID = mapID;
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//                                             CompleteProximityRMSubRoute =
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

inline uint32
CompleteProximityRMSubRoute::getPacketSize()
{
   return (8 * m_offsets.getSize() + CompleteRMSubRoute::getPacketSize());
      // Same as 4 * (m_offsets.getSize() + m_nodeCosts.getSize())
}

inline Vector&
CompleteProximityRMSubRoute::getOffsets()
{
   return m_offsets;
}

inline Vector&
CompleteProximityRMSubRoute::getNodeCosts()
{
   return m_nodeCosts;
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//                                                              RMSubRoute =
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

inline uint32  
RMSubRoute::getNbrConnections() const
{
   return m_extMapID.getSize();
}

inline void  
RMSubRoute::setSubRouteID( uint32 id )
{
   m_subRouteID =  id;
}

inline uint32  
RMSubRoute::getSubRouteID()
{
   return m_subRouteID;
}

inline void  
RMSubRoute::setPrevSubRouteID( uint32 id )
{
   m_prevSubRouteID =  id;
}

inline uint32  
RMSubRoute::getPrevSubRouteID() 
{
   return m_prevSubRouteID;
}

inline uint32
RMSubRoute::getSucSubRouteID()
{
   return m_sucSubRouteID;
}

inline void
RMSubRoute::setSucSubRouteID(uint32 subRouteID)
{
   m_sucSubRouteID = subRouteID;
}

inline bool  
RMSubRoute::isVisited() 
{
   return m_visited;
}

inline void  
RMSubRoute::setVisited( bool visited ) 
{
   m_visited = visited;
}

inline void  
RMSubRoute::setStartOffset( uint16 offset )
{
   m_startOffset = offset;
}

inline uint16  
RMSubRoute::getStartOffset()
{
   return m_startOffset;
}

inline void  
RMSubRoute::setEndOffset( uint16 offset )
{
   m_endOffset = offset;
}

inline uint16  
RMSubRoute::getEndOffset()
{
   return m_endOffset;
}

inline void  
RMSubRoute::setStartToNode0( bool toNode0 )
{
   m_startToNode0 = toNode0; 
}

inline bool  
RMSubRoute::getStartToNode0()
{
   return m_startToNode0;
}

inline void  
RMSubRoute::setEndToNode0( bool toNode0 )
{
   m_endToNode0 = toNode0; 
}

inline bool  
RMSubRoute::getEndToNode0()
{
   return m_endToNode0;
}

inline void 
RMSubRoute::setRouteComplete(bool complete) 
{
   m_routeComplete = complete;
}

inline bool 
RMSubRoute::getRouteComplete() 
{
   return m_routeComplete;
}

inline bool  
RMSubRoute::isForward()
{
   return m_forwardRoute;
}

inline void  
RMSubRoute::setForward( bool forward )
{
   m_forwardRoute = forward;
}

inline bool  
RMSubRoute::isNodeOnLowLevel( uint32 index ) 
{
   return IS_LOWER_LEVEL(m_extNodeID[index]);
}

inline uint32 
RMSubRoute::getPacketSize()
{
   return 16 + m_extMapID.getSize()*(28+16);
}

inline uint16 
RMSubRoute::getNbrMapsVisited()
{
   return m_nbrMapsVisited;
}

inline void 
RMSubRoute::setNbrMapsVisited( uint16 nbrMaps )
{
   m_nbrMapsVisited = nbrMaps;
}


inline void
RMSubRoute::getExternal(uint32 index,
                        uint32 &mapID,
                        uint32 &nodeID,
                        uint32 &cost,
                        uint32 &estimatedCost,
                        int32& lat,
                        int32& lon,
                        uint32& costASum,
                        uint32& costBSum,
                        uint32& costCSum)
{
   mapID         = m_extMapID[index];
   nodeID        = m_extNodeID[index];
   cost          = m_extCost[index];
   estimatedCost = m_extEstimatedCost[index];
   lat           = m_lats[index];
   lon           = m_lons[index];
   costASum      = m_costsA[index];
   costBSum      = m_costsB[index];
   costCSum      = m_costsC[index];
}

inline void
RMSubRoute::getCostSums(uint32 index,
                        uint32& costA,
                        uint32& costB,
                        uint32& costC)
{
   if ( index < m_costsA.getSize() ) {
      costA = m_costsA[index];
      costB = m_costsB[index];
      costC = m_costsC[index];
   } else {
      mc2log << error << "RMSR: Reading costs outside vector " << endl;
      costA = costB = costC = 0;
   }
}

inline void
RMSubRoute::getExternal(uint32 index,
                        uint32 &mapID,
                        uint32 &nodeID,
                        uint32 &cost,
                        uint32 &estimatedCost)
{
   int32 lat,lon;
   uint32 dummy;
   getExternal(index, mapID, nodeID, cost, estimatedCost, lat, lon, dummy,
               dummy, dummy);
}


#endif
