/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Crossing.h"
#include "OldExternalConnections.h"
#include "GfxUtility.h"
#include "OldStreetSegmentItem.h"
#include "OldFerryItem.h"
#include "GfxData.h"
#include "NationalProperties.h"
#include "Math.h"

#define MAX_ROUNDABOUT_ERROR  0.25



// MapEditor dump
// use "NODE: mapID.nodeID;"+  endl; for nodes and
//"TURNDESCRIPTION: mapID.nodeID;mapID.nodeID;int(turndesc);int(crossingKind);"
// + endl;
#define CROSSING_DUMP_SIZE 11       // Set high (>10) to avoid dumps.
#define ROUNDABOUT_DUMP false       // Set to false to avoid dumps.
#define TURNDESCRIPTION_DUMP false  // Set to false to avoid dumps.
#define NO_CONNECTION_DUMP  false  // Set to false to avoid dumps.
#define NO_THROUGHFARE_DUMP false
#define SUSPECTED_MAP_ERROR_DUMP false
#define SUSPECT_TURN_DUMP true
#define SUSPECT_NAME_DUMP false
#define NO_MULTIDIG_DUMP false


// Need to keep in sync with allocation of m_nodes and m_transfV
const uint32 Crossing::m_maxNbrNodesInACrossing = 15;

// CrossingConnectionElement --------------------------------

CrossingConnectionElement::CrossingConnectionElement()
{

}


CrossingConnectionElement::CrossingConnectionElement(float64 angle,
                                                     float64 endNodeAngle,
                                                     byte node,
                                                     byte roadClass,
                                                     bool roundabout,
                                                     bool ramp,
                                                     bool multiDig,
                                                     Vector name,
                                                     ItemTypes::
                                                     entryrestriction_t
                                                     restr,
                                                     bool ctrlAcc,
                                                     uint32 length,
                                                     ItemTypes::junction_t
                                                     junction,
                                                     bool roundaboutish)
   : m_nameIndex(5,5)
{
   init(angle, endNodeAngle, node, roadClass,  roundabout, roundaboutish, ramp,
        multiDig, ctrlAcc, length, name, restr, junction);
}

void
CrossingConnectionElement::init(float64 angle,
                                float64 endNodeAngle,
                                byte node,
                                byte roadClass,
                                bool roundabout,
                                bool roundaboutish,
                                bool ramp,
                                bool multiDig,
                                bool ctrlAcc,
                                uint32 length,
                                Vector nameIndex,
                                ItemTypes::entryrestriction_t restr,
                                ItemTypes::junction_t junction)
{
   m_angle         = angle;
   m_endAngle      = endNodeAngle;
   m_nodeIndex     = node;
   m_roadClass     = roadClass,
   m_roundabout    = roundabout;
   m_ramp          = ramp;
   m_ismulti       = multiDig;
   m_isCtrAcc      = ctrlAcc;
   m_length        = length;
   m_junctionType  = junction;
   m_roundaboutish = roundaboutish;  
   int size = nameIndex.getSize();
   for(int i = 0 ; i < size ; i++)
      m_nameIndex.addLast( nameIndex[i] );
   m_passNodeRestrictions = restr;
   
}


// Crossing ----------------------------------------------------------

Crossing::Crossing()
{
  
}

Crossing::Crossing(OldNode* firstNode, OldMap* theMap)
      : m_conNotices(10,10)
{
   m_map             = theMap;
   m_noRoundabout    =   true;
   m_noRoundaboutish =   true;
   m_nbrMulti        =      0;
   m_nbrNames        =      0;
   m_printOut        =  false;
   m_numberOfNodes   = 0;

   fillConnectionNotices(firstNode);
   if(m_printOut)
      cerr << (int)m_numberOfNodes << " nodes, " << m_conNotices.getSize()
           << "CCE's " << endl;
   reduceConnections();
   if(m_printOut)
      cerr << (int)m_numberOfNodes << " nodes, " << m_conNotices.getSize()
           << "reduced CCE's " << endl;
}
Crossing::~Crossing()
{
   m_conNotices.deleteAllObjs();
   if(m_printOut)
      cerr << "printOut end..." << endl;
}


byte
Crossing::fillConnectionNotices(OldNode* toNode)
{
   // If increasing this, also change hardcoded check in RoutingMap 
   // of numConnections (all of them)
   uint32 maxNbrTempNodes = 45;
   OldNode*  tempNodes[maxNbrTempNodes];
   // First node is always a ssi node.
   OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
                              (m_map->itemLookup(toNode->getNodeID()));
   GfxData* gfx = ssi->getGfxData();
   uint16 nbrCon = toNode->getNbrConnections();
   if ( nbrCon > maxNbrTempNodes ) {
      mc2log << error << "Crossing::fillConnectionNotices node "
             << toNode->getNodeID() << " has too many connections ("
             << nbrCon << " vs " << maxNbrTempNodes 
             << ") - reserve more" << endl;
      MC2_ASSERT(false);
   } else if ( nbrCon > 30 ) {
      mc2dbg << "Node " << toNode->getNodeID() << " has many connections: "
             << nbrCon << endl;
   }

   // Turn on debug print
//   if ((uint32(toNode->getNodeID()) == 402653947) &&(m_map->getMapID() == 0x3))
//   {
//      //m_printOut = true;
//      //cerr << "Debug m_printOut = true for node " 
//      //     << toNode->getNodeID() << " nbrCon=" << nbrCon << endl;
//   }

   

   
   // Maintain the memeberflags
   if (ssi->isRoundabout())
      m_noRoundabout = false;
   if (ssi->isRoundaboutish())
      m_noRoundaboutish = false;
   if (ssi->isRamp())
      m_nbrRamps++;
   if (ssi->isMultiDigitised())
       m_nbrMulti++;
   if (ssi->getNbrNames() > 0)
      m_nbrNames++;
   // To-coordinates
   int32 toLat1; 
   int32 toLat2; 
   int32 toLon1; 
   int32 toLon2;
   
   // From-coordinates
   int32 fromLat1; 
   int32 fromLat2; 
   int32 fromLon1; 
   int32 fromLon2;
   
   uint32 fromID;
   Vector itemName(5,5);
   for(int j= 0 ; j < ssi->getNbrNames() ; j++)
      itemName.addLast( ssi->getStringIndex((byte) j) ) ;
   

   // Calculate the angle of allthe connection that lead to this node
   getLatLon(gfx, toNode->isNode0(), toLat1, toLon1, toLat2, toLon2);
   
   float64 toAngleFromNorth = GfxUtility::getAngleFromNorth( 
      toLat1, toLon1, toLat2, toLon2);
   float64 adjAng = getAdjustedAngle(gfx, toNode->isNode0(), m_printOut);
   if(toAngleFromNorth != adjAng){
      if(m_printOut){
         cerr << "     ANGLES  : "  << toAngleFromNorth << " : "
              << adjAng <<  "     OldNode: " << toNode->getNodeID() <<  endl;
      }
      toAngleFromNorth = adjAng;
      if(SUSPECTED_MAP_ERROR_DUMP){
         cout << "NODE: " << m_map->getMapID() << "."
              << toNode->getNodeID() << ";" << endl;
      }
   }
   
   // cout << "Angle " << 1 << " : " << toAngleFromNorth <<endl;

   // Get the coords of the nodes.
   getNodeCoords(gfx, toNode->isNode0(), toLat1, toLon1, toLat2, toLon2);
   uint32 firstLength = ssi->getLength();
   if(m_printOut)
      cerr << "Length : " << firstLength << endl;
   if(firstLength > 50){
      uint32 offset;
      if(toNode->isNode0()){
         // Calc. offset from node0.
         offset = (MAX_UINT16/firstLength)*50;
      } else {
         // Calc. offset from node1.
         offset = (MAX_UINT16/firstLength)*(firstLength-50);
      }
      if(m_printOut)
         cerr << "Offset : " << offset << "/" << MAX_UINT16<< endl;
      ssi->getGfxData()-> getCoordinate(offset, toLat2, toLon2);
      if(m_printOut){
         cerr << "lat1 : " << toLat1  << endl;
         cerr << "lat2 : " << toLat2  << endl;
         cerr << "lon1 : " << toLon1  << endl;
         cerr << "lon2 : " << toLon2  << endl;
      }
   }
   
   float64 endNodeAngleFromNorth = GfxUtility::getAngleFromNorth( 
      toLat1, toLon1, toLat2, toLon2);
   CrossingConnectionElement* elm = new CrossingConnectionElement(
      0, // angle
      0, // end node angle
      0, // node nbr
      ssi->getRoadClass(),
      ssi->isRoundabout(),
      ssi->isRamp(),
      (ssi->isMultiDigitised() || ssi->isControlledAccess()),
      itemName,
      toNode->getEntryRestrictions(),
      ssi->isControlledAccess(),
      ssi->getLength(),
      toNode->getJunctionType(),
      ssi->isRoundaboutish());
   /*
     if(toNode->getJunctionType() != 0){
      cout << "NODE: " << m_map->getMapID() << "."
           << toNode->getNodeID() << ";" << endl;
   }
   if(ssi->isRoundaboutish()){
      cout << "NODE: " << m_map->getMapID() << "."
           << toNode->getNodeID() << ";" << endl; 
   }
   */
   m_majRoadClass = ssi->getRoadClass();
   m_majConNbr    = 1;
   if (toNode->getEntryRestrictions() == ItemTypes::noWay)
      m_majRestrCon = 1;
   else
      m_majRestrCon = 0;
   m_conNotices.addLast(elm);
   tempNodes[0] = toNode;
   if (nbrCon > 0)
   {

      //---------------------------------------------------
      OldNode* fromNode;
      for (uint16 i=0; i < nbrCon; i++) {
         if(!toNode->getEntryConnection(i)->isMultiConnection()){
            fromID = toNode->getEntryConnection(i)->getConnectFromNode();
            OldStreetSegmentItem* fromSSI = dynamic_cast<OldStreetSegmentItem*>
               (m_map->itemLookup(fromID));
            if(fromSSI != NULL) // It could be a ferry segment item.
            {
               fromNode = fromSSI->getNodeFromID(fromID);
               
               getLatLon(  fromSSI->getGfxData(), !(fromNode->isNode0()), 
                           fromLat1, fromLon1, fromLat2, fromLon2);
               
               float64 fromAngleFromNorth = GfxUtility::getAngleFromNorth(
                  fromLat1, fromLon1, fromLat2, fromLon2);
               float64 adjAng = getAdjustedAngle(fromSSI->getGfxData(),
                                                 !fromNode->isNode0(),
                                                 m_printOut);
               if(fromAngleFromNorth != adjAng){
                  if(m_printOut){
                     cerr << "     ANGLES  : "  << fromAngleFromNorth << " : "
                          << adjAng <<  "     OldNode: "
                          <<  TOGGLE_UINT32_MSB(fromID) <<  endl;
                  }
                  
                  fromAngleFromNorth = adjAng;
                  
                  if(SUSPECTED_MAP_ERROR_DUMP){
                     cout << "NODE: " << m_map->getMapID() << "."
                          << TOGGLE_UINT32_MSB(fromID) << ";" << endl;
                  }
                  
               }
               float64 deltaAngle = fromAngleFromNorth - toAngleFromNorth;

            
               // Find suitable point on the ssi and calculate angle.
               getNodeCoords(fromSSI->getGfxData(), !(fromNode->isNode0()),
                             fromLat1, fromLon1, fromLat2, fromLon2);
               uint32 length = fromSSI->getLength();
               // Segment is too long use point after 50 m instead.
               if(m_printOut)
                  cerr << "Length : " << length << endl;
               if(length > 50){
                  uint32 offset;
                  if(!fromNode->isNode0()){
                     // Calc. offset from node0.
                     offset = (MAX_UINT16/length)*50;
                  } else {
                     // Calc. offset from node1.
                     offset = (MAX_UINT16/length)*(length-50);
                  }
                  if(m_printOut)
                     cerr << "Offset : " << offset << "/" << MAX_UINT16<< endl;
                  fromSSI->getGfxData()->
                     getCoordinate(offset, fromLat2, fromLon2);
                  if(m_printOut){
                     cerr << "lat1 : " << fromLat1  << endl;
                     cerr << "lat2 : " << fromLat2  << endl;
                     cerr << "lon1 : " << fromLon1  << endl;
                     cerr << "lon2 : " << fromLon2  << endl;
                  }
                  
               }
               
               float64 fromNodesAngle = GfxUtility::getAngleFromNorth(
                  fromLat1, fromLon1, fromLat2, fromLon2);
               fromNodesAngle = fromNodesAngle - endNodeAngleFromNorth;
               if (deltaAngle > 2*M_PI)
                  deltaAngle -= M_PI*2;
               if (deltaAngle < 0)
                  deltaAngle += M_PI*2;
               
               if (fromNodesAngle > 2*M_PI)
                  fromNodesAngle -= M_PI*2;
               if (fromNodesAngle < 0)
                  fromNodesAngle += M_PI*2;
               
               if(m_printOut)
                  cerr << "fromNodesAngle : " << fromNodesAngle << endl;
               
               // Maintain the memeberflags
               if (fromSSI->isRoundabout())
                  m_noRoundabout = false;
               if (fromSSI->isRamp())
                  m_nbrRamps++;
               if (fromSSI->isMultiDigitised())
                  m_nbrMulti++;
               if (fromSSI->getNbrNames() > 0)
                  m_nbrNames++;
               if (fromSSI->getRoadClass() == m_majRoadClass)
                  m_majConNbr++;
               // Add a new OldConnectionElement to the array
               OldNode* passNode = m_map->nodeLookup(fromID ^ 0x80000000);
               if((passNode->getNodeID() ==   402655009)&&
                  (m_map->getMapID() == 0xba ))
               {
                  //m_printOut = true;
                  //cerr << "Debug m_printOut = true " << endl;
               }
            
               m_numberOfNodes++; // why is this increased??
               Vector itemName(5,5);
               for(int j= 0 ; j < fromSSI->getNbrNames() ; j++)
                  itemName.addLast( fromSSI->getStringIndex((byte) j) );
               elm = new CrossingConnectionElement(deltaAngle,
                                                   fromNodesAngle,
                                                   i+1,
                                                   fromSSI->getRoadClass(),
                                                   fromSSI->isRoundabout(),
                                                   fromSSI->isRamp(),
                                                   (fromSSI->
                                                    isMultiDigitised() ||
                                                    fromSSI->
                                                    isControlledAccess()),
                                                   itemName,
                                                   passNode->
                                                   getEntryRestrictions(),
                                                   fromSSI->
                                                   isControlledAccess(),
                                                   fromSSI->getLength(),
                                                   passNode->getJunctionType(),
                                                   fromSSI->isRoundaboutish());
               /*
                 if(passNode->getJunctionType() != 0){
                 cout << "NODE: " << m_map->getMapID() << "."
                 << passNode->getNodeID() << ";" << endl;
                 }
               if(fromSSI->isRoundaboutish()){
                  cout << "NODE: " << m_map->getMapID() << "." 
                       << passNode->getNodeID() << ";" << endl; 
               } 
               */
               m_conNotices.addLast(elm);
               tempNodes[i+1] = passNode;
               if (fromSSI->getRoadClass() < m_majRoadClass)
               {
                  m_majRoadClass = fromSSI->getRoadClass();
                  m_majConNbr   = 1;
                  m_majRestrCon = 0;
                  if(passNode->getEntryRestrictions() == ItemTypes::noWay)
                     m_majRestrCon++;
               }
            }
            
            
            // Its a ferry item. Dont include in the conNotice vector.
            // Set exit ferry in this connection.
            else if(dynamic_cast<OldFerryItem*>(m_map->itemLookup(fromID))
                    != NULL)
            {
               ItemTypes::turndirection_t turn;
               // Check if it is a boundry segment or not.
               if ( m_map->getBoundrySegments()
                    ->getBoundrySegment( ssi->getID() ) == NULL ) {
                  // Ordinary ssi.
                  turn = ItemTypes::EXIT_FERRY;
                  if(m_printOut)
                     cerr << "Setting EXIT_FERRY from : " << fromID << " to " 
                          << toNode->getNodeID() << endl;
               } else {
                  // Boundary segment. Follow road.
               turn = ItemTypes::FOLLOWROAD;
               if(m_printOut)
                  cerr << "Setting FOLLOWROAD (ferry) from : " << fromID 
                       << " to " << toNode->getNodeID() << endl;
               }
               OldConnection* ferryConn = toNode->getEntryConnection(i);
               ferryConn->setTurnDirection( turn );
               ferryConn->setExitCount(1);
               ferryConn->setCrossingKind(ItemTypes::NO_CROSSING);
            }
         }
      }

      // Sort the connections to make it easier to say "The second road to 
      // the left in the 5-way crossing" 
      m_conNotices.sort();
      uint32 nbrOfCon = m_conNotices.getSize();
      
      // check that size fits allocation of m_nodes
      MC2_ASSERT( nbrOfCon <= m_maxNbrNodesInACrossing);
      for (uint32 i = 0 ; i < nbrOfCon; i++)
      {
         m_nodes[i] = tempNodes[static_cast<CrossingConnectionElement*>
                                   (m_conNotices[i])->m_nodeIndex];
         static_cast<CrossingConnectionElement*>
            (m_conNotices[i])->m_nodeIndex = i;
      }
      
      //    for (int i= 0; i < nbrCon+1; i++)        
      //   static_cast<CrossingConnectionElement*>(m_conNotices[i])->
      //      m_nodeIndex = i;      

   }
   m_numberOfNodes = (byte) m_conNotices.getSize();
   if (m_printOut)
   {
      cerr << "----------------------------------------" << endl;
      cerr << toAngleFromNorth << endl;
      cerr << endNodeAngleFromNorth << endl;
      cerr << m_nodes[0]->getNodeID() << endl;
      for(int i = 0; i < m_numberOfNodes; i++)
      {
         cerr << "Node "<< i << ", " << "angle : "
              << static_cast<CrossingConnectionElement*>
            (m_conNotices[i])->m_angle << endl;
         cerr << " fromNodesAngle : "
              << static_cast<CrossingConnectionElement*>
            (m_conNotices[i])->m_endAngle << endl;

         if (static_cast<CrossingConnectionElement*>(m_conNotices[i])
             ->m_ramp)
            cerr << " is ramp" << endl;
         if (static_cast<CrossingConnectionElement*>(m_conNotices[i])
             ->m_roundabout)
            cerr << " is rampoundabout" << endl;
         if (static_cast<CrossingConnectionElement*>
             (m_conNotices[i])->m_ismulti)
            cerr << " is multi" << endl;
         else
            cerr << " not multi" << endl;
         if (static_cast<CrossingConnectionElement*>(m_conNotices[i])
             ->m_passNodeRestrictions == ItemTypes::noWay)
            cerr << " noWay" << endl;
         cerr << " rc=" << (int)static_cast<CrossingConnectionElement*>
              (m_conNotices[i])->m_roadClass << endl;
         if ( static_cast<CrossingConnectionElement*>
               (m_conNotices[i])->m_nameIndex.getSize() > 0 ) {
            cerr << " " << i << ":" 
                 << m_map->getName(static_cast<CrossingConnectionElement*>
                                   (m_conNotices[i])->m_nameIndex[0]) << endl;
         } else {
            cerr << " " << i << ":-noName-" << endl;
         }
         cerr << " junction type: "
              << (int)static_cast<CrossingConnectionElement*>
              (m_conNotices[i])->m_junctionType << endl;
         if (static_cast<CrossingConnectionElement*>(m_conNotices[i])
             ->m_roundaboutish)
            cerr << " is roundaboutish traffic figure" << endl;
      }
   }
   return m_numberOfNodes;
} // fillConnectionNotices

byte
Crossing::reduceConnections()
{
   byte reductions = 0;
   int k = 0;
   int i = 0;
   m_transfV[i++] = k++;
   CrossingConnectionElement* con0 = (CrossingConnectionElement*)
      m_conNotices[0];
   // check that size fits allocation of m_transfV
   MC2_ASSERT( m_numberOfNodes <= m_maxNbrNodesInACrossing);
   while (i< m_numberOfNodes)
   {
      CrossingConnectionElement* con1 = (CrossingConnectionElement*)
         m_conNotices[k-1];
      CrossingConnectionElement* con2 = (CrossingConnectionElement*)
         m_conNotices[k];
      if((m_numberOfNodes > 2) && sameCon(con1, con2))
      {
         if (m_printOut)
            cerr << " Angles : " << con1->m_angle << ", "
                 << con2->m_angle << endl;
         m_transfV[i] = k-1;
         float64 newAngle = (con1->m_angle +con2->m_angle) / 2 ;
         con1->m_angle = newAngle;
         con1->m_passNodeRestrictions = ItemTypes::noRestrictions;
         con1->m_ismulti = false;
         CrossingConnectionElement* elementToRemove =
            (CrossingConnectionElement*) m_conNotices.getElementAt(k);
         m_conNotices.removeElementAt(k);
         delete elementToRemove;
         reductions++;
         
      }
      else if((m_numberOfNodes > 2) && (i == m_numberOfNodes-1)
              && sameCon(con2, con0))
      {
         if (m_printOut)
            cerr << " angles : " << con0->m_angle << ", "
                 << con2->m_angle << endl;
         m_transfV[i] = 0;
         float64 newAngle = (2*M_PI + con2->m_angle) / 2 ;
         if (m_printOut)
            cerr << " new angle : " << newAngle << endl;
         con0->m_angle = newAngle;
         con0->m_passNodeRestrictions = ItemTypes::noRestrictions;
         con0->m_ismulti = false;
         CrossingConnectionElement* elementToRemove =
            (CrossingConnectionElement*) m_conNotices.getElementAt(k);
         m_conNotices.removeElementAt(k);
         delete elementToRemove;
         reductions++;
      }
      else
      {
         m_transfV[i] = k;
         k++;
      }
      i++;
   }
   if(reductions > 0)
   {
      if(m_printOut)
      {   
         cerr << "Reductions : " << (int) reductions << endl;
         cerr << "Transf : " ;
         for(int i = 0; i < m_numberOfNodes; i++)
         {
            cerr << (int) m_transfV[i] << " ";
         }
         cerr << endl;
      }
      
      float64 adjAngle = con0->m_angle;
      con0->m_angle = 0;
      if(adjAngle != 0)
      {
         if(adjAngle > M_PI)
            adjAngle -= 2*M_PI;
         if(m_printOut)
         cerr << "Adjusting angles. : "<< adjAngle << endl;
         for(uint32 i = 1 ; i < m_conNotices.getSize();i++)
         {
            CrossingConnectionElement* con = (CrossingConnectionElement*)
               m_conNotices[i];
            con->m_angle -= adjAngle;
         }
      }
   }
   
   return reductions;
}

bool
Crossing::nameSame(Vector& name1, Vector& name2)
{
   uint32 size = name1.getSize();
   if (size !=  name2.getSize())
      return false;
   for(int i = 0; i < (int) size; i++)
      if(name1[i] != name2[i])
         return false;
   return true;
}

bool
Crossing::sameCon(CrossingConnectionElement* con1,
                  CrossingConnectionElement* con2)
{
   bool mergePrintOut = false;
   if ((con1->m_ismulti && con2->m_ismulti)  &&
       (con1->m_roadClass == con2->m_roadClass)
       // && (nameSame(con1->m_nameIndex, con2->m_nameIndex)) --v- moved
       )
   {
      bool h = m_map->driveOnRightSide();
      if (((con1->m_passNodeRestrictions == ItemTypes::noWay)&& h &&
          (con2->m_passNodeRestrictions != ItemTypes::noWay)) ||
          ((con1->m_passNodeRestrictions != ItemTypes::noWay)&& !h &&
           (con2->m_passNodeRestrictions == ItemTypes::noWay)))
      {
         float64 angle =  con2->m_endAngle - con1->m_endAngle;
         if (angle < 0)
            angle += M_PI*2;
         if (angle > M_PI*2)
            angle -= M_PI*2;
         OldStreetSegmentItem* ssi1 = static_cast<OldStreetSegmentItem*>
            (m_map->itemLookup(m_nodes[con1->m_nodeIndex]->getNodeID()));
         OldStreetSegmentItem* ssi2 = static_cast<OldStreetSegmentItem*>
            (m_map->itemLookup(m_nodes[con2->m_nodeIndex]->getNodeID()));
         
         GfxData* gfx1 = ssi1->getGfxData();
         GfxData* gfx2 = ssi2->getGfxData();
         float64 angle1 = 0;
         float64 angle2 = M_PI;
         uint32 nbrPts1 = gfx1->getNbrCoordinates(0);
         uint32 nbrPts2 = gfx2->getNbrCoordinates(0);
         if((nbrPts1 > 2) && (nbrPts2 > 2)){
            if(m_nodes[con1->m_nodeIndex]->isNode0())
               angle1 = GfxUtility::getAngleFromNorth(gfx1->getLat(0,1),
                                                      gfx1->getLon(0,1),
                                                      gfx1->getLat(0,2),
                                                      gfx1->getLon(0,2));
            else
               angle1 = GfxUtility::getAngleFromNorth(
                  gfx1->getLat(0,nbrPts1-2), gfx1->getLon(0,nbrPts1-2),
                  gfx1->getLat(0,nbrPts1-3), gfx1->getLon(0,nbrPts1-3));
            
            if(m_nodes[con2->m_nodeIndex]->isNode0())
               angle2 = GfxUtility::getAngleFromNorth(gfx2->getLat(0,1),
                                                      gfx2->getLon(0,1),
                                                      gfx2->getLat(0,2),
                                                      gfx2->getLon(0,2));
            else
               angle2 = GfxUtility::getAngleFromNorth(
                  gfx2->getLat(0,nbrPts2-2), gfx2->getLon(0,nbrPts2-2),
                  gfx2->getLat(0,nbrPts2-3), gfx2->getLon(0,nbrPts2-3));
         }
         float64 secondAngle = angle1-angle2;
         if (secondAngle < -M_PI)
            secondAngle += M_PI*2;
         if (secondAngle > M_PI)
            secondAngle -= M_PI*2;                           
         // Lets seee.

         if((secondAngle > -0.5 *M_PI_6) && (secondAngle < 0.5 *M_PI_6)){
            
            return true;
         }
         if (((angle  < 2.6 *M_PI_6)) && true)
         {
            if(nameSame(con1->m_nameIndex, con2->m_nameIndex)){
               if(m_printOut)
               {
                  cerr << "ANGLE : " << angle << " MEARGING !!! "  << endl;
                  cerr << "Conn elements merged " << endl;
               }
               if(mergePrintOut)
               {
                  cerr << "Merge ! : node : "
                       << REMOVE_UINT32_MSB(m_nodes[0]->getNodeID())<< ", a: "
                       << angle << endl;
               }
               
               return true;
            } else {
               // cerr << " Name differs no mearge !" << endl;
               if(SUSPECT_NAME_DUMP &&
                  (m_numberOfNodes == 3)){
                  cout << "NODE: " << m_map->getMapID() << "."
                       << m_nodes[con1->m_nodeIndex]->getNodeID()
                       << ";" << endl;
               }
               return false;
            }
            
         }
         else if((angle < 3 *M_PI_6)&& mergePrintOut)
         {
            cerr << "Not merging any more node : "
                 << REMOVE_UINT32_MSB(m_nodes[0]->getNodeID())<< ", a: "
                 << angle << endl;
         }
         float64 angle3 =  con1->m_angle - con2->m_angle;
         if ((angle3 < 5 * M_PI_6) && (angle3 > - 5 * M_PI_6))
         {
            if(m_printOut)
            {
               cerr << "Conn elements not merged anymore ! - node:" 
                    << hex << m_nodes[0]->getNodeID() << dec << endl;
               cerr << "angle1 : " << con1->m_endAngle << " angle2 : "
                    <<  con2->m_endAngle << endl;
            }
         }
         else
         {
            if(m_printOut)
            {
               cerr << "No merge ! :" << endl;
               cerr << "angle1 : " << con1->m_endAngle << " angle2 : "
                    <<  con2->m_endAngle << " diff : " << angle<< endl;
            }
            
         }
         
      }
   }
   return false;
}
/*
bool
Crossing::sameCon(CrossingConnectionElement* con1,
                  CrossingConnectionElement* con2)
{
   bool mergePrintOut = false;
   if ((con1->m_ismulti && con2->m_ismulti)
       // && (nameSame(con1->m_nameIndex, con2->m_nameIndex)) --v- moved
       )
   {
      bool h = m_map->driveOnRightSide();
      float64 angle =  con2->m_endAngle - con1->m_endAngle;
      if (angle < 0)
         angle += M_PI*2;
      if (angle > M_PI*2)
         angle -= M_PI*2;      

      if (((con1->m_passNodeRestrictions == ItemTypes::noWay)&& h &&
          (con2->m_passNodeRestrictions != ItemTypes::noWay)) ||
          ((con1->m_passNodeRestrictions != ItemTypes::noWay)&& !h &&
           (con2->m_passNodeRestrictions == ItemTypes::noWay)))
      {
         if (((angle  < 2.6 *M_PI_6)) && true)
         {
            if(nameSame(con1->m_nameIndex, con2->m_nameIndex)){
               if(m_printOut)
               {
                  cerr << "ANGLE : " << angle << " MEARGING !!! "  << endl;
                  cerr << "Conn elements merged " << endl;
               }
               if(mergePrintOut)
               {
                  cerr << "Merge ! : node : "
                       << REMOVE_UINT32_MSB(m_nodes[0]->getNodeID())<< ", a: "
                       << angle << endl;
               }
               return true;
            } else {
               // cerr << " Name differs no mearge !" << endl;
               if(SUSPECT_NAME_DUMP &&
                  (m_numberOfNodes == 3)){
                  cout << "NODE: " << m_map->getMapID() << "."
                       << m_nodes[con1->m_nodeIndex]->getNodeID()
                       << ";" << endl;
               }
               return false;
            }
            
         }
         else if((angle < 3 *M_PI_6)&& mergePrintOut)
         {
            cerr << "Not merging any more node : "
                 << REMOVE_UINT32_MSB(m_nodes[0]->getNodeID())<< ", a: "
                 << angle << endl;
         }
         float64 angle2 =  con1->m_angle - con2->m_angle;
         if ((angle2 < 5 * M_PI_6) && (angle2 > - 5 * M_PI_6))
         {
            if(m_printOut)
            {
               cerr << "Conn elements not merged anymore ! - node:" 
                    << hex << m_nodes[0]->getNodeID() << dec << endl;
               cerr << "angle1 : " << con1->m_endAngle << " angle2 : "
                    <<  con2->m_endAngle << endl;
            }
         }
         else
         {
            if(m_printOut)
            {
               cerr << "No merge ! :" << endl;
               cerr << "angle1 : " << con1->m_endAngle << " angle2 : "
                    <<  con2->m_endAngle << " diff : " << angle<< endl;
            }
            
         }
      }
   }
   return false;
}*/

CrossingConnectionElement*
Crossing::getCrossingConnection(OldNode* node)
{
   int i = 0;
   while((i < m_numberOfNodes - 1) && (m_nodes[i] != node))
      i++;
   if (m_nodes[i] != node)
      return NULL;
   return (CrossingConnectionElement*) m_conNotices[m_transfV[i]];
}

void
Crossing::getLatLon(GfxData* gfx, bool isNode0,
                    int32& lat1, int32& lon1, int32& lat2, int32& lon2)
{
   MC2_ASSERT(gfx->getNbrPolygons() == 1);
   uint32 nbrPoints = gfx->getNbrCoordinates(0);
   if (nbrPoints < 2 ) {
      if ( nbrPoints > 0 ) {
         cout << " coord 0 " << gfx->getLat(0,0) << "," 
              << gfx->getLon(0,0) << endl;
      }
      PANIC("setlatLon()", "nbrPoints on streetSegment < 2");
   }

   if ( isNode0 ) {
      lat1 = gfx->getLat(0,0);
      lon1 = gfx->getLon(0,0);
      lat2 = gfx->getLat(0,1);
      lon2 = gfx->getLon(0,1);
   } else {
      lat1 = gfx->getLat(0,nbrPoints-1);
      lon1 = gfx->getLon(0,nbrPoints-1);
      lat2 = gfx->getLat(0,nbrPoints-2);
      lon2 = gfx->getLon(0,nbrPoints-2);
   }
}

void
Crossing::getNodeCoords(GfxData* gfx, bool isNode0,
                        int32& lat1, int32& lon1, int32& lat2, int32& lon2)
{
   MC2_ASSERT(gfx->getNbrPolygons() == 1);
   uint32 nbrPoints = gfx->getNbrCoordinates(0);
   if (nbrPoints < 2 ) {
      if ( nbrPoints > 0 ) {
         cout << " coord 0 " << gfx->getLat(0,0) << "," 
              << gfx->getLon(0,0) << endl;
      }
      PANIC("setlatLon()", "nbrPoints on streetSegment < 2");
   }
   if ( isNode0 ) {
      lat1 = gfx->getLat(0,0);
      lon1 = gfx->getLon(0,0);
      lat2 = gfx->getLat(0,nbrPoints-1);
      lon2 = gfx->getLon(0,nbrPoints-1);
   } else {
      lat1 = gfx->getLat(0,nbrPoints-1);
      lon1 = gfx->getLon(0,nbrPoints-1);
      lat2 = gfx->getLat(0,0);
      lon2 = gfx->getLon(0,0);
   }
}

float64
Crossing::getAdjustedAngle(GfxData* gfx, bool isNode0, bool printOut)
{
   MC2_ASSERT(gfx->getNbrPolygons() == 1);
   uint32 nbrPoints = gfx->getNbrCoordinates(0);
   if (nbrPoints < 2 ) {
      if ( nbrPoints > 0 ) {
         cout << " coord 0 " << gfx->getLat(0,0) << "," 
              << gfx->getLon(0,0) << endl;
      }
      PANIC("setlatLon()", "nbrPoints on streetSegment < 2");
   }
   int32 lat1, lon1, lat2, lon2;
   int32 lat3 = 0; // To make compiler happy.
   int32 lon3 = 0; // To make compiler happy.
   
   if ( isNode0 ) {
      lat1 = gfx->getLat(0,0);
      lon1 = gfx->getLon(0,0);
      lat2 = gfx->getLat(0,1);
      lon2 = gfx->getLon(0,1);
      if(nbrPoints > 2 ){
         lat3 = gfx->getLat(0,2);
         lon3 = gfx->getLon(0,2);
      }
   } else {
      lat1 = gfx->getLat(0,nbrPoints-1);
      lon1 = gfx->getLon(0,nbrPoints-1);
      lat2 = gfx->getLat(0,nbrPoints-2);
      lon2 = gfx->getLon(0,nbrPoints-2);
      if(nbrPoints > 2 ){
         lat3 = gfx->getLat(0,nbrPoints-3);
         lon3 = gfx->getLon(0,nbrPoints-3);         
      }
   }
   float64 angle1 = GfxUtility::getAngleFromNorth(lat1, lon1,
                                                  lat2, lon2);
   // Let's see if this angle was suspect.
   if(nbrPoints ==  2 ){
      return angle1;   // (nope!)
   } else {
      float64 angle2 = GfxUtility::getAngleFromNorth(lat2, lon2,
                                                     lat3, lon3);
      float64 aDiff = angle1 - angle2;
      if(aDiff < - M_PI){
         aDiff += M_PI* 2;
      } else if (aDiff > M_PI){
         aDiff -= M_PI* 2;
      } 
      
      float64 dist1 = sqrt(
         GfxUtility::squareP2Pdistance_linear(lat1,lon1,lat2,lon2));
      float64 dist2 = sqrt(
         GfxUtility::squareP2Pdistance_linear(lat2,lon2,lat3,lon3));
      
      if(((dist1 < 5) && (dist2 > 3* dist1) && 
         ((aDiff > M_PI_2) || (aDiff < - M_PI_2)))||
         ((dist1 < 5) && (dist2 > 20* dist1) &&
          ((aDiff > M_PI_4) || (aDiff < - M_PI_4))) ||
           ((dist1 < 2) && (dist2 > 20* dist1) &&
            ((aDiff > M_PI_6) || (aDiff < - M_PI_6)))
         ){
         if(printOut){
         cerr << " Changing angle, dist : " << dist1
              << " diff :  " << aDiff << endl;
         cerr << " lat1,lon1 :" << lat1 << "," << lon1
              << " lat2,lon2 :" << lat2 << "," << lon2 << endl;
         }
         return angle2; // (yep!)
      }
      return angle1;   // (nope!)
   }
}


CrossingConnectionElement*
Crossing::getCrossingConnection(OldConnection* conn)
{
   uint32 fromNodeId = conn->getConnectFromNode();
   uint32 compNodeId = TOGGLE_UINT32_MSB(fromNodeId);
   OldNode* compNode = m_map->nodeLookup(compNodeId);
   int i = 0;
   while((i < m_numberOfNodes-1) && (m_nodes[i] != compNode))
      i++;
   if (m_nodes[i] != compNode)
      return NULL;
   return (CrossingConnectionElement*) m_conNotices[m_transfV[i]];
}

byte
Crossing::getCrossingConnectionIndex(CrossingConnectionElement* cConn)
{
   if(cConn == NULL)
      return MAX_BYTE;
   byte i = 0;
   while ((i<  m_conNotices.getSize()) && (cConn != m_conNotices[i]))
      i++;
   if(cConn != m_conNotices[i])
      return MAX_BYTE;
   return i;
}

bool
Crossing::setTurnDescriptions()
{
   bool setOk = true;
   turndescvector_t turnDescriptons;
   ItemTypes::crossingkind_t crossingKind;
   // Check to se if a virtual segment is present if more than 2 ways.
   if(m_conNotices.getSize() > 2)
   {
      for(byte i = 0 ; i < m_numberOfNodes ; i++){
         OldRouteableItem* ri = static_cast<OldRouteableItem*>
            (m_map->itemLookup(m_nodes[i]->getNodeID()));
         if(ri->getGfxData()->getLength(0) == 0)
         {
            if(m_printOut)
               cerr << "Null length segment in crossing, aborting turndesc."
                    << endl;
            return false;
         }
      }
   }
      
   if(!getTurnDescriptions(&m_conNotices,
                           m_map->driveOnRightSide(),
                           turnDescriptons,
                           crossingKind,
                           m_map->getCountryCode(),
                           m_printOut))
   {
      if((!m_printOut) && (m_conNotices.getSize()>1))
      {
         // skriv ut nod info.
         cerr << "Failed : " << endl;
         cerr << "Size : " << m_conNotices.getSize() << endl;
         cerr << "OldNodes: " << m_numberOfNodes << endl;
         cerr << "OldNode [0] : "<< hex <<  m_nodes[0] << dec << endl;
         cerr << " Trying with debug" << endl;
         setOk = getTurnDescriptions(&m_conNotices,
                                     m_map->driveOnRightSide(),
                                     turnDescriptons,
                                     crossingKind,
                                     m_map->getCountryCode(),
                                     true);
         
      }
      else
         setOk = false;
   }
   else
      setOk = true;
   if(setOk)
   {
      if(m_conNotices.getSize() >= CROSSING_DUMP_SIZE ){   
         cout << "NODE: " << m_map->getMapID() << "."
              << m_nodes[0]->getNodeID() << ";" << endl;
      }
      OldNode* deadEndNode = NULL;
      if((crossingKind == ItemTypes::CROSSING_4WAYS) &&
         isPartOfMultidigCrossing()){
         crossingKind = getMultidigCrossingKind(deadEndNode);
      }
      
      if(m_printOut)
         cerr << "conNotices.Size : " << m_conNotices.getSize() << endl;
      if(((int)crossingKind > 15)||((int)crossingKind < 0))
         cerr <<"CrossingKind Out of bounds : " << (int)crossingKind
              << endl;
      for(int i = 0; i < m_numberOfNodes; i++)
      {
         bool printTransf = false; //(m_nodes[i]->getNodeID() == 805312308);
         byte toConIndex = getCrossingConnectionIndex(
            getCrossingConnection(m_nodes[i]));
         if (printTransf|| m_printOut)
            cerr << "toCon index : " << (int)toConIndex << endl;
         static_cast<GMSNode*>(m_nodes[i])->setTurnSet();
         for(uint16 j = 0; j < m_nodes[i]->getNbrConnections(); j++)
         {
            // Check that the connection comes from a SSI!
            if(!m_nodes[i]->getEntryConnection(j)->isMultiConnection()){
               m_nodes[i]->getEntryConnection(j)->
                  setCrossingKind(crossingKind);
               byte fromConIndex = getCrossingConnectionIndex(
                  getCrossingConnection(m_nodes[i]->getEntryConnection(j)));
               if((fromConIndex>10) && m_printOut)
                  cerr << "fromConIndex = " << (int)fromConIndex << " j = "
                       << j << endl;
               if (printTransf|| m_printOut)
                  cerr << "  fromCon index : " << (int)fromConIndex << endl;
               if(toConIndex == fromConIndex)
               {
                  m_nodes[i]->getEntryConnection(j)->
                     setTurnDirection(ItemTypes::UTURN);
                  m_nodes[i]->getEntryConnection(j)->
                     setExitCount(1);
                  if (printTransf|| m_printOut)
                     cerr << "U-turn" << endl;
               }
               else // not U-turn
               {  
                  bool notFound = true;
                  turndescvector_t::iterator ti = turnDescriptons.begin();
                  while((notFound) &&(ti != turnDescriptons.end()))
                  {
                     if((ti->toConIndex == toConIndex) &&
                        (ti->fromConIndex == fromConIndex))
                     {
                        // Found the turndescription.
                        notFound = false;
                        m_nodes[i]->getEntryConnection(j)->
                           setTurnDirection(ti->description);
                        if(deadEndNode != m_nodes[i])
                           m_nodes[i]->getEntryConnection(j)->
                              setExitCount(ti->exitCount);
                        if((ti->debug)&& SUSPECT_TURN_DUMP){
                           // If the connection is allowed dump.
                           OldConnection* conn = m_nodes[i]->
                              getEntryConnection(j);
                           
                           if((conn->isVehicleAllowed(ItemTypes::passengerCar)
                               && conn->getVehicleRestrictions() != 0x1080)){
                              //uint32 nodeID = conn->getConnectFromNode();
                              //OldNode* node =  m_map->nodeLookup(nodeID);
                              //if(node->getEntryRestrictions() !=
                              //   ItemTypes::noWay){
                              
                              cout << "TURNDESCRIPTION: " << m_map->getMapID()
                                   << "." << m_nodes[i]->
                                 getEntryConnection(j)->getConnectFromNode()
                                   << ";" << m_map->getMapID() << "."
                                   << m_nodes[i]->getNodeID()
                                   << ";" << int(ti->description) << ";"
                                   << int(crossingKind ) << ";" << endl;
                              //}
                           
                           }
                        } else if((ti->debug)&&( TURNDESCRIPTION_DUMP)){
                           cout << "TURNDESCRIPTION: " << m_map->getMapID()
                                << "." << m_nodes[i]->
                              getEntryConnection(j)->getConnectFromNode()
                                << ";" << m_map->getMapID() << "."
                                << m_nodes[i]->getNodeID()
                                << ";" << int(ti->description) << ";"
                                << int(crossingKind ) << ";" << endl;
                        
                        }
                        if(printTransf|| m_printOut)
                           cerr << " [" << (int)ti->toConIndex<<","
                                << (int)ti->fromConIndex << "] , desc:"
                                << (int)ti->description ;
                     
                     }
                     ti++;
                  }
                  if (printTransf|| m_printOut)
                     cerr << endl;
                  if(notFound)
                  {
                     if(m_printOut)
                        cerr << " CONNECTION NOT FOUND TO NODE : "
                          << " (Is it from a ssi ?)" << endl;
/*
  else
  cerr << " connection not found to node : "
                          << " (Is it from a ssi ?)" << endl;

                  if(NO_CONNECTION_DUMP){
                     cout << "NODE: " << m_map->getMapID() << "."
                          << m_nodes[i]->getNodeID() << ";" << endl;
                  }
                  cerr  << m_nodes[i]->getNodeID() << " , nodes : "
                        << (int)m_numberOfNodes << " , ConnE : "
                        << m_conNotices.getSize() << " [" << (int)toConIndex
                        << ","<< (int)fromConIndex << "]" ;
                  cerr << " Transf : " ;
                  for(int k = 0 ; k < m_numberOfNodes; k++)
                     cerr << (int)m_transfV[k]<< " " ;
                  cerr << endl;
                  ti = turnDescriptons.begin();
                  while((ti != turnDescriptons.end()))
                  {
                     cerr << " [" << (int)ti->toConIndex<<","
                          << (int)ti->fromConIndex << "] " ;
                     ti++;
                  }
                  cerr << endl;
*/
                  }  
               }     
            }
         }     
      }
   }
   return setOk;
}


bool
Crossing::getTurnDescriptions(ObjVector* conNotices, 
                              bool rSide,
                              turndescvector_t& result,
                              ItemTypes::crossingkind_t& kind,
                              StringTable::countryCode countryCode,
                              bool printOut)
{
   int size = conNotices->getSize();
   if(size < 2)
   {
      return false;
   }
   // Find out how much road class may differ between two forks in a
   // bifurcating 3-way crossing for setting keep left/keep right 
   // (if no country code is given, the default value 0 is returned)
   const int rcDiffForBifurc =
      NationalProperties::getMaxRoadClassDiffForBifurc( countryCode );
   // Crossing initializations
   kind                 = ItemTypes::UNDEFINED_CROSSING;
   bool tcrossing       =  true;
   bool rampAccess      = false;
   int  leftRight       =     0;
   bool noRoundabout    =  true;
   bool noRoundaboutish =  true;
   conNotices->sort();
   int nbrRbish         =     0;

   if (printOut) {
      cerr << "rcDiffForBifurc = " << rcDiffForBifurc << endl ;
   }

   for(int i = 0; i < size; i ++)
   {
      if(static_cast<CrossingConnectionElement*>(conNotices->getElementAt(i))
         ->m_roundabout){
         if(printOut){
            cerr << "Part of roundaboutish traffic figure. " << endl;
         }
         
         noRoundabout = false;
      }
      if(static_cast<CrossingConnectionElement*>(conNotices->getElementAt(i))
          ->m_roundaboutish){
         if(printOut){
            cerr << "Part of roundaboutish traffic figure. " << endl;
         }
         noRoundaboutish = false;
         nbrRbish += 1;
      }
      
   }

   if ( printOut ) {
      cerr << "Start getting turn descriptions." << endl;
   }

   for(int i = 0; i < size; i ++)
   {
      // Fix the attributes for the first CrossingConnectionElement
      CrossingConnectionElement* thisElement =
         static_cast<CrossingConnectionElement*>(conNotices->getElementAt(i));
      float64 angle0  = thisElement->m_angle;
      byte roadClass0 = thisElement->m_roadClass;
      bool ramp0      = thisElement->m_ramp;
      bool noEntry0   = (thisElement->m_passNodeRestrictions ==
         ItemTypes::noWay);
      //bool multDig0   = thisElement->m_ismulti && !thisElement->m_isCtrAcc;
      bool ctrlA0     = thisElement->m_isCtrAcc;
      
      if(noRoundabout && noRoundaboutish)
      {
         if (printOut) {
            cerr << "------------- " << i << " ------------" << endl;
            cerr << size << "-way crossing " << endl;
         }

         switch (size)
         {
            // Some notes on turndescriptions in a crossing.
            //
            // All the turns set here are FROM the item in question.
            // (They are later set into the right TO-connection.)
            // Therfore only 1 AHEAD is possible.
            //
            // When the traffic runs both ways KEEP -descriptions crossing 
            // the opposing lane should be avoided. (maybe on rc4)
            //
            
            case 2 :
            {
               // No real crossing. The method can be compleded here.
               // We still have to check angles to see if its a turn.
               // Later use FOLLOW_RIGHT & FOLLOW_LEFT.
               
               turndesc_t* firstDesc    = new turndesc_t;
               turndesc_t* secondDesc   = new turndesc_t;

               int first  = (i+1)% size;
               CrossingConnectionElement* secondElement =
                  static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(first));
               float64 angle = secondElement->m_angle - angle0;
               if (angle < 0)
                  angle += 2* M_PI;
               bool sameName = nameSame(thisElement->m_nameIndex,
                                        secondElement->m_nameIndex);
               bool noName = ((thisElement->m_nameIndex.getSize() == 0 )||
                              (secondElement->m_nameIndex.getSize() == 0 ));
               firstDesc->fromConIndex  = 0;
               firstDesc->toConIndex    = 1;
               firstDesc->debug         = false;
               if((angle < 4.1*M_PI_6) && !sameName && !noName)
               {
                  firstDesc->description   = ItemTypes::LEFT;
                  secondDesc->description  = ItemTypes::RIGHT;
               }
               else if((angle > 7.9*M_PI_6)&& !sameName && !noName)
               {
                  firstDesc->description   = ItemTypes::RIGHT;
                  secondDesc->description  = ItemTypes::LEFT;
               }
               else
               {
                  firstDesc->description   = ItemTypes::FOLLOWROAD;
                  secondDesc->description  = ItemTypes::FOLLOWROAD;
               }
               firstDesc->exitCount     = 1;
               secondDesc->fromConIndex = 1;
               secondDesc->toConIndex   = 0;
               secondDesc->exitCount    = 1;
               secondDesc->debug         = false;
               kind = ItemTypes::NO_CROSSING;
               result.push_back(*firstDesc);
               result.push_back(*secondDesc);
               delete firstDesc;
               delete secondDesc;
               // All return parameters completed we can:
               return true;
            }
            break;
            case 3 :
            {
               // Threeway crossing.
               // Inits.
               int first  = (i+1)% size;
               int second = (i+2)% size;
               float64 angle1 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(first))->m_angle - angle0;
               if (angle1 < 0)
                  angle1 += 2* M_PI;
               float64 angle2 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(second))->m_angle - angle0;
               if (angle2 < 0)
                  angle2 += 2 *M_PI;
               byte roadClass1 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(first))->m_roadClass;
               bool ramp1 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(first))->m_ramp;
               bool noEntry1 = (static_cast<CrossingConnectionElement*>
                    (conNotices->getElementAt(first))
                                ->m_passNodeRestrictions !=
                                ItemTypes::noRestrictions);
               bool ctrlA1  = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(first))->m_isCtrAcc;

               bool bifurc1 = (static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(first))->m_junctionType ==
                               ItemTypes::bifurcation);
               if(noEntry1)
                  roadClass1 += 2;
               byte roadClass2 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(second))->m_roadClass;
               bool ramp2 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(second))->m_ramp;
               bool noEntry2 =
                  (static_cast<CrossingConnectionElement*>
                   (conNotices->getElementAt(second))->m_passNodeRestrictions
                   != ItemTypes::noRestrictions);
               bool ctrlA2  = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(second))->m_isCtrAcc;
               bool bifurc2 = ((static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(second)))->m_junctionType ==
                               ItemTypes::bifurcation);

               if(noEntry2)
                  roadClass2 +=2;
               if (printOut)
               {
                  cerr <<"Road classes: " << (int)roadClass0 << ", " << (int)roadClass1 << ", "
                       << (int)roadClass2 << endl;
               }

               bool sameName1 = false;
               bool sameName2 = false;
               if(thisElement->m_nameIndex.getSize() != 0){
                  sameName1 =
                     nameSame(thisElement->m_nameIndex,
                              static_cast<CrossingConnectionElement*>
                              (conNotices->getElementAt(first))->m_nameIndex);
                  sameName2 =
                     nameSame(thisElement->m_nameIndex,
                              static_cast<CrossingConnectionElement*>
                              (conNotices->getElementAt(second))->m_nameIndex);
               }
               if(printOut){
                  cerr << "angle1 " << angle1 << ", angle2 " << angle2 << endl;
               }
               
               if(printOut && sameName1)
                     cerr << "First connection same name : ";
               
               if(printOut && sameName2)
                     cerr <<  "Second connection same name : ";

               if(printOut){
                  if(bifurc1)
                     cerr << "BIFURK 1 " ;
                  if(bifurc2)
                     cerr << "BIFURK 2" ;
                  if(bifurc1 || bifurc2)
                     cerr << endl;
               }
               
               
               // Create the turnstructs.
               turndesc_t* firstDesc  = new turndesc_t;
               firstDesc->fromConIndex = i;
               firstDesc->toConIndex = byte(first);
               firstDesc->debug = false;
               turndesc_t* secondDesc = new turndesc_t;
               secondDesc->fromConIndex = i;
               secondDesc->toConIndex = byte(second);
               secondDesc->debug = false;
               
               bool bifurcJunction = false;
               
               // First exit. ---------------------------------
               // Keep_Right is not possible.



               // Controlled access.   ( Maybe from ramps as well?)
               // KEEP_LEFT  is much more probable.
               if( ctrlA1 && ctrlA2 &&
                  (angle1 > 4 * M_PI_6) &&
                  (angle2 < 8*M_PI_6) &&
                  ((angle2-angle1) < 3*(angle2-M_PI))
                  ){
                  firstDesc->description = ItemTypes::KEEP_LEFT;
                  firstDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "Keep_Left(ctrlA), ";
                  firstDesc->debug       = false;
               }
               
               // Bifurcation crossing, KEEP_LEFT is much more probable.
               else if(bifurc1 && bifurc2 &&
                       ((roadClass1 < 3) || (roadClass2 < 3)) && 
                       (angle1 > 4 * M_PI_6) &&
                       (angle2 < 8*M_PI_6)  && noEntry0 
                       //((angle2-angle1) < 3*(angle2-M_PI)) 
                       ){
                  firstDesc->description = ItemTypes::KEEP_LEFT; 
                  firstDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "Keep_Left(bifurc), "; 
                  firstDesc->debug       = false;
                  bifurcJunction         = true; 
                  tcrossing              = false; 
                  
               } 
               
               // At ramp
               // KEEP_LEFT  is much more probable.
               else if(ramp0 && !noEntry1 && !noEntry2 && noEntry0 &&
                       (roadClass1+2 > roadClass2) &&
                       (roadClass2+2 > roadClass1) &&
                       (angle1 > 4 * M_PI_6) && (angle2 < 8*M_PI_6) &&
                       ((angle2-angle1 < 3*M_PI_6) ||
                        ((M_PI > angle1)&&(2*(M_PI-angle1) > (angle2-M_PI)))||
                        ((M_PI< angle2)&& (2*(angle2-M_PI)> (M_PI-angle1))))
                       ){
                  firstDesc->description = ItemTypes::KEEP_LEFT;
                  firstDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "Keep_Left(ramps), ";
                  firstDesc->debug       = false;                 
               }
               // Special end-of-ramp situation.
               // After exiting a ramp
               // Ahead
               else if((ramp0 && !ramp1 && !ramp2) &&
                       (roadClass1 == roadClass2) &&
                       (angle1 > 4.5 * M_PI_6) &&(angle1 < 6.5 * M_PI_6) &&
                       (angle2-angle1 < M_PI_6) && noEntry0 && !noEntry1 &&
                       !noEntry2 && rSide){
                  firstDesc->description = ItemTypes::AHEAD;
                  firstDesc->exitCount   = 1;
                  printOut = true;
                  if(printOut)
                     cerr << "Ahead(ramp end), ";
                  firstDesc->debug       = false;
               }
               
               // Special end-of-ramp situation. Left side traffic.
               // After exiting a ramp
               // Ahead
               /*   Must test!
               else if((ramp0 && !ramp1 && !ramp2) &&
                       (roadClass1 == roadClass2) && 
                       (angle1 > 4.5 * M_PI_6) &&(angle1 < 6.5 * M_PI_6) && 
                       (angle2-angle1 < M_PI_6) && noEntry0 && !noEntry1 && 
                       !noEntry2 && !rSide){ 
                  firstDesc->description = ItemTypes::LEFT; 
                  firstDesc->exitCount   = 1;
                  printOut = true; 
                  if(printOut)
                     cerr << "Left(ramp end), "; 
                  firstDesc->debug       = true; 
               }
               */
               // Ahead on a much larger road.
               else if((angle1 > 3 *M_PI_6) && (angle1 < 9*M_PI_6) &&
                       (roadClass1 == roadClass0) &&
                       ((!noEntry2 && (roadClass1 +3 < roadClass2)) ||
                        (roadClass1 +5 < roadClass2))){
                  firstDesc->description = ItemTypes::AHEAD;
                  firstDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "Ahead(larger road), ";
                  firstDesc->debug       = false;                  
               }
               // Left from a much larger road.
               else if((angle2 > 3 *M_PI_6) && (angle2 < 9*M_PI_6) &&
                       (roadClass2 == roadClass0) &&
                       ((!noEntry1 && (roadClass2 +3 < roadClass1)) ||
                        (roadClass2 +5 < roadClass1))){
                  firstDesc->description = ItemTypes::LEFT;
                  firstDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "Left(of larger road), ";
                  firstDesc->debug       = false; 
               }
               // Keep_Left,
               // Less than 60 deg turn. Less than 60 between options.
               // Almost same roadClass and oneway if rC is less than 2.
               // ( (max(rc)=4 + rc equal) || (max(rc)<4 + abs(rc)<rcdiff) )
               else if((angle1 > 4.5 *M_PI_6) && (angle2-angle1 < 2*M_PI_6) &&
                       ( (ramp1 && ramp2) ||
                         ((MAX(roadClass1,roadClass2)>=4) &&
                          (roadClass1==roadClass2)) ||
                         ((MAX(roadClass1,roadClass2)<4) &&
                          (abs(int(roadClass1) - int(roadClass2))<=rcDiffForBifurc)) ) &&
                       ((ramp1 == ramp2) || noEntry0) &&
                       (noEntry1 == noEntry2) && 
                       (noEntry0 || (roadClass1 > 3) || (ramp1 && ramp2)) &&
                       ((angle2-angle1 < 0.5*M_PI_6) ||
                        ((angle1 < M_PI) &&
                         (((angle2<M_PI) &&
                           ((M_PI-angle1) > 2*(M_PI-angle2))) ||
                          ((angle2 > M_PI) && ((M_PI-angle1) >2*(angle2-M_PI))&&
                           (2*(M_PI-angle1) <(angle2-M_PI))))))
                  )
                  
               {
                  firstDesc->description = ItemTypes::KEEP_LEFT;
                  firstDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "Keep_Left, ";
               }
               
               // Smaller road connects with a larger.
               else if ((roadClass1 == roadClass2)&&
                        ((roadClass0 > roadClass1) ||
                         (ramp0 && !ramp1 && !ramp2)) &&
                        (angle1 < 5*M_PI_6) &&
                        ((angle2 - angle1) < 7 *M_PI_6) &&
                        ((angle2 - angle1) > 5 *M_PI_6)
                        )
               {
                  firstDesc->description = ItemTypes::LEFT;
                  firstDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "Left(sp), ";
               }
               
               // Right, (right)
               // more than 60 deg to right and 90 deg if larger road.
               else if((angle1> 8.5*M_PI_6) &&
                       ((angle1> 9*M_PI_6) ||(roadClass2 <= roadClass1)))
               {
                  firstDesc->description = ItemTypes::RIGHT;
                  firstDesc->exitCount   = 2;
                  if(printOut)
                     cerr << "Right, ";
               }
               
               // Ahead,
               else if((angle1 > 4.5*M_PI_6) && (angle1 < 8 *M_PI_2) &&
                       ((!noEntry1 ||(angle2> 7*M_PI_6)|| noEntry2)) &&
                       (((angle2-angle1)< 5.9*M_PI_6) ||
                        ((angle2-angle1)> 6.1*M_PI_6) ||
                        (angle1 > 5*M_PI_6)||
                        (roadClass1 != roadClass2))  &&
                       //Roads same size.
                       (((roadClass1 == roadClass2) &&
                        (((angle1<M_PI) && (angle2>M_PI) &&
                          (2*(M_PI-angle1) < (angle2-M_PI))) ||
                         (angle1 >= M_PI)))
                        || // Lesser road ahead.
                        ((roadClass1 > roadClass2) &&(angle2>M_PI)&&
                         (roadClass1 <= (roadClass2 +3 )) && 
                         (((angle1<=M_PI)&&(3*(M_PI-angle1)<(angle2-M_PI)))||
                          ((angle1>M_PI)&&(2*(angle1-M_PI)<(angle2-M_PI)))))
                        
                        || //Much larger road ahead.
                        ((((roadClass1 +2) < roadClass2)&& !noEntry2 ) ||
                         ((roadClass1 +3) < roadClass2))
                        || // Larger road ahead.
                        ((roadClass1 < roadClass2) &&
                         
                         ((angle1>M_PI) ||
                          (((angle2< M_PI)&&((M_PI-angle1)<2*(M_PI-angle2)))||
                           ((angle2>M_PI)&&((M_PI-angle1)<3*(angle2-M_PI))))))
                        ))
               {
                  firstDesc->description = ItemTypes::AHEAD;
                  firstDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "Ahead, ";
               }
               // Ahead because of same road name.
               else if(!noEntry1 && (roadClass1 == roadClass2) && !ramp1 &&
                       !ramp2 && sameName1 && !sameName2 && (roadClass1 >2)&&
                       (angle1 > 4.5*M_PI_6) && (angle1 < 7*M_PI_6) &&
                       (angle2 > M_PI) &&
                       ((angle1 > M_PI) ||
                        ((M_PI-angle1) < (angle2- M_PI))))
               {
                  firstDesc->description = ItemTypes::AHEAD;
                  firstDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "Ahead(rn), ";                  
               }
               
               // Left,
               // Default.
               else
               {
                  firstDesc->description = ItemTypes::LEFT;
                  firstDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "Left, ";
               }
               // Second exit -------------------------------------
               // No Keep_Left possible
               // , keep_right or ahead. If first was keep_left.
               if(firstDesc->description == ItemTypes::KEEP_LEFT) {
                  // No bifurc and the left ramp is much smaller than the right.
                  // Continue ahead on larger ramp. (Keep right is just confusing since
                  // the left ramp is small)
                  if(!bifurcJunction && ramp1 && ramp2  && roadClass2 <= (roadClass1 - 3) &&
                     (angle2 - angle1) > ( M_PI_6/3 ) ) {
                     secondDesc->description = ItemTypes::AHEAD;
                     secondDesc->exitCount   = 1; 
                     if(printOut) {
                        cerr << " ahead (larger ramp)" << endl;
                     }
                  } else {
                     secondDesc->description = ItemTypes::KEEP_RIGHT;
                     secondDesc->exitCount   = 1;
                     if(printOut)
                        cerr << " keep_right." << endl;
                  }
               }
               
               // Smaller road (or ramp)connects with a larger.
               else if ((roadClass1 == roadClass2)&&
                        ((roadClass0 > roadClass1)||
                         (ramp0 && !ramp1 && !ramp2)) &&
                        (angle2 > 7*M_PI_6) &&
                        ((angle2 - angle1) < 7 *M_PI_6) &&
                        ((angle2 - angle1) > 5 *M_PI_6) 
                        )
               {
                  secondDesc->description = ItemTypes::RIGHT;
                  secondDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "right(sp)" << endl;
               }
               
               
               // , ahead.
               // First must have been to the left 
               // 
               else if((firstDesc->description == ItemTypes::LEFT) &&
                       (angle2> 4*M_PI_6)&&(angle2< 8*M_PI_6)&&
                       // Roads same size. (angle1 <M_PI)
                       (((roadClass2 == roadClass1) &&
                         ((angle2<=M_PI) ||
                          (2*(angle2-M_PI)<(M_PI-angle1))))
                        || //Lesser road ahead.(angle1 <M_PI)
                        ((roadClass2 > roadClass1) &&
                         ((angle2<=M_PI) || (3*(angle2-M_PI)<(M_PI-angle1))))
                        || //Major road ahead. or same name.
                        (roadClass2 < roadClass1)
                        ))
               {
                  secondDesc->description = ItemTypes::AHEAD;
                  secondDesc->exitCount   = 1;
                  if(roadClass2 == roadClass1)
                     secondDesc->debug       = false;
                  if(printOut)
                     cerr << " ahead." << endl;
               }
               // , ahead
               // Ahead because of same road name.
               else if(!noEntry2 && (roadClass1 == roadClass2) && !ramp1 &&
                       !ramp2 && !sameName1 && sameName2 &&(roadClass1 > 2)&&
                       (angle2> 5*M_PI_6)&&(angle2< 7*M_PI_6)&&
                       (angle1 < M_PI) &&
                       ((angle2 < M_PI) ||
                        ((angle2 - M_PI) < 1.3*(M_PI-angle1))) &&
                       (firstDesc->description == ItemTypes::LEFT)){
                  secondDesc->description = ItemTypes::AHEAD;
                  secondDesc->exitCount   = 1;
                  secondDesc->debug       = false;
                  if(printOut)
                     cerr << " ahead(rn)." << endl;
               }

               // , ahead.
               // Ahead because of much larger road.
               else if((angle2 > 3 *M_PI_6) && (angle2 < 9*M_PI_6) &&
                       (roadClass2 == roadClass0) &&
                       ((!noEntry1 && (roadClass2 +3 < roadClass1)) ||
                        (roadClass2 +5 < roadClass1))){
                  secondDesc->description = ItemTypes::AHEAD;
                  secondDesc->exitCount   = 1;
                  secondDesc->debug       = false;
                  if(printOut)
                     cerr << " ahead(larger road)." << endl;
               }
               
               // (Left,) left.
               // If first was left turn and is more than 60 deg and more
               // than 90 deg if larger road.
               else if((firstDesc->description == ItemTypes::LEFT) &&
                  (angle2 < 4*M_PI_6) &&
                  ((roadClass1 <= roadClass2) || (angle2 < M_PI_2))) 
               {
                  secondDesc->description = ItemTypes::LEFT;
                  secondDesc->exitCount   = 2;
                  //secondDesc->debug       = true;
                  if(printOut)
                     cerr << " left." << endl;
               }

               // , right
               // Default.
               else 
               {
                  secondDesc->description = ItemTypes::RIGHT;
                  secondDesc->exitCount   = 1;
                  if(printOut){
                     cerr << " right." << endl;
                  }
                  
               }

               
               // If ramps are present, could be ON/OFF-RAMP.
               // On/off ramps to the "wrong" side is allowed on roadClass0.
               // The connecting angle must be close to PI and the road cant
               // be to small (then its probably the other side of the ramp)
               // the nonramp-nonramp angle must also be small.

               // Temp fix ! off ramps on the "wrong" side is replaced with
               // Keep_left / Keep_right
               if(!bifurcJunction &&
                  !ramp0 && !ramp1 && ramp2 && (rSide||(roadClass0 == 0)) &&
                  (roadClass0 < 4) && (angle2 < 8* M_PI_6) &&
                  (angle1 > 5*M_PI_6)&&(angle1 <7*M_PI_6))
               {
                  firstDesc->description = ItemTypes::FOLLOWROAD;
                  tcrossing = false;
                  rampAccess = true;
                  firstDesc->debug   = false;
                  secondDesc->debug = false;
                  if(!rSide && !noEntry2)
                  {
                     secondDesc->description = ItemTypes::OFF_RAMP_RIGHT;
                     secondDesc->debug = true;
                     //if(printOut)
                        cerr << " OFF_RAMP_RIGHT" << endl;
                     /*
                     secondDesc->description = ItemTypes::KEEP_RIGHT;
                     if(printOut) 
                     cerr << " KEEP__RIGHT(ramp)" << endl;
                     */
                  } else {
                     secondDesc->description = ItemTypes::OFF_RAMP;
                     if(printOut)
                        cerr << " OFF_RAMP" << endl;
                  }
 
               }
               else if(!bifurcJunction &&
                       !ramp0 && ramp1 && !ramp2 &&
                       (!rSide||(roadClass0 == 0)) &&
                       (roadClass0 < 4) && (angle1 > 4* M_PI_6)&&
                       (angle2 > 5*M_PI_6)&&(angle2 <7*M_PI_6))
               {
                  secondDesc->description = ItemTypes::FOLLOWROAD;
                  firstDesc->debug   = false;
                  secondDesc->debug = false;
                  tcrossing = false;
                  rampAccess = true;
                  if(rSide && !noEntry1)
                  {
                     firstDesc->description = ItemTypes::OFF_RAMP_LEFT;
                     firstDesc->debug   = true;
                     //if(printOut)
                        cerr << " OFF_RAMP_LEFT" << endl;
                     /*
                     firstDesc->description = ItemTypes::KEEP_LEFT;
                     if(printOut) 
                     cerr << " KEEP__LEFT(ramp)" << endl;
                     */
                  } else {
                     firstDesc->description = ItemTypes::OFF_RAMP; 
                     if(printOut)
                        cerr << " OFF_RAMP" << endl;
                  }
               }
               else if(!bifurcJunction &&
                       ramp0 && !ramp1 && !ramp2 && 
                       (rSide ||(roadClass2 == 0)) &&
                       (roadClass2 < 4) &&
                       (angle2 < 8.25* M_PI_6) &&
                       (angle1 < 2.25 * M_PI_6)&&
                       (((angle2 < 7.75* M_PI_6)&&(angle1 < 1.75 * M_PI_6))
                        || (noEntry1 && noEntry0 && (roadClass2 < 4))) &&
                       ((angle2-angle1)> 5*M_PI_6) &&
                       ((angle2-angle1)< 7*M_PI_6))
               {
                  firstDesc->description = ItemTypes::LEFT;
                  secondDesc->description = ItemTypes::ON_RAMP;
                  tcrossing = false;
                  firstDesc->debug   = false;
                  secondDesc->debug = false;
                  rampAccess = true;
                  if(printOut)
                     cerr << " ON_RAMP(a)" << endl;
               }
               else if(!bifurcJunction &&
                       ramp0 && !ramp1 && !ramp2 && //noEntry2 &&
                       (!rSide||(roadClass1 == 0))&&
                       (roadClass1 < 4.5) && (angle1 > 4* M_PI_6)&&
                       ((angle2-angle1)> 5*M_PI_6) &&
                       ((angle2-angle1)< 7*M_PI_6)
                       )
               {
                  firstDesc->description = ItemTypes::ON_RAMP;
                  secondDesc->description = ItemTypes::RIGHT;
                  tcrossing = false;
                  rampAccess = true;
                  firstDesc->debug   = false;
                  secondDesc->debug = false;
                   if(printOut)
                     cerr << " ON_RAMP(b)" << endl; 
               }
               // Check if the crossing still can be a t-crossing.
               if((firstDesc->description == ItemTypes::LEFT) &&
                  (secondDesc->description == ItemTypes::RIGHT)){
                  if((angle2 - angle1) > 5 * M_PI_6)
                     leftRight++;
                  else
                     tcrossing = false;
               }
               
               else if((firstDesc->description != ItemTypes::AHEAD) &&
                       (secondDesc->description != ItemTypes::AHEAD))
                  tcrossing = false;


               // Set exitCount to MAX_BYTE to noway connections that we 
               // dont want to be counted by ExpandRouteProcessor.
               if((firstDesc->description == ItemTypes::LEFT) &&
                  noEntry1 && !noEntry2 && (angle1 < M_PI_4)){
                   if(printOut)
                     cerr << "    Setting exitCount to MAX_BYTE for left turn"
                          << endl;
                   if(!ramp1 && !ramp0)
                      firstDesc->debug       = false;
                   firstDesc->exitCount   = MAX_BYTE;
               }
               if((secondDesc->description == ItemTypes::RIGHT) &&
                  !noEntry1 && noEntry2 && (angle2 > 7*M_PI_4)){
                  if(printOut)
                     cerr << "    Setting exitCount to MAX_BYTE for right turn"
                          << endl;
                  if(!ramp2 && !ramp0)
                     secondDesc->debug       = false;
                  secondDesc->exitCount   = MAX_BYTE;
               }
               
               // Check suspect turns.
               
               // 1. Sharp angles
               if(!noEntry2 && (angle2 > 10.5 * M_PI_6) && !noEntry1 &&
                  (roadClass0 < 3) && (roadClass2 < 4) && SUSPECT_TURN_DUMP){
                  secondDesc->debug = true;
               }
               if(!noEntry1 && (angle1 < 1.5*M_PI_6) && !noEntry2 &&
                  (roadClass0 < 3) && (roadClass1 < 4) &&SUSPECT_TURN_DUMP){
                  firstDesc->debug = true;
               }

               
               // Enter structs in vector.
               result.push_back(*firstDesc);
               result.push_back(*secondDesc);
               delete firstDesc;
               delete secondDesc;
            }
            break;
            case 4 :  
            {
               // Fourway crossing
               // Inits.
               int first  = (i+1)% size;
               int second = (i+2)% size;
               int third  = (i+3)% size;
               float64 angle1 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(first))->m_angle - angle0;
               if (angle1 < 0)
                  angle1 += 2* M_PI;
               float64 angle2 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(second))->m_angle - angle0;
               if (angle2 < 0)
                  angle2 += 2 *M_PI;
               float64 angle3 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(third))->m_angle - angle0;
               if (angle3 < 0)
                  angle3 += 2 *M_PI;
               byte roadClass1 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(first))->m_roadClass;
               bool ramp1 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(first))->m_ramp;
               bool noEntry1 = (static_cast<CrossingConnectionElement*>
                    (conNotices->getElementAt(first))
                                ->m_passNodeRestrictions !=
                                ItemTypes::noRestrictions);
               byte roadClass2 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(second))->m_roadClass;
               bool ramp2 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(second))->m_ramp;
               bool noEntry2 =
                  (static_cast<CrossingConnectionElement*>
                   (conNotices->getElementAt(second))->m_passNodeRestrictions
                   != ItemTypes::noRestrictions);
               byte roadClass3 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(third))->m_roadClass;
               bool ramp3 = static_cast<CrossingConnectionElement*>
                  (conNotices->getElementAt(third))->m_ramp;
               bool noEntry3 =
                  (static_cast<CrossingConnectionElement*>
                   (conNotices->getElementAt(third))->m_passNodeRestrictions
                   != ItemTypes::noRestrictions);
              if (printOut)
               {
                  cerr << (int)roadClass0 << ", " << (int)roadClass1 << ", "
                       << (int)roadClass2 << ", " << (int)roadClass3 <<endl;
               }

              bool sameName1 = false;
              bool sameName2 = false;
              bool sameName3 = false;
              if(thisElement->m_nameIndex.getSize() != 0){
                 sameName1 =
                    nameSame(thisElement->m_nameIndex,
                             static_cast<CrossingConnectionElement*>
                             (conNotices->getElementAt(first))->m_nameIndex);
                 sameName2 =
                    nameSame(thisElement->m_nameIndex,
                             static_cast<CrossingConnectionElement*>
                             (conNotices->getElementAt(second))->m_nameIndex);
                 sameName3 =
                    nameSame(thisElement->m_nameIndex,
                             static_cast<CrossingConnectionElement*>
                             (conNotices->getElementAt(third))->m_nameIndex);
               }
               
               if(printOut && sameName1)
                     cerr << "First connection same name : ";
               
               if(printOut && sameName2)
                     cerr <<  "Second connection same name : ";

               if(printOut && sameName3)
                     cerr <<  "Third connection same name : ";

              // Create the turnstructs
              turndesc_t* firstDesc  = new turndesc_t;
              firstDesc->fromConIndex = i;
              firstDesc->toConIndex = byte(first);
              firstDesc->debug = false;
              turndesc_t* secondDesc = new turndesc_t;
              secondDesc->fromConIndex = i;
              secondDesc->toConIndex = byte(second);
              secondDesc->debug = false;
              turndesc_t* thirdDesc  = new turndesc_t;
              thirdDesc->fromConIndex = i;
              thirdDesc->toConIndex = byte(third);
              thirdDesc->debug = false;

              // First connection ------ default left --------------------
              // Keep_right not possible.
              
              // Keep_Left,
              if((roadClass1 == roadClass2) && (ramp1==ramp2) &&
                 ((angle2-angle1) < 2*M_PI_6) && (angle1>4.5*M_PI_6) &&
                 (angle2< 7.5* M_PI_6) && 
                 (((angle2-angle1) < 0.5*M_PI_6 ) ||
                 ((angle1<M_PI) && (2*(M_PI-angle1)< (angle2-M_PI)) &&
                  ((roadClass1 >2) || (ramp1)||noEntry0)))
                 )
              {
                 firstDesc->description = ItemTypes::KEEP_LEFT;
                 firstDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "K_L, ";
              }

              // Exit is to the left and to the right.
              else if((!ramp0 && ramp1 && !ramp2 && ramp3) &&
                      (ctrlA0 || !rSide) && 
                      (roadClass0 == roadClass2) &&
                      (roadClass0 <= roadClass1)&&
                      (angle1 > 5*M_PI_6)
                      )
              { 
                 firstDesc->description = ItemTypes::OFF_RAMP_LEFT;
                 firstDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "Off_r, ";
              }

              // Case with two exits on the left
              else if((!ramp0 && ramp1 && ramp2 && !ramp3) &&
                      (ctrlA0 || !rSide) && 
                      (roadClass0 == roadClass3) &&
                      (roadClass0 <= roadClass1) &&
                      (angle1 > 4.5*M_PI_6)
                      )
              {  
                 firstDesc->description = ItemTypes::OFF_RAMP_LEFT;
                 firstDesc->exitCount   = 2;
                 if(printOut)
                    cerr << "Off_r, ";
              }

              // Only on-ramp from the left.
              else if ((ramp0 && !ramp1 && ramp2 && !ramp3) &&
                       (angle1 > 5* M_PI_6) &&
                       ((angle3-angle1)>5*M_PI_6) && ((angle3-angle1)<7*M_PI_6)
                       )
              {  
                 firstDesc->description = ItemTypes::ON_RAMP;
                 firstDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "On_r, ";
              }

              // First on-ramp from the left of two possible.
              else if ((ramp0 && !ramp1 && !ramp2 && ramp3) &&
                       (angle1 > 4.5* M_PI_6) &&
                       ((angle2-angle1)>5*M_PI_6) && ((angle2-angle1)<7*M_PI_6)
                       )
              {  
                 firstDesc->description = ItemTypes::ON_RAMP;
                 firstDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "On_r, ";
              }
              
              // Right, (right,right)
              // more than 60 deg to right and 90 deg if larger road.
              else if((angle1> 8*M_PI_6) &&
                      ((angle1> 9*M_PI_6) ||(roadClass2 <= roadClass1)))
              {
                 firstDesc->description = ItemTypes::RIGHT;
                 firstDesc->exitCount   = 3;
                 //firstDesc->debug       = true;
                 if(printOut)
                    cerr << "Right, ";
              }
              
              // Ahead,
              else if((angle1>4*M_PI_6)&& (angle1< 7.5*M_PI_6) && 
                      (angle3 > M_PI)&& (angle2 > M_PI)&&
                      (((roadClass1 == roadClass2) &&
                        (2.5*(M_PI-angle1) < (angle2-M_PI))) ||
                       ((roadClass1 < roadClass2) &&
                        ((M_PI-angle1) < (angle2-M_PI))) ||
                       ((roadClass1 > roadClass2) && (!ramp1 || ramp2) && 
                        (3*(M_PI-angle1) < (angle2-M_PI))))
                      )
              {
                 firstDesc->description = ItemTypes::AHEAD;
                 firstDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "Ahead, ";
              }
              // Ahead because of same name.
              else if((angle1>5*M_PI_6) && (angle1< 7.5*M_PI_6) &&
                      sameName1 && !sameName2 && !sameName3 &&
                      (roadClass1 <= roadClass2) && (angle2 > M_PI)&&
                      (roadClass1 < roadClass2) &&
                      ((M_PI-angle1) < 2*(angle2-M_PI))){
                 firstDesc->description = ItemTypes::AHEAD;
                 firstDesc->exitCount   = 1;
                 //firstDesc->debug       = true;
                 if(printOut)
                    cerr << "Ahead(same name), ";
              }
              
              // Ahead because of ramps
              else if((!ramp0 && !ramp1 && ramp2 && ramp3) &&
                      (angle1 > 5* M_PI_6) && (angle2 < 7.5 *M_PI_6)
                      )
              {
                 firstDesc->description = ItemTypes::AHEAD;
                 firstDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "Ahead, ";
              }
                 
              // Left,
              else
              {
                 firstDesc->description = ItemTypes::LEFT;
                 firstDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "Left, ";
              }
              
              // Second connection ----- default ahead -------------------

              // (Keep_left,) ahead, (keep_right) SPECIAL CASE!!!
              if((firstDesc->description == ItemTypes::KEEP_LEFT) &&
                 (roadClass2 == roadClass3) && (ramp2==ramp3) &&
                 ((angle3-angle2) < 2*M_PI_6) && (angle3<7.5*M_PI_6)
                 )
              {
                 secondDesc->description = ItemTypes::AHEAD;
                  secondDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "ahead, " ;
              }
              
              // exit to left.
              else if (((firstDesc->description == ItemTypes::OFF_RAMP) ||
                        (firstDesc->description == ItemTypes::OFF_RAMP_LEFT))
                       && (ctrlA0 || !rSide) && 
                       (!ramp0 && ramp1 && ramp2 && !ramp3 ) &&
                       (angle3 < 7.5* M_PI_6) && (angle2 < 7*M_PI_6))
              {  
                 secondDesc->description = ItemTypes::OFF_RAMP_LEFT;
                 secondDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "off_r, " ;
              }
              // exit to right.
              else if ((firstDesc->description == ItemTypes::AHEAD) &&
                       (!ramp0 && !ramp1 && ramp2 && ramp3 ) &&
                       (angle1 > 5* M_PI_6) && (ctrlA0 || rSide) && 
                       (angle2 < 7 * M_PI_6))
              {  
                 secondDesc->description = ItemTypes::OFF_RAMP_RIGHT;
                 secondDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "off_r, " ;
              }
              
              // Two ramps on the left side of the road.
              else if ((ramp0 && ramp1 && !ramp2 && !ramp3) &&
                       (angle2 > 5* M_PI_6) &&
                       ((angle3-angle2)>5*M_PI_6) && ((angle3-angle2)<7*M_PI_6)
                       )
              {
                 secondDesc->description = ItemTypes::ON_RAMP;
                 secondDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "on_r, " ;
              }
              
              // Two ramps on the right side of the road
              else if ((ramp0 && !ramp1 && !ramp2 && ramp3) &&
                       (angle2 < 8* M_PI_6) &&
                       ((angle2-angle1)>5*M_PI_6) && ((angle2-angle1)<7*M_PI_6)
                       )
              {  
                 secondDesc->description = ItemTypes::ON_RAMP;
                 secondDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "on_r, " ;
              } 
              // keep_right, 
              else if(firstDesc->description == ItemTypes::KEEP_LEFT)
              {
                 secondDesc->description = ItemTypes::KEEP_RIGHT;
                 secondDesc->exitCount   = 2;
                 if(printOut)
                    cerr << "k_r, " ;
              }
              
              // keep_left,
              else if((firstDesc->description == ItemTypes::LEFT) &&
                      (roadClass2 ==roadClass3)&&((angle3-angle2)<2*M_PI_6)&&
                      (angle2<M_PI)&& (2*(M_PI-angle2) >(angle3-M_PI)) &&
                      (ramp2 == ramp3) && 
                      ((roadClass2 <3) ||
                       (noEntry0 && !noEntry3 && !noEntry2 &&
                        (((angle2-angle1)> 2*M_PI_6) || noEntry1)))
                      )
              {
                 secondDesc->description = ItemTypes::KEEP_LEFT;
                 secondDesc->exitCount   = 2;
                 if(printOut)
                    cerr << "k_l, ";
              }

              // right, (right)
              // First was right or
              // more than 90 deg to right and 90 deg if larger road.
              else if((firstDesc->description == ItemTypes::AHEAD) ||
                      ((firstDesc->description == ItemTypes::RIGHT) ||
                      ((angle2> 8*M_PI_6) &&
                       ((angle2> 9*M_PI_6) ||(roadClass2 <= roadClass1)))
                       ))
              {
                 secondDesc->description = ItemTypes::RIGHT;
                 secondDesc->exitCount   = 2;
                 if(printOut)
                    cerr << "right, ";
              }

              // left,
              else if((firstDesc->description == ItemTypes::LEFT) &&
                      (((roadClass2 == roadClass3) && (angle2 < M_PI)&&
                        ((angle3<M_PI)||(2*(angle3-M_PI)<(M_PI-angle2))||
                         (angle2<4*M_PI_6))) ||
                       
                       ((roadClass2 > roadClass3) &&
                        (!(!ramp0 && ramp1 && ramp2 && ramp3))&&
                        ((angle3<M_PI)||(2.5*(angle3-M_PI)<(M_PI-angle2)) ||
                         ((roadClass2 > roadClass3+2)&&
                          ((angle3)< 7.5*M_PI_6))))    ||
                       
                       ((angle2<M_PI) && !ramp3 && 
                        ((angle2< 4*M_PI_6) ||
                         ((angle3>M_PI) && (3*(angle3-M_PI)<(M_PI-angle2))) ||
                         ((angle3<=M_PI) && (2*(M_PI-angle3)<(M_PI-angle2)))))
                       ))
              {
                 secondDesc->description = ItemTypes::LEFT;
                 secondDesc->exitCount   = 2;
                 if(printOut)
                    cerr << "left, " ;
              }
              // Left because of next has same name,
              else if ((firstDesc->description == ItemTypes::LEFT) &&
                       !sameName1 && !sameName2 && sameName3 &&
                       (roadClass3 <= roadClass2) && (angle2 < M_PI) &&
                       ((angle3-M_PI) < 2*(M_PI -angle2)) &&
                       (angle3 < 9*M_PI_6)
                       ){
                 secondDesc->description = ItemTypes::LEFT;
                 secondDesc->exitCount   = 2;
                 // secondDesc->debug       = true;
                 if(printOut)
                    cerr << "left(same name), " ;
              } 
               
              // Ahead.   
              else
              {
                 secondDesc->description = ItemTypes::AHEAD;
                  secondDesc->exitCount   = 1;
                  if(printOut)
                     cerr << "ahead, " ;
              }
              
              // Third connection ------ default right -------------------
              // keep_left not possible.
              // keep_right
              if(((secondDesc->description == ItemTypes::AHEAD) &&
                      (firstDesc->description == ItemTypes::KEEP_LEFT))||
                      (secondDesc->description == ItemTypes::KEEP_LEFT))
              {
                 thirdDesc->description = ItemTypes::KEEP_RIGHT;
                 thirdDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "k_r." << endl;
              }
              else if ((!ramp0 && !ramp1 && ramp2 && ramp3)&&
                       //(roadClass0 == roadClass1) &&
                       (roadClass0 <= roadClass3) &&
                       (ctrlA0 || rSide) && 
                       (angle1 > 5* M_PI_6 ) &&
                       (angle3 < 7 * M_PI_6)
                       )
              {  // Two exits to the right
                 thirdDesc->description = ItemTypes::OFF_RAMP_RIGHT;
                 thirdDesc->exitCount   = 2;
                 if(printOut)
                    cerr << "off_r." << endl;
              }
              
              else if ((!ramp0 && ramp1 && !ramp2 && ramp3) &&
                       //(roadClass0 == roadClass2) &&
                       (roadClass0 <= roadClass3) &&
                       (ctrlA0 || rSide) && 
                       (angle2 >5 * M_PI_6) && (angle2 < 7* M_PI_6) &&
                       ((angle3 - angle2) < M_PI_6)
                       )
              {  // Exit to the left and the right.
                 thirdDesc->description = ItemTypes::OFF_RAMP_RIGHT;
                 thirdDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "off_r(2)." << endl;
              }  

              // One of two onramps right of road.
              else if((ramp0 && ramp1 && !ramp2 && !ramp3) &&
                      (angle3< 7* M_PI_6) &&
                      ((angle3-angle2)>5*M_PI_6) && ((angle3-angle2)<7*M_PI_6)
                      )
              {
                 thirdDesc->description = ItemTypes::ON_RAMP;
                 thirdDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "on_r." << endl;
              }

              // On ramp ramp on each side of maj.road
              else if((ramp0 && !ramp1 && ramp2 && !ramp3) &&
                      (angle3< 7* M_PI_6) &&
                      ((angle3-angle1)>5*M_PI_6) && ((angle3-angle1)<7*M_PI_6)
                      )
              {
                 thirdDesc->description = ItemTypes::ON_RAMP;
                 thirdDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "on_r(2)." << endl;
              }
              
              // left.
              // more than 60 deg to right and 90 deg if larger road.
              else if((secondDesc->description == ItemTypes::LEFT)&&
                 (angle3< 4*M_PI_6) &&
                 ((angle3> M_PI_2) ||(roadClass3 <= roadClass2)))
              {
                 thirdDesc->description = ItemTypes::LEFT;
                 thirdDesc->exitCount   = 3;
                 if(printOut)
                    cerr << "left." << endl;
              }
              
              // ahead
              // last left or k_l
              else if((secondDesc->description == ItemTypes::KEEP_LEFT)||
                       ((secondDesc->description == ItemTypes::LEFT) &&
                        ((angle3 < M_PI) ||(2*(angle3-M_PI)<(M_PI-angle2)) ||
                         (roadClass2 > roadClass3 +2))
                        ))
                         
              {
                 thirdDesc->description = ItemTypes::AHEAD;
                 thirdDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "ahead." << endl;
              }
              // ahead because of same name.
              else if(((secondDesc->description == ItemTypes::KEEP_LEFT)||
                       (secondDesc->description == ItemTypes::LEFT)) &&
                        !sameName1 && !sameName2 && sameName3 &&
                        (roadClass3 <= roadClass2) &&
                      (angle3 < 9*M_PI_6)){
                 thirdDesc->description = ItemTypes::AHEAD;
                 thirdDesc->exitCount   = 1;
                 // thirdDesc->debug       = true;
                 if(printOut)
                    cerr << "ahead." << endl;
              }
              
              // right.
              else
              {
                 thirdDesc->description = ItemTypes::RIGHT;
                 thirdDesc->exitCount   = 1;
                 if(printOut)
                    cerr << "right." << endl;
              }

              // Enter structs in vector.
              result.push_back(*firstDesc);
              result.push_back(*secondDesc);
              result.push_back(*thirdDesc);
              delete firstDesc;
              delete secondDesc;
              delete thirdDesc;
              kind = ItemTypes::CROSSING_4WAYS;
            }
            break;
            default :
            {
               if((size>6) && (i==0))
                  cerr << "Warning " << size <<
                     " road crossing in map, better check!"<< endl;
               ItemTypes::turndirection_t lastSetTurn = ItemTypes::UNDEFINED;
               if(printOut){
                  cerr << "From street :" << i << endl;
               }
               
               
               for(uint16 j =1; j < size; j++)
               {
                  int connIndex  = (i+j)% size;
                  int rightIndex = (connIndex+1)% size;
                  int leftIndex  = (connIndex+size-1)% size;
                  float64 angle = static_cast<CrossingConnectionElement*>
                     (conNotices->getElementAt(connIndex))->m_angle - angle0;
                  if(angle < 0)
                     angle += 2* M_PI;
                  float64 leftAngle  = static_cast<CrossingConnectionElement*>
                     (conNotices->getElementAt(leftIndex))->m_angle - angle0;
                  if(leftAngle < 0)
                     leftAngle += 2* M_PI;
                  float64 rightAngle = static_cast<CrossingConnectionElement*>
                     (conNotices->getElementAt(rightIndex))->m_angle - angle0;
                  if(rightAngle < 0 )
                     rightAngle += 2* M_PI;
                  byte roadClass = static_cast<CrossingConnectionElement*>
                     (conNotices->getElementAt(connIndex))->m_roadClass;
                  byte leftRoadClass = static_cast<CrossingConnectionElement*>
                     (conNotices->getElementAt(leftIndex))->m_roadClass;
                  byte rightRoadClass = static_cast<CrossingConnectionElement*>
                     (conNotices->getElementAt(rightIndex))->m_roadClass;
                  bool noEntry  = (static_cast<CrossingConnectionElement*>
                                   (conNotices->getElementAt(connIndex))
                                   ->m_passNodeRestrictions !=
                                   ItemTypes::noRestrictions);
                  bool leftEntry = (static_cast<CrossingConnectionElement*>
                                    (conNotices->getElementAt(leftIndex))
                                    ->m_passNodeRestrictions !=
                                    ItemTypes::noRestrictions);
                  bool rightEntry = (static_cast<CrossingConnectionElement*>
                                     (conNotices->getElementAt(rightIndex))
                                     ->m_passNodeRestrictions !=
                                     ItemTypes::noRestrictions);
                  
                  bool multi = static_cast<CrossingConnectionElement*>
                     (conNotices->getElementAt(connIndex))->m_ismulti;
                  bool leftMulti = static_cast<CrossingConnectionElement*>
                     (conNotices->getElementAt(leftIndex))->m_ismulti;
                  bool rightMulti = static_cast<CrossingConnectionElement*>
                     (conNotices->getElementAt(rightIndex))->m_ismulti;
                  
                  if(noEntry && !multi
                     )
                     roadClass += 2; // 1 might not be enough.
                  if(leftEntry && !leftMulti
                     )
                     leftRoadClass +=2;
                  if(rightEntry && !rightMulti)
                     rightRoadClass +=2;
                  if(printOut){
                     cerr << j << ": " << endl;
                     cerr << "   angle : " << angle << endl;
                     cerr << "   rightAngle : " <<rightAngle << endl;
                     cerr << "   roadClass : " <<(int)roadClass << endl;
                     cerr << "   rightRoadClass : " << (int)rightRoadClass
                          << endl;
                     cerr << "    rightAngle-angle: " <<rightAngle-angle
                          << endl;
                     cerr << "   2*(M_PI-angle) : " <<2*(M_PI-angle) << endl;
                     cerr << "   lastSetTurn : " << int(lastSetTurn) << endl;
                     
                  }
                     

                  // Create the turnstruct.
                  turndesc_t* desc   = new turndesc_t;
                  desc->toConIndex   = byte(connIndex);
                  desc->fromConIndex = i;
                  desc->debug = false;
                  
                  if(lastSetTurn == ItemTypes::UNDEFINED)
                  {
                     // First conn.
                        // Ahead.
                     if((angle > 4.5*M_PI_6) && 
                        (((roadClass <= rightRoadClass) &&
                         (((angle<M_PI) &&
                           ((rightAngle-angle)>2*(M_PI-angle))) ||
                          ((angle>= M_PI) &&
                           ((rightAngle-angle)>(angle-M_PI)))))
                         ||
                         (((roadClass > rightRoadClass) &&
                          (rightAngle>M_PI) &&
                          ((angle > M_PI) &&((rightAngle-angle)>(angle-M_PI))))
                          || (3*(M_PI-angle) < (rightAngle-M_PI))))
                        )
                        {
                           desc->description = ItemTypes::AHEAD;
                           desc->exitCount   = 1;
                           lastSetTurn       = ItemTypes::AHEAD;
                           if(printOut)
                              cerr << "Ahead, ";
                        }
                        // keep_right not possible.
                        
                        // right, keep_l, 
                        else if(false)
                        {
                           // Wait with this one, rather unlikely.
                        }

                        // Left, 
                        else
                        {
                           desc->description = ItemTypes::LEFT;
                           desc->exitCount   = 1;
                           lastSetTurn       = ItemTypes::LEFT;
                           if(printOut)
                              cerr << "Left, " ;
                        }
                  }
                  else if (j < size-1) 
                  { //Not the first, not the last.
                     // Keep_Left
                     if((lastSetTurn == ItemTypes::LEFT) &&
                        (roadClass == rightRoadClass) &&
                        ((rightAngle-angle)< 1.5* M_PI_6) &&
                        (angle> 4.5*M_PI_6) && (angle < M_PI) &&
                        (((rightAngle>M_PI) &&
                          (2*(M_PI-angle) > (rightAngle-M_PI))) ||
                         (rightAngle<=M_PI))
                        )
                          
                     {
                        desc->description = ItemTypes::KEEP_LEFT;
                        desc->exitCount   = j;
                        //desc->debug       = true;
                        lastSetTurn       = ItemTypes::KEEP_LEFT;
                        if(printOut)
                           cerr << "keep_left, " ;
                     }
                     // Keep_Right
                     else if(((lastSetTurn == ItemTypes::KEEP_LEFT)||
                              (lastSetTurn == ItemTypes::AHEAD)) &&
                             ((roadClass == leftRoadClass) &&
                             ((angle-leftAngle)< 1.5* M_PI_6) &&
                             (angle< 7.5*M_PI_6) && (angle> M_PI) &&
                             (((leftAngle<M_PI) &&
                               ((angle-M_PI)>(M_PI-leftAngle))) ||
                              (leftAngle>=M_PI))
                              ))
                     {
                        desc->description = ItemTypes::KEEP_RIGHT;
                        desc->exitCount   = j;
                        //desc->debug       = true;
                        lastSetTurn       = ItemTypes::RIGHT;//might as well
                        if(printOut)
                           cerr << "keep_right, " ;
                     }
                     // Ahead                     
                     else if((lastSetTurn == ItemTypes::KEEP_LEFT) ||
                             ((lastSetTurn == ItemTypes::LEFT) &&
                              (angle > 4.5*M_PI_6) &&(angle < 7.5*M_PI_6) &&
                              (leftAngle < M_PI) && (rightAngle > M_PI)
                              && // Check to the left.
                              (((((roadClass < leftRoadClass)) &&
                               ((angle <= M_PI) ||
                                ((roadClass+1 < leftRoadClass) &&
                                ((angle-M_PI)< 5*(M_PI-leftAngle)))||
                                ((angle-M_PI)< 2*(M_PI-leftAngle)))) ||
                               ((roadClass > leftRoadClass) &&
                                ((angle <= M_PI) ||
                                 (3*(angle-M_PI)<(M_PI-leftAngle)))) ||
                                ((roadClass == leftRoadClass) &&
                                 ((angle <= M_PI) ||
                                  (2*(angle-M_PI)<(M_PI-leftAngle)))))
                               && // Check to the right.
                               (((roadClass < rightRoadClass) &&
                                 ((angle >= M_PI) ||
                                  ((roadClass+1 < rightRoadClass) &&
                                   ((M_PI-angle)< 5*(rightAngle-M_PI))) ||
                                  ((M_PI-angle)< 2*(rightAngle-M_PI)))) ||
                                ((roadClass > rightRoadClass) &&
                                 ((angle >= M_PI) ||
                                  (3*(M_PI-angle)<(rightAngle-M_PI)))) ||
                                ((roadClass == rightRoadClass) && 
                                 ((angle >= M_PI) ||
                                  (2*(M_PI-angle)<(rightAngle-M_PI))))))
                              ))
                     {
                        desc->description = ItemTypes::AHEAD;
                        desc->exitCount   = 1;
                        lastSetTurn       = ItemTypes::AHEAD;
                        if(printOut)
                           cerr << "ahead, " ;
                     }
                     // left, 
                     else if(angle < M_PI)
                     {
                        desc->description = ItemTypes::LEFT;
                        desc->exitCount   = j;
                        lastSetTurn       = ItemTypes::LEFT;
                        if(printOut)
                           cerr << "left, " ;
                     }
                     // Right
                     else
                     {
                        desc->description = ItemTypes::RIGHT;
                        desc->exitCount   = size-j;
                        lastSetTurn       = ItemTypes::RIGHT;
                        if(printOut)
                           cerr << "right, ";
                     }
                  }
                  else // last.
                  {
                     // Ahead
                     if((lastSetTurn == ItemTypes::KEEP_LEFT) ||
                         ((lastSetTurn == ItemTypes::LEFT) &&

                        (((roadClass <= leftRoadClass) &&
                         (((angle>M_PI) &&
                           ((angle-leftAngle)>2*(angle-M_PI))) ||
                          ((angle<= M_PI) &&
                           ((angle-leftAngle)>(M_PI-angle)))))
                        ||
                        ((roadClass > leftRoadClass) &&
                         (leftAngle< M_PI) &&
                         (((angle<M_PI) && ((angle-leftAngle)>(M_PI-angle)))
                          || (3*(angle - M_PI) < (M_PI-leftAngle)))))
                          ))
                     {
                        desc->description = ItemTypes::AHEAD;
                        desc->exitCount   = 1;
                        if(printOut)
                           cerr << "ahead. " << endl;
                     }
                     // keep_left not possible
                     
                     // keep_right, left.
                     else if(false // Wait with this one, rather unlikely.
                             )
                     {
                        desc->description = ItemTypes::LEFT;
                        desc->exitCount   = 1;
                        if(printOut)
                           cerr << "left. " << endl;
                     }
                     // right.
                     else
                     {
                        desc->description = ItemTypes::RIGHT;
                        desc->exitCount   = 1;
                        if(printOut)
                           cerr << "right. " << endl;
                     }
                     
                  }
                  result.push_back(*desc);
                  delete desc;
               }
               
               kind = (ItemTypes::crossingkind_t)
                  (size - 4 + (int)ItemTypes::CROSSING_4WAYS);
            }
                  
         }
      }
      else // noRoundabout == false
      {
         kind = ItemTypes::UNDEFINED_CROSSING;
         bool fromRoundAbout = (thisElement->m_roundabout ||
                                thisElement->m_roundaboutish);
         
         for (uint16 j=1; j < size; j++)
         {
            int connIndex = (i+j)% size;
            float64 angle = static_cast<CrossingConnectionElement*>
               (conNotices->getElementAt(connIndex))->m_angle - angle0;
            if(angle < 0)
               angle += 2*M_PI;
            bool toRoundAbout =
               (static_cast<CrossingConnectionElement*>
                (conNotices->getElementAt(connIndex))->m_roundabout ||
                static_cast<CrossingConnectionElement*>
                (conNotices->getElementAt(connIndex))->m_roundaboutish);
            turndesc_t* desc   = new turndesc_t;
            desc->toConIndex   = byte(connIndex);
            desc->fromConIndex = i;
            desc->exitCount    = 0;
            desc->debug = false;
            
            // - Let's decide !
            if(!noRoundabout){
               if (fromRoundAbout && toRoundAbout)
                  desc->description = ItemTypes::AHEAD;
               else if (!fromRoundAbout && toRoundAbout)
                  desc->description = ItemTypes::ENTER_ROUNDABOUT;
               else if (fromRoundAbout && !toRoundAbout)
                  desc->description = ItemTypes::EXIT_ROUNDABOUT;
               else if ((!fromRoundAbout && !toRoundAbout) && (angle >= M_PI))
                  desc->description = ItemTypes::RIGHT_ROUNDABOUT;
               else if ((!fromRoundAbout && !toRoundAbout) && (angle < M_PI))
                  desc->description = ItemTypes::LEFT_ROUNDABOUT;
               else
               {
                  cerr<<"::Error:: Undefined roundabout turn exist in map: "
                      << angle <<endl;
                  desc->description = ItemTypes::UNDEFINED;
                  //desc->debug = true;
               }
               // Test exit angle for map errors.
               if(fromRoundAbout && !toRoundAbout && ROUNDABOUT_DUMP &&
                  (((angle > 9.2 * M_PI_6) && rSide ) ||
                   ((angle < 2.8 * M_PI_6) && !rSide))){
                  
                  bool noWayExit = (static_cast<CrossingConnectionElement*>
                                    (conNotices->getElementAt(connIndex))
                                    ->m_passNodeRestrictions ==
                                    ItemTypes::noWay);
                  if(!noWayExit){
                     // Probable map error debug connection
                     cerr << "Probable map error in roundabout exit" << endl;
                     desc->debug = true;
                  }
               }
               if(fromRoundAbout && !toRoundAbout && NO_MULTIDIG_DUMP){
                  bool noWayExit = (static_cast<CrossingConnectionElement*>
                                    (conNotices->getElementAt(connIndex))
                                    ->m_passNodeRestrictions ==
                                    ItemTypes::noWay);
                  bool noWayEntry = (thisElement->m_passNodeRestrictions ==
                                     ItemTypes::noWay);
                  bool multiDig = (static_cast<CrossingConnectionElement*>
                                   (conNotices->getElementAt(connIndex))
                                   ->m_ismulti);
                  bool ramp = (static_cast<CrossingConnectionElement*>
                               (conNotices->getElementAt(connIndex))
                               ->m_ramp);
                  if(noWayExit && !multiDig && noWayEntry && !ramp
                     ){
                     cerr << "Noway roundabout exit not multi." << endl;
                     desc->debug = true;
                  }
               
               }
            } else { // roundaboutish               
               
               if(!fromRoundAbout && toRoundAbout &&
                  (((angle > M_PI) && rSide) ||((angle < M_PI) && !rSide) ||
                   (nbrRbish == 1)))
                  desc->description = ItemTypes::ENTER_ROUNDABOUT; 
               else if(fromRoundAbout && !toRoundAbout &&
                       (((angle > M_PI) && rSide) || 
                        ((angle < M_PI) && !rSide) || (nbrRbish == 1)))
                  desc->description = ItemTypes::EXIT_ROUNDABOUT;
               else if((fromRoundAbout && toRoundAbout) ||
                  ((angle > 5*M_PI_6) && (angle < 7*M_PI_6 )))
                  desc->description = ItemTypes::AHEAD;
               else if(angle < M_PI)
                  desc->description = ItemTypes::LEFT;
               else
                  desc->description = ItemTypes::RIGHT;
               
            }
            
            
            
            // Store the result
            result.push_back(*desc);
            delete desc;
         }
      }
   } // for(int i ...
   if(printOut)
      cerr << "conNotices->Size : " << conNotices->getSize() << endl;

   if(rampAccess){
      // If 3-way crossing change all AHEAD to FOLLOWROAD.
      if (printOut ) {
         cerr << "rampAccess => change all AHEAD to FOLLOWROAD" << endl;
      }
      if(size == 3){
         turndescvector_t::iterator trnDscit;
         for(trnDscit = result.begin(); 
             trnDscit != result.end();
             trnDscit++){
            if(trnDscit->description == ItemTypes::AHEAD){
               trnDscit->description = ItemTypes::FOLLOWROAD;
               trnDscit->debug = false;
            }
         }
      }
      // Here it could be nice to use a new crossing  kind.
      kind = ItemTypes::NO_CROSSING;
      
      
   }
   else if((size == 3) && noRoundabout && noRoundaboutish)
   {
      if(leftRight != 1)
         tcrossing = false;
      if (tcrossing)
         kind = ItemTypes::CROSSING_3WAYS_T;
      else
         kind = ItemTypes::CROSSING_3WAYS_Y;
   }
   if(printOut)
      cerr << "leftRight : " << leftRight << endl;
   if(result.size() == uint32(size*(size-1)))
      return true;
   else
   {
      cerr << " Bad size on result vector " << endl;
      return false;
   }
   
}

bool
Crossing::getToFerryItemTurndescriptions(OldNode* ferryNode,
                                         OldMap* theMap)
{
   uint16 nbrCon = ferryNode->getNbrConnections();
   int nbrFerry = 0;
   int nbrRoad  = 0;
   bool printOut = false;
   // Check type of crossing. Count the item types
   for (uint16 i=0; i < nbrCon; i++){
      if(!ferryNode->getEntryConnection(i)->isMultiConnection()){
         uint32 fromID = ferryNode->getEntryConnection(i)->
            getConnectFromNode();
      
         if(theMap->itemLookup(fromID)->getItemType() == ItemTypes::ferryItem)
            nbrFerry++;
         else if (theMap->itemLookup(fromID)->getItemType()
                  == ItemTypes::streetSegmentItem)
            nbrRoad++;
      }
   }
   
   // Set the connections
   for (uint16 i=0; i < nbrCon; i++) {
      OldConnection* con = ferryNode->getEntryConnection(i);
      if(!con->isMultiConnection()){
         uint32 fromID =  con->getConnectFromNode();
         if(printOut)
            cerr << "Ferry con. from node: " << fromID << " to ferry node: "
                 << ferryNode->getNodeID() << " is : " ;
         
         OldRouteableItem* fromRI = dynamic_cast<OldRouteableItem*>
            (theMap->itemLookup(fromID));
         if((fromRI != NULL) &&
            (fromRI->getItemType() == ItemTypes::ferryItem))
         {
            if(nbrRoad == 0)
            {
               con->setTurnDirection(ItemTypes::FOLLOWROAD);
               if(printOut)
                  cerr << "Follow road(ferry)." << endl;
            }
            else
            {
               con->setTurnDirection(ItemTypes::CHANGE_FERRY);
               if(printOut)
                  cerr << "Change ferry." << endl;
            }
         }
         else if((fromRI !=NULL)&&
                 (fromRI->getItemType() == ItemTypes::streetSegmentItem ))
         {
            // Check if it is a virtual boundary segment.
            if ( theMap->getBoundrySegments()
                 ->getBoundrySegment( fromRI->getID() ) == NULL ) {
               con->setTurnDirection(ItemTypes::ENTER_FERRY);
               if(printOut)
                  cerr << "Enter ferry." << endl;
            } else {
               con->setTurnDirection(ItemTypes::FOLLOWROAD);
               if(printOut)
                  cerr << "Follow road(ferry)." << endl;
            }
         }
         else
         {
            if(printOut)
               cerr << endl;
            mc2log << warn
                   << " Ferry connection from wrong OldItemtype, from item : "
                   << fromID << ", to node : " << ferryNode->getNodeID()
                   << endl;
            return false;
            
         }
         con->setCrossingKind(ItemTypes::NO_CROSSING);
         con->setExitCount(1);
      }
   }
   static_cast<GMSNode*>(ferryNode)->setTurnSet();
   return true;
}

OldNode*
Crossing::getNoEntryNode(bool &leavingRoundabout) const
{
   OldNode* returnNode = NULL;
   CrossingConnectionElement* returnElement = NULL;
   bool inRoundAbout = static_cast<CrossingConnectionElement*>
      (m_conNotices[0])->m_roundabout;
   
   for (int i = 0 ; i < (int)m_conNotices.getSize(); i++){
      CrossingConnectionElement* con =
            static_cast<CrossingConnectionElement*>(m_conNotices[i]);

      if((con->m_passNodeRestrictions == ItemTypes::noWay) &&
         !con->m_roundabout){
         // found a noWay connection.
         if((returnElement != NULL) && !returnElement->m_roundabout)
            return NULL;
         else
            returnElement = con;
      }
      if((con->m_passNodeRestrictions == ItemTypes::noRestrictions) &&
         con->m_roundabout && (returnElement == NULL)){
         // Following a roundabout
         returnElement = con;
      }
      if((con->m_passNodeRestrictions == ItemTypes::noRestrictions) &&
         inRoundAbout && !con->m_roundabout){
         returnElement = con;
      }
   }
   if(returnElement == NULL)
      return NULL;
   if(inRoundAbout && !returnElement->m_roundabout)
      leavingRoundabout = true;
   returnNode = m_nodes[returnElement->m_nodeIndex];
   return returnNode;
}

bool
Crossing::setRoundaboutCrossingKind()
{
   // Some tests.
   if(m_noRoundabout && m_noRoundaboutish)
      return true;
   if(m_numberOfNodes < 2){
      //cerr << "ERROR to few nodes (<2) at node : "
      //     << m_nodes[0]->getNodeID() << endl;
      if(ROUNDABOUT_DUMP){
         //cout << "NODE: " << m_map->getMapID() << "."
         //     << m_nodes[0]->getNodeID() << ";" << endl;
      }
      return false;
   }

   // Ok lets go
   else {
      // These are the fields to fill:
      ItemTypes::crossingkind_t setKind = ItemTypes::UNDEFINED_CROSSING;
      rbtconnvector_t connections;
      connections.clear();
      // Help variables
      int exits = 0;
      
      bool h = m_map->driveOnRightSide();
      
      // The first of the rbt(ish) connections leading out of the crossing.
      OldNode* firstRbtNode = NULL;
      
      // The second.
      OldNode* secondRbtNode = NULL;
      
      int32 firstLength = 0;
      int32 secondLength = 0;
      uint32 nodeLength[4];
      for(int i = 0 ; i < 4 ; i++)
         nodeLength[i] = 0;
   
      // Lets check the connections to find exits and nodes.
      for (int i = 0 ; i < (int)m_conNotices.getSize(); i++){
         CrossingConnectionElement* con =
            static_cast<CrossingConnectionElement*>(m_conNotices[i]);
         
         if((!con->m_roundabout) && (!con->m_roundaboutish) && 
            (con->m_passNodeRestrictions == ItemTypes::noRestrictions)){
            // We found an exit!
             exits++;
         }
         else if ((!con->m_roundabout) &&(!con->m_roundaboutish) && 
                  (con->m_passNodeRestrictions == ItemTypes::noThroughfare))
         {
            exits++;
            // We found a restricted exit!
            if(NO_THROUGHFARE_DUMP)
               cout << "NODE: " << m_map->getMapID() << "."
                    << m_nodes[0]->getNodeID() << ";" << endl;
         }
      
         else if ((con->m_roundabout) ||  (con->m_roundaboutish)){
            //  roundabout(ish) connection found.
            if(firstRbtNode == NULL){
               firstRbtNode = m_nodes[con->m_nodeIndex];
               firstLength = con->m_length;
            }
            else if(secondRbtNode == NULL){
               secondRbtNode = m_nodes[con->m_nodeIndex];
               secondLength = con->m_length;
            }
            else {
               // We dont want more than two rbt(ish) connections.
               cerr << "ERROR IN ROUNDABOUT !!" << endl;
               cerr << "To many  roundabout(ish) connections found" << endl;
               cerr << firstRbtNode->getNodeID() << endl;
               if(ROUNDABOUT_DUMP){
                  cout << "NODE: " << m_map->getMapID() << "."
                       << m_nodes[0]->getNodeID() << ";" << endl;
               }
               return false;  
            }
         }
         else {
            // No exit allowed at this con. Ignored here.
         }
      }

      // We must have at least one rbtish or two rbt connections
      if((firstRbtNode == NULL) ||
         ((secondRbtNode == NULL) && !m_noRoundabout)){
         cerr << "ERROR IN ROUNDABOUT !!" << endl;
         cerr << "Not enough roundabout";
         if(!m_noRoundaboutish){
            cerr << "ish";
         }
         
         cerr << " connections found" << endl;
         cerr << m_nodes[0]->getNodeID() << endl;
         if(ROUNDABOUT_DUMP){
            cout << "NODE: " << m_map->getMapID() << "."
                 << m_nodes[0]->getNodeID() << ";" << endl;
         }
         return false;
      }
      if(secondRbtNode!= NULL){
         // Go counter clockwise
         if((h &&
             (firstRbtNode->getEntryRestrictions() == ItemTypes::noWay) &&
             (secondRbtNode->getEntryRestrictions() != ItemTypes::noWay)) ||
            (!h &&
             (firstRbtNode->getEntryRestrictions() != ItemTypes::noWay) &&
             (secondRbtNode->getEntryRestrictions() == ItemTypes::noWay))){
            
            OldNode* temp = firstRbtNode;
            firstRbtNode = secondRbtNode;
            secondRbtNode = temp;
            int32 tempLength = firstLength;
            firstLength = secondLength;
            secondLength = tempLength;
         }
      }
      
      
      
      // We only count one exit per crossing.
      if (exits > 0)
         exits = 1;

      
      // Add rbtstructs to vector
      // Find entries and exits in this crossing.
      OldNode* firstOutOfNode = m_map->nodeLookup(
         TOGGLE_UINT32_MSB(firstRbtNode->getNodeID()));
      OldNode* secondOutOfNode = NULL;
      if(secondRbtNode != NULL){
         secondOutOfNode = m_map->nodeLookup(
            TOGGLE_UINT32_MSB(secondRbtNode->getNodeID()));
      }
      
      // Find entries for first node.
      for(uint32 i = 0; i < firstRbtNode->getNbrConnections() ; i++){
         OldConnection* conn = firstRbtNode->getEntryConnection(i);
         if(!conn->isMultiConnection() &&
            (conn->getTurnDirection() == ItemTypes::ENTER_ROUNDABOUT) &&
            (conn->isVehicleAllowed(ItemTypes::passengerCar))){
            OldNode* connNode = m_map->nodeLookup(
            TOGGLE_UINT32_MSB(conn->getConnectFromNode()));
            OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
            (m_map->itemLookup(REMOVE_UINT32_MSB(conn->getConnectFromNode())));
            rbtconn_t rbconn;
            rbconn.conn = conn;
            rbconn.exit = false;
            rbconn.distance = 0;
            rbconn.multiDig = (ssi->isMultiDigitised() ||
                                ssi->isRamp() || ssi->isControlledAccess());
            rbconn.angle = getAdjustedAngle(ssi->getGfxData(),
                                            connNode->isNode0(),
                                            m_printOut);
            rbconn.index = 255;
            rbconn.valid = true; //rbtAngleIsValid(connNode, ssi);
            addRbtConnToVector(connections, rbconn, firstRbtNode ,true);
            //connections.push_back(rbconn);
         }    
      }
      // Find entries for second node.
      if(secondRbtNode != NULL){
         for(uint32 i = 0; i < secondRbtNode->getNbrConnections() ; i++){
            OldConnection* conn = secondRbtNode->getEntryConnection(i);
            if(!conn->isMultiConnection() &&
               (conn->getTurnDirection() == ItemTypes::ENTER_ROUNDABOUT) &&
               (conn->isVehicleAllowed(ItemTypes::passengerCar))){
               OldNode* connNode = m_map->nodeLookup(
                  TOGGLE_UINT32_MSB(conn->getConnectFromNode()));
               OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
                  (m_map->itemLookup(
                     REMOVE_UINT32_MSB(conn->getConnectFromNode())));
               rbtconn_t rbconn;
               rbconn.conn = conn;
               rbconn.exit = false;
               rbconn.distance = 0;
               rbconn.multiDig = (ssi->isMultiDigitised() ||
                                   ssi->isRamp() || ssi->isControlledAccess());
               rbconn.angle = getAdjustedAngle(ssi->getGfxData(),
                                               connNode->isNode0(),
                                               m_printOut);
               rbconn.index = 255;
               rbconn.valid = true; //rbtAngleIsValid(connNode, ssi);
               addRbtConnToVector(connections, rbconn, firstRbtNode ,true);
               //connections.push_back(rbconn);
            }    
         }
      }
      

      // Find the exits 
      for(uint32 i = 0; i < firstRbtNode->getNbrConnections() ; i++){
         OldConnection* oConn = firstRbtNode->getEntryConnection(i);
         OldConnection* conn = m_map->getOpposingConnection(
            oConn, firstRbtNode->getNodeID());
         if((conn != NULL) && !conn->isMultiConnection() &&
            //(conn->getConnectFromNode() != startNode->getNodeID()) &&
            (conn->getTurnDirection() == ItemTypes::EXIT_ROUNDABOUT) &&
            (conn->isVehicleAllowed(ItemTypes::passengerCar))){
            OldNode* connNode = m_map->nodeLookup(
            TOGGLE_UINT32_MSB(oConn->getConnectFromNode()));
            OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
            (m_map->itemLookup(
               REMOVE_UINT32_MSB(oConn->getConnectFromNode())));
            rbtconn_t rbconn;
            rbconn.conn = conn;
            rbconn.exit = true;
            rbconn.distance = 0;
            rbconn.multiDig = (ssi->isMultiDigitised() ||
                                ssi->isRamp() || ssi->isControlledAccess());
            rbconn.angle = getAdjustedAngle(ssi->getGfxData(),
                                            connNode->isNode0(),
                                            m_printOut);
            rbconn.index = 255;
            rbconn.valid = true; //rbtAngleIsValid(connNode, ssi);
            addRbtConnToVector(connections, rbconn, firstRbtNode ,true);
            //connections.push_back(rbconn);
         }
      }
      if(secondOutOfNode != NULL){
         for(uint32 i = 0; i < secondRbtNode->getNbrConnections() ; i++){
            OldConnection* oConn = secondRbtNode->getEntryConnection(i);
            OldConnection* conn = m_map->getOpposingConnection(
               oConn, secondRbtNode->getNodeID());
            if((conn != NULL) && !conn->isMultiConnection() &&
               //(conn->getConnectFromNode() != startNode->getNodeID()) &&
               (conn->getTurnDirection() == ItemTypes::EXIT_ROUNDABOUT) &&
               (conn->isVehicleAllowed(ItemTypes::passengerCar))){
               OldNode* connNode = m_map->nodeLookup(
                  TOGGLE_UINT32_MSB(oConn->getConnectFromNode()));
               OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
                  (m_map->itemLookup(
                     REMOVE_UINT32_MSB(oConn->getConnectFromNode())));
               rbtconn_t rbconn;
               rbconn.conn = conn;
               rbconn.exit = true;
               rbconn.distance = 0;
               rbconn.multiDig = (ssi->isMultiDigitised() ||
                                   ssi->isRamp() || ssi->isControlledAccess());
               rbconn.angle = getAdjustedAngle(ssi->getGfxData(),
                                               connNode->isNode0(),
                                               m_printOut);
               rbconn.index = 255;
               rbconn.valid = true; //rbtAngleIsValid(connNode, ssi);
               addRbtConnToVector(connections, rbconn, firstRbtNode ,true);
               //connections.push_back(rbconn);
            }
         }
      }
      bool firstBackwards = false;
      if((secondOutOfNode == NULL) &&
         ((h && (firstRbtNode->getEntryRestrictions() == ItemTypes::noWay)) ||
          (!h && (firstOutOfNode->getEntryRestrictions()
                  == ItemTypes::noWay)))){
         firstBackwards = true;
      }

      
      
      uint32 index = (uint32(exits)-1)% 4;
      nodeLength[index] += firstLength;
      // Make recursive calls.
      Crossing firstCrossing(firstOutOfNode, m_map);
      firstCrossing.setTurnDescriptions();
      setKind = firstCrossing.setRoundaboutCrossingKind(secondRbtNode,
                                                        exits,
                                                        1,
                                                        nodeLength,
                                                        firstLength,
                                                        connections,
                                                        firstBackwards);
      
      
      
      if((setKind == ItemTypes::UNDEFINED_CROSSING ) && !m_noRoundaboutish  &&
         (exits < 20) && (secondOutOfNode != NULL)){
         int32 extraDist = 0-secondLength;
         Crossing secondCrossing(secondOutOfNode, m_map);
         secondCrossing.setTurnDescriptions();
         secondCrossing.setRoundaboutCrossingKind(firstOutOfNode,
                                                  exits,
                                                  2, // Not really true but...
                                                  nodeLength, // will not use
                                                  extraDist,
                                                  connections,
                                                  true);
         setKind = ItemTypes::UNDEFINED_CROSSING;
         firstLength -= extraDist;
      }
      
      // set in all connections of this crossing.
      for(int j = 0 ; j < m_numberOfNodes ; j++){
         for(int i = 0 ; i <  m_nodes[j]->getNbrConnections(); i++)
         {
            OldConnection* conn = m_nodes[j]->getEntryConnection(i);
            if(!conn->isMultiConnection()){
               conn->setCrossingKind(setKind);
            }
            
         }
      }
      uint32 circumference = firstLength;
      // reset the exitCounts of the roundabout entries & exits to be able
      // to detect symmetry.
      if((setKind != ItemTypes::CROSSING_4ROUNDABOUT)){
         int nbrSet =
            setRoundAboutConnectionSymmetry(connections,
                                            circumference);
         
         if((nbrSet == 0)) {
            //cerr << "Three way roundabout symmetry was NOT set! (" << nbrSet 
            //     << ")  at node : " << m_nodes[0]->getNodeID() << endl;
         }
      }
   connections.clear();
   return true;
   }

}


ItemTypes::crossingkind_t
Crossing::setRoundaboutCrossingKind(OldNode* endNode,
                                    int &exits,
                                    int calls,
                                    uint32 nodeLength[4],
                                    int32 &distance,
                                    rbtconnvector_t &connections,
                                    bool backward )
{
   ItemTypes::crossingkind_t returnKind = ItemTypes::UNDEFINED_CROSSING;
   // Some tests.
   if(m_noRoundabout && m_noRoundaboutish)
      return returnKind;
   if(m_numberOfNodes < 2){
      //cerr << "ERROR to few nodes (<2) at node : "
      //     << m_nodes[0]->getNodeID() << endl;
      if(ROUNDABOUT_DUMP){
         //cout << "NODE: " << m_map->getMapID() << "."
         //     << m_nodes[0]->getNodeID() << ";" << endl;
      }
      return returnKind;
   }
   // Ok lets go
   else {
      // In this method we know that we are starting with a rbt(ish) node.
      
      //bool h = m_map->driveOnRightSide();
      int foundExits = 0;
      
      // The first of the rbt(ish) connections leading out of the crossing.
      OldNode* firstRbtNode = m_nodes[0];
      int32 firstLength = 0;
      
      // The second rbt(ish) node. . 
      OldNode* secondRbtNode = NULL;
      

      // Lets check the connections to find exits and nodes.
      for (int i = 1 ; i < (int)m_conNotices.getSize(); i++){
         CrossingConnectionElement* con =
            static_cast<CrossingConnectionElement*>(m_conNotices[i]);
         
         if((!con->m_roundabout) && (!con->m_roundaboutish) && 
            (con->m_passNodeRestrictions == ItemTypes::noRestrictions)){
            // We found an exit!
            foundExits ++;
         }
         else if ((!con->m_roundabout) && (!con->m_roundaboutish) && 
                  (con->m_passNodeRestrictions == ItemTypes::noThroughfare))
         {
            foundExits++;
            // We found a restricted exit!
            if(NO_THROUGHFARE_DUMP)
               cout << "NODE: " << m_map->getMapID() << "."
                    << m_nodes[0]->getNodeID() << ";" << endl;
         }
         
         else if ((con->m_roundabout) ||  (con->m_roundaboutish)){
            //  roundabout(ish) connection found.
            
            if(secondRbtNode == NULL){
               secondRbtNode = m_nodes[con->m_nodeIndex];
               firstLength = con->m_length;
            }
            else {
               // We dont want more than two rbt(ish) connections.
               cerr << "ERROR IN ROUNDABOUT !!" << endl;
               cerr << "To many  roundabout(ish) connections found" << endl;
               cerr << m_nodes[0]->getNodeID() << endl;
               if(ROUNDABOUT_DUMP){
                  cout << "NODE: " << m_map->getMapID() << "."
                       << m_nodes[0]->getNodeID() << ";" << endl;
               }
               return returnKind;  
            }
         }
         else {
            // No exit allowed at this con. Ignored here.
         }
         
      }
      // We only count one exit per crossing.
      if (foundExits > 0)
         exits++;

      uint32 index = (uint32(exits)-1)% 4;
      nodeLength[index] += firstLength;
      
      // Add rbtstructs to vector
      // Find entries and exits in this crossing.
      //OldNode* firstOutOfNode = m_map->nodeLookup(
      // TOGGLE_UINT32_MSB(firstRbtNode->getNodeID()));
      
      OldNode* secondOutOfNode = NULL;
      if(secondRbtNode != NULL){
         secondOutOfNode = m_map->nodeLookup(
            TOGGLE_UINT32_MSB(secondRbtNode->getNodeID()));
      }
      

      
      // Find entries for first node.
      for(uint32 i = 0; i < firstRbtNode->getNbrConnections() ; i++){
         OldConnection* conn = firstRbtNode->getEntryConnection(i);
         if(!conn->isMultiConnection() &&
            (conn->getTurnDirection() == ItemTypes::ENTER_ROUNDABOUT) &&
            (conn->isVehicleAllowed(ItemTypes::passengerCar))){
            OldNode* connNode = m_map->nodeLookup(
            TOGGLE_UINT32_MSB(conn->getConnectFromNode()));
            OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
            (m_map->itemLookup(REMOVE_UINT32_MSB(conn->getConnectFromNode())));
            rbtconn_t rbconn ;
            rbconn.conn = conn;
            rbconn.exit = false;
            rbconn.multiDig = (ssi->isMultiDigitised() ||
                               ssi->isRamp() || ssi->isControlledAccess());
            rbconn.distance = distance;
            rbconn.angle = getAdjustedAngle(ssi->getGfxData(),
                                            connNode->isNode0(),
                                            m_printOut);
            rbconn.index = 255;
            rbconn.valid = true; // rbtAngleIsValid(connNode, ssi);
            addRbtConnToVector(connections, rbconn,
                               firstRbtNode ,!backward);
            /*
            if(backward){
                  cerr << "Adding conn first" << endl;
                  connections.insert(connections.begin(), rbconn);
            }
            else
               connections.push_back(rbconn);
            */
         }    
      }
      // Find entries for second node.
      if(secondRbtNode != NULL){
         for(uint32 i = 0; i < secondRbtNode->getNbrConnections() ; i++){
            OldConnection* conn = secondRbtNode->getEntryConnection(i);
            if(!conn->isMultiConnection() &&
               (conn->getTurnDirection() == ItemTypes::ENTER_ROUNDABOUT) &&
               (conn->isVehicleAllowed(ItemTypes::passengerCar))){
               OldNode* connNode = m_map->nodeLookup(
                  TOGGLE_UINT32_MSB(conn->getConnectFromNode()));
               OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
                  (m_map->itemLookup(
                     REMOVE_UINT32_MSB(conn->getConnectFromNode())));
               rbtconn_t rbconn;
               rbconn.conn = conn;
               rbconn.exit = false;
               rbconn.distance = distance;
               rbconn.multiDig = (ssi->isMultiDigitised() ||
                                   ssi->isRamp() || ssi->isControlledAccess());
               rbconn.angle = getAdjustedAngle(ssi->getGfxData(),
                                               connNode->isNode0(),
                                               m_printOut);
               rbconn.index = 255;
               rbconn.valid = true; // rbtAngleIsValid(connNode, ssi);
               addRbtConnToVector(connections, rbconn,
                                  firstRbtNode ,!backward);
               /*
               if(backward){
                  cerr << "Adding conn first" << endl;
                  connections.insert(connections.begin(), rbconn);
               }
               
               else
                  connections.push_back(rbconn);
               */
            }    
         }
      }

      // Find the exits 
      for(uint32 i = 0; i < firstRbtNode->getNbrConnections() ; i++){
         OldConnection* oConn = firstRbtNode->getEntryConnection(i);
         OldConnection* conn = m_map->getOpposingConnection(
            oConn, firstRbtNode->getNodeID());
         
         if((conn != NULL) && !conn->isMultiConnection() &&
            //(conn->getConnectFromNode() != startNode->getNodeID()) &&
            (conn->getTurnDirection() == ItemTypes::EXIT_ROUNDABOUT)&&
            (conn->isVehicleAllowed(ItemTypes::passengerCar))){
            OldNode* connNode = m_map->nodeLookup(
            TOGGLE_UINT32_MSB(oConn->getConnectFromNode()));
            OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
            (m_map->itemLookup(
               REMOVE_UINT32_MSB(oConn->getConnectFromNode())));
            rbtconn_t rbconn;
            rbconn.conn = conn;
            rbconn.exit = true;
            rbconn.distance = distance;
            rbconn.multiDig = (ssi->isMultiDigitised() ||
                                ssi->isRamp() || ssi->isControlledAccess());
            rbconn.angle = getAdjustedAngle(ssi->getGfxData(),
                                            connNode->isNode0(),
                                            m_printOut);
            rbconn.index = 255;
            rbconn.valid = true; //rbtAngleIsValid(connNode, ssi);
            addRbtConnToVector(connections, rbconn,
                               firstRbtNode ,!backward);
            /*
            if(backward){
                  cerr << "Adding conn first" << endl;
                  connections.insert(connections.begin(), rbconn);
            }
            
            else
               connections.push_back(rbconn);
            */
         }
      }
      if(secondOutOfNode != NULL){
         for(uint32 i = 0; i < secondRbtNode->getNbrConnections() ; i++){
            OldConnection* oConn = secondRbtNode->getEntryConnection(i);
            OldConnection* conn = m_map->getOpposingConnection(
               oConn, secondRbtNode->getNodeID());
            if((conn != NULL) && !conn->isMultiConnection() &&
               //(conn->getConnectFromNode() != startNode->getNodeID()) &&
               (conn->getTurnDirection() == ItemTypes::EXIT_ROUNDABOUT) &&
               (conn->isVehicleAllowed(ItemTypes::passengerCar))){
               OldNode* connNode = m_map->nodeLookup(
                  TOGGLE_UINT32_MSB(oConn->getConnectFromNode()));
               OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
                  (m_map->itemLookup(
                     REMOVE_UINT32_MSB(oConn->getConnectFromNode())));
               rbtconn_t rbconn;
               rbconn.conn = conn;
               rbconn.exit = true;
               rbconn.multiDig = (ssi->isMultiDigitised() ||
                                   ssi->isRamp() || ssi->isControlledAccess());
               rbconn.distance = distance;
               rbconn.angle = getAdjustedAngle(ssi->getGfxData(),
                                               connNode->isNode0(),
                                               m_printOut);
               rbconn.index = 255;
               rbconn.valid = true; //rbtAngleIsValid(connNode, ssi);
               addRbtConnToVector(connections, rbconn,
                                  firstRbtNode ,!backward);
               /*
               if(backward){
                  cerr << "Adding conn first" << endl;
                  connections.insert(connections.begin(), rbconn);
               }
               else
                  connections.push_back(rbconn);
               */
            }
         }
      }
      if(backward)
         distance -= firstLength;
      else
         distance += firstLength;

      // Check if time to break.
      if((secondOutOfNode == endNode) || (secondOutOfNode == NULL) ||
         (calls > 30) || (exits > 20)){
         if((calls > 30) || (secondOutOfNode == NULL)) {
            exits = 10;
         }
         switch (exits)
         {
            case 2 :
            {
               returnKind = ItemTypes::CROSSING_2ROUNDABOUT;
               break;
            }
            case 3 :
            {
               returnKind = ItemTypes::CROSSING_3ROUNDABOUT;
               break;
            }
            case 4 :
            {
               if (isSymetricRoundAbout(nodeLength, MAX_ROUNDABOUT_ERROR))
                  returnKind = ItemTypes::CROSSING_4ROUNDABOUT;
               else
               {
                  returnKind = ItemTypes::CROSSING_4ROUNDABOUT_ASYMMETRIC;
               }
               break;
            }
            case 5 :
            {
               returnKind = ItemTypes::CROSSING_5ROUNDABOUT;
               break;
            }
            case 6 :
            {
               if(ROUNDABOUT_DUMP){
                  cout << "NODE: " << m_map->getMapID() << "."
                       << m_nodes[0]->getNodeID() << ";" << endl;
            }            
               returnKind = ItemTypes::CROSSING_6ROUNDABOUT;
               break;
            }
            case 7 :
            {
               if(ROUNDABOUT_DUMP){
                  cout << "NODE: " << m_map->getMapID() << "."
                       << m_nodes[0]->getNodeID() << ";" << endl;
               }            
               returnKind = ItemTypes::CROSSING_7ROUNDABOUT;
               break;
            }
            default :
            {
               if(!m_noRoundaboutish && (secondOutOfNode == NULL)){

// cerr << "Reached end of this side of a rbish" << endl;
               }
               
               else if((secondOutOfNode == endNode) && !backward){
                  returnKind = ItemTypes::CROSSING_7ROUNDABOUT;
                  cerr << "Warning very large roundabout in map nr:"
                       << m_map->getMapID()<< " !" << endl;
                  cerr << exits << " exits found. " << endl;
                  cerr <<"StartNode = " << m_nodes[0]->getNodeID()
                       << endl;
               } else {
                  returnKind = ItemTypes::UNDEFINED_CROSSING;
                  cerr << "Warning undefined roundabout in map nr:"
                       << m_map->getMapID()<< " !" << endl;
                  cerr << exits << " exits found. " << endl;
                  cerr <<"StartNode = " << m_nodes[0]->getNodeID()
                       << endl;
               }
               if(ROUNDABOUT_DUMP){
                  cout << "NODE: " << m_map->getMapID() << "."
                       << m_nodes[0]->getNodeID() << ";" << endl;  
               }
               break;
            }
         }
         
      } else {
         // Continue to next crossing.
         Crossing nextCrossing(secondOutOfNode, m_map);
         nextCrossing.setTurnDescriptions();
         // Make recursive call.
         calls++;
         returnKind = nextCrossing.setRoundaboutCrossingKind(endNode,
                                                             exits,
                                                             calls,
                                                             nodeLength,
                                                             distance,
                                                             connections,
                                                             backward);

      }
      if((m_noRoundabout) && (returnKind != ItemTypes::CROSSING_4ROUNDABOUT)){
         //cerr << "Roundaboutish, dont set rbt-crossingkind" << endl;
         returnKind = ItemTypes::UNDEFINED_CROSSING;
      }
      
      
      // set in all connections in this crossing.
      for(int j = 0 ; j < m_numberOfNodes ; j++)
         for(int i = 0 ; i < m_nodes[j]->getNbrConnections(); i++)
         {
            OldConnection* conn = m_nodes[j]->getEntryConnection(i);
            if(!conn->isMultiConnection())
               conn->setCrossingKind(returnKind);
         }
   
      return returnKind;  
   }
}



bool
Crossing::isSymetricRoundAbout(uint32 nodeLength[4], float64 maxError)
{
   nodeLength[1] +=  nodeLength[0];
   nodeLength[2] +=  nodeLength[1];
   nodeLength[3] +=  nodeLength[2];
   float64 errorVec[4];
   for (int i = 0; i < 4 ; i++)
   {
      errorVec[i] = (float64(nodeLength[i])/
                     float64(nodeLength[3]))
         -0.25 * (i+1);
   }
   if((fabs(errorVec[1])+fabs(errorVec[2] - errorVec[0])) > maxError)
   {
      return false;
   }
   else
   {
      return true;
   }
}

int
Crossing::setRoundAboutConnectionSymmetry(rbtconnvector_t connections,
                                          uint32 circumference)
{
   //cerr << " ---------------------------" << endl;
   if(connections.size() > m_maxNbrNodesInACrossing) {
      //cerr << " To many connections, " << connections.size()
      //     << ", no symmetry" << endl;
      return 0;
   }
   
   // Merge the multidig entries and exits to get new angle value
   bool rightTraff = m_map->driveOnRightSide();
   

   // The number of single or double connections to the roundabout.
   int16 nbrConnectPoint = 0;
   int nbrEntries = 0;
   int nbrExits   = 0;
   
   rbtconnvector_t::iterator ri;
   int32 lastExit   = MAX_INT32;
   int32 lastEntry  = MAX_INT32;
   byte exitCount[m_maxNbrNodesInACrossing];
   byte entryCount[m_maxNbrNodesInACrossing];
   for(int i = 0 ; i < 8 ; i++){
      exitCount[i] = 0;
      entryCount[i] = 0;
   }
   
   bool lastMulti = false;
   bool firstMulti = false;

   //  Theese are needed as ramps are treated as multidigs.
   bool firstWasExit = false;
   bool lastWasExit = false;
   
   uint32 multiDist = 0;
   float64 angles[m_maxNbrNodesInACrossing];
   
   
   for(ri = connections.begin(); ri != connections.end() ; ri++){
      if(ri->valid == false && !ri->multiDig){
         cerr << "Roundabout connecting ssi too short, aborting symmetry"
              << endl;
         return 0;
      }
      
      //rbtconn_t* conn = static_cast<rbtconn_t*>
      bool isLast = (uint32(nbrEntries+ nbrExits) == connections.size()-1);
      int32 thisDist = ri->distance;
      /*
        cerr << "Thisdist : " << thisDist << ", ";
        if(ri->multiDig)
            cerr << "M, ";
            if(lastMulti)
            cerr << "lM, ";
            if(isLast)
            cerr << "L, ";
            if(firstMulti)
            cerr << "FM, ";
            cerr << "angle = " << ri->angle << ", ";
      */
      if(nbrConnectPoint > 11){ 
         cerr << " nbrConnectPoint : " << nbrConnectPoint;
         return 0;
      }
      
            
      if(ri->exit){
         nbrExits++;
         
         // Same as last entry.
         if(lastEntry == thisDist){
            if(nbrConnectPoint == 0){
               cerr << " nbrConnectPoint2 : " << nbrConnectPoint;
               abort();
            }
            
            ri->index = nbrConnectPoint-1;
            lastMulti = false; // As it is already "used".
         }
         // Same as multiDig entry at other distance
         else if(ri->multiDig  && !rightTraff && lastMulti && !lastWasExit){
            if(nbrConnectPoint == 0){
               cerr << " nbrConnectPoint3 : " << nbrConnectPoint;
               abort();
            }
            ri->index = nbrConnectPoint-1;

            
            if(((angles[nbrConnectPoint-1]-ri->angle) > M_PI ) ||
               ((ri->angle - angles[nbrConnectPoint-1]) > M_PI )){
               angles[nbrConnectPoint-1] += 2*M_PI;
            }
            angles[nbrConnectPoint-1] = (angles[nbrConnectPoint-1]
                                         + ri->angle)/2;
         }
         // Same as multiDig entry first in vector.
         else if (ri->multiDig  && rightTraff && firstMulti && isLast &&
                  !firstWasExit){
            if(nbrConnectPoint == 0){
               cerr << " nbrConnectPoint4 : " << nbrConnectPoint;
               abort();
            }
            ri->index = 0;
            
            if(((angles[0]-ri->angle) > M_PI ) ||
               ((ri->angle - angles[0]) > M_PI )){
               angles[0] += 2*M_PI;
            }
            angles[0] = (angles[0] + ri->angle)/2;
            lastMulti = false; // As it is already "used".
         }
         
         // Not part of earlier connection points. New connection here
         else {
            
            // Did we have to "leave" a unmearged first multi?
            if (lastMulti && (nbrEntries + nbrExits == 2)){
               lastMulti = false;
               firstMulti = true;
               firstWasExit = true;
            }
            
            lastMulti = ri->multiDig;
            
            if(lastMulti)
               multiDist = ri->distance;
            nbrConnectPoint++;
            angles[nbrConnectPoint-1] = ri->angle;
            ri->index = nbrConnectPoint-1;
         }
         lastExit = thisDist;
         lastWasExit = true;
      } else {
         nbrEntries++;
         // Same as last exit.
         if(lastExit == thisDist){
            if(nbrConnectPoint == 0){
               cerr << " nbrConnectPoint5 : " << nbrConnectPoint;
               abort();
            }

            ri->index = nbrConnectPoint-1;
            lastMulti = false; // As it is already "used".
            //if(!m_noRoundaboutish)
             //  cerr << "old entry" << nbrConnectPoint-1 << ", " << endl;
         }
         // Same as multiDig exit at other distance
         else if(ri->multiDig  && rightTraff && lastMulti && lastWasExit){
            
            
            ri->index = nbrConnectPoint-1;
            lastMulti = false; // As it is already "used".
            if(((angles[nbrConnectPoint-1]-ri->angle) > M_PI ) ||
               ((ri->angle - angles[nbrConnectPoint-1]) > M_PI )){
               angles[nbrConnectPoint-1] += 2*M_PI;
            }
            angles[nbrConnectPoint-1] = (angles[nbrConnectPoint-1]
                                         + ri->angle)/2;

         }
         // Same as multiDig exit first in vector.
         else if (ri->multiDig  && !rightTraff && firstMulti && isLast &&
                  firstWasExit){
            ri->index = 0;
            if(((angles[0]-ri->angle) > M_PI ) ||
               ((ri->angle - angles[0]) > M_PI )){
               angles[0] += 2*M_PI;
            }
            angles[0] = (angles[0] + ri->angle)/2;
            lastMulti = false; // As it is already "used".
         }
         
         // Not part of earlier connection points. New connection here
         else {
            // Did we have to "leave" a unmearged first multi?
            if (lastMulti && (nbrEntries + nbrExits == 2)){
               lastMulti = false;
               firstMulti = true;
               firstWasExit = false;
            }
            
            lastMulti = ri->multiDig;
            
            if(lastMulti)
               multiDist = ri->distance;
            angles[nbrConnectPoint] = ri->angle;
            nbrConnectPoint++;
            ri->index = nbrConnectPoint-1;
         }
         lastEntry = thisDist;
         lastWasExit = false;
      }
      
   }
   //cerr << endl;

   // Change angles so the first is 0 deg.
   // cerr << " ROUNDABOUT";
  //  if(m_noRoundabout){
   //    cerr << " ROUNDABOUT";
   //    cerr << "ISH";
  //     cerr << " at node : " << m_nodes[0]->getNodeID() << endl;
  //     cerr << " Nbr conn points : " << nbrConnectPoint << endl;
  //     cout << "NODE: " << m_map->getMapID() << "."
  //          << m_nodes[0]->getNodeID() << ";" << endl;
  //  }
   
   if(nbrConnectPoint > 8){
      //if(m_noRoundabout)
      //   cerr << "To many connections, " << nbrConnectPoint
      //        << " no symmetry set" << endl;
      return 0;
   }

   float64 factor = angles[0];
   for(int i = 0 ; i < nbrConnectPoint ; i++){
      angles[i] = factor - angles[i];
      if(angles[i] < 0){
         angles[i] += M_PI*2;
      }
      //if(m_noRoundabout)
      //cerr << "Angle " << i << " : " << angles[i] << endl;
   }

   
   // Decide the exitCout values to use

   switch(nbrConnectPoint){ 
      case 2:
         if(angles[1]< 4 *M_PI_6){
            exitCount[0]  = 1; 
            entryCount[0] = 1;
            exitCount[1]  = 2;
            entryCount[1] = 2;            
         }
         else if(angles[1] > 8* M_PI_6){
            exitCount[0]  = 1; 
            entryCount[0] = 1;
            exitCount[1]  = 4;
            entryCount[1] = 4;
         }
         else{
            exitCount[0]  = 1; 
            entryCount[0] = 1;
            exitCount[1]  = 3;
            entryCount[1] = 3;
         }
         break;
      case 3: 
         // Three way roundabout. 8 diffrent cases.
         
         // Y-way roundabout with only ahead to first exit.
         if((angles[1] >= 5.5*M_PI_6) && (angles[2] <= 6.5*M_PI_6)){
            // Disabling Y-roundabouts using exit count instead
            /*
            //cerr << "Y-rbt" << endl;
            exitCount[0]  = 1;  
            entryCount[0] = 1;
            exitCount[1]  = 2; 
            entryCount[1] = 3; 
            exitCount[2]  = 4; 
            entryCount[2] = 3;
            */
         }
         
         // Y-way roundabout with only ahead to second exit.
         else if((angles[1] >= 5.5*M_PI_6) &&  (angles[2] <= 10.5*M_PI_6) &&
                 ((angles[2]-angles[1]) >= 5.5*M_PI_6)){
            // Disabling Y-roundabouts using exit count instead
            //cerr << "Y-rbt" << endl;
            /*
            exitCount[0]  = 4; 
            entryCount[0] = 3; 
            exitCount[1]  = 1; 
            entryCount[1] = 1; 
            exitCount[2]  = 2;
            entryCount[2] = 3; 
            */
         }
         // Y-way roundabout with only ahead to third exit.
         else if((angles[1] <= 1.5*M_PI_6) && (angles[2] <= 8*M_PI_6) &&
                 (angles[2]-angles[1]) >= 5*M_PI_6){
            //cerr << "Y-rbt" << endl;
            // Disabling Y-roundabouts using exit count instead
            /*
            exitCount[0]  = 2; 
            entryCount[0] = 3;
            exitCount[1]  = 4;
            entryCount[1] = 3;
            exitCount[2]  = 1;
            entryCount[2] = 1;
            */
         }
         // Y-way roundabout crossing with no aheads.
         // Bit difficult and 1 special exitcount and 1 special
         // entrycount are used.
         else if((angles[1] < 4.5*M_PI_6) && (angles[1] > 1 *M_PI_6) &&
                 ((angles[2]-angles[1]) < 4.5*M_PI_6) &&
                 (angles[2] > 7.5 * M_PI_6)){
            // Disabling Y-roundabouts using exit count instead
            //cerr << "Yy-rbt ***** Y_RBT " << endl;
            //cerr << " angle 0 : " << angles[0] << endl;
            //cerr << " angle 1 : " << angles[1] << endl;
            //cerr << " angle 2 : " << angles[2] << endl;
            //cerr << " angle 2-1 : " << angles[2]- angles[1] << endl;
            //cerr << " 2*pi-2 : " << 2* M_PI - angles[2] << endl;
            /*
            exitCount[0]  = 6; // Special value
            entryCount[0] = 1;
            exitCount[1]  = 2;
            entryCount[1] = 3;
            exitCount[2]  = 4;
            entryCount[2] = 5; // Special value
            cout << "NODE: " << m_map->getMapID() << "."
                 << m_nodes[0]->getNodeID() << ";" << endl;
            */
         }
         
         // T-way crossing.
         else if((angles[1] <= 4.5*M_PI_6) && (angles[2] >= 7.5*M_PI_6) &&
                 ((angles[2]-angles[1]) > 5*M_PI_6) &&
                 (angles[1] > 1*M_PI_6) && (angles[2] < 10.5*M_PI_6)){
            //cerr << "T-rbt" << endl;
            exitCount[0]  = 1; 
            entryCount[0] = 1;
            exitCount[1]  = 2;
            entryCount[1] = 2;
            exitCount[2]  = 4;
            entryCount[2] = 4;
            //cout << "NODE: " << m_map->getMapID() << "."
            //     << m_nodes[0]->getNodeID() << ";" << endl;
         }   
         // T-way crossing.
         else if((angles[1] <= 4.5*M_PI_6) && (angles[1] > 1 *M_PI_6) &&
                 (angles[2] <= 7*M_PI_6) && (angles[2] <= 7*M_PI_6)
                 )
         {
            //cerr << "T-rbt" << endl;
            exitCount[0]  = 1; 
            entryCount[0] = 1;
            exitCount[1]  = 2;
            entryCount[1] = 2;
            exitCount[2]  = 3;
            entryCount[2] = 3;
            //cout << "NODE: " << m_map->getMapID() << "."
            //     << m_nodes[0]->getNodeID() << ";" << endl;
         }   
         // T-way crossing.
         else if((angles[2] >= 7.5*M_PI_6) &&
                 (angles[1] >= 5*M_PI_6) &&
                 ((angles[2]-angles[1]) < 4 *M_PI_6))
         {
            //cerr << "T-rbt" << endl;
            exitCount[0]  = 1; 
            entryCount[0] = 1;
            exitCount[1]  = 3;
            entryCount[1] = 3;
            exitCount[2]  = 4;
            entryCount[2] = 4;
            //cout << "NODE: " << m_map->getMapID() << "."
            //     << m_nodes[0]->getNodeID() << ";" << endl;
         }   

         // Unknown
         else{
            //cerr << "WAS NOT ABLE TO SET EXIT/ENTRY COUNT" << endl;
            // All exitcounts
            exitCount[0]  = 0; 
            entryCount[0] = 0;
            exitCount[1]  = 0;
            entryCount[1] = 0;
            exitCount[2]  = 0;
            entryCount[2] = 0;
             
         } 
         break;
      case 4:
         if((angles[1] > 5* M_PI_6) || (angles[1] < 1* M_PI_6) ||
            ((angles[2]-angles[1]) > 5* M_PI_6) ||
            ((angles[2]-angles[1]) < 1* M_PI_6) ||
            ((angles[3]-angles[2]) > 5* M_PI_6) ||
            ((angles[3]-angles[2]) < 1* M_PI_6) ||
            (angles[3] < 7* M_PI_6) || (angles[3] > 11* M_PI_6)) {
         }
         else{
         // Four way roundabout
            exitCount[0]  = 1; 
            entryCount[0] = 1;
            exitCount[1]  = 2;
            entryCount[1] = 2;
            exitCount[2]  = 3;
            entryCount[2] = 3;
            exitCount[3]  = 4;
            entryCount[3] = 4;
            
         }
         
            break;
      default:
         break;
   }
   for(ri = connections.begin(); ri != connections.end() ; ri++){
      if(ri->index !=  255){
         if(ri->exit){
            ri->conn->setExitCount(exitCount[ri->index]);
            // cerr << "   Setting exit to  " << (int)exitCount[ri->index]
            //      << " (from index " << ri->index << " ). " << endl;
         } else {
            ri->conn->setExitCount(entryCount[ri->index]);
            // cerr << "   Setting entry to " << (int)exitCount[ri->index]
            //      << " (from index " << ri->index << " ). " << endl;
         }
      } else {
         cerr << " WARNING error in rbtconn_t.index " << endl;
      }
      
   }
    return nbrConnectPoint; 
}


bool
Crossing::isPartOfMultidigCrossing()
{   
   for (int i = 0 ; i < (int)m_conNotices.getSize(); i++){
      CrossingConnectionElement* con =
            static_cast<CrossingConnectionElement*>(m_conNotices[i]);
      if(con->m_ismulti)
         return true;
   }
   
   return false;
}


ItemTypes::crossingkind_t
Crossing::getMultidigCrossingKind(OldNode* &endNode)
{
   // Check which connections that should be checked
   // if 1 & 3 is multi check 2 & 4
   // if 2 & 4 is multi check 1 & 3
   endNode = NULL;
   // Check the crossings in the end of the segments to be checked.
   // If any of them is 3-way, set 3-way else 4-way.
   int trueConns = 0;
   for (int i = 0 ; i < (int)m_conNotices.getSize(); i++){
      CrossingConnectionElement* con =
         static_cast<CrossingConnectionElement*>(m_conNotices[i]);
      if(con->m_length > 35){
         // This must be a true con.
         trueConns++;
      }
      // Have to check the other end
      else {
         uint32 oppNodeID = TOGGLE_UINT32_MSB(
            m_nodes[con->m_nodeIndex]->getNodeID());
         OldNode* oppNode =  m_map->nodeLookup(oppNodeID);
         if(oppNode != NULL){
            Crossing newCrossing(oppNode, m_map);
            if(newCrossing.continueOtherSide()){
               trueConns++;
            } else {
               //  cerr << "Dont continue : " << oppNodeID << endl;
               // Setting exitCount to MAX_BYTE in connections leading to
               // this node.
               endNode = m_nodes[con->m_nodeIndex];
               // cerr << " Settting exitCount to MAX_BYTE for node "
               //      << endNode->getNodeID() << endl;
               
               for(uint16 i = 0; i < endNode->getNbrConnections(); i++){
                  endNode->getEntryConnection(i)->setExitCount(MAX_BYTE);
               }
            }
         }
      }
      
    }
   
   ItemTypes::crossingkind_t setKind;
   if(trueConns == 3){
      setKind = ItemTypes::CROSSING_3WAYS_T;
      // cerr << "Setting 3way crossing at node "
      //      << m_nodes[0]->getNodeID() << endl;
   } else {// if(trueConns == 4)
      setKind = ItemTypes::CROSSING_4WAYS;
      // cerr << "Setting 4way. trueConns = " << trueConns 
      //     << ", node :  " << m_nodes[0]->getNodeID() << endl;
   }
   
   return setKind;
}

bool
Crossing::continueOtherSide()
{
   if(m_conNotices.getSize() == 1){
      return true;
   }
   if(m_conNotices.getSize() == 2){
      if((static_cast<CrossingConnectionElement*>(m_conNotices[0])->m_length +
         static_cast<CrossingConnectionElement*>(m_conNotices[1])->m_length)>
         35){
         return true;
      }
      OldNode* nextNode = m_map->nodeLookup(
         TOGGLE_UINT32_MSB(m_nodes[1]->getNodeID()));
      if(nextNode == NULL)
         return true;
      Crossing newCrossing(nextNode, m_map);
      return newCrossing.continueOtherSide();
   }
   if(isPartOfMultidigCrossing()){
      bool found = false;
      int nbrMult = 0;
      for (int i = 1 ; i < (int)m_conNotices.getSize(); i++){
         CrossingConnectionElement* con =
         static_cast<CrossingConnectionElement*>(m_conNotices[i]);
         if(con->m_ismulti){
            nbrMult++;
         }
         else if(nbrMult == 1){
            found = true;
         }
      }
      if(nbrMult != 2)
         found = true;
      return found;
   }
   return true;
}

bool
Crossing::rbtAngleIsValid(OldNode* connNode, OldStreetSegmentItem* connItem)
{ /*
   if(connItem->getLength() < m_maxNbrNodesInACrossing){
      return false;
      }*/
   return true;
}

void
Crossing::addRbtConnToVector(rbtconnvector_t &vector,
                             rbtconn_t conn,
                             OldNode* rbtNode,
                             bool last)
{
   rbtconnvector_t::iterator vi;
   bool notFound = true;
   bool loop = true;
   
   if(vector.empty()){
      // No problem!
      vector.push_back(conn);
   } else if(last){
      vi = vector.end();
      vi--;
      do{
         if(conn.distance != vi->distance){
            notFound = false;
         } else {
            // Check angles
            OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
               (m_map->itemLookup(REMOVE_UINT32_MSB(rbtNode->getNodeID()))); 
            float64 rbtAngle = getAdjustedAngle(ssi->getGfxData(),
                                                rbtNode->isNode0(),
                                                m_printOut);
            float64 angleOld = rbtAngle - vi->angle;
            if(angleOld < 0){
               angleOld += M_PI*2;
            }
            
            float64 angleNew = rbtAngle - conn.angle;
            if(angleNew < 0){
               angleNew += M_PI*2;
            }
            if(angleNew >= angleOld){
               notFound = false;
            }
            else{
               if(vi == vector.begin()){
                  loop  = false;
               } else {
                  vi--;
               }  
            }
         }
      } while (notFound && loop);
      if(!notFound)
         vi++;
      if(vi == vector.end()){
         vector.push_back(conn);
      }
      else{
         vector.insert(vi, conn);
      }
      
      
   } else { // = ! last
      vi = vector.begin();
      while((vi != vector.end()) && notFound){
         if(conn.distance != vi->distance){
            notFound = false;
         } else {
            // Check angles
            
            OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
               (m_map->itemLookup(REMOVE_UINT32_MSB(rbtNode->getNodeID())));
            float64 rbtAngle = getAdjustedAngle(ssi->getGfxData(),
                                                rbtNode->isNode0(),
                                                m_printOut);
            float64 angleOld = rbtAngle - vi->angle;
            if(angleOld < 0){
               angleOld += M_PI*2;
            }
            
            float64 angleNew = rbtAngle - conn.angle;
            if(angleNew < 0){
               angleNew += M_PI*2;
            }
            if(angleNew <= angleOld){
               notFound = false;
            }
            else{
               vi++;
            }
         }
      }
      if(vi == vector.end()){
         vector.push_back(conn);
      }
      else{
         vector.insert(vi, conn);         
      }
      
   }
}

