/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WAYFINDERSMSREQUEST_H
#define WAYFINDERSMSREQUEST_H


#include "Types.h"
#include "config.h"
#include "Request.h"
#include "SMSSendRequest.h"
#include "MC2Coordinate.h"

/**
 *    Sends an SMS to a Wayfinder client
 *    <b>TODO</b>: Implement getStatus in SMSSendRequest and inherit
 *    from RequestWithStatus.
 *
 */
class WayfinderSMSRequest : public SMSSendRequest {

  public:

   /**
     *   Creates a new request.
     *   @param requestID   The requestID to use
     */
   WayfinderSMSRequest( uint16 requestID);

   /**
     *   Destructs the request.
     */
   virtual ~WayfinderSMSRequest();

   /**   Add a destination message
     *   @param senderService  the service to send the Wayfinder SMS via.
     *   @param recipientNumber Recipient phone number
     *   @param lat  The latitude (MC2 coordinate)
     *   @param lon  The longitude (MC2 coordinate)
     *   @param description Description
     */
   void addMessage( const char* senderService,
                    const char* recipientNumber,
                    MC2Coordinate& originCoord,
                    const char* originDescription,
                    MC2Coordinate& destCoord,
                    const char* destDescription,
                    const char* signature,
                    int32 wayfinderSMSVersion );


   /**
    *   Adds a Wayfinder favourite sms.
    */
   void addFavouriteMessage( const char* senderService,
                             const char* recipientNumber,
                             MC2Coordinate& coord,
                             MC2String& name,
                             MC2String& shortName,
                             MC2String& description,
                             MC2String& category,
                             MC2String& mapIconName,
                             const char* signature,
                             int32 wayfinderSMSVersion );
   
   /**
    * Get the last destination message made by addDestinationMessage.
    */
   const char* getLastDestinationMessage() const;


  private:

   /**
    *  Calculates the simple check number
    *  @param lat Latitude
    *  @param lon Longitude
    *
    */
   int calcCheckNum(int32 lat, int32 lon);

   enum {
      MAX_MSG_SIZE = 200,
   };
   
   char m_wayfinderMessage[MAX_MSG_SIZE+1];

};

#endif // WAYFINDERSMSREQUEST_H

