/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BOUNDINGBOX_REQUEST_H
#define BOUNDINGBOX_REQUEST_H

#include "Request.h"
#include "PacketContainer.h"
#include "AllMapPacket.h"

/** 
  *   A request that returns all boundingboxes and mapID in mapdatabase.
  *
  */

class MC2BoundingBoxRequest : public Request {
   public:
      /**
        *   Create a MC2BoundingBoxRequest.
        *   @param reqID      Unique request ID.
        *   @param p          Packet to be sent by request.
        *   @param moduletype Which module to send packet(s) to.
        */
      MC2BoundingBoxRequest( uint16 reqID,
                          AllMapRequestPacket* p,
                          moduletype_t moduletype = MODULE_TYPE_MAP );

      /**
       *    Delete this request.
       */
      virtual ~MC2BoundingBoxRequest();

      /** 
        *   Return the one and only MC2BoundingBoxPacket, then NULL.
        *   @return  The next packet that should be send for this 
        *            request.
        */
      PacketContainer* getNextPacket();

      /** 
        *   Process a packet that have been returned from the modules.
        *   Indicate request done when processed.
        *   @param   pack  The packet that is returned from the modules
        *                  and should be processed in this request.
        */
      void processPacket(PacketContainer* pack);

      /**
       *    Get the answer form this request, when done.
       *    @return  The answer to the request.
       */
      PacketContainer* getAnswer();


   private:
      /**
       */
      PacketContainer* m_request;

      /**
       *    The answer that will be returned in the getAnswer-method.
       *    Will be NULL until the answer is calculated.
       */
      PacketContainer* m_answer;
};

#endif

