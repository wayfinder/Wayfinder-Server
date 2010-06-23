/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SetTurnDescriptions.h"
#include "OldStreetSegmentItem.h"
#include "OldNode.h"
#include "OldConnection.h"
#include "GfxData.h"
#include "GfxUtility.h"
#include "NodeBits.h"
#include "Math.h"

// ================================================ ConnectionElement

ConnectionElement::ConnectionElement()
{
   init(0, NULL, 4, false, false, false, false, ItemTypes::noRestrictions);
}

ConnectionElement::ConnectionElement(float64 a, OldConnection* c)
{
   init(a, c , 4, false, false, false, false, ItemTypes::noRestrictions);
}

ConnectionElement::ConnectionElement(float64 a, OldConnection* c,
                                     byte roadClass, bool roundabout, 
                                     bool ramp, bool divided, bool ismulti,
                                     ItemTypes::entryrestriction_t passRestr)
{
   init(a, c, roadClass, roundabout, ramp, divided, ismulti, passRestr);
}

void
ConnectionElement::init(float64 a, OldConnection* c, byte roadClass,
                        bool roundabout, bool ramp, bool divided,
                        bool ismulti, 
                        ItemTypes::entryrestriction_t passNodeRestrictions)
{
   m_angle = a;
   m_con = c;
   m_roadClass = roadClass;
   m_roundabout = roundabout;
   m_ramp = ramp;
   m_divided = divided;
   m_ismulti = ismulti;
   m_passNodeRestrictions = passNodeRestrictions;
}



// ============================================== SetTurnDescriptions

SetTurnDescriptions::SetTurnDescriptions() 
   : m_connectionNotices(4)
{

}

SetTurnDescriptions::SetTurnDescriptions(OldNode* toNode, OldMap* theMap)
   : m_connectionNotices(4)
{
   setTurnDescriptions(toNode, theMap);
}

SetTurnDescriptions::~SetTurnDescriptions()
{
   // nothing to since we do not have allocated any memory
}

bool 
SetTurnDescriptions::setTurnDescriptions(OldNode* toNode, OldMap* theMap)
{
   // Reset all the members in this object
   reset(theMap);

   // Fill the m_connectionNotiece with data about all the 
   // connections that should be processed.
   fillConnectionNotices(toNode);

   // Initiate variables for the toSegment
   bool toRoundabout = ((OldStreetSegmentItem*) 
         m_map->itemLookup(toNode->getNodeID()))->isRoundabout();
   bool toRamp = ((OldStreetSegmentItem*) 
         m_map->itemLookup(toNode->getNodeID()))->isRamp();
   byte toRoadClass = ((OldStreetSegmentItem*) 
         m_map->itemLookup(toNode->getNodeID()))->getRoadClass();
   bool toIsMulti = ((OldStreetSegmentItem*) 
                   m_map->itemLookup(toNode->getNodeID()))->isMultiDigitised();
   
   // Huge switch-statement that sets the actual turndescriptions
   switch (m_connectionNotices.getSize()) {
      case 1 : {
         // No real crossing...
         ConnectionElement* curConElm = 
               (ConnectionElement*) (m_connectionNotices[0]);
         OldConnection* curCon = curConElm->m_con;
         curCon->setTurnDirection(ItemTypes::FOLLOWROAD);
      }
      break;

      case 2 : {
         // ============================================== Threewaycrossing


         // Debug-code to make it possible for gdb halt on toNodeID
         DEBUG1(
            uint32 toNodeID = toNode->getNodeID();
            if (toNodeID == MAX_UINT32) 
               cerr << "toNodeID == MAX_UINT32" << endl;
         );

         // Set some local variables to make the setting easier
         ConnectionElement* firstConElm = 
               (ConnectionElement*) (m_connectionNotices[0]);
         OldConnection* firstCon = firstConElm->m_con;
         ConnectionElement* secondConElm = 
               (ConnectionElement*) (m_connectionNotices[1]);
         OldConnection* secondCon = secondConElm->m_con;


         // Huge if () {} else if () {} ... statement that covers all
         // possible cases (!?)

         if (! m_noRoundabout) {
            // At least one of the SSIs is a roundabout.

            DEBUG8(cerr << "GMSMap Found roundabout-flag!" << endl);
            if (toRoundabout) {
               // Driving to a roundabout segment
               for (uint32 i=0; i<2; i++) {
                  ConnectionElement* curConElm = 
                     (ConnectionElement*) (m_connectionNotices[i]);
                  // Into or stay inside the roundabout
                  if (curConElm->m_roundabout) {
                     curConElm->m_con->setTurnDirection(ItemTypes::AHEAD);
                  } else if (curConElm->m_con
                              ->isVehicleAllowed(ItemTypes::passengerCar)) {
                     curConElm->m_con->setTurnDirection(
                              ItemTypes::ENTER_ROUNDABOUT);
                  } else {
                     curConElm->m_con->setTurnDirection(ItemTypes::UNDEFINED);
                  }
               }

            } else {
               if (firstCon->isVehicleAllowed(ItemTypes::passengerCar))
                  firstCon->setTurnDirection(ItemTypes::EXIT_ROUNDABOUT);
               else
                  firstCon->setTurnDirection(ItemTypes::UNDEFINED);

               if (secondCon->isVehicleAllowed(ItemTypes::passengerCar))
                  secondCon->setTurnDirection(ItemTypes::EXIT_ROUNDABOUT);
               else
                  secondCon->setTurnDirection(ItemTypes::UNDEFINED);
            }

         } else if (toIsMulti &&
                    firstConElm->m_ismulti &&
                    (!secondConElm->m_ismulti) &&
                    firstConElm->m_angle < -M_PI_2) {
            // 
            firstCon->setTurnDirection(ItemTypes::UTURN);
            secondCon->setTurnDirection(ItemTypes::AHEAD);

         } else if (toIsMulti &&
                    (!firstConElm->m_ismulti) &&
                    secondConElm->m_ismulti &&
                    secondConElm->m_angle > M_PI_2) {
            // 
            firstCon->setTurnDirection(ItemTypes::AHEAD);
            secondCon->setTurnDirection(ItemTypes::UTURN);

         } else if (firstConElm->m_ismulti &&
                    secondConElm->m_ismulti &&
                    firstConElm->m_angle > -M_PI_6*2.0 &&
                    secondConElm->m_angle < M_PI_6*2.0) {
            // one road in real world (!)
            firstCon->setTurnDirection(ItemTypes::AHEAD);
            secondCon->setTurnDirection(ItemTypes::AHEAD);
            
         } else if ( (toRoadClass == firstConElm->m_roadClass) &&
                     (toRoadClass < secondConElm->m_roadClass) &&
                     (m_nbrRamps == 0) &&
                     (m_nbrNames < 3)) {
            // If driving via firstCon we are following the big road!
            firstCon->setTurnDirection(ItemTypes::AHEAD);
            secondCon->setTurnDirection(ItemTypes::RIGHT);
         
         } else if ( (toRoadClass == secondConElm->m_roadClass) &&
                     (toRoadClass < firstConElm->m_roadClass) &&
                     (m_nbrRamps == 0) &&
                     (m_nbrNames < 3)) {
            // If driving via secondCon we are following the big road!
            firstCon->setTurnDirection(ItemTypes::LEFT);
            secondCon->setTurnDirection(ItemTypes::AHEAD);

         } else if (toRamp) {
         
            if ((firstConElm->m_roadClass == secondConElm->m_roadClass) &&
               (toRoadClass > firstConElm->m_roadClass)) {
               // Exiting motorway (mabye) ?
               firstCon->setTurnDirection(ItemTypes::OFF_RAMP);
               secondCon->setTurnDirection(ItemTypes::OFF_RAMP);
            } else if (m_nbrRamps == 1) {
               if ((firstConElm->m_angle > -2.0*M_PI_6))
                  firstCon->setTurnDirection(ItemTypes::OFF_RAMP);
               else
                  firstCon->setTurnDirection(ItemTypes::LEFT);
         
               if ((secondConElm->m_angle < 2.0*M_PI_6))
                  secondCon->setTurnDirection(ItemTypes::OFF_RAMP);
               else
                  secondCon->setTurnDirection(ItemTypes::RIGHT);
            } else if (m_nbrRamps == 3) {
               // All segments are ramps!
               if ((firstConElm->m_angle > -2.0*M_PI_6))
                  firstCon->setTurnDirection(ItemTypes::AHEAD);
               else
                  firstCon->setTurnDirection(ItemTypes::LEFT);
         
               if ((secondConElm->m_angle < 2.0*M_PI_6))
                  secondCon->setTurnDirection(ItemTypes::AHEAD);
               else
                  secondCon->setTurnDirection(ItemTypes::RIGHT);
            } else {
               // Two of the segments are ramps (toSegment + one more)
               firstCon->setTurnDirection(ItemTypes::LEFT);
               secondCon->setTurnDirection(ItemTypes::RIGHT);
            }

         
         } else if ( (toRoadClass == firstConElm->m_roadClass) &&
                     (toRoadClass < secondConElm->m_roadClass) &&
                     (secondConElm->m_ramp)) {
            // If driving via firstCon we are following the big road!
            firstCon->setTurnDirection(ItemTypes::AHEAD);
            secondCon->setTurnDirection(ItemTypes::ON_RAMP);
         
         } else if ( (toRoadClass == secondConElm->m_roadClass) &&
                     (toRoadClass < firstConElm->m_roadClass) &&
                     (firstConElm->m_ramp)) {
            // If driving via secondCon we are following the big road!
            firstCon->setTurnDirection(ItemTypes::ON_RAMP);
            secondCon->setTurnDirection(ItemTypes::AHEAD);

         } else if ( (firstConElm->m_angle <= 0) &&
                     (firstConElm->m_angle >= -M_PI_2) &&
                     (secondConElm->m_angle > M_PI_2) &&
                     (toRoadClass == firstConElm->m_roadClass) &&
                     (toRoadClass == secondConElm->m_roadClass) &&
                     (toRoadClass < ItemTypes::fourthClassRoad) &&
                     (allNameSame(toNode)) &&
                     (secondConElm->m_passNodeRestrictions == 
                        ItemTypes::noWay)) {
            // Probably found a road that is splited into two lanes
            firstCon->setTurnDirection(ItemTypes::AHEAD);
            secondCon->setTurnDirection(ItemTypes::UTURN);
         
         } else if ( (firstConElm->m_angle < -M_PI_2) &&
                     (secondConElm->m_angle >= 0) &&
                     (secondConElm->m_angle <= M_PI_2) &&
                     (toRoadClass == firstConElm->m_roadClass) &&
                     (toRoadClass == secondConElm->m_roadClass) &&
                     (allNameSame(toNode)) &&
                     (firstConElm->m_passNodeRestrictions == 
                        ItemTypes::noWay)) {
            // Probably found a road that is splited into two lanes
            firstCon->setTurnDirection(ItemTypes::UTURN);
            secondCon->setTurnDirection(ItemTypes::AHEAD);

         } else if ( (firstConElm->m_angle > -M_PI_4) &&
                     (firstConElm->m_angle < 0) &&
                     (secondConElm->m_angle > 0) &&
                     (secondConElm->m_angle < M_PI_4) &&
                     ( fabs(fabs(firstConElm->m_angle) - 
                            fabs(secondConElm->m_angle)) < M_PI/9) &&
                     (toRoadClass == firstConElm->m_roadClass) &&
                     (toRoadClass == secondConElm->m_roadClass) &&
                     (m_nbrRamps == 0) &&
                     (allNameSame(toNode)) ) {
            // Probably found a road that is splited into two lanes
            // and comes together in this point
            firstCon->setTurnDirection(ItemTypes::AHEAD);
            secondCon->setTurnDirection(ItemTypes::AHEAD);
         
         } else if ( (fabs(firstConElm->m_angle) < (M_PI/9)) && 
               (fabs(secondConElm->m_angle) < (M_PI/9)) ) {
            //Both connections are almost straight ahead
            //Remove this if possible, use other criteria
            if (firstConElm->m_ramp) {
               firstCon->setTurnDirection(ItemTypes::ON_RAMP);
            } else {
               firstCon->setTurnDirection(ItemTypes::AHEAD);
            }

            if (secondConElm->m_ramp) {
               secondCon->setTurnDirection(ItemTypes::ON_RAMP);
            } else {
               secondCon->setTurnDirection(ItemTypes::AHEAD);
            }
            DEBUG2(cerr << "Both connections AHEAD, " << toNode->getNodeID()
                        << ", angles=" << firstConElm->m_angle << ", " 
                        << secondConElm->m_angle << endl);
          
         } else if (secondConElm->m_angle < (-M_PI_2)) {
            //FIXME RIGHT1/RIGHT2
            firstCon->setTurnDirection(ItemTypes::RIGHT);
            secondCon->setTurnDirection(ItemTypes::RIGHT);
            
         } else if ( firstConElm->m_angle > M_PI_2 ) {
            // FIXME: LEFT1/LEFT2
            firstCon->setTurnDirection(ItemTypes::LEFT);
            secondCon->setTurnDirection(ItemTypes::LEFT);
            
         } else if (secondConElm->m_angle <= 0) {
            // both negative angle
            firstCon->setTurnDirection(ItemTypes::LEFT);
            secondCon->setTurnDirection(ItemTypes::AHEAD);
            
         } else if (firstConElm->m_angle >= 0) {
            //both positive angle
            firstCon->setTurnDirection(ItemTypes::AHEAD);
            secondCon->setTurnDirection(ItemTypes::RIGHT);
            
         } else if ( (secondConElm->m_angle >= 0) &&
                      (secondConElm->m_angle <= M_PI_6) &&
                      (firstConElm->m_angle <= 0) &&
                      (firstConElm->m_angle >= (-M_PI_6)) ) {
            // one ahead and the other left or right
            if ( fabs(secondConElm->m_angle) < fabs(firstConElm->m_angle) ) {
               firstCon->setTurnDirection(ItemTypes::LEFT);
               secondCon->setTurnDirection(ItemTypes::AHEAD);
            } else {
               firstCon->setTurnDirection(ItemTypes::AHEAD);
               secondCon->setTurnDirection(ItemTypes::RIGHT);
            }
            
         } else if ( (firstConElm->m_angle <= 0) &&
                      (firstConElm->m_angle >= (-M_PI_6)) &&
                      (secondConElm->m_angle > M_PI_6) ) {
            // the second and first are almost M_PI apart
            if ( ( (fabs(firstConElm->m_angle)*1.5) < 
                          (M_PI-fabs(secondConElm->m_angle))) ||
                        (secondConElm->m_ramp) )
               firstCon->setTurnDirection(ItemTypes::AHEAD);
            else
               firstCon->setTurnDirection(ItemTypes::LEFT);
            
            secondCon->setTurnDirection(ItemTypes::RIGHT);
            
         } else if ( (secondConElm->m_angle >= 0) &&
                      (secondConElm->m_angle <= M_PI_6) &&
                      (firstConElm->m_angle < (-M_PI_6)) ) {
            // the second and first are almost M_PI apart
            if ( ( (fabs(secondConElm->m_angle)*1.5) < 
                   (M_PI-fabs(firstConElm->m_angle))) ||
                 (firstConElm->m_ramp))
               secondCon->setTurnDirection(ItemTypes::AHEAD);
            else
               secondCon->setTurnDirection(ItemTypes::RIGHT);
         
            firstCon->setTurnDirection(ItemTypes::LEFT);
            
         } else if ( (firstConElm->m_angle <= (-M_PI_6)) && 
                      (secondConElm->m_angle >= M_PI_6) ) {
            // T-crossing
            firstCon->setTurnDirection(ItemTypes::LEFT);
            secondCon->setTurnDirection(ItemTypes::RIGHT);
            
         } else {
            DEBUG1( cerr << "::Error:: Undefined turn exist in map " 
                         << firstConElm->m_angle << " : " 
                         << secondConElm->m_angle <<endl); 
         }
      }         
      break;

      case 3: {
         // ============================================== Fourway crossing
      
         // Get the connection and the connection elements
         ConnectionElement* firstConElm = 
                  (ConnectionElement*) (m_connectionNotices[0]);
         OldConnection* firstCon = firstConElm->m_con;
         ConnectionElement* secondConElm = 
                  (ConnectionElement*) (m_connectionNotices[1]);
         OldConnection* secondCon = secondConElm->m_con;
         ConnectionElement* thirdConElm = 
                  (ConnectionElement*) (m_connectionNotices[2]);
         OldConnection* thirdCon = thirdConElm->m_con;

         if (! m_noRoundabout) {
            DEBUG8(cerr << "GMSMap Found roundabout-flag!" << endl);
            // At least one of the SSIs is a roundabout.

            if (toRoundabout) {
               // Driving into a roundabout segment
               for (uint32 i=0; i<3; i++) {
                  ConnectionElement* curConElm = 
                     (ConnectionElement*) (m_connectionNotices[i]);
               
                  // Into or stay inside the roundabout?
                  if (curConElm->m_roundabout)
                     curConElm->m_con->setTurnDirection(ItemTypes::AHEAD);
                  else if (curConElm->m_con
                              ->isVehicleAllowed(ItemTypes::passengerCar)) {
                     curConElm->m_con->setTurnDirection(
                              ItemTypes::ENTER_ROUNDABOUT);
                  } else {
                     curConElm->m_con->setTurnDirection(
                              ItemTypes::UNDEFINED);
                  }
               }

            } else {
               for (uint32 i=0; i<3; i++) {
                  ConnectionElement* curConElm = 
                     (ConnectionElement*) (m_connectionNotices[i]);

                  if (curConElm->m_roundabout &&
                      curConElm->m_con->isVehicleAllowed(
                                            ItemTypes::passengerCar)) {
                     curConElm->m_con->setTurnDirection(
                                    ItemTypes::EXIT_ROUNDABOUT);
                  } else if (!curConElm->m_roundabout) {
                     curConElm->m_con->setTurnDirection(
                                    ItemTypes::RIGHT_ROUNDABOUT);
                  } else {
                     curConElm->m_con->setTurnDirection(ItemTypes::UNDEFINED);
                  }
               }
            }
            
         } else if ((!toIsMulti) &&
                    firstConElm->m_ismulti &&
                    secondConElm->m_ismulti &&
                    (!thirdConElm->m_ismulti) &&
                    firstConElm->m_angle <= 0 &&
                    secondConElm->m_angle >= 0) {
            
            firstCon->setTurnDirection(ItemTypes::AHEAD);
            // not allowed but
            secondCon->setTurnDirection(ItemTypes::AHEAD);
            // not allowed but
            thirdCon->setTurnDirection(ItemTypes::RIGHT);

         } else if ((!toIsMulti) &&
                    firstConElm->m_ismulti &&
                    (!secondConElm->m_ismulti )&&
                    thirdConElm->m_ismulti) {
            
            firstCon->setTurnDirection(ItemTypes::LEFT);
            // not allowed but
            secondCon->setTurnDirection(ItemTypes::AHEAD);
            // not allowed but
            thirdCon->setTurnDirection(ItemTypes::RIGHT);

         } else if (toIsMulti &&
                    !firstConElm->m_ismulti &&
                    secondConElm->m_ismulti &&
                    !thirdConElm->m_ismulti ) {
            
            firstCon->setTurnDirection(ItemTypes::LEFT);
            // not allowed but
            secondCon->setTurnDirection(ItemTypes::AHEAD);
            // not allowed but
            thirdCon->setTurnDirection(ItemTypes::RIGHT);


         } else if ((!toIsMulti) &&
                    (!firstConElm->m_ismulti) &&
                    secondConElm->m_ismulti &&
                    thirdConElm->m_ismulti &&
                    secondConElm->m_angle <= 0 &&
                    thirdConElm->m_angle >= 0) {
            
            firstCon->setTurnDirection(ItemTypes::LEFT);
            // not allowed but
            secondCon->setTurnDirection(ItemTypes::AHEAD);
            // not allowed but
            thirdCon->setTurnDirection(ItemTypes::AHEAD);
            
         } else if ( (toRoadClass == firstConElm->m_roadClass) &&
                     (toRoadClass < secondConElm->m_roadClass) &&
                     (toRoadClass < thirdConElm->m_roadClass) &&
                     (fabs(firstConElm->m_angle) < 2.0*M_PI_6) &&
                     (m_nbrRamps == 0)) {
            // If driving via firstCon we are following the big road!
            firstCon->setTurnDirection(ItemTypes::AHEAD);
            secondCon->setTurnDirection(ItemTypes::RIGHT);
            thirdCon->setTurnDirection(ItemTypes::RIGHT);

         } else if ( (toRoadClass == secondConElm->m_roadClass) &&
                     (toRoadClass < firstConElm->m_roadClass) &&
                     (toRoadClass < thirdConElm->m_roadClass) &&
                     (m_nbrRamps == 0)) {
            // If driving via secondCon we are following the big road!
            firstCon->setTurnDirection(ItemTypes::LEFT);
            secondCon->setTurnDirection(ItemTypes::AHEAD);
            thirdCon->setTurnDirection(ItemTypes::RIGHT);

         } else if ( (toRoadClass == thirdConElm->m_roadClass) &&
                     (toRoadClass < secondConElm->m_roadClass) &&
                     (toRoadClass < firstConElm->m_roadClass) &&
                     (fabs(thirdConElm->m_angle) < 2.0*M_PI_6) &&
                     (m_nbrRamps == 0)) {
            // If driving via thirdCon we are following the big road!
            firstCon->setTurnDirection(ItemTypes::LEFT);
            secondCon->setTurnDirection(ItemTypes::LEFT);
            thirdCon->setTurnDirection(ItemTypes::AHEAD);

         } else if ( (!toRamp) && 
                     (firstConElm->m_ramp) &&
                     (thirdConElm->m_ramp)) {
            // Driving to a non-ramp segment in a crossing that have two
            // ramps
            if ( (firstConElm->m_angle <= 0) &&
                 (firstConElm->m_angle >= -M_PI_4) &&
                 (secondConElm->m_angle < 2.0*M_PI_6)) {
               firstCon->setTurnDirection(ItemTypes::ON_RAMP);
            } else {
               firstCon->setTurnDirection(ItemTypes::LEFT);
            }

            // Second always ahead!
            secondCon->setTurnDirection(ItemTypes::AHEAD);

            if ( (thirdConElm->m_angle >= 0) &&
                 (thirdConElm->m_angle <= M_PI_4) &&
                 (secondConElm->m_angle > -2.0*M_PI_6)) {
               thirdCon->setTurnDirection(ItemTypes::ON_RAMP);
            } else {
               thirdCon->setTurnDirection(ItemTypes::RIGHT);
            }

         } else if ( (fabs(firstConElm->m_angle) < (M_PI/8)) && 
                     (fabs(thirdConElm->m_angle) < (M_PI/8)) ) {
            //All connections are almost straight ahead
            //Remove this if possible, use other criteria
            firstCon->setTurnDirection(ItemTypes::AHEAD);
            secondCon->setTurnDirection(ItemTypes::AHEAD);
            thirdCon->setTurnDirection(ItemTypes::AHEAD);
          
         } else if (thirdConElm->m_angle < (-M_PI_2)) {
            //FIXME RIGHT1/RIGHT2/RIGHT3
            firstCon->setTurnDirection(ItemTypes::RIGHT);
            secondCon->setTurnDirection(ItemTypes::RIGHT);
            thirdCon->setTurnDirection(ItemTypes::RIGHT);
               
         } else if (firstConElm->m_angle > M_PI_2) {
            // FIXME: LEFT1/LEFT2/LEFT3
            firstCon->setTurnDirection(ItemTypes::LEFT);
            secondCon->setTurnDirection(ItemTypes::LEFT);
            thirdCon->setTurnDirection(ItemTypes::LEFT);
          
         } else if (thirdConElm->m_angle <= 0) {
            // both negative angle
            firstCon->setTurnDirection(ItemTypes::LEFT);
            secondCon->setTurnDirection(ItemTypes::LEFT);
            thirdCon->setTurnDirection(ItemTypes::AHEAD);
          
         } else if (firstConElm->m_angle >= 0) {
            //both positive angle
            firstCon->setTurnDirection(ItemTypes::AHEAD);
            secondCon->setTurnDirection(ItemTypes::RIGHT);
            thirdCon->setTurnDirection(ItemTypes::RIGHT);
          
         } else if ( (thirdConElm->m_angle <=  M_PI_2) &&
                     (firstConElm->m_angle >= -M_PI_2) ) {
          // All connections from almost straight ahead
            if ((fabs(firstConElm->m_angle) < M_PI_6) &&
                (fabs(secondConElm->m_angle) < M_PI_6) &&
                (fabs(thirdConElm->m_angle) > M_PI_6) ) {
               firstCon->setTurnDirection(ItemTypes::AHEAD);
               secondCon->setTurnDirection(ItemTypes::AHEAD);
               thirdCon->setTurnDirection(ItemTypes::RIGHT);
          
            } else if ( (fabs(firstConElm->m_angle) > M_PI_6) &&
                        (fabs(secondConElm->m_angle) < M_PI_6) &&
                        (fabs(thirdConElm->m_angle) < M_PI_6) ) {
               firstCon->setTurnDirection(ItemTypes::LEFT);
               secondCon->setTurnDirection(ItemTypes::AHEAD);
               thirdCon->setTurnDirection(ItemTypes::AHEAD);
          
            } else if ( (fabs(thirdConElm->m_angle) < 
                         fabs(firstConElm->m_angle)) &&
                 (fabs(thirdConElm->m_angle) < fabs(secondConElm->m_angle)) ) {
               thirdCon->setTurnDirection(ItemTypes::AHEAD);
               secondCon->setTurnDirection(ItemTypes::LEFT);
               firstCon->setTurnDirection(ItemTypes::LEFT);
            } else if ( (fabs(secondConElm->m_angle) < 
                         fabs(firstConElm->m_angle)) &&
                        (fabs(secondConElm->m_angle) < 
                         fabs(thirdConElm->m_angle)) ) {
               thirdCon->setTurnDirection(ItemTypes::RIGHT);
               secondCon->setTurnDirection(ItemTypes::AHEAD);
               firstCon->setTurnDirection(ItemTypes::LEFT);
            } else if ( (fabs(firstConElm->m_angle) < 
                         fabs(secondConElm->m_angle)) &&
                        (fabs(firstConElm->m_angle) < 
                         fabs(thirdConElm->m_angle)) ) {
               thirdCon->setTurnDirection(ItemTypes::RIGHT);
               secondCon->setTurnDirection(ItemTypes::RIGHT);
               firstCon->setTurnDirection(ItemTypes::AHEAD);
            }
          
         } else if ( (thirdConElm->m_angle <= M_PI_2) &&
                     (thirdConElm->m_angle >= 0) &&
                     (secondConElm->m_angle <= 0) &&
                     (secondConElm->m_angle >= -M_PI_2) &&
                     (firstConElm->m_angle <= -M_PI_2) ) {
            // maybe add some criterias here
            if ( (fabs(thirdConElm->m_angle) < fabs(secondConElm->m_angle)) &&
                  (fabs(thirdConElm->m_angle < M_PI_4))) {
               firstCon->setTurnDirection(ItemTypes::LEFT);             
               secondCon->setTurnDirection(ItemTypes::LEFT);
               thirdCon->setTurnDirection(ItemTypes::AHEAD);
            } else {
               firstCon->setTurnDirection(ItemTypes::LEFT);             
               secondCon->setTurnDirection(ItemTypes::AHEAD);
               thirdCon->setTurnDirection(ItemTypes::RIGHT);
            }
          
         } else if ( (thirdConElm->m_angle >= M_PI_2) &&
                     (secondConElm->m_angle >= 0) &&
                     (secondConElm->m_angle <= M_PI_2) &&
                     (firstConElm->m_angle <= 0) &&
                     (firstConElm->m_angle >= -M_PI_2)  ) {
            // maybe add some criterias here
            if ( (fabs(firstConElm->m_angle) < fabs(secondConElm->m_angle)) &&
                  (fabs(firstConElm->m_angle < M_PI_4))) {
               firstCon->setTurnDirection(ItemTypes::AHEAD);             
               secondCon->setTurnDirection(ItemTypes::RIGHT);
               thirdCon->setTurnDirection(ItemTypes::RIGHT);
            } else {
               firstCon->setTurnDirection(ItemTypes::LEFT);             
               secondCon->setTurnDirection(ItemTypes::AHEAD);             
               thirdCon->setTurnDirection(ItemTypes::RIGHT);
            }

         } else if ( (thirdConElm->m_angle >= M_PI_2) &&
                     (secondConElm->m_angle <= 0) &&
                     (firstConElm->m_angle >= -M_PI_2) ) {
            // 
            if ( (fabs(secondConElm->m_angle)*1.5) < 
                  (M_PI-fabs(thirdConElm->m_angle)))
               secondCon->setTurnDirection(ItemTypes::AHEAD);
            else
               secondCon->setTurnDirection(ItemTypes::LEFT);
            thirdCon->setTurnDirection(ItemTypes::RIGHT);
            firstCon->setTurnDirection(ItemTypes::LEFT);
          
         } else if ( (thirdConElm->m_angle <= M_PI_2) &&
                     (secondConElm->m_angle >= 0) &&
                     (firstConElm->m_angle <= -M_PI_2) ) {
            //
            if ( (fabs(secondConElm->m_angle)*1.5) < 
                 (M_PI-fabs(firstConElm->m_angle)))
               secondCon->setTurnDirection(ItemTypes::AHEAD);
            else
               secondCon->setTurnDirection(ItemTypes::RIGHT);
            thirdCon->setTurnDirection(ItemTypes::RIGHT);
            firstCon->setTurnDirection(ItemTypes::LEFT);
          
         } else if ( (thirdConElm->m_angle >= M_PI_2) &&
                     (secondConElm->m_angle >= 0) &&
                     (secondConElm->m_angle <= M_PI_2) &&
                     (firstConElm->m_angle <= -M_PI_2) ) {
            //
            if ( (fabs(secondConElm->m_angle)*1.5) < 
                  (M_PI-fabs(firstConElm->m_angle)))
               secondCon->setTurnDirection(ItemTypes::AHEAD);
            else
               secondCon->setTurnDirection(ItemTypes::RIGHT);
            thirdCon->setTurnDirection(ItemTypes::RIGHT);
            firstCon->setTurnDirection(ItemTypes::LEFT);
          
         } else if ( (thirdConElm->m_angle >= M_PI_2) &&
                     (secondConElm->m_angle <= 0) &&
                     (secondConElm->m_angle >= -M_PI_2) &&
                     (firstConElm->m_angle <= -M_PI_2) ) {
            //
            if ( (fabs(secondConElm->m_angle)*1.5) < 
                  (M_PI-fabs(thirdConElm->m_angle)))
               secondCon->setTurnDirection(ItemTypes::AHEAD);
            else
               secondCon->setTurnDirection(ItemTypes::LEFT);
            thirdCon->setTurnDirection(ItemTypes::RIGHT);
            firstCon->setTurnDirection(ItemTypes::LEFT);
          
         } else if ( (secondConElm->m_angle >= M_PI_2) &&                       
                     (firstConElm->m_angle >= -M_PI_2) &&
                     (firstConElm->m_angle <= 0) ) {
            //
            if ( (fabs(firstConElm->m_angle)*1.5) < 
                  (M_PI-fabs(thirdConElm->m_angle))) {
               firstCon->setTurnDirection(ItemTypes::AHEAD);
               secondCon->setTurnDirection(ItemTypes::RIGHT);
            } else {
               firstCon->setTurnDirection(ItemTypes::LEFT);
               secondCon->setTurnDirection(ItemTypes::AHEAD);
            }
            thirdCon->setTurnDirection(ItemTypes::RIGHT);
          
         } else if ( (secondConElm->m_angle <= -M_PI_2) &&                    
                    (thirdConElm->m_angle <= M_PI_2) &&
                    (thirdConElm->m_angle >= 0) ) {
          //
            if ( (fabs(thirdConElm->m_angle)*1.5) < 
                  (M_PI-fabs(firstConElm->m_angle))) {
               secondCon->setTurnDirection(ItemTypes::LEFT);
               thirdCon->setTurnDirection(ItemTypes::AHEAD);
            } else {
               secondCon->setTurnDirection(ItemTypes::AHEAD);
               thirdCon->setTurnDirection(ItemTypes::RIGHT);
            }
            firstCon->setTurnDirection(ItemTypes::LEFT);

         } else if ( (secondConElm->m_angle <= -M_PI_2) &&                    
                     (thirdConElm->m_angle >= M_PI_2) ) {
            // Strange crossing
            firstCon->setTurnDirection(ItemTypes::LEFT);
            secondCon->setTurnDirection(ItemTypes::LEFT);
            thirdCon->setTurnDirection(ItemTypes::RIGHT);

         } else if ( (firstConElm->m_angle <= -M_PI_2) &&                    
                     (secondConElm->m_angle >= M_PI_2) ) {
            // Strange crossing
            firstCon->setTurnDirection(ItemTypes::LEFT);
            secondCon->setTurnDirection(ItemTypes::RIGHT);
            thirdCon->setTurnDirection(ItemTypes::RIGHT);

         } else {
            DEBUG1(cerr << "Error::: 4-way crossing with no turnDirecion" 
                      << firstConElm->m_angle << " " 
                      << secondConElm->m_angle << " " 
                      << thirdConElm->m_angle << endl);
         }

      } // case 3
      break;
                 
      default : { 
         for (uint16 i=0; i<m_connectionNotices.getSize(); i++) {
            ConnectionElement* curConElm = 
                  (ConnectionElement*) (m_connectionNotices[i]);
            OldConnection* curCon = curConElm->m_con;
            
            if (curConElm->m_angle < -M_PI_4 ) {          // left
               curCon->setTurnDirection(ItemTypes::LEFT);
            } else if (curConElm->m_angle > M_PI_4) {       // right
               curCon->setTurnDirection(ItemTypes::RIGHT);
            } else {                                        // ahead
               curCon->setTurnDirection(ItemTypes::AHEAD);
            }
         }
      }
      break;
   }

   // Return the correct value
   // XXX

   return (true);
}

bool
SetTurnDescriptions::isSymmetricRoundabout(Vector* nodeLength, float64 maxError)
{
   // Check if it is a 4 exit roundabout. If not, return false
   if (nodeLength->getSize() != 4) {
      return (false);
   }
   DEBUG8(nodeLength->dump(true));
   
   // Create the error vector, which gives a measurement of how symmetric
   // the roundabout is. A perfect roundabout would have the following
   // errorVec: [0.25, 0.5, 0.75, 1.00]. 
   float64 errorVec[4]; 
   for (uint32 i = 0; i < 4; i++) {
      // Element i (for entry i in the roundabout) is equal to
      // (length of roundabout until this entry / total length of rb)
      // subtracted by the reference value 0.25*(i+1).
      errorVec[i] = (float64(nodeLength->getElementAt(i)) / 
                     float64(nodeLength->getElementAt(3)))
                    - 0.25 * (i + 1);
      DEBUG8(cout << errorVec[i] << endl);
   }

   DEBUG4(cout << "SymmetricDifference= " << errorVec[1] << ", "
               << (errorVec[2] - errorVec[0])
               << ", => "
               << (fabs(errorVec[1]) + fabs((errorVec[2] - errorVec[0])))
               << endl);
   // The symmetric difference is way of measuring how much the opposing
   // entries of the roundabout differs.
   if  ((fabs(errorVec[1]) + fabs(errorVec[2] - errorVec[0])) > maxError) {
      DEBUG4(cout << "Asymmetric roundabout found!" << endl);
      return (false);
   } else {
      return (true);
   }
}

bool
SetTurnDescriptions::setCrossingKind(OldNode* toNode, OldMap* theMap)
{
   // The crossingKind at this node
   ItemTypes::crossingkind_t setKind = ItemTypes::UNDEFINED_CROSSING;

   // Get some data for the toNode-segment 
   // (roundabout-flag and nbr coonnections)
   bool toSegmentRoundabout = ((OldStreetSegmentItem*) 
   theMap->itemLookup(toNode->getNodeID() & 0x7FFFFFFF))->isRoundabout();
   uint16 nbrCons = toNode->getNbrConnections();
   
   if (toSegmentRoundabout) {
      // The node is inside a roundabout. All OldConnections that leads to 
      // this node should have crossingKind CROSSING_?ROUNDABOUT

      // First find out the "?" above by following the connection
      // that leads to a segment where the roundabout flag is set,
      // until we end up at the starting node (one revolution).
      // Count the number of connections that have 
      // ENTER_ROUNDABOUT_TURN as turn descriptions; this is equal
      // to the "?" above if we are going through the roundabout
      // in positive (the allowed) direction. Otherwise, 0 exits
      // will be counted.
      
      OldNode* thisNode = toNode;
      uint32 nbrExits = 0;
      uint32 nextSSIID = 0;
      uint32 thisSSIID = thisNode->getNodeID() & 0x7fffffff;
      OldStreetSegmentItem* thisSSI = dynamic_cast<OldStreetSegmentItem*>
                                   (theMap->itemLookup(thisSSIID));
      
      // nodeLength, vector where element i contains the length between
      // node i and node 0, on the trajectory of the roundabout.
      Vector* nodeLength = new Vector(4, 1);
      uint32 lengthOfRoundabout = 0;
      uint32 lastLength = 0;
      bool exitLoop = false; 
      
      // Stop when we have gone 360 degrees in the roundabout.
      while ((nextSSIID != thisSSIID) && (!exitLoop)) {
         bool foundExit = false;
         OldNode* nextNode = NULL;
         // Find the next ssi that is a roundabout
         for (uint32 i = 0; i < thisNode->getNbrConnections(); i++) {
            OldStreetSegmentItem* nextSSI;
            // OldNode where this connection leads from. 
            OldNode* fromNode = theMap->nodeLookup(thisNode->getEntryConnection(i)
                                                      ->getFromNode());
               
            nextSSI = dynamic_cast<OldStreetSegmentItem*>
                      (theMap->itemLookup(fromNode->getNodeID() &
                                        0x7fffffff));
            if (nextSSI->isRoundabout()) {
               // Found a node with a roundabout! Store it in nextNode until
               // the for loop is done.
               nextNode = fromNode;
               nextSSIID = nextSSI->getID();
               // Increase the length of the roundabout
               lastLength = nextSSI->getLength();
               lengthOfRoundabout += lastLength;
            } 
            // Check if the connection is a roundabout exit (that is,
            // it is allowed to drive to this node).
            // In that case increase the number of exits in the roundabout
            // after leaving this for loop.
            else if (thisNode->getEntryConnection(i)
                             ->getTurnStringCode() ==
                             StringTable::ENTER_ROUNDABOUT_TURN) {
               foundExit = true;
            }
         }
         
         // If the nextNode == NULL, that means that the roundabout is
         // not completely on this map, ie. is crossing the boundary. 
         if (nextNode != NULL) {
               
            // If the first roundabout entrance hasn't been found,
            // "rotate" the roundabout until we start at a rb entrance.
            if ((! foundExit) && (nbrExits == 0)) {
               if (nextSSIID != (toNode->getNodeID() & 0x7fffffff)) {
                  // We haven't already done one revolution
                  thisSSIID = nextSSIID;
                  // Reset length variables
                  lastLength = 0;
                  lengthOfRoundabout = 0;
                  // Set to zero to avoid exiting the loop
                  nextSSIID = 0;
               } else {
                  // One revoulution without finding any rb entrances.
                  // Exit loop.
                  exitLoop = true;
               }
            }

            // Continue with the next node that was a roundabout.
            thisNode = nextNode;
            if (foundExit) {
               // Don't add the length of the first exit, since that
               // length will be zero anyway
               if (nbrExits > 0) {
                  // Add the length of the roundabout so far, but subtract the
                  // last length, since that belongs to the next ssi.
                  nodeLength->addLast(lengthOfRoundabout - lastLength); 
               }
               nbrExits++;
            }
         } else {
            // Signal that we want to exit this loop since the roundabout
            // is not completely inside this map.
            exitLoop = true;
         }
      }

      // Add the total length of the roundabout as the last element in
      // nodeLength vector.
      nodeLength->addLast(lengthOfRoundabout);
      
      // Set the roundabout type based on the number of exits
      switch (nbrExits) {
         case (0):
            // 0 exits in the roundabout.
            // This will be the case when trying to traverse the roundabout
            // in the "wrong" (against traffic) direction. Don't set any
            // crossing kind for this roundabout since it could overwrite
            // the true value. (Each roundabout will be traversed a each
            // time for each node in the roundabout. Not very nice :( )
            break;
         case (3):
            setKind = ItemTypes::CROSSING_3ROUNDABOUT;
            DEBUG4(cout << "Setting 3 exit roundabout" << endl);
            break;
         case (4):
            // Check if it is a normal 4 exit roundabout or not.
            DEBUG4(cout << "SSI id in roundabout to be checked:"
                        << (toNode->getNodeID() & 0x7fffffff)
                        << endl);
            // 0.25 comes from manual tuning...
            if (isSymmetricRoundabout(nodeLength, 0.25)) {
               setKind = ItemTypes::CROSSING_4ROUNDABOUT;
            } else {
               setKind = ItemTypes::CROSSING_4ROUNDABOUT_ASYMMETRIC;
            }
            DEBUG4(cout << "Setting 4 exit roundabout" << endl);
            break;
         case (5):
            setKind = ItemTypes::CROSSING_5ROUNDABOUT;
            DEBUG4(cout << "Setting 5 exit roundabout" << endl);
            break;
         case (6):
            setKind = ItemTypes::CROSSING_6ROUNDABOUT;
            DEBUG4(cout << "Setting 6 exit roundabout" << endl);
            break;
         case (7):
            setKind = ItemTypes::CROSSING_7ROUNDABOUT;
            DEBUG4(cout << "Setting 7 exit roundabout" << endl);
            break;
         default:
            // Strange number of exits in the roundabout
            setKind = ItemTypes::UNDEFINED_CROSSING;
            cerr << "GMSMap::setCrossingKind() ERROR, " << nbrExits
                 << " exits in the roundabout!. OldNodeid = "
                 << toNode->getNodeID() << endl;
            break;
      }
      
      // Set the same crossing kind for all connections that start from this
      // node, and end in a node outside the roundabout. In order to find
      // the connections that leads from this node, use the map's hashtable
      // to find items close to the opposing node of this node.
      OldMapHashTable* hashTable = theMap->getHashTable();
      // Check if the hashtable is present
      if (hashTable == NULL) {
         cerr << "SetTurnDescriptions::setCrossingKind() HashTable == NULL"
              << endl;
         return (false);
      }
      // Vector containing id:s of close items.
      Vector* closeItems;
      // Whether vector closeItems should be deleted or not
      bool shouldKill;
      
      int32 hpos;
      int32 vpos;
      // Give the coordinates of the OPPOSING node of toNode.
      if (toNode->isNode0()) {
         vpos = thisSSI->getGfxData()->getLastLat(0);
         hpos = thisSSI->getGfxData()->getLastLon(0);
      } else {
         vpos = thisSSI->getGfxData()->getLat(0,0);
         hpos = thisSSI->getGfxData()->getLon(0,0);
      }
      
      closeItems = hashTable->getAllWithinRadius_meter(  hpos,
                                                         vpos,
                                                         3, // 3 meter radius
                                                         shouldKill);
      // Go through all the close items.
      for (uint32 i = 0; i < closeItems->getSize(); i++) {
         OldStreetSegmentItem* closeSSI = dynamic_cast<OldStreetSegmentItem*>
                                       (theMap->itemLookup(closeItems
                                                        ->getElementAt(i)));
         if (closeSSI != NULL) {
            // Check both nodes
            for (uint32 j = 0; j < 2; j++) {
               // Go through all connections for the node in question and
               // check if it's from toNode, in that case set crossing kind.
               OldNode* closeNode = closeSSI->getNode(j);
               for (uint32 k = 0; k < closeNode->getNbrConnections(); k++) {
                  OldConnection* conn = closeNode->getEntryConnection(k);
                  if (conn->getConnectFromNode() == toNode->getNodeID()) {
                     // Set crossing kind
                     conn->setCrossingKind(setKind);
                  }
               }
            }
         }
      }
      
      // Free memory
      if (shouldKill) {
         delete closeItems;
      }
      delete nodeLength;
      
   } else {
      switch (nbrCons) {
         case (0) :
         case (1) :
            setKind = ItemTypes::NO_CROSSING;
            break;
         case (2) : {
            // Treeway crossing
            ItemTypes::turndirection_t firstTurnDesc = 
                  toNode->getEntryConnection(0)->getTurnDirection();
            ItemTypes::turndirection_t secondTurnDesc = 
                  toNode->getEntryConnection(1)->getTurnDirection();
            if ( (firstTurnDesc == ItemTypes::AHEAD) ||
                 (secondTurnDesc == ItemTypes::AHEAD)) {
               // T-crossing
               setKind = ItemTypes::CROSSING_3WAYS_T;
            } else {
               // Y-crossing
               // XXX: This is not always true. It might be a T-crossing
               //      in this case to. Must check the connections to the
               //      other two nodes as well.
               setKind = ItemTypes::CROSSING_3WAYS_Y;
            }
            } break;
         case (3) :
            // Fourway crossing
            setKind = ItemTypes::CROSSING_4WAYS;
            break;
         case (4) :
            // Fiveway crossing
            setKind = ItemTypes::CROSSING_5WAYS;
            break;
         case (5) :
            // Sixway crossing
            setKind = ItemTypes::CROSSING_6WAYS;
            break;
         case (6) :
            // Sevenway crossing
            setKind = ItemTypes::CROSSING_7WAYS;
            break;
         case (7) :
            // Eightway crossing
            setKind = ItemTypes::CROSSING_8WAYS;
            break;
         default :
            // More than nine road-crossing !?!?
            setKind = ItemTypes::UNDEFINED_CROSSING;
            DEBUG1(cerr << "GMSMap::setCrossingKind ERROR, more than" 
                        << "nineway-crossing -- " << nbrCons << endl);
      }
   }
   
   // Loop over all the connections and set the crossing kind
   for (uint32 i=0; i<nbrCons; i++) {
      // OldConnections leading from roundabouts to a ssi outside
      // the roundabout has already the correct crossingKind. For this
      // not to be overwritten, ignore all connections that has already
      // had their crossing kind set.
      if (toNode->getEntryConnection(i)->getCrossingKind() == 
          ItemTypes::UNDEFINED_CROSSING) {
         toNode->getEntryConnection(i)->setCrossingKind(setKind);
      }
   }
   
   //Everything OK
   return (true);
}

void 
SetTurnDescriptions::reset(OldMap* theMap)
{
   m_connectionNotices.reset();

   // Set the map-member
   m_map = theMap;

   // The flags with data about all the connections
   m_noRoundabout = true;
   m_noDivided = true;
   m_nbrRamps = 0;
   m_nbrMulti = 0;
   m_nbrNames = 0;

}

void
SetTurnDescriptions::getLatLon(GfxData* gfx, bool isNode0,
                  int32& lat1, int32& lon1, int32& lat2, int32& lon2)
{
   uint32 nbrPoints = gfx->getNbrCoordinates(0);
   if (nbrPoints < 2 ) {
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
SetTurnDescriptions::fillConnectionNotices(OldNode* toNode) 
{
   OldStreetSegmentItem* ssi = dynamic_cast<OldStreetSegmentItem*>
                              (m_map->itemLookup(toNode->getNodeID()));
   GfxData* gfx = ssi->getGfxData();
   uint16 nbrCon = toNode->getNbrConnections();

   // Maintain the memeberflags
   if (ssi->isRoundabout())
      m_noRoundabout = false;
   if (ssi->isRamp())
      m_nbrRamps++;
   if (ssi->isMultiDigitised())
       m_nbrMulti++;
   if (ssi->isDivided())
      m_noDivided = false;
   if (ssi->getNbrNames() > 0)
      m_nbrNames++;

   if (nbrCon > 0) {

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

      // Calculate the angle of all connections that lead to this node
      getLatLon(gfx, toNode->isNode0(), toLat1, toLon1, toLat2, toLon2);

      float64 toAngleFromNorth = GfxUtility::getAngleFromNorth( 
                                       toLat1, toLon1, toLat2, toLon2);

      OldNode* fromNode;
      for (uint16 i=0; i<nbrCon; i++) {
         fromID = toNode->getEntryConnection(i)->getConnectFromNode();

         DEBUG4(
            m_map->printItemNames(fromID);

            // "cout" because printItemNames prints to std.out...
            if (MapBits::isNode0(fromID))
               cout << "(0) --> ";
            else 
               cout << "(1) --> ";

            m_map->printItemNames(toNode->getNodeID());
            if (toNode->isNode0())
               cout << "(0):";
            else
               cout << "(1):";
         );

         OldStreetSegmentItem* fromSSI = dynamic_cast<OldStreetSegmentItem*>
                                       (m_map->itemLookup(fromID));
         fromNode = fromSSI->getNodeFromID(fromID);

         getLatLon(  fromSSI->getGfxData(), !(fromNode->isNode0()), 
                     fromLat1, fromLon1, fromLat2, fromLon2);

         float64 fromAngleFromNorth = GfxUtility::getAngleFromNorth(
                                     fromLat1, fromLon1, fromLat2, fromLon2);

         fromAngleFromNorth = fromAngleFromNorth - M_PI;
         float64 deltaAngle = toAngleFromNorth - fromAngleFromNorth;

         if (deltaAngle > M_PI)
            deltaAngle -= M_PI*2;
         if (deltaAngle < -M_PI)
            deltaAngle += M_PI*2;

         DEBUG8(cerr << " fromA=" << fromAngleFromNorth << ", toA=" 
                     << toAngleFromNorth << ", deltaAngle=" << deltaAngle 
                     << endl);

         // Maintain the memeberflags
         if (fromSSI->isRoundabout())
            m_noRoundabout = false;
         if (fromSSI->isRamp())
            m_nbrRamps++;
         if (ssi->isMultiDigitised())
             m_nbrMulti++;
         if (fromSSI->isDivided())
            m_noDivided = false;
         if (fromSSI->getNbrNames() > 0)
            m_nbrNames++;

         // Add a new ConnectionElement to the array
         OldNode* passNode = m_map->nodeLookup(fromID ^ 0x80000000);
         ConnectionElement* elm = new ConnectionElement(
                           deltaAngle, 
                           (OldConnection *) toNode->getEntryConnection(i),
                           fromSSI->getRoadClass(),
                           fromSSI->isRoundabout(),
                           fromSSI->isRamp(),
                           fromSSI->isDivided(),
                           fromSSI->isMultiDigitised(),
                           passNode->getEntryRestrictions());
         m_connectionNotices.addLast(elm);
      }

      // Sort the connections to make it easier to say "The second road to 
      // the left in the 5-way crossing" 
      m_connectionNotices.sort();
   }
}

bool
SetTurnDescriptions::allNameSame(OldNode* toNode)
{
   // Create and fill a Vector with all the stringindices in the toNode
   Vector toStringIndex(4);
   OldItem* toItem = m_map->itemLookup(toNode->getNodeID());
   for (uint32 i=0; i<toItem->getNbrNames(); i++) {
      toStringIndex.addLast(toItem->getStringIndex(i));
   }

   // Loop over all the connections and check if the names are equal
   for (uint32 i=0; i<toNode->getNbrConnections(); i++) {
      uint32 fromID = toNode->getEntryConnection(i)->getConnectFromNode();
      OldItem* fromItem = m_map->itemLookup(fromID);
      
      // Check all names in fromItem
      uint32 j = 0;
      while (j < toStringIndex.getSize()) {
         uint32 curToStringIndex = toStringIndex.getElementAt(j);
         bool found = false;
         for (uint32 u=0; u<fromItem->getNbrNames(); u++) {
            if (fromItem->getStringIndex(u) == curToStringIndex) {
               found = true;
            }
         }
         if (!found) {
            toStringIndex.removeElementAt(j);
            // Do not update the counter!
         } else {
            j++;
         }
      }
   }
   
   // Return true if any name is common, false otherwise
   return (toStringIndex.getSize() > 0);
}



