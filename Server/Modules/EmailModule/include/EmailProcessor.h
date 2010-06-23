/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EMAIL_PROCESSOR_H
#define EMAIL_PROCESSOR_H

#include "Processor.h"
#include <memory>

class SendEmailRequestPacket;
class EmailSender;

/**
  *   Processes the requestpackets that are send to the EmailModule.
  * 
  */
class EmailProcessor : public Processor {
   public:
      /**
        *   Creates a new EmailProcessor that uses the loadedMaps to
        *   communicate with the reader.
        *
        *   @param   loadedMaps  Vector used to tell the Reader about
        *                        the currently loaded maps. {\it {\bf
        *                        NB!} Since this module does not have
        *                        any maps, this is not used!}
        *   @param   emailSender The object with which emails are sent,
        *                        the EmailProcessor takes ownership of
        *                        the EmailSender and deletes it.
        */
      EmailProcessor(MapSafeVector* loadedMaps, EmailSender* emailSender );

      /**
        *   Deletes the EmailProcessor and releases the allocated 
        *   memory.
        */
      virtual ~EmailProcessor();

      
      /**
        *   Used to return the current status (load) of this processor.
        *   This is currently not used.
        *
        *   @return  The status (load) of this processor.
        */
      int getCurrentStatus();

protected:
      /**
        *   The ``main method'' that the JobThread calls to get answers
        *   to the request packets.
        *
        *   @param   p  The request packet to handle.
        *   @return  A packet that is a reply of the request given as
        *            parameter.
        */
      Packet *handleRequestPacket( const RequestPacket& p,
                                   char* packetInfo );

private:
   /**
    * @param reqPacket request packet
    * @return answer packet
    */
   Packet* handleEmailRequest( const SendEmailRequestPacket& reqPacket );

   /**
    *    The object that is used to send emails.
    */
   std::auto_ptr<EmailSender> m_emailSender;
};

#endif

