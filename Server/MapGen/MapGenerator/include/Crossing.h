/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CROSSING_H
#define CROSSING_H

#include "OldConnection.h"
#include "GMSNode.h"
#include "GMSMap.h"
#include <vector>
#include "ObjVector.h"
#include "Vector.h"
#include "OldRouteableItem.h"
#include "OldStreetSegmentItem.h"

#define XOR(a,b) ((bool(a) && (! bool(b))) || (bool(b) && (! bool(a))))

/*
 *  Feature! 
 *  Use exit count (1-4)in roundabout exit/entry connection so
 *  ExpandRouteProcessor can detect symmetries.
 *  This will enable the use of LEFT_ROUNDABOUT, RIGHT_ROUNDABOUT and AHEAD
 *  in some 3-way, and 5-way roundabout.
 *
 *  Ex. enter rbt with exit count (1). exit rbt with exit count (3), the turn
 *      is ahead in roundabout.
 *  Ex  enter rbt with exit count (4). exit rbt with exit count (1), the turn
 *      is right in roundabout.
 *
 *  If the exit count is at entry or exit is (0) the normal rbExitnbr should
 *  be used --  Take the second exit in the roundabout.
 *
 *  Sometimes (5-way rbt) the entry could get a number but not the exit.
 *  Other values might be used, 11-14 might mean that only RIGHT_ROUNDABOUT
 *  should be allowed. 
 */





/**
 * Describes a connection from to a crossing and contains information
 * about angle, roadclass, entry restrictions and if the streetsegment
 * of the connection is a ramp, part of a roundabout and if it is
 * multi digitalized.
 *
 */
class CrossingConnectionElement : public VectorElement
{
  public:
   /**
    *   @name Operators needed for searching and sorting.
    *   Implements the virtual methods in VectorElement
    *   to make it possible to sort the items.
        */
   //@{
   ///   Define the "equal"-operator. 
   virtual bool operator == (const VectorElement& elm) const {
      return ( (dynamic_cast<const CrossingConnectionElement*> (&elm) )
                        ->m_angle == m_angle );
   }
   
         ///   Define the "not equal"-operator. 
   virtual bool operator != (const VectorElement& elm) const {
      return ( (dynamic_cast<const CrossingConnectionElement*> (&elm) )
               ->m_angle != m_angle );
   }
   
         ///   Define the "greater than"-operator. 
   virtual bool operator > (const VectorElement& elm) const {
      return ( m_angle >
               (dynamic_cast<const CrossingConnectionElement*>
                (&elm) )->m_angle);
   }
   
   ///   Define the "less than"-operator. 
   virtual bool operator < (const VectorElement& elm) const {
            return ( m_angle <
                     (dynamic_cast<const CrossingConnectionElement*>
                      (&elm))->m_angle);
   }


   /**
    * Creates a new CrossingConnectionElement.
    * @param angle The angle of this connection.
    * @param endNodeAngle The angle from the node in the crossing to the
    * SSI's other node. Only used when merging two multiDig CCE's to one.
    * @param node The index of the node in the crossing to which
    *              this connection leads.
    * @param roadClass The road class of the connection.
    * @param roundabout True if this connection is part of a roundabout.
    * @param ramp True if this connection is part of a ramp.
    * @param multiDig True if this connection is multi digitalized.
    * @param name A vector with the name indexes of this connection.
    * @param restr The restrictions for LEAVING the crossing through this
    *              connection.
    * @param ctrlAcc True if this connection has controlled access.
    *                (Default value used to ease its introduction.)
    * @param length the lenght of the SSI of this element. 
    * @param junction The type of junction when leaving the crossing on this
    *                 element
    */
   CrossingConnectionElement(float64 angle,
                             float64 endNodeAngle,
                             byte nodeNbr,
                             byte roadClass,
                             bool roundabout,
                             bool ramp,
                             bool multiDig,
                             Vector name,
                             ItemTypes::entryrestriction_t restr,
                             bool ctrlAcc = false,
                             uint32 length = 0,
                             ItemTypes::junction_t junction =
                             ItemTypes::normalCrossing,
                             bool roundaboutish = false);

   /**
    * Destructor. (Inlined)
    */
   virtual ~CrossingConnectionElement(){};

   /// The index of the node in the crossing to which this connection leads.
   byte    m_nodeIndex;

   /// The angle of this connection.
   float64 m_angle;

   /// The angle all the way to the other node of the streetsegment.
   float64 m_endAngle;

   /// The road class of the connection.
   byte    m_roadClass;

   /// Tells if this connection is part of a ramp.
   bool    m_ramp;

   /// Tells if this connection is part of a roundabout.
   bool    m_roundabout;

   /**
    *  Tells if this connection is part of a traffic figure, similar to
    *  a roundabout.
    */
   bool    m_roundaboutish;   

   /// Tells if this connection is multi digitalized.
   bool    m_ismulti;

   /// Tells if this connection has controlled access.
   bool    m_isCtrAcc;

   /// A vector with the name indexes of this connection.
   Vector  m_nameIndex;

   /// The length of the item.
   uint32 m_length;
   

   /// The restrictions for LEAVING the crossing through this connection.
   ItemTypes::entryrestriction_t m_passNodeRestrictions;

   /// The type of junction when leaving the crossing through this connection.
   ItemTypes::junction_t m_junctionType;
   
   
  private:
   void init(float64 angle,
             float64 endNodeAngle,
             byte node,
             byte roadClass,
             bool multiDig,
             bool roundabout,
             bool roundabouish,
             bool ramp,
             bool ctrlAcc,
             uint32 length,
             Vector name,
             ItemTypes::entryrestriction_t restr,
             ItemTypes::junction_t junction);

   /**
    *   Declare the default constructor private to make sure no one
    *   uses it.
    */
   CrossingConnectionElement();
   
};  // CrossingConnectionElement


/**
 * Describes a crossing and is used to set the turn descriptions and
 * crossin kinds of the connections to the nodes in the crossing.
 *
 */
class Crossing
{
  public:
   /**
    * Constructor.
    * @param firstNode A node in the crossing to start with.
    * @param theMap The map containing the node.
    */
   Crossing(OldNode* firstNode, OldMap* theMap);

   /**
    * Destructor.
    */
   ~Crossing();

   /**
    * Sets turnDescriptions and crossing kind for all connections in
    * the crossing. Even the turns that might be illegal are described.
    * Crossing kind roundabout crossings are set to UNDEFINED_CROSSING   
    * and needs to be set by setRoundaboutCrossingKind().
    * (Uses getTurnDescriptions() and sets the result to member fields.)
    * @return False if the Crossing only contains one node.
    */
   bool setTurnDescriptions();

   /**
    *  Struct containing the output vector of a getTurnDescriptions()
    *  operation.
    *  @param fromConIndex The CCE this Struct comes from.
    *  @param toConIndex The CCE this Struct leads to.
    *  @param description The turndescription of the connection.
    *  @param exitCount The exit count of the connection.
    *  @param debug Flag indicating that this turndescription should be
    *              debuged.
    */ 
   struct turndesc_t {
      byte fromConIndex;
      byte toConIndex;
      ItemTypes::turndirection_t description;
      byte exitCount;
      bool debug;
   };
   
   /// Vector of turndesc_t structs.
   typedef vector<turndesc_t> turndescvector_t;

   /**
    *  Retuens a vector with the turn descripton to the ObjVector with
    *  CrossingConnectionElements that is given.
    *  The method is {\em static} so no Crossing object is needed as no
    *  member fields are accessed.
    *  @param conNotices The connElements of the crossing.
    *  @param rightSideTraffic Set true if not a leftside country.
    *  @param result The output result as a vector.
    *  @param kind The output crossing-kind parameter.
    *  @param countryCode The countryCode of the map where turn descr are
    *             set. If setting turn desc for external conns, give
    *             NBR_COUNTRY_CODES as default. Used for deciding certain
    *             country specific rules.
    *  @param printOut Set to true if debug printout for this crossing is
    *                  needed.
    *  @param mapID The id of the map used. Only used for debugs.
    *  @param nodeID The id of one  of the nodes in the crossing.
    *                Only used for debugs.
    *  @return True if the operation was Ok and the result is correct.
    */
   static bool getTurnDescriptions(ObjVector* conNotices,
                                   bool rightSideTraffic,
                                   turndescvector_t& result,
                                   ItemTypes::crossingkind_t& kind,
                                   StringTable::countryCode countryCode,
                                   bool printOut = false);
   
   /*
    *  Set the correct crossing kind and ext count in the connections that are
    *  part of a roundabout or roundaboutish traffic figure.
    *  Creates new crossings and uses recursive calls to find the whole figure.
    *  All these crossings are then completed.
    *  @return True if a roundabout crossingkind is found
    */
   bool setRoundaboutCrossingKind();

  /**
    *  Sets the turndescriptions to a OldFerryItem node. Turn descriptions from
    *  FItems to SSI are handled by set/getTurnDescriptions().
    *  @param ferryNode The ferry item to set connections to.
    *  @param theMap The map on which the ferry item is located.
    */
   static bool getToFerryItemTurndescriptions(OldNode* ferryNode,
                                              OldMap* theMap);
   
   bool partOfRoundabout() const
      { return !m_noRoundabout; }

   int getNbrOfConnElements() const
      { return m_conNotices.size(); }

   OldNode* getNoEntryNode(bool &leavingRoundabout) const;

   
   /**
    *   Get the connecting angle.
    *   This is normaly the angle from the node to the first gfxData point.
    *   If the distance to the first point is small and the gfxData makes
    *   a sharp turn, the angle between the first and second point is used
    *   instead.
    *   @param gfx The gfxData of the item.
    *   @param isNode0 True if node 0 is in the crossing.
    *   @param printOut Set to true if debug printout for this crossing is
    *                  needed.
    *   @return The angle from north.
    */   
   static float64 getAdjustedAngle(GfxData* gfx, bool isNode0, 
                                   bool printOut=false);


   
  protected:
   /**
    * Default constructor. Protected to avoid use.
    */
   Crossing();
   
   /**
    *   Get the two first or last coordinates in one GdfData.
    *
    *   @param   gfx      Pointer to the GfxData
    *   @param   isNode0  Shall the position be set for node0 (first
    *                     coorinate in gfx) or node1 (last coorinate).
    *   @param   lat1     Outparameter. Set to the first latitude.
    *   @param   lon1     Outparameter. Set to the first longitude.
    *   @param   lat2     Outparameter. Set to the second latitude.
    *   @param   lon2     Outparameter. Set to the second longitude.
    */
   void getLatLon(GfxData* gfx, bool isNode0,
                  int32& lat1, int32& lon1, int32& lat2, int32& lon2);

   /**
    *   Get the angle to the other node.
    */
   void getNodeCoords(GfxData* gfx, bool isNode0,
                      int32& lat1, int32& lon1, int32& lat2, int32& lon2);

   
   /**
    * Fills the connection vector, node vector and sets the other variables
    * of the crossing.
    * @param startNode The node the crossing starts with.
    * @return the number of nodes in the crossing.
    */
   byte fillConnectionNotices(OldNode* startNode);

   /**
    * Reduces the connection vector by merging multidigitalized connections.
    * Creates a transfer vector, m_transfV, to link nodes to the connections
    * in the reduced vonnection vector,
    * @return Number of reductions.
    */
   byte reduceConnections();

   /**
    * Checks if two name index vectors are the same.(The names of the
    * connections are the same.) Used to deside if two multidigitalized
    * connections are realy the same road.
    * @return True if same.
    */
   static bool nameSame(Vector &name1, Vector &name2);

   /**
    * Checks if two adjacent multidigitalized crossing connections are
    * realy the same road. Checks names, angles and entryrestrictions.
    * @return True if actually the same road.
    */
   bool sameCon(CrossingConnectionElement* con1,
                CrossingConnectionElement* con2);

   /**
    * Gets the crossing connection belonging to a  specified node.
    * @param node The node to get the crossing connection from.
    * @return The crossing connection.
    */
   CrossingConnectionElement* getCrossingConnection(OldNode* node);

   /**
    * Gets the crossing connection belonging to a specified connection.
    * @param conn The connection to get the crossing connection from.
    * @return The crossing connection.
    */ 
   CrossingConnectionElement* getCrossingConnection(OldConnection* conn);

   /**
    * Finds the index in the connection vector of the specified
    * crossing connection.
    * @param The crossing connection.
    * @return The index of the crossing connection.
    */
   byte getCrossingConnectionIndex(CrossingConnectionElement* cConn);

   /**
    *  Struct with info on an exit or entry of a roundabout.
    *  @param connection Pointer to turndesc struct repesenting this
    *         connection.
    *  @param multiDig true if this connection i multidig.
    *  @param exit True if this is a exit, false if it is an entry.
    *  @param distance The distance in meters from the first conection of
    *         the roundabout.
    *  @param angle The connecting angle from north.
    *  @param index The rbt connections index in the roundabout. (Each index
    *               can have 0-1 entry and 0-1 exit.
    */
   struct rbtconn_t{ 
      OldConnection* conn;
      bool multiDig;
      bool exit;
      int32 distance;
      float64 angle;
      int index;
      bool valid;
   };

   /**
    * Vector of rbtconn_t structs. Used to keep track of the exit and entries
    * of a roundabout.
    */
   typedef vector<rbtconn_t> rbtconnvector_t;
   
   /*
    *  Recursive method to set crossing kind and exit count in roundabouts.
    *  Can make more recursive calls. After this method is done this crossing
    *  and the one created after this has crossing kind but not exitCount set.
    *  The conns vector is used by the caller to set exit counts.
    *  @param endNode The end node of this roundabout/figure. Indicates that a
    *                 full lap has been made.
    *  @param exits The number of exits found so far.
    *  @param calls The number of recursive calls. To abort endless loops
    *  @param conns The vector with entries and exits to hav their exitCounts
    *               set. Used to indicate symmetry.
    *  @return The final crossing kind for the whole figure (size of Rbt)
    */
   ItemTypes::crossingkind_t setRoundaboutCrossingKind(OldNode* endNode,
                                                       int &exits,
                                                       int calls,
                                                       uint32 nodeLength[4],
                                                       int32 &distance,
                                                       rbtconnvector_t &conns,
                                                       bool backward = false);

  
   /**
    * Checks if a four-way roundabout is symetric, which means that the
    * exits will be marked right, ahead, and left.
    * @param nodeLength A vector with the segment length to each exit.
    * @param maxError The maximum allowed error.
    * @return True if The roundabout is 4-way and symetric.
    */
   bool isSymetricRoundAbout(uint32 nodeLength[4], float64 maxError);

   /**
    *  Sets the exit count value of the roundabout connections according to
    *  the shape of the roundabout.
    *  @param connections Vector with the connections to/from this roundabout.
    *  @param circumference The circumference of the roundabout in meters.
    *  @return The number of set exits+entries.
    */
   int setRoundAboutConnectionSymmetry(rbtconnvector_t connections,
                                       uint32 circumference);
   
  
   /*
    *  Returns true if this crossing contains un merged multidig segments.
    */
   bool isPartOfMultidigCrossing();

   /**
    *  Check neighbouring crossings to see if this multidig crossing is
    *  4 or 3-way.
    */
   ItemTypes::crossingkind_t getMultidigCrossingKind(OldNode* &endNode);

   /**
    *
    */
   bool continueOtherSide();

   /**
    *  True if the connecting OldItem is longer than 12 m.
    *  This is just a temp. half fix of a map error
    */
   bool rbtAngleIsValid(OldNode* connNode, OldStreetSegmentItem* connItem);


   /**
    *  Puts a new rbtconn_t at the right place in the rbtconnvector_t.
    *  @param vector The vector to insert into.
    *  @param conn The rbtconn_t to add.
    *  @param rbtNode The entry node of the rbtconn_t.
    *  @param last If true the conns should be added to the end of the list.
    *              If false in the begining.(Used when going backwards)
    */
   void addRbtConnToVector(rbtconnvector_t &vector, rbtconn_t conn, 
                           OldNode* rbtNode, bool last);
   
   

   /**
    *    The max number of nodex there can be in the crossing.
    */
   static const uint32 m_maxNbrNodesInACrossing;

   /**
    * A vector with the nodes of the crossing. Sorted clockwise. 
    * Keep size in sync with the m_maxNbrNodesInACrossing
    */
   OldNode*  m_nodes[15];

   /**
    * A vector wich links the nodes of the node vector to the crossing
    * connections of the crossing connection vector.
    * Keep size in sync with the m_maxNbrNodesInACrossing
    */
   byte  m_transfV[15];

   /**
    * The vector of crossing connections elements leading to the crossing.
    * Sorted clockwise.
    */
   ObjVector m_conNotices;
   
   /// the number of nodes in the crossing.
   byte m_numberOfNodes;

   /// The map which the crossing lies on.
   OldMap* m_map;

   /// Tells if the crossing is part of a roundabout.
   bool m_noRoundabout;

   /**
    *  Tells if this crossing is part of a traffic figure that is similar
    *  to a roundabout.
    */
   bool m_noRoundaboutish;

   /// The number of ramps in the crossing.
   byte m_nbrRamps;

   /// The number of multidigitalized connections to the crossing.
   byte m_nbrMulti;

   /// The number of road names in the crossing.
   byte m_nbrNames;

   /// Debug variable. Marks if a crossing shall be printed out.
   bool m_printOut;

   /// The largest roadclass in the crossing.
   byte m_majRoadClass;

   /// The number of connections which is of the major roadclass.
   byte m_majConNbr;

   /// The number of connections of the major roadclass with entry
   /// restrictions.
   byte m_majRestrCon;
   
};  // Crossing


#endif // CROSSING_H
