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

#include "RouteRequestData.h"
#include "PacketContainer.h"
#include "RouteObject.h"
#include "RouteRequest.h"
#include "ExpandRoutePacket.h"
#include "StringTable.h"
#include "DisturbanceList.h"
#include "ExpandStringItem.h"
#include "ExpandedRoute.h"
#include "UnexpandedRoute.h"
#include "SearchMatch.h"
#include "RouteExpander.h"
#include "DisturbanceInfoPacket.h"
#include "DisturbanceDescription.h"
#include "UserData.h"
#include "RouteRequestParams.h"

#define RRU "[" << (m_user.getUser() ? m_user.getUser()->getLogonID() : "NA") << "]"

// To be able to change this into printing the state
// always use SETSTATE instead of m_state=
#define SETSTATE(x) do { mc2dbg2 << RRU << "[RR]: Changing state from " \
                          << int(m_data->m_state) << " to " \
                          << int(x) << endl; \
                         m_data->m_state = x;} while(0)

RouteRequest::RouteRequest( const RequestUserData& user,
                            uint16 reqID,
                            const RouteRequestParams& params,
                            const TopRegionRequest* topReq,
                            const DisturbanceList* disturbances,
                            Request* parentRequest) :
   RequestWithStatus( reqID ),
   m_data( NULL ),
   m_nbrDisturbanceInfoPackets( 0 ),
   m_topReq( topReq ),
   m_user( user )
{
   m_parent = parentRequest;
   if ( m_parent == this ) {
      m_parent = NULL;
   }
   m_data = new RouteRequestData( params );

}

RouteRequest::RouteRequest( const RequestUserData& user,
                            uint16 reqID,
                            uint32 expandType,
                            StringTable::languageCode language,
                            bool noConcatenate,
                            uint32 passedRoads,
                            const TopRegionRequest* topReq,
                            const DisturbanceList* disturbances,
                            Request* parentRequest,
                            uint32 nbrWantedRoutes )
      : RequestWithStatus( reqID ),
        m_data( NULL ),
        m_nbrDisturbanceInfoPackets( 0 ),
        m_topReq( topReq ),
        m_user( user )
{
   m_parent = parentRequest;
   if ( m_parent == this ) {
      m_parent = NULL;
   }
   
   RouteRequestParams params(expandType, language, noConcatenate, 
                             passedRoads, nbrWantedRoutes);
   m_data = new RouteRequestData(params);


   // Blargh!! I should make this into a contructor for
   // RouteRequestData
   m_data->m_disturbances       = disturbances;
   m_answer = NULL;

   SETSTATE( RouteRequestData::INIT );
}


RouteRequest::~RouteRequest()
{
   mc2dbg4 << RRU << "RR request done, destruct" << endl;
   delete m_data;
}

void RouteRequest::extractParams(RouteRequestParams& params) const
{
   params = RouteRequestParams(*m_data);
}


// FIXME: More checks here please.
bool 
RouteRequest::operator == ( const RouteRequest &other ) const {
   if ( m_data->m_routeObject[0] == NULL &&
        other.m_data->m_routeObject[0] == NULL ) {
      // Both are NULL - equal
      return true;
   } else if ( m_data->m_routeObject[0] == NULL ||
               other.m_data->m_routeObject[0] == NULL ) {
      // One is NULL but not both - not equal.
      return false;
   } else {
      return ( *m_data->m_routeObject[0] == *other.m_data->m_routeObject[0] );
   }
}

RouteObject*
RouteRequest::prepareRouteObject(bool expand, bool costC)
{
   // Create the routeobject. Only expand if expand is true.
   RouteObject* ro = new RouteObject( m_user,
                                      (m_parent == NULL) ? this : m_parent,
                                      expand ? m_data->m_expandType : 0,
                                      m_data->m_language,
                                      m_data->m_noConcatenate,
                                      m_data->m_passedRoads,
                                      m_topReq,
                                      true, // Abbrevation, set soon
                                      m_data->m_disturbances,
                                      m_data->m_nbrWantedRoutes,
                                      m_data->m_removeAheadIfDiff,
                                      m_data->m_nameChangeAsWP );
   
   ro->setAbbreviate( m_data->m_abbreviate );
   ro->setLandmarks( m_data->m_landmarks );
   ro->setRouteParameters( m_data->m_isStartTime,
                           0, // Deprecated route type
                           // Use cost C only on real route.
                           m_data->m_driverPrefs.getCostA(),
                           costC ? m_data->m_driverPrefs.getCostB() :
                                    1,
                           costC ? m_data->m_driverPrefs.getCostC() :
                                    0,
                           m_data->m_driverPrefs.getCostD(),
                           m_data->m_driverPrefs.getVehicleRestriction(),
                           m_data->m_time,
                           m_data->m_turnCost,
                           m_data->m_avoidTollRoads,
                           m_data->m_avoidHighways );
   ro->setAllowedMaps( m_data->m_allowedMaps );

   // Add the origins.
   for( uint32 i = 0; i < m_data->m_origins.size(); ++i ) {
      const OrigDestInfo& orig = m_data->m_origins[i].first;
      switch ( m_data->m_origins[i].second ) {
         case coord:
            ro->addOriginCoord( orig.getLat(), orig.getLon(), orig.getAngle());
            break;
         case item:
            ro->addOriginID( orig.getMapID(), orig.getNodeID(),
                             uint16(orig.getOffset() * MAX_UINT16) );
            break;
         case coordAndItem:
            ro->addOrigin( orig.getMapID(), orig.getNodeID(),
                           uint16(orig.getOffset() * MAX_UINT16),
                           orig.getLat(), orig.getLon() );
            break;
      }
   }

   // Add the destinations
   for( uint32 i = 0; i < m_data->m_dests.size(); ++i ) {
      const OrigDestInfo& dest = m_data->m_dests[i].first;
      switch ( m_data->m_dests[i].second ) {
         case coord:
            ro->addDestinationCoord( dest.getLat(), dest.getLon() );
            break;
         case item:
            ro->addDestinationID( dest.getMapID(), dest.getNodeID(),
                                  uint16(dest.getOffset() * MAX_UINT16));
            break;
         case coordAndItem:
            ro->addDestination( dest.getMapID(), dest.getNodeID(),
                                uint16(dest.getOffset() * MAX_UINT16),
                                dest.getLat(), dest.getLon() );
            break;
      }
   }
   // ro->dumpData();
   return ro;   
}

int
RouteRequest::enqueuePacketsFromRouteObject(RouteObject* robj)
{
   int nbrPacks = 0;
   for ( PacketContainer* pack = robj->getNextPacket();
         pack != NULL;
         pack = robj->getNextPacket() ) {
      // Register the packet.
      m_data->m_whoShouldGetPackets.insert(
         make_pair(pack->getPacket()->getPacketID(),
                   robj) );
      mc2dbg8 << RRU << "Packet type: " << int(pack->getPacket()->getSubType())
              << endl;
      
      enqueuePacketContainer(pack);
      ++nbrPacks;
   }
   mc2dbg8 << RRU << "[RR]: " << nbrPacks << " packets from RouteObject" 
           << endl;
   return nbrPacks;
}

int
RouteRequest::enqueuePacketsFromRouteExpander(RouteExpander* rexp)
{
   int nbrPacks = 0;
   for ( PacketContainer* pack = rexp->getNextRequest();
         pack != NULL;
         pack = rexp->getNextRequest() ) {
      
      enqueuePacketContainer(pack);
      ++nbrPacks;
   }
   mc2dbg8 << RRU << "[RR]: " << nbrPacks << " packets from RouteExpander" 
           << endl;
   return nbrPacks;
}


PacketContainer* 
RouteRequest::getNextPacket()
{
   // I will now try to convert this into a Request with
   // a queue.
   if ( m_data->m_state == RouteRequestData::INIT ) {
      // Only make one route for now. Now is then and now we can.
      if ( !m_data->m_compareDisturbanceRoute ) {
         m_data->m_nbrRouteObjectsUsed = 1;
         mc2dbg4 << RRU << " ***********  Using 1 routeObject  ********" 
                 << endl;
         SETSTATE( RouteRequestData::USING_RO );
      } else {
         m_data->m_nbrRouteObjectsUsed = 2;
         mc2dbg4 << RRU << " ***********  Using 2 routeObject  ********" 
                 << endl;
         SETSTATE( RouteRequestData::USING_TWO_RO );
      }
      
      // Get some packets from the RouteObjects.
      for ( int i = 0; i < m_data->m_nbrRouteObjectsUsed; ++i ) {
         // Only expand if one route.
         
         m_data->m_routeObject[i] = prepareRouteObject(
            false, //(m_data->m_nbrRouteObjectsUsed == 1),
            // Present traffic info even when no cost C.
            (i == 0));  
         
            RouteObject* ro = m_data->m_routeObject[i];
         
         // Check if there were zero packets or not.
         if ( enqueuePacketsFromRouteObject(ro) == 0 ) {
            // Could be StringTable::OK, but then
            // it will work anyway.
            m_data->m_status = ro->getStatus();
            SETSTATE( RouteRequestData::ERROR );
            break;
         } 
      }
   }
   else if (m_data->m_state == RouteRequestData::EXPANDING){
      //enqueuePacketsFromRouteExpander(m_routeExpander);
      if ( enqueuePacketsFromRouteObject(m_data->m_routeObject[0]) == 0 ) {
            // Could be StringTable::OK, but then
            // it will work anyway.
      //   m_data->m_status = m_data->m_routeObject[0]->getStatus();
      //   SETSTATE( RouteRequestData::ERROR );
      }      
   }
   
   return Request::getNextPacket();
}


inline void
RouteRequest::prepareExpansion()
{
   m_routeExpander = m_data->m_routeObject[0]->
      getRouteExpander(m_data->m_expandType);
   if(m_data->m_routeObject[0]->readyToExpand(m_routeExpander)){

      // Disturbances?
      if(m_disturbanceInfo.size() != 0){
         m_routeExpander->addDisturbanceInfo(m_disturbanceInfo);
         if(m_data->m_nbrRouteObjectsUsed == 2)
            m_routeExpander->setUseAddCost(true);
      }
      
      SETSTATE( RouteRequestData::EXPANDING );
   } else {
      SETSTATE( RouteRequestData::ERROR );
   }
} 


bool
RouteRequest::compareRoutes()
{
   
   // If we get here there are at least 2 routes.
   if ( m_data->m_nbrRouteObjectsUsed == 2 ) {
      UnexpandedRoute* r1 =
         m_data->m_routeObject[0]->getUnexpandedRoute();
      
      UnexpandedRoute* r2 =
         m_data->m_routeObject[1]->getUnexpandedRoute();
      return r2->compareToOriginal(*r1);
   } else {
      mc2log << warn << RRU << "RouteRequest::compareRoutes called but "
             << "have only one route!" << endl;
      return false;
   }
}

StringTable::languageCode 
RouteRequest::getLanguage() const {
   return m_data->m_routeObject[0]->getLanguage();
}


inline PacketContainer*
RouteRequest::handleExpansionState(PacketContainer* pack)
{
   
   if( pack != NULL ) {
      RouteObject* ro =
         m_data->m_whoShouldGetPackets[pack->getPacket()->getPacketID()];
      ro->processPacket( pack );
   }
   
   int nbrDone = 0;
   for ( int i = 0; i < m_data->m_nbrRouteObjectsUsed; ++i ) {
      if ( ! m_data->m_routeObject[i]->requestDone() ) {
         enqueuePacketsFromRouteObject( m_data->m_routeObject[i] );
      } else {
         ++nbrDone;
         if ( m_data->m_routeObject[i]->getStatus() != StringTable::OK ) {
            SETSTATE( RouteRequestData::ERROR );
            m_data->m_status = m_data->m_routeObject[i]->getStatus();
            break;
         }
      }
   } 
   if ( nbrDone == m_data->m_nbrRouteObjectsUsed &&
        m_data->m_state != RouteRequestData::ERROR ) {
      // All the RouteObjects are now done.
      SETSTATE( RouteRequestData::DONE );
      setOriginalOrigAndDest();   
   }
   
   return pack;
}

inline PacketContainer*
RouteRequest::handleDistState(PacketContainer* cont)
{
   if( cont != NULL ) {
      Packet* pack = cont->getPacket(); 
      if(pack->getSubType() == Packet::PACKETTYPE_DISTURBANCEINFOREPLY){
         mc2dbg2 << info << RRU << " Got DisturbanceInfoReplyPacket " << endl;
         DisturbanceInfoReplyPacket* traffPack =
            static_cast<DisturbanceInfoReplyPacket*>(pack);
         uint32 nbrOfDist = traffPack->getNumberOfDist();
         mc2dbg4 << RRU << nbrOfDist  << " disturbances in DistInfoReply" 
                 << endl;

         // Fill the vector with the info.
         int nbrFilled = traffPack->fillDisturbanceVector(m_disturbanceInfo);
         mc2dbg4 << RRU << " Filled " << nbrFilled 
                 << " disturbances with info. " << endl;
         
         m_nbrDisturbanceInfoPackets--;
         
         // Are we done?
         if(m_nbrDisturbanceInfoPackets == 0){
            // Lets prepare the expansion with this data.
            prepareExpansion();
         }
         
      } else {	
         mc2log << error << RRU << " ******** NO distPack " << endl; 
         mc2log << RRU << "SubType was :" << (int)pack->getSubType() << endl;

      }
      
   }
   return cont;
}

inline
SearchMatch*
RouteRequest::createOriginalOrigOrDest(RouteObject* routeObject, bool dest)
{
   int origDestNbr = routeObject->getOriginalOrigOrDestIndex(dest);

   if ( origDestNbr < 0 ) {
      return NULL;
   }
   
   uint32 dmapID, ditemID;
   int32 dlat;
   int32 dlon;
   uint16 doffset;
   if ( dest ) {
      routeObject->getDestination(origDestNbr, dmapID, ditemID,
                                            doffset, dlat, dlon);
   } else {
      routeObject->getOrigin(origDestNbr, dmapID, ditemID,
                                          doffset, dlat, dlon);
   }
   SearchMatch* match = SearchMatch::createMatch(SEARCH_MISC,
                                                 IDPair_t(dmapID, ditemID),
                                                 doffset);
   match->setCoords(MC2Coordinate(dlat, dlon));

   // FIXME: The names do not work in the RouteObject.
   if ( dest ) {
      match->setName(
         routeObject->getRouteDestinationName());
   } else {
      match->setName(
         routeObject->getRouteOriginName());
   }
   return match;
}

inline
void
RouteRequest::setOriginalOrigAndDest()
{
   if ( m_data->m_originalDest == NULL ) {
      m_data->m_originalDest =
         createOriginalOrigOrDest(m_data->m_routeObject[0],
                                  true);
   }

   if ( m_data->m_originalOrigin == NULL ) {
      m_data->m_originalOrigin =
         createOriginalOrigOrDest(m_data->m_routeObject[0],
                                  false);
   }
}

inline PacketContainer*
RouteRequest::handleRouteObjectState(PacketContainer* pack)
{
   mc2dbg8 << RRU << "RouteRequest::handleRouteObjectState" << endl;
   // Move this to handleRouteObjectState.
   if( pack != NULL ) {
      RouteObject* ro =
         m_data->m_whoShouldGetPackets[pack->getPacket()->getPacketID()];
      ro->processPacket( pack );
   }
   
   int nbrDone = 0;
   for ( int i = 0; i < m_data->m_nbrRouteObjectsUsed; ++i ) {
      if ( ! m_data->m_routeObject[i]->requestDone() ) {
         enqueuePacketsFromRouteObject( m_data->m_routeObject[i] );
      } else {
         ++nbrDone;
         if ( m_data->m_routeObject[i]->getStatus() != StringTable::OK ) {
            SETSTATE( RouteRequestData::ERROR );
            m_data->m_status = m_data->m_routeObject[i]->getStatus();
            break;
         }
      }
   } 
   if ( nbrDone == m_data->m_nbrRouteObjectsUsed &&
        m_data->m_state != RouteRequestData::ERROR ) {
      
      // All the RouteObjects are now done.
      //if ( nbrDone == 1 ) {
         // We only have one route object which means
         // that it should have done all the expansion
         // already.
         //m_data->m_routeObject[0]->dumpData();
      //   SETSTATE( RouteRequestData::DONE );
      //   setOriginalOrigAndDest();
      //} else {
         
         UnexpandedRoute* route = m_data->m_routeObject[0]
            ->getUnexpandedRoute();
         UnexpandedRoute* refRoute = NULL;
         
         bool twoRoutes = (nbrDone == 2);
            
         if ( twoRoutes ) {
            refRoute = m_data->m_routeObject[1]
               ->getUnexpandedRoute();
         }
         
         //detourVect = route->getDetourNodes(*refRoute);
         int nbrDist = 0;
         nbrDist = route->getDisturbedNodes(m_disturbanceInfo);
         
         mc2dbg2 << RRU << "Added " << nbrDist << " disturbances" << endl;
         if ( twoRoutes ) {
            nbrDist = route->getDetourNodes(m_disturbanceInfo, *refRoute);
            mc2dbg2 << RRU << "Added " << nbrDist << " detours" << endl;
         }
         
         mc2dbg2 << RRU << m_disturbanceInfo.size() 
                 << " disturbances in vector" << endl;
         
         if(m_disturbanceInfo.size() != 0 ){
            
            SETSTATE( RouteRequestData::DIST_INFO );
	    
            // Create the DisturbanceInfoRequest
            // The unexpanded routes differ we must ask the InfoModule for
            DisturbanceInfoRequestPacket* traffPack = NULL;
            uint32 currMap = MAX_UINT32;
            for (uint32 j = 0; j < m_disturbanceInfo.size() ; j ++){
               DisturbanceDescription distDesc = m_disturbanceInfo[j];
               uint32 thisMap = distDesc.getMapID();
               
               if(traffPack == NULL || currMap != thisMap){
                  if(traffPack != NULL ){
                     mc2dbg2 << RRU << "[RR] new map, sending old packet."
                             << endl;
                     // Send the packet with the old map.
                     traffPack->setPacketID( this->getNextPacketID() );
                     traffPack->setRequestID( this->getID() );
                     enqueuePacket(traffPack, MODULE_TYPE_TRAFFIC);
                     m_nbrDisturbanceInfoPackets++;
                  }
                  // Update map, create packet for this map.
                  currMap = thisMap;
                  traffPack =
                     new DisturbanceInfoRequestPacket( m_user.getUser(),
                                                       thisMap);
               }
               
               traffPack->addDisturbanceDescription(distDesc);
            }
            if(traffPack != NULL){ // Just in case of something..
               traffPack->setPacketID( this->getNextPacketID() );
               traffPack->setRequestID( this->getID() );	   
               enqueuePacket(traffPack, MODULE_TYPE_TRAFFIC);
               m_nbrDisturbanceInfoPackets++;
               mc2dbg2 << RRU << "[RR] sending last packet." << endl;
            }
         
            
         } else {
            // No need to check traffic info go directly to expand.
            
            // The first route must be expanded.
            // Traffic info from the info module must be included.
            //If expansion should be done, then do it.
            if ( m_data->m_expandType != 0 ) {
             prepareExpansion();
            } else {
               SETSTATE( RouteRequestData::DONE );
               setOriginalOrigAndDest();
            }         
         }
         
         //}
   }
   return pack;
}

void 
RouteRequest::processPacket( PacketContainer* pack)
{  
   mc2dbg8 << RRU << "RouteRequest::processPacket in state " 
          << (int)m_data->m_state << endl;

   switch ( m_data->m_state ) {
      // This corresponds to the old behaviour.
      case RouteRequestData::USING_RO:
      case RouteRequestData::USING_TWO_RO:
         pack = handleRouteObjectState(pack);
         break;
      case  RouteRequestData::DIST_INFO:
         pack  = handleDistState(pack);
         break;
      case RouteRequestData::EXPANDING:
         //pack = handleRouteObjectState(pack);
         pack = handleExpansionState(pack);
         break;
      case RouteRequestData::INIT:
      case RouteRequestData::DONE:
      case RouteRequestData::ERROR: 
      break;
   }
   
   // Only delete if we're not serving another request.
   if ( m_parent == NULL ) {
      delete pack;
   }
}
  
PacketContainer* 
RouteRequest::getAnswer()
{
	
   DEBUG2(
   PacketContainer* cont = m_data->m_routeObject[0]->getAnswer();
   if (cont != NULL ){
      ExpandRouteReplyPacket* reply =
         (ExpandRouteReplyPacket*)cont->getPacket();
      if( reply != NULL ){
         mc2dbg << RRU << " Packet Length : " << reply->getLength() << endl;
         mc2dbg << RRU << "Start dir " << reply->getStartDir() << " offset "
              << reply->getStartOffset() << endl;
         mc2dbg << RRU << "End dir " << reply->getEndDir() << " offset "
              << reply->getEndOffset() << endl;
         mc2dbg << RRU << "getStartDirectionHousenumber()= "
              << int(reply->getStartDirectionHousenumber()) << endl;
         mc2dbg << RRU << "getStartDirectionOddEven()="
              << int(reply->getStartDirectionOddEven()) << endl;
      }
   }
   );

   DEBUG8(
      // Get the expandRouteReplyPacket and print the crossing kinds
      ExpandRouteReplyPacket* reply = dynamic_cast<ExpandRouteReplyPacket*>
      (m_data->m_routeObject[0]->getAnswer()->getPacket());
      if (reply != NULL) {
         mc2dbg << RRU << "- - - - - - - - - - - - - - - -" << endl;
         ExpandStringItem** items = reply->getStringDataItem();
         for (uint32 i=0; i<reply->getNumStringData(); i++) {
            mc2dbg << RRU << i << " " << items[i]->getText() << " \""
                   << StringTable::getString(
                      ItemTypes::getCrossingKindSC(items[i]
                                                   ->getCrossingKind()),
                      StringTable::ENGLISH) << "\"" << endl;
         }
         mc2dbg << RRU << "- - - - - - - - - - - - - - - -" << endl;
      }         
   );
      
   if ( m_data->m_routeObject[0] == NULL ) {
      return NULL;
   } else {
      return m_data->m_routeObject[0]->getAnswer();
   }
}


ExpandedRoute* 
RouteRequest::getExpandedRoute() {
   if ( m_data->m_route == NULL && m_data->m_routeObject[0] != NULL ) {
      if ( getStatus() == StringTable::OK && 
           m_data->m_routeObject[0]->getAnswer() != NULL )
      {
         ExpandRouteReplyPacket* r = dynamic_cast<ExpandRouteReplyPacket*>
            (m_data->m_routeObject[0]->getAnswer()->getPacket()); 
         if ( r != NULL && r->getStatus() == StringTable::OK ) {
            m_data->m_route = new ExpandedRoute( r );
            m_data->m_route->setRouteID( getRouteID() );
            m_data->m_route->setRouteCreateTime( getRouteCreateTime() );

            uint32 mapID = 0;
            uint32 itemID = 0;
            const char* name = NULL;
            int32 lat = 0;
            int32 lon = 0;
            ItemTypes::itemType type = ItemTypes::nullItem;

            // Set start vehicle
            bool boolTmp = false;
            byte byteTmp = false;
            uint32 vehicleType = 0;
            uint32 uint32Tmp = 0;
            m_data->m_routeObject[0]->getRouteParameters( 
               boolTmp, byteTmp, byteTmp, byteTmp, byteTmp, byteTmp, 
               vehicleType, uint32Tmp, uint32Tmp  );
            m_data->m_route->setStartingVehicle( ItemTypes::vehicle_t( 
               vehicleType ) );

            // Add origin(s)
            for ( uint32  i = 0 ; i < getNbrValidOrigins() ; ++i ) {
               getValidOrigin( i, mapID, itemID, name, lat, lon, type );

               Name originName( name, LangTypes::english );
               ExpandedRouteMatch* origin = new ExpandedRouteMatch(
                  &originName, mapID, itemID, type, lat, lon,
                  r->getStartDirectionHousenumber(),
                  r->getStartDirectionOddEven(), 0, MAX_UINT16 );

               m_data->m_route->addOrigin( origin );
            }
            // Add destination(s)
            for ( uint32  i = 0 ; i < getNbrValidDestinations() ; ++i ) {
               getValidDestination( i, mapID, itemID, name, 
                                    lat, lon, type );

               Name destinationName( name, LangTypes::english );
               ExpandedRouteMatch* destination = new ExpandedRouteMatch(
                  &destinationName, mapID, itemID, type, lat, lon,
                  ItemTypes::unknown_nbr_t,
                  ItemTypes::unknown_oddeven_t, 0, MAX_UINT16 );

               m_data->m_route->addDestination( destination );
            }
         }

       }
   }
   return m_data->m_route;
}


void
RouteRequest::setRouteParameters(bool pIsStartTime,
                                 byte costA,
                                 byte costB,
                                 byte costC,
                                 byte costD,
                                 uint32 pVehicleParam,
                                 uint32 pTime,
                                 uint32 pTurnCost,
                                 bool pAbbreviate,
                                 bool landmarks,
                                 bool avoidTollRoads,
                                 bool avoidHighways)
{
   DEBUG4(
      mc2dbg << RRU << "Setting abbreviate to ";
      if (pAbbreviate)
         mc2dbg << "true" << endl;
      else
         mc2dbg << "false" << endl;
   );


   m_data->m_landmarks  = landmarks;
   m_data->m_abbreviate = pAbbreviate;
   m_data->m_driverPrefs.setRoutingCosts( costA, costB, costC, costD );
   m_data->m_driverPrefs.setVehicleRestriction( pVehicleParam );
   m_data->m_isStartTime = pIsStartTime;
   m_data->m_time        = pTime;
   m_data->m_turnCost    = pTurnCost;
   m_data->m_avoidTollRoads = avoidTollRoads;
   m_data->m_avoidHighways  = avoidHighways;

   // Check so that time cost is not used unless the vehicle is OK for it
   if ( costB || costC || costD ) {
      if ( (m_data->m_driverPrefs.getVehicleRestriction() &
            ItemTypes::pedestrian ) ||
           (m_data->m_driverPrefs.getVehicleRestriction() &
            ItemTypes::bicycle ) ) {
         mc2log << warn << RRU << "[RReq]: Changing cost to costA only since "
                << "vehicle is" 
                << hex << m_data->m_driverPrefs.getVehicleRestriction()
                << dec << endl;
         m_data->m_driverPrefs.setRoutingCosts( 1, 0, 0, 0 );
         // Set the costs right for next check.
         costA = 1;
         costB = 0;
         costC = 0;
         costD = 0;
      }
   }

   MapRights trafficRights = MapRights(MapRights::TRAFFIC_AND_SPEEDCAM);

   bool hasTraffic = m_user.getUser() ? (
      m_user.getUser()->hasAnyRightIn( trafficRights ) ) : true ;
   
   const char* userName = (m_user.getUser() ? m_user.getUser()->getLogonID() :
      "NO USER");
   mc2dbg << RRU << "[RR]: User " << userName
          << " has " << ( hasTraffic ? " " : " no " )
          << "traffic rights" << endl;
   
   if ( costC ) {
      //setCompareDisturbanceRoute( true );
      if ( ! hasTraffic ) {
         m_data->m_driverPrefs.setRoutingCosts( costA, 1, 0, 0 );
         mc2dbg << RRU << "[RR]: Changed cost C to costB " << endl;
      }
   }
}

void 
RouteRequest::setRouteParameters(bool pIsStartTime,
                                 RouteTypes::routeCostType pRouteCost,
                                 uint32 pVehicleParam,
                                 uint32 pTime,
                                 uint32 pTurnCost,
                                 bool pAbbreviate,
                                 bool landmarks,
                                 bool avoidTollRoads,
                                 bool avoidHighways)
{
   byte costA;
   byte costB;
   byte costC;
   byte costD;
   RouteTypes::routeCostTypeToCost(pRouteCost, costA, costB, costC, costD);
   setRouteParameters( pIsStartTime, costA, costB, costC, costD,
                       pVehicleParam, pTime, pTurnCost, pAbbreviate,
                       landmarks, avoidTollRoads, avoidHighways );
}


int
RouteRequest::addOriginID(uint32 mapID, uint32 itemID, uint16 offset)
{
   // Add the origin to temporary list, since we don't know how
   // to route yet.
   return m_data->m_origins.addItemID(mapID, itemID, offset);
}

int
RouteRequest::addOriginCoord(int32 lat, int32 lon, uint16 angle)
{
   // Add the origin to temporary list, since we don't know how
   // to route yet.
   return m_data->m_origins.addCoord(lat, lon, angle);
}

int
RouteRequest::addOrigin(uint32 mapID, uint32 itemID, 
                        uint16 offset,
                        int32 lat, int32 lon)
{
   // Add the origin to temporary list, since we don't know how
   // to route yet.
   return m_data->m_origins.addItemAndCoord(mapID, itemID, offset,
                                            lat, lon, MAX_UINT16);
}

int
RouteRequest::addOrigin( const SearchMatch& origin )
{
   // First try using the ID.
   if ( origin.getID().isValid() ) {
      uint16 offset = MAX_INT16;
      if ( origin.getType() == SEARCH_STREETS ) {
         offset =
            static_cast< const VanillaStreetMatch& > ( origin ).getOffset();
      }
      return addOriginID( origin.getMapID(), origin.getItemID(), offset );
   } else if ( origin.getCoords().isValid() ) {
      return addOriginCoord( origin.getCoords().lat,
                             origin.getCoords().lon,
                             origin.getAngle() );
   } else {
      mc2log << warn << RRU << "[RR]: addOrigin: No valid coord/id" << endl;
      return -1;
   }
}

int
RouteRequest::addDestination( const SearchMatch& dest )
{
   // First try using the ID.
   if ( dest.getID().isValid() ) {
      uint16 offset = MAX_INT16;
      if ( dest.getType() == SEARCH_STREETS ) {
         offset =
            static_cast< const VanillaStreetMatch& > ( dest ).getOffset();
      } 
      return addDestinationID( dest.getMapID(), dest.getItemID(), offset );
   } else if ( dest.getCoords().isValid() ) {
      return addDestinationCoord( dest.getCoords().lat,
                                  dest.getCoords().lon );
   } else {
      mc2log << warn << RRU << "[RR]: addDestination: No valid coord/id" 
             << endl;
      return -1;
   }
}

int
RouteRequest::addDestinationID(uint32 mapID, uint32 itemID, uint16 offset)
{
   // Add the destination to temporary list, since we don't know how
   // to route yet.
   return m_data->m_dests.addItemID(mapID, itemID, offset);
}

int
RouteRequest::addDestinationCoord(int32 lat, int32 lon, uint16 angle)
{
   // Add the destination to temporary list, since we don't know how
   // to route yet.
   // No angle for destination.
   return m_data->m_dests.addCoord(lat, lon, angle);
}

int
RouteRequest::addDestination(uint32 mapID, uint32 itemID, 
                             uint16 offset,
                             int32 lat, int32 lon)
{
   // Add the destination to temporary list, since we don't know how
   // to route yet.
   // No angle for destination.
   return m_data->m_dests.addItemAndCoord(mapID, itemID, offset,
                                          lat, lon, MAX_UINT16);
}

int
RouteRequest::addOrigOrDest( const SearchMatch& orignOrDest, bool dest )
{
   if ( dest ) {
      return addDestination( orignOrDest );
   } else {
      return addOrigin( orignOrDest );
   }
}

uint32 
RouteRequest::getNbrValidOrigins() const
{
   if (m_data->m_routeObject[0] != NULL) {
      return m_data->m_routeObject[0]->getNbrValidOrigins();
   } else {
      return 0;
   }
}

uint32 
RouteRequest::getNbrValidDestinations() const
{
   if (m_data->m_routeObject[0] != NULL) {
      return m_data->m_routeObject[0]->getNbrValidDestinations();
   } else {
      return 0;
   }
}

bool 
RouteRequest::getValidOrigin(uint32 i, 
                             uint32& mapID, 
                             uint32& itemID,
                             const char*& name,
                             int32& lat,
                             int32& lon,
                             ItemTypes::itemType& type)
{
   if (m_data->m_routeObject[0] != NULL) {
      return (m_data->m_routeObject[0]->getValidOrigin(i, mapID, itemID, 
                                              lat, lon, name, type));
   } else {
      return (false);
   }
}
         
bool 
RouteRequest::getValidDestination(uint32 i, 
                                  uint32& mapID, 
                                  uint32& itemID,
                                  const char*& name,
                                  int32& lat,
                                  int32& lon,
                                  ItemTypes::itemType& type)
{  
   if (m_data->m_routeObject[0] != NULL) {
      return (m_data->m_routeObject[0]->getValidDestination(i, mapID, itemID, 
                                                   lat, lon, name, type));
   } else {
      return (false);
   }
}


const RouteReplyPacket* 
RouteRequest::getRouteReplyPacket() 
{
   if (m_data->m_routeObject[0] != NULL)
      return (m_data->m_routeObject[0]->getRouteReplyPacket());
   else
      return (NULL);
}


void 
RouteRequest::clearTemporaryRouteData()
{
   // FIXME: [0]
   m_data->m_routeObject[0]->clearTemporaryRouteData();
}


uint32
RouteRequest::getSize()
{
   // FIXME: [0]
   return m_data->m_routeObject[0]->getSize() + 4; // The m_data->m_routeObject[0] pointer
}



const char* 
RouteRequest::getRouteOriginName() const
{
   return (m_data->m_routeObject[0]->getRouteOriginName());
}

bool 
RouteRequest::getRouteOriginCoord(int32 &lat, int32 &lon) const
{
   return (m_data->m_routeObject[0]->getRouteOriginCoord(lat, lon));
}



void 
RouteRequest::getRouteOrigin( uint32& mapID, 
                              uint32& itemID,
                              uint16& offset,
                              const char*& name ) const
{
   m_data->m_routeObject[0]->getRouteOrigin( mapID, itemID,
                                             offset, name );
}

const char* 
RouteRequest::getRouteDestinationName() const
{
   return (m_data->m_routeObject[0]->getRouteDestinationName());
}

bool 
RouteRequest::getRouteDestinationCoord(int32 &lat, int32 &lon) const
{
   return (m_data->m_routeObject[0]->getRouteDestinationCoord(lat, lon));
}


void 
RouteRequest::getRouteDestination( uint32& mapID, 
                                   uint32& itemID,
                                   uint16& offset,
                                   const char*& name ) const
{
   m_data->m_routeObject[0]->getRouteDestination( mapID, itemID,
                                                  offset, name );
}


ItemTypes::routedir_nbr_t 
RouteRequest::getStartDirectionHousenumber()
{
   return m_data->m_routeObject[0]->getStartDirectionHousenumber();
}


ItemTypes::routedir_oddeven_t 
RouteRequest::getStartDirectionOddEven()
{
   return m_data->m_routeObject[0]->getStartDirectionOddEven();
}


 uint32 
RouteRequest::getRouteID() const {
   return m_data->m_routeObject[0]->getRouteID();
}


uint32 
RouteRequest::getRouteCreateTime() const {
   return m_data->m_routeObject[0]->getRouteCreateTime();
}


void 
RouteRequest::dumpState() {
   cout << "RR::currentState: " << int(m_data->m_state) << endl;
   for ( int i = 0; i < m_data->m_nbrRouteObjectsUsed; ++i ) {
      m_data->m_routeObject[ i ]->dumpState();
   }
}

StringTable::stringCode
RouteRequest::getStatus() const
{
   if ( m_data->m_state != RouteRequestData::DONE ) {
      return m_data->m_status;
   } else {
      return StringTable::OK;
   }
   
}

ServerSubRouteVectorVector*
RouteRequest::getRoute(bool steal)
{
   return m_data->m_routeObject[0]->getRoute(steal);
}

void 
RouteRequest::setAllowedMaps( RouteAllowedMap* maps )
{
   m_data->m_allowedMaps = maps;
}

void 
RouteRequest::setRemoveAheadIfDiff( bool removeAheadIfDiff)
{
   m_data->m_removeAheadIfDiff = removeAheadIfDiff;
}

void 
RouteRequest::setNameChangeAsWP( bool nameChangeAsWP )
{
   m_data->m_nameChangeAsWP = nameChangeAsWP;
}


void 
RouteRequest::setCompareDisturbanceRoute( bool val ) {
   // Only for timed routes, hard to compare distance with time.
   if ( val && !m_data->m_driverPrefs.getCostA() ) {
      m_data->m_compareDisturbanceRoute = val;
   }
}

const SearchMatch*
RouteRequest::getOriginalRouteOrigin() const
{
   // Do an ugly trick since it is not always that the DONE-state
   // is entered when using the RouteObject.
   if ( m_data->m_originalOrigin == NULL ) {
      (const_cast<RouteRequest*>(this))->setOriginalOrigAndDest();
   }
   return m_data->m_originalOrigin;
}

const SearchMatch*
RouteRequest::getOriginalRouteDest() const
{
   // Do an ugly trick since it is not always that the DONE-state
   // is entered when using the RouteObject.
   if ( m_data->m_originalDest == NULL ) {
      (const_cast<RouteRequest*>(this))->setOriginalOrigAndDest();
   }
   return m_data->m_originalDest;
}

uint16
RouteRequest::getOriginAngle() const {
   return m_data->m_routeObject[0]->getOriginAngle();
}

uint32
RouteRequest::getNbrRouteObjectsUsed() const {
   return m_data->m_nbrRouteObjectsUsed;
}

