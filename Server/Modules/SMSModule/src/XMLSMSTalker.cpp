/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "XMLSMSTalker.h"
#include "StringUtility.h"
#include "URL.h"
#include "URLFetcher.h"
#include "Utility.h"
#include "StringTable.h"

#include <sstream>

XMLSMSTalker::XMLSMSTalker(const char* xmlServerURL,
                           const char* userName,
                           const char* password)
{
   m_urlString   = xmlServerURL;
   m_userName    = StringUtility::newStrDup(userName);
   m_password    = StringUtility::newStrDup(password);
   m_transaction = 0;
   m_urlFetcher  = new URLFetcher();
   m_urlFetcher->setUserAgent( m_urlFetcher->getUserAgent() + " XMLSMSTalker");
   mc2dbg << "[XMLSMSTalker] - created" << endl;
}

XMLSMSTalker::~XMLSMSTalker()
{
   delete [] m_userName;
   delete [] m_password;
   delete m_urlFetcher;
}

bool
XMLSMSTalker::init(SMSProcessor* processor)
{
   SMSTalker::init(processor);
   URL url(m_urlString.c_str());
   if ( ! url.isValid() ) {
      mc2log << error << "[XMLSMSTalker]: Invalid url: \"" << m_urlString
             << "\"" << endl;
      return false;
   } else {
      mc2dbg << "[XMLSMSTalker]: Host : " << url.getHost() << " Port : "
             << url.getPort() << endl;
      bool useSSL = strcmp(url.getProto(), "https") == 0;
      if ( useSSL == false && ( strcmp(url.getProto(), "http") != 0 ) ) {
         mc2log << error << "[XMLSMSTalker]: Invalid protocol: "
                << url.getProto() << endl;
         return false;
      }
   }
   return true;
}

MC2String
XMLSMSTalker::makeAuth(const char* userName,
                       const char* password)
{
   return MC2String("") + 
      "<auth>\n" +
      "   <auth_user>"+userName+"</auth_user>\n" +
      "   <auth_passwd>"+password+"</auth_passwd>\n" +
      "</auth>";
}

MC2String
XMLSMSTalker::makeMessage(const byte* data,
                          int dataLength,
                          const char* phoneNumber,
                          uint32 transactionID)
{
   stringstream strstre;
   strstre << "<send_sms_request transaction_id=\"SMSModule_"
           << transactionID << "\">" << endl 
           << "  <phone_number>" << phoneNumber << "</phone_number>" << endl
           << "<smsmessage>"
           << "<![CDATA[" // CDATA begins
           << MC2String((const char*)data,dataLength)
           << "]]>" // CDATA ends
           << "</smsmessage>"
           << endl
           << "</send_sms_request>" << endl;
   MC2String retval(strstre.str());
   return retval;
}

MC2String
XMLSMSTalker::makeDocument(const MC2String& auth,
                           const MC2String& message)
{
   MC2String enc;
#ifdef MC2_UTF8
   enc = "UTF-8";
#else
   enc = "ISO-8859-1";
#endif
   stringstream strstre;
   strstre << "<?xml version=\"1.0\" encoding=\""
           << enc
           << "\" ?>"
           << endl
           << "<!DOCTYPE isab-mc2 SYSTEM \"isab-mc2.dtd\">" << endl
           << "<isab-mc2>" << endl
           << auth 
           << message
           << "</isab-mc2>" << endl;
   MC2String retval(strstre.str());
   return retval;
}

uint32
XMLSMSTalker::parseAnswer(const MC2String& answer)
{
   // FIXME: This function may or may not work correctly
   if ( answer.length() == 0 ) {
      return StringTable::WRONG_ANSWER_FROM_SMSC;
   } else {
      const char* str = answer.c_str();
      const char* statusCode = "<status_code>";
      char* status = StringUtility::strstr(str, statusCode);
      if ( status != NULL ) {
         status += strlen(statusCode);
         uint32 code = atoi(status);
         if ( code == 0 ) {
            return StringTable::OK;
         } else {
            return StringTable::ERROR_FROM_SMSC;
         }
      } else {
         return StringTable::ERROR_FROM_SMSC;
      }
   }
}


uint32
XMLSMSTalker::sendXML(const MC2String& xmlDocument)
{
   // FIXME: Use the URLFetcher instead.
   MC2String answer;
   int res = m_urlFetcher->post( answer, m_urlString.c_str(), xmlDocument );
   
   mc2dbg << "[XMLSMSTalker]: Server says " << answer << endl;
   mc2dbg << "[XMLSMSTalker]: Fetch result code = " << res << endl;
   
   if ( res < 0 ) {
      return StringTable::NO_CONTACT_WITH_SMSC;
   } else if ( res != 200 ) {
      return StringTable::WRONG_ANSWER_FROM_SMSC;
   }
   
   return parseAnswer(answer);
}


SMSSendReplyPacket*
XMLSMSTalker::sendSMS(SMSSendRequestPacket* p)
{
   mc2dbg << "[XMLSMSTalker::sendSMS]" << endl;
   uint32 startTime = TimeUtility::getCurrentTime();
   int dataLength = 0;
   byte* data = NULL;
   char* senderPhone   = NULL;
   char* recipientPhone = NULL;
   int encodingType = CONVERSION_TEXT;
   
   p->getPacketContents(encodingType, senderPhone, recipientPhone,
                        dataLength, data);
   mc2dbg << "XMLSMSTalker got an SMSReqeustPacket: " << endl
          << "\tSenderPhone = " << senderPhone << endl
          << "\tRecipientPhone = " << recipientPhone << endl
          << "\tDataLength = " << dataLength << endl
          << "\tData = " << endl;
#ifdef DEBUG_LEVEL_1
   HEXDUMP(mc2dbg, data, dataLength, "\t       ");
#endif
   mc2dbg << endl
          << "\017"; // Reset terminal if there were ugly characters.

   if ( strlen(recipientPhone) == 0 ) { // Added check for empty / pi
      mc2log << error << "No recipient phone in XMLSMSTalker"
         " - can't send SMS to no-one" << endl;
      SMSSendReplyPacket* rp = (SMSSendReplyPacket*)p;
      rp->setSubType(Packet::PACKETTYPE_SMSREPLY);
      rp->setStatus(StringTable::NO_RECIPIENT_PHONE);
      delete [] data;
      return rp; // RETURNS EARLY
   }

   // Simple check for country code.
   if ( recipientPhone[0] == '0' ) {
      mc2log << error << "Recipient phone begins with 0 - no country code"
             << endl;
      SMSSendReplyPacket* rp = (SMSSendReplyPacket*)p;
      rp->setSubType(Packet::PACKETTYPE_SMSREPLY);
      rp->setStatus(StringTable::WRONG_FORMAT_FOR_ADDRESS);
      delete [] data;
      return rp; // RETURNS EARLY      
   }
   
   // Make loginstuff.
   MC2String auth = makeAuth(m_userName, m_password);
   // Make the message
   MC2String smsMessage = makeMessage(data, dataLength, recipientPhone,
                                   m_transaction++);
   // Put together a document
   MC2String xmlDocument = makeDocument(auth, smsMessage);
   // Send the document
   uint32 status = sendXML(xmlDocument);
   
   // Create answer.
   SMSSendReplyPacket* rp = (SMSSendReplyPacket*)p;
   rp->setSubType(Packet::PACKETTYPE_SMSREPLY);
   rp->setStatus(status);

   uint32 stopTime = TimeUtility::getCurrentTime();
   mc2dbg << "[XMLSMSTalker]: time = " << (stopTime - startTime)
          << endl;

   delete [] data;
   return rp; 
}
