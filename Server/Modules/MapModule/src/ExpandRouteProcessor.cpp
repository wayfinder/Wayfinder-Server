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


#include "GfxUtility.h"
#include "GfxData.h"

#include "ExpandRouteProcessor.h"
#include "StringUtility.h"
#include "RouteableItem.h"
#include "StreetSegmentItem.h"
#include "FerryItem.h"
#include "BuiltUpAreaItem.h"
#include "AbbreviationTable.h"
#include "ExpandItemID.h"
#include "ExternalConnections.h"
#include "ExpandRoutePacket.h"
#include "ExpandRouteList.h"
#include "ItemTypes.h"
#include "AlteredGfxList.h"

#include "Map.h"
#include "OverviewMap.h"
#include "GenericMap.h"

#include "Connection.h"
#include "Node.h"

#include "LandmarkHead.h"
#include "RoutePacket.h"

#include "ScopedArray.h"
#include "StackOrHeap.h"
#include "GDColor.h"
#include "MapBits.h"
#include "Math.h"

#include <time.h>

#define MAXIMUM_SIZE_OF_ROADNAME 256
#define COUNT_OFF_RAMP_AS_TURN false

#define ADD_TIME_FACTOR 1.2  // Trims travel time.

#define TURN_DESC_STATE_CHANGE StringTable::LMLOCATION_ROADNAMECHANGE
//#define FILTERCOORDINATES

ExpandRouteProcessor::ExpandRouteProcessor()
      : m_possibleNames(4), m_preferedLanguages(4)
{
   m_currentRoadName    = new char[MAXIMUM_SIZE_OF_ROADNAME];
   m_currentStringIndex = 0;
   m_routeList          = new ExpandRouteHead();
   m_badGfxList         = new AlteredGfxList;
   m_noConcatinate      = false;
}

ExpandRouteProcessor::~ExpandRouteProcessor()
{
   delete [] m_currentRoadName;  
   delete m_routeList;
   delete m_badGfxList;
}



ExpandRouteReplyPacket*
ExpandRouteProcessor::processExpandRouteRequestPacket(
   const ExpandRouteRequestPacket* req,
   GenericMap* theMap)
{  
   // Create the reply and reset the members of this object
   reset(theMap, req);
   mc2log << info << "ExRoProcessor, expanding from map "
          << theMap->getMapID();
   if(req->getNoConcatinate()){
      mc2log << " with NO concatination";
   }
   if(req->getNbrPassStreets() > 0){
      mc2log << " " << (int)req->getNbrPassStreets() << " passed streets";
   }
   if(req->includeLandmarks()){
      mc2log << " landmarks included";
   }
   if(req->getRemoveAheadIfNameDiffer()){
      mc2log << " name changes removed";
   }
   if(req->getNameChangeAsTurn()){
      mc2log << " name changes as waypoints";
   }
   if(req->useAddCost()){
      mc2log << " additional costs used.";
   }
   mc2log << endl;
   
   
   ExpandRouteReplyPacket* reply = new ExpandRouteReplyPacket(req);
   
   bool useAdditionalCost =  req->useAddCost();
   
   // Get some data from the incoming packet
   byte reqType = req->getType();
   bool includeLandmarks = req->includeLandmarks();
   m_nbrOfPassedStreets  = req->getNbrPassStreets();
   reply->setRouteType(reqType);
   uint32 statusCode = StringTable::OK;
   list<uint32> nodeIDs;
   uint32 nbrItems = req->getRouteItems( theMap, nodeIDs );
   m_noConcatinate = req->getNoConcatinate();
   
   // Get the map id to country id lookup table.
   req->getMapIDToCountryID(m_mapIDToCountryID);
   
   // Short languages not handled
   m_preferedLanguages.addLast( StringTable::getNormalLanguageCode(
      StringTable::languageCode(req->getLanguage() ) ) );

   m_requestedLanguage = ItemTypes::getLanguageCodeAsLanguageType(
      StringTable::languageCode( req->getLanguage() ) );
   
   bool printAllNames =  req->getPrintAllNames();
   bool uTurn = req->getUturn();
   
   reply->setUturn(uTurn);
   m_badGfxList->clear();
   m_alteredGfxData = false;
   
   mc2dbg4 << "Packet contains " << nbrItems << " items." << endl;
   DEBUG4(
      {
         list<uint32>::const_iterator it = nodeIDs.begin();
         uint32 i = 0;
         while ( it != nodeIDs.end() ) {
            mc2dbg4 << "item [" << i << "] = 0x" << hex << 
            *it << dec << endl;
            ++i;
            ++it;
         }
      }
   );
 
   
   // Make sure the route has at least one item...
   if (nbrItems > 0) {
      // For all items in the route lookup the item and insert it into
      // the list.
      //uint32 startTime = TimeUtility::getCurrentMicroTime();
      list<uint32>::const_iterator it = nodeIDs.begin();
      while ( it != nodeIDs.end() ) {
         uint32 nodeID = *it;
         if ( !GET_ADDITIONAL_COST_STATE(nodeID) ) {
            RouteableItem* item = Item::routeableItem(theMap->itemLookup(nodeID));
            ExpandRouteLink* link = new ExpandRouteLink(nodeID, item );
            link->setDrivingSide( getDriveOnRightSide( nodeID ) );
            link->into( m_routeList );
         } else {
            ExpandRouteLink* lastLink = static_cast<ExpandRouteLink*>
               (m_routeList->last());
            // StandstillTime not affected.
            if(useAdditionalCost){
               mc2dbg8 << "ADDITIONAL_COST_STATE : "
                    << GET_ADDITIONAL_COST_STATE(nodeID) << " s." << endl;
               lastLink->addTimes(GET_ADDITIONAL_COST_STATE(nodeID),0);
               mc2dbg4 << "[ERP] Removed additional cost statenode : "
                       << GET_ADDITIONAL_COST_STATE(nodeID) << endl;
            }
            
         }
         ++it;
      }
      
      uint32 x;
      if(GET_TRANSPORTATION_STATE(nodeIDs.front()) != 0){
         x = 1;
      }
      else 
         x = 0;
      
//      // Add the last one twice, to compensate for the fence-effect
//      // However if routing on the same segment, no extra link is needed.
//      if ( (nbrItems != (2+x) ) || 
//           ( req->ignoreEndOffset() ) ||
//           (req->getRouteItem(x) != req->getRouteItem(x+1))  ) {
//         uint32 nodeID = req->getRouteItem(req->getNbrItems()-1);
//         RouteableItem* item = dynamic_cast<RouteableItem*>
//            (theMap->itemLookup(nodeID));
//         (new ExpandRouteLink(nodeID, item))->into(m_routeList);
//         mc2dbg2 << "Added extra end link" << endl;
//      }

      // Add the last one twice, to compensate for the fence-effect
      // However if routing on the same segment, no extra link is needed.
      // Check if to add an extra link to the end.
      bool addExtraLink = false;

      if ( ( nbrItems != (2+x) ) ||
           ( req->ignoreEndOffset() ) ) {
         addExtraLink = true;
      } else {
         it = nodeIDs.begin();
         for ( uint32 i = 0; i < x; ++i ) 
            ++it;
         list<uint32>::const_iterator nextIt = it;
         ++nextIt;
         if (*it != *nextIt ) {
            addExtraLink = true;
         }
      }

      if ( addExtraLink ) {
         ExpandRouteLink* lastLink = static_cast<ExpandRouteLink*>
            (m_routeList->last());
         RouteableItem* item = static_cast<RouteableItem*>
            (theMap->itemLookup(lastLink->m_nodeID));
         ExpandRouteLink* link = new ExpandRouteLink(lastLink->m_nodeID,
                                                     item);
         link->setDrivingSide( lastLink->driveOnRightSide() );
         link->into( m_routeList );
         mc2dbg << "Added extra end link " << MC2HEX( lastLink->m_nodeID)
                << endl;
      }
   
      
      //mc2dbg << "TIME: All links created in " 
      //       << (Utility::getCurrentMicroTime()-startTime)/1000.0 
      //       << " ms" << endl;


      // Set the state in all the links. After this call, no links will
      // have m_item == NULL.
      //startTime = TimeUtility::getCurrentMicroTime();
      
      if(!setTransportationType()){
         // We have unknown itemIDs in the route.
         // This route is impossible to expand.
         statusCode = StringTable::ERROR_NO_ROUTE;
         reply->setStatus(statusCode);

         // No reason to to do anything else.
         return reply;
      }
      
      //mc2dbg << "TIME: Transportation type set in " 
      //       << (Utility::getCurrentMicroTime()-startTime)/1000.0 
      //       << " ms" << endl;
      
      // Set the distance in all links.
      //startTime = TimeUtility::getCurrentMicroTime();
      setDistance();
      //mc2dbg << "TIME: Distance set in " 
      //       << (Utility::getCurrentMicroTime()-startTime)/1000.0 
      //       << " ms" << endl;
      
      // Set the turndescriptions of if the links in the list
      // Set data that is stored in the connections, such as:
      //    turndescription
      //    traverse time
      //    crossingkind
      //    signposts
      //startTime = TimeUtility::getCurrentMicroTime();
      storeDataFromConnections(uTurn, req->getType());
      //mc2dbg << "TIME: Data from connections stored in " 
      //       << (Utility::getCurrentMicroTime()-startTime)/1000.0 
      //       << " ms" << endl;

      if(includeLandmarks) {
         // Add landmarks in the route list
         //startTime = TimeUtility::getCurrentMicroTime();
         addLandmarks(regionIDToCountryCode(req->getLastCountry()), 
                      m_mapIDToCountryID);
         
         //mc2dbg << "TIME: Landmarks added in " 
         //       << (Utility::getCurrentMicroTime()-startTime)/1000.0 
         //       << " ms" << endl;
      }
      //uint32 startTime = TimeUtility::getCurrentMicroTime();
      createTrafficLandmarks(req);

      //cerr << "TIME: createTrafficLandmarks  in " 
      //      << (Utility::getCurrentMicroTime()-startTime)/1000.0 
      //       << " ms" << endl;
      
         //}
      // See if we can merge some turns to others.
      //startTime = TimeUtility::getCurrentMicroTime();
      mergeMultiTurns();
      //mc2dbg << "TIME: Turns merged in " 
      //       << (Utility::getCurrentMicroTime()-startTime)/1000.0 
      //       << " ms" << endl;

      if((req->getType() & ROUTE_TYPE_NAVIGATOR) !=0)
         m_alteredGfxData = findBadGfxCoords();
      else
         m_alteredGfxData = false;
      // Uppdate the distance and time for the first and last link. 
      mc2dbg4 << "startOffset=" << req->getStartOffset() 
              << ", endOffset=" << req->getEndOffset() << endl;
      //startTime = TimeUtility::getCurrentMicroTime();
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
/* Test for GCC > 3.2.0 */
#if GCC_VERSION > 30200
      updateStartAndEnd(req->getStartOffset(),
                        req->ignoreStartOffset(),
                        req->getStartDir(),
                        req->getEndOffset(),
                        req->ignoreEndOffset(),
                        req->getEndDir(),
                        req->getLastID(),
                        req->getLastMapID(),
                        uTurn,
                        req->getType() );
#else
      {int pos = REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE;
      byte b = req->incReadByte( pos );
      updateStartAndEnd(req->getStartOffset(),
                        b & (1<<4),
                        b & (1<<7),
                        req->getEndOffset(),
                        b & (1<<5),
                        b & (1<<6),
                        req->getLastID(),
                        req->getLastMapID(),
                        uTurn,
                        req->getType() );}
#endif
      // Exchange UNDEFINED_TURNs with AHEAD_TURNs
      removeUndefinedTurns();
      
      // Set the street counts to the left and right
      newSetTurnNumbersAndRoundabouts( true );

      setNamesAndConcatenate( !req->ignoreStartOffset(),
                              !req->ignoreEndOffset(),
                              printAllNames,
                              req->getRemoveAheadIfNameDiffer(),
                              (req->getType() & ROUTE_TYPE_NAVIGATOR)!=0,
                              req->getType(),
                              req->getNameChangeAsTurn());

      expandNames();

      setPossibleTurns();
      
      // Add turndescriptions due to changes of transportation type, 
      // for example "park car".
      addTransportationChanges();

      if ( includeLandmarks ) {
         
         // Update locations for buaLMs into <-> pass
         updateLandmarksBeforeSelect();
          
         // Look at all landmarks and remove the ones that should not
         // be included in the final route.
         selectLandmarks();

         // Update location e.g. pass -> at for endLMs when the 
         // turndescription is ahead. Also sort the landmarks so 
         // that buaLMs are in the correct order.
         updateLandmarks();

         // Turn the route links vectors of passed streets into landmarks.
         //startTime = TimeUtility::getCurrentMicroTime();
         if ( m_nbrOfPassedStreets != 0 ) {
            mc2dbg4 << "Nbr of required passed streets : "
                 << m_nbrOfPassedStreets << endl;
            createPassRoadLandmarks(m_nbrOfPassedStreets);
         }         
      }

      // DEBUG-code
      mc2dbg4 << "StringTable::DRIVE_FINALLY = "
              << StringTable::getString(StringTable::DRIVE_FINALLY, 
                                        StringTable::ENGLISH) << endl;
      DEBUG4( m_routeList->dump(); );
      mc2dbg4 << "reply->getBufSize()=" << reply->getBufSize() 
              << "reply->getLength()=" << reply->getLength() << endl;
      // End debug-code
      // ===================================================
      // -+-+-+-+-+-+- Save the list into the packet -+-+-+-+-+-+-+-
      // check before save.
      if(static_cast<ExpandRouteLink*>
         (m_routeList->first())->m_nbrItems != 0 ){
         mc2log << warn << " The first routeLink has not zero items "
                << "better check that the groups are allright in the result!"
                << endl;
         // Remove this warning if format changes.
      }
      // Write total dist, time and sst into the reply packet
      uint32 totDist, totTime, totSST;
      getTotals(totDist, totTime, totSST, !req->ignoreStartOffset());
      reply->setTotalDist(totDist);
      reply->setTotalTime(totTime + totSST);
      reply->setStandStillTime(totSST);
      mc2dbg4 << "totDist=" << totDist << ", totTime=" << totTime
              << ", totSST=" << totSST << endl;

      // Add start-direction
      reply->setStartDirectionHousenumber(m_startDirHousenumber);
      reply->setStartDirectionOddEven(m_startDirOddEven);

      // As strings !?
      if (reqType & ROUTE_TYPE_STRING) {
         m_routeList->saveStringDataIntoPacket(
            reply,
            (req->getType() & ROUTE_TYPE_GFX_TRISS) );
      }
      
      // As gfx ?!
      if (reqType & ROUTE_TYPE_GFX) {
         // The user wants graphics
         saveRouteGfxIntoPacket(reply, req, nodeIDs, nbrItems);
      }

      // As Items/String ?!
      if ( reqType & ROUTE_TYPE_ITEM_STRING ) {
         if (m_routeList->saveItemsPerStringIntoPacket(reply)) { 
            mc2dbg8 << "The items per string data is transferred "
                    << "from list to packet" << endl;
         } else {
            mc2log << warn 
                   << "Error transfering items per string from list to packet" 
                   << endl;
         }
      }
      //mc2dbg << "TIME: Route saved in reply packet in "
      //       << (Utility::getCurrentMicroTime()-startTime)/1000.0 
      //       << " ms" << endl;

   } else {
      mc2log << warn 
             << "ExpandRouteProcessor:: Single point in route" << endl;
      statusCode = StringTable::SINGLEPOINT_IN_ROUTE;
   }

   // Return the packet
   reply->setStatus(statusCode);
   mc2dbg4 << "reply->getBufSize()=" << reply->getBufSize() 
           << "reply->getLength()=" << reply->getLength() << endl;

   return (reply);
}

bool
ExpandRouteProcessor::commonName(ExpandRouteLink* newLink,
                              ExpandRouteLink* oldLink)
{
   // Check all combinations of names agains each other. Return
   // when if a common name is found.
   Item* newItem = newLink->m_item;
   Item* oldItem = oldLink->m_item;
   
   for (uint32 i=0; i < newItem->getNbrNames(); i++) {       
      for (uint32 j=0; j< oldItem->getNbrNames(); j++) { 
         if ( newItem->getStringIndex(i) == 
              oldItem->getStringIndex(j)) {
            mc2dbg4 << "commonName returning true" << endl; 
         return (true);
         }
      }
   }
   mc2dbg4 << "commonName returning false" << endl; 
   return (false);
}

bool
ExpandRouteProcessor::newName(ExpandRouteLink* newLink,
                              ExpandRouteLink* oldLink)
{
   // Check all combinations of names agains each other.
   Item* newItem = newLink->m_item;
   Item* oldItem = oldLink->m_item;
   
   for (uint32 i=0; i < newItem->getNbrNames(); i++) {       
      bool thisNew = true;
      for (uint32 j=0; j< oldItem->getNbrNames(); j++) {
         if ( newItem->getStringIndex(i) == 
              oldItem->getStringIndex(j)) {
            thisNew = false;
         }
      }
      if(thisNew)
         return true;
   }
   mc2dbg4 << "newName returning true" << endl; 
   return (false);
}

bool
ExpandRouteProcessor::namePresent(ExpandRouteLink* newLink,
                                  uint32 stringCode)
{
   if((stringCode == MAX_UINT32) ||
      (stringCode == 0)){
      return false;
   }
   if(stringCode == newLink->m_stringCode)
      return true;
   
   Item* newItem = newLink->m_item;
   for (uint32 i=0; i < newItem->getNbrNames(); i++) {
      if(GET_STRING_INDEX(stringCode) == newItem->getStringIndex(i)){
         return true;
      }
   }
   return false;
   
}

bool
ExpandRouteProcessor::getTotals(uint32 &totDist, 
                                uint32& totTime, 
                                uint32& totStandStillTime,
                                bool firstPacket)
{
   // Reset the outparameters
   totDist = 0;
   totTime = 0;
   totStandStillTime = 0;
   
   ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>
                              (m_routeList->first());
   // Skip the first link if it is the first packet, since there will
   // be the angle stored in the distance variable
   if ((curLink != NULL) && (firstPacket)) { 
      curLink = static_cast<ExpandRouteLink*> (curLink->suc());
   }
   // Ackumulate the total-values
   while (curLink != NULL) {
      totDist += curLink->m_dist;
      mc2dbg4 << "curLink->m_dist = " << curLink->m_dist << endl;
      totTime += curLink->m_time;
      mc2dbg4 << "curLink->m_time = " << curLink->m_time << endl;
      mc2dbg4 << "speed = "
           << (float(curLink->m_dist) / float(curLink->m_time)) << endl;
      totStandStillTime += curLink->getStandStillTime();
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   }
   
   return (true);
   
}


bool
ExpandRouteProcessor::removeUndefinedTurns()
{
   // Exchanges undefined turns with ahead turns.
   ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>
                            (m_routeList->first());
   
   while (curLink != NULL) {
      if (curLink->m_turnDescription == StringTable::UNDEFINED_TURN) {
         curLink->m_turnDescription = StringTable::AHEAD_TURN;
      }
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   }
   
   return (true);
}

bool
ExpandRouteProcessor::setStreetCounts()
{
   return (true);
}



bool
ExpandRouteProcessor::setDistance()
{
   // Set the distanceto 0 for the first node
   ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>
                              (m_routeList->first());
   curLink->m_dist = 0;
   uint32 dist = uint32( rint( curLink->m_item->getLength() ) );
   // Set the distance for all nodes
   curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   while (curLink != NULL) {
      curLink->m_dist = dist;
      dist = uint32( rint( curLink->m_item->getLength() ) );
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   }

   return (true);
}


bool
ExpandRouteProcessor::updateStartAndEnd(
      uint16 startOffset, bool ignoreStartOffset, bool startTowards0,
      uint16 endOffset, bool ignoreEndOffset, bool endTowards0,
      uint32 lastID, uint32 lastMapID, bool uTurn, byte routeType)
   
{
   float64 startFactor = 0;
   float64 endFactor = 0;
   
   // Set the start angle, distance and time
   ExpandRouteLink* startLink = static_cast<ExpandRouteLink*>
                                (m_routeList->first());
   // If the first link starts from node 0 -> invert the startfactor
   // because we should then measure the distance from startOffset to node 1
   bool invertStartFactor = (GET_UINT32_MSB(startLink->m_nodeID) == 0);
   
   startLink = static_cast<ExpandRouteLink*>(startLink->suc());
   // Store the distance and time before compensating for offsets
   uint32 startTime = startLink->m_time;
   uint32 startDist = startLink->m_dist;
   if ((startLink != NULL) && (!ignoreStartOffset)) {
      startFactor = float64(startOffset / float64(MAX_UINT16));
      // Check that we count distance from right node.
      if(invertStartFactor)
         startFactor = 1-startFactor;
      float64 startAngle = 
         static_cast<ExpandRouteLink*>(m_routeList->first())
            ->m_item->getGfxData()->getAngle(startOffset);
      
      if (startTowards0) {
         startAngle += 180;
         if (startAngle > 360) {
            startAngle -= 360;
         }
      }
      
      if (uTurn) {
         startAngle += 180;
         if (startAngle > 360)
            startAngle -=360;
      }

      
      if( !(routeType & ROUTE_TYPE_GFX_TRISS) )
      {
         startLink->m_dist = uint32(startLink->m_dist * startFactor + 0.5);
         startLink->m_time = uint32(startLink->m_time * startFactor + 0.5);
      // This isn't exactly logical, but is here in order to get times that
      // match the ones the routemodule calculates...
         //  startLink->m_standStillTime = 
         //   uint32(startLink->m_standStillTime * startFactor + 0.5);

         // Also update the dist for all landmarks in startLink
         LandmarkLink* lmLink = 
            static_cast<LandmarkLink*>(startLink->m_landmarks->first());
         while (lmLink != NULL) {
            lmLink->m_lmDist = startLink->m_dist;
            lmLink = static_cast<LandmarkLink*>(lmLink->suc());
         }
      }
      // Set the start-angle as the distance in the _first_ link
      ExpandRouteLink* theFirstLink =
         static_cast<ExpandRouteLink*>(m_routeList->first());
      theFirstLink->m_dist = uint32(startAngle);
      
      // Find out if we will start in increasing or decreasing house-number
      if (theFirstLink->m_item->getItemType() == ItemTypes::streetSegmentItem)
      {
         StreetSegmentItem* ssi = static_cast<StreetSegmentItem*>
            (theFirstLink->m_item);

         const uint16 leftStart = m_map->getStreetLeftSideNbrStart( *ssi );
         const uint16 leftEnd = m_map->getStreetLeftSideNbrEnd( *ssi );
         const uint16 rightStart = m_map->getStreetRightSideNbrStart( *ssi );
         const uint16 rightEnd = m_map->getStreetRightSideNbrEnd( *ssi );
         
         mc2dbg8 << "Housenumbers L / R : " << leftStart << "-" << leftEnd 
                 << " / " << rightStart << "-" << rightEnd << ", ID=" 
                 << ssi->getID() << endl;
         
         // Make sure that we have any housenumbers
         if ( ((leftStart != 0) || (rightStart!= 0)) &&
              ((leftStart != leftEnd) || (rightStart != rightEnd))) {
            // Determine if we're driving towards higher or lower housenumbers
            if ( (leftStart < leftEnd)  && (rightStart < rightEnd)) {
               // Housenumbers increasing in positive direction
               if (startTowards0) {
                  // Driving in negative direction
                  mc2dbg8 << "   DRIVING TOWARDS LOWER HOUSENUMBERS (1)" 
                          << endl;
                  m_startDirHousenumber = ItemTypes::decreasing;
               } else {
                  // Driving in positive direction
                  mc2dbg8 << "   DRIVING TOWARDS HIGHER HOUSENUMBERS (1)" 
                          << endl;
                  m_startDirHousenumber = ItemTypes::increasing;
               }
            } else if ( (leftStart >= leftEnd)  && (rightStart >= rightEnd)) {
               // Housenumbers decreasing in positive direction
               if (startTowards0) {
                  // Driving in negative direction
                  mc2dbg8 << "   DRIVING TOWARDS HIGHER HOUSENUMBERS (2)" 
                          << endl;
                  m_startDirHousenumber = ItemTypes::increasing;
               } else {
                  // Driving in positive direction
                  mc2dbg8 << "   DRIVING TOWARDS LOWER HOUSENUMBERS (2)" 
                          << endl;
                  m_startDirHousenumber = ItemTypes::decreasing;
               }
            } else {
               mc2dbg8 << "   Some kind of mixed housenumbers..." << endl;
            }

            // Determine if we have odd at the left or right side
            ItemTypes::streetNumberType nbrType = ssi->getStreetNumberType();
            if (startTowards0) {
               // Driving in negative direction
               if (nbrType == ItemTypes::leftEvenStreetNumbers) {
                  mc2dbg8 << "   Odd number to the left, even to the right (1)"
                          << endl;
                  m_startDirOddEven = ItemTypes::leftOddRightEven;
               } else if (nbrType == ItemTypes::leftOddStreetNumbers) {
                  mc2dbg8 << "   Even number to the left, odd to the right (1)"
                          << endl;
                  m_startDirOddEven = ItemTypes::rightOddLeftEven;
               } else {
                  mc2dbg8 << "   Some other kind of street numbering (1)" 
                          << endl;
               }
            } else {
               // Driving in positive direction
               if (nbrType == ItemTypes::leftEvenStreetNumbers) {
                  mc2dbg8 << "   Even number to the left, odd to the right (2)"
                          << endl;
                  m_startDirOddEven = ItemTypes::rightOddLeftEven;
               } else if (nbrType == ItemTypes::leftOddStreetNumbers) {
                  mc2dbg8 << "   Odd number to the left, even to the right (2)"
                          << endl;
                  m_startDirOddEven = ItemTypes::leftOddRightEven;
               } else {
                  mc2dbg8 << "   Some other kind of street numbering (2)" 
                          << endl;
               }
            }

         }
      }
   } else if (startLink != NULL) {
      // Does not start here for real. The packet is part of an expand.
      mc2dbg4 << "  Do not start ... " << endl;
      StringTable::stringCode turnCode = StringTable::AHEAD_TURN;
      BoundrySegmentsVector* extConns = m_map->getBoundrySegments();

      // If routing to the overview map, no boundrySegmentsVector
      // If routing from the overview map, no bs with connection from 
      // the ssi overviewMapID.overviewItemID.
      
      if ( extConns != NULL ) {
         ExpandRouteLink* firstLink = 
            static_cast<ExpandRouteLink*>(m_routeList->first());
         // There can be more than one bs in this map with connections 
         // from lastMapID.lastID, collect all and then loop until 
         // the correct one is found.
         BoundrySegmentVector bsCand;
         extConns->getBoundrySegments(lastMapID, lastID, &bsCand);
         uint32 i = 0;
         bool found = false;
         mc2dbg1 << bsCand.size() << " bs has conn from " 
                 << lastMapID << "." << lastID 
                 << " - looking for m_item " << firstLink->m_item->getID() 
                 << " nodeID " << firstLink->m_nodeID << endl;
         while ((i < bsCand.size()) && !found) {
            BoundrySegment* bs = 
               dynamic_cast<BoundrySegment*>(bsCand[ i ]);
            if (bs->getConnectRouteableItemID() == 
                  firstLink->m_item->getID()) {
               byte closeNode = 0;
               if (bs->getCloseNodeValue() == BoundrySegment::node1close)
                  closeNode = 1;
               uint32 c = 0;
               while ((c < bs->getNbrConnectionsToNode(closeNode)) && !found) {
                  Connection* conn = bs->getConnectToNode(closeNode, c);
                  if (conn->getConnectFromNode() == lastID) {
                     mc2dbg8 << " Found external connection" << endl;
                     found = true;
                     turnCode = conn->getTurnStringCode();
                     if (turnCode == StringTable::UNDEFINED_TURN)
                        turnCode = StringTable::AHEAD_TURN;
                  }
                  c++;
               }
            }
            i++;
         }
         if (!found) {
            turnCode = StringTable::AHEAD_TURN;
         }
      } else {
         mc2log << warn << " ERROR: external connection = NULL"
                << endl;
         turnCode = StringTable::AHEAD_TURN;
      }
      mc2dbg8 << "Startlink turndesc set to " << 
         StringTable::getString(turnCode, StringTable::ENGLISH)
         << endl;
      // Modify the turndescription of the _first_ node (we don't have
      // any "start at bla bla..."
      // NO! Modify the _second_ node (=startLink), since the first node 
      // (= a virtual ssi) will always be removed in setNamesAndConcatenate
      startLink->m_turnDescription = turnCode;
      
   } else {
      mc2dbg4 << "ExpandRouteProcessor:: Intructed not to use "
              << "offset for start" << endl;
   }
   
   // Set the end distance and time
   ExpandRouteLink* endLink = static_cast<ExpandRouteLink*>
                              (m_routeList->last());
   if ((endLink != NULL) && (!ignoreEndOffset)) {
      endFactor = float64(endOffset / float64(MAX_UINT16));
      // Check that we count distance from right node.
      if(GET_UINT32_MSB(endLink->m_nodeID) != 0)
         endFactor = 1-endFactor;
      
      mc2dbg8 << "END BEFORE: time=" << endLink->m_time 
              << ", dist=" << endLink->m_dist << endl;
      if( !(routeType & ROUTE_TYPE_GFX_TRISS) )
      {
         endLink->m_dist = uint32(endLink->m_dist * endFactor + 0.5);
         endLink->m_time = uint32(endLink->m_time * endFactor + 0.5);
         // This isn't exactly logical, but is here in order to get times that
         // match the ones the routemodule calculates...
         endLink->setStandStillTime(
            uint32(endLink->getStandStillTime() * endFactor + 0.5));
         
         // Also update the dist for all landmarks in endLink
         // - in addLandmarks only the distance before the last link 
         // is considered, thus e.g. the boarder of buas that you drive 
         // into are located further before the endpoint -> subtract!
         LandmarkLink* lmLink = 
            static_cast<LandmarkLink*>(endLink->m_landmarks->first());
         while (lmLink != NULL) {
            lmLink->m_lmDist -= endLink->m_dist;
            lmLink = static_cast<LandmarkLink*>(lmLink->suc());
         }
      }
      
      mc2dbg8 << "END AFTER:  time=" << endLink->m_time 
              << ", dist=" << endLink->m_dist << endl;
   } else {
      mc2log << info
             << "ExpandRouteProcessor:: Did not use offset for dest" << endl;
   }
   
   // Take care of the special case when routing within the same segment,
   // ie. start segment is same as end segment.
   if ((startLink != NULL) && (startLink == endLink) &&
       (!ignoreStartOffset) && (!ignoreEndOffset)) {
      mc2dbg4 << "Routing within the same segment!" << endl;
      // Check if startFactor or endFactor has been inverted, 
      // change back to get the correct distance.
      if (invertStartFactor) {
         startFactor = 1 - startFactor;
      } else {
         endFactor = 1 - endFactor;
      }
      float64 factor = abs(startFactor - endFactor);
      
      mc2dbg8 << "startfactor " << startFactor 
              << ", endfactor " << endFactor
              << ", factor " << factor << endl;
      
      
      startLink->m_dist = uint32(factor * startDist + 0.5);
      startLink->m_time = uint32(factor * startTime + 0.5);
   }
   
   // Update the last items turndescription
   if (endLink != NULL) {
      
      StringTable::stringCode finalCode = 
                StringTable::DRIVE_FINALLY;
      if ( ignoreEndOffset ) {
         finalCode = StringTable::AHEAD_TURN;
      }
      
      endLink->m_turnDescription = finalCode;
   }
  
   return (true);
}


void
ExpandRouteProcessor::mergeMultiTurns()
{
   if(m_routeList->cardinal() < 3)
      return ;
   ExpandRouteLink* firstLink = static_cast<ExpandRouteLink*>
      (m_routeList->first());
   ExpandRouteLink* secondLink = static_cast<ExpandRouteLink*>
      (firstLink->suc());
   ExpandRouteLink* thirdLink = static_cast<ExpandRouteLink*>
      (secondLink->suc());
   while(thirdLink != NULL)
   {
      if(thirdLink->m_dist < 50)
      {
         StreetSegmentItem* ssiFirst = item_cast<StreetSegmentItem*>
            (firstLink->m_item);
         StreetSegmentItem* ssiSecond = item_cast<StreetSegmentItem*>
            (secondLink->m_item);
         StreetSegmentItem* ssiThird = item_cast<StreetSegmentItem*>
            (thirdLink->m_item);
         if((ssiFirst == NULL)||(ssiSecond == NULL)||(ssiThird == NULL))
         {
            // Do nothing.
            mc2dbg4 << "Dynamic cast failed not ssi's. No merge!" << endl;
         }
         else if((ssiFirst->isRoundabout()) || (ssiSecond->isRoundabout()) ||
                 (ssiThird->isRoundabout()))
         {
            // Part of a roundabout do nothing here.
            mc2dbg4 << "Roundabout! No merge !" <<endl;
         }
         else if(thirdLink->m_dist > uint32(50-5*(ssiFirst->getRoadClass())))
         {
            // Do nothing.
            mc2dbg4 << "Distance to long for roadClass! No merge!" << endl;
         }
       
         // FIXME: Merge these two into one (left left and right right)
         // Be careful.
         // If the check here is too lax, then two left-left or
         // right-right might be merged into a U-turn when they should
         // not. A strange thing is that I do not see any distance
         // comparison here.
         // Left Left to UTURN
         else if(secondLink->driveOnRightSide() && 
                 ssiFirst->isMultiDigitised() &&
                 ssiThird->isMultiDigitised() &&
                 (secondLink->m_turnDescription == StringTable::LEFT_TURN) &&
                 (thirdLink->m_turnDescription == StringTable::LEFT_TURN))
         {
            mc2dbg8 << "Trying LEFT LEFT -> UTURN!" << endl;
            float64 totAngle = getConnectionAngle(firstLink, thirdLink);
            if((totAngle > 5.5 * M_PI_6) || (totAngle < -5.5 * M_PI_6))
            {
               // merge second to third.
               // change turn desc on third to UTURN
               m_routeList->concatinateLinks(thirdLink, false);
               thirdLink->m_turnDescription = StringTable::U_TURN;
            }
         }
         
         // Right Right to UTURN
         else if(!secondLink->driveOnRightSide() && 
                 ssiFirst->isMultiDigitised() &&
                 ssiThird->isMultiDigitised() &&
                 (secondLink->m_turnDescription == StringTable::RIGHT_TURN) &&
                 (thirdLink->m_turnDescription == StringTable::RIGHT_TURN))
         {
            mc2dbg8 << "Trying RIGHT RIGHT -> UTURN!" << endl;
            float64 totAngle = getConnectionAngle(firstLink, thirdLink);
            if((totAngle > 5.5 * M_PI_6) || (totAngle < -5.5 * M_PI_6))
            {
               // merge second to third.
               // change turn desc on third to UTURN
               m_routeList->concatinateLinks(thirdLink, false);
               thirdLink->m_turnDescription = StringTable::U_TURN;
            }
         }
         
         // Left Right or Right Left to AHEAD
         else if((thirdLink->m_dist < uint32(15-2*(ssiFirst->getRoadClass())))
                 && 
                 (((secondLink->m_turnDescription == StringTable::LEFT_TURN) &&
                   (thirdLink->m_turnDescription == StringTable::RIGHT_TURN))||
                  ((secondLink->m_turnDescription == StringTable::RIGHT_TURN)&&
                   (thirdLink->m_turnDescription == StringTable::LEFT_TURN))))
         {
            mc2dbg8 << "Trying LEFT RIGHT -> AHEAD!" << endl;
            float64 totAngle = getConnectionAngle(firstLink, thirdLink);
            if((totAngle < 0.5*M_PI_6) && (totAngle > -0.5 * M_PI_6))
            {
               // merge second to third.
               // change turn desc on third to AHEAD.
               m_routeList->concatinateLinks(thirdLink, false);
               thirdLink->m_turnDescription = StringTable::AHEAD_TURN;

               // Update lm-descriptions for any after|before|at|in-landmarks
               // on third link to pass, side unknown
               LandmarkLink* lmLink = 
                  static_cast<LandmarkLink*>(thirdLink->m_landmarks->first());
               while (lmLink != NULL) {
                  if((lmLink->m_lmDist == int32(thirdLink->m_dist)) ||
                     (lmLink->m_lmdesc.location != ItemTypes::pass)) {
                     mc2dbg8 << "for LM " << lmLink->getLMName()
                          << " change loc=" << int(lmLink->m_lmdesc.location)
                          << " side=" << int(lmLink->m_lmdesc.side)
                          << " to pass and unknown_side." << endl;
                     lmLink->m_lmdesc.location = ItemTypes::pass;
                     lmLink->m_lmdesc.side = SearchTypes::unknown_side;
                  }
                  lmLink = static_cast<LandmarkLink*>(lmLink->suc());
               }
               
            }
         }
      }
      firstLink  = static_cast<ExpandRouteLink*>
         (firstLink->suc());
      secondLink = static_cast<ExpandRouteLink*>
         (firstLink->suc());
      if(secondLink != NULL) 
         thirdLink  = static_cast<ExpandRouteLink*>
            (secondLink->suc());
      else
         thirdLink = NULL;
   }
}


bool
ExpandRouteProcessor::setTransportationType()
{
   ItemTypes::transportation_t curState = ItemTypes::drive;
   ItemTypes::transportation_t nextState = curState;
   ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>
                              (m_routeList->first());
      
   // Set the state for all the nodes
   while (curLink != NULL) {
      
      // If the item can't be found, that means it is a statechange!
      // No, it does not !!
      if (curLink->m_item == NULL &&
          GET_TRANSPORTATION_STATE(curLink->m_nodeID)) {
         mc2dbg8 << "GET_TRANSPORTATION_STATE(" 
                 << hex << curLink->m_nodeID << dec
                 << ") = "
                 << uint32(GET_TRANSPORTATION_STATE(curLink->m_nodeID))
                 << endl;
         // Change state and remove curLink from the link
         
         // Note that the state shouldn't be changed of curLink->suc(),
         // but the link after that. This is because the state
         // refers to the distance travelled of the previous link.
         // 
         // For instance:
         // 
         // DRIVE 100 meters (of the previous link) and then park. 
         // (state=drive)
         //
         // and the following link could be:
         // Then WALK 200 meters (state=walk)
         
         nextState = GET_TRANSPORTATION_STATE(curLink->m_nodeID);
   
         // However an exception of the above is when the route starts
         // with a change of transportation, for instance when routing
         // a pedestrian. In this case we want the state to be walk
         // right from the beginning.
         if (curLink == m_routeList->first()) {
            curState = nextState;
         }
         ExpandRouteLink* deleteLink = curLink;
         curLink = static_cast<ExpandRouteLink*>(curLink->suc());
         deleteLink->out();
         delete deleteLink;
      } else if (curLink->m_item == NULL){
         mc2log << error << "Item was null but no state change" << endl
                << " nodeID was " << curLink->m_nodeID << endl;
         mc2log << "RoutList size : " << m_routeList->cardinal() << endl;
         mc2log << "***************************************" << endl;
         curLink->dump();
         mc2log << "***************************************" << endl;
         m_routeList->dump();
         
         // abort(); // We dont do abort() better to return a reply.
         return false;
      } else {
         // Set state
         mc2dbg8 << "Setting state to " << uint32(curState) << endl;
         curLink->m_transportation = curState;
         curLink = static_cast<ExpandRouteLink*>(curLink->suc());
         curState = nextState;
      }
   }

   return (true);
}

bool
ExpandRouteProcessor::addLandmarks(uint32 prevCountryID,
                                   const map<uint32, uint32>& mapIDToCountryID)
{
   mc2dbg1 << "addLandmarks() - adding LMs to the route." << endl;
   ExpandRouteLink* prevLink =
      static_cast<ExpandRouteLink*>(m_routeList->first());
   ExpandRouteLink* curLink = 
      static_cast<ExpandRouteLink*>(prevLink->suc());

   typedef GenericMap::landmarkTable_t::iterator LI;
   GenericMap::landmarkTable_t lmTable = m_map->getLandmarkTable();
   

   // Used when finding "pass"-buas resp "turn after"-buas
   uint32 oldBuaID = MAX_UINT32;
   uint32 buaID = MAX_UINT32;
   uint32 nextBuaID = MAX_UINT32;
   // Importance of the builtUpAreas..
   uint32 oldImportance = MAX_UINT32;  //oldBuaID
   uint32 importance = MAX_UINT32;     //buaID
   // Used to get a distance when passing or turning in buas,
   // gives the distance from the bua border to the turn/end of the bua
   uint32 buaDist = 0;
   // If the routeable item before passig a buaborder has location = that bua,
   // its length should not be counted into buaDist
   bool skipFirstLinkInBuaDist = false;
   // Used to check if the routeable item after passing a buaboarder 
   // has location = that bua,
   bool skipLast = false;

   // Variabel to hold info about previous underviewMap when routing 
   // in overviewMaps. Used for checking if a new country is entered.
   uint32 lastTrueMapID = MAX_UINT32;

   
   // If the route enters a new country, add landmark!
   bool addCountryLM = false;
   StringTable::countryCode newCountry = StringTable::NBR_COUNTRY_CODES;
   OverviewMap* overviewMap = dynamic_cast<OverviewMap*>(m_map);
   if ((overviewMap == NULL) && 
       !MapBits::isOverviewMap(m_map->getMapID())) {
      StringTable::countryCode curCountry = m_map->getCountryCode();
      mc2dbg8 << " from country " << prevCountryID << " to country "
              << int(curCountry) << endl;
      if ((prevCountryID != MAX_UINT32) && 
          (curCountry != StringTable::countryCode(prevCountryID)) &&
          (curCountry < StringTable::NBR_COUNTRY_CODES) &&
          (prevCountryID < StringTable::NBR_COUNTRY_CODES)) {
         // We have a new country in this map
         addCountryLM = true;
         newCountry = curCountry;
         mc2dbg4 << "addLandmarks: Entering new country: "
                 << StringTable::getStringToDisplayForCountry(
                         newCountry,
                         StringTable::languageCode(
                            m_preferedLanguages.getElementAt(0)))
                 << " (leaving "
                 << StringTable::getStringToDisplayForCountry(
                         StringTable::countryCode(prevCountryID),
                         StringTable::languageCode(
                            m_preferedLanguages.getElementAt(0)))
                 << ")" << endl;
      }
   } // else the check is within the while-loop

   // If the route start in a bua, add it to a vector with "used buas";
   Vector allBuas;
   bool buaUsed = false;      // buaID was used as a landmark
   bool oldBuaUsed = false;   // oldBuaID was used as a landmark
   buaID = m_map->getRegionID(prevLink->m_item, ItemTypes::builtUpAreaItem);
   if (buaID != MAX_UINT32) {
      allBuas.addLast(buaID);
      buaUsed = true;
      mc2dbg4 << "route starts in bua " << m_map->getFirstItemName(buaID)
              << endl;
   }
   if (curLink != NULL) {
      nextBuaID = m_map->getRegionID(curLink->m_item, 
                                     ItemTypes::builtUpAreaItem);
   }

   // Search for landmarks for every connection (route-item)
   while (curLink != NULL) {

      mc2dbg8 << "prev(" << prevLink->m_item->getID() << ") "
           << StringTable::getString(
                 ItemTypes::getItemTypeSC(prevLink->m_item->getItemType()),
                 StringTable::ENGLISH)
           << " " << prevLink->m_dist
           << ", cur(" << curLink->m_item->getID() << ") "
           << StringTable::getString(
                 ItemTypes::getItemTypeSC(curLink->m_item->getItemType()),
                 StringTable::ENGLISH)
           << " " << curLink->m_dist
           << ", turn="
           << StringTable::getString(curLink->m_turnDescription,
              StringTable::languageCode(m_preferedLanguages.getElementAt(0)))
           << endl;

      // If not the first part of a mulitmap expand and if routing 
      // in an overviewMap, get the true mapID (underviewMap) of the 
      // route-item, in order to check if we have a new country.
      if ((overviewMap != NULL) && (prevCountryID != MAX_UINT32)) {
         IDPair_t fullID = overviewMap->lookup(curLink->m_item->getID());
         if (fullID.getMapID() != MAX_UINT32) {
            uint32 trueMapID = fullID.getMapID();
            if ((lastTrueMapID == MAX_UINT32) ||
                (lastTrueMapID != trueMapID) ) {
               // entering new underviewMap, check the country
               mc2dbg4 << "addLandmarks: Entering new underviewMap: "
                       << trueMapID << " (last " << lastTrueMapID 
                       << ")" << endl;
               map<uint32,uint32>::const_iterator findIt = 
                             mapIDToCountryID.find(trueMapID);
               if (findIt != mapIDToCountryID.end()) {
                  // found the country of the underviewMap
                  uint32 countryID = 
                     regionIDToCountryCode( findIt->second );
                  if (countryID != prevCountryID) {
                     // the underviewMap belongs to a new country
                     addCountryLM = true;
                     newCountry = StringTable::countryCode(countryID);
                     mc2dbg1 << "addLandmarks: Entering new country: "
                             << countryID << " (leaving " 
                             << prevCountryID << ")" << endl;
                     // update prevCountryID, to be correct when entering
                     // the next underviewMap
                     prevCountryID = countryID;
                  }
               } else {
                  mc2log << error << "Did not find country for trueMapID "
                         << trueMapID << " in the mapIDToCountryID table." 
                         << endl;
               }
               lastTrueMapID = trueMapID;
            }
            
         }
      }

      // First look if there are any landmarks in the landmark table
      uint32 fromNode = prevLink->m_nodeID;
      uint32 toNode = curLink->m_nodeID;
      uint64 key = (uint64(fromNode) << 32) | uint64(toNode);

      for (LI it = lmTable.lower_bound(key); 
           it != lmTable.upper_bound(key); it++) {
         if ( ((*it).first == key) &&  // not necessary? should be key
              (allBuas.linearSearch((*it).second.itemID) == MAX_UINT32) &&
              !((static_cast<StreetSegmentItem*>(prevLink->m_item)->
                  isControlledAccess()) && ((*it).second.importance == 4)) ) {
            // add the landmark to curLink
            // not if importance=4 and routing from ssi with controlled access
            
            // get the name of the landmark
            char name[50];
            if ((*it).second.type == ItemTypes::railwayLM) {
               strcpy(name, StringTable::getString(StringTable::RAILWAYITEM,
                            StringTable::languageCode(
                               m_preferedLanguages.getElementAt(0))));
            } else if (StringUtility::strcasecmp(
               m_map->getFirstItemName((*it).second.itemID), "missing") == 0) {
               strcpy(name, "");
            } else {
               const char* tmpname = m_map->getItemName(
                   (*it).second.itemID,
                   LangTypes::language_t(m_preferedLanguages.getElementAt(0)),
                   ItemTypes::invalidName);
               if (tmpname != NULL)
                  strcpy(name, tmpname);
               else 
                  strcpy(name, m_map->getFirstItemName((*it).second.itemID));
            }
            
            // get the dist of the landmark
            int32 dist = curLink->m_dist;
            if (((*it).second.location == ItemTypes::pass) &&
                ((*it).second.type == ItemTypes::builtUpAreaLM)) {
               dist = dist / 2;
            }
            
            mc2dbg1 << "Landmark: itemID=" << (*it).second.itemID 
                    << " " << name 
                    << " (lmTable: imp=" << int((*it).second.importance)
                    << " loc=" << int((*it).second.location)
                    << " side=" << int((*it).second.side) << ")" << endl;
            
            LandmarkLink* newLM = 
               new LandmarkLink((*it).second, dist);
            newLM->setLMName(name);
            newLM->into(curLink->m_landmarks);
            if ( (*it).second.type == ItemTypes::builtUpAreaLM ) {
               allBuas.addLast((*it).second.itemID);
            }
         }
      }

      // Note if the route goes through a bua.
      // For !ahead-turns or if the itemnames of prevLink and curLink differ: 
      //  - if the buaID not already exists in any of the links, 
      //    add it to curLink with location=into/after, side=undefined_side.
      buaID = nextBuaID; // from latest visit in the while-loop..
      nextBuaID = m_map->getRegionID(
            curLink->m_item, ItemTypes::builtUpAreaItem);

      if (buaID != MAX_UINT32) {
         if (buaID == oldBuaID) {
            importance = oldImportance;
            buaUsed = oldBuaUsed;
         } else {
            importance = static_cast<BuiltUpAreaItem*>
                        (m_map->itemLookup(buaID))->getBuiltUpAreaType();
            buaUsed = allBuas.linearSearch(buaID) != MAX_UINT32;
         } 
      }

      // use controlled access and ramp to decide
      // when not to add lm of importance=4.
      bool prevCA = static_cast<StreetSegmentItem*>
         (prevLink->m_item)->isControlledAccess();
      bool curCA = static_cast<StreetSegmentItem*>
         (curLink->m_item)->isControlledAccess();
      bool prevRamp = static_cast<StreetSegmentItem*>
         (prevLink->m_item)->isRamp();

      // calculate/update the buaDist
      if ((buaID != MAX_UINT32) && (buaID != oldBuaID)) {
         //start counting
         buaDist = 0;
        
         if (!buaUsed) {
            // the first segment with location=buaID can be located 
            // outside the bua, then that segment should be skipped when
            // counting the buadist
            GfxData* buaGfx = m_map->itemLookup(buaID)->getGfxData();
            GfxData* ssiGfx = prevLink->m_item->getGfxData();
            int32 checklat, checklon;
            MC2_ASSERT(ssiGfx->getNbrPolygons() == 1);
            if (ssiGfx->getNbrCoordinates(0) == 2) {
               checklat = (ssiGfx->getLat(0,0) + ssiGfx->getLat(0,1)) / 2;
               checklon = (ssiGfx->getLon(0,0) + ssiGfx->getLon(0,1)) / 2;
            } else {
               uint32 checkcoord = (ssiGfx->getNbrCoordinates(0) + 1) / 2 ;
               mc2dbg8 << "checkcoord " << checkcoord << " of "
                       << ssiGfx->getNbrCoordinates(0) << endl;
               checklat = ssiGfx->getLat(0,checkcoord -1); // numbering starts
               checklon = ssiGfx->getLon(0,checkcoord -1); // with 0..
            }
            if (buaGfx == NULL || !buaGfx->insidePolygon(checklat,checklon)) {
               skipFirstLinkInBuaDist = true;
               mc2dbg4 << "to skip first link for buaDist"
                       << " for " << buaID << ":"
                       << m_map->getFirstItemName(buaID) << endl;
            }
         }
         
      } else if ((buaID != MAX_UINT32) && (buaID == oldBuaID)) {
         // we are driving through a bua, add the distance of prevLink
         // NOT if prevLink is the last link in the bua (buaID != nextBuaID)

         skipLast = false;
         if (buaID != nextBuaID) {
            GfxData* buaGfx = m_map->itemLookup(buaID)->getGfxData();
            GfxData* ssiGfx = prevLink->m_item->getGfxData();
            int32 checklat, checklon;
            if (ssiGfx->getNbrCoordinates(0) == 2) {
               checklat = (ssiGfx->getLat(0,0) + ssiGfx->getLat(0,1)) / 2;
               checklon = (ssiGfx->getLon(0,0) + ssiGfx->getLon(0,1)) / 2;
            } else {
               uint32 checkcoord = (ssiGfx->getNbrCoordinates(0) + 1) / 2 ;
               mc2dbg8 << "checkcoord " << checkcoord << " of "
                       << ssiGfx->getNbrCoordinates(0) << endl;
               checklat = ssiGfx->getLat(0,checkcoord -1); // numbering starts
               checklon = ssiGfx->getLon(0,checkcoord -1); // with 0..
            }
            if (buaGfx == NULL || !buaGfx->insidePolygon(checklat,checklon)) {
               skipLast = true;
               mc2dbg4 << "to skip last link for buaDist"
                       << " for " << buaID << ":"
                       << m_map->getFirstItemName(buaID) << endl;
            }
         }
         
         if (!((prevCA || curCA || prevRamp) && (importance == 4))){
            // no bua is added as past etc, don't update the buadist
            if (!skipFirstLinkInBuaDist ) {
               buaDist += prevLink->m_dist;
               if (skipLast) {
                  // to make dist for LM = 0, add also curLink->m_dist
                  buaDist += curLink->m_dist;
               }
            } else {
               skipFirstLinkInBuaDist = false;
            }
         }
      }

      
      mc2dbg8 << "buaid=" << buaID << " oldBuaID=" << oldBuaID
              << " (cur=" << nextBuaID << ")" << endl << "  \""
              << StringTable::getString(curLink->m_turnDescription,
               StringTable::languageCode(m_preferedLanguages.getElementAt(0)))
              << "\" buadist=" << buaDist << endl;

      if ((buaID != MAX_UINT32) && (!buaUsed) ) {
         mc2dbg8 << " prev=" << m_map->getFirstItemName(prevLink->m_item)
                 << "(" << int(prevLink->m_item->getNbrNames()) << ")"
                 << " cur=" << m_map->getFirstItemName(curLink->m_item) 
                 << "(" << int(curLink->m_item->getNbrNames()) << ")" << endl;
         
         // If here is a turn, or if the names differ, add the bua
         // as a landmark with location into/after
         if (( (curLink->suc() != NULL) &&
               (
                (curLink->m_turnDescription != StringTable::AHEAD_TURN) &&
                (curLink->m_turnDescription != StringTable::FOLLOWROAD_TURN)
                ) ) || (
              ((curLink->m_turnDescription == StringTable::AHEAD_TURN) ||
               (curLink->m_turnDescription == StringTable::FOLLOWROAD_TURN))
              &&
              (prevLink->m_item->getNbrNames() > 0) &&
              (curLink->m_item->getNbrNames() > 0) &&
              ((StringUtility::strcmp(
                   m_map->getItemName(prevLink->m_item,
                   LangTypes::language_t(m_preferedLanguages.getElementAt(0)),
                                      ItemTypes::invalidName),
                   m_map->getItemName(curLink->m_item,
                   LangTypes::language_t(m_preferedLanguages.getElementAt(0)),
                                      ItemTypes::invalidName)) != 0)
               ||
               (prevLink->m_item->getNbrNames() != 
                curLink->m_item->getNbrNames()))
             )) {
         
            if (item_cast<StreetSegmentItem*>(prevLink->m_item) == NULL) {
               // we are leaving e.g. a ferry 
               // (the last ferryitem has the bua as location)
               
               if ((item_cast<FerryItem*>(prevLink->m_item) != NULL) 
                     &&
                   (item_cast<StreetSegmentItem*>(curLink->m_item) != NULL)
                     &&
                   (curLink->m_turnDescription == 
                    StringTable::EXIT_FERRY_TURN)) {
                  mc2dbg4 << "EXIT FERRY" << endl;

                  ItemTypes::lmdescription_t desc;
                  desc.itemID = buaID;
                  desc.side = SearchTypes::undefined_side;
                  desc.importance = importance;
                  desc.location = ItemTypes::arrive;
                  LandmarkLink* newLM = 
                     new LandmarkLink(desc, curLink->m_dist);
                     
                  if (addCountryLM && 
                      (strcasecmp(getCountryAndBuaName(newCountry),
                                  "unknown") != 0)) {
                     // we have both new country and bua as arrive
                     newLM->m_lmdesc.type = ItemTypes::countryAndBuiltUpAreaLM;
                     const char* name = 
                        getCountryAndBuaName(newCountry, buaID);
                     newLM->setLMName(name);

                     newLM->into(curLink->m_landmarks);
                     addCountryLM = false;
                     mc2dbg1 << "Landmark: itemID=" << buaID << " "
                             << name << " (bualocation, arrive imp="
                             << importance << ")" << endl;
                  } else {
                     // add just the bua as arrive
                     newLM->m_lmdesc.type = ItemTypes::builtUpAreaLM;
                     const char* name = m_map->getItemName(
                       buaID, LangTypes::language_t(
                          m_preferedLanguages.getElementAt(0)),
                       ItemTypes::invalidName);
                     newLM->setLMName(name);

                     newLM->into(curLink->m_landmarks);
                     mc2dbg1 << "Landmark: itemID=" << buaID << " "
                             << name << " (bualocation, arrive imp="
                             << importance << ")" << endl;
                  }
               } else {
                  mc2dbg1 << "Won't add " << m_map->getFirstItemName(buaID)
                          << " as landmark" << endl;
               }
               allBuas.addLast(buaID);
               buaUsed = true;
               
            } else if ( !((prevCA || curCA || prevRamp) && 
                          (importance == 4)) ) {
               // add lm into/after-bua (not if imp=4 and controlled access)
               ItemTypes::lmdescription_t desc;
               desc.itemID = buaID;
               desc.importance = importance;
               desc.side = SearchTypes::undefined_side;
               desc.type = ItemTypes::builtUpAreaLM;
               const char* name = m_map->getItemName( buaID,
                 LangTypes::language_t(m_preferedLanguages.getElementAt(0)),
                 ItemTypes::invalidName);
               const char* locstr;
               int32 dist;
               if ((nextBuaID == MAX_UINT32) && 
                   (! ( ( curLink->m_item->getItemType() == 
                          ItemTypes::streetSegmentItem ) && 
                        (static_cast<StreetSegmentItem*> (curLink->m_item))
                         ->isRoundabout() ) ) ) {
                  if ((curLink->m_turnDescription == 
                              StringTable::AHEAD_TURN) ||
                      (curLink->m_turnDescription == 
                              StringTable::FOLLOWROAD_TURN)) {
                     // pass
                     desc.location = ItemTypes::pass;
                     locstr = "pass";
                     if (skipLast) {
                        // curLink->m_dist is already included in the buaDist
                        dist = - buaDist;
                     } else {
                        // the buaborder is located dist meters before the 
                        // aheadturn
                        dist = -(curLink->m_dist + buaDist);
                     }
                  } else {
                     // after
                     desc.location = ItemTypes::after;
                     locstr = "after";
                     dist = curLink->m_dist;
                  }
               } else {
                  desc.location = ItemTypes::into;
                  locstr = "into";
                  if (skipFirstLinkInBuaDist) {
                     // the border is located in front of curLink,
                     // = coincides with the turn (the turn is ON the border)
                     dist = curLink->m_dist - buaDist;
                  } else if (skipLast) {
                     // curLink->m_dist is already included in the buaDist
                     dist = - buaDist;
                  } else {
                     // the buaborder is located dist meters before the turn
                     dist = -(curLink->m_dist + buaDist);
                  }
               }
               
               mc2dbg8 << " buadist " << buaDist << " dist " << dist << endl;
               mc2dbg1 << "Landmark: itemID=" << buaID << " "
                       << name << " (bualocation," 
                       << locstr << " imp=" << importance << ")" << endl;

               LandmarkLink* newLM = new LandmarkLink(desc, dist);
               newLM->setLMName(name);
               newLM->into(curLink->m_landmarks);
               allBuas.addLast(buaID);
               buaUsed = true;
               oldBuaID = MAX_UINT32;
            }
         } else if ( !((buaID != oldBuaID) && 
                       (oldBuaID != MAX_UINT32) &&
                       (allBuas.linearSearch(oldBuaID) == MAX_UINT32)) ){
            //update oldBuaID, if it is not another bua added to allBuas
            //(Karlskrona-Lyckeby-karlskrona...)
            oldBuaID = buaID;
            oldImportance = importance;
            oldBuaUsed = buaUsed;
         }
      }
      
      // If there is no turn in the bua (only ahead-turns) add it if buaID
      // not already exists, add it with location=pass, side=undefined_side 
      // to prevLink (The case is when buaID turns to maxuint32 or to 
      // another buaID and we have an unused oldBuaID)
      if (((buaID == MAX_UINT32) || (buaID != oldBuaID)) && 
          (oldBuaID != MAX_UINT32) && (!oldBuaUsed)) {

         // add lm pass-bua (not if importance=4 and controlled access)
         if ( !((prevCA || prevRamp ) && (oldImportance == 4)) ) {
            ItemTypes::lmdescription_t desc;
            desc.itemID = oldBuaID;
            desc.importance = oldImportance;
            desc.side = SearchTypes::undefined_side;
            desc.location = ItemTypes::pass;
            desc.type = ItemTypes::builtUpAreaLM;
            const char* name = m_map->getItemName( oldBuaID,
              LangTypes::language_t(m_preferedLanguages.getElementAt(0)),
              ItemTypes::invalidName);
            
            mc2dbg1 << "Landmark: itemID=" << oldBuaID << " " << name
                    << " (bualocation,pass imp=" << oldImportance 
                    << ")" << endl;
            int32 dist = -buaDist;
            mc2dbg8 << " dist=" << dist << "(bD=" << buaDist << ", pL="
                    << prevLink->m_dist << ")" << endl;

            LandmarkLink* newLM = new LandmarkLink(desc, dist);
            newLM->setLMName(name);
            newLM->into(prevLink->m_landmarks);
            allBuas.addLast(oldBuaID);
            oldBuaUsed  = true;
         }
         if ((buaID != MAX_UINT32) && (!buaUsed)) {
            // The last for Karlskrona-Lyckeby-Karlskrona
            oldBuaID = buaID;
            oldImportance = importance;
            oldBuaUsed = buaUsed;
         } else {
            oldBuaID = MAX_UINT32;
         }
      }

      // Check if to add just a country.
      if ( addCountryLM && 
           (strcasecmp(getCountryAndBuaName(newCountry), "unknown") != 0) ) {
         
         ItemTypes::lmdescription_t desc;
         desc.itemID = MAX_UINT32;
         desc.side = SearchTypes::undefined_side;
         desc.importance = 0;
         desc.type = ItemTypes::countryLM;
         
         LandmarkLink* newLM = new LandmarkLink(desc, prevLink->m_dist);
                  
         if ((item_cast<FerryItem*>(prevLink->m_item) != NULL) && 
             (curLink->m_turnDescription == StringTable::EXIT_FERRY_TURN)) {
            mc2dbg4 << "FERRY arrive to COUNTRY" << endl;
            newLM->m_lmdesc.location = ItemTypes::arrive;
            addCountryLM = false;
            
         } else if (item_cast<StreetSegmentItem*>
                                (prevLink->m_item) != NULL) {
            mc2dbg4 << "DRIVE into COUNTRY" << endl;
            // Make sure that the landmark is not added to any virtual ssi 
            // on the map boarder (also corrects for virtual ssi for ferrys)
            if (curLink->m_dist > 0) {
               newLM->m_lmdesc.location = ItemTypes::into;
               // you enter the country before curLink
               newLM->m_lmDist = -curLink->m_dist;
               addCountryLM = false;
            } else {
               mc2dbg4 << "  virtual item, don't add countryLM now" << endl;
            }
         }
         if (!addCountryLM) {
            const char* name = getCountryAndBuaName(newCountry);
            newLM->setLMName(name);
            newLM->into(curLink->m_landmarks);
            mc2dbg1 << "Landmark: country " << name << endl;
         } else {
            delete newLM;
         }
      }
      
      
      prevLink = curLink;
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());

      // If the route ends in a bua, without any turn in the bua, add.
      if ((curLink == NULL) && (buaID != MAX_UINT32) && 
          (!buaUsed) && ((buaDist > 0) || (!skipFirstLinkInBuaDist))) {
         
         if ( !((prevCA || curCA) && (importance == 4)) ) {
            ItemTypes::lmdescription_t desc;
            desc.itemID = buaID;
            desc.importance = importance;
            desc.side = SearchTypes::undefined_side;
            desc.location = ItemTypes::into;
            desc.type = ItemTypes::builtUpAreaLM;
            const char* name = m_map->getItemName( buaID,
              LangTypes::language_t(m_preferedLanguages.getElementAt(0)),
              ItemTypes::invalidName);
            // How far have we routed since passing the buaboarder until 
            // we reached the last routeitem. The length of this last 
            // segment is considered in the updateStartAndEnd-function
            int32 dist = 0;
            if (buaDist > 0) {
               // The bua boarder is located buaDist meters before
               // reaching this last segment.
               dist = -(buaDist);
            }
            
            mc2dbg4 << " prevlink=" << prevLink->m_dist << " buaDist=" 
                    << buaDist << " ->dist=" << dist << endl;
            mc2dbg1 << "Landmark: itemID=" << buaID << " " << name
                    << " (bualocation,into(end)"
                    << " imp=" << importance << ")" << endl;

            LandmarkLink* newLM = new LandmarkLink(desc, dist);
            newLM->setLMName(name);
            newLM->into(prevLink->m_landmarks);
            allBuas.addLast(buaID);
            buaUsed = true;
            oldBuaID = MAX_UINT32;
         }
      }
   }

   return (true);
}

bool
ExpandRouteProcessor::storeDataFromConnections( bool uTurn,
                                                byte routeType )
{
   // This will not work if there are any links that have
   // m_item == NULL; And the m_state in the links must be set
   // before calling this method.
   
   // Set "StringTable::DRIVE_START" or "DRIVE_START_WITH_UTURN to the 
   // first link. Will also initiate prevLink and curLink
   ExpandRouteLink* prevLink = 
      static_cast<ExpandRouteLink*>(m_routeList->first());
   ExpandRouteLink* curLink = 
      static_cast<ExpandRouteLink*>(prevLink->suc());

   if(uTurn)
      prevLink->m_turnDescription = StringTable::DRIVE_START_WITH_UTURN;
   else
      prevLink->m_turnDescription = StringTable::DRIVE_START;

   prevLink->setTimes(0, 0);
 
   // Update the rest of the links
   while (curLink != NULL) {
      Node* node = m_map->nodeLookup(curLink->m_nodeID);
      int j=0;
      Connection* curCon = node->getEntryConnection(j);
      while ( (curCon != NULL) &&
              (curCon->getConnectFromNode() != prevLink->m_nodeID )) {
         // Doesn't matter if a connection is multi, ignored anyway.
         curCon = node->getEntryConnection(++j);
      }
      
      if (curCon != NULL) {
         curLink->m_turnDescription = curCon->getTurnStringCode();
         curLink->m_exitCount = curCon->getExitCount();
         
         
         
         float64 length, tmpTime, tmpSST;
         m_map->getConnectionData(curCon, 
                                  m_map->nodeLookup(prevLink->m_nodeID),
                                  tmpTime, tmpSST, length);
         tmpTime += curLink->m_time;
         tmpTime =  tmpTime * ADD_TIME_FACTOR;
         tmpSST  += curLink->getStandStillTime();
         curLink->addTimes(uint32(tmpTime + 0.5), uint32(tmpSST + 0.5));
         
         // Set signposts
         setSignposts( curCon, curLink );
         // Set crossingKind
         curLink->setCrossingKind( curCon->getCrossingKind() );
      } else {
         curLink->m_turnDescription = StringTable::PARK_CAR;
         curLink->setTimes(0, 0);         
      }      
      // Get and set the lanes for this Node
      setLanes( curLink );

      prevLink = curLink;
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   }

   // Update the last node (prevLink)
   
   // Get all nodes that are connected to either of the two nodes of
   // the last segment. Is used to get all connections that are traversing
   // the last segment.
   Vector nodeIDs(8,8);
   m_map->getConnectToNodeIDs(prevLink->m_nodeID, &nodeIDs);
   m_map->getConnectToNodeIDs(prevLink->m_nodeID ^ 0x80000000, 
                              &nodeIDs);
   
   if (nodeIDs.getSize() > 0) {
      mc2dbg8 << " =================" << endl;
      DEBUG8(nodeIDs.dump(););
      mc2dbg8 << " =================" << endl;
      Connection* con = NULL;
      uint32 curNodeIndex = 0;
      // Minimum standstill time
      float64 minSST = MAX_FLOAT64;
      float64 time = 0;
      
      // Find a connection that has the smallest standstill time and
      // use that for setting the times.
      // This is done since the RouteModule includes standstill time
      // of the last segment (but really it shouldn't be), and we want
      // to get times from the route expanding that match the costs
      // that the RouteModule uses.
      while ( curNodeIndex < nodeIDs.getSize() ) {
         Node* node = (m_map->nodeLookup(nodeIDs.getElementAt(curNodeIndex)));
         con = node->getEntryConnection(0);
         int i=0;
         while ( (con != NULL) && 
                 (REMOVE_UINT32_MSB(con->getConnectFromNode()) != 
                  REMOVE_UINT32_MSB(prevLink->m_nodeID))) {
            mc2dbg8 << "Checking conneticion from " 
                    << con->getConnectFromNode()
                    << " to " << node->getNodeID() << endl;
            con = node->getEntryConnection(i++);
         }
         curNodeIndex++;
      
         if (con != NULL) {
            float64 length, tmpTime, tmpSST;
            m_map->getConnectionData(
                  con, 
                  m_map->nodeLookup(nodeIDs.getElementAt(curNodeIndex-1)),
                  tmpTime, tmpSST, length);
            
            // Store the times if the standstill time is 
            // the smallest so far
            DEBUG8(cout << "Trying..." << endl);
            if (minSST > tmpSST) {
               minSST = tmpSST;
               time = tmpTime;
               mc2dbg8<< "Smallest sst so far = " << minSST << endl;
            }
         } 
         
      } // while

      // Set the times.
      mc2dbg4 << "Setting time on last segment to " 
              << uint32(time + 0.5) << ", sst = " 
              << uint32(minSST + 0.5) << endl;
      prevLink->addTimes( uint32(time + 0.5), uint32(minSST + 0.5) );
   
   } else {
      // Single segment, not connected to any other 
      // segments in the map. Set the costs to zero since we don't now
      // the real cost.
      mc2log << warn << "Setting segment time to zero, no connections" << endl;
      prevLink->setTimes( 0, 0);
   }
   
   return (true);
}

bool
ExpandRouteProcessor::setSignposts(Connection* conn, ExpandRouteLink* link)
{
   // Get and set new signposts
   const GenericMap::ConnectionSignPostMapArray* signs = 
      m_map->getConnectionSigns( m_map->getConnectionID( *conn ) );
   if ( signs != NULL ) {
      // Add the signs, they are already sorted
      for ( uint32 i = 0 ; i < signs->size() ; ++i ) {
         ExpandStringSignPost sign(
            m_map->getName( (*signs)[ i ].getText() ),
            (*signs)[ i ].getColors().textColor,
            (*signs)[ i ].getColors().frontColor,
            (*signs)[ i ].getColors().backColor,
            0/*priority*/, 
            (*signs)[ i ].getExit() ? ExpandStringSignPost::exit : 
            ExpandStringSignPost::undefined,
            (*signs)[ i ].getDist(),
            false/*ambiguous*/ );
         link->addSignPost( sign );
         mc2dbg4 << "setSignposts added sign " << sign.getText() 
                 << " to " << m_map->getConnectionID( *conn ) << endl;
      }
      mc2dbg8 << "All signs ";
      for ( uint32 i = 0 ; i < signs->size() ; ++i ) {
         mc2dbg8 << "\"" << m_map->getName( (*signs)[ i ].getText() ) 
                 << "\" ";
      }
      mc2dbg8 << endl;
      return true;
   } else {
      return false;
   }
}

bool
ExpandRouteProcessor::setLanes( ExpandRouteLink* link ) {
   ExpandRouteLink* suc = static_cast<ExpandRouteLink*>( link->suc() );

   // Connection data
   mc2dbg2 << "setLanes for link->m_nodeID " << link->m_nodeID << endl;
   Connection* curCon = NULL;
   if ( suc != NULL ) {
      Node* nextNode = m_map->nodeLookup( suc->m_nodeID );
      curCon = nextNode->getEntryConnectionFrom( link->m_nodeID );
      mc2dbg2 << " nextNode->getNodeId() " << nextNode->getNodeID() << endl;
   }
   
   uint32 conIdx = 0;
   if ( curCon != NULL ) {
      conIdx = m_map->getLaneConnectionIdx( *curCon );
      mc2dbg2 << "conIdx " << MC2HEX( conIdx ) << endl;
   }

   // Node data
   const GenericMap::NodeLaneMapArray* laneData = m_map->getNodeLanes(
      link->m_nodeID );

   if ( laneData != NULL ) {
      typedef ExpandStringLane EL;
      ExpandStringLanes l;
      l.setDist( 0 ); // Whole link
      for ( byte i = 0 ; i < laneData->size() ; ++i ) {
         bool preferredLane = false;
         if ( (conIdx & (1<<i)) != 0 ) {
            preferredLane = true;
         }
         if ( (*laneData)[ i ].getDirections() & 
              ExpandStringLane::opposite_direction ) {
            // No oppisite direction lanes
            continue;
         }
         l.addLane( ExpandStringLane( 
                       (*laneData)[ i ].getDirections(),
                       preferredLane, false/*notCarLane in dir*/) );
      }
      mc2dbg2 << "lane data for node: " << link->m_nodeID << " is " 
              << l  << " sc " << StringTable::getString( 
                 link->m_turnDescription, StringTable::ENGLISH ) << endl;
      if ( suc != NULL ) {
         suc->addLane( l );
      }
   } else {
      mc2dbg4 << "No lane data for node: " << link->m_nodeID 
              << " sc " << StringTable::getString( 
                 link->m_turnDescription, StringTable::ENGLISH ) << endl;
      mc2dbg2 << "link->hasNonStopOfLanesLane() " 
              << link->hasNonStopOfLanesLane() << endl;
      // Check if end of lanes lane...
      if ( link->hasNonStopOfLanesLane() ) {
         // Then add StopOfLanes
         if ( suc != NULL ) {
            suc->addLane( ExpandStringLanes( true/*stopOfLanes*/ ) );
         }
      }
   }
   mc2dbg2 << "  " << endl;

   return true;
}

bool
ExpandRouteProcessor::findBadGfxCoords()
{
   if(m_routeList->cardinal() < 2)
      return false;
   bool badCoordFound = false;
   ExpandRouteLink* thisLink = static_cast<ExpandRouteLink*>
      (m_routeList->first());
   ExpandRouteLink* nextLink = static_cast<ExpandRouteLink*>
      (thisLink->suc());
   while(nextLink != NULL){
      if((nextLink->m_turnDescription == StringTable::ENTER_ROUNDABOUT_TURN) ||
         (nextLink->m_turnDescription == StringTable::EXIT_ROUNDABOUT_TURN)){
         float64 angle = getConnectionAngle(thisLink, nextLink);
         if((angle< -M_PI_4) ||(angle > M_PI_4) ){
            badCoordFound = true;
            if(nextLink->m_turnDescription ==
               StringTable::ENTER_ROUNDABOUT_TURN){
               m_badGfxList->setRemoveLastCoordinate(
                  thisLink->m_nodeID, thisLink->m_item->getGfxData());
            }
            m_badGfxList->setRemoveFirstCoordinate(
               nextLink->m_nodeID, nextLink->m_item->getGfxData());
         }
      }
      thisLink = nextLink;
      nextLink = static_cast<ExpandRouteLink*>(thisLink->suc());
   }
   mc2dbg4 << "Bad coord found = " <<badCoordFound << endl;
   return badCoordFound;
}

float64
ExpandRouteProcessor::getConnectionAngle(ExpandRouteLink* first,
                                         ExpandRouteLink* second)
{
   int32 lat1, lon1, lat2, lon2;
   GfxData* gfx = first->m_item->getGfxData();
   MC2_ASSERT(gfx->getNbrPolygons() == 1);
   if(GET_UINT32_MSB(first->m_nodeID) == 0){
      uint32 points = gfx->getNbrCoordinates(0);
      lat1 = gfx->getLat(0,points-2);
      lon1 = gfx->getLon(0,points-2);
      lat2 = gfx->getLat(0,points-1);
      lon2 = gfx->getLon(0,points-1);
   }else{
      lat1 = gfx->getLat(0,1);
      lon1 = gfx->getLon(0,1);
      lat2 = gfx->getLat(0,0);
      lon2 = gfx->getLon(0,0);      
   }
   float64 angle1 = GfxUtility:: getAngleFromNorth(lat1,lon1,lat2,lon2);
   
   gfx = second->m_item->getGfxData();
   MC2_ASSERT(gfx->getNbrPolygons() == 1);
   if(GET_UINT32_MSB(second->m_nodeID) == 0){
      lat1 = gfx->getLat(0,0);
      lon1 = gfx->getLon(0,0);
      lat2 = gfx->getLat(0,1);
      lon2 = gfx->getLon(0,1);
   }else{
      uint32 points = gfx->getNbrCoordinates(0);
      lat1 = gfx->getLat(0,points-1);
      lon1 = gfx->getLon(0,points-1);
      lat2 = gfx->getLat(0,points-2);
      lon2 = gfx->getLon(0,points-2);      
   }
   float64 angle2 = GfxUtility::getAngleFromNorth(lat1,lon1,lat2,lon2);
   mc2dbg4 << "Angle1 : " << angle1 << ", Angle2 : " << angle2 << endl;
   float64 angle = angle2-angle1;
   mc2dbg4 << "Angle : " << angle << endl;
   if(angle > M_PI)
      angle -= 2*M_PI;
   else if(angle < -M_PI)
      angle += 2*M_PI;
   return angle;
}


bool
ExpandRouteProcessor::saveRouteGfxIntoPacket(ExpandRouteReplyPacket* reply,
                                             const ExpandRouteRequestPacket* req,
                                             const list<uint32>& nodeIDs,
                                             uint32 nbrItems)
{
   OverviewMap* overviewMap = dynamic_cast<OverviewMap*> (m_map);
   uint32 mapID = m_map->getMapID();
   if ((overviewMap == NULL) && (MapBits::isOverviewMap(mapID))) {
      mc2log << error << here << " Overview map id is not a "
             << "OverviewMap." << endl;
      return (false);
    }
   
   // Respond with the route as gfxData (in all zoomlevels)
   mc2dbg4 << "Generating gfx-route" << endl;
   uint32 nbrZoomLevels = 1;
   byte reqType = req->getType();
   uint32 nbrCoordPos = 0;
   for (uint32 curZoom=0; curZoom < nbrZoomLevels; curZoom++) {      
      bool routeOnSingleSegment = false;

      uint32 x;
      if ( GET_TRANSPORTATION_STATE(nodeIDs.front()) != 0 ) {
         x = 1;
      } else {
         x = 0;
      }


      if ( (nbrItems == (2+x) ) && 
           (!req->ignoreStartOffset()) && 
           (!req->ignoreEndOffset()) ) {
         
         list<uint32>::const_iterator it = nodeIDs.begin();
         for ( uint32 i = 0; i < x; ++i ) 
            ++it;
         list<uint32>::const_iterator nextIt = it;
         ++nextIt;
         if (*it == *nextIt ) {
            routeOnSingleSegment = true;
            nbrItems--;
         }
      }
            
      bool start = true;
      list<uint32>::const_iterator it = nodeIDs.begin();
      for (uint32 i=0; i<nbrItems; i++) {
         uint32 nodeID = *it;

         // Find the real map id and node id (exchange ids if it is the
         // overview map)
         uint32 mapIDtoWrite = mapID;
         uint32 nodeIDtoWrite = nodeID;
         if( overviewMap != NULL ){
            mc2dbg8 << "Exchanging " << nodeID;
            overviewMap->lookupNodeID(nodeID, mapIDtoWrite, 
                                             nodeIDtoWrite );
            mc2dbg8 << " for " << mapIDtoWrite << ":" 
                    << nodeIDtoWrite << endl;
         } else {
            // OK. overViewMap should be null here or we would
            // have exited already.
         }
         mc2dbg4 << i << ". GfxRoute " << mapIDtoWrite << "." 
                 << nodeIDtoWrite << endl;
         
         
         // Do a lookup to see if it is a valid street item
         RouteableItem* curItem = Item::routeableItem
            (m_map->itemLookup(nodeID & 0x7fffffff));
        
         
         if (curItem != NULL ) {
            if ( reqType & ROUTE_TYPE_GFX_COORD ) {
               // One coordinate per item requested:
               mc2dbg4 << "ROUTE_TYPE_GFX_COORD " <<endl;
               int32 lat = 0;
               int32 lon = 0;
               uint32 idx = 0;
               GfxData* gfx = curItem->getGfxData();
               if ( gfx != NULL ) {
                  MC2_ASSERT(gfx->getNbrPolygons() == 1);
                  if ( (start) && (! req->ignoreStartOffset()) ) {
                     gfx->getCoordinate(req->getStartOffset(),
                                                     lat, lon);
                  } else {
                     if ( nodeID & 0x80000000 ) // Node 1
                        idx = gfx->getNbrCoordinates(0) - 1; // Last coord
                     lat = gfx->getLat(0,idx);
                     lon = gfx->getLon(0,idx);
                  }
                  mc2dbg4 << "lat :" << lat << " lon :" 
                          << lon << endl;
               }
               reply->setItemData(mapIDtoWrite,  
                                  nodeIDtoWrite & 0x7fffffff, lat, lon);
               
               
            } else if ( reqType & ROUTE_TYPE_GFX_TRISS ) {
               mc2dbg4 << "ROUTE_TYPE_GFX_TRISS " << endl;
               int32 lat = 0;
               int32 lon = 0;
               
               byte nodeNbr = 0;
               if ( nodeID & 0x80000000 ) {
                  nodeNbr = 1;
               }
               
               byte attributes = 0;
               

               uint32 nbrCoordPos = reply->setNavItemData(
                  mapIDtoWrite,
                  nodeIDtoWrite,
                  curItem->getNode(nodeNbr)->getSpeedLimit(),
                  attributes);
               
               GfxData* gfx = curItem->getGfxData();
               if ( gfx != NULL ) {
                  gfx->getCoordinate((uint16) (MAX_UINT16/2), lat, lon);
                  
                  reply->addCoordToNavItemData(lat, lon, nbrCoordPos);
                  
                  float64 angle = gfx->getAngle(MAX_UINT16/2);

                  if ( nodeNbr == 1 ) {
                     angle = float64( angle + 180 );
                  }

                  if ( angle >= (float64) 360 ) {
                     angle = (float64) (angle - 360);
                  }

                  
                  int n = 0;   // The quadrant of the angle
                  
                  if ( (angle >= 0) && (angle <= 90) ) {
                     n = 1;
                  } else if( (angle > 90) && (angle <= 180) ) {
                     n = 2;
                  } else if( (angle > 180) && (angle <= 270) ) {
                     n = 3;
                  } else if( angle > 270 ) {
                     n = 4;
                  }
                                    
                  float64 deltaY = 0, deltaX = 0;
                  float64 alfa;
                  
                  switch(n) {
                     case 1:
                     {
                        alfa = (float64) (angle / 360 * 2 * M_PI);
                        float64 h = (float64) 1000000;
                        deltaY = (float64) ( h * sin(alfa) );
                        deltaX = (float64) ( h * cos(alfa) );
                     }
                     break;
                     
                     case 2:
                     {
                        if( angle == 180 ) {
                           deltaY = 0;
                           deltaX = -1000000;
                        }
                        else {
                           alfa = (float64) (angle - 90);
                           alfa = (float64) (alfa / 360 * 2 * M_PI);
                           float64 h = (float64) 1000000;
                           deltaY = (float64) ( h * sin(alfa) );
                           deltaX = -(float64) ( h * cos(alfa) );
                        }
                     }
                     break;
                     
                     case 3:
                     {
                        alfa = (float64) (angle - 180);
                        alfa = (float64) (alfa / 360 * 2 * M_PI);
                        float64 h = (float64) 1000000;
                        deltaY = -(float64) ( h * sin(alfa) );
                        deltaX = -(float64) ( h * cos(alfa) );
                     }
                     break;
                     
                     case 4:
                     {
                        alfa = (float64) (angle - 270);
                        alfa = (float64) (alfa / 360 * 2 * M_PI);
                        float64 h = (float64) 1000000;
                        deltaY = -(float64) ( h * sin(alfa) );
                        deltaX = (float64) ( h * cos(alfa) );
                     }
                     break;
                  }
                                    
                  reply->addCoordToNavItemData(lat + (int32) deltaY,
                                               lon + (int32) deltaX,
                                               nbrCoordPos);
                  
                  
                  mc2dbg4 << "lat :" << lat << " lon :" 
                          << lon << " angle :" << angle << endl;
               }
               

            } else if ( (reqType & ROUTE_TYPE_NAVIGATOR) ||
                        (reqType & ROUTE_TYPE_ALL_COORDINATES))
            {
               // Navigator route type or all coordinates 
               // Means we should add all coordinates after filtering them
               // and also the speedlimits.
               mc2dbg4 << "Navigator route type or all coordinates " << endl;
               // Add ids and speedlimit.
               byte nodeNbr = 0;
               if ( nodeID & 0x80000000 ) {
                  nodeNbr = 1;
               }
               
               // Whether this road is a freeway/motorway or not.
               // attributes.
               StreetSegmentItem* ssi = 
                  item_cast<StreetSegmentItem*> (curItem);
               bool controlledAccess = false;
               bool ramp = false;
               bool driveOnRightSide = true;
               if (ssi != NULL) {
                  controlledAccess = ssi->isControlledAccess();
                  ramp = ssi->isRamp();
                  driveOnRightSide = getDriveOnRightSide( ssi->getID() );
               }
               byte attributes = 
                  ExpandRouteReplyPacket::createAttributes( 
                                                controlledAccess,
                                                ramp, driveOnRightSide );
               nbrCoordPos = 
                  reply->setNavItemData(
                               mapIDtoWrite, nodeIDtoWrite,
                               curItem->getNode(nodeNbr)->getSpeedLimit(),
                               attributes);
               mc2dbg4 << "Setting itemdata: mapID = " << mapIDtoWrite
                       << ", nodeID = " << nodeIDtoWrite
                       << ", speedlimit = " 
                       << curItem->getNode(nodeNbr)->getSpeedLimit() 
                       << ", freeway = "
                       << endl;
               
               GfxData* gfx = curItem->getGfxData();
               if ( gfx != NULL ) {

                  // Filter the GfxData
                  // Note that this isn't exactly the optimal
                  // solution of doing this! 
                  // #1. The stack should be filled in reversed
                  //     order so that we get the correct order when
                  //     adding the coordinates by popping the stack.
                  bool successfulFiltering = false;
#ifdef FILTERCOORDINATES
                  Stack* filteredIndices = new Stack(10);
                  const uint32 maxLatFilteringDist = 5; // 5 meters
                  if (gfx->openPolygonFilter(filteredIndices, 0, 
                                             maxLatFilteringDist, 
                                             MAX_UINT32)) {
                     successfulFiltering = true;
                     GfxData* unfilteredGfx = gfx;
                     gfx = new GfxDataFull(true);
                     // Add the filtered indices to the gfxdata.
                     uint32 size = filteredIndices->getStackSize();
                     for (uint32 i = 0; i < size; i++) {
                        uint32 idx = filteredIndices->pop();
                        gfx->addCoordinate(unfilteredGfx->getLat(0,idx),
                                           unfilteredGfx->getLon(0,idx),
                                           false,
                                           true); // Adding to the front
                                           
                     }
                  }
                  // Update length since getCoordinate() will use it
                  // later on!
                  gfx->updateLength();
                  delete filteredIndices;
#endif 
                  
                  // Calculate the start and end index of the gfxdata
                  // Note that we don't want to include the last coordinate
                  // since that otherwise will be duplicated.
                  int32 startIndex = 0;
                  int32 endIndex = gfx->getNbrCoordinates(0) - 1;
                  
                  if (nodeNbr == 1) {
                     startIndex = gfx->getNbrCoordinates(0) - 1;
                     endIndex = 0;
                  }
                  // Check if this is the first item and the first part of 
                  // the route. In that case, the coordinates should 
                  // start at the offset.
                  if ( (start) && (! req->ignoreStartOffset()) ) {
                     int32 lat, lon;
                     startIndex = gfx->getCoordinate(req->getStartOffset(),
                                                     lat, lon);
                     // Add the offset to the reply
                     reply->addCoordToNavItemData(lat, lon, nbrCoordPos);
                     mc2dbg4 <<"lat :" << lat << " lon :"
                             << lon << endl;
                     // If we should add the coordinates backwards, then
                     // the startIndex should be decreased (since that
                     // is the next coordinate after the offset)
                     if (nodeNbr == 1) {
                        startIndex--;
                        endIndex = 0;
                     }
                  // Check if it the end of the route.
                  }
                  if ((i == (nbrItems - 1)) &&
                             (! req->ignoreEndOffset())) {
                     int32 lat, lon;
                     endIndex = gfx->getCoordinate(req->getEndOffset(),
                                                   lat, lon);
                     if (nodeNbr == 1) {
                        // We want to include the last coordinate
                        // before the interpolated coordinate.
                        endIndex--;
                        // Update the startindex if we're not routing on
                        // a single segment 
                        if (! routeOnSingleSegment) {
                           startIndex = gfx->getNbrCoordinates(0) - 1;
                        }
                     }
                  // Reverse order of coordinates. 
                  // Update start and end index
                  }
                  int32 firstLat , firstLon, lastLat, lastLon;
                  if((m_alteredGfxData) && (nodeNbr == 0))
                  {
                     if(m_badGfxList->
                        removeFirstCoord(nodeID, firstLat, firstLon))
                        startIndex ++;
                     m_badGfxList->removeLastCoord(nodeID, lastLat, lastLon);
                  }
                  else if(m_alteredGfxData)
                  {
                     if(m_badGfxList->
                        removeFirstCoord(nodeID, firstLat, firstLon))
                        startIndex--;
                     m_badGfxList->removeLastCoord(nodeID, lastLat, lastLon);
                  }
                  mc2dbg4 << "NodeNbr == " << int(nodeNbr) << endl;
                  if(m_alteredGfxData && (firstLat != MAX_INT32) &&
                     (firstLon != MAX_INT32)){
                     reply->addCoordToNavItemData(firstLat,
                                               firstLon,
                                               nbrCoordPos);
                     mc2dbg4 << "flat:" << firstLat
                             << " flon:" << firstLon << endl;
                  }
                  if (nodeNbr == 0) {
                     for (int32 i = startIndex; 
                           i < endIndex; i++) {
                        reply->addCoordToNavItemData(gfx->getLat(0,i),
                                                  gfx->getLon(0,i),
                                                  nbrCoordPos);
                        mc2dbg4 << "lat :" << gfx->getLat(0,i)
                                << " lon :" << gfx->getLon(0,i) << endl;
                     }
                  } else {
                     // Add coordinates in opposite order
                     for (int32 i = startIndex; i > endIndex; i--) {
                        reply->addCoordToNavItemData(gfx->getLat(0,i),
                                                  gfx->getLon(0,i),
                                                  nbrCoordPos);
                        mc2dbg4 << "lat :" << gfx->getLat(0,i)
                                << " lon :" << gfx->getLon(0,i) << endl;
                     }
                  }
                  if(m_alteredGfxData && (lastLat != MAX_INT32) &&
                     (lastLon != MAX_INT32)){
                     reply->addCoordToNavItemData(lastLat,
                                                  lastLon,
                                               nbrCoordPos);
                     mc2dbg4 << "llat:" << lastLat
                             << " llon:" << lastLon << endl;
                  }

                  // If we filtered the gfxdata then gfx should be deleted
                  // since it was allocated here.
                  if (successfulFiltering) {
                     delete gfx;
                  }
               } // if gfx != NULL 
               
            } else {
               reply->setItemData(mapIDtoWrite, nodeIDtoWrite);
            }
            if (start)
               start = false;
         } else {
            // Don't include state changes in the beginning 
            // (ie. not really a state change)
            // And check the state of the node! It can be an
            // additional cost node.
            if (!start && GET_TRANSPORTATION_STATE(nodeID) ) {
               mc2dbg4 << "  state change" << endl;
               // State change, from driving to walking for instance. 
               if ( reqType & ROUTE_TYPE_GFX_COORD ) {
                  // No coordinates available since it is a state change,
                  // add MAX_INT32 to show this.
                  reply->setItemData(mapIDtoWrite, nodeIDtoWrite, 
                                     MAX_INT32, MAX_INT32);
               } else if ( (reqType & ROUTE_TYPE_NAVIGATOR) ||
                           (reqType & ROUTE_TYPE_ALL_COORDINATES) ||
                           (reqType & ROUTE_TYPE_GFX_TRISS) ) {
                  // Navigator!
                  // No coordinates available since it is a state change.
                  // Set speedlimit and attributes to 0.
                  reply->setNavItemData(mapIDtoWrite, nodeIDtoWrite, 0, 0);
               } else {
                  reply->setItemData(mapIDtoWrite, nodeIDtoWrite);
               }
            }
         }
         ++it;   
      }   
         
   }  
 
   // Add the coordinates of the destination
   if ( reqType & (ROUTE_TYPE_GFX_COORD | ROUTE_TYPE_NAVIGATOR |
                   ROUTE_TYPE_ALL_COORDINATES | ROUTE_TYPE_GFX_TRISS) ) 
   {
      Item* lastItem = NULL;      
      uint32 lastID = MAX_UINT32;
      list<uint32>::const_reverse_iterator rit = nodeIDs.rbegin();
      // The last item in the node list may very well be a addcost item.
      while ( lastItem == NULL && rit != nodeIDs.rend() ) {
         lastID = *rit;
         lastItem = item_cast<RouteableItem*>
            (m_map->itemLookup(lastID & 0x7fffffff));
         ++rit;
      }
      
      GfxData* gfx;
      if ( (lastItem != NULL) && 
           ((gfx = lastItem->getGfxData()) != NULL)) 
      {
         int32 lat, lon;
         if( reqType & ROUTE_TYPE_GFX_TRISS )
         {
            gfx->getCoordinate( (uint16) (MAX_UINT16/2), lat, lon);
         }
         else {
            if (req->ignoreEndOffset()) {
               // Just add the last coordinate, no offset.
               uint32 n = 0;
               if ((lastID & 0x80000000) == 0) {
                  n = gfx->getNbrCoordinates(0) - 1;
               }
               lat = gfx->getLat(0,n);
               lon = gfx->getLon(0,n);
            } else {
               // Use offset
               if (! gfx->getCoordinate(req->getEndOffset(), lat, lon)) {
                  // Error getting offset
                  mc2log << warn <<"ExpandRouteProcessor Error getting "
                         << "coordinates from offset" << endl;
                  lat = lon = 0;
               }
            }
         }
         
         if (reqType & (ROUTE_TYPE_NAVIGATOR |
                        ROUTE_TYPE_ALL_COORDINATES) )
         {
            reply->addCoordToNavItemData(lat, lon, nbrCoordPos);
            mc2dbg4 << "lat :" << lat << " lon :" << lon << endl;
         }
         else if( reqType & ROUTE_TYPE_GFX_TRISS ) {
            byte nodeNbr = 0;
            if ( lastID & 0x80000000 ) {
               nodeNbr = 1;
            }

            float64 angle = gfx->getAngle(MAX_UINT16/2);

            if( nodeNbr == 1 )
               angle = float64( angle + 180 );
            
            if( angle >= (float64) 360 )
               angle = (float64) (angle - 360);
                               
            int n = 0;   // The quadrant of the angle
             
            if( (angle >= 0) && (angle <= 90) )
               n = 1;
            else if( (angle > 90) && (angle <= 180) )
               n = 2;
            else if( (angle > 180) && (angle <= 270) )
               n = 3;
            else if( angle > 270 )
               n = 4;
             
            float64 deltaY = 0, deltaX = 0;
            float64 alfa;
             
            switch(n) {
              case 1:
              {
                 alfa = (float64) (angle / 360 * 2 * M_PI);
                 float64 h = (float64) 1000000;
                 deltaY = (float64) ( h * sin(alfa) );
                 deltaX = (float64) ( h * cos(alfa) );
              }
              break;
                
              case 2:
              {
                 if( angle == 180 ) {
                    deltaY = 0;
                    deltaX = -1000000;
                 }
                 else {
                    alfa = (float64) (angle - 90);
                    alfa = (float64) (alfa / 360 * 2 * M_PI);
                    float64 h = (float64) 1000000;
                    deltaY = (float64) ( h * sin(alfa) );
                    deltaX = -(float64) ( h * cos(alfa) );
                 }
              }
              break;
                     
              case 3:
              {
                 alfa = (float64) (angle - 180);
                 alfa = (float64) (alfa / 360 * 2 * M_PI);
                 float64 h = (float64) 1000000;
                 deltaY = -(float64) ( h * sin(alfa) );
                 deltaX = -(float64) ( h * cos(alfa) );
              }
              break;
              
              case 4:
              {
                 alfa = (float64) (angle - 270);
                 alfa = (float64) (alfa / 360 * 2 * M_PI);
                 float64 h = (float64) 1000000;
                 deltaY = -(float64) ( h * sin(alfa) );
                 deltaX = (float64) ( h * cos(alfa) );
              }
              break;
            }
             
            reply->addCoordToNavItemData(lat + (int32) deltaY,
                                         lon + (int32) deltaX,
                                         nbrCoordPos);
         } 
         // Set the last coordinate. 
         // (Done again for ROUTE_TYPE_NAVIGATOR for compatibility reasons)
         reply->setLastItemPosition(lat, lon);
      } else {
         mc2log << warn <<"ExpandRouteProcessor lastItem == NULL"<< endl;
      }
      
   }
   
   // DEBUGGING
   DEBUG8(
      reply->dump2(true);
      ExpandItemID* exp = reply->getItemID();
      Vector& mapIDs = exp->getMapID();
      Vector& itemIDs = exp->getItemID();
      for (uint32 i=0; i<exp->getNbrItems(); i++)
      mc2dbg << i << ". GfxRoute " << mapIDs[i] << "." 
      << itemIDs[i] << endl;
   );                     

   return (true);
} 



bool
ExpandRouteProcessor::setTurnNumbersAndRoundabouts()
{
   return (true);
}
bool
ExpandRouteProcessor::newSetTurnNumbersAndRoundabouts(bool passedRoadLM)
{
   mc2dbg4 << "newSetTurnNumbersAndRoundabouts" << endl;
   ExpandRouteLink* setLink = static_cast<ExpandRouteLink*>
      (m_routeList->last());
   ExpandRouteLink* toLink = NULL;
   ExpandRouteLink* fromLink = NULL;
   byte leftCount  = 0;
   byte rightCount = 0;
   
   while(setLink != NULL) {
      bool lastLink = (setLink->suc() == NULL);
      if((setLink->m_exitCount != MAX_BYTE )){
         toLink = setLink;
      } else {
         toLink = static_cast<ExpandRouteLink*>(setLink->pred());
      }
      bool search = false;
      if((toLink != NULL) &&
         ((setLink->m_turnDescription == StringTable::RIGHT_TURN) ||
          (setLink->m_turnDescription == StringTable::LEFT_TURN) ||
          (setLink->m_turnDescription == StringTable::EXIT_ROUNDABOUT_TURN) ||
          lastLink       // Special case! handle diffrent!
          )){
         fromLink = static_cast<ExpandRouteLink*>(toLink->pred());
         setLink->m_turnNumber = 1;
         if(fromLink != NULL){
            search = true;
         }
      }
      
      // No need to search for these turns.
      else if (((setLink->m_turnDescription ==
                 StringTable::RIGHT_ROUNDABOUT_TURN) && 
                 setLink->driveOnRightSide()) ||
               ((setLink->m_turnDescription ==
                 StringTable::LEFT_ROUNDABOUT_TURN) && 
                 !setLink->driveOnRightSide())){
         // Direct turn in roundabout.
         // (Not certain that this description kan be used. Might be
         //  assymmetric.)
         // Could just check crossingKind, but hopefully more cases of 
         // symmetry could be detected by getRoundaboutInfo()
         if(int(setLink->getCrossingKind()) >=
            int(ItemTypes::CROSSING_2ROUNDABOUT)){
            byte entryCount = 0;
            if(toLink != NULL){
               entryCount = toLink->m_exitCount;
            }
            
            StringTable::stringCode roundaboutTurn;
            getRoundaboutInfo(1,0,setLink->getCrossingKind(),
                              setLink->m_transportation, 
                              setLink->driveOnRightSide(),
                              roundaboutTurn,
                              entryCount,
                              setLink->m_exitCount);
            setLink->m_turnDescription = roundaboutTurn;
         }
         
      }
      
      // Lets search for earlier possible turns.
      if(search){
         fromLink = static_cast<ExpandRouteLink*>(toLink->pred());
         uint32 searchDist = setLink->m_dist;
         
         // Search loop. 
         while((fromLink != NULL) && search && (searchDist < 10000) &&
               (setLink->m_turnNumber < 15)){
            if((toLink == setLink) ||
               (toLink->m_turnDescription == StringTable::FOLLOWROAD_TURN)||
               (toLink->m_turnDescription == StringTable::AHEAD_TURN)){

               if(!lastLink){
                  // Normal addition of turn count.
                  bool useLM = passedRoadLM && (searchDist < 1000);
                  addStreetCount(setLink, fromLink, toLink, useLM);
               } else {
                  // Last link need info for reply packet
                  // Use addStreetCount to increase left & right turn count.
                  StringTable::stringCode oldTurn = setLink->m_turnDescription;
                  setLink->m_turnDescription = StringTable::LEFT_TURN;
                  leftCount += addStreetCount(setLink, fromLink,
                                              toLink, false, true);
                  setLink->m_turnDescription = StringTable::RIGHT_TURN;
                  rightCount += addStreetCount(setLink, fromLink,
                                              toLink, false, true);
                  setLink->m_turnDescription = oldTurn;
               }
               
               
               // update distance.
               searchDist += toLink->m_dist;
               // update pointers.
               toLink   = fromLink;
               fromLink = static_cast<ExpandRouteLink*>(fromLink->pred());
               
            } else {
               // We reached a turn and aborts.
               search = false;
            }
         }
         // Was it a roundabout?
         if(setLink->m_turnDescription == StringTable::EXIT_ROUNDABOUT_TURN) {
            if(toLink->m_turnDescription == StringTable::ENTER_ROUNDABOUT_TURN) {
               addStreetCount(setLink, fromLink, toLink, false);
               
               StringTable::stringCode roundaboutTurn;
               getRoundaboutInfo(setLink->m_turnNumber,
                                 setLink->getRestrictedRbExit(),
                                 setLink->getCrossingKind(),
                                 setLink->m_transportation,
                                 setLink->driveOnRightSide(),
                                 roundaboutTurn,
                                 toLink->m_exitCount,
                                 setLink->m_exitCount);
               setLink->m_turnDescription = roundaboutTurn;               
            } else {
               mc2log << error
                      << "Exit roundabout wasn't preceded by enter roundabout"
                      << endl;
            }
         } else { // No roundabout!
            // Did we turn left (if right side traffic)from a multiDig street? 
            // If so and this is a left turn remove the
            // first turn and the first passed road landmark.
            if((setLink->m_turnNumber > 0) &&
               (toLink->m_turnDescription == setLink->m_turnDescription) &&
               ((setLink->driveOnRightSide() &&
                 (toLink->m_turnDescription == StringTable::LEFT_TURN)) ||
                (!setLink->driveOnRightSide() &&
                 (toLink->m_turnDescription == StringTable::RIGHT_TURN)))){

               StreetSegmentItem* ssi =
                  item_cast<StreetSegmentItem*>(fromLink->m_item);
               if((ssi != NULL) && (ssi->isMultiDigitised())){
                  // reduce turn number
                  setLink->m_turnNumber -= 1;
                  
                  // remove first passed street.
                  if(!setLink->m_passedStreets.empty()){
                     setLink->m_passedStreets.pop_front();
                  }
               }
            }
            
            // Check if we should loose the turn number.
            if((setLink->m_turnDescription
                 != StringTable::EXIT_ROUNDABOUT_TURN) &&
               ((searchDist > 10000) ||(setLink->m_turnNumber > 9 ))){
               setLink->m_turnNumber = MAX_BYTE;
            }
         }
      }
      setLink = static_cast<ExpandRouteLink*>(setLink->pred());
   }
   // Set left and right turnCount for the replyPacket.
   
   m_routeList->m_lastSegmentLeftStreetCount  = leftCount;
   m_routeList->m_lastSegmentRightStreetCount = rightCount;
   
   return true;
}

byte
ExpandRouteProcessor::addStreetCount(ExpandRouteLink* turnLink,
                                     ExpandRouteLink* fromLink,
                                     ExpandRouteLink* toLink,
                                     bool usePassedRoadsLM,
                                     bool noSetting)
{
   
   /*
   // Description of some of the variables used used here.
   // This method checks for a turn type when driving from fromLink to 
   // toLink
   //
   // Driving direction --------------->
   //
   //          fromLink               toLink
   //             ^                     ^
   //  fromNode   ^      oppositeNode   ^
   //     ^       ^          ^          ^
   //     o------------------o o-----------------o
   //      \----------------->o <- 
   //            ^            |   \
   //       fromOppNodeConn   |     toNode
   //                         |
   //                         |
   //                         |
   //                         o 
   */
   StringTable::stringCode searchedTurn = turnLink->m_turnDescription;
   bool useOffRamp =(COUNT_OFF_RAMP_AS_TURN || m_noConcatinate);
   //Node* fromNode = fromLink->item->getNodeFromID(fromLink->m_nodeID);

   // Need const on oppositeNode and fromAngle
   // gcc 3.4.4 20050721 (Red Hat 3.4.4-2) with -O3
   // Seems to over optimize and get "this" pointer wrong when
   // calling getNodeAngle when calculating thisAngle and toAngle.
   // Also need volatile on thisAngle and toAngle.

   const Node* const oppositeNode = fromLink->m_item->
      getNodeFromID(fromLink->m_nodeID ^ 0x80000000);
   byte landMarksAdded = 0;
   uint16 nbrConn = oppositeNode->getNbrConnections();
   const char** landmarks = new const char*[nbrConn];
   // Setting all strings to null so ignored streets can be avoided
   for(uint16 ix = 0 ; ix < nbrConn ; ix++){
      landmarks[ix] = NULL;
   }
   
   volatile const float64 fromAngle = getNodeAngle(oppositeNode);
   byte turnValue = 0;
   
   for (uint32 i=0; i < nbrConn; i++) {
      Connection* conn = oppositeNode->getEntryConnection(i);
      Connection* fromOppNodeConn = 
         m_map->getOpposingConnection(conn, oppositeNode->getNodeID());
      // All connection with wrong turndiredction can be ignored.
      if((fromOppNodeConn != NULL) &&
         ((ItemTypes::getTurndirectionSC(fromOppNodeConn->getTurnDirection())
           == searchedTurn)  ||
          ((searchedTurn == StringTable::EXIT_ROUNDABOUT_TURN) &&
           (fromOppNodeConn->getTurnDirection() ==
            ItemTypes::RIGHT_ROUNDABOUT)) ||
          ((searchedTurn == StringTable::EXIT_ROUNDABOUT_TURN) &&
           (fromOppNodeConn->getTurnDirection() ==
            ItemTypes::LEFT_ROUNDABOUT)) ||
         ((useOffRamp &&
          ((fromOppNodeConn->getTurnDirection() == ItemTypes::OFF_RAMP) &&
          (((searchedTurn == StringTable::RIGHT_TURN) && 
             turnLink->driveOnRightSide()) ||
           ((searchedTurn == StringTable::LEFT_TURN) && 
            !turnLink->driveOnRightSide())))
          )
          ||
          ((searchedTurn == StringTable::RIGHT_TURN) &&
           (fromOppNodeConn->getTurnDirection() == ItemTypes::OFF_RAMP_RIGHT))
          ||
          ((searchedTurn == StringTable::LEFT_TURN) &&
           (fromOppNodeConn->getTurnDirection() == ItemTypes::OFF_RAMP_LEFT))
          ))){


         
         // Check if this exit should be added to turn nbr.
         Node* toNode = m_map->nodeLookup(
            conn->getConnectFromNode() ^ 0x80000000);
         
         bool incTurnNumber = true;
         bool incRestricted = false;
         if(searchedTurn == StringTable::EXIT_ROUNDABOUT_TURN){
            
            // Dont count noWay e.g. entries into the roundabout
            if(toNode->getEntryRestrictions() == ItemTypes::noWay){
               incTurnNumber = false;
               incRestricted = true;
            }
            
            // If this is the link of the turn, we have to check if there are
            // multiple exits here.
            if(( (incRestricted || incTurnNumber) && (turnLink == toLink))){

               // Find angles for the entry && exit.
               // need volatile here, due to GCC 3.4.4 with -O3 bug 
               // See comment at start of function.
               volatile float64 toAngle = fromAngle -
                  getNodeAngle(toLink->m_item->
                               getNodeFromID(toLink->m_nodeID));

               if(toAngle < 0)
                  toAngle += M_PI;

               volatile float64 thisAngle = fromAngle - getNodeAngle(toNode);

               if(thisAngle < 0)
                  thisAngle += M_PI;


               if(toLink->driveOnRightSide()){
                  // Right side traffic.
                  if(thisAngle >= toAngle){
                     // The exit is not before the made one. Dont count.
                     // NB: if it is the same exit this is false.
                     incTurnNumber = false;
                     incRestricted = false;
                  } 
               } else if(thisAngle <= toAngle){
                     incTurnNumber = false;
                     incRestricted = false;
               }
            }
            
            
         } else {
            // Not roundabout.
            StreetSegmentItem* ssi = item_cast<StreetSegmentItem*>
                  (m_map->itemLookup(conn->getConnectFromNode() & 0x7fffffff));
            
            // Check cases where noWay streets are ignored.
            if(toNode->getEntryRestrictions() == ItemTypes::noWay) {
               // Check if ramp or multidig.
               if((ssi != NULL) &&
                  ((ssi->isRamp()) || (ssi->isMultiDigitised()))){
                  incTurnNumber = false;
               }
               
            }
            // Ignore connections where the exit count is set to MAX_BYTE.
            // As this is used in the map to indicate that the connection
            // shouldn't be counted.
            if(fromOppNodeConn->getExitCount() == MAX_BYTE){
               incTurnNumber = false;
            }

            // Is this in the same crossing as the made turn?
            if(turnLink == toLink){
               mc2dbg4 << " Same link ";
               // Only add if the exitcount is lower than the exitcount of
               // the turnLink. (This turn is before the made turn)
               // This means that the actual turn isn't counted (as it has
               // same exitCount as it self)
               if(turnLink->m_exitCount <= fromOppNodeConn->getExitCount()){
                  incTurnNumber = false;
                  mc2dbg4 << "exit count is higher, will not include";
               }
               mc2dbg4 << endl;
            }
           
         
            // Should a passed road landmark be added?
            if(incTurnNumber && usePassedRoadsLM && !noSetting){
               LangTypes::language_t prefLang =
                  ItemTypes::getLanguageCodeAsLanguageType(
                     StringTable::languageCode(
                        m_preferedLanguages.getElementAt(0)));
               const char* name = m_map->getBestItemName(
                  ssi->getID(), prefLang);
               if(fromOppNodeConn->getExitCount() <= 0){
                   mc2log << error << "ExitCount "
                          << (int)fromOppNodeConn->getExitCount()
                          << " for passed roads landmark "
                          << "is zero, passed road "
                          << name << " not added. Item ID : "
                          << ssi->getID() << " map = "
                          << hex << m_map->getMapID() << dec <<  endl;
                   
               }
               else if(fromOppNodeConn->getExitCount() < nbrConn){
                  landMarksAdded++;
                  landmarks[fromOppNodeConn->getExitCount()-1] = name;
                  
               } else {
                  mc2log << error << "ExitCount "
                         << (int)fromOppNodeConn->getExitCount()
                         << " for passed roads landmark "
                         << "is higher than nbrConn ("
                         << nbrConn  << "), passed road "
                         << name << " not added." << endl;
                  //abort();
               }   
            }
            
         }
         
         
         // Ok add the turn.
         if(incTurnNumber){
            mc2dbg4 << "made turn " << StringTable::
               getString( toLink->m_turnDescription, StringTable::ENGLISH )
                   << " ";
            if(searchedTurn == StringTable::EXIT_ROUNDABOUT_TURN){
               turnLink->m_turnNumber += 1;
            } else if(!noSetting){
               toLink->m_turnNumber += 1;
            }
            turnValue += 1;
         }
         else if(incRestricted){
            turnLink->setRestrictedRbExit( turnLink->getRestrictedRbExit() + 1 ); 
         }
      }
      
   }
   
   if(landMarksAdded > 0){
      // add the passed roads landmarks that are set.
      for(uint16 ix = nbrConn ; ix > 0 ; ix--){
         // Add if name set an not to many
         if((landmarks[ix-1] != NULL) &&
            (turnLink->m_passedStreets.size() < uint32(m_nbrOfPassedStreets))){
            turnLink->addPassedStreet(landmarks[ix-1]);
         }
      }
   }
   
   delete[] landmarks;
   return turnValue;
}

bool
ExpandRouteProcessor::setNamesAndConcatenate(bool firstPacket,
                                             bool lastPacket,
                                             bool printAllNames,
                                             bool removeAheadIfNameDiffer,
                                             bool navigator,
                                             byte routeType,
                                             bool nameChangeAsWaypoint)
{
   
   mc2dbg2 <<"ENTERING SET NAMES AND CONC: " << endl;
   
   if(nameChangeAsWaypoint){
      removeAheadIfNameDiffer = true;
      mc2dbg2 << " ++++++  nameChangeAsWaypoint    = true  ++++++  " << endl;
   } else if(removeAheadIfNameDiffer){
      mc2dbg2 << " ++++++  removeAheadIfNameDiffer = true  ++++++  " << endl;
   } else {
      mc2dbg2 << " ++++++  removeAheadIfNameDiffer = false ++++++  " << endl;
   }
   
   ExpandRouteLink* prevLink = NULL;
   ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>
      (m_routeList->first());
   curLink->m_nbrItems = 0;

   while (curLink != NULL) {
      updatePossibleNames(curLink->m_item);
      setBestName(curLink, prevLink, false);
      prevLink = curLink;
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   }
   
   // Make another loop to see if name changes at FOLLOW_ROAD should be 
   // discarded.
   prevLink = static_cast<ExpandRouteLink*>  
      (m_routeList->first());  
   curLink = static_cast<ExpandRouteLink*>
      (prevLink->suc());  
   while (curLink != NULL) {  
      if((curLink->m_turnDescription == StringTable::FOLLOWROAD_TURN) &&   
         !sameName(prevLink, curLink)){
         ExpandRouteLink* nextLink = static_cast<ExpandRouteLink*>  
            (curLink->suc());   
         if(prevLink->m_item->hasSameNames(nextLink->m_item)){  
            mc2dbg4 << "Found short name switch" << endl;
            if(newName(curLink, prevLink)){
               mc2dbg4 << "New name : " << curLink->m_streetName
                       << " found." << endl;
            } else {
               mc2dbg4 << "No new name found, changing "
                       << curLink->m_streetName << " to "
                       << prevLink->m_streetName << endl;
               
               // Set the corrected name of curLink
               curLink->replaceName(prevLink->m_stringCode);
               
               do{
                  // Set the name of the following links until we have a new
                  // name change or not follow road anymore.
                  
                  
               } while((nextLink != NULL) &&
                       curLink->m_item->hasSameNames(nextLink->m_item) &&
                       (nextLink->m_turnDescription !=
                        StringTable::FOLLOWROAD_TURN));
               
                  
               
               //curLink->setNewName(prevLink->m_streetName);
            }
            
            
         }  
            
      }   
      prevLink = static_cast<ExpandRouteLink*>(prevLink->suc());  
      curLink = static_cast<ExpandRouteLink*>(prevLink->suc());   
   }
   
   // Expand names & correct names when turning
   // 1. Change name: if turning-> short segment with same name-> ahead to
   //    new name.
   // 2. Expand name: if no name, not to long + follow road to a name.
   prevLink = static_cast<ExpandRouteLink*>
      (m_routeList->first());
   curLink = static_cast<ExpandRouteLink*>
      (prevLink->suc());
   ExpandRouteLink* nextLink = static_cast<ExpandRouteLink*>
      (curLink->suc());

   
    // Initialize the possible names.
   updatePossibleNames(prevLink->m_item);
   mc2dbg4 << "Modifying names ::" << endl;
   while (nextLink != NULL) {

      
      // Remove short links if they are between FOLLOW_ROAD and/or AHEAD 
      if((nextLink->m_dist < 10) &&
         ((curLink->m_turnDescription == StringTable::FOLLOWROAD_TURN) ||
          (curLink->m_turnDescription == StringTable::AHEAD_TURN)) &&
         ((nextLink->m_turnDescription == StringTable::FOLLOWROAD_TURN) ||  
          (nextLink->m_turnDescription == StringTable::AHEAD_TURN)) &&
         !sameName(prevLink, curLink) && !sameName(curLink, nextLink)){ 
         curLink->replaceName(prevLink->m_stringCode);
         mc2dbg4 << "Small route segment using old name instead : "  
                 << prevLink->m_streetName << endl;  
      }  
      

      // if curLink has no name && nextlink has  && follow between
      // check lenght of curent
      // current = next
      else if(((nextLink->m_turnDescription == StringTable::FOLLOWROAD_TURN) &&
          curLink->noName() && !nextLink->noName())
         )
      {
         // Add prevLink name to curLink name
         curLink->replaceName(nextLink->m_stringCode);
         mc2dbg4 << "Extending next name to current : "
                 << nextLink->m_streetName << endl;
      }
      
      // If turning to a _short_ segment with the same name and then AHEAD
      // to a item that has a name, this name replaces the name of the short
      // segment. Also when exiting a roundabout, the name of the small
      // "exit segment" is ignored.
      
     // else if turnLeft or right  to same name, current is short and then
      // ahead to new name current = next
      else if((((curLink->m_turnDescription == StringTable::RIGHT_TURN) ||
                (curLink->m_turnDescription == StringTable::LEFT_TURN) ||
                (curLink->m_turnDescription == StringTable::EXIT_ROUNDABOUT_TURN)) &&
               !nextLink->noName() &&
               ((nextLink->m_turnDescription == StringTable::AHEAD_TURN) ||
                (nextLink->m_turnDescription == StringTable::FOLLOWROAD_TURN)) &&
               (sameName(prevLink, curLink) || curLink->noName())) ||
              (((curLink->m_turnDescription == StringTable::EXIT_ROUNDABOUT_TURN)) &&
               !sameName(curLink, nextLink) && (nextLink->m_dist < 51) )
              )
      {
         curLink->replaceName(nextLink->m_stringCode);
         mc2dbg4 << "Using next name after turn : "
                 << nextLink->m_streetName << endl;
      }
      // If turning LEFT (or RIGHT if left side traffic) from a multidig
      // street to a small segment and then AHEAD to a new name, this new
      // name is used on the small segment.
      else if((nextLink->m_turnDescription == StringTable::AHEAD_TURN) &&
              (((curLink->m_turnDescription == StringTable::LEFT_TURN) &&
                curLink->driveOnRightSide()) ||
               ((curLink->m_turnDescription == StringTable::RIGHT_TURN) &&
                curLink->driveOnRightSide())) &&
              (nextLink->m_dist < 35) && !sameName(curLink, nextLink)
              ){
         curLink->replaceName(nextLink->m_stringCode);
         mc2dbg4 << " name before turn to multidig: "
                 << nextLink->m_streetName << endl;
      }      
      // If we are turning left(if rightside traffic) into a multi dig street
      // the last "inbetween" segment might have the wrong name which will
      // show up on the navClient. Set previous name instead
      else if((curLink->m_turnDescription == StringTable::AHEAD_TURN) &&
               (nextLink->m_dist < 35) && !sameName(curLink, prevLink) &&
              (((nextLink->m_turnDescription == StringTable::LEFT_TURN) &&
                curLink->driveOnRightSide()) ||
               (!curLink->driveOnRightSide() &&
                (nextLink->m_turnDescription == StringTable::RIGHT_TURN)))){
         curLink->replaceName(prevLink->m_stringCode);
         mc2dbg4 << "Keeping name before turn to multidig: "
                 << prevLink->m_streetName << endl;
      }
      // A ramp has no name, or with the same name as the road left.
      // We follow the ramp until we reach a non ramp
      // segment and/or we find a name.
      else if((((curLink->m_turnDescription == StringTable::OFF_RAMP_TURN) ||
                (curLink->m_turnDescription ==
                 StringTable::LEFT_OFF_RAMP_TURN) ||
                (curLink->m_turnDescription ==
                 StringTable::RIGHT_OFF_RAMP_TURN)) ||
               (((curLink->m_turnDescription ==
                StringTable::EXIT_ROUNDABOUT_TURN) ||
                 (curLink->m_turnDescription ==
                StringTable::LEFT_TURN) ||
                 (curLink->m_turnDescription ==
                StringTable::RIGHT_TURN)) &&
                (curLink->m_item->getItemType() ==
                ItemTypes::streetSegmentItem) &&
                static_cast<StreetSegmentItem*>(curLink->m_item)->isRamp())) &&
              (curLink->noName() ||  commonName(curLink, prevLink))){
         // byte exitRC = prevLink->m_iteamm->getRoadClass(); could B used
         bool searchDone = false;
         ExpandRouteLink* searchLink = nextLink;
         uint32 searchDist = 0;
         while((searchLink != NULL) && !searchDone && 
               (searchLink->m_item->getItemType() ==
                ItemTypes::streetSegmentItem)){
            StreetSegmentItem* ssi = static_cast<StreetSegmentItem*>
               (searchLink->m_item);
            searchDist += ssi->getLength();
            
            if(ssi->isRoundabout()){
               // Ignore name if in a roundabout
               mc2dbg4 << " on roundabout";
               
               searchDist = 0;
            }
            else if(!searchLink->noName() &&
                    !commonName(searchLink, prevLink)){
               searchDone = true;
               mc2dbg4 << " found new name " << endl;
            }
            if(ssi->isRamp()){
               mc2dbg4 << " still on ramp";
               searchDist = 0;
            }
         
            if((searchLink->m_turnDescription == StringTable::LEFT_TURN) ||
               (searchLink->m_turnDescription == StringTable::RIGHT_TURN) ||
               (searchLink->m_turnDescription == StringTable::PARK_CAR)){
               searchDone = true;
            }
            mc2dbg4 << " distance " << searchDist << endl;
            if((searchDist > 500) ||
               ((searchDist > 100) && !searchLink->noName()))
               searchDone = true;
            if(!searchDone){
               searchLink = static_cast<ExpandRouteLink*>(searchLink->suc());
            }
            
         }
         if((searchLink != NULL) && !searchLink->noName()  &&
            !commonName(searchLink, curLink)){
            ExpandRouteLink* nameLink = curLink;
            do{
               nameLink->replaceName(searchLink->m_stringCode);
               mc2dbg4 << "Moving name to ramp from later name : "
                      << searchLink->m_streetName << endl;
               nameLink = static_cast<ExpandRouteLink*>(nameLink->suc());
            } while (nameLink != searchLink);
         }
      }
      else
      {
         mc2dbg4 << "Nothing special, name = " << curLink->m_streetName
                 << ", turn  = " << (int)curLink->m_turnDescription
                 << ", nTurn = " << (int)nextLink->m_turnDescription
                 << ", dist = " << nextLink->m_dist <<  endl;
         
      }
     // Possible changes in turndescription
      // Remove KEEP_LEFT ?
      if((curLink->m_turnDescription == StringTable::KEEP_LEFT) &&
         !m_noConcatinate){
         bool remove = false;
         bool search = true;
         uint32 dist = 0;
         ExpandRouteLink* searchLink = nextLink;
         while((searchLink != NULL) && search){
            dist += searchLink->m_dist;
            float64 turnAngle = getConnectionAngle(curLink, searchLink);
            mc2dbg4 << "Turn angle : " << turnAngle << endl;
            if(((searchLink->m_turnDescription != StringTable::AHEAD_TURN) &&
               (searchLink->m_turnDescription
                != StringTable::FOLLOWROAD_TURN))||
               (dist > 100) || (turnAngle > M_PI_6) || (turnAngle < -M_PI_6)){
               search = false;
            }
            if(searchLink->m_turnDescription == StringTable::LEFT_TURN){
               if((dist < 150) && (turnAngle < -2*M_PI_6) &&
                  (turnAngle > -4*M_PI_6)){
                  remove = true;
               }
               else{
                  mc2dbg4 << " no KEEP_LEFT change d= " << dist << " ang :"
                         << turnAngle << endl;
               }
               
            } else if(!search){
               mc2dbg4 << "no left turn" << endl;
            }
            
            searchLink = static_cast<ExpandRouteLink*>(searchLink->suc());
         }
         if(remove){
            curLink->m_turnDescription = StringTable::AHEAD_TURN;
            mc2dbg4 << "KEEP_LEFT is followed by LEFT."
                    << " KEEP_LEFT changed to AHEAD" << endl;
         }
         
      }
      // Remove KEEP_RIGHT ?
      else if ((curLink->m_turnDescription == StringTable::KEEP_RIGHT) &&
               !m_noConcatinate) {
         bool remove = false;
         bool search = true;
         uint32 dist = 0;
         ExpandRouteLink* searchLink = nextLink;
         while((searchLink != NULL) && search){
            dist += searchLink->m_dist;
            float64 turnAngle = getConnectionAngle(curLink, searchLink);
            mc2dbg4 << "Turn angle : " << turnAngle << endl;
            if(((searchLink->m_turnDescription != StringTable::AHEAD_TURN) &&
               (searchLink->m_turnDescription
                != StringTable::FOLLOWROAD_TURN))||
               (dist > 100) || (turnAngle > M_PI_6) || (turnAngle < -M_PI_6)){
               search = false;
            }
            if(searchLink->m_turnDescription == StringTable::RIGHT_TURN){
               if((dist < 150) && (turnAngle > 2*M_PI_6) &&
                  (turnAngle < 4*M_PI_6)){
                  remove = true;
               }
               else{
                  mc2dbg4 << " no KEEP_RIGHT change d= " << dist << " ang :"
                         << turnAngle << endl;
               }
               
            } else if(!search){
               mc2dbg4 << "no right turn" << endl;
            }
            
            searchLink = static_cast<ExpandRouteLink*>(searchLink->suc());
         }
         if(remove){
            curLink->m_turnDescription = StringTable::AHEAD_TURN;
            mc2dbg4 << "KEEP_RIGHT is followed by RIGHT."
                    << " KEEP_RIGHT changed to AHEAD" << endl;
         }
         
      }
      // Replace OFF_RAMP ( +? ON_RAMP) with turn.
      else if ((curLink->m_turnDescription == StringTable::OFF_RAMP_TURN) ||
               (curLink->m_turnDescription ==
                StringTable::LEFT_OFF_RAMP_TURN) ||
               (curLink->m_turnDescription ==
                StringTable::RIGHT_OFF_RAMP_TURN)){
         bool replace = false;
         bool search  = true;
         uint32 dist  = 0;
         StreetSegmentItem* ssi = NULL;
         ExpandRouteLink* searchLink = nextLink;
         while((searchLink != NULL) && search && (dist < 70)){
            dist += searchLink->m_dist;

            ssi = item_cast<StreetSegmentItem*>(searchLink->m_item);
            if(searchLink->m_turnDescription == StringTable::ON_RAMP_TURN){
               search = false;
            } else {
               if((ssi == NULL) || !ssi->isRamp()){
                  search = false;
               }
            }
            if(search)
               searchLink = static_cast<ExpandRouteLink*>(searchLink->suc());
         }
         replace = ((dist < 70) &&  // 70 is the longest known bad ramp.
                    (ssi != NULL));
         if(replace){
            mc2dbg4 << "Ramp to short , OFF_RAMP_TURN changed to ";
            if((curLink->driveOnRightSide() &&
                (curLink->m_turnDescription == StringTable::OFF_RAMP_TURN)) ||
               (curLink->m_turnDescription ==
                StringTable::RIGHT_OFF_RAMP_TURN)) {
                mc2dbg4 << " RIGHT_TURN." << endl;
               curLink->m_turnDescription = StringTable::RIGHT_TURN;
               
            } else {
               mc2dbg4 << " LEFT_TURN." << endl;
               curLink->m_turnDescription = StringTable::LEFT_TURN;
            }
            if ( ( searchLink != NULL ) &&
                ( searchLink->m_turnDescription == 
                  StringTable::ON_RAMP_TURN ) ) {
               mc2dbg4 << "    ON_RAMP_TURN changed do AHEAD_TURN" << endl;
               searchLink->m_turnDescription = StringTable::AHEAD_TURN;
            }
         }
         
      }
      // Replace OFF_RAMP with OFF_MAIN if there is no highway
      if((curLink->m_turnDescription == StringTable::OFF_RAMP_TURN) ||
               (curLink->m_turnDescription ==
                StringTable::LEFT_OFF_RAMP_TURN) ||
               (curLink->m_turnDescription ==
                StringTable::RIGHT_OFF_RAMP_TURN)){
         // Check if prevLink was ctr_acess if not go back untill not on
         // ramp, and the 500 m to see if we can find a ctrl_acc.
         mc2dbg2 << "checking for OFF_MAIN" << endl;
         bool  notFound = (!static_cast<StreetSegmentItem*>(curLink->m_item)
                           ->isControlledAccess());
         int32 dist = 0;
         if(!static_cast<StreetSegmentItem*>(curLink->m_item)->isRamp())
            dist = curLink->m_dist;
         
         ExpandRouteLink* searchLink = prevLink;
         while((searchLink != NULL) && notFound && (dist < 500)){
            // If not ssi any more. (ferry maybe)
            if(item_cast<StreetSegmentItem*>(searchLink->m_item) == NULL){
               searchLink = NULL;
               mc2dbg2 << " Not ssi " << endl;
            }
            else {
               if(static_cast<StreetSegmentItem*>
                  (searchLink->m_item)->isControlledAccess()){
                  mc2dbg2 << "found ctrl_acc" << endl;
                  notFound = false; // We are done!
               } else {
                  // Increase counter as soon as we leave first ramp.
                  if((!static_cast<StreetSegmentItem*>
                      (searchLink->m_item)->isRamp()) ||
                     (dist != 0)){ 
                     dist+= searchLink->m_dist;
                     mc2dbg2 << "Searching for ctrl_acc : " << dist << " m.(1)"
                            << endl;
                  }

                  searchLink = static_cast<ExpandRouteLink*>
                     (searchLink->pred());
               }
            }
         }

         if(notFound){
            mc2dbg2 << "OFF_RAMP changed to OFF_MAIN" << endl;
            curLink->m_turnDescription = StringTable::OFF_MAIN_TURN;
         }
         DEBUG1(
         else {
            mc2dbg << "Ctrl Access found using OFF_RAMP." << endl;
         }
         );
         
         
      }
      // Replace ON_RAMP with ON_MAIN if there is no highway
      else if (curLink->m_turnDescription == StringTable::ON_RAMP_TURN){
         mc2dbg << "checking for ON_MAIN" << endl;
         bool  notFound = true;
         int32 dist     = 0;
         ExpandRouteLink* searchLink = nextLink;
         while((searchLink != NULL) && notFound && (dist < 500)){
            if (item_cast<StreetSegmentItem*>(searchLink->m_item) == NULL){
               searchLink = NULL;
               mc2dbg2 << " Not ssi " << endl;
            }
            else {
               if(static_cast<StreetSegmentItem*>
                  (searchLink->m_item)->isControlledAccess()){
                  notFound = false;
                  mc2dbg << "found ctrl_acc" << endl;
               } else {
                  // Increase counter as soon as we leave the ramp.
                  if((!static_cast<StreetSegmentItem*>
                      (searchLink->m_item)->isRamp()) ||
                     (dist != 0)){
                     dist += searchLink->m_dist;
                     mc2dbg << "Searching for ctrl_acc : " << dist << " m."
                            << endl;
                  }
                  
                  searchLink = static_cast<ExpandRouteLink*>
                     (searchLink->suc());
               }
            }
         }
         
         
         if(notFound){
            mc2dbg << "ON_RAMP changed to ON_MAIN" << endl;
            curLink->m_turnDescription = StringTable::ON_MAIN_TURN;
         }
         DEBUG1(
         else {
            mc2dbg << "Ctrl Access found using ON_RAMP." << endl;
         }
         );

      }
      
      // Update pointers
      prevLink = curLink;
      curLink  = nextLink;
      nextLink = static_cast<ExpandRouteLink*>(nextLink->suc()); 
   }

   // Concatenate (without spoiling the names set)
   prevLink = static_cast<ExpandRouteLink*>(m_routeList->first());
   curLink = static_cast<ExpandRouteLink*>(prevLink->suc());
   nextLink = static_cast<ExpandRouteLink*>(curLink->suc());
   while (nextLink != NULL) {

      // Is current link a roundabout.
      bool roundabout = false;
      if ((curLink->m_item->getItemType() == ItemTypes::streetSegmentItem) && 
          (((static_cast<StreetSegmentItem*>
             (curLink->m_item))->isRoundabout()) ||
           ((static_cast<StreetSegmentItem*> 
             (curLink->m_item))->isRoundaboutish()))){
         roundabout = true;
      }

      // Print names if: 
      //    the names differ
      //    If it is an ahead turn, and removeAheadIfNameDiffer = false
      //    the transportation type changes
      // However don't print names if travelling through a roundabout.
      
      bool toPrintName = ( (!removeAheadIfNameDiffer) &&
                           !curLink->noName() &&
                           !namePresent(prevLink, curLink->m_stringCode)
                           // !sameName(prevLink, curLink)
                           );

      mc2dbg8 << "removeAheadIfNameDiffer = " << (int)removeAheadIfNameDiffer
              << endl;
      mc2dbg8 << "toPrintName = " << (int)toPrintName << endl;
      
      // Mergable if 
      //    - both are of same transportation type
      //    - special case for navigator:
      // <Removing this for now - map atributes are to unreliable >
      //        - merge unless both curLink and nextLink are streetSegmentItems
      //          and of different ControlledAccess. 
      bool mergable =
         ((curLink->m_transportation == nextLink->m_transportation) );
      
      bool streetSegments = ((prevLink->m_item->getItemType() ==
                              ItemTypes::streetSegmentItem) &&
                             (curLink->m_item->getItemType() ==
                              ItemTypes::streetSegmentItem));

      if(( mergable && !(routeType & ROUTE_TYPE_GFX_TRISS)) &&
         (((curLink->m_turnDescription == StringTable::FOLLOWROAD_TURN) &&
           (!toPrintName || commonName(prevLink, curLink) || !streetSegments) &&
           (!m_noConcatinate ||(curLink->getNbrPossibleTurns() == 0))) ||
          (roundabout) ||
          ((!toPrintName) && !m_noConcatinate &&
           (curLink->m_turnDescription == StringTable::AHEAD_TURN)))
         )
      {

         if( nameChangeAsWaypoint && !curLink->noName() && !roundabout &&
            !namePresent(prevLink, curLink->m_stringCode)){
            // Do not remove link but change turn to TURN_DESC_STATE_CHANGE
            curLink->m_turnDescription = TURN_DESC_STATE_CHANGE;
            mc2dbg4 << "Changing turn to TURN_DESC_STATE_CHANGE " << endl;

            // Move to next.
            prevLink = static_cast<ExpandRouteLink*>(prevLink->suc());
         } else {
            
            // Remove the link
            DEBUG4(
               mc2dbg << "   REMOVING LINK curItem=" << curLink->m_streetName
               << ": turn =" << (int)curLink->m_turnDescription << endl;
               );
            // Merge curLink into prevLink
            m_routeList->concatinateLinks(prevLink, true);
         }
         
      } else{
        prevLink = static_cast<ExpandRouteLink*>(prevLink->suc());
        DEBUG4(
           mc2dbg << " Keeping link curItem="<< curLink->m_streetName
             << ": turn =" << (int)curLink->m_turnDescription << endl;
           );
        if(m_noConcatinate &&
           (curLink->m_turnDescription == StringTable::FOLLOWROAD_TURN)){
           curLink->m_turnDescription = StringTable::AHEAD_TURN;
        }
      }
      
      // Update the link-pointers
      curLink = static_cast<ExpandRouteLink*>(prevLink->suc());
      nextLink = static_cast<ExpandRouteLink*>(curLink->suc()); 
   }

   ExpandRouteLink* firstLink = static_cast<ExpandRouteLink*>
         (m_routeList->first());
   ExpandRouteLink* secondLink = static_cast<ExpandRouteLink*>
      (firstLink->suc());
   ExpandRouteLink* thirdLink = NULL;
   if(secondLink != NULL)
      thirdLink = static_cast<ExpandRouteLink*>(secondLink->suc());
   ExpandRouteLink* fourthLink = NULL;
   if(thirdLink != NULL)
      fourthLink = static_cast<ExpandRouteLink*>(thirdLink->suc());
 
   while (thirdLink != NULL) {
      // Try to remove uninteresting name changes. 
      if((fourthLink != NULL) && !m_noConcatinate &&
         (((secondLink->m_turnDescription == StringTable::AHEAD_TURN) ||
          (secondLink->m_turnDescription == StringTable::FOLLOWROAD_TURN)) &&
         ((thirdLink->m_turnDescription == StringTable::AHEAD_TURN) ||
          (thirdLink->m_turnDescription == StringTable::FOLLOWROAD_TURN)) &&
         sameName(firstLink, thirdLink) && !sameName(firstLink, secondLink) &&
         (secondLink->m_dist > 10*thirdLink->m_dist) &&
            (fourthLink->m_dist > 10*thirdLink->m_dist))){
         
         mc2dbg4 << "Removing unintresting name change : "
                 << secondLink->m_streetName << endl;

         m_routeList->concatinateLinks(firstLink, true);
         // A second time is needed!
         m_routeList->concatinateLinks(firstLink, true);
      }
      // If navigator remove ahead && follow road to short segment before turn.
      else if(//navigator && 
              ((secondLink->m_turnDescription == StringTable::AHEAD_TURN) ||
               (secondLink->m_turnDescription ==
                StringTable::FOLLOWROAD_TURN)) &&
              (thirdLink->m_turnDescription != StringTable::AHEAD_TURN) &&
              (thirdLink->m_turnDescription !=
               StringTable::FOLLOWROAD_TURN) &&
              (thirdLink->m_turnDescription !=
               StringTable::DRIVE_FINALLY) &&
              (thirdLink->m_turnDescription !=
               StringTable::FINALLY_WALK) &&
              ((thirdLink->m_dist < 60) ||
               ((secondLink->m_dist > 10 * thirdLink->m_dist) &&
                (thirdLink->m_dist < 500)))){
         // Remove the name change link
         m_routeList->concatinateLinks(firstLink, true);
      }
      
      // If navigator and  ahead && follow road from short segment after
      // turn.
      else if (//navigator && 
              (firstLink->m_turnDescription != StringTable::AHEAD_TURN)&&
               (firstLink->m_turnDescription !=
                StringTable::FOLLOWROAD_TURN) &&
              (firstLink->m_turnDescription !=
                StringTable::DRIVE_START) &&
              ((secondLink->m_turnDescription == StringTable::AHEAD_TURN) ||
              (secondLink->m_turnDescription ==
               StringTable::FOLLOWROAD_TURN)) &&
              ((secondLink->m_dist < 60) ||
               ((thirdLink->m_dist > 10 * secondLink->m_dist) &&
                (secondLink->m_dist < 500)))){
         firstLink->replaceName(secondLink->m_stringCode);
         firstLink->m_allStringCodes = secondLink->m_allStringCodes;

         m_routeList->concatinateLinks(firstLink, true);
         
      }
      // remove short ramps with same name as the next road for navigators.
      else if (navigator &&
               ((firstLink->m_turnDescription
                 == StringTable::OFF_RAMP_TURN)||
                (firstLink->m_turnDescription ==
                 StringTable::LEFT_OFF_RAMP_TURN)||
                (firstLink->m_turnDescription ==
                 StringTable::RIGHT_OFF_RAMP_TURN))&&
               (secondLink->m_turnDescription
                == StringTable::ON_RAMP_TURN) &&
               sameName(firstLink, secondLink) &&
               (secondLink->m_dist < 75)){

         m_routeList->concatinateLinks(firstLink, true);
      }
      
      // Update the link-pointers
      firstLink = static_cast<ExpandRouteLink*>(firstLink->suc());
      secondLink = static_cast<ExpandRouteLink*>(firstLink->suc());
      if(secondLink != NULL){
         thirdLink = static_cast<ExpandRouteLink*>(secondLink->suc());
      } else {
         thirdLink = NULL;
      }
      if(thirdLink != NULL){
         fourthLink = static_cast<ExpandRouteLink*>(thirdLink->suc());
      } else {
         fourthLink = NULL;
      }
      
   }

   // Add name to the last two items.
   //prevLink->setNewName(getBestName(printAllNames));
   if(lastPacket)
   {
      updatePossibleNames(curLink->m_item);
      curLink->setNewName(getBestName(printAllNames));
   }
  
   // Remove the first node in the list, the "false" node that makes
   // the routing work, when you are routing between two different maps.
   if (!firstPacket) {
      prevLink = static_cast<ExpandRouteLink*> (m_routeList->first());
      curLink = static_cast<ExpandRouteLink*> (prevLink->suc());
      m_routeList->concatinateLinks(curLink, false);
   }
   else // If first packet include all names of origin.
   {
      prevLink = static_cast<ExpandRouteLink*> (m_routeList->first());
      updatePossibleNames(prevLink->m_item);
      prevLink->setNewName(getBestName(printAllNames));
   }
   
   // Concatenate the first link with the rest if this is not the
   // first packet, if the first link's turndescription isn't an
   // AHEAD_TURN.  Ie. the route does not start here and therefore 
   // we want to avoid "ahead into E6" just because we are crossing a
   // map boundary. 
   
   if (!firstPacket &&
       (((static_cast<ExpandRouteLink*> (m_routeList->first()))
         ->m_turnDescription == StringTable::AHEAD_TURN)||
        ((static_cast<ExpandRouteLink*> (m_routeList->first()))
         ->m_turnDescription == StringTable::FOLLOWROAD_TURN))) 
   {
      prevLink = static_cast<ExpandRouteLink*> (m_routeList->first());
      curLink = static_cast<ExpandRouteLink*> (prevLink->suc());
      // Merge prevLink into curLink
      m_routeList->concatinateLinks(curLink, false);
   }
   
   return (true); // XXX 
}

bool
ExpandRouteProcessor::sameName(ExpandRouteLink* prevLink,
                               ExpandRouteLink* curLink)
{
   if(((prevLink->m_stringCode == MAX_UINT32) &&
      (curLink->m_stringCode == 0)) ||
      ((prevLink->m_stringCode == 0) &&
       (curLink->m_stringCode == MAX_UINT32))){
      return true;
   }
   return (prevLink->m_stringCode == curLink->m_stringCode);
}

byte
ExpandRouteProcessor::getRoundaboutInfo(byte rbExitNumber,
                                        byte restrictedRbExit,
                                        ItemTypes::crossingkind_t crossKind,
                                        ItemTypes::transportation_t trans,
                                        bool driveOnRightSide,
                                        StringTable::stringCode& sc,
                                        byte entryC,
                                        byte exitC) 
{
#if 1
   // Disabled left/right etc instructions for now.
   // Inly instructions like "then take first exit from the roundabout".
   entryC = 0;
   exitC = 0;
   rbExitNumber = 0;
   restrictedRbExit = 0;
#endif

   if (trans == ItemTypes::drive || trans == ItemTypes::bike)
      sc = StringTable::EXIT_ROUNDABOUT_TURN;
   else if(trans == ItemTypes::walk)
      sc = StringTable::EXIT_ROUNDABOUT_TURN_WALK;
   
   if ( crossKind == ItemTypes::CROSSING_4ROUNDABOUT) {
      rbExitNumber += restrictedRbExit;
      if (driveOnRightSide) { // right side of the road
         // Modify string code
         if (rbExitNumber == 1) {
            // Right turn
            sc = StringTable::RIGHT_ROUNDABOUT_TURN;
         } else if (rbExitNumber == 2) {
            // Ahead
            sc = StringTable::AHEAD_ROUNDABOUT_TURN;
         } else if (rbExitNumber == 3) {
            // Left
            sc = StringTable::LEFT_ROUNDABOUT_TURN;
         } else if (rbExitNumber == 4) {
            sc = StringTable::U_TURN_ROUNDABOUT_TURN;
         }
      } else { // driving on left side of the road
         // Modify string code
         if (rbExitNumber == 1) {
            // Right turn
            sc = StringTable::LEFT_ROUNDABOUT_TURN;
         } else if (rbExitNumber == 2) {
            // Ahead
            sc = StringTable::AHEAD_ROUNDABOUT_TURN;
         } else if (rbExitNumber == 3) {
            // Left
            sc = StringTable::RIGHT_ROUNDABOUT_TURN;
         } else if (rbExitNumber == 4) {
            sc = StringTable::U_TURN_ROUNDABOUT_TURN;
         }
      }
   } else if((entryC != 0) && (exitC != 0) && (entryC != exitC)){
      if(((entryC == 5) && (exitC == 6))||
         ((entryC != 5) && (exitC != 6) && (((entryC + 1)%4) == exitC%4))){
         // Right turn
         sc = StringTable::RIGHT_ROUNDABOUT_TURN;
      }
      else if ((entryC != 5) && (exitC != 6) &&
               (((entryC + 2)%4) == exitC%4)){
         // Ahead
         sc = StringTable::AHEAD_ROUNDABOUT_TURN;
      }
      else if (((entryC ==3) && (exitC == 6)) ||
               ((entryC ==5) && (exitC == 2)) ||
               ((entryC != 5) && (exitC != 6) &&
                (((entryC + 3)%4) == exitC%4))){
         // Left
         sc = StringTable::LEFT_ROUNDABOUT_TURN;
      }
      else {
         mc2log << warn << " UNKNOWN ROUNDABOUT SYMMETRY COMBINATION" << endl;
      }
      
   }
   return (rbExitNumber);
}

bool 
ExpandRouteProcessor::addTransportationChanges()
{

   // Example of what's happening in this method:
   //
   //
   // Before:
   //
   // curLink          nextLink
   //
   // drive 200m       walk 100m
   // then right       then left
   // into Barav       into Getingev
   //
   // After:
   // curLink          newLink         nextLink
   //
   // drive 200m       then walk       walk 100m
   // then park car    right           then left
   // on Barav         into Barav      into Getingev
   //
   
   ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>
                              (m_routeList->first());
   if(curLink == NULL){
      mc2log << warn << "ExpRP::addTransportationChanges() routeList empty"
             << endl;
      return false;
   }
   
   ExpandRouteLink* nextLink = static_cast<ExpandRouteLink*>
                              (curLink->suc());
   while (nextLink != NULL) {
      if (curLink->m_transportation != nextLink->m_transportation) {
          
         if (curLink->m_turnDescription != StringTable::AHEAD_TURN ||
             (curLink->m_turnDescription == StringTable::AHEAD_TURN &&
              curLink->m_transportation == ItemTypes::drive &&
              nextLink->m_transportation == ItemTypes::walk) ) 
         {
            ExpandRouteLink* newLink = new ExpandRouteLink;
            newLink->m_turnDescription = curLink->m_turnDescription;
            newLink->m_transportation = nextLink->m_transportation;
            newLink->setNewName(curLink->m_streetName);
            newLink->follow(curLink);

         }
         // Add the transportation change (for now, parking only supported)
         if ((curLink->m_transportation == ItemTypes::drive) &&
             (nextLink->m_transportation == ItemTypes::walk)) {
             curLink->m_turnDescription = StringTable::PARK_CAR;
            // We should park on the previous road, not this one since
            // it is not allowed to drive in here.
            // The previous link must exist (can't be NULL)
            /*     curLink->setNewName(
               (static_cast<ExpandRouteLink*> 
               (curLink->pred()))->m_streetName); */
         }
         else if ((curLink->m_transportation == ItemTypes::bike) &&
                  (nextLink->m_transportation == ItemTypes::walk)) {
            curLink->m_turnDescription = StringTable::PARK_BIKE;
         }
      }
      // Update link pointers
      curLink = nextLink;
      nextLink = static_cast<ExpandRouteLink*> (curLink->suc());
   }

   return (true);
}

bool
ExpandRouteProcessor::updateLandmarksBeforeSelect()
{
   ExpandRouteLink* curLink =
      static_cast <ExpandRouteLink*>(m_routeList->first());

   while (curLink != NULL) {
      if (curLink->m_item != NULL) {
         uint32 buaID = m_map->getRegionID(
               curLink->m_item, ItemTypes::builtUpAreaItem);
         mc2dbg8 << "Routeable item " << curLink->m_streetName << "("
                 << curLink->m_dist << ") bualocation "
                 << m_map->getFirstItemName(buaID) << endl;
         
         LandmarkLink* curLM = 
            static_cast<LandmarkLink*>(curLink->m_landmarks->first());
         while (curLM != NULL) {
            
            mc2dbg8 << "Landmark: " << curLM->m_lmdesc.itemID << ":"
                    << m_map->getFirstItemName(curLM->m_lmdesc.itemID)
                    << ", loc=" << int(curLM->m_lmdesc.location)
                    << ", type=" << int(curLM->m_lmdesc.type) << endl;
            
            if ((curLM->m_lmdesc.type == ItemTypes::builtUpAreaLM) &&
                (curLM->m_lmdesc.location == ItemTypes::pass) &&
                (curLM->m_lmdesc.itemID == buaID)) {
               mc2dbg1 << "PASS-bua when bualocation == buaLM (" 
                       << m_map->getFirstItemName(buaID)
                       << "), changing to \"into\"" << endl;
               curLM->m_lmdesc.location = ItemTypes::into;
            }
            if ((curLM->m_lmdesc.type == ItemTypes::builtUpAreaLM) &&
                (curLM->m_lmdesc.location == ItemTypes::into) &&
                (curLM->m_lmdesc.itemID != buaID)) {
               mc2dbg1 << "INTO-bua when bualocation != buaLM ("
                       << m_map->getFirstItemName(curLM->m_lmdesc.itemID)
                       << "!=" << m_map->getFirstItemName(buaID)
                       << "), changing to \"pass\"" << endl;
               curLM->m_lmdesc.location = ItemTypes::pass;
            }
            
            curLM = static_cast<LandmarkLink*>(curLM->suc());
         }
      }
      
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   }
   return (true);
}

bool
ExpandRouteProcessor::selectLandmarks()
{
   mc2dbg1 << "selectLandmarks() - to remove unwanted LMs." << endl;

   ExpandRouteLink* prevLink = 
      static_cast <ExpandRouteLink*>(m_routeList->first());
   if(prevLink == NULL){
      mc2log << warn << "ExpRP::selectLandmarks() routeList empty" << endl;
      return false;
   }
   
   ExpandRouteLink* curLink;
   // The first link (= startlink) never has any landmarks, go to next
   if (prevLink->m_turnDescription == StringTable::DRIVE_START) {
      curLink = static_cast<ExpandRouteLink*>(prevLink->suc());
   } else {
      curLink = prevLink;
   }

   //startBua and endBua
   uint32 startBua = m_map->getRegionID(
         prevLink->m_item, ItemTypes::builtUpAreaItem);
   uint32 endBua = m_map->getRegionID(
         static_cast <ExpandRouteLink*>(m_routeList->last())->m_item,
         ItemTypes::builtUpAreaItem);
   mc2dbg4 << " startBua=" << m_map->getFirstItemName(startBua)
           << " endBua=" << m_map->getFirstItemName(endBua) << endl;

   //routing in the overviewmap?
   bool overviewMap = (dynamic_cast<OverviewMap*>(m_map) != NULL);
   if (overviewMap){
      mc2dbg4 << " routing in overviewmap" << endl;
   }
   
   uint32 longLinkDist = 0;   // distance until next turn !ahead
   uint32 nbrLinks = 0;       // untill next turn !ahead
   uint32 nbrLongLMs = 0;     // number of LMs along longLink
   uint32 passedLinkDist = 0;

   uint32 nbrLMsPerImportance[5];
   uint32 wantedMaxImportance = 4;
   uint32 nbrWantedLMs = 0;
   int32 nbrVacantLMs = 0;
   int32 nbrSkipLMs = 0;
   bool longLinkHasEndLM = false;
   
   while (curLink != NULL) {

      Vector endLMs;

      if (nbrLinks == 0) {
         // See how many links there are until the next turndescription 
         // that is !ahead, also count the total dist there (longLinkDist),
         // also count nbr landmarks on the longlink.
         
         ExpandRouteLink* testLink = curLink;
         passedLinkDist = 0;
         for (uint32 i = 0; i < 5; i++) {
            nbrLMsPerImportance[i] = 0;
         }
         while (testLink != NULL) {
            if(nbrLinks == 0){
               longLinkDist = curLink->m_dist;
               nbrLongLMs = curLink->m_landmarks->cardinal();
            } else {
               longLinkDist += testLink->m_dist;
               nbrLongLMs += testLink->m_landmarks->cardinal();
            }
            nbrLinks +=1;

            LandmarkLink* lmlink = 
               static_cast<LandmarkLink*>(testLink->m_landmarks->first());
            while (lmlink != NULL) {
               if (lmlink->m_lmdesc.location == ItemTypes::pass)
                  nbrLMsPerImportance[lmlink->m_lmdesc.importance]++;
               lmlink = static_cast<LandmarkLink*>(lmlink->suc());
            }

            if ((testLink->m_turnDescription == StringTable::AHEAD_TURN) ||
               (testLink->m_turnDescription == StringTable::FOLLOWROAD_TURN)) {
               testLink = static_cast<ExpandRouteLink*>(testLink->suc());
               // Here we can get cross the map border. Ok, then we don't
               // have any endLM - since the route continues on the other map
               longLinkHasEndLM = false;
            } else {
               LandmarkLink* testLinkEndLM = 
                  static_cast<LandmarkLink*>(testLink->m_landmarks->last());
               if (testLinkEndLM != NULL) {
                  longLinkHasEndLM = 
                     ((testLinkEndLM->m_lmDist == int32(testLink->m_dist)) ||
                      (testLinkEndLM->m_lmdesc.location == ItemTypes::in) ||
                      (testLinkEndLM->m_lmdesc.location == ItemTypes::into));
                  mc2dbg4 << "lastlmdist=" << testLinkEndLM->m_lmDist
                       << ", lastlinkdist=" << testLink->m_dist
                       << ", longLinkHasEndLM: " << longLinkHasEndLM << " "
                       << m_map->getFirstItemName(testLinkEndLM->m_lmdesc.itemID)
                       << " loc=" << int(testLinkEndLM->m_lmdesc.location)
                       << endl;
               } else {
                  longLinkHasEndLM = false;
               }
               testLink = NULL;
            }
         }
         
         if (nbrLongLMs > 0) {
          
            // How many _pass_LMs do we want
            if (longLinkDist < 50000) 
               nbrWantedLMs = 2;
            else if (longLinkDist < 200000)
               nbrWantedLMs = 3;
            else if (longLinkDist < 500000)
               nbrWantedLMs = 4;
            else
               nbrWantedLMs = 5;
            // Reduce
            if (longLinkHasEndLM)
               nbrWantedLMs -= 1;
            if (overviewMap)
               nbrWantedLMs -= 1;
            // if within same city startbua=endbua, increase ??? XXX

            // Which importance
            uint32 nbr = 0;
            uint32 imp = 0;
            bool cont = true;
            while (cont && (imp < 5)) {
               nbr += nbrLMsPerImportance[imp];
               if (nbr > nbrWantedLMs)
                  cont = false;
               else 
                  imp++;
            }
            if (imp < 2) {
               // keep 0and1-LMs, 
               // XXX as long as the builtUpAreaType is incorrect: the 
               // bbox-area is misleading.. wait for population (then keep 0)
               wantedMaxImportance = imp;
               nbrSkipLMs = nbr - nbrWantedLMs;
               imp++; // just to get it right when calc nbrWantedLMs
               nbr += nbrLMsPerImportance[imp];
            } else {
               wantedMaxImportance = imp - 1;
            }

            // Any vacant LMs? (to fill the nbrWantedLMs quote)
            if (cont)
               // we have reached imp=5, i.e. will include all LM:s
               nbrVacantLMs = 0;
            else
               nbrVacantLMs = nbrWantedLMs - (nbr - nbrLMsPerImportance[imp]);
         
            // Some debugging
            DEBUG4(
            mc2dbg << "nbrLinks=" << nbrLinks << ", nbrLMs=" 
                    << nbrLongLMs << ", longLinkDist=" << longLinkDist 
                    << " => max " << nbrWantedLMs << " \"pass\"-LMs"
                    << endl;

            mc2dbg << "nbr \"pass\"-LMs:";
            for (uint32 i = 0; i < 5; i++) {
               mc2dbg << " imp" << i << "=" << nbrLMsPerImportance[i];
            }
            mc2dbg << endl;
            mc2dbg << "will keep imortance " << wantedMaxImportance;
            if (cont)
               mc2dbg << ", all \"pass\"-LMs: " << nbr;
            else if (nbrSkipLMs > 0)
               mc2dbg << ", " << nbr - nbrLMsPerImportance[imp]
                      << " LMs -> will skip first " << nbrSkipLMs;
            else
               mc2dbg << ", " << nbr - nbrLMsPerImportance[imp]
                      << " LMs -> will miss " << nbrVacantLMs;
           
            if (longLinkHasEndLM)
               mc2dbg << ", longlink has endLM.";
            mc2dbg << endl;
            );
             
         }
      }

      if (nbrLongLMs > 0) {

         // Loop over the landmarks in this route link 
         LandmarkLink* lmLink = 
            static_cast<LandmarkLink*>(curLink->m_landmarks->first());
         uint32 lmNbr = 0;
         while (lmLink != NULL) {
            if(!lmLink->isTraffic()){
               float64 offset = 
                  (passedLinkDist + lmLink->m_lmDist) / float64(longLinkDist);
               uint32 importance = lmLink->m_lmdesc.importance;
               
               mc2dbg4 << " m_dist=" << lmLink->m_lmDist << " passed=" 
                       << passedLinkDist << " => lm offset=" << offset 
                       << ", imp=" << importance << endl;
               
               const char* lmName = lmLink->getLMName();
               
               // should the landmark be kept or removed?
               bool remove = false;
               
               if ( (lmLink->m_lmdesc.type == ItemTypes::builtUpAreaLM) &&
                    (lmLink->m_lmdesc.location == ItemTypes::into)) {
                  mc2dbg1 << " " << lmName << "(" << importance << ")"
                          << " keeping \"into\"-bua" << endl;
               }
               else if ( (offset == 1) ||
                         ((nbrLinks > 1) && 
                          (int32(curLink->m_dist) == lmLink->m_lmDist)) ) {
                  // The landmark is located at the end of curLink,
                  // if several lm's, only one should be kept. 
                  // Add to endLM vector.
                  endLMs.addLast(lmNbr);
                  lmLink->setEndLM(true);
                  mc2dbg1 << " " << lmName << "(" << importance << ")"
                          << " added to endLMs (offset=" << offset << ")"
                          << endl;
               } 
               else if (( (curLink->m_turnDescription ==
                           StringTable::EXIT_ROUNDABOUT_TURN) ||
                          (curLink->m_turnDescription ==
                           StringTable::AHEAD_ROUNDABOUT_TURN) ||
                          (curLink->m_turnDescription ==
                           StringTable::RIGHT_ROUNDABOUT_TURN) ||
                          (curLink->m_turnDescription ==
                           StringTable::LEFT_ROUNDABOUT_TURN) ) &&
                        (lmLink->m_lmdesc.location != ItemTypes::pass) &&
                        ((longLinkDist - passedLinkDist - lmLink->m_lmDist) 
                         < 200 )  ) {
                  // The landmark is located at a roundabout, offset != 1
                  // but at the end of longlink, so add to endLM vector.
                  endLMs.addLast(lmNbr);
                  lmLink->setEndLM(true);
                  mc2dbg1 << " " << lmName << "(" << importance << ")"
                          << " added to endLMs (offset=" << offset 
                          << ", rbt)" << endl;
               } 
               else if (offset < 1) {
                  
                  mc2dbg1 << " " << lmName << "(" << importance << ")"
                          << " along curLink (" << offset << ":"
                          << 1.2 * importance / 5.0 << ")"
                          << (int)lmLink->isTraffic();
                  
                  if (lmLink->m_lmdesc.importance > wantedMaxImportance) {
                     // The LM is not important enough
                     
                     if (!longLinkHasEndLM && (nbrVacantLMs > 0) &&
                         (wantedMaxImportance > 2) && (lmLink->suc() == NULL) &&
                         (offset > (1.2 * importance / 5.0)) ) {
                        // keep this last one
                        mc2dbg1 << ", keeping last LM";
                     } else {
                     remove = true;
                     mc2dbg1 << ", removing";
                     }
                  } else if (longLinkHasEndLM && (longLinkDist < 30000) && 
                             ((startBua == MAX_UINT32) || (endBua == MAX_UINT32) || 
                              (startBua != endBua)) && 
                             (offset < (1.2 * importance / 5.0)) ) {
                     //remove unnecessary lm if not routing within a city
                     mc2dbg1 << ", removing(ll short + endLM)";
                     remove = true; 
                  } else if (nbrSkipLMs > 0) {
                     // too many LMs of high importance, skip the first onces
                  remove = true;
                  mc2dbg1 << ", removing(skipLM)";
                  nbrSkipLMs -= 1;
                  }
                  
                  mc2dbg1 << endl;
               
               }
               
               if (remove) {
                  LandmarkLink* deleteLink = lmLink;
                  lmLink = static_cast<LandmarkLink*>(deleteLink->suc());
               deleteLink->out();
               } else {
                  lmLink = static_cast<LandmarkLink*>(lmLink->suc());
                  lmNbr += 1;
               }
               
            } else {
               if((curLink->suc() != NULL) &&
                  (int32(curLink->m_dist) == lmLink->m_lmDist )){
                  //cerr << "*********************************" << endl
                  //     << "Traffic landmark has same dist as link" << endl
                  //     << "*********************************" << endl;
                  
                  lmLink->m_lmDist = 0;
                  lmLink->out();
                  lmLink->into(static_cast<ExpandRouteLink*>
                               (curLink->suc())->m_landmarks);
                  
               }
               
               lmLink = static_cast<LandmarkLink*>(lmLink->suc());
            }
         }
         

         if (endLMs.getSize() > 1) {
            // Several landmarks at the end of curLink, only the most 
            // important one should be kept ("in"-bua should always be kept).
            uint32 endLMsize = endLMs.getSize();
            mc2dbg1 << "To remove not important endLMs (" << endLMsize 
                    << " candidates)" << endl;
            // first=lmNbr, second=importance, third=lmtype
            // (of the so far most important end lm)
            uint32 mostImportant[3];
            mostImportant[0] = MAX_UINT32;
            mostImportant[1] = MAX_UINT32;
            mostImportant[2] = MAX_UINT32;

            // Find the most important endLM
            lmLink = static_cast<LandmarkLink*>(curLink->m_landmarks->first());
            uint32 n = 0;
            while (lmLink != NULL) {
               if ((endLMs.binarySearch(n) != MAX_UINT32) &&
                   ( (lmLink->m_lmdesc.importance < mostImportant[1]) || 
                     ((lmLink->m_lmdesc.importance == mostImportant[1]) &&
                      (uint32(lmLink->m_lmdesc.type) < mostImportant[2])) ) ) {
                  //the LM is more important or of "better" lmtype
                  mostImportant[0] = n;
                  mostImportant[1] = lmLink->m_lmdesc.importance;
                  mostImportant[2] = lmLink->m_lmdesc.type;
               }
               lmLink = static_cast<LandmarkLink*>(lmLink->suc());
               n++;
            }

            // Remove the ones that are not_most_important
            lmLink = static_cast<LandmarkLink*>(curLink->m_landmarks->first());
            n = 0;
            uint32 i = 0;
            bool remove;
            while (lmLink != NULL) {
               remove = false;
               if ((i < endLMsize) && (n == endLMs.getElementAt(i))) {
                  //one of the endLMs
                  if (n != mostImportant[0]) {
                     //remove
                     remove = true;
                     mc2dbg1 << "  removing " 
                             << m_map->getFirstItemName(lmLink->m_lmdesc.itemID) 
                             << endl;
                  }
                  i++;
               }
               n++;
               if (remove) {
                  LandmarkLink* deleteLink = lmLink;
                  lmLink = static_cast<LandmarkLink*>(deleteLink->suc());
                  deleteLink->out();
               } else {
                  lmLink = static_cast<LandmarkLink*>(lmLink->suc());
               }
            }
         }
      } // else no landmark along the "longLink"

      nbrLinks -= 1;    // go to next link in the route
      passedLinkDist += curLink->m_dist;
      prevLink = curLink;
      curLink = static_cast<ExpandRouteLink*>(prevLink->suc());
   }

   return true;
}

bool
ExpandRouteProcessor::updateLandmarks()
{
   mc2dbg2 << "Updating landmarks." << endl;
   
   // Exchanging pass->at for endLMs when the turndescription is ahead, 
   // e.g. when the street name changes.
   
   ExpandRouteLink* curLink = 
      static_cast <ExpandRouteLink*>(m_routeList->first());
   if(curLink == NULL){
      mc2log << warn << "ExpRP::updateLandmarks() routeList empty" << endl;
      return false;
   }
   
   while (curLink != NULL) {
      
      if (((curLink->m_turnDescription == StringTable::AHEAD_TURN) ||
           (curLink->m_turnDescription == StringTable::FOLLOWROAD_TURN)) &&
          (!curLink->m_landmarks->empty())) {
         LandmarkLink* lmLink = 
            static_cast<LandmarkLink*>(curLink->m_landmarks->first());
         while (lmLink != NULL) {
            //for endLMs, use "at" instead of "pass"
            if ((lmLink->m_lmdesc.location == ItemTypes::pass) &&
                (lmLink->isEndLM()) &&
                (lmLink->m_lmdesc.type != ItemTypes::builtUpAreaLM)) {
               mc2dbg4 << "Exchanging location \"pass\" -> \"at\" for LM:"
                       << lmLink->getLMName() << " when routing from "
                       << m_map->getFirstItemName( static_cast <ExpandRouteLink*>
                             (curLink->pred())->m_item ) << " to "
                       << m_map->getFirstItemName(curLink->m_item) << endl;
               lmLink->m_lmdesc.location = ItemTypes::at;
            }
            
            lmLink = static_cast<LandmarkLink*>(lmLink->suc());
         }
         
      }
      
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   }

   // Move a landmark to the previous ExpandRouteLink if:
   // - the m_lmDist is a larger negative value than the length of 
   //   its ERLink. Then also update the lmDist to be counted from the 
   //   start of prevLink (e.g. routing Hyllinge -> Klippan, exiting 
   //   E4 in Åstorp)
   // - the m_lmdist is a negative value and the location is
   //   into for buaLMs.
   // - the m_lmDist is = 0 and the location is pass/into for buaLMs
   //
   curLink = static_cast<ExpandRouteLink*>(m_routeList->last());
   ExpandRouteLink* prevLink = static_cast<ExpandRouteLink*>(curLink->pred());
   
   while (prevLink != NULL) {
      
      if (!curLink->m_landmarks->empty()) {
         LandmarkLink* curLM = 
            static_cast<LandmarkLink*>(curLink->m_landmarks->first());

         while (curLM != NULL) {
            if ( (curLM->m_lmDist < 0) &&
                 (abs(curLM->m_lmDist) > int(curLink->m_dist)) ) {
               int32 tmpdist = curLM->m_lmDist;
               curLM->out();
               curLM->m_lmDist += (prevLink->m_dist + curLink->m_dist);
               curLM->into(prevLink->m_landmarks);
               mc2dbg1 << "Moving LM " << curLM->getLMName() 
                       << " to previous link, old lmdist=" << tmpdist
                       << " new=" << curLM->m_lmDist << endl;
            }

            // the route ended in a bua without any turn
            else if ( (curLM->m_lmDist < 0) &&
                      (curLM->m_lmdesc.location == ItemTypes::into) &&
                      (curLM->m_lmdesc.type == ItemTypes::builtUpAreaLM)) {
               curLM->out();
               curLM->m_lmDist = prevLink->m_dist;
               curLM->into(prevLink->m_landmarks);
               mc2dbg1 << "Moving into-bua " << curLM->getLMName()
                       << " with negative lmdist to previous link." << endl;
            }

            // there is a new ahead-routelink on the bua boarder
            else if ( (curLM->m_lmDist == 0) &&
                      ((curLM->m_lmdesc.location == ItemTypes::pass) ||
                       (curLM->m_lmdesc.location == ItemTypes::into)) &&
                      (curLM->m_lmdesc.type == ItemTypes::builtUpAreaLM) ) {
               curLM->out();
               curLM->m_lmDist = prevLink->m_dist;
               curLM->m_lmdesc.location = ItemTypes::into;
               curLM->into(prevLink->m_landmarks);
               mc2dbg1 << "Moving buaLM " << curLM->getLMName()
                       << " with lmdist=0 to prev link as into-bua." << endl;
            }

            curLM = static_cast<LandmarkLink*>(curLM->suc());
         }
      }

      curLink = prevLink;
      prevLink = static_cast<ExpandRouteLink*>(curLink->pred());
   }

   // Sort the landmarklist, so that "pass"- and "into"-buas are placed
   // before any other landmarks in that bua.
   // Compare the m_lmDist, all landmarks with dist > bua_dist (the dist
   // of the bualandmarks) should be placed after the bua(s).

   curLink = static_cast <ExpandRouteLink*>(m_routeList->first());
   while (curLink != NULL) {

      if (!curLink->m_landmarks->empty()) {
         LandmarkLink* curLM = 
            static_cast<LandmarkLink*>(curLink->m_landmarks->last());
         LandmarkLink* predLM = static_cast<LandmarkLink*>(curLM->pred());
         
         while (predLM != NULL) {
            if (curLM->m_lmdesc.type == ItemTypes::builtUpAreaLM) {
               if (predLM->m_lmDist > curLM->m_lmDist) {
                  // put curLM in front of predLM
                  mc2dbg4 << "Moving " << curLM->getLMName() 
                          << " ahead of " << predLM->getLMName() << endl;
                  curLM->out();
                  curLM->precede(predLM);
                  //update predLM
                  predLM = static_cast<LandmarkLink*>(curLM->pred());
               } else {
                  curLM = predLM;
                  predLM =  static_cast<LandmarkLink*>(curLM->pred());
                  
               }
            } else {
               // check if there are any other buas earlier in the LM-list
               curLM = predLM;
               predLM =  static_cast<LandmarkLink*>(curLM->pred());
            }
         }
         
      }
      
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   }

   
   return true;
}

bool
ExpandRouteProcessor::createPassRoadLandmarks(int maxNbrStreets)
{
   
   ExpandRouteLink* curLink = static_cast <ExpandRouteLink*>
      (m_routeList->first());
   StringTable::languageCode lc = StringTable::languageCode(
      m_preferedLanguages.getElementAt(0));
   int nbrStreets;
   while (curLink != NULL) {
      if(!curLink->m_passedStreets.empty() &&
         (curLink->m_turnNumber > 1)){

         // Dont allow more streets than the turn number -1
         if(maxNbrStreets+1 > curLink->m_turnNumber){
            nbrStreets = curLink->m_turnNumber -1;
         } else {
            nbrStreets = maxNbrStreets;
         }
         

         ItemTypes::lmdescription_t desc;
         desc.itemID = curLink->m_nodeID;
         desc.importance = 0;
         desc.location = ItemTypes::pass;
         desc.type = ItemTypes::passedStreetLM;
         uint32 maxSizeOfNames = (20*nbrStreets)-1;
         
         char* nameOfStreets = new char[maxSizeOfNames+1];
         ScopedArray<char> nameOfStreetsPtr( nameOfStreets );

         int trueNameFound   = 0; // dont create LM if no name found.
         if(curLink->m_turnDescription == StringTable::LEFT_TURN){
            desc.side = SearchTypes::left_side;
         } else {
            desc.side = SearchTypes::right_side;
         }
         
         uint32 i = curLink->m_passedStreets.size()-1;
         list<char*>::iterator PI =
            curLink->m_passedStreets.end();
         PI--;
            
         uint32 writeIndex = 0;
         list<char*>::iterator writeI = curLink->m_passedStreets.end();
         list<char*>::iterator testI;
         int namesFound = 0;
         bool done = false;
         uint8 sizeOfNames = 0;
         do {
            if((*PI) == NULL){
               //if(curLink->m_passedStreetsLeft[i].name == NULL){
               if(namesFound > 0){
                  done = true; // quit
                  mc2dbg4 << "No steetname exit" << endl;
               } else {
                  namesFound = 1;
                  sizeOfNames += strlen(
                     StringTable::getString(
                        StringTable::LM_PASS_STREET_NONAME, lc));
               }
            } else {
               //sizeOfNames +=strlen(curLink->m_passedStreetsLeft[i].name);
               sizeOfNames += strlen(*PI);
               trueNameFound += 1;
               if(namesFound == 1){
                  //sizeOfNames += 3; // " & "
                  sizeOfNames += 2 + strlen(
                     StringTable::getString(
                        StringTable::AND_SIGN, lc));;
               }
               else if(namesFound > 1)
                  sizeOfNames += 2; // ", "
               namesFound++;
            }
            if(!done && (sizeOfNames <= maxSizeOfNames)){
               writeI = PI;
               writeIndex = i;
            } else {
               mc2dbg4 << " sizeOfNames " << int(sizeOfNames)
                       << " maxSizeOfNames " << int(maxSizeOfNames) << endl;
               done = true;
               trueNameFound -= 1;
               mc2dbg4 << "Name : " << (*PI) << endl
                       << "Name to long, reducing number of found names."
                       << endl;
            }
            if(PI == curLink->m_passedStreets.begin()){
               //if(i == 0){
               mc2dbg4 << "i reached zilch!" << endl;
               done = true;
            } else {
               PI--;
               i--;
            }
               
         } while((i+nbrStreets >= curLink->m_passedStreets.size())
                 && !done);
         mc2dbg4 << "trueNameFound :" << trueNameFound << endl;
         bool firstName = true;
         mc2dbg4 << "writeIndex= " << writeIndex << ", nbr streets = "
                 << nbrStreets << ", size() "
                 << curLink->m_passedStreets.size() << endl
                 << "SizeOfNames = " << (int)sizeOfNames << endl;
         if(trueNameFound > 0){
            for(PI = writeI;
                PI != curLink->m_passedStreets.end(); PI++){
               if(!firstName ){
                  testI = PI;
                  testI++;
                  if(testI == curLink->m_passedStreets.end()){
                     //if(j == curLink->m_passedStreetsLeft.size()-1){
                     strcat(nameOfStreets, " ");
                     strcat(nameOfStreets,
                            StringTable::getString(
                               StringTable::AND_SIGN, lc));
                     strcat(nameOfStreets, " ");
                  } else {
                     strcat(nameOfStreets, ", ");
                  }
                  if((*PI) == NULL){
                     //if(curLink->m_passedStreetsLeft[j].name == NULL){
                     strcat(nameOfStreets,
                            StringTable::getString(
                               StringTable::LM_PASS_STREET_NONAME, lc));
                  } else {
                     strcat(nameOfStreets,(*PI));
                               //curLink->m_passedStreetsLeft[j].name);
                  }
               } else {
                  strcpy(nameOfStreets, (*PI));
                            //curLink->m_passedStreetsLeft[j].name);
                  firstName = false;
               }  
            }
         }
         if(trueNameFound > 0){
            LandmarkLink* passedStreets = new LandmarkLink(desc, 0); 
            mc2dbg2 << "Setting name :" << nameOfStreets 
                    << " to passed street landmark"  << endl; 
            passedStreets->setLMName(nameOfStreets); 
            passedStreets->into(curLink->m_landmarks); 
         }  
      } 
      curLink = static_cast<ExpandRouteLink*>(curLink->suc()); 
   } 
   return true; 
}

bool
ExpandRouteProcessor::createTrafficLandmarks(const ExpandRouteRequestPacket* req){

   
   //  loop
   //    Extract a disturbance.
   //    Find node in route.
   //    add Landmark.
   //  remove where a disturbance (unique ID) lies on more than 2 segments
   int DIST_STR_NAME_LENGTH = 20;
   uint16 nbrOfDist = req->getNbrTrafficInfo();
   mc2dbg << " ***************************************************" << endl
           << "Found " << nbrOfDist << " disturbance descriptions" << endl;
   if(nbrOfDist == 0){
      return true;
   }
   // Extract the info
   int pos = 0;
   uint32 nodeID, distID, distNodeID;
   bool split, merge;
   TrafficDataTypes::disturbanceType type;
   char* text;
   ExpandRouteLink* curLink;
   for(uint16 i = 0 ; i < nbrOfDist ; i++){
      req->getTrafficInfo(pos, nodeID, split, distNodeID, merge,
                          distID, type, text);
      mc2dbg4 << "Found landmark of node " << nodeID << ", type "
           << (int)type << ", id : " << distID  << ", text :"
                  << text << ": split " << split << " distNodeID " 
                  << distNodeID << " merge " << merge << endl;
      curLink = static_cast <ExpandRouteLink*>(m_routeList->first());
      while((curLink != NULL) && (curLink->m_nodeID != nodeID)){
         curLink = static_cast<ExpandRouteLink*>(curLink->suc()); 
      }
      if(curLink != NULL){
         // Found the node.
         if((curLink == static_cast <ExpandRouteLink*>(m_routeList->first()))
            && (curLink->suc() != NULL)){
            curLink = static_cast<ExpandRouteLink*>(curLink->suc()); 
         }
         
            
         // Create the Landmark and add it here.
         ItemTypes::lmdescription_t desc;
         desc.itemID     = distID;
         desc.importance = 0;
         desc.side       = SearchTypes::undefined_side;
         desc.location   = ItemTypes::pass;
        // cerr << " TrafficDataType : " << int(type)  << endl;
         switch(type){
            case TrafficDataTypes::Accident :
               desc.type = ItemTypes::accidentLM;
               break;
            case TrafficDataTypes::RoadWorks :
               desc.type = ItemTypes::roadWorkLM;
               break;
            case TrafficDataTypes::Camera :
               desc.type = ItemTypes::cameraLM;
               //cerr << " Found camera " << endl;
               break;
            case TrafficDataTypes::Police :
               desc.type = ItemTypes::policeLM;
               //cerr << " Found camera " << endl;
               break;
            case TrafficDataTypes::SpeedTrap :
               desc.type = ItemTypes::speedTrapLM;
               //cerr << " Found camera " << endl;
               break;
            case TrafficDataTypes::SnowIceEquipment :
            case TrafficDataTypes::SnowOnTheRoad :
            case TrafficDataTypes::WeatherData :
            case TrafficDataTypes::Wind :
            case TrafficDataTypes::FogSmokeDust :
            case TrafficDataTypes::SkidHazards :
               desc.type = ItemTypes::weatherLM;
               break;
            case TrafficDataTypes::UserDefinedCamera:
               desc.type = ItemTypes::userDefinedCameraLM;
               break;
               
            default :
               desc.type = ItemTypes::trafficOtherLM;
               //cerr << " Found traffic - other " << endl;
               break;
         }
         
         ScopedArray<char> disturbanceText( new char[300] );
         
         StringUtility::utf8lcpy( disturbanceText.get(), text, 298 );
         //strcat(disturbanceText, "\0");
         
         
         LandmarkLink* lm = new LandmarkLink(desc, 0);
         mc2dbg4 << "Adding dist landmark of type " << (int)desc.type
                 << ", id : " << desc.itemID  
                 << ", text :" << disturbanceText.get()
                 << ":" << endl;

         lm->setLMName( disturbanceText.get() );
         lm->setIsTraffic();
         lm->m_distNodeID = distNodeID;
         
         if(split){
            lm->setDetour();
            lm->setIsStart(true);
            lm->setIsStop(false);
         }
         if(merge){
            lm->setDetour();
            lm->setIsStart(false);
            lm->setIsStop(true);
         }
         lm->m_distNodeID = distNodeID;
         lm->into(curLink->m_landmarks); 
         
      } else {
         mc2log << error
                << "The node (id:" << nodeID
                << ") of a DisturbanceDescription was not found! "
                << endl << "    type : " << (int)type
                << ", text :" << text << endl;
      }
   }
   // Go through all links and remove all doubles(distID)
   bool useEnds = ((req->getType() & ROUTE_TYPE_NAVIGATOR)!=0 );
   set<uint32> usedIds;
   map<uint32, LandmarkLink*> lastLM;
   map<uint32, LandmarkLink*>::iterator mi;
   int lmCount = 0;
   curLink = static_cast <ExpandRouteLink*>(m_routeList->first());
   while(curLink != NULL){
      LandmarkLink* testLM = static_cast <LandmarkLink*>
         (curLink->m_landmarks->first());
      
      while (testLM != NULL){
         lmCount++;
         bool removeTest = false;
         if(testLM->isTraffic() && !testLM->isDetour()){
            uint32 distID = testLM->m_lmdesc.itemID;
            bool first = (usedIds.find(distID) == usedIds.end());
            if(first){
               //mc2dbg4
               //cerr << endl << "Found new disturbanceID : " << distID;
               usedIds.insert(distID);
            } else {
               //mc2dbg4
               //cerr << "+";
            }
            
            // kolla traff lm
            if(useEnds){
               testLM->setIsStop(true);
               if(first){
                  testLM->setIsStart(true);
                  lastLM.insert(make_pair(distID, testLM));
               } else {
                  testLM->setIsStart(false);
                  mi = lastLM.find(distID);
                  if(mi != lastLM.end()){
                     // We have at least one before
                     if(mi->second->isStart()){
                        // It was the first, keep it but set the first to not
                        // stop
                        mc2dbg4 << ", setting prev not stop";
                        mi->second->setIsStop(false);
                     } else {
                        // It was not the first 
                        // delete the old landmark
                        mi->second->out();
                        delete (mi->second);
                        mc2dbg4 << ", removing prev lm";
                     }
                     // Change pointer in the map
                     mi->second = testLM;
                     
                  }
                  
                  lastLM.insert(make_pair(distID, testLM));
               }
               
            } else if (!first){
               // This disturbance is already here once, remove
               removeTest = true;
            }
            
         }
         if(removeTest){
            // go to next landmark and remove this
            LandmarkLink* deleteLM = testLM;
            testLM = static_cast<LandmarkLink*>(testLM->suc());
            deleteLM->out();
            delete deleteLM;
            deleteLM = NULL;
            mc2dbg4 << ", removing landmark";
         } else {
            // just go to next landmark
            testLM = static_cast<LandmarkLink*>(testLM->suc());
         }
         
      }
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
      mc2dbg4 << endl;
   }
   curLink = static_cast <ExpandRouteLink*>(m_routeList->last());
   while((curLink != NULL) && useEnds){

      // Hitta camera.
      bool newCamera = false;
      int cameraCount = 0; // number of actual cameras in this lm
      
      LandmarkLink* testLM = static_cast <LandmarkLink*>
         (curLink->m_landmarks->first());
      while (testLM != NULL){
         bool deleted = false;   // Will we delete this LM 
         if(testLM->isStart() && (curLink->pred() != NULL) &&
            (testLM->m_lmdesc.type == ItemTypes::cameraLM)
            // || ()   other types to present earlier.
            ){
            cameraCount++;
            if(newCamera){
               // we already have a camera on this link. delete.
               deleted = true;
               LandmarkLink* succLM = static_cast <LandmarkLink*>
                           (testLM->suc());
               testLM->out();
               delete testLM;
               testLM = succLM;
            } else {
               newCamera = true;
               // Search for a place to put the start.
               ExpandRouteLink* searchLink = curLink;
               int linkDist = 0;
               do{
                  searchLink = static_cast<ExpandRouteLink*>
                     (searchLink->pred());
                  // Any cameras here? delete all
                  bool anyCameras = false;
                  LandmarkLink* searchLM = static_cast <LandmarkLink*>
                     (searchLink->m_landmarks->first());
                  while(searchLM != NULL){
                     if(searchLM->m_lmdesc.type == testLM->m_lmdesc.type){
                        anyCameras = true;
                        LandmarkLink* succLM = static_cast <LandmarkLink*>
                           (searchLM->suc());
                        searchLM->out();
                        delete searchLM;
                        searchLM = succLM;
                     } else {
                        searchLM = static_cast <LandmarkLink*>
                           (searchLM->suc());
                     }
                  }
                  // update distance
                  if(anyCameras)
                     linkDist = 0;
                  else
                     linkDist+= searchLink->m_item->getLength();
                  
               } while((searchLink->pred() != NULL ) &&
                       (linkDist < 500));
               //cerr << "Leaving search with " <<  linkDist << "m" << endl;
               if(searchLink->pred() == NULL)
                  searchLink= static_cast<ExpandRouteLink*>
                     (searchLink->suc());
               
               // We have the place to put the new lm.
               LandmarkLink* newStart = new LandmarkLink(testLM->m_lmdesc,
                                                         0,
                                                         false,
                                                         testLM->getLMName()
                                                         );
               newStart->into(searchLink->m_landmarks);
               newStart->setIsStart( true);
               newStart->setIsStop( false );
               newStart->setIsTraffic();
               if(testLM->isDetour())
                  newStart->setDetour();
               //newStart->setStreetName(testLM->getStreetName());
               newStart->m_distNodeID = testLM->m_distNodeID;
               testLM->setIsStart( false);

               // Push the end lm a bit back if possible just to make sure.
               // A bit crude should be enhanced later.
               if(curLink->suc() != NULL){
                  testLM->out();
                  testLM->into(static_cast<ExpandRouteLink*>
                               (curLink->suc())->m_landmarks);
               } else {
               
                  testLM->m_lmDist += curLink->m_item->getLength();
                  testLM->m_lmdesc.location = ItemTypes::before;
                  testLM->m_lmDistUpdated = true;
               }
               
               curLink = searchLink;
            }
         }
         if(!deleted)
            testLM = static_cast <LandmarkLink*>(testLM->suc());
      }
      if(curLink != NULL)
         curLink = static_cast<ExpandRouteLink*>(curLink->pred());
   }
   

   // Go through the landmarks and add Streetnames and lm name.
   char* distStreet;
   char* tempStr;

   curLink = static_cast <ExpandRouteLink*>(m_routeList->first());
   while(curLink != NULL){
      LandmarkLink* testLM = static_cast <LandmarkLink*>
         (curLink->m_landmarks->first());
      
      while (testLM != NULL){
      
         // Get best name of disturbance street.
         if(testLM->m_distNodeID != MAX_UINT32){
            Item* item = m_map->itemLookup(testLM->m_distNodeID);
            if(item != NULL){
               int nameLength = 0;
               uint32 coosenName    = MAX_UINT32;
               uint32 alternateName = MAX_UINT32;
               set<uint32>* usedNames = new set<uint32>;
               coosenName = item->getBestRawNameOfType(ItemTypes::roadNumber,
                                                       m_requestedLanguage,
                                                       usedNames);
               if(coosenName == MAX_UINT32){ 
                  coosenName = item->getBestRawName(m_requestedLanguage,
                                                    ItemTypes::officialName,
                                                    usedNames);
               }
               
               // Find alternate name?  if 1st nbr -> o-name else a-name
               if(coosenName != MAX_UINT32){
                  distStreet = new char[DIST_STR_NAME_LENGTH*10];
                  alternateName = item->getBestRawName(m_requestedLanguage,
                                                       ItemTypes::officialName,
                                                       usedNames);
                  bool exitnbr =
                     (GET_STRING_TYPE(coosenName) == ItemTypes::exitNumber);
                  
                  const char* nameToFormat = m_map->getName(GET_STRING_INDEX(coosenName));
                  formatName( nameToFormat ? nameToFormat : "",
                              distStreet, m_requestedLanguage, exitnbr);
                  if(alternateName != MAX_UINT32){
                     if(distStreet != NULL)
                        nameLength = strlen(distStreet);
                     if(2*nameLength < DIST_STR_NAME_LENGTH){
                        tempStr = new char[DIST_STR_NAME_LENGTH*10];
                        exitnbr = (GET_STRING_TYPE(alternateName) ==
                                   ItemTypes::exitNumber);
                        formatName(m_map->getName(
                                      GET_STRING_INDEX(alternateName)),
                                   tempStr, m_requestedLanguage, exitnbr);
                        if((tempStr != NULL)&&
                           (int(nameLength + 2 + strlen(tempStr)) <
                            DIST_STR_NAME_LENGTH)){
                           strcat(distStreet,"/");
                           strcat(distStreet,tempStr);
                        }
                        // delete the name;
                        delete [] tempStr;
                     }
                     
                     
                  }
                  testLM->setStreetName(distStreet);
                  // delete the name;
                  delete [] distStreet;
               }
               
               
               usedNames->clear();
               delete usedNames;
            }
         }
         
         testLM = static_cast <LandmarkLink*>(testLM->suc());
      }
         
      curLink = static_cast<ExpandRouteLink*>(curLink->suc()); 
   }
   
   
   mc2dbg4 << "Found " << lmCount << " landmarks" << endl;
   
   return true;
}

StringTable::countryCode 
ExpandRouteProcessor::regionIDToCountryCode( uint32 regionID )
{
   // Temporary ugly method...
   switch ( regionID ) {
      case ( 0 ) :
         return StringTable::ENGLAND_CC;
      case ( 1 ) :
         return StringTable::SWEDEN_CC;
      case ( 2 ) :
         return StringTable::GERMANY_CC;
      case ( 3 ) :
         return StringTable::DENMARK_CC;
      case ( 4 ) :
         return StringTable::FINLAND_CC;
      case ( 5 ) :
         return StringTable::NORWAY_CC;
      case ( 6 ) :
         return StringTable::BELGIUM_CC;
      case ( 7 ) :
         return StringTable::NETHERLANDS_CC;
      case ( 8 ) :
         return StringTable::LUXEMBOURG_CC;
      case ( 9 ) :
         return StringTable::USA_CC;
      case ( 61 ) :
         return StringTable::IRELAND_CC;
      case ( 62 ) :
         return StringTable::SWITZERLAND_CC;
      case ( 63 ) :
         return StringTable::AUSTRIA_CC;
      case ( 64 ) :
         return StringTable::FRANCE_CC;
      case ( 65 ) :
         return StringTable::SPAIN_CC;
      case ( 66 ) :
         return StringTable::LIECHTENSTEIN_CC;
      case ( 67 ) :
         return StringTable::ITALY_CC;
      case ( 68 ) :
         return StringTable::ANDORRA_CC;
      case ( 69 ) :
         return StringTable::MONACO_CC;
      case ( 70 ) :
         return StringTable::PORTUGAL_CC;
      case ( 73 ) :
         return StringTable::CANADA_CC;
      case ( 74 ) :
         return StringTable::HUNGARY_CC;
      case ( 75 ) :
         return StringTable::CZECH_REPUBLIC_CC;
      case ( 76 ) :
         return StringTable::POLAND_CC;
      case ( 77 ) :
         return StringTable::GREECE_CC;
      case ( 78 ) :
         return StringTable::ISRAEL_CC;
      case ( 79 ) :
         return StringTable::BRAZIL_CC;
      case ( 80 ) :
         return StringTable::SLOVAKIA_CC;
      case ( 81 ) :
         return StringTable::RUSSIA_CC;
      case ( 82 ) :
         return StringTable::TURKEY_CC;
      case ( 83 ) :
         return StringTable::HONG_KONG_CC;
      case ( 84 ) :
         return StringTable::SINGAPORE_CC;
      case ( 85 ) :
         return StringTable::CROATIA_CC;
      case ( 86 ) :
         return StringTable::SLOVENIA_CC;
      case ( 87 ) :
         return StringTable::AUSTRALIA_CC;
      case ( 88 ) :
         return StringTable::UAE_CC;
      case ( 89 ) :
         return StringTable::BAHRAIN_CC;
      case ( 90 ) :
         return StringTable::AFGHANISTAN_CC;
      case ( 91 ) :
         return StringTable::ALBANIA_CC;
      case ( 92 ) :
         return StringTable::ALGERIA_CC;
      case ( 93 ) :
         return StringTable::AMERICAN_SAMOA_CC;
      case ( 94 ) :
         return StringTable::ANGOLA_CC;
      case ( 95 ) :
         return StringTable::ANGUILLA_CC;
      case ( 96 ) :
         return StringTable::ANTARCTICA_CC;
      case ( 97 ) :
         return StringTable::ANTIGUA_AND_BARBUDA_CC;
      case ( 98 ) :
         return StringTable::ARGENTINA_CC;
      case ( 99 ) :
         return StringTable::ARMENIA_CC;
      case ( 100 ) :
         return StringTable::ARUBA_CC;
      case ( 101 ) :
         return StringTable::AZERBAIJAN_CC;
      case ( 102 ) :
         return StringTable::BAHAMAS_CC;
      case ( 103 ) :
         return StringTable::BANGLADESH_CC;
      case ( 104 ) :
         return StringTable::BARBADOS_CC;
      case ( 105 ) :
         return StringTable::BELARUS_CC;
      case ( 106 ) :
         return StringTable::BELIZE_CC;
      case ( 107 ) :
         return StringTable::BENIN_CC;
      case ( 108 ) :
         return StringTable::BERMUDA_CC;
      case ( 109 ) :
         return StringTable::BHUTAN_CC;
      case ( 110 ) :
         return StringTable::BOLIVIA_CC;
      case ( 111 ) :
         return StringTable::BOSNIA_CC;
      case ( 112 ) :
         return StringTable::BOTSWANA_CC;
      case ( 113 ) :
         return StringTable::BRITISH_VIRGIN_ISLANDS_CC;
      case ( 114 ) :
         return StringTable::BRUNEI_DARUSSALAM_CC;
      case ( 115 ) :
         return StringTable::BULGARIA_CC;
      case ( 116 ) :
         return StringTable::BURKINA_FASO_CC;
      case ( 117 ) :
         return StringTable::BURUNDI_CC;
      case ( 118 ) :
         return StringTable::CAMBODIA_CC;
      case ( 119 ) :
         return StringTable::CAMEROON_CC;
      case ( 120 ) :
         return StringTable::CAPE_VERDE_CC;
      case ( 121 ) :
         return StringTable::CAYMAN_ISLANDS_CC;
      case ( 122 ) :
         return StringTable::CENTRAL_AFRICAN_REPUBLIC_CC;
      case ( 123 ) :
         return StringTable::CHAD_CC;
      case ( 124 ) :
         return StringTable::CHILE_CC;
      case ( 125 ) :
         return StringTable::CHINA_CC;
      case ( 126 ) :
         return StringTable::COLOMBIA_CC;
      case ( 127 ) :
         return StringTable::COMOROS_CC;
      case ( 128 ) :
         return StringTable::CONGO_CC;
      case ( 129 ) :
         return StringTable::COOK_ISLANDS_CC;
      case ( 130 ) :
         return StringTable::COSTA_RICA_CC;
      case ( 131 ) :
         return StringTable::CUBA_CC;
      case ( 132 ) :
         return StringTable::CYPRUS_CC;
      case ( 133 ) :
         return StringTable::DJIBOUTI_CC;
      case ( 134 ) :
         return StringTable::DOMINICA_CC;
      case ( 135 ) :
         return StringTable::DOMINICAN_REPUBLIC_CC;
      case ( 136 ) :
         return StringTable::DR_CONGO_CC;
      case ( 137 ) :
         return StringTable::ECUADOR_CC;
      case ( 138 ) :
         return StringTable::EGYPT_CC;
      case ( 139 ) :
         return StringTable::EL_SALVADOR_CC;
      case ( 140 ) :
         return StringTable::EQUATORIAL_GUINEA_CC;
      case ( 141 ) :
         return StringTable::ERITREA_CC;
      case ( 142 ) :
         return StringTable::ESTONIA_CC;
      case ( 143 ) :
         return StringTable::ETHIOPIA_CC;
      case ( 144 ) :
         return StringTable::FAEROE_ISLANDS_CC;
      case ( 145 ) :
         return StringTable::FALKLAND_ISLANDS_CC;
      case ( 146 ) :
         return StringTable::FIJI_CC;
      case ( 147 ) :
         return StringTable::FRENCH_GUIANA_CC;
      case ( 148 ) :
         return StringTable::FRENCH_POLYNESIA_CC;
      case ( 149 ) :
         return StringTable::GABON_CC;
      case ( 150 ) :
         return StringTable::GAMBIA_CC;
      case ( 151 ) :
         return StringTable::GEORGIA_CC;
      case ( 152 ) :
         return StringTable::GHANA_CC;
      case ( 153 ) :
         return StringTable::GREENLAND_CC;
      case ( 154 ) :
         return StringTable::GRENADA_CC;
      case ( 155 ) :
         return StringTable::GUADELOUPE_CC;
      case ( 156 ) :
         return StringTable::GUAM_CC;
      case ( 157 ) :
         return StringTable::GUATEMALA_CC;
      case ( 158 ) :
         return StringTable::GUINEA_CC;
      case ( 159 ) :
         return StringTable::GUINEA_BISSAU_CC;
      case ( 160 ) :
         return StringTable::GUYANA_CC;
      case ( 161 ) :
         return StringTable::HAITI_CC;
      case ( 162 ) :
         return StringTable::HONDURAS_CC;
      case ( 163 ) :
         return StringTable::ICELAND_CC;
      case ( 164 ) :
         return StringTable::INDIA_CC;
      case ( 165 ) :
         return StringTable::INDONESIA_CC;
      case ( 166 ) :
         return StringTable::IRAN_CC;
      case ( 167 ) :
         return StringTable::IRAQ_CC;
      case ( 168 ) :
         return StringTable::IVORY_COAST_CC;
      case ( 169 ) :
         return StringTable::JAMAICA_CC;
      case ( 170 ) :
         return StringTable::JAPAN_CC;
      case ( 171 ) :
         return StringTable::JORDAN_CC;
      case ( 172 ) :
         return StringTable::KAZAKHSTAN_CC;
      case ( 173 ) :
         return StringTable::KENYA_CC;
      case ( 174 ) :
         return StringTable::KIRIBATI_CC;
      case ( 175 ) :
         return StringTable::KUWAIT_CC;
      case ( 176 ) :
         return StringTable::KYRGYZSTAN_CC;
      case ( 177 ) :
         return StringTable::LAOS_CC;
      case ( 178 ) :
         return StringTable::LATVIA_CC;
      case ( 179 ) :
         return StringTable::LEBANON_CC;
      case ( 180 ) :
         return StringTable::LESOTHO_CC;
      case ( 181 ) :
         return StringTable::LIBERIA_CC;
      case ( 182 ) :
         return StringTable::LIBYA_CC;
      case ( 183 ) :
         return StringTable::LITHUANIA_CC;
      case ( 184 ) :
         return StringTable::MACAO_CC;
      case ( 185 ) :
         return StringTable::MACEDONIA_CC;
      case ( 186 ) :
         return StringTable::MADAGASCAR_CC;
      case ( 187 ) :
         return StringTable::MALAWI_CC;
      case ( 188 ) :
         return StringTable::MALAYSIA_CC;
      case ( 189 ) :
         return StringTable::MALDIVES_CC;
      case ( 190 ) :
         return StringTable::MALI_CC;
      case ( 191 ) :
         return StringTable::MALTA_CC;
      case ( 192 ) :
         return StringTable::MARSHALL_ISLANDS_CC;
      case ( 193 ) :
         return StringTable::MARTINIQUE_CC;
      case ( 194 ) :
         return StringTable::MAURITANIA_CC;
      case ( 195 ) :
         return StringTable::MAURITIUS_CC;
      case ( 196 ) :
         return StringTable::MAYOTTE_CC;
      case ( 197 ) :
         return StringTable::MEXICO_CC;
      case ( 198 ) :
         return StringTable::MICRONESIA_CC;
      case ( 199 ) :
         return StringTable::MOLDOVA_CC;
      case ( 200 ) :
         return StringTable::MONGOLIA_CC;
      case ( 201 ) :
         return StringTable::MONTSERRAT_CC;
      case ( 202 ) :
         return StringTable::MOROCCO_CC;
      case ( 203 ) :
         return StringTable::MOZAMBIQUE_CC;
      case ( 204 ) :
         return StringTable::MYANMAR_CC;
      case ( 205 ) :
         return StringTable::NAMIBIA_CC;
      case ( 206 ) :
         return StringTable::NAURU_CC;
      case ( 207 ) :
         return StringTable::NEPAL_CC;
      case ( 208 ) :
         return StringTable::NETHERLANDS_ANTILLES_CC;
      case ( 209 ) :
         return StringTable::NEW_CALEDONIA_CC;
      case ( 210 ) :
         return StringTable::NEW_ZEALAND_CC;
      case ( 211 ) :
         return StringTable::NICARAGUA_CC;
      case ( 212 ) :
         return StringTable::NIGER_CC;
      case ( 213 ) :
         return StringTable::NIGERIA_CC;
      case ( 214 ) :
         return StringTable::NIUE_CC;
      case ( 215 ) :
         return StringTable::NORTHERN_MARIANA_ISLANDS_CC;
      case ( 216 ) :
         return StringTable::NORTH_KOREA_CC;
      case ( 217 ) :
         return StringTable::OCCUPIED_PALESTINIAN_TERRITORY_CC;
      case ( 218 ) :
         return StringTable::OMAN_CC;
      case ( 219 ) :
         return StringTable::PAKISTAN_CC;
      case ( 220 ) :
         return StringTable::PALAU_CC;
      case ( 221 ) :
         return StringTable::PANAMA_CC;
      case ( 222 ) :
         return StringTable::PAPUA_NEW_GUINEA_CC;
      case ( 223 ) :
         return StringTable::PARAGUAY_CC;
      case ( 224 ) :
         return StringTable::PERU_CC;
      case ( 225 ) :
         return StringTable::PHILIPPINES_CC;
      case ( 226 ) :
         return StringTable::PITCAIRN_CC;
      case ( 227 ) :
         return StringTable::QATAR_CC;
      case ( 228 ) :
         return StringTable::REUNION_CC;
      case ( 229 ) :
         return StringTable::ROMANIA_CC;
      case ( 230 ) :
         return StringTable::RWANDA_CC;
      case ( 231 ) :
         return StringTable::SAINT_HELENA_CC;
      case ( 232 ) :
         return StringTable::SAINT_KITTS_AND_NEVIS_CC;
      case ( 233 ) :
         return StringTable::SAINT_LUCIA_CC;
      case ( 234 ) :
         return StringTable::SAINT_PIERRE_AND_MIQUELON_CC;
      case ( 235 ) :
         return StringTable::SAINT_VINCENT_AND_THE_GRENADINES_CC;
      case ( 236 ) :
         return StringTable::SAMOA_CC;
      case ( 237 ) :
         return StringTable::SAO_TOME_AND_PRINCIPE_CC;
      case ( 238 ) :
         return StringTable::SAUDI_ARABIA_CC;
      case ( 239 ) :
         return StringTable::SENEGAL_CC;
      case ( 240 ) :
         return StringTable::SERBIA_CC;
      case ( 241 ) :
         return StringTable::SEYCHELLES_CC;
      case ( 242 ) :
         return StringTable::SIERRA_LEONE_CC;
      case ( 243 ) :
         return StringTable::SOLOMON_ISLANDS_CC;
      case ( 244 ) :
         return StringTable::SOMALIA_CC;
      case ( 245 ) :
         return StringTable::SOUTH_AFRICA_CC;
      case ( 246 ) :
         return StringTable::SOUTH_KOREA_CC;
      case ( 247 ) :
         return StringTable::SRI_LANKA_CC;
      case ( 248 ) :
         return StringTable::SUDAN_CC;
      case ( 249 ) :
         return StringTable::SURINAME_CC;
      case ( 250 ) :
         return StringTable::SVALBARD_AND_JAN_MAYEN_CC;
      case ( 251 ) :
         return StringTable::SWAZILAND_CC;
      case ( 252 ) :
         return StringTable::SYRIA_CC;
      case ( 253 ) :
         return StringTable::TAIWAN_CC;
      case ( 254 ) :
         return StringTable::TAJIKISTAN_CC;
      case ( 255 ) :
         return StringTable::TANZANIA_CC;
      case ( 256 ) :
         return StringTable::THAILAND_CC;
      case ( 257 ) :
         return StringTable::TIMOR_LESTE_CC;
      case ( 258 ) :
         return StringTable::TOGO_CC;
      case ( 259 ) :
         return StringTable::TOKELAU_CC;
      case ( 260 ) :
         return StringTable::TONGA_CC;
      case ( 261 ) :
         return StringTable::TRINIDAD_AND_TOBAGO_CC;
      case ( 262 ) :
         return StringTable::TUNISIA_CC;
      case ( 263 ) :
         return StringTable::TURKMENISTAN_CC;
      case ( 264 ) :
         return StringTable::TURKS_AND_CAICOS_ISLANDS_CC;
      case ( 265 ) :
         return StringTable::TUVALU_CC;
      case ( 266 ) :
         return StringTable::UGANDA_CC;
      case ( 267 ) :
         return StringTable::UKRAINE_CC;
      case ( 268 ) :
         return StringTable::UNITED_STATES_MINOR_OUTLYING_ISLANDS_CC;
      case ( 269 ) :
         return StringTable::UNITED_STATES_VIRGIN_ISLANDS_CC;
      case ( 270 ) :
         return StringTable::URUGUAY_CC;
      case ( 271 ) :
         return StringTable::UZBEKISTAN_CC;
      case ( 272 ) :
         return StringTable::VANUATU_CC;
      case ( 273 ) :
         return StringTable::VENEZUELA_CC;
      case ( 274 ) :
         return StringTable::VIETNAM_CC;
      case ( 275 ) :
         return StringTable::WALLIS_AND_FUTUNA_ISLANDS_CC;
      case ( 276 ) :
         return StringTable::WESTERN_SAHARA_CC;
      case ( 277 ) :
         return StringTable::YEMEN_CC;
      case ( 278 ) :
         return StringTable::ZAMBIA_CC;
      case ( 279 ) :
         return StringTable::ZIMBABWE_CC;
      default: 
         return StringTable::NBR_COUNTRY_CODES;
   }
}

bool 
ExpandRouteProcessor::toPrintNewName(Item* item) 
{
   
   // If driving on a roundabout, don't print any names, and don't
   // update m_possibleames, since we want to be able to choose the
   // same roadname as before entering the rb.
   if ((item->getItemType() == ItemTypes::streetSegmentItem) &&
      (static_cast<StreetSegmentItem*> (item)->isRoundabout())) {
      return (false);
   }
   
   // If we are driving into a MISSING-street from a named street, don't 
   // print the name.
   
   if (item->getNbrNames() == 0) {
      // Update possible names to none.
      //m_possibleNames.reset();
      return (false);
   }
   

   // Vector with names to remove
   Vector removeNames;
   
   // Check if item have any name that is the same as the ones in
   // m_possibleNames
   uint32 i = 0;
   while (i < m_possibleNames.getSize()) {
      bool found = false;
      uint32 j = 0;
      while ((! found) && (j < item->getNbrNames())) {
         // Check if it is the same string (never mind what language
         // and type).
         if (GET_STRING_INDEX(item->getRawStringIndex(j)) == 
             GET_STRING_INDEX(m_possibleNames.getElementAt(i))) {
            found = true;
         } 
         j++;
      }
      
      if (! found) {
         // Name number i not present in new item. Note that the
         // indices will be in growing order.. (ascending)
         removeNames.addLast(i);
      }
      i++;
   }

   // XXX:
   // If the flag m_printNamesIfTheyDiffer is set, return that the
   // names should be printed in case that itemNames does not 
   // contain all possible names.
   // The flag is only set to true for packets that are not first, and
   // as soon as a name is printed, it is set to false again.
   // This code is here since we don't want an extra turndescription 
   // when crossing a map boundary. We assume that the names have not
   // for the two segments on each side of the map boundary.
   // It would be better to skip this and let the route concatenator
   // in the server choose the proper name, however this is here until
   // that is taken care of!
   if ((m_printNamesIfTheyDiffer) && (removeNames.getSize() > 0)) {
      m_printNamesIfTheyDiffer = false;
      return (true);
   }
   
   // Update m_possibleNames if not all names should be removed.
   if (m_possibleNames.getSize() > removeNames.getSize()) {
      // Remove the elements in reverse order. This means that the
      // highest indices will be removed first.
      for (int i = removeNames.getSize() - 1; i >= 0; i--) {
         m_possibleNames.removeElementAt(removeNames.getElementAt(i));
      }
      
      // The name should not be printed!
      return (false);
   } else {
      // No names in common -> the name should be printed.
      // Print the name(s) in m_possibleNames for the "previous" item.
      // Then afterwards updatePossibleNames() should be run in order
      // to reset m_possibleNames with the new item names.
      return (true);
   }
}

void
ExpandRouteProcessor::updatePossibleNames(Item* item) 
{
   // Reset possible names
   m_possibleNames.reset();
   
   // Update possible names with the names of the item.
   for (uint32 i = 0; i < item->getNbrNames(); i++) {
      m_possibleNames.addLast(item->getRawStringIndex(i));
   }
   
}



const char* 
ExpandRouteProcessor::getBestName(bool manyNames,
                                  bool excludeBestName) 
{
   uint32 stringIndex = MAX_UINT32;
   uint32 indexToExclude = MAX_UINT32;
   LangTypes::language_t chosenLang = LangTypes::invalidLanguage;

   // Is a name an exit number? In that case we must add exit- or avfart-
   // before the name.
   bool exitNumber = false; 
   bool nonNbrName = false;
   
   // If we don't want all names, get the "best" one.
   if (!manyNames || excludeBestName) {
      if (getNbrPossibleNames() > 0) {
         // Check if we have any road number on any language
         // (E-road names and A-road names will be prefered)
         uint32 nameIndex;
         stringIndex = getStringWithType(
            ItemTypes::roadNumber, 
            LangTypes::invalidLanguage,
            chosenLang,
            nameIndex);
         if (stringIndex == MAX_UINT32) {
            // Check if we have any official name on prefered language
            stringIndex = getStringWithType(
               ItemTypes::officialName, 
               ItemTypes::getLanguageCodeAsLanguageType(
                  StringTable::languageCode(
                     m_preferedLanguages.getElementAt(0) ) ),
               chosenLang,
            nameIndex );
            nonNbrName = true;
         }

         if (stringIndex == MAX_UINT32) {
            // Check if we have any exit number.
            stringIndex = getStringWithType(
               ItemTypes::exitNumber, 
               LangTypes::invalidLanguage,
               chosenLang,
               nameIndex );
            exitNumber = (stringIndex != MAX_UINT32);
         }

         // Use NameUtility::getBestName()
         
         if (stringIndex == MAX_UINT32) {
            // Check if we have any alternative name on prefered language
            stringIndex = getStringWithType(
               ItemTypes::alternativeName, 
               ItemTypes::getLanguageCodeAsLanguageType(
                  StringTable::languageCode(
                     m_preferedLanguages.getElementAt(0) ) ),
               chosenLang,
            nameIndex );
            nonNbrName = true;
         }
         if (stringIndex == MAX_UINT32) {
            // Check for offical english name.
            stringIndex = getStringWithType(
               ItemTypes::officialName,
               LangTypes::english,
               chosenLang,
            nameIndex );
            nonNbrName = true;
         }
         
         if (stringIndex == MAX_UINT32) {
            // Check for any offical name.
            stringIndex = getStringWithType(
               ItemTypes::officialName,
               LangTypes::invalidLanguage,
               chosenLang,
            nameIndex);
            nonNbrName = true;
         }
         
         if (stringIndex == MAX_UINT32) {
            // Check for english alternative name.
            stringIndex = getStringWithType(
               ItemTypes::alternativeName,
               LangTypes::english,
               chosenLang,
            nameIndex );
            nonNbrName = true;
         }
         if (stringIndex == MAX_UINT32) {
            // Check for any alternative name.
            stringIndex = getStringWithType(
               ItemTypes::alternativeName,
               LangTypes::invalidLanguage,
               chosenLang ,
            nameIndex);
            nonNbrName = true;
         }
        
         if (stringIndex == MAX_UINT32) {
            mc2dbg4 << "  Take the first name..." << endl;
            // Take the first name...
            stringIndex = 
               GET_STRING_INDEX(m_possibleNames.getElementAt(0));
            chosenLang = LangTypes::language_t(
               GET_STRING_LANGUAGE(m_possibleNames.getElementAt(0)));
         }         
      } else {
         // No names -- return MISSING :(
         stringIndex = 0;
      }
      //chosenLang = LangTypes::english;
      mc2dbg8 << "Chosen language = " << (int) chosenLang << endl;
      // Format the name and put it into m_currentRoadName
      const char* tmpName = m_map->getName( stringIndex );
      formatName( tmpName ? tmpName : "", m_currentRoadName,
                  chosenLang, exitNumber);
      
   }
   if(manyNames){
      // Use many names
      
      if(excludeBestName)
         indexToExclude = stringIndex;
      stringIndex = MAX_UINT32;
      bool nameFound = false;
      if((getNbrPossibleNames() > 1) ||
         ((getNbrPossibleNames() > 0) && !excludeBestName))  {
         // Temporary string
         char tmpRoadName[80];
         
         
         // Add the available nbr road names
         for (uint32 i = 0; i < getNbrPossibleNames(); i++) {
            if(GET_STRING_TYPE(m_possibleNames.getElementAt(i)) ==
               ItemTypes::roadNumber)
            {
               stringIndex = GET_STRING_INDEX(m_possibleNames.getElementAt(i));
               if(stringIndex != indexToExclude){
                  LangTypes::language_t currLang =
                     GET_STRING_LANGUAGE(m_possibleNames.getElementAt(i));
                  mc2dbg4 << "Currlang = " << (int) currLang << endl;
                  formatName(m_map->getName(stringIndex), tmpRoadName,
                             currLang);
                  if (!nameFound) {
                     // First name
                     strcpy(m_currentRoadName, tmpRoadName);
                     nameFound = true;
                  } else {
                     // Add the name at the end, seperated by "/"
                     strcat(m_currentRoadName, " / ");
                     strcat(m_currentRoadName, tmpRoadName);
                  }
               }
               
            }
         }
         
         // Get Official name,
         stringIndex = MAX_UINT32;
         for (uint32 i = 0; i < getNbrPossibleNames(); i++) {
            if(GET_STRING_TYPE(m_possibleNames.getElementAt(i)) ==
               ItemTypes::officialName){
               
               if((stringIndex == MAX_UINT32) ||
                  (GET_STRING_LANGUAGE(m_possibleNames.getElementAt(i)) ==
                       ItemTypes::getLanguageCodeAsLanguageType(
                          StringTable::languageCode(
                             m_preferedLanguages.getElementAt(0)))) ||
                  ((GET_STRING_LANGUAGE(stringIndex) !=
                    ItemTypes::getLanguageCodeAsLanguageType(
                       StringTable::languageCode(
                          m_preferedLanguages.getElementAt(0)))) &&
                   (GET_STRING_LANGUAGE(m_possibleNames.getElementAt(i)) ==
                    LangTypes::english))
                  ){
                     
                  stringIndex =
                     GET_STRING_INDEX(m_possibleNames.getElementAt(i));
                  chosenLang =
                     GET_STRING_LANGUAGE(m_possibleNames.getElementAt(i));
                  
                  
               }
            }
         }
         
         if((stringIndex != MAX_UINT32) && (stringIndex != indexToExclude)){
            formatName(m_map->getName(stringIndex), tmpRoadName,
                       chosenLang, false, true);
            if (!nameFound) {
               // First name
               strcpy(m_currentRoadName, tmpRoadName);
            } else {
               // Add the name at the end, seperated by "/"
               strcat(m_currentRoadName, " / ");
               strcat(m_currentRoadName, tmpRoadName);
            }
            nameFound = true;
         }
         
         // Add a single alternative name of pref. lang.
         stringIndex = MAX_UINT32;
         for (uint32 i = 0; i < getNbrPossibleNames(); i++) {
            if((GET_STRING_TYPE(m_possibleNames.getElementAt(i)) ==
                ItemTypes::alternativeName) &&
               (GET_STRING_LANGUAGE(m_possibleNames.getElementAt(i)) ==
                       ItemTypes::getLanguageCodeAsLanguageType(
                          StringTable::languageCode(
                             m_preferedLanguages.getElementAt(0))))){
               stringIndex =
                  GET_STRING_INDEX(m_possibleNames.getElementAt(i));
               chosenLang =
                  GET_STRING_LANGUAGE(m_possibleNames.getElementAt(i));
            }
         }  
         if((stringIndex != MAX_UINT32) && (stringIndex != indexToExclude)){
            formatName(m_map->getName(stringIndex), tmpRoadName,
                       chosenLang, false, true);
           if (!nameFound) {
               // First name
               strcpy(m_currentRoadName, tmpRoadName);
            } else {
               // Add the name at the end, seperated by "/"
               strcat(m_currentRoadName, " / ");
               strcat(m_currentRoadName, tmpRoadName);
            }
            nameFound = true;
         }
         
      }
      if(!nameFound) {
         // No names -- return MISSING :(
         mc2dbg4 << "No names -- return MISSING :(" << endl;
         if(excludeBestName)
            return NULL;

         const char* tmpName = m_map->getName( 0 );
         formatName( tmpName ? tmpName : "", m_currentRoadName,
                     LangTypes::invalidLanguage);
      }
   }
   if(stringIndex != 0){
      if(indexToExclude == MAX_UINT32)
         m_currentStringIndex = stringIndex;
      else
         m_currentStringIndex = indexToExclude;
   }
   
   // The name to return is stored in the preallocated buffer
   // m_currentRoadName
   
   return (m_currentRoadName);
      
}

void 
ExpandRouteProcessor::formatName(const char* roadName, 
                                 char* formattedName,
                                 LangTypes::language_t language,
                                 bool exitNbr,
                                 bool noNumberName)
{
   StackOrHeap< 2*MAXIMUM_SIZE_OF_ROADNAME + 4, char > tmpRoadName( 
      strlen( roadName ) + 1 + MAXIMUM_SIZE_OF_ROADNAME + 4 );
   strcpy(tmpRoadName, roadName);

   if ( StringUtility::onlyDigitsInString(roadName) ) {
      
      if(!exitNbr){
         // This roadname consists of only digits -- assume that it is a
         // roadnumber and print "Road" (in correct language) in front of it!
         sprintf(
            tmpRoadName, "%s%c%s", 
            StringTable::getString(
               StringTable::PRE_ROUTE_NUMBER, 
               StringTable::languageCode(m_preferedLanguages.getElementAt(0))),
            ' ', 
            roadName);
      } else{
         // This roadname is an exitNumber. Print "Exit" (in correct language)
         // in front of it!
         sprintf(
            tmpRoadName, "%s%c%s", 
            StringTable::getString(
               StringTable::EXIT, 
               StringTable::languageCode(m_preferedLanguages.getElementAt(0))),
            ' ', 
            roadName);         
      }
      
   }
/*
   else if (false){
      // check if not number name, add "calle" to spanish.Z
      sprintf(
            tmpRoadName, "%s%c%s", 
            StringTable::getString(
               StringTable::PRE_ROADNAME, 
               StringTable::languageCode(m_preferedLanguages.getElementAt(0))),
            ' ', roadName);
   }
      */
   
   // Check if the name should be abbreviated or not
   // XXX: The name should not be abbreviated according to the 
   // language the user has chosen, but rather to the language of the
   // roadname.
      if (m_abbreviateStreetNames) {
      AbbreviationTable::abbreviate(tmpRoadName, formattedName, 
                                    language,
                                    AbbreviationTable::beginningAndEnd);
      mc2dbg4 << here << " Abbrevating streetname " << tmpRoadName
             <<" , new name=" << formattedName << endl;
   } else {
      strcpy(formattedName, tmpRoadName);
      mc2dbg4 << here << " Does NOT abbrevate streetname, name=" 
              << formattedName << endl;
   }   
}

void
ExpandRouteProcessor::expandNames()
{
   ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>
                              (m_routeList->first());
   ExpandRouteLink* prevLink = NULL;
   while(curLink != NULL){
      if((prevLink!=NULL) &&
         ((curLink->m_turnDescription == StringTable::KEEP_LEFT) ||
         (curLink->m_turnDescription == StringTable::KEEP_RIGHT) ||
          (curLink->m_turnDescription == StringTable::U_TURN)) &&
         curLink->m_item->hasSameNames(prevLink->m_item)){
         mc2dbg2 << "Same names dont show: " <<curLink->m_streetName << endl;
         curLink->m_stringCode = 0;
         curLink->m_allStringCodes.clear();
      } 
      char* roadName  = new char[ MAXIMUM_SIZE_OF_ROADNAME ];
      char* linkName  = new char[ 2 * MAXIMUM_SIZE_OF_ROADNAME * 
                                  (1 + curLink->m_allStringCodes.size()) + 4];

      uint32 stringIndex = 0;
      if(curLink->m_stringCode != MAX_UINT32){
         stringIndex = GET_STRING_INDEX(curLink->m_stringCode);
      }

      const char* tmpName = m_map->getName( stringIndex );
      formatName( tmpName ? tmpName : "",
                  roadName, m_requestedLanguage, curLink->getNameIsExitNr());

      strcpy(linkName, roadName);
      if(!curLink->m_allStringCodes.empty()){
         strcat(linkName, " (");
         bool firstName = true;
         while(!curLink->m_allStringCodes.empty()){
            if(!firstName){
               strcat(linkName, " / ");
            }
            formatName(
               m_map->getName(GET_STRING_INDEX(
                                 curLink->m_allStringCodes.front())),
                  roadName, m_requestedLanguage, curLink->getNameIsExitNr());
            strcat(linkName, roadName);
            curLink->m_allStringCodes.pop_front();
            firstName = false;
         }
            strcat(linkName, ")");
      }
      curLink->setNewName(linkName);
      delete []linkName;
      delete []roadName;
      
      prevLink = curLink;
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   }   
}


const char*
ExpandRouteProcessor::getCountryAndBuaName(StringTable::countryCode country,
                                           uint32 buaID)
{
   /*const char* countryName = StringTable::getString(
            StringTable::getCountryStringCode(country),
            StringTable::languageCode(
               m_preferedLanguages.getElementAt(0))); */
   const char* countryName =
      StringTable::getStringToDisplayForCountry(
            country,
            StringTable::languageCode(m_preferedLanguages.getElementAt(0)));
   
   if (buaID != MAX_UINT32) {
      const char* buaName = m_map->getItemName(
               buaID,
               LangTypes::language_t(m_preferedLanguages.getElementAt(0)),
               ItemTypes::invalidName);
      // the name to return
      char* name = new char[strlen(buaName) + strlen(countryName) + 3];
      strcpy(name, buaName);
      strcat(name, ", ");
      strcat(name, countryName);
      mc2dbg8 << " getCountryAndBuaName returning " << name << endl;
      return name;
   } 

   mc2dbg8 << " getCountryAndBuaName returning " << countryName << endl;
   return countryName;
}


bool
ExpandRouteProcessor::setPossibleTurns() 
{
   ExpandRouteLink* prevItem = static_cast <ExpandRouteLink*>
      (m_routeList->first());
   if(prevItem == NULL){
      mc2log << warn << "ExpRP::setPossibleTurns -- empty route list" << endl;
      return false;
   }
   
   ExpandRouteLink* thisItem = static_cast <ExpandRouteLink*>
      (prevItem->suc());
   while(thisItem != NULL){
      //currentNode = thisItem->m_item->getNodeFromID(thisItem->m_nodeID);
      Node* oppositeNode = prevItem->m_lastItem->
      getNodeFromID(prevItem->m_lastNodeID ^ 0x80000000);

      mc2dbg4 << "Check possible turns " << endl;
      // Go through the nodes that connect to this node.
      bool multiDigLeftAdded  = false; // Dont add twice
      bool multiDigRightAdded = false; // Dont add twice
      bool aheadAdded         = false; // Cant add more than one ahead/follow
      bool madeFollowRoad     = false; // Only U-turn  and exit are possible
      bool enterTurnAdded     = false; // Dont add the "wrong way" entry

      // Check the connection to this element.
      StreetSegmentItem* ssi = item_cast<StreetSegmentItem*>
         (thisItem->m_item);
      
      if(ssi != NULL && ssi->isMultiDigitised()){
         if(thisItem->m_turnDescription == StringTable::LEFT_TURN)
            multiDigLeftAdded = true;
         else if (thisItem->m_turnDescription == StringTable::RIGHT_TURN)
            multiDigRightAdded = true;
      }
      if(thisItem->m_turnDescription == StringTable::AHEAD_TURN)
      {
         aheadAdded = true;
      }
      if(thisItem->m_turnDescription == StringTable::FOLLOWROAD_TURN){
         madeFollowRoad = true;
      }
      if((thisItem->m_turnDescription == StringTable::ENTER_ROUNDABOUT_TURN)||
         (thisItem->m_turnDescription == StringTable::ON_RAMP_TURN))
      {
         enterTurnAdded = true;
      }
      
      uint16 nbrConn = oppositeNode->getNbrConnections();
      for(uint16 i = 0 ; i < nbrConn ; i++){
         Connection* conn = oppositeNode->getEntryConnection(i);
         if ( ! conn->isMultiConnection() ) { 
            // Find the opposite connection.
            Connection* fromOppNodeConn = m_map->getOpposingConnection(
                  conn, oppositeNode->getNodeID());
            
            // Add the turn description if not the coosen turn,
            // or second part of a multi dig.
            if(conn->getConnectFromNode() !=
               (thisItem->m_nodeID ^ 0x80000000)){
               
               ItemTypes::turndirection_t turn =
                  fromOppNodeConn->getTurnDirection();
               bool multiDigConn = false;
               if(madeFollowRoad){
                  if((turn == ItemTypes::UTURN) ||
                     (turn == ItemTypes::OFF_RAMP) ||
                     (turn == ItemTypes::OFF_RAMP_LEFT) ||
                     (turn == ItemTypes::OFF_RAMP_RIGHT) ){
                     thisItem->addPossibleTurn(
                        ItemTypes::getTurndirectionSC(turn));
                  }
               }
               else if((turn == ItemTypes::RIGHT) && multiDigConn ){
                  if(!multiDigRightAdded){
                     thisItem->addPossibleTurn(
                        ItemTypes::getTurndirectionSC(turn));
                  }
                  multiDigRightAdded = !multiDigRightAdded;
               }
               else if ((turn == ItemTypes::LEFT)&& multiDigConn ){
                  if(!multiDigLeftAdded){
                     thisItem->addPossibleTurn(
                        ItemTypes::getTurndirectionSC(turn));
                  }
                  multiDigLeftAdded = !multiDigLeftAdded;
               }
               else if ((turn == ItemTypes::AHEAD)||
                        (turn == ItemTypes::FOLLOWROAD))
               {
                  if(!aheadAdded){
                     thisItem->addPossibleTurn(
                        ItemTypes::getTurndirectionSC(turn));
                  }
                  aheadAdded = true;
               }
               else if ((turn == ItemTypes::ENTER_ROUNDABOUT) ||
                        (turn == ItemTypes::ON_RAMP))
               {
                  if(!enterTurnAdded) {
                     thisItem->addPossibleTurn(
                        ItemTypes::getTurndirectionSC(turn));
                  }
                  enterTurnAdded = true;
               }
               else
                  thisItem->addPossibleTurn(
                        ItemTypes::getTurndirectionSC(turn));
            } else {
               ItemTypes::turndirection_t turn =
                  fromOppNodeConn->getTurnDirection();
               mc2dbg8 << "  The made turn was found("
                       << StringTable::getString(
                          ItemTypes::getTurndirectionSC(turn),
                          StringTable::ENGLISH) 
                       <<"), ignored." << endl;
            }
         }
      }
      prevItem = thisItem;
      thisItem = static_cast <ExpandRouteLink*>(prevItem->suc());
   }
   return true;
}
   




bool
ExpandRouteProcessor::getStreetCount(Node* curNode,
                                     StreetSegmentItem* curItem,
                                     StreetSegmentItem* nextItem,
                                     byte& nbrLeft,
                                     byte& nbrRight,
                                     byte& nbrRbExits,
                                     byte& nbrRestRbExits,
                                     char* &leftStreetName,
                                     char* &rightStreetName,
                                     bool driveOnRightSide )
{
    
   // Description of some of the variables used used here.
   // This method counts the left/right/roundabout turns
   // when driving from curItem to nextItem.
   //
   // Driving direction --------------->
   //
   //         curItem                nextItem
   //             ^                     ^
   //  curNode    ^      oppositeNode   ^
   //     ^       ^          ^          ^
   //     o------------------o o-----------------o
   //      \----------------->o
   //            ^            | 
   //       fromOppNodeConn   |
   //                         |
   //                         |
   //                         |
   //                         | 
   //                         |
   //                         |
   //                         |
   //                         |
   //                         o
   //
   
   // Check the inparameters
   if (  (curNode == NULL) ||
         (curItem == NULL) || 
        (nextItem == NULL)) {
      nbrRight = nbrLeft = nbrRbExits = nbrRestRbExits = 0;
      leftStreetName = rightStreetName = NULL;
      return (false);
   }

   // 3 booleans to make sure we only increase one time/node
   bool incLeft   = false;
   bool incRight  = false;
   bool incRb     = false;
   bool incRestRb = false;
   
   // Get the opposite node to curNode.
   Node* oppositeNode = 
       m_map->nodeLookup(curNode->getNodeID() ^ 0x80000000);

   // Go through all connections leading to opposite node.
   for (uint32 i=0; i<oppositeNode->getNbrConnections(); i++) {
      // This is a connection that is connected to oppositeNode.
      // Note that this one is leading _to_ opposite node,
      // but what we need is the connection leading _from_
      // oppositeNode (ie. the opposite connection.)
      Connection* conn = oppositeNode->getEntryConnection(i);
      
      // Ok, so we get the opposite connection to conn.
      // Ie. a connection leading _from_ oppositeNode to
      // another segment (a turn).
      
      Connection* fromOppNodeConn = 
         m_map->getOpposingConnection(conn, oppositeNode->getNodeID());

      Node* fromNode = m_map->nodeLookup(conn->getConnectFromNode());
      Node* toNode = m_map->nodeLookup(fromNode->getNodeID() ^ 0x80000000);
      // We wish to include ordinary "wrong way" streets 
      // in the street count.
      // There are however exceptions to this. In the case of
      // crossing a multidigitised road, we only want to count
      // that as one crossing, not two. We don't want to 
      // include ramps that we are not allowed do drive into
      // either in the streetcount, since that is very obviously
      // not a valid turn.
     
      bool checkForFakeCrossings = false;
      if (
         //(!nextItem->isMultiDigitised()) && (!curItem->isMultiDigitised()) &&
          (!nextItem->isRoundabout()) && (!curItem->isRoundabout())// &&
          // (!nextItem->isRamp())  && (!curItem->isRamp())
          ) {
         // Only check for these "fake" crossings if the two segments
         // are normal streets. Otherwise the streetcount will be wrong
         // when driving out of a ramp, roundabout and so on... 
         checkForFakeCrossings = true;
         mc2dbg4 << "Check for fake crossings" << endl;
      }         
    
      // Make sure there exists an opposing connection (not true for
      // busroutes for instance)
      if (fromOppNodeConn != NULL) {
         // Whether this turn should be counted as a real turn or not when
         // updating street count.
         bool ignoreFakeTurn = false;
         StreetSegmentItem* ssi;
         LangTypes::language_t prefLang =
            ItemTypes::getLanguageCodeAsLanguageType(
                  StringTable::languageCode(
                     m_preferedLanguages.getElementAt(0)));
         if (/*
            
             // Ignoring transportation for theese cases.
             // They should not be counted even if walking
             
            (!fromOppNodeConn->isVehicleAllowed(ItemTypes::pedestrian) &&
               static_cast<ExpandRouteLink*>(m_routeList->first())
               ->m_transportation == ItemTypes::walk) ||
             */
            (!m_map->isVehicleAllowed(*fromOppNodeConn, ItemTypes::passengerCar))
               /*
               &&
               static_cast<ExpandRouteLink*>(m_routeList->first()) 
               ->m_transportation == ItemTypes::drive)
               */
            )
         {
            // Make sure we don't count unallowed turns in roundabouts.
            if (curItem->isRoundabout()) {
               // Get turndescription.
               
                  ignoreFakeTurn = true;
               if(toNode->getEntryRestrictions() == ItemTypes::noThroughfare){
                  StringTable::stringCode turnDesc =
                     fromOppNodeConn->getTurnStringCode();

                  if(turnDesc == StringTable::EXIT_ROUNDABOUT_TURN){
                     incRestRb = true;
                  }
                  
                  mc2dbg4 << "MM-ERP::getStreetCount() - Passing noThrougfare "
                         << "not allowed to enter, marked as restricted "
                         << endl;
                  
               }
               if(toNode->getEntryRestrictions() == ItemTypes::noRestrictions){
                  StringTable::stringCode turnDesc =
                     fromOppNodeConn->getTurnStringCode();

                  if(turnDesc == StringTable::EXIT_ROUNDABOUT_TURN){
                     incRestRb = true;
                  }
                   mc2dbg4 << "MM-ERP::getStreetCount()"
                           << " - Passing noRestrictions "
                           << "not allowed to enter, marked as restricted "
                           << endl;
               }
               
            }
            // Make sure we don't count a crossing of a multidigitalised 
            // road as one extra exit count, or a wrong-way ramp.
            else if (checkForFakeCrossings) {
               // Check the ssi that the connection is leading to
               ssi = item_cast<StreetSegmentItem*>
                  (m_map->itemLookup(conn->getConnectFromNode() & 0x7fffffff));
               // Check if it is a ramp or multi digitised road
               if ( (ssi != NULL) && 
                    ((ssi->isRamp()) || (ssi->isMultiDigitised()))) {
                  // Don't count this connection as a real turn, since
                  // there is no possibility to turn there.
                  mc2dbg4 << "Ignoring fake turn when updating streetcount"
                          << endl;
                  ignoreFakeTurn = true;
                  mc2dbg8 << "ignoring fake turn" << endl;
               }
            }
            
            
            
         }
         // Multidig T-crossings can  have a connection that only leads to
         // the other part of the multidig street. Ignore this if the
         // exit count was set to MAX_BYTE.
         if(fromOppNodeConn->getExitCount() == MAX_BYTE)
            ignoreFakeTurn = true;
         
         if (!ignoreFakeTurn) {
            
            // Get turndescription.
            StringTable::stringCode turnDesc =
               fromOppNodeConn->getTurnStringCode();
            uint32 itemID = conn->getConnectFromNode() & 0x7fffffff;
            
            if (turnDesc == StringTable::LEFT_TURN) {
               incLeft = true;
               const char* name = m_map->getBestItemName(
                  itemID, prefLang);
               if ( name != NULL ) {
                  leftStreetName = StringUtility::newStrDup(name);
               }
            } else if (turnDesc == StringTable::RIGHT_TURN) {
               incRight = true;
               const char* name = m_map->getBestItemName(
                  itemID, prefLang);
               if ( name != NULL ) {
                  rightStreetName = StringUtility::newStrDup(name);
               }
            } else if ((turnDesc == StringTable::OFF_RAMP_TURN) &&
                       m_noConcatinate){
               // Depends on driving side.
               /*   Wait with this. if used perhaps keep_L & keep_r as well,
                */
               if (driveOnRightSide) 
                  incRight = true;  
               else  
               incLeft = true;
              
            } else if (turnDesc == StringTable::EXIT_ROUNDABOUT_TURN) { 
               incRb = true;
            } else if ( ( (driveOnRightSide) &&
                          (turnDesc == StringTable::RIGHT_ROUNDABOUT_TURN) ) ||
                        ( (! driveOnRightSide) &&
                          (turnDesc == StringTable::LEFT_ROUNDABOUT_TURN) ) ) {
               incRb = true;
            }
         } // if !ignoreFakeTurn
      } // if (fromOppNodeConn != NULL) 
   } // for         
   
   // Update the outparameters if necessary
   if (incLeft) {
      nbrLeft = 1;
   }
   if (incRight) {
      nbrRight = 1;
   }
   if (incRb) {
      nbrRbExits = 1;
   }
   if (incRestRb){
      nbrRestRbExits = 1;
   }
   
   return (true);
}


void
ExpandRouteProcessor::reset(GenericMap* theMap, 
                            const ExpandRouteRequestPacket* req)
{
   mc2dbg2 << "ExpandRouteProcessor:: Expaning in map " 
           << theMap->getMapID() << endl;

   // Set the map-member
   m_map = theMap;

   // Remove all possible names
   m_possibleNames.reset();
   m_preferedLanguages.reset();

   // Reset start direction
   m_startDirOddEven = ItemTypes::unknown_oddeven_t;
   m_startDirHousenumber = ItemTypes::unknown_nbr_t;

   // Reset the list with the route-items
   m_routeList->clear();
   
   // We need to print names if they differ if this expand is not the
   // first packet.
   m_printNamesIfTheyDiffer = req->ignoreStartOffset();
   m_abbreviateStreetNames = req->getAbbreviate();

   DEBUG1(
      if (m_abbreviateStreetNames)
         mc2dbg << "Abbreviate street names = true" << endl;
      else
         mc2dbg << "Abbreviate street names = false" << endl;
   );
   
}

uint32 
ExpandRouteProcessor::getNbrPossibleNames()
{
   return (m_possibleNames.getSize());
}

int
ExpandRouteProcessor::setBestName(ExpandRouteLink* thisLink,
                                  ExpandRouteLink* prevLink, 
                                  bool manyNames)  
{
   //cerr << thisLink->m_dist << " setBestName ------" << endl;
   
   if(thisLink == NULL){ 
      mc2log << fatal << " Tried to set name to nothing ! exit"<< endl;
      abort();
      return 0; 
   }
   if((prevLink != NULL) &&
      (thisLink->m_item->hasSameNames(prevLink->m_item))){
      thisLink->m_stringCode = prevLink->m_stringCode;
      thisLink->m_allStringCodes = prevLink->m_allStringCodes;
      return (1+ thisLink->m_allStringCodes.size());
   }
   
   StringTable::stringCode turn  = thisLink->m_turnDescription;
   int nbrSetNames = 0;
   /*
    *  Two cases:
    *  1. Use same name as last item.
    *     - no new name
    *     - Ahead or follow road
    *
    *  2. Use best name acording to turn.
    *     - If no previous item.
    *     - If no common name.
    *     - If turn
    */
   set<uint32>* usedNames = new set<uint32>;
   Item* item = thisLink->m_item;
   uint32 coosenName = MAX_UINT32;
   bool rampExit = false;
   
   if((prevLink != NULL) && (prevLink->m_stringCode != MAX_UINT32) &&
      namePresent(thisLink, prevLink->m_stringCode) &&
      (  //!newName(thisLink, prevLink) ||
         (turn == StringTable::AHEAD_TURN) ||
       (turn == StringTable::FOLLOWROAD_TURN))){
      thisLink->m_stringCode = prevLink->m_stringCode;
      coosenName = prevLink->m_stringCode;
      usedNames->insert(coosenName);
      nbrSetNames++;
      
   } else {
      
      
      // Check the turns to se which name type to prefer.

      // 
      if((turn == StringTable::ON_RAMP_TURN) ||
         (turn == StringTable::AHEAD_TURN) ||
         //(turn == StringTable::KEEP_LEFT) ||
         //(turn == StringTable::KEEP_RIGHT) ||
         (turn == StringTable::FOLLOWROAD_TURN)){
         
         // Order : number, o-name a-name
         coosenName = item->getBestRawNameOfType(ItemTypes::roadNumber,
                                                 m_requestedLanguage,
                                                 usedNames);
         
         if(coosenName == MAX_UINT32){ 
            coosenName = item->getBestRawName(m_requestedLanguage,
                                              ItemTypes::officialName,
                                              usedNames);
            //cerr << " settled for " << m_map->getName(coosenName) << endl;
         } else {
            //cerr << " choose number name " << m_map->getName(coosenName)
            //<< endl;
         }
         
         
      } else {

         if((turn == StringTable::OFF_RAMP_TURN) ||
              (turn == StringTable::LEFT_OFF_RAMP_TURN) ||
              (turn == StringTable::RIGHT_OFF_RAMP_TURN)){

            // check for exitName first.
            coosenName = item->getBestRawNameOfType(ItemTypes::exitNumber,
                                                    m_requestedLanguage,
                                                    usedNames);
            rampExit = true;
            
         }
         // check for official name
         if(coosenName == MAX_UINT32){
            coosenName = item->getBestRawNameOfType(ItemTypes::officialName,
                                                    m_requestedLanguage,
                                                    usedNames);
         } else {
            //cerr << " choose exit name " << m_map->getName(coosenName)
            //   << endl;
         }
         
         // preffer alternative name
         if(coosenName == MAX_UINT32){
            coosenName = item->getBestRawName(m_requestedLanguage,
                                              ItemTypes::alternativeName,
                                              usedNames);
            //cerr << " settled for other name " << m_map->getName(coosenName)
            //    << endl;
         } else {
            //cerr << " settled for off- name " << m_map->getName(coosenName)
            //  << endl;
         }
      }
   }
   
   // Add other names to allnames.
   if(coosenName != MAX_UINT32){
      thisLink->m_stringCode = coosenName;
      //cerr << "name " << m_map->getName(GET_STRING_INDEX(coosenName)) << endl;
      nbrSetNames++;
      ItemTypes::name_t prefType;
      if((GET_STRING_TYPE(coosenName) != ItemTypes::roadNumber) &&
         !rampExit){
         prefType = ItemTypes::roadNumber;
      } else {
         if(GET_STRING_TYPE(coosenName) == ItemTypes::exitNumber)
            prefType = ItemTypes::officialName;
         else
            prefType = ItemTypes::alternativeName;
         
      }
      uint32 altName = MAX_UINT32;
      do {
         altName = item->getBestRawName(m_requestedLanguage,
                                        prefType,
                                        usedNames);
         if(altName != MAX_UINT32){
            if(GET_STRING_TYPE(altName) == ItemTypes::synonymName){
               //cerr << " (--- found synonym name, "
                //    << m_map->getName(GET_STRING_INDEX(altName))
                //    << ", ignoring) "<< endl;
               
            } else {
               nbrSetNames++;
               thisLink->m_allStringCodes.push_back(altName);
               //cerr << "(found name) "
               //     << m_map->getName(GET_STRING_INDEX(altName)) 
               //     << ", type : " << GET_STRING_TYPE(altName) << endl;
            }
         }
         } while(altName != MAX_UINT32);
   } else {
      // No name found.
      
   }
      
   usedNames->clear();
   delete usedNames;
   return nbrSetNames;
}

uint32
ExpandRouteProcessor::getStringWithType(ItemTypes::name_t strType, 
                                        LangTypes::language_t strLang,
                                        LangTypes::language_t &chosenLang,
                                        uint32 &itemNameIndex)
{
   uint32 i=0;
   uint32 retValue = MAX_UINT32;
   bool found = false;
   chosenLang = LangTypes::invalidLanguage;
   
   while ((i<m_possibleNames.getSize()) && (!found)) {
      ItemTypes::name_t curType =
         GET_STRING_TYPE(m_possibleNames.getElementAt(i));
      LangTypes::language_t curLang = 
         GET_STRING_LANGUAGE(m_possibleNames.getElementAt(i));
      uint32 curIndex = GET_STRING_INDEX(m_possibleNames.getElementAt(i));
      if (strType != ItemTypes::roadNumber) {
         if(curIndex == m_currentStringIndex){
            found = true;
            retValue = curIndex;
            itemNameIndex = m_possibleNames.getElementAt(i);
            chosenLang = curLang;
         }
         else if ( !found &&((strType == curType) || 
                             (strType == ItemTypes::invalidName)) &&
                   ((strLang == curLang) || 
                    (strLang == LangTypes::invalidLanguage)) ) {
            retValue = curIndex;
            chosenLang = curLang;
            itemNameIndex = m_possibleNames.getElementAt(i);
         }
      } else {
        // cerr << "***** choosing number name ****** " << (int)strType << "," 
         //     << (int)curType << ","<< (int)strLang << ","
         //     << (int)curLang << endl;
         // We want to use Europa roads like E22 and E6 before road 126,
         // 123 and so on...
         if ( ((strType == curType) || 
               (strType == ItemTypes::invalidName)) &&
              ((strLang == curLang) || 
               (strLang == LangTypes::invalidLanguage))
              )
         {
         //   cerr << "***** choosing number name 2 ******" << endl;
            if(curIndex == m_currentStringIndex) {
          //     cerr << " same as old" << endl;
           //    cerr << "Name at i" << (int)m_possibleNames.getElementAt(i)
            //        << endl;
               itemNameIndex = m_possibleNames.getElementAt(i);
               retValue = curIndex;
               chosenLang = curLang;
               found = true;
            }
            else if(retValue == MAX_UINT32) {
            //   cerr << " first found" << endl;
            //   cerr << "Name at i" << (int)m_possibleNames.getElementAt(i)
            //        << endl;
               itemNameIndex = m_possibleNames.getElementAt(i);
               retValue = curIndex;
               chosenLang = curLang;
            }
            else if(betterNumberName(curIndex, retValue))
            {
           //    cerr << "betterNumberName " << endl;
            //   cerr << "Name at i" << (int)m_possibleNames.getElementAt(i)
            //        << endl;
               itemNameIndex = m_possibleNames.getElementAt(i);
               retValue = curIndex;
               chosenLang = curLang;
            }
         }         
      }
      
      i++;
   }
   
   return (retValue);
}

bool
ExpandRouteProcessor::betterNumberName(uint32 newIndex,
                                       uint32 oldIndex)
{
 //  cerr << "************** betterNumberName ************" << endl;
   const char* newString = m_map->getName(newIndex);
   const char* oldString = m_map->getName(oldIndex);
   char** endptr = NULL;
   if((newString != NULL) && (oldString == NULL))
      return true;
   if(newString == NULL)
      return false;
   
   uint32 oldNumber = strtol(oldString,endptr,10);
   uint32 newNumber = strtol(newString,endptr,10);
  
   if(StringUtility::onlyDigitsInString(oldString) &&
      !StringUtility::onlyDigitsInString(newString))
      return true;
   if(!StringUtility::onlyDigitsInString(oldString) &&
      StringUtility::onlyDigitsInString(newString))
      return false;
   if(StringUtility::onlyDigitsInString(oldString) &&
      StringUtility::onlyDigitsInString(newString))
      return (newNumber < oldNumber);
   // Number roads with a character.
   // 0.M -roads
   // 1.A -roads
   // 2.B -roads
   // 3.E -roads
   // 4.U -roads
   // 5.L -roads
   // 6.K -roads
   // 7. roadnumber value
   if(oldString[0] == 'M'){
      if(newString[0] == 'M')
         return (newNumber < oldNumber);
      else
         return false;
   }
   if(oldString[0] == 'A'){
      if(newString[0] == 'M')
         return true;
      if(newString[0] == 'A')
         return (newNumber < oldNumber);
      else{
         cerr << "Old A was better than " << newString[0]  << endl;
         return false;
      }
      
   }
   if(oldString[0] == 'B'){
      if((newString[0] == 'M')|| (newString[0] == 'A'))
         return true;
      if(newString[0] == 'B')
         return (newNumber < oldNumber);
      else
         return false;
   }
   if(oldString[0] == 'U'){
      if((newString[0] == 'M')|| (newString[0] == 'A') ||
         (newString[0] == 'B'))
         return true;
      if(newString[0] == 'U')
         return (newNumber < oldNumber);
      else
         return false;
   }
   if(oldString[0] == 'E'){
      if((newString[0] == 'M')|| (newString[0] == 'A') ||
         (newString[0] == 'B') || (newString[0] == 'U')){
         cerr << "New " << newString[0]  <<" is better than old E" << endl;
         return true;
      }
      if(newString[0] == 'E')
         return (newNumber < oldNumber);
      else
         return false;
   }
   if(oldString[0] == 'L'){
      if((newString[0] == 'M')|| (newString[0] == 'A') ||
         (newString[0] == 'B') || (newString[0] == 'E') ||
         (newString[0] == 'U'))
         return true;
      if(newString[0] == 'L')
         return (newNumber < oldNumber);
      else
         return false;
   }
   if(oldString[0] == 'K'){
      if((newString[0] == 'M') || (newString[0] == 'A') ||
         (newString[0] == 'B') || (newString[0] == 'E') ||
         (newString[0] == 'U') || (newString[0] == 'L'))
         return true;
      if(newString[0] == 'K')
         return (newNumber < oldNumber);
      else
         return false;
   }
   
   return (newNumber < oldNumber);
   
}


bool 
ExpandRouteProcessor::getDriveOnRightSide( uint32 nodeID ) {
   bool driveOnRightSide = m_map->driveOnRightSide();
   if ( ( MapBits::getMapLevel( m_map->getMapID() ) >= 2 ) &&
        ( dynamic_cast<OverviewMap*> ( m_map ) != NULL ) ) 
   { 
      OverviewMap* overviewMap = static_cast<OverviewMap*> ( m_map );
      // Super overview map!
      // Cannot trust driveOnRightSide from the map since
      // the map may contain several countries.
      
      // Lookup which lower level map this node comes from.
      uint32 lowerMapID, lowerNodeID;
      overviewMap->lookupNodeID( nodeID, 
                                 lowerMapID, 
                                 lowerNodeID );
      // Get which country that map corresponds to.
      map<uint32, uint32>::const_iterator mapIt =
         m_mapIDToCountryID.find( lowerMapID );
      if ( mapIt != m_mapIDToCountryID.end() ) {
         StringTable::countryCode cc = 
            regionIDToCountryCode( mapIt->second );
         switch ( cc ) {
            // England
            case ( StringTable::ENGLAND_CC ):
               // Ireland
            case ( StringTable::IRELAND_CC ):
               mc2dbg8 << "Driving on left side, according to map "
                       << lowerMapID << " and country id "
                       << mapIt->second << endl;
               driveOnRightSide = false;
               break;
            default:
               mc2dbg8 << "Driving on right side, according to map "
                       << lowerMapID << " and country id "
                       << mapIt->second << endl;
               driveOnRightSide = true;
               break;
         }
      } else {
         mc2log << warn << "[ERP]: Couldnt lookup which country "
                << hex << "0x" << lowerMapID << dec 
                << " belongs to." << endl;
      }
   }
   return driveOnRightSide;
}

float64
ExpandRouteProcessor::getNodeAngle(const Node* node) const {
   int32 lat1, lon1, lat2, lon2;
   const Item* item = m_map->itemLookup(REMOVE_UINT32_MSB(node->getNodeID()));
   const GfxData* gfx = item->getGfxData();
   MC2_ASSERT(gfx->getNbrPolygons() == 1);
   if(GET_UINT32_MSB(node->getNodeID()) == 0){
      uint32 points = gfx->getNbrCoordinates(0);
      lat1 = gfx->getLat(0,points-2);
      lon1 = gfx->getLon(0,points-2);
      lat2 = gfx->getLat(0,points-1);
      lon2 = gfx->getLon(0,points-1);
   }else{
      lat1 = gfx->getLat(0,1);
      lon1 = gfx->getLon(0,1);
      lat2 = gfx->getLat(0,0);
      lon2 = gfx->getLon(0,0);      
   }
   return  GfxUtility:: getAngleFromNorth(lat1,lon1,lat2,lon2);
   


}

void
ExpandRouteProcessor::dumpLandmarks(){
   ExpandRouteLink* curLink = static_cast<ExpandRouteLink*>
                              (m_routeList->first());
   int i = 0;
   while(curLink != NULL){
      cerr << i++ << ") ************************************************"
           << endl;
      curLink->m_landmarks->dump();
      curLink = static_cast<ExpandRouteLink*>(curLink->suc());
   }
}
