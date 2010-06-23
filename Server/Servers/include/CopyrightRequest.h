/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COPYRIGHT_REQUEST_H
#define COPYRIGHT_REQUEST_H

#include "Request.h"
#include "CopyrightHandler.h"

class LangType;

/**
 * Sends copyright request packets to mapmodule ( to all map sets )
 * and composes them into one set of copyrights and their boxes.
 */
class CopyrightRequest: public RequestWithStatus {
public:
   /// @parem requestID @see Request
   CopyrightRequest( uint32 requestID,
                     const LangType& language );
   /// Process packages received for this request
   void processPacket( PacketContainer* pack );
   /// @return copyrights and their boxes, only valid if status is OK
   const CopyrightHolder& getCopyrightHolder() const {
      return m_copyrights.getCopyrightHolder();
   }
   /// @return status
   StringTable::stringCode getStatus() const;
private:
   /// Setup a packet to send
   void setupPacketToSend( const LangType& language, uint32 mapSet );
   uint32 m_packetsLeft; ///< number of packet left to receive
   CopyrightHandler m_copyrights; ///< all copyrights from all map sets
  
};

#endif // COPYRIGHT_REQUEST_H
