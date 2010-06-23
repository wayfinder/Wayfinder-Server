/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLSMSTALKER_H
#define XMLSMSTALKER_H

#include "config.h"

#include "MC2String.h"

#include "SMSTalker.h"

class SMSProcessor;
class URLFetcher;

/**
 *    SMSTalker to be used with an mc2 XMLServer.
 *
 */
class XMLSMSTalker : public SMSTalker {
public:

   /**
    *    Creates a new XMLSMSTalker with the supplied URL.
    *    Handles http and https.
    */
   XMLSMSTalker(const char* xmlServerURL,
                const char* userName,
                const char* password);

   /**
    *    Deletes internal data etc.
    */
   virtual ~XMLSMSTalker();

   /**
    *    Tries to send an SM.
    */
   SMSSendReplyPacket* sendSMS(SMSSendRequestPacket* p);

   /**
    *    Initializes the talker.
    */
   bool init(SMSProcessor* p);
   
   /**
    *   Returns the status of the XMLSMSTalker.
    */
   uint32 checkStatus() { return 0; }

   
private:

   /** 
    *    The url of the server. Used before init.
    */  
   MC2String m_urlString;

   /**
    *    Username to use.
    */
   char* m_userName;

   /**
    *    Password to use.
    */
   char* m_password;
   
   /**
    *   The transaction number.
    */
   uint32 m_transaction;

   /**
    *   URL Fetcher.
    */
   URLFetcher* m_urlFetcher;
   
private:
   /**
    *   Makes the authorization element.
    */
   static MC2String makeAuth(const char* userName,
                             const char* password);

   /**
    *   Makes the SMS message
    */
   static MC2String makeMessage(const byte* data,
                                int dataLength,
                                const char* phoneNumber,
                                uint32 transactionID);

   /**
    *
    */
   static MC2String makeDocument(const MC2String& auth,
                                 const MC2String& message);

   /**
    *   
    */
   uint32 sendXML(const MC2String& xmlDocument);

   /**
    *   Parses the answer returned by the XMLServer.
    *   @return StringTable::stringCode to put in the packet.
    */
   uint32 parseAnswer(const MC2String& answer);
   
};

#endif
