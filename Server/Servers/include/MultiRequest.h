/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MULTI_REQUEST_H
#define MULTI_REQUEST_H

#include "config.h"

#include "Request.h"
#include "MC2String.h"

#include <map>
#include <list>
#include <vector>

/**
 *    Class that takes multiple requests and sends their packets
 *    almost simultaneously.
 */
class MultiRequest : public RequestWithStatus {
public:
   /**
    *   Creates a new MultiRequest with the supplied children.
    */
   MultiRequest( const RequestData& reqData,
                 const vector<RequestWithStatus*>& children );

   /**
    *   Deletes internal data.
    */
   virtual ~MultiRequest();

   /**
    *   Processes one packet. Sends it to the correct sub-request.
    */
   void processPacket( PacketContainer* pack );
   
   /// Returns true when all the requests are done.
   bool requestDone();

   /// Returns the status of the request - return Ok if at least one was ok.
   StringTable::stringCode getStatus() const { return m_status; }

   /**
    *    Returns a good debugname for the request.
    */
   virtual const char* getName() const {
      return m_name.c_str();
   }
   void sendSubPackets( RequestWithStatus* request );
private:
   
   /**
    *   Registers a packet container and sends it.
    */
   void registerAndSend( RequestWithStatus* req,
                         PacketContainer* cont );

   /**
    *   Sends sub-request packets and removes the requests that
    *   are done.
    */
   void sendSubPackets();

   /// Removes all children that are done
   int removeDone();

   class RequestEntry {
   public:
      /**
       *   Contains original request and packet ids for a
       *   packet sent by a request.
       */
      RequestEntry( PacketContainer* origCont,
                    RequestWithStatus* originalReq );

      /// Puts back the original ids.
      PacketContainer* changeBackIDs( PacketContainer* cont );
      /// Returns the request
      RequestWithStatus* getRequest() const;
   private:
      uint32 m_origReqID;
      uint32 m_origPackID;
      RequestWithStatus* m_origRequest;
   };

   /// Type of packetid map.
   typedef map<uint32, RequestEntry> packetIdMap_t;
   /// Map that maps received packets back to their requests.
   packetIdMap_t m_packetIDMap;
   /// Type of list for children
   typedef list<RequestWithStatus*> child_list_t;
   /// List of subrequests that are not done yet
   child_list_t m_not_done_children;
   /// List of subrequests that are done
   child_list_t m_done_requests;
   /// Status code
   StringTable::stringCode m_status;
   /// Name for debug
   MC2String m_name;
   
};

#endif
