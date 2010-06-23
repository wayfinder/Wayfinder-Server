/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExtServiceProcessor.h"

#include "ExtServiceTalker.h"
#include "Packet.h"
#include "ExtServices.h"
#include "ExternalSearchPacket.h"
#include "ExternalItemInfoPacket.h"
#include "UserRightsMapInfo.h"
#include "ExternalSearchRequestData.h"
#include "ExternalSearchDescGenerator.h"
#include "ExtInfoQuery.h"

#include "ExternalSearchDesc.h"

#include "DeleteHelpers.h"
#include "StringTable.h"

#include "ExternalSearchHeadingDesc.h"
#include "ExtServicePacket.h"

#include <memory>

ExtServiceProcessor::
ExtServiceProcessor( MapSafeVector* loadedMaps ):
   Processor(loadedMaps),
   m_searchDescGenerator( new ExternalSearchDescGenerator() )
{
   Talkers::setupTalkers( m_talkerMap, m_talkers );
}

ExtServiceProcessor::~ExtServiceProcessor() {
   STLUtility::deleteValues( m_talkers );
}

Packet*
ExtServiceProcessor::handleRequestPacket( const RequestPacket& request,
                                          char* packetInfo )
{
   Packet* reply = NULL;

   mc2dbg4 << "EXTP::handleRequest() going to handle packet" << endl;

   switch ( request.getSubType() ) {
   case Packet::PACKETTYPE_EXTERNALSEARCH_REQUEST:
      reply = handleExternalSearchRequest( request, packetInfo );
      break;
   case Packet::PACKETTYPE_EXTERNALITEMINFO_REQUEST:
      reply = handleExternalItemInfoRequest( request, packetInfo );
      break;
   case Packet::PACKETTYPE_GFXFEATUREMAPREQUEST:
      // If we can get POIs in an area from, say Qype, add it here
      break;
   case Packet::PACKETTYPE_PERIODIC_REQUEST:
      // Periodic checkups here
      break;
   case Packet::PACKETTYPE_EXTSERVICE_LIST_REQUEST:
      reply = handleExtServiceRequest( request );
      break;
   default:
      mc2log << warn << "[EXTP]: Unknown packet "
             << request.getSubTypeAsString() << endl;
      break;
   }
   
   return reply;
}


int 
ExtServiceProcessor::getCurrentStatus() {
   return 0;
}

Packet* 
ExtServiceProcessor::handleExternalSearchRequest( const RequestPacket& request,
                                                  char* packetInfo ) {
   ReplyPacket* reply = NULL;
   // Mebbe we should move some of this.
   ExternalSearchRequestData searchData;
   const ExternalSearchRequestPacket* extReq =
      static_cast<const ExternalSearchRequestPacket*>( &request );
   UserRightsMapInfo rights;
   extReq->get( searchData, rights );

   const ExternalSearchDesc& searchDesc =
      m_searchDescGenerator->getDesc( searchData.getService(),
                                      searchData.getLang() );

   bool dontCheckValues =
      searchData.getService() == ExtService::google_local_search ||
      searchData.getService() == ExtService::qype;

   if ( dontCheckValues || searchDesc.checkParams( 
           searchData.getValues(), searchData.getService() ) ) {
      mc2dbg << "[EXTP]: SearchData ok = "
             << (dontCheckValues || searchDesc.checkParams( 
                    searchData.getValues(), searchData.getService() ) ) 
             << endl;
      
      if ( searchDesc.getService() == ExtService::not_external ||
           searchDesc.getService() == ExtService::nbr_services ) {
         mc2dbg << "[EXTP] Bad service id: " << searchDesc.getService()
                << endl;
         reply = NULL;               
      } else {
         reply = handleTalkerSearch( searchDesc.getService(),
                                     extReq, searchData,
                                     packetInfo );
      }
   } else {
      mc2dbg << "[EXTP]: SearchData NOT ok = "
             << searchDesc.checkParams( searchData.getValues(), 
                                        searchData.getService() ) << endl;
      // return empty answer, not null 
      // ( since null will make the server resend it )
      reply = new ExternalSearchReplyPacket( &request,
                                             SearchReplyData() );
   }

   return reply;
}

Packet*  
ExtServiceProcessor::handleExternalItemInfoRequest( 
   const RequestPacket& request, char* packetInfo ) {
   ReplyPacket* reply = NULL;
   uint32 service;
   MC2String externalID;
   UserRightsMapInfo rights;
   LangType lang;
   MC2Coordinate coord;
   MC2String name;

   const ExternalItemInfoRequestPacket* extReq =
      static_cast<const ExternalItemInfoRequestPacket*>( &request );

   extReq->get( service, externalID, lang, rights, coord, name );

   if ( service == ExtService::not_external ||
        service == ExtService::nbr_services ) {
      mc2dbg << "[EXTP] Bad service id: " << service << endl; 
      reply = NULL;
   } else {
      reply = handleTalkerInfoRequest( extReq, service, externalID, 
                                       lang, packetInfo,
                                       coord, name );
   }
   return reply;

}

ExtServiceTalker* 
ExtServiceProcessor::findTalker( const ExtService& service ) {
   Talkers::TalkerMap::iterator talkerIt = m_talkerMap.find( service );
   
   if ( talkerIt == m_talkerMap.end() ) {
      return NULL;
   }
   return (*talkerIt).second;
}

ReplyPacket* 
ExtServiceProcessor::handleTalkerSearch( 
   const ExtService& service,
   const ExternalSearchRequestPacket* extReq,
   const ExternalSearchRequestData& searchData,
   char* packetInfo ) 
{
   ExtServiceTalker* talker = findTalker( service );
   if ( talker == NULL ) {
      return NULL;
   }

   SearchReplyData searchReply;
   int result = talker->doQuery( searchReply, searchData, 1 );
   ReplyPacket* reply = new ExternalSearchReplyPacket( extReq,
                                                       searchReply );

   sprintf( packetInfo, talker->getServiceName().c_str() );

   if ( result < 0 ) {
      reply->setStatus( StringTable::NOTOK );
   }
   
   return reply;
}

ReplyPacket*
ExtServiceProcessor::handleTalkerInfoRequest( 
   const ExternalItemInfoRequestPacket* extReq,
   const ExtService& service, 
   const MC2String& externalID,
   const LangType& lang, char* packetInfo,
   const MC2Coordinate& coord,
   const MC2String& name ) 
{
   ExtServiceTalker* talker = findTalker( service );
   if ( talker == NULL ) {
      return NULL;
   }
   mc2dbg << "[ExtServiceProcessor] handleTalkerInfoRequest: "
          << "name: " << name << endl
          << "coord: " << coord << endl;
   SearchReplyData searchReply;
   ExtInfoQuery infoQuery( searchReply, service,
                           externalID, lang,
                           1, // #retries
                           coord, name );

   int queryRes = talker->doInfoQuery( infoQuery );

   ReplyPacket* reply = new ExternalSearchReplyPacket( extReq,
                                                       searchReply );
   sprintf( packetInfo, talker->getServiceName().c_str() );

   if ( queryRes < 0 ) {
      reply->setStatus( StringTable::NOTOK );
   }

   return reply;
}

ReplyPacket*
ExtServiceProcessor::handleExtServiceRequest( const RequestPacket& inRequest )
{
   const ExtServiceRequestPacket& request =
      static_cast< const ExtServiceRequestPacket& >( inRequest );

   ExternalSearchHeadingDesc desc;
   CompactSearchHitTypeVector headings;
   uint32 crc;
   desc.getHeadings( headings, crc );

   return new ExtServiceReplyPacket( request, headings,
                                     crc ); 
}
