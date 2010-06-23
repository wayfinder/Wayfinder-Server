/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ISABBOXMESSAGEFACTORY_H
#define ISABBOXMESSAGEFACTORY_H

#include "NavMessageFactory.h"

/**
 *   Class for creating messages to isabBoxes.
 *   This class is a singleton so no new is allowed, instead
 *   use the class variable Instance.
 *   Note that this class must be changed every time a new type of
 *   message is added and also when a new factory class is added.
 *
 *   If we had been smarter when creating this class we would only
 *   had two methods in the class. instance() and convertToBytes.
 *
 */
class isabBoxMessageFactory : public NavMessageFactory {

public:
   
   /**
    *   Get an instance of a factory suitable for sending
    *   messages to the type of address in address.
    *   @return        An instance of a NavMessageFactory.
    */
   static isabBoxMessageFactory* instance();


   /**
    * Creates a reply with a status.
    *
    * @return A new reply.
    */
   virtual OutGoingNavMessage* createStatusReply( 
      const NavAddress& address,
      NavMessage::MessageType type,
      uint8 status,
      NavSession* session, 
      uint8 protoVer );

protected:

   /**
    *   Creates a new isabBoxMessageFactory.
    */
   isabBoxMessageFactory();
   
private:
   
   /**
    *   The single instance of this class if any.
    */
   static isabBoxMessageFactory* m_myInstance;
   
};

#endif
