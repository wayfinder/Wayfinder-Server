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

#include "PacketContainer.h"
#include "RouteList.h"
#include "RouteElement.h"
#include "ExpandRoutePacket.h"
#include "ExpandItemID.h"
#include "ExpandStringItem.h"
#include "ExpandedRouteLandmarkItem.h"
#include "Name.h"
#include "NameCollection.h"
#include "LandmarkHead.h"
#include "LandmarkLink.h"
#include "STLUtility.h"


/**
 *   Creates a new RouteList.
 *   @param routeReq The request that was sent from the client.
 *   @param pc       The packetContainer containing the answer.
 */
RouteList::RouteList(RouteReq* routeReq,
                     PacketContainer* pc,
                     RouteRequest* rr )
{
   m_routeReq = routeReq;
   m_pc = pc;
   m_truncated = false;
   m_truncatedDist = 0;
   m_truncatedWPTNbr = 0;
   m_rr = rr;
   m_valid = fillList();
}

/**
 *
 *
 */
RouteList::~RouteList()
{
   // Delete everything in the list.
   STLUtility::deleteValues( m_routeVector );
}

/**
 *   Take the data and fill the list.
 */
bool
RouteList::fillList() {
   // Get the packet from the pc
   if ( m_pc == NULL )
      return false;
   Packet* p = m_pc->getPacket();
   // Make expandroutepacket of the Packet.
   ExpandRouteReplyPacket* erp =
      dynamic_cast<ExpandRouteReplyPacket*>(p);

   // save the total dist of the route
   m_totalDist = erp->getTotalDist();

   // Total time might be good for ETA
   m_totalStandStillTime = erp->getStandStillTime();
   m_totalTime = erp->getTotalTime() - m_totalStandStillTime;
   
   if ( erp == NULL /*|| erp->getStatus() != StringTable::OK */) {
      mc2dbg1 << "erp->getStatus() != StringTable::OK.. returning" << endl;
      return false;
   }
   // Get the StringItems.
   int numStringData = erp->getNumStringData();
   ExpandStringItem** items = erp->getStringDataItem();

   // Make a stringtable, make the RouteElements.
   for(int i=0; i < numStringData; ++i) {
      // Get the index and add the string if it's not already there.
      int index = 0;

      if ( items[i]->hasName() ) {
         index = m_stringTable.addString(items[i]->getText());
      } else {
         index = m_stringTable.addString("");
      }
      RouteElement* element = new RouteElement();
      element->setText(index);
      m_routeVector.push_back(element);
   }

   // Now we have a vector with RouteElements that only has names.
   // Let's add coordinates and stuff.
   ExpandItemID* gfxItems = erp->getItemID();
   Vector& groups     = gfxItems->getGroupID();
   IntVector& lats    = gfxItems->getLat();
   IntVector& lons    = gfxItems->getLon();
   Vector& offset     = gfxItems->getCoordinateOffset();
   Vector& speeds     = gfxItems->getSpeedLimit();
   Vector& attributes = gfxItems->getAttributes();
   
   // Loop again and add the coordinates
   int32 groupSize = groups.getSize();

   //mc2dbg1 << "SPEEDS" << endl;
   //speeds.dump();
   
   uint32 lastGroup = MAX_UINT32;
   for(int32 i=0; i < groupSize; ++i ) {
//      mc2dbg1 << "groupID[" << i << "] = " << groups[i] << endl;
//      mc2dbg1 << "speed[" << i << "] = " << speeds[i] << endl;
      // Add to the next group - not the current one. Start at doesn't
      // have a start coordinate.
      uint32 groupIdx = groups[i] + 1;      
      if ( groupIdx < m_routeVector.size() ) {
         RouteElement* el = m_routeVector[groupIdx];
         uint32 startIndex = offset[i];
         uint32 endIndex = gfxItems->nextCoordStart(i);         
         for (uint32 j = startIndex; j < endIndex; ++j ) {            
            el->addCoord(lats[j], lons[j]);
//            mc2dbg1 << "lat[" << j << "] = " << lats[j] << endl;
//            mc2dbg1 << "lon[" << j << "] = " << lons[j] << endl;
            //cout << "Adding speed: " <<  speeds[i] << endl;
            el->addSpeed(speeds[i]);
            el->addAttribute(attributes[i]);
         }

         
         //mc2dbg1 << " Adding to element " << groupIdx << endl;
         // If this is a new group we add the coordinate last in
         // the previous element too.
         if ( lastGroup != groupIdx && ( (groupIdx - 1) >= 0 )) {
            RouteElement* el = m_routeVector[groupIdx-1];
            el->addCoord(lats[startIndex], lons[startIndex]);
            //mc2dbg1 << "Adding last coordinate to " << groupIdx -1 << endl;
            lastGroup = groupIdx;
         }         
      } else {
         MC2ERROR("groupID larger than vectorsize");
      }
   }

   // Add the last coordinate to the last RouteElement
   if (numStringData != 0) {
      //mc2dbg1 << "Adding last last coordinate to " << (numStringData -1) << endl;
      RouteElement* el = m_routeVector[numStringData - 1];
      el->addCoord( erp->getLastItemLat(), erp->getLastItemLon() );
   }
   // Loop through the StringItems and add the turncodes and distances
   uint32 totalTime = 0;
   for(int i=0; i < numStringData; ++i ) {
      RouteElement* el = m_routeVector[i];
      el->setDist(items[i]->getDist());
      el->setTurnCode(items[i]->getStringCode());
      el->setExitCount(items[i]->getExitCount());
//      mc2dbg1 << "Crossingkind: " << (int)items[i]->getCrossingKind() << endl;
//      mc2dbg1 << "Crossingkind: " <<
//         StringTable::getString(ItemTypes::getCrossingKindSC(items[i]->getCrossingKind()), StringTable::ENGLISH) << endl;
      el->convertAndSetCrossingKind(items[i]->getCrossingKind());
      el->setTime( items[i]->getTime() );
      totalTime += items[i]->getTime();
      el->setTotalTimeSoFar( totalTime );

      // Calculate dist
      int32 dist = items[ i ]->getDist();
      int j = i;
      while ( j + 1 < numStringData && items[ j ]->getStringCode() ==
              StringTable::LMLOCATION_ROADNAMECHANGE )
      {
         dist += items[ j + 1 ]->getDist();
         j++;
      }
 
      // Add landmarks
      if ( items[ i ]->hasLandmarks() ) {
         LandmarkLink* land = static_cast<LandmarkLink*>( 
            items[ i ]->getLandmarks()->first() );
         while ( land != NULL ) {
            NameCollection* name = new NameCollection();
            name->addName( new Name( land->getLMName(), 
                                     LangTypes::english ) );
           el->addLandmark( new ExpandedRouteLandmarkItem(
                                land->m_lmdesc.type,
                                land->m_lmdesc.location,
                                land->m_lmdesc.side,
                                land->isEndLM(),
                                land->m_lmdesc.importance,
                                (dist - land->m_lmDist),
                                name,
                                land->m_lmdesc.itemID,
                                land->isTraffic(),
                                land->isDetour(),
                                land->isStart(),
                                land->isStop(),
                                land->getStreetName()
                               ) ); 

            land = static_cast<LandmarkLink*>( land->suc() );
         }
      } // End item[ i ] hasLandmarks

      // Get road size
      bool controlledAccess = false;
      bool ramp = false;
      bool driveOnRightSide = true;
      ExpandRouteReplyPacket::extractAttributes( 
         el->getAttributeSafeAt( 0/*First, but is last better?*/ ), 
         controlledAccess, ramp, driveOnRightSide );
      const int32 roadMoveBackDist = controlledAccess ? 2000 : 1000;

      // Add lanes and reverse dist
      ExpandStringLanesCont lanes;
      for ( uint32 l = 0 ; l < items[ i ]->getLanes().size() ; ++l ) {
         ExpandStringLanes lg( items[ i ]->getLanes()[ l ] );
         // If last lanes move back roadMoveBackDist. But not more than dist
         int32 laneDist = dist - lg.getDist();
         lg.setDist( MIN( dist, laneDist + roadMoveBackDist ) );
         lanes.addLaneGroup( lg );
      }
      el->setLanes( lanes );
      // Add signPosts and reverse dist
      ExpandStringSignPosts signPosts;
      for ( uint32 l = 0 ; l < items[ i ]->getSignPosts().size() ; ++l ) {
         ExpandStringSignPost lg( items[ i ]->getSignPosts()[ l ] );
         mc2dbg4 << "RouteList signpost dist " << lg.getDist() 
                 << " wpt dist " << items[i]->getDist();
         lg.setDist( MIN( dist, roadMoveBackDist ) );
         mc2dbg4 << " updated dist " << lg.getDist() << " var dist " 
                 << dist << endl;
         signPosts.addSignPost( lg );
      }
      el->setSignPosts( signPosts );

      el->setTransportationType( items[ i ]->getTransportationType() );
   }

   /* Clean up a little */
   for ( int j=0; j < erp->getNumStringData(); j++ )
      delete items[j];
   delete [] items;
   /* The gfxItems too */
   delete gfxItems;

   return true;
}

int
RouteList::getSize() const
{
   return m_routeVector.size();
}

NavStringTable&
RouteList::getStringTable()
{
   return m_stringTable;
}

int
RouteList::getTotalDist() const
{
   return m_totalDist;
}


void 
RouteList::setTotalDist( uint32 dist ) {
   m_totalDist = dist; 
}


uint32 
RouteList::getTotalTime( bool includeStandstill ) const {
   if ( includeStandstill ) {
      return m_totalTime + m_totalStandStillTime;
   } else {
      return m_totalTime;
   }
}


void 
RouteList::setTotalTime( uint32 newTime ) {
   m_totalTime = newTime;
}


const RouteElement*
RouteList::routeElementAt(int idx) const
{
   return m_routeVector[idx];
}


bool
RouteList::isTruncated() const {
   return m_truncated;
}


int 
RouteList::getTruncatedDist() const {
   return m_truncatedDist;
}


uint32 
RouteList::getTruncatedWPTNbr() const {
   return m_truncatedWPTNbr;
}


void 
RouteList::setTruncated( bool truncated ) {
   m_truncated = truncated;
}


void 
RouteList::setTruncatedDist( uint32 dist ) {
   m_truncatedDist = dist;
}


void 
RouteList::setTruncatedWPTNbr( uint32 nbr ) {
   m_truncatedWPTNbr = nbr;
}

RouteRequest* 
RouteList::getRouteRequest() {
   return m_rr;
}
