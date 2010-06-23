/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "LandmarkHead.h"
#include "ExpandRouteList.h"
#include "StringUtility.h"
#include "GfxData.h"
#include "Math.h"

ExpandRouteLink::ExpandRouteLink(uint32 nodeID,
                                 RouteableItem* item)
   : Link()
{
   m_nodeID     = nodeID;
   m_item       = item;
   m_lastItem   = item;
   m_lastNodeID = nodeID;

   m_streetName       = NULL;
   m_allStreetNames   = NULL;

   if ( item != NULL && item->getGfxData() != NULL ) {
      GfxData* gfx = item->getGfxData();
      if((nodeID & 0x80000000) != 0x80000000){
         // is node 0
         m_latitude  = gfx->getLat(0,0);
         m_longitude = gfx->getLon(0,0);
      } else {
         // is node 1
         uint32 nbrPoints = gfx->getNbrCoordinates(0);
         m_latitude  = gfx->getLat(0,nbrPoints-1);
         m_longitude = gfx->getLon(0,nbrPoints-1);
      }
   } else {
      // Item is NULL. Set lat/lon to invalid values.
      m_latitude = m_longitude = MAX_UINT32;
   }
   
   m_nbrItems  = 1;

   m_time = 0;
   m_standStillTime = 0;

   m_turnNumber       = 0;
   m_restrictedRbExit = 0;
   
   m_nbrPossTurns = 0;

   m_nameType = 0;

   m_dist = 0;
   m_time = 0;

   m_crossingKind    = ItemTypes::UNDEFINED_CROSSING;
   m_turnDescription = StringTable::UNDEFINED_TURN;
   
   m_landmarks = new LandmarkHead();
   m_exitCount  = 0;
   m_driveOnRightSide = true;

   m_nameIsExitNr = false;
   m_stringCode   = MAX_UINT32;
   m_allStringCodes.clear();
   m_passedStreets.clear();
}

void
ExpandRouteLink::addTimes(uint32 time, uint32 standStillTime)
{
   // Extra checks so that walking and riding a ferry won't calculate
   // the travel time as the time it takes to walk the distance.
   if ( m_transportation == ItemTypes::walk &&
        m_turnDescription != StringTable::CHANGE_FERRY_TURN &&
        m_turnDescription != StringTable::EXIT_FERRY_TURN ) {
      // Assume 5 km/h when walking
      m_time += uint32(float64(m_dist) / float64(Math::KMH_TO_MS * 5));
      m_standStillTime += 0; // Don't stand still while walking
   } else if (m_transportation == ItemTypes::bike) {
      m_time += uint32(float64(m_dist) / float64(Math::KMH_TO_MS * 15));
      m_standStillTime += standStillTime;
   } else {
      m_time += time;
      m_standStillTime += standStillTime;
   }
}
void
ExpandRouteLink::setTimes(uint32 time, uint32 standStillTime)
{
   m_time           = time;
   m_standStillTime = standStillTime;
}


void
ExpandRouteLink::dump()
{
   mc2dbg << "   name = " << m_streetName << " [" << m_stringCode << "]";
   if(m_allStreetNames != NULL){
      mc2dbg << "(" << m_allStreetNames << ")";
   }
   mc2dbg << ", nodeID = " << m_nodeID << endl;
   mc2dbg << "   turndescription = " 
        << StringTable::getString(m_turnDescription, StringTable::ENGLISH) 
        << endl;
   mc2dbg << "   dist = " << m_dist << ", time = " << m_time 
        << ", standStillTime = " << m_standStillTime << endl;
   mc2dbg << "   turnNumber = " << uint32(m_turnNumber)
          << "   restrEx = " << uint32(m_restrictedRbExit) << endl;
   mc2dbg << "   number items = " << m_nbrItems << endl;
   mc2dbg << "   drive on right side = " << (int)m_driveOnRightSide 
          << endl;
   mc2dbg << "   crossingK = " << (int)m_crossingKind << endl;
   mc2dbg << " lat, lon (" << m_latitude << ", " << m_longitude << ")"
          << endl;
   list<char*>::iterator i;
   for(i = m_passedStreets.begin(); i != m_passedStreets.end() ;i++){
      if(i != m_passedStreets.begin())
         mc2dbg << ", ";
      if((*i) != NULL)
         mc2dbg << (*i);
      else
         mc2dbg << " unnamed street";
   }
   mc2dbg << "." << endl;

   // Transportation, 
   mc2dbg << "   transportation = ";
   switch (m_transportation) {
      case (ItemTypes::undefined) : mc2dbg << "undefined"; break;
      case (ItemTypes::drive) : mc2dbg << "drive"; break;
      case (ItemTypes::walk) : mc2dbg << "walk"; break;
      case (ItemTypes::bike) : mc2dbg << "bike"; break;
      case (ItemTypes::bus) : mc2dbg << "bus"; break;
   }
   mc2dbg << endl;
   m_landmarks->dump();
}


ExpandRouteLink::ExpandRouteLink(
                      const char* str, 
                      uint32 stringCode, 
                      uint32 dist, 
                      uint32 time,
                      ItemTypes::transportation_t transport, 
                      int32 lat, 
                      int32 lon,
                      uint32 nbrItems,
                      uint8 nameType, 
                      byte turnNumber,
                      byte restrictedRbExit,
                      const ExpandStringLanesCont& lanes,
                      const ExpandStringSignPosts& signPosts,
                      bool driveOnRightSide )
   :  Link(), 
      m_dist( dist ),
      m_time( time ),
      m_transportation( transport ),
      m_latitude( lat ),
      m_longitude( lon ),
      m_nbrItems( nbrItems ),
      m_nameType( nameType ),
      m_turnNumber( turnNumber ),
      m_restrictedRbExit( restrictedRbExit) ,
      m_lanes( lanes ), m_signPosts( signPosts ),
      m_driveOnRightSide( driveOnRightSide )
{
   m_turnDescription = StringTable::stringCode(stringCode);
   if (str != NULL) {
      m_streetName = StringUtility::newStrDup(str);
   } else {
      mc2dbg4 << "str == NULL" << endl;
      m_streetName = StringUtility::newStrDup("");
   }
   mc2dbg8 << "ExpandRouteLink:: m_streetName = " << m_streetName 
               << endl;

   // Check if this is a "E-road" and set the member according to that
   if ( ( strlen(m_streetName) > 2) &&
        (m_streetName[0] == 'E') &&
        ( (StringUtility::onlyDigitsInString((char*)&(m_streetName[1]))))
      ) {
      m_eroad = true;
   } else {
      m_eroad = false;
   }

   // landmarks
   m_landmarks = new LandmarkHead();

   mc2dbg4 << "Nbr items=" << m_nbrItems << endl;
   m_nbrPossTurns = 0;
   m_passedStreets.clear();

}

ExpandRouteLink::~ExpandRouteLink()
{
   delete [] m_streetName;
   delete [] m_allStreetNames;
   m_passedStreets.clear();

   delete m_landmarks;
}

bool 
ExpandRouteLink::addToPacket(ExpandRouteReplyPacket* p,
                             bool triss)
{
   mc2dbg2 << "ExpandRouteLink::addToPacket m_streetName = " 
               << m_streetName << endl;
   uint32 time = m_time;
   if (triss || true) { // Is this ever wrong ???? 
      time += m_standStillTime;
   }
   
   p->setStringData( m_streetName,
                     m_turnDescription,
                     m_dist,
                     time,
                     m_transportation,
                     m_latitude,
                     m_longitude,
                     m_nameType,
                     m_turnNumber,
                     m_crossingKind,
                     m_nbrPossTurns,
                     (uint32*)m_possTurns,
                     m_lanes,
                     m_signPosts,
                     m_landmarks );
   // The setStringData should return bool, and that value should be
   // returned by this method.
   return (true);
}


void
ExpandRouteLink::setNewName(const char* newName) 
{
   delete [] m_streetName;
   if ( newName != NULL ) {
      m_streetName =
         StringUtility::newStrDup(newName);
   } else {
      m_streetName = NULL;
   }
}
void
ExpandRouteLink::setNewAllName(const char* newName) 
{
   delete [] m_allStreetNames;
   if ( newName != NULL ) {
      m_allStreetNames =
         StringUtility::newStrDup(newName);
   } else {
      m_allStreetNames = NULL;
   }
}

void
ExpandRouteLink::addLane( const ExpandStringLanes& lanes ) {
   m_lanes.addLaneGroup( lanes );
}

void
ExpandRouteLink::addLaneFirst( const ExpandStringLanes& lanes ) {
   m_lanes.addLaneGroupFirst( lanes );
}

bool
ExpandRouteLink::hasNonStopOfLanesLane() const {
   bool res = false;

   for ( ExpandStringLanesCont::const_iterator it = m_lanes.begin() ;
         it != m_lanes.end() ; ++it ) {
      if ( !(*it).getStopOfLanes() ) {
         res = true; // Last is lanes
      } else {
         res = false; // Last is stop
      }
   }

   return res;
}

void
ExpandRouteLink::addSignPost( const ExpandStringSignPost& signPost ) {
   m_signPosts.addSignPost( signPost );
}

bool
ExpandRouteLink::noName()
{
   if((m_stringCode == MAX_UINT32) || (m_stringCode == 0))
      return true;
   return false;
}



bool
ExpandRouteLink::addPossibleTurn(StringTable::stringCode possTurn)
{
   // Check if possible to add (Multidig streets can allow a U-turn )
   MC2_ASSERT(m_nbrPossTurns < 7);
   uint32 nbrPossExits;
   if(m_crossingKind <= ItemTypes::NO_CROSSING)
      nbrPossExits = 1;
   else if(m_crossingKind == ItemTypes::CROSSING_3WAYS_T)
      nbrPossExits = 2;
   else if(m_crossingKind < ItemTypes::CROSSING_2ROUNDABOUT)
      nbrPossExits = int(m_crossingKind)-1;
   else  // Roundabout xrossing difficult to know the number of exits.
      nbrPossExits = 2;
   
   
   if(m_nbrPossTurns < nbrPossExits){
      m_possTurns[m_nbrPossTurns++] = possTurn;
      mc2dbg4 << "  POSSIBLE TURN ADDED[" << m_nbrPossTurns << "]: "
              << StringTable::getString(possTurn, StringTable::ENGLISH)
              << endl;
      return true;
   }
   return false;
}
 

uint32
ExpandRouteLink::getNbrPossibleTurns()
{
   return m_nbrPossTurns;
}


StringTable::stringCode
ExpandRouteLink::getPossibleTurn(uint32 index)
{
   uint32 nbrPossExits;
   MC2_ASSERT(index < 7);
   if(m_crossingKind <= ItemTypes::NO_CROSSING)
      nbrPossExits = 0;
   else if(m_crossingKind == ItemTypes::CROSSING_3WAYS_T)
      nbrPossExits = 1;
   else if(m_crossingKind < ItemTypes::CROSSING_2ROUNDABOUT)
      nbrPossExits = int(m_crossingKind)-2;
   else  // Roundabout xrossing difficult to know the number of exits.
      nbrPossExits = 2;
   
   // Check if index ok
   if(index >= nbrPossExits){
      return StringTable::NOTFOUND;
   }

   // Check if poss turm exist
   if(index >= m_nbrPossTurns){
      return StringTable::UNDEFINED_TURN;
   }
   
   // Return pos turn at index
   return m_possTurns[index];
}
   
void
ExpandRouteLink::addPassedStreet(const char* streetName)
{
   if(streetName != NULL){
      m_passedStreets.push_front( StringUtility::newStrDup(streetName) );
   } else {
      char* name = NULL;
      m_passedStreets.push_front(name);
   }
   
   
}

/// Checks if same lanes in a and b
bool sameLanes( const ExpandStringLanes& a, const ExpandStringLanes& b ) {
   bool same = a.size() == b.size() && 
      a.getStopOfLanes() == b.getStopOfLanes();

   if ( same ) {
      for ( uint32 i = 0 ; i < a.size() && same ; ++i ) {
         same = a[ i ].getDirections() == b[ i ].getDirections();
      }
   }

   return same;
}

bool
ExpandRouteLink::concatinateWith(ExpandRouteLink* otherLink, bool after) 
{
   if (after) {
      ExpandRouteLink* nextLink = 
         static_cast<ExpandRouteLink*> (otherLink->suc());
      if (nextLink != NULL) {
         // Move this to after passed streets are updated. 
         nextLink->m_time += otherLink->m_time;
         nextLink->m_nbrItems += otherLink->m_nbrItems;
         nextLink->m_dist += otherLink->m_dist;
         nextLink->m_turnNumber += otherLink->m_turnNumber;

         // Its otherLink thats lead to the next link so we have to
         // remember its connectOutNode. (otherLink will be discarded)
         m_lastItem   = otherLink->m_lastItem;
         m_lastNodeID = otherLink->m_lastNodeID;

         // The landmarks:
         // First update the dist of the lm's in nextLink
         LandmarkLink* nextLM = 
            static_cast<LandmarkLink*>(nextLink->m_landmarks->first());
         while (nextLM != NULL) {
            // if dist already was updated only add the dist of the otherLink
            if (nextLM->m_lmDistUpdated) {
               nextLM->m_lmDist += otherLink->m_dist;
            // if lmDist is negative it is a "pass"-bua
            } else if (nextLM->m_lmDist < 0) {
               nextLM->m_lmDist += nextLink->m_dist;
            } else {
               nextLM->m_lmDist = nextLink->m_dist;
            }
            nextLM->m_lmDistUpdated = true;
            nextLM = static_cast<LandmarkLink*>(nextLM->suc());
         }
         // Then add lm's from otherLink. Add with intoAsFirst, 
         // start with last(), to maintain the correct order
         LandmarkLink* otherLM = 
            static_cast<LandmarkLink*>(otherLink->m_landmarks->last());
         while (otherLM != NULL) {
            // Into m_landmarks of nextLink, 
            // update the landmark dist if not updated before
            uint32 dist;
            if (otherLM->m_lmDistUpdated) {
               dist = otherLM->m_lmDist;
            } else {
               dist = otherLink->m_dist;
            }
            LandmarkLink* newLM = new LandmarkLink(
                  otherLM->m_lmdesc, dist, true, otherLM->getLMName());
            if(otherLM->isTraffic()){
               newLM->setIsTraffic();
               if(otherLM->isDetour())
                  newLM->setDetour();
               newLM->setIsStart(otherLM->isStart());
               newLM->setIsStop(otherLM->isStop());
               newLM->setStreetName(otherLM->getStreetName());
            }
            newLM->intoAsFirst(nextLink->m_landmarks);
            otherLM = static_cast<LandmarkLink*>(otherLM->pred());
         }

         // The Lanes
         // Add otherLink's dist to nextLink's m_lanes
         for ( ExpandStringLanesCont::iterator it = 
                  nextLink->m_lanes.begin() ; it != nextLink->m_lanes.end() ;
               ++it ) {
            mc2dbg4 << "concatinateWith after dist " << it->getDist()
                   << " + " << otherLink->m_dist << " " << *it << endl;
            it->setDist( it->getDist() + otherLink->m_dist );
         }
         // Copy otherLink's m_lanes to nextLink
         for ( uint32 i = otherLink->m_lanes.size() ; i > 0  ; --i ) {
            mc2dbg4 << "concatinateWith other's after dist " 
                   << otherLink->m_lanes[ i - 1 ].getDist() << " " 
                   << otherLink->m_lanes[ i - 1 ] << endl;
            nextLink->addLaneFirst( otherLink->m_lanes[ i - 1 ] );
         }
         // Remove now duplicates in the combined link
         // ei. two ahead lanes in both links is now after each other 
         // with only different dist.
         for ( ExpandStringLanesCont::iterator it = 
                  nextLink->m_lanes.begin() ; it != nextLink->m_lanes.end() ;
               ++it ) {
            ExpandStringLanesCont::iterator nextIt = it;
            ++nextIt;
            if ( nextIt != nextLink->m_lanes.end() ) {
               if ( sameLanes( *it, *nextIt ) ) {
                  // Remove duplicate
                  ExpandStringLanesCont::iterator elIt = 
                     nextLink->m_lanes.erase( nextIt );
                  it = --(elIt);
               }
            }
         }

         // The SignPosts (use one set but which?)
         // Using nextLink's SignPosts, ie. leave it as is.
      }
   } else {
      m_dist += otherLink->m_dist;
      m_time += otherLink->m_time;
      m_nbrItems += otherLink->m_nbrItems;
      m_turnNumber += otherLink->m_turnNumber;

      // Its this link thats lead to the next link so we don't need the
      // connectOutNode of otherLink.
 
      // The landmarks:
      // First update the dist of the lm's in this link
      LandmarkLink* curLM = static_cast<LandmarkLink*>(m_landmarks->first());
      while (curLM != NULL) {
         if (curLM->m_lmDistUpdated) {
            //then just add distance from the otherLink
            curLM->m_lmDist += otherLink->m_dist;
         } else {
            curLM->m_lmDist = m_dist;
            curLM->m_lmDistUpdated = true;
         }
         curLM = static_cast<LandmarkLink*>(curLM->suc());
      }
      // Then add lm's from otherLink. Add with intoAsFirst, 
      // start with last(), to maintain the correct order
      LandmarkLink* otherLM = 
         static_cast<LandmarkLink*>(otherLink->m_landmarks->last());
      while (otherLM != NULL) {
         // Into m_landmarks of this Link, 
         // update the landmark dist if not updated before
         uint32 dist;
         if (otherLM->m_lmDistUpdated) {
            dist = otherLM->m_lmDist;
         } else {
            dist = otherLink->m_dist;
         }
         LandmarkLink* newLM = new LandmarkLink(
               otherLM->m_lmdesc, dist, true, otherLM->getLMName());
         if(otherLM->isTraffic()){
            newLM->setIsTraffic();
            if(otherLM->isDetour())
               newLM->setDetour();
            newLM->setIsStart(otherLM->isStart());
            newLM->setIsStop(otherLM->isStop());

            newLM->setStreetName(otherLM->getStreetName());
         }
         
         newLM->intoAsFirst(m_landmarks);
         otherLM = static_cast<LandmarkLink*>(otherLM->pred());
      }

      // The Lanes
      // Add otherLink's dist to m_lanes
      for ( ExpandStringLanesCont::iterator it = 
               m_lanes.begin() ; it != m_lanes.end() ;
            ++it ) {
         mc2dbg4 << "concatinateWith before dist " << it->getDist()
                << " + " << otherLink->m_dist << " " << *it << endl;
         it->setDist( it->getDist() + otherLink->m_dist );
      }
      // Copy otherLink's m_lanes to m_lanes
      for ( uint32 i = otherLink->m_lanes.size() ; i > 0  ; --i ) {
         mc2dbg4 << "concatinateWith other's before dist " 
                << otherLink->m_lanes[ i - 1 ].getDist() << " " 
                << otherLink->m_lanes[ i - 1 ] << endl;
         addLaneFirst( otherLink->m_lanes[ i - 1 ] );
      }
      // Remove now duplicates in the combined link
      // ei. two ahead lanes in both links is now after each other 
      // with only different dist.
      for ( ExpandStringLanesCont::iterator it = 
               m_lanes.begin() ; it != m_lanes.end() ;
            ++it ) {
         ExpandStringLanesCont::iterator nextIt = it;
         ++nextIt;
         if ( nextIt != m_lanes.end() ) {
            if ( sameLanes( *it, *nextIt ) ) {
               // Remove duplicate
               ExpandStringLanesCont::iterator elIt = 
                  m_lanes.erase( nextIt );
               it = --(elIt);
            }
         }
      }
      
      // The SignPosts (use one set but which?)
      // Using otherLink's SignPosts.
      m_signPosts = otherLink->m_signPosts;
   }
   
   return (true);
}

void
ExpandRouteLink::replaceName(uint32 newStringCode)
{
   m_stringCode = newStringCode;
   if(!m_allStringCodes.empty()){
      // Is the new name present among the extra names?
      list<uint32>::iterator result = find(m_allStringCodes.begin(),
                                           m_allStringCodes.end(),
                                           newStringCode);
      if(result != m_allStringCodes.end()){
         // Ok remove it from the list.
         m_allStringCodes.erase(result);
      }
   }
}


// =======================================================================
//                                                       ExpandRouteHead =

ExpandRouteHead::ExpandRouteHead()
   : Head()
{
   m_lastSegmentLeftStreetCount  = 0;
   m_lastSegmentRightStreetCount = 0;
}
   
ExpandRouteHead::~ExpandRouteHead()
{
   // Nothing to do...
}

int
ExpandRouteHead::dump()
{
   int nbrItems = 0;
   ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>(first());
   while (curLink != NULL) {
      mc2dbg << "=============================================" << endl;
      curLink->dump();

      nbrItems++;
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   }
    mc2dbg << "=============================================" << endl;
   return (nbrItems);
}

int
ExpandRouteHead::removeStreetsWithoutName(uint32 maxDist)
{
   int nbrRemovedStreets = 0;
   if (cardinal() > 2) {
      ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>(first());
      ExpandRouteLink* nextLink = 
         static_cast<ExpandRouteLink*>(curLink->suc());
      while (nextLink != NULL) {
         // Check the names
         if ( (curLink->m_streetName != NULL) &&
              (StringUtility::strcasecmp(curLink->m_streetName, "missing") 
                  == 0) &&
              (nextLink->m_dist < maxDist) &&
              (nextLink->m_turnDescription == StringTable::AHEAD_TURN)) {
            // Set the name of curLink to the one that follows
            curLink->setNewName(nextLink->m_streetName);

            // Remove the link that is after nextLink...
            if (nextLink->suc() != NULL) {
               concatinateLinks(static_cast<ExpandRouteLink*>(nextLink->suc()), 
                                false);
               nextLink = static_cast<ExpandRouteLink*>(curLink->suc());
            } else {
               mc2dbg4 << "nextLink->suc() == NULL" << endl;
               nextLink = NULL;
            }
         } else {
            // Update the link pointers
            curLink = nextLink;
            nextLink = static_cast<ExpandRouteLink*>(curLink->suc());
         }
      }
   }

   // Return the number of streets removed
   mc2dbg4 << "Removed " << nbrRemovedStreets 
               << " streets without name" << endl;
   return (nbrRemovedStreets);
}
/*
int
ExpandRouteHead::removeAheadTurns() 
{ 
    int nbrRemovedStreets = 0;
   if (cardinal() > 2) {
      ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>(first());
      // Loop over all the links in the list
      while (curLink != NULL) {
         // Take special care to the E-roads
         if ( (curLink->m_turnDescription == StringTable::AHEAD_TURN) &&
              (!curLink->m_eroad) ) {
            mc2dbg4 << "To remove \"ahead street\"" 
                        << curLink->m_streetName << ", turnNumber="
                        << (uint32) curLink->m_turnNumber << endl;

            // Update curLink to be able to remove it
            curLink = static_cast<ExpandRouteLink*>(curLink->suc());
            if (curLink != NULL) {
               // Remove the link before curLink (previous curLink)
               concatinateLinks(curLink, false);
               nbrRemovedStreets++;
               switch (curLink ->m_turnDescription) {
                  case (StringTable::LEFT_TURN) :
                     curLink->m_turnNumber = curLink->m_leftTurnNumber;
                     break;
                  case (StringTable::RIGHT_TURN) :
                     curLink->m_turnNumber = curLink->m_rightTurnNumber;
                     break;
                  default :
                     ; // Nothing to do.
               }
            }
         } else {
            // Update the current link
            curLink = static_cast<ExpandRouteLink*>(curLink->suc());
         }
      }
   }

   // Return the number of streets removed
   mc2dbg4 << "Removed " << nbrRemovedStreets 
               << " streets without name" << endl;
   return (nbrRemovedStreets); 
   return 0; 
}*/



bool 
ExpandRouteHead::saveStringDataIntoPacket(ExpandRouteReplyPacket* reply,
                                          bool triss) 
{
   bool retValue = true;
   ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>(first());
   while ((curLink != NULL) && (retValue)) {
      retValue = curLink->addToPacket(reply, triss);
      if (retValue) {
         curLink = static_cast<ExpandRouteLink*>(curLink->suc());
      }
   }
   
   mc2dbg4 << "Last: left " << (int)m_lastSegmentLeftStreetCount
        << ", right " << (int)m_lastSegmentRightStreetCount  << endl;
   reply->setLastSegmentsLeftStreetCount(m_lastSegmentLeftStreetCount);
   reply->setLastSegmentsRightStreetCount(m_lastSegmentRightStreetCount);

   return (retValue);
}


bool
ExpandRouteHead::saveItemsPerStringIntoPacket(
                                 ExpandRouteReplyPacket* reply)
{
   ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>(first());
   while (curLink != NULL) {
      if (curLink->m_nbrItems > 0) {
         reply->addItemsPerString(curLink->m_nbrItems);
      }
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   }
   
   return (true);

}

void 
ExpandRouteHead::concatinateLinks(ExpandRouteLink* master, bool after)
{
   if (master != NULL) {
      // Find the slave
      ExpandRouteLink* slave;
      if (after) {
         slave = static_cast<ExpandRouteLink*>(master->suc());
      } else {
         slave = static_cast<ExpandRouteLink*>(master->pred());
      }

      // Concatinate and delete
      if (slave != NULL) {
         master->concatinateWith(slave, after);
         slave->out();
         delete slave;
      } else {
         mc2dbg4 << "ExpandRouteHead:: slave == NULL" << endl;
      }
   }
}

