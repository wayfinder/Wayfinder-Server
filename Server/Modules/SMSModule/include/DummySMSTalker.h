/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DUMMYSMSTALKER_H
#define DUMMYSMSTALKER_H

#include "config.h"
#include "SMSTalker.h"
#include "ISABThread.h"
#include "SMSProcessor.h"


#define DEBUG_SMS_SERVER

/**
 *    SMS Talker that always succeeds.
 *
 */
class DummySMSTalker : public SMSTalker, public ISABThread {

  public:
   /**
    *   Constructor
    */
   DummySMSTalker();
   
   /**
    *   Destructor
    */
   virtual ~DummySMSTalker();


   /**
    * The run function for the receiver. (thread)
    */
   virtual void run();

   
   /**
    *   Initializes the SMSTalker
    *   @return True if the initialization was succesful.
    */
   bool init(SMSProcessor* processor);

   
   /**
    *   Send an SMS.
    *   @param p A packet containing the SMS to send.
    *   @return The packet with the header changed.
    */
   SMSSendReplyPacket* sendSMS(SMSSendRequestPacket* p);

   
   /**
    *   Returns the status of the SMSTalker.
    *   @return StringTable::OK
    */
   uint32 checkStatus();

   
  private:
#ifdef DEBUG_SMS_SERVER
   vector<char*> m_vector;
#endif
   
};

#endif

