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

#include "DebitMaking.h"

#include "MC2Coordinate.h"
#include "SearchRequestParameters.h"
#include "SearchRequest.h"
#include "RouteRequest.h"
// VanillaMatch
#include "SearchReplyPacket.h"
// RouteItem
#include "RoutePacket.h"
#include "PacketContainer.h"
#include "ProximityRequest.h"

#include "ExpandedRoute.h"


bool
DebitMaking::makeSearchDebit( char* operation,
                              SearchResultRequest* inReq,
                              const char* const locationString, 
                              const char* const searchString,
                              const char* extraString )
{
   if ( true /*inReq->getStatus() == StringTable::OK FIXME: Was always timeout*/ ) {
      char debitLocationStr[ 61 ];
      char debitSearchStr[ 61 ];   
      
      if ( locationString != NULL ) {
         strncpy( debitLocationStr, locationString, 60 );
         debitLocationStr[ 60 ] = '\0';
      }
      if ( searchString != NULL ) {
         strncpy( debitSearchStr, searchString, 60 );
         debitSearchStr[ 60 ] = '\0';
      }
   
      mc2dbg4 << "locationString " << locationString << endl
              << (locationString ? debitLocationStr : locationString) 
              << endl;
      sprintf( operation, 
               "SingleSearch:%d:%lu:\"%s\":\"%s\"",
               inReq->getNbrOverviewMatches(), // locationNbrMatches
               searchString ? static_cast<unsigned long>( 
                   inReq->getMatches().size() ) : 0, // NbrMatches
               locationString ? debitLocationStr : locationString,
               searchString ? debitSearchStr : searchString );
      if ( extraString &&
           strlen( extraString ) != 0 ) {
         strcat( operation, ":" );
         strcat( operation, extraString );
      }

      return true;
   } else {
      mc2log << "DebitMaking::makeSearchDebit Searchrequest not ok: " 
             << StringTable::getString( inReq->getStatus(), 
                                        StringTable::ENGLISH )
             << " (" << int(inReq->getStatus()) << ")" << endl;
      return false;
   }
}


bool 
DebitMaking::makeProximityDebit( char* operation,
                                 ProximityRequest* inReq,
                                 PacketContainer* ansCont,
                                 const char* const searchString,
                                 int32 lat, int32 lon, 
                                 uint32 distance )
{
   if ( inReq != NULL && ansCont != NULL && static_cast<ReplyPacket*>(
           ansCont->getPacket() )->getStatus() == StringTable::OK )
   {
      char debitSearchStr[ 61 ];   
      
      if ( searchString != NULL ) {
         strncpy( debitSearchStr, searchString, 60 );
         debitSearchStr[ 60 ] = '\0';
      }

      if ( ansCont->getPacket()->getSubType() == 
           Packet::PACKETTYPE_VANILLASEARCHREPLY )
      {
         uint32 nbrHits = 0;
         if ( ansCont->getPacket()->getSubType() == 
              Packet::PACKETTYPE_PROXIMITYREPLYPACKET )
         {
            ProximityReplyPacket* p = static_cast<ProximityReplyPacket*> (
               ansCont->getPacket() );
            for ( uint32 i = 0 ; i < p->getNumberMaps() ; i++ ) {
               nbrHits += p->getNumberItems( i );
            }
         } else if ( ansCont->getPacket()->getSubType() == 
              Packet::PACKETTYPE_VANILLASEARCHREPLY )
         {
            nbrHits = static_cast< VanillaSearchReplyPacket* > ( 
               ansCont->getPacket() )->getNumberOfMatches();
         }

         sprintf( operation, 
                  "ProximitySearch:C-%d-%d-%u:%u:%u:\"%s\":%u:%s:%s",
                  lat, lon, distance, nbrHits,
                  inReq->getSearchParameters().getEndHitIndex(),
                  searchString ? debitSearchStr : searchString,
                  inReq->getSearchParameters().getSearchForTypes(),
                  SearchTypes::getMatchingString(
                     inReq->getSearchParameters().getMatchType() ),
                  SearchTypes::getPartString( 
                     inReq->getSearchParameters().getStringPart() ) );

         return true;
      } else {
         mc2log << "DebitMaking::makeProximityDebit answer is of unknown "
                << "type: " << int(ansCont->getPacket()->getSubType())
                << endl;
         return false;
      }
   } else {
      mc2log << "DebitMaking::makeProximityDebit Proximity not ok: ";
      if ( inReq == NULL ) {
         mc2log << " ProximityRequest is NULL! ";
      }
      if ( ansCont == NULL ) {
         mc2log << " answer is NULL. ";
      }
      if ( ansCont != NULL && static_cast<ReplyPacket*>(
              ansCont->getPacket() )->getStatus() != StringTable::OK )
      {
         mc2log << " answer not ok: " 
                << StringTable::getString( StringTable::stringCode( 
                      static_cast<ReplyPacket*>(
                         ansCont->getPacket() )->getStatus() ), 
                                           StringTable::ENGLISH );
      }
      mc2log << endl;
      return false;
   }
}

bool 
DebitMaking::makeProximityDebit( char* operation,
                                 const SearchResultRequest* searchReq,
                                 const SearchRequestParameters& params,
                                 const char* const searchString,
                                 const MC2Coordinate& coord, 
                                 uint32 distance )
{
   if ( searchReq != NULL && searchReq->getStatus() == StringTable::OK ) {
      char debitSearchStr[ 61 ];   
      
      if ( searchString != NULL ) {
         strncpy( debitSearchStr, searchString, 60 );
         debitSearchStr[ 60 ] = '\0';
      }

      sprintf( operation, 
               "ProximitySearch:C-%d-%d-%u:%lu:%u:\"%s\":%u:%s:%s",
               coord.lat, coord.lon, distance,
               static_cast<unsigned long>( searchReq->getMatches().size() ),
               params.getNbrSortedHits(),
               searchString ? debitSearchStr : searchString,
               params.getSearchForTypes(), SearchTypes::getMatchingString(
                  params.getMatchType() ),
               SearchTypes::getPartString( 
                  params.getStringPart() ) );

         return true;     
   } else {
      mc2log << "DebitMaking::makeProximityDebit Proximity not ok: ";
      if ( searchReq == NULL ) {
         mc2log << " ProximitySearchRequest is NULL! ";
      } else if ( searchReq->getStatus() != StringTable::OK ) {
         mc2log << " answer not ok: " 
                << StringTable::getString( StringTable::stringCode( 
                      searchReq->getStatus() ), 
                                           StringTable::ENGLISH );
      }
      mc2log << endl;
      return false;
   }
}


bool
DebitMaking::makeRouteDebit( 
   char* operation,
   RouteRequest* inReq, 
   const VanillaMatch* const originVM, 
   const VanillaMatch* const destinationVM,
   uint32 rerouteID, 
   uint32 rerouteCreateTime,
   uint32 rerouteReason)
{
   char originName[ 61 ];
   char destinationName[ 61 ];

   if ( originVM != NULL ) {
      strncpy( originName, originVM->getName(), 60 );
      originName[ 60 ] = '\0';
   } else {
      originName[ 0 ] = '\0';
   }
   if ( destinationVM != NULL ) {
      strncpy( destinationName, destinationVM->getName(), 60 );
      destinationName[ 60 ] = '\0';
   } else {
      destinationName[ 0 ] = '\0';
   }
   
   ExpandedRoute* r = inReq->getExpandedRoute();

   sprintf( 
         operation, 
         "Route:O\"%s\"-%hu-%u-%u:D\"%s\"-%hu-%u-%u:%u-%u-%u:"
         "\"%s\":\"%s\":\"%s\":\"%s\":%u,%u:%u,%u:%u",
         originName,
         originVM ? originVM->getType() : 0,
         originVM ? originVM->getMapID() : 0,
         originVM ? originVM->getMainItemID() : 0,
         destinationName,
         destinationVM ? destinationVM->getType() : 0,
         destinationVM ? destinationVM->getMapID() : 0,
         destinationVM ? destinationVM->getMainItemID() : 0,
         r ? r->getTotalDistance() : 0,
         r ? r->getTotalTime() : 0,
         r ? r->getTotalStandStillTime() : 0,
         "",
         "",
         "",
         "",
         inReq->getRouteID(), inReq->getRouteCreateTime(),
         rerouteID, rerouteCreateTime, rerouteReason );

   return true;
}


bool
DebitMaking::makeRouteDebit( 
   char* operation,
   RouteRequest* inReq,
   uint32 nbrRouteItems, RouteItem** theRouteItems,
   uint32 rerouteID, 
   uint32 rerouteCreateTime,
   uint32 rerouteReason)
{
   const uint32 MAX_OP_LENGTH = 255 - 89 - 10; // Max - Stats at end - safety
   // Add "header" to result-string
   strcpy(operation, "Route");
      
   // Add origin and destinations
   char tmpStr[ 128 ];
   for ( uint32 i = 0 ; i < nbrRouteItems ; i++ ) {
      RouteItem* curRI = theRouteItems[ i ];
      sprintf( tmpStr, ":%c\"%s\"-%u-%u-%u(%d,%d)",
               curRI->getIsOrigin() ? 'O' : 'D',   // Origin or Destination
               "",                                 // StreetName
               1,                                  // type (street, company...)
               curRI->getMapID(),
               curRI->getItemID(),
               curRI->getLat(),
               curRI->getLong());
      strcat( operation, tmpStr );

      // Hack to not write outside chararray...
      if (strlen(operation) > MAX_OP_LENGTH) {
         i = MAX_UINT32 - 1; // i++ adds one...
         // break here? More clear than above i = ...
      }
   }

   ExpandedRoute* r = inReq->getExpandedRoute();

   // Add length, time etc.
   sprintf( tmpStr, ":%d-%d-%d:\"\":\"\":\"\":\"\":%u,%u:%u,%u:%u",
            r ? r->getTotalDistance() : 0,
            r ? r->getTotalTime() : 0,
            r ? r->getTotalStandStillTime() : 0,
            inReq->getRouteID(), inReq->getRouteCreateTime(),
            rerouteID, rerouteCreateTime, rerouteReason );
   strcat( operation, tmpStr );

   return true;
}


bool
DebitMaking::makeRouteDebit( char* operation, RouteRequest* inReq,
                             uint32 rerouteID, 
                             uint32 rerouteCreateTime,
                             uint32 rerouteReason) 
{
   const uint32 MAX_OP_LENGTH = 255 - 89 - 61*2; // Max - Stats at end - safety
   // Add "header" to result-string
   strcpy( operation, "Route" );
      
   // Add origins
   char tmpStr[ 128 ];
   for ( uint32 i = 0 ; i < inReq->getNbrValidOrigins(); i++ ) {
      uint32 mapID, itemID;
      const char* name;
      int32 lat, lon;
      ItemTypes::itemType type = ItemTypes::nullItem;
      if ( inReq->getValidOrigin( i, mapID, itemID, name, lat, lon, type ) ) {
         sprintf( tmpStr, ":%c\"%s\"-%u-%u-%u(%d,%d)",
                  'O',           // Origin
                  name,          // StreetName
                  type,          // type (street, company...)
                  mapID, itemID, lat, lon );
         strcat( operation, tmpStr );

         // Hack to not write outside char-array...
         if ( strlen( operation ) > MAX_OP_LENGTH ) {
            i = MAX_UINT32 - 1; // i++ adds one...
            // break here? More clear than above i = ...
         }
      }
   }

   for  (uint32 i = 0 ; i < inReq->getNbrValidDestinations() ; i++ ) {
      uint32 mapID, itemID;
      const char* name;
      int32 lat, lon;
      ItemTypes::itemType type = ItemTypes::nullItem;
      if ( inReq->getValidDestination( i, mapID, itemID, name, 
                                       lat, lon, type ) )
      {
         sprintf( tmpStr, ":%c\"%s\"-%u-%u-%u(%d,%d)",
                  'D',           // Destination
                  name,          // StreetName
                  type,          // type (street, company...)
                  mapID, itemID, lat, lon );
         strcat( operation, tmpStr );

         // Hack to not write outside char-array...
         if ( strlen( operation ) > MAX_OP_LENGTH ) {
            i = MAX_UINT32 - 1; // i++ adds one...
            // break here? More clear than above i = ...
         }
      }
   }

   ExpandedRoute* r = inReq->getExpandedRoute();

   // Add length, time etc.
   sprintf( tmpStr, ":%d-%d-%d:\"\":\"\":\"\":\"\":%u,%u:%u,%u:%u",
            r ? r->getTotalDistance() : 0,
            r ? r->getTotalTime() : 0,
            r ? r->getTotalStandStillTime() : 0,
            inReq->getRouteID(), inReq->getRouteCreateTime(),
            rerouteID, rerouteCreateTime, rerouteReason );
   strcat( operation, tmpStr );

   return true;
}

