/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDCATEGORYREQUEST_H
#define EXPANDCATEGORYREQUEST_H

#include "config.h"
#if 0
#include "SearchPacket.h"
#include "PacketContainer.h"
#include "Request.h"

/**
  *   Sends one category to the map module to expand it. It will be
  *   expanded into the subcategories or into the companies, depending
  *   on the level of the category. It is also possible to get the
  *   highest level of categories.
  *
  */
class ExpandCategoryRequest : public Request {
   public:
      /**
       *    Creates a request who owns and uses a existing 
       *    ExpandCategoryRequestPacket.
       *    @param reqID   A unique requestid.
       *    @param p       The ExpandCategoryRequestPacket to send.
       */
      ExpandCategoryRequest(uint16 reqID, 
                            ExpandCategorySearchRequestPacket* p);

      /**
       *    Get the MapRequestPacket.
       *    @return The MapRequestPacket or NULL if allready sent.
       */
      PacketContainer* getNextPacket();

      /**
       *    Handle an answer.
       *    @param pack    The answer from the modules that will be 
       *                   processed here.
       */
      void processPacket(PacketContainer *pack);

      /**
       *    Get the answer of this request.
       *    @return The answer
       */
      PacketContainer* getAnswer();
      
   private:
      /// The request with IP and Port.
      PacketContainer* m_request;
      
      /// The answer with IP and Port.
      PacketContainer* m_answer;
};
#endif // if 0
#endif // EXPANDCATEGORYREQUEST_H

