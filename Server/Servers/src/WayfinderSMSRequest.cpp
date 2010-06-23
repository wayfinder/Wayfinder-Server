/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "WayfinderSMSRequest.h"
#include "isabBoxNavMessage.h"
#include "SMSPacket.h"
#include "SMSFormatter.h"

WayfinderSMSRequest::WayfinderSMSRequest( uint16 requestID)
      : SMSSendRequest( requestID)
{
   m_wayfinderMessage[ 0 ] = '\0';
}

WayfinderSMSRequest::~WayfinderSMSRequest()
{
  // NOP for now   
}

void
WayfinderSMSRequest::addMessage( const char* senderService,
                                 const char* recipientNumber,
                                 MC2Coordinate& originCoord,
                                 const char* originDescription,
                                 MC2Coordinate& destCoord,
                                 const char* destDescription,
                                 const char* signature,
                                 int32 wayfinderSMSVersion )
{
   // Fill the message with zeroes
   memset( m_wayfinderMessage, 0, MAX_MSG_SIZE + 1 );
   
   int32 navOriginLat = isabBoxNavMessageUtil::MC2ToNavRad(originCoord.lat);
   int32 navOriginLon = isabBoxNavMessageUtil::MC2ToNavRad(originCoord.lon);
   int32 navDestLat   = isabBoxNavMessageUtil::MC2ToNavRad(destCoord.lat);
   int32 navDestLon   = isabBoxNavMessageUtil::MC2ToNavRad(destCoord.lon);

   int check = calcCheckNum(navDestLat, navDestLon);

   const char* signatureToAdd = signature ? signature : "";

   // Add this prefix for new version of SMS (Java and C++)
   const char* cykel = ( wayfinderSMSVersion == 1 ) ? "" : "//SCKL3F6B ";   
      
   if( (originCoord.lat == MAX_INT32) &&
       (originCoord.lon == MAX_INT32) ) {
      snprintf( m_wayfinderMessage, MAX_MSG_SIZE,
                "%s//WAYFDz%i %i %i %s%c%s",
                cykel,
                navDestLat, navDestLon, check, destDescription, '}',
                signatureToAdd );
   } else {
      snprintf( m_wayfinderMessage, MAX_MSG_SIZE,
                "%s//WAYFR%i %i %i %i %i %s%c%s%c%s", cykel,
                navDestLat, navDestLon, navOriginLat, navOriginLon,
                check, originDescription, '}', destDescription, '}',
                signatureToAdd );
   }
   
   SMSSendRequestPacket* p =
      new SMSSendRequestPacket(getNextPacketID(),
                               getID(), 0, 0);
   
   p->fillPacket(CONVERSION_TEXT, senderService, recipientNumber, 
                 MIN(strlen(m_wayfinderMessage), MAX_SMS_SIZE), 
                 (byte*)m_wayfinderMessage);
   
   addNewSMS(p);
}

void
WayfinderSMSRequest::addFavouriteMessage( const char* senderService,
                                          const char* recipientNumber,
                                          MC2Coordinate& coord,
                                          MC2String& name,
                                          MC2String& shortName,
                                          MC2String& description,
                                          MC2String& category,
                                          MC2String& mapIconName,
                                          const char* signature,
                                          int32 wayfinderSMSVersion)
{
   // Fill the message with zeroes.
   memset( m_wayfinderMessage, 0, MAX_MSG_SIZE + 1 );
   
   int32 navLat = isabBoxNavMessageUtil::MC2ToNavRad(coord.lat);
   int32 navLon = isabBoxNavMessageUtil::MC2ToNavRad(coord.lon);
   int check = calcCheckNum(navLat, navLon);

   const char* signatureToAdd = signature ? signature : "";
   
   // Add this prefix for new version of SMS (Java and C++)
   const char* cykel = ( wayfinderSMSVersion == 1 ) ? "" : "//SCKL3F6B ";   
   
   snprintf( m_wayfinderMessage, MAX_MSG_SIZE, "%s//WAYFF%li %li %i "
             "%s%c%s%c%s%c%s%c%s%c%s", cykel,
             (long) navLat, (long) navLon, check, name.c_str(), '}',
             shortName.c_str(), '}', description.c_str(), '}',
             category.c_str(), '}', mapIconName.c_str(), '}',
             signatureToAdd );
   
   SMSSendRequestPacket* p =
      new SMSSendRequestPacket(getNextPacketID(),
                               getID(), 0, 0);
   
   p->fillPacket(CONVERSION_TEXT, senderService, recipientNumber, 
                 MIN(strlen(m_wayfinderMessage), MAX_SMS_SIZE), 
                 (byte*)m_wayfinderMessage);
   
   addNewSMS(p);
}
                                         

const char* 
WayfinderSMSRequest::getLastDestinationMessage() const {
   return m_wayfinderMessage;
}


int 
WayfinderSMSRequest::calcCheckNum(int32 lat, int32 lon)
{
	int c;

 	c = (lat & 0xFF) ^ (lon & 0xFF) ^ (int)'W';
	return c;
}

