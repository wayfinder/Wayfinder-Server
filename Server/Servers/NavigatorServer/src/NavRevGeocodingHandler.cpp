/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavRevGeocodingHandler.h"
#include "NavPacket.h"
#include "UserData.h"
#include "ReverseGeocoding.h"
#include "InterfaceParserThread.h"
#include <boost/lexical_cast.hpp>

namespace {

/**
 * Converts a ReverseGeocoding::LookupStatus to a status code
 * for the navigator protocol.
 */
NavReplyPacket::ReplyStatus 
toNavStatusCode( ReverseGeocoding::LookupStatus lookupStatus ) {
   switch ( lookupStatus ) {
      case ReverseGeocoding::LOOKUP_OK:
         return NavReplyPacket::NAV_STATUS_OK;
      case ReverseGeocoding::LOOKUP_FAILED:
         return NavReplyPacket::NAV_STATUS_NOT_OK;
      case ReverseGeocoding::OUTSIDE_ALLOWED_AREA:
         return NavReplyPacket::NAV_STATUS_OUTSIDE_ALLOWED_AREA;
      case ReverseGeocoding::OUTSIDE_MAP_COVERAGE:
         return NavReplyPacket::NAV_STATUS_OUTSIDE_MAP;
      case ReverseGeocoding::TIMEOUT_ERROR:
         return NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
      default:
         return NavReplyPacket::NAV_STATUS_NOT_OK;
   }
}

/**
 * Adds a string as a parameter if it isn't empty.
 */
void addNonEmptyParam( NParamBlock& params, 
                       uint16 paramID,
                       const MC2String& str ) {
   if ( !str.empty() ) {
      params.addParam( NParam( paramID, str, false ) );
   }
}

/**
 * Adds parameters and sets status code/message in a NavReplyPacket
 * given the result of a reverse geocoding lookup.
 */
void createReply( const ReverseGeocoding::LookupResult& result,
                  NavReplyPacket* reply ) {
   reply->setStatusCode( toNavStatusCode( result.m_status ) );
   reply->setStatusMessage( result.m_errorString );

   NParamBlock& params = reply->getParamBlock();
   addNonEmptyParam( params, 1500, result.m_country );
   addNonEmptyParam( params, 1501, result.m_municipal );
   addNonEmptyParam( params, 1502, result.m_city );
   addNonEmptyParam( params, 1503, result.m_district );
   addNonEmptyParam( params, 1504, result.m_street );
   // Add top region id
   params.addParam( NParam( 1505, uint32(result.m_topRegionID) ) );

   mc2log << info << "handleRevGeocoding: Country " << result.m_country
          << " municipal " << result.m_municipal
          << " city " << result.m_city
          << " district " << result.m_district
          << " street " << result.m_street
          << " topRegionID " << result.m_topRegionID << endl;
}

}

NavRevGeocodingHandler::NavRevGeocodingHandler( InterfaceParserThread* thread,
                                                NavParserThreadGroup* group )
      : NavHandler( thread, group ) {
   m_expectations.push_back( ParamExpectation( 1400, NParam::Int32_array ) );
}

NavRevGeocodingHandler::~NavRevGeocodingHandler() {
}

bool NavRevGeocodingHandler::
handleRevGeocoding( UserItem* userItem,
                    UserEnums::URType urmask,
                    StringTable::languageCode language,
                    NavRequestPacket* pack, 
                    NavReplyPacket* reply ) {
   if ( !checkExpectations( pack->getParamBlock(), reply ) ) {
      return false;
   }

   mc2log << info << "handleRevGeocoding: ";

   // Get the position to lookup
   MC2Coordinate position;
   if ( !getParam( 1400, position, pack->getParamBlock() ) ) {
      mc2log << "The client didn't supply a valid coordinate" << endl;
      reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
      return false;
   } else {
      mc2log << "Coord " << position;
   }

   // End parameter printing
   mc2log << endl;

   // Get the allowed maps for this user
   set<uint32>* allowedMaps = NULL;
   UserUser* user = userItem->getUser();
   uint32 now = TimeUtility::getRealTime();
   bool gotMapIds = m_thread->getMapIdsForUserRegionAccess( user, 
                                                            allowedMaps, 
                                                            now,
                                                            urmask );
   auto_ptr< set<uint32> > autoMaps( allowedMaps );

   if ( !gotMapIds ) {
      reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
      reply->setStatusMessage( "Failed to get allowed area for user." );
      return false;
   }

   // Do the lookup
   ReverseGeocoding::LookupResult result = 
      ReverseGeocoding::lookup( position, 0, language, 
                                allowedMaps, m_thread );
   
   // Put the result into the NavReplyPacket
   createReply( result, reply );

   return true;
}
