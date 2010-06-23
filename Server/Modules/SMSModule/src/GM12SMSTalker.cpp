/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <ctype.h> // isprint, isspace

#include "GM12SMSTalker.h"
#include "SMSTPDU.h"
#include "StringTable.h"
#include "StringUtility.h"
#include "SMSProcessor.h"

GM12SMSTalker::GM12SMSTalker(const char* port,
                             const char* pin) : HayesSMSTalker(port)
{
   DEBUG1(cout << "Creating GM12SMSTalker" << endl);
   
   if ( pin != NULL ) {
      m_pin = StringUtility::newStrDup(pin);
   } else {
      m_pin = NULL;
   }
   m_monitor = new JTCMonitor;   
}

GM12SMSTalker::~GM12SMSTalker()
{
   delete m_monitor;
   delete [] m_pin;
}

static inline SerialComm&
operator<<(SerialComm &serial, const char *s)
{
  serial.send(s, strlen(s));
  return serial;
}

static inline SerialComm&
operator<<(SerialComm &serial, int n)
{
  char buf[80];
  sprintf(buf, "%d", n);
  return serial << buf;
}

bool GM12SMSTalker::init(SMSProcessor* processor)
{
   DEBUG1(cout << " GM12SMSTalker::init()" << endl);
   // Open port and so on
   bool result = HayesSMSTalker::init(processor);
   if ( result )
      this->start();
   {
      JTCSynchronized synchronize(*m_monitor);
      // FIXME: Check the results of the init.
      if ( result ) {
         serial << "AT\r";
         waitForEvent ( ModemParserResult::OK );
         serial << "ATZ\r";
         waitForEvent( ModemParserResult::OK );
         serial << "ATE0\r";
         waitForEvent ( ModemParserResult::OK );
         serial << "ATE=0\r";
         waitForEvent ( ModemParserResult::OK );
         serial << "AT+CRC=1\r"; // Cellular result codes on
         waitForEvent ( ModemParserResult::OK );
         serial << "AT+CFUN=1\r"; // Telephone on
         waitForEvent ( ModemParserResult::OK );
         if ( m_pin != NULL ) {
            // FIXME: Should check if pin necessary first.
            serial << "AT+CPIN=\"" << m_pin << "\"\r";
         }
         result = waitForEvent ( ModemParserResult::OK );
         serial << "AT+CPMS=\"ME\",\"ME\"\r";
         // Warning !! This implies that the parser doesn't know
         //            about CPMS
         result |= ! waitForEvent ( ModemParserResult::OK );
         serial << "AT+CNMI=1,1\r";         
         result |= ! waitForEvent ( ModemParserResult::CNMI );
         // XXX: TODO: Add reading of this station's phonenumber
         // XXX: TODO: Set the right servicecenter number for
         //            "0709" "0706" and so on.
      }
   }
   return result;
}

size_t
packSMSAddress(
	       char *pchServiceCenter,
	       char *pchSenderPhone,
	       char *pchRecipientPhone
	       )
{
   return 0; // tmp
}

#define sendlits(x) send(x, sizeof(x))


SMSSendReplyPacket* GM12SMSTalker::sendSMS(SMSSendRequestPacket* p)
{
   static uint32 lastSMSTime = 0; // The time for the last sent SMS.
   static int smsCount = 0;

   smsCount++;
   if (smsCount > 5) {
      mc2log << info << "Resetting modem!" << endl;
      smsCount = 0;
      // copied and modified from init()
      // reset the phone since after x sms messages the phone will
      // stop sending them (error message CMS ERROR: 300)
      serial << "AT\r";
      waitForEvent ( ModemParserResult::OK );
      serial << "AT+CFUN=0\r";
      waitForEvent ( ModemParserResult::OK );
      serial << "AT\r";
      waitForEvent ( ModemParserResult::OK );
      serial << "ATZ\r";
      waitForEvent( ModemParserResult::OK );
      serial << "ATE0\r";
      waitForEvent ( ModemParserResult::OK );
      serial << "ATE=0\r";
      waitForEvent ( ModemParserResult::OK );
      serial << "AT+CRC=1\r"; // Cellular result codes on
      waitForEvent ( ModemParserResult::OK );
      serial << "AT+CFUN=1\r"; // Telephone on
      waitForEvent ( ModemParserResult::OK );
      if ( m_pin != NULL ) {
         serial << "AT+CPIN=\"" << m_pin << "\"\r";
      }
      waitForEvent ( ModemParserResult::OK );
      serial << "AT+CPMS=\"ME\",\"ME\"\r";
      waitForEvent ( ModemParserResult::OK );
      serial << "AT+CNMI=1,1\r";         
      waitForEvent ( ModemParserResult::CNMI );
   }

   uint32 diff = TimeUtility::getCurrentTime() - lastSMSTime;
   if ( diff < m_timeBetweenSMS ) {
      DEBUG1(cout << "-- Delaying SMS : " << m_timeBetweenSMS - diff
             << "msec" << endl);
      sleep( m_timeBetweenSMS - diff );
   }
   
   // Lock out the message-fetching thread.
   JTCSynchronized synchronize(*m_monitor);
   /* static int smsCounter = 0;
   if ( smsCounter++ > 4 )
   exit(0); */
   // Empty the old queue so that it won't be full of old garbage.
   clearEventQueue();

   int dataLength = 0;
   byte* data = NULL;
   char* senderPhone   = NULL;
   char* recipientPhone = NULL;
   int encodingType = CONVERSION_NO;

   p->getPacketContents(encodingType, senderPhone, recipientPhone,
                        dataLength, data);
   
   cout << "GM12SMSTalker got an SMSReqeustPacket: " << endl
        << "\tSenderPhone = " << senderPhone << endl
        << "\tRecipientPhone = " << recipientPhone << endl
        << "\tDataLength = " << dataLength << endl
        << "\tData = " << data << endl;

   const char* serviceCenter = ""; // use the default for now.
   
   SMSTPDUContainer sms(data, dataLength,
                        recipientPhone,
                        serviceCenter);

   bool success = false;
   // Send AT+CMGS=<number of octets>
   serial << "AT+CMGS=" << sms.count() << "\r";
   // Wait for the prompt ">"
   if ( waitForEvent(ModemParserResult::PROMPT) ) {
      // No timeout
      // Send the message. End with control-z
      char* buffer= new char[1024];
      char ctrlz[2];
      ctrlz[0] = char(26);
      ctrlz[1] = '\0';
      sms.getMessageAsHex(buffer, 1024);
      cout << "Message as hex:" << "\"" << buffer << "\"" << endl;
      serial << buffer;
      serial << ctrlz;
      if ( waitForEvent ( ModemParserResult::CMGS ) )
         if ( waitForEvent ( ModemParserResult::OK ) )
            success = true;
      delete [] buffer;
   }
   
   SMSSendReplyPacket* rp = (SMSSendReplyPacket*)p;
   rp->setSubType(Packet::PACKETTYPE_SMSREPLY);
   if ( success ) {
      char infostring[1024];
      sprintf(infostring, "GM12 : SMS sent OK to %s", recipientPhone);
      MC2INFO(infostring);
      rp->setStatus(StringTable::OK);
   } else {
      char infostring[1024];      
      sprintf(infostring, "GM12 : SMS error when sending to %s",
              recipientPhone);
      MC2WARNING(infostring);
      rp->setStatus(StringTable::NOTOK);
   }

   lastSMSTime = TimeUtility::getCurrentTime();
 
   delete [] data;
 
   return rp;
}


bool GM12SMSTalker::waitForEvent( ModemParserResult::result_t typeOfMess)
{
   DEBUG1(cerr << "Entering waitForEvent(ModemParserResult::result_t)"
          << endl);
   DEBUG1(cerr << "waiting for " << (long) typeOfMess << endl);
   bool retVal = false;
   GM12Event* event = waitForEvent();   
   if ( event != NULL ) {
      ModemParserResult::result_t result = event->m_parserResult.m_resultCode;
      if ( typeOfMess == result )
         retVal = true;
      delete event;
   }
   
   DEBUG1(cerr << "I will return " << retVal << endl);
   DEBUG1(cerr << "Exiting waitForEvent" << endl);
   return retVal;
}


GM12Event*
GM12SMSTalker::waitForEvent()
{
   JTCSynchronized synchronized(*this);
   DEBUG1(cerr << "Entering waitForEvent()" << endl);
   // Check if there is something in the queue already.
   GM12Event* event = NULL;
   if ( ! m_eventQueue.empty() ) {
      DEBUG1(cerr << "Found event in queue" << endl);
      event = m_eventQueue.front();
      m_eventQueue.erase( m_eventQueue.begin() );
      // NB! EARLY RETURN
      return event;
   } else {
      DEBUG1(cerr << "No event in the queue - will wait" << endl);
   }
   try {
      DEBUG1(cerr << "Waiting on eventqueue" << endl);
      wait(20000);
   } catch (JTCInterruptedException& ) {
      cerr << "Interruptedexception" << endl;
   }
   if ( ! m_eventQueue.empty() ) {
      DEBUG1(cerr << "Found event in queue" << endl);
      event = m_eventQueue.front();
      m_eventQueue.erase( m_eventQueue.begin() );
   } else {
      DEBUG1(cerr << "No event in the queue - timeout?" << endl);
   }
   DEBUG1(cerr << " Exiting waitForEvent()" << endl);
   return event;
}


void GM12SMSTalker::postEvent( GM12Event *e)
{
   JTCSynchronized synchronized(*this);
   DEBUG1(cerr << "Entering postEvent" << endl);
   DEBUG1(cerr << "Posting " << (long)(e->m_parserResult.m_resultCode)
               << endl;);
   m_eventQueue.push_back( e );
   notify();
   DEBUG1(cerr << "Exiting postEvent" << endl);
}


void GM12SMSTalker::clearEventQueue()
{
   JTCSynchronized synchronized(*this);
   DEBUG1(cerr << "clearing event queue" << endl);
   while ( ! m_eventQueue.empty() ) {
      try {
         wait(1);
         GM12Event* event = m_eventQueue.front();
         m_eventQueue.erase( m_eventQueue.begin() );
         delete event;      
      } catch (JTCInterruptedException& ) {
      }
   }
}
uint32 GM12SMSTalker::checkStatus()
{
   return StringTable::OK;
}


////////////////////////////////////
// The receiver thread            //
////////////////////////////////////

void
GM12SMSTalker::run()
{
   int n;
   const int rxbufsize = 256;
   char *rxbuf = new char[rxbufsize];  /* maybe have to be just 1 char?? */

   
   DEBUG1(cout << "GM12SMSTalker reader thread started " << endl);
   while ( ! terminated ) {
      n = readLine(rxbuf, rxbufsize);
      ModemParserResult mpResult = modemParser.parse(rxbuf);
      cout << "modemParser says: " << (long) mpResult.m_resultCode << endl;
      if ( mpResult.m_resultCode == ModemParserResult::CMTI ) {
         // Start a receiverThread
         DEBUG1(cerr << "Starting GM12ReceiverThread" << endl);
         GM12ReceiverThread* rt =
            new GM12ReceiverThread(this,
                                   m_processor,
                                   m_monitor,
                                   &serial,
                                   mpResult.m_secondParam);
         rt->start();
      } else if ( mpResult.m_resultCode != ModemParserResult::NO_MATCH ) {
         postEvent( new GM12Event(mpResult) );
      }
   }
   DEBUG1(cout << "GM12SMSTalker reader thread finished" << endl);
   delete [] rxbuf;
}

   
int
GM12SMSTalker::readLine(char* buffer, int bufLength)
{
   static const char* endOfLineIn = "\r\n>"; // Endlines and prompt
   char* origBuf = buffer;
   int  bufferPos = 0;
   char c;
   
   do {
      if ( serial.receive(&c,1) == 1) {
         // Don't add lines that start with blanks.
         if ( ! ( isspace(c) && bufferPos == 0  ) ) {
            // Don't add non-printable characters.
            if ( isprint(c) && bufferPos < bufLength ) {
               *buffer++ = c;
               bufferPos++;
            }
         }
      }
      // Skip \n\r if we haven't found anything else and break if we
      // have found anything else + \n or \r
   } while ( bufferPos == 0 ||
             ( c != endOfLineIn[0] && c != endOfLineIn[1]
               && c!= endOfLineIn[2] )  );
   
   // Make buf a null-terminated string
   *buffer = '\0';
   cout << "readLine: buffer = \"" << origBuf << '"' << endl;
   return bufferPos;
}


GM12ReceiverThread::GM12ReceiverThread(GM12SMSTalker *gm12smstalker,
                                       SMSProcessor* smsProcessor,
                                       JTCMonitor* monitor,
                                       SerialComm* serial,
                                       int memoryIndex)
{
   m_smsProcessor = smsProcessor;
   m_smsTalker    = gm12smstalker;
   m_monitor      = monitor;
   m_memoryIndex  = memoryIndex;
   m_serial       = serial;
}


void
GM12ReceiverThread::run()
{
   // Lock out sendSMS / wait for sendSMS to finish
   JTCSynchronized synchronized(*m_monitor);
   m_smsTalker->clearEventQueue();
   DEBUG1(cerr << "GM12ReceiverThread starting ..." << endl);
   *m_serial << "AT+CMGR=" << m_memoryIndex << "\r";
   m_smsTalker->waitForEvent( ModemParserResult::CMGR );
   GM12Event* pduEvent = m_smsTalker->waitForEvent();
   char* messageString = NULL;
   SMSTPDUContainer* pdu = NULL;
   if( pduEvent != NULL && pduEvent->m_parserResult.m_resultCode ==
       ModemParserResult::PDU ) {
      messageString = pduEvent->m_parserResult.m_stringParam;
      pdu = new SMSTPDUContainer(messageString);
#ifdef DEBUG_LEVEL2
      cerr << "messageString = " << messageString << endl;      
      cerr << "Telefon : " << pdu->getPhoneNumber() << endl;
      cerr << "PayloadSize = " << pdu->getPayloadSize() << endl;
      cerr << "Data ";
      for (int i = 0; i < pdu->getPayloadSize(); i++ )
         cerr << pdu->getPayload()[i];
      cerr << endl;
#endif
   }
   m_smsTalker->waitForEvent( ModemParserResult::OK );
   // Now delete the message
   *m_serial << "AT+CMGD=" << m_memoryIndex << "\r";
   m_smsTalker->waitForEvent( ModemParserResult::OK );

   if ( pdu != NULL ) {
      char infostr[1024];
      sprintf(infostr, "Got an SM from %s", pdu->getPhoneNumber());
      MC2INFO(infostr);
      SMSSendRequestPacket* srp = new SMSSendRequestPacket;
      srp->fillPacket(CONVERSION_NO,
                      pdu->getPhoneNumber(), m_smsTalker->getPhoneNumber(0),
                      pdu->getPayloadSize(), pdu->getPayload());
      
      m_smsProcessor->SMSReceived(srp);
   }
   delete pdu;
   DEBUG1(cerr << "GM12ReceiverThread exiting" << endl);
   delete pduEvent;
}



