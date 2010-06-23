/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SETTURNDESCRIPTIONS_T
#define SETTURNDESCRIPTIONS_T

#include "config.h"
#include "ObjVector.h"
#include "OldConnection.h"
#include "OldNode.h"
#include "OldMap.h"
#include "OldMapHashTable.h"

/**
  *   Objects of this class contains data about one connection that is 
  *   needed when the turndescriptions are set. E.g. the angle, roadclass, 
  *   ramp-, roundabout- and divided flags etc.
  *
  */
class ConnectionElement : public VectorElement {
   public :
      /**
        *   @name Operators needed for searching and sorting.
        *   Implements the virtual methods in VectorElement
        *   to make it possible to sort the items.
        */
      //@{
         ///   Define the "equal"-operator. 
         bool operator == (const VectorElement& elm) const {
            return ( (dynamic_cast<const ConnectionElement*> (&elm) )
                        ->m_angle == m_angle );
         }

         ///   Define the "not equal"-operator. 
         bool operator != (const VectorElement& elm) const {
            return ( (dynamic_cast<const ConnectionElement*> (&elm) )
                        ->m_angle != m_angle );
         }

         ///   Define the "greater than"-operator. 
         bool operator > (const VectorElement& elm) const {
            return ( m_angle >
               (dynamic_cast<const ConnectionElement*> (&elm) )->m_angle);
         }

         ///   Define the "less than"-operator. 
         bool operator < (const VectorElement& elm) const {
            return ( m_angle <
                  (dynamic_cast<const ConnectionElement*> (&elm))->m_angle);
         }
      //@}

      /**
        *   Creates a ConnectionElement-object.
        *   @param   a  The angel of this connection.
        *   @param   c  The connection.
        */
      ConnectionElement(float64 a, OldConnection* c);

      /**
        *   Creates a ConnectionElement-object.
        *   @param   a           The angel of this connection.
        *   @param   c           The connection.
        *   @param   roundabout  True if this connection comes from a 
        *                        roundabout-segment, false otherwise.
        *   @param   ramp        True if this connection comes from a 
        *                        ramp-segment, false otherwise.
        *   @param   divided     True if this connection comes from a 
        *                        divided-segment, false otherwise.
        *   @param   ismulti     True if this segment is multiDigitalised 
        *   @param   restr       The restrictions {\em for the node that
        *                        this connection pass}, not the node where
        *                        it start!
        */
      ConnectionElement(float64 a, OldConnection* c, byte roadClass,
                        bool roundabout, bool ramp, bool divided,
                        bool ismulti,
                        ItemTypes::entryrestriction_t restr);

      /**
        *   Pointer to the corresponding connection.
        */
      OldConnection* m_con;

      /**
        *   The angle between the streetSegments that are combined with 
        *   this ConnectionElement.
        */
      float64 m_angle;

      /**
        *   The class of the road where this connection start.
        */
      byte m_roadClass;

      /**
        *   @name Flags for some values in the corresponding OldStreetSegmentItem
        */
      //@{
         /// Does this connection come from a roundabout segment?
         bool m_roundabout;

         /// Does this connection come from a ramp segment?
         bool m_ramp;

         /// Does this connection come from a divided segment?
         bool m_divided;

         /// Does this connection come from a divided segment?
         bool m_ismulti;
      //@}

      /**
        *   The restrictions of the node that this connection passes.
        *   The situation looks something like this:
        *   \begin{verbatim}
                              O (I)
                              *
                              *
                              *
                              *
                              *  (II)
                              X O************O
               If the connection is from node (I) to (II) then the entry
               restrictions in node X is stored in this member variable.
        *   \end{verbatim}
        */
      ItemTypes::entryrestriction_t m_passNodeRestrictions;

   private:
      /**
        *   Set all the attributes in this ConnectionElement.
        *   @param   angle       The angle between the two segments.
        *   @param   con         The corresponding connection.
        *   @param   roundabout  Does con come from a roundabout segment?
        *   @param   ramp        Does con come from a ramp segment?
        *   @param   divided     Does con come from a divided segment?
        *   @param   rest        The restrictions that con pass {\em not
        *                        in the node where it starts}.
        */
      void init(float64 angle, OldConnection* con, byte roadClass,
                bool roundabout, bool ramp, bool divided, bool ismulti,
                ItemTypes::entryrestriction_t rest);
      
      /**
        *   Declare the default constructor private to make sure no one
        *   uses it.
        */
      ConnectionElement();
};

/**
  *   Use object(s) of this class to set the turndescriptions in all the 
  *   connections that leads to a node.
  *
  *   @see setTurnDir.dxx
  */
class SetTurnDescriptions {
   public:
      //@Include: setTurnDir.dxx
   
      /**
        *   Create an empty object. This constructor could be used when
        *   the setTurnDescriptions-method will be used!
        *   @see setTurnDescriptions
        */
      SetTurnDescriptions();

      /**
        *   Set the turndescriptions for all the connections that leads
        *   to a given node.
        *   @param   toNode   The node to set the connections in.
        *   @param   theMap   The map where toNode is located.
        */
      SetTurnDescriptions(OldNode* toNode, OldMap* theMap);

      /**
        *   Delete this object including the allocated memory.
        */
      virtual ~SetTurnDescriptions();

      /** 
        *   Process the connections to a new node. This method can be
        *   used if this object should be reused!
        *   @param   toNode   The node to set the connections in.
        *   @param   theMap   The nmap where toNode i s located.
        *   @return  True if the turndescriptions are set properly, false
        *            otherwise.
        */
      bool setTurnDescriptions(OldNode* toNode, OldMap* theMap);

      /**
        *   Sets the crossingkinds for the connections to a specific node.
        *   In case the node is part of a roundabout, all connections
        *   leading to and from the roundabout will have their crossingkind
        *   set. 
        *   Note that this method requires that setTurnDescriptions() have
        *   been run previously.
        *   @param   toNode   The node to set the crossingkind for
        *   @param   theMap   The map where toNode is located.
        *   @param   True if the crossingkinds are set properly, false
        *            otherwise.
        */
      bool setCrossingKind(OldNode* toNode, OldMap* theMap);


   private:
      /**
        *   Reset all the member- variables in this object, so that 
        *   it could be reused to update another node.
        *   @param   theMap   The map where the nodes are located.
        */
      void reset(OldMap* theMap);

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
        *   Create notice-objects for all connections that leads to
        *   the current node.
        *   @param   toNode   The node that the connections leads to.
        */
      void fillConnectionNotices(OldNode* toNode);

      /**
        *   Find out if all streetsegments have the same name (the 
        *   segment where current node is located and all segments
        *   where the connections start).
        *
        *   @return  True if all street segments have the same name,
        *            false otherwise.
        */
      bool allNameSame(OldNode* toNode);

      /**
        *   All notice-objects for the connections that leads to the
        *   current node.
        */
      ObjVector m_connectionNotices;

      /**
        *   Checks if a 4 way roundabout is symmetric or not.
        *   @param   nodeLength  Vector with 4 elements, where each element
        *                        relates to a entry in the roundabout. Each
        *                        element should contain the length of the
        *                        roundabout up that entry. The last element
        *                        will then have the circumference of the rb.
        *   @param   maxError    The error threshold that tells us if the
        *                        rb is symmetric enough or not. maxError ==
        *                        0, would mean totally symmetric rb.
        *   @return  True if the roundabout was considered symmetric, false
        *            otherwise.
        */
      bool isSymmetricRoundabout(Vector* nodeLength, 
                                 float64 maxError = 0.25); 
            
      /**
        *   @name Members with data about all the connections.
        */
      //@{
         /// No roundabout flags?
         bool m_noRoundabout;

         /// No divided flags?
         bool m_noDivided;

         /// The number of ramps in the current crossing.
         byte m_nbrRamps;

         /// The number of multiDigitalised roads in the current crossing.
         byte m_nbrMulti;

         /// The number of names of the streets in the current crossing.
         byte m_nbrNames;
      //@}

      /**
        *   The map where the nodes we are processing are located.
        */
      OldMap* m_map;

};

#endif

