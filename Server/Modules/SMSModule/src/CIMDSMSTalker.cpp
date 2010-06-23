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

#include "CIMDSMSTalker.h"
#include "StringTable.h"
#include "CIMDUtil.h"
#include "StringUtility.h"
#include "SMSPacket.h"
#include "TCPSocket.h"
#include "NetUtility.h"
#include "Utility.h"

#ifdef DEBUG_LEVEL_2
   #include "StringUtility.h"
   #include <iomanip>
#endif


CIMDSMSTalker::CIMDSMSTalker(const char* ip,
                             uint16 port,
                             const char* userName,
                             const char* password,
                             const char* bindAddress)
{
   mc2dbg << "[CIMD] Creating CIMDSMSTalker" << endl;
   m_monitor = new JTCMonitor;
   if ( ip != NULL && strlen(ip) > 0 )
     m_ip      = NetUtility::iptoh(ip);
   else // Use default value.
     m_ip      = 0;

   if ( port != 0 )
     m_port    = port;
   else // Use default value.
     m_port    = 1234;
   
   m_socket  = new TCPSocket;
   m_currentPacket = 1; // This side has odd numbers.
   if ( userName == NULL || strlen(userName) == 0 )
     userName = "test";
   if ( password == NULL || strlen(password) == 0 )
     password = "login";
   m_userName = new char[strlen(userName) + 1];
   strcpy(m_userName, userName);
   m_password = new char[strlen(password) + 1];
   strcpy(m_password, password);
   mc2dbg << "[CIMD] Using IP: " << m_ip << endl
          << "[CIMD] Using port: " << m_port << endl
          << "[CIMD] Using username: " << m_userName << endl
          << "[CIMD] Using password: " << m_password << endl;
   
   if ( bindAddress ) {
      m_bindAddress = StringUtility::newStrDup(bindAddress);
   } else {
      m_bindAddress = NULL;
   }
   
   mc2dbg << "[CIMD] Will connect to " << prettyPrintIP(m_ip)
          << ":" << m_port << endl;
   if ( m_bindAddress != NULL ) {
      mc2log << info << "[CIMD] Will bind to address "
             << MC2CITE(m_bindAddress) << endl;
   }
}

CIMDSMSTalker::~CIMDSMSTalker()
{
   delete [] m_userName;
   delete [] m_password;
   delete [] m_bindAddress;
   delete m_monitor;
   delete m_socket;   
}


bool CIMDSMSTalker::init(SMSProcessor* processor)
{
   mc2dbg << "[CIMD] CIMDSMSTalker::init()" << endl;
   // Open socket and so on ...
   SMSTalker::init(processor);
   this->start();
//   bool result = reconnect();
   
//   if ( result ) {
//      result = login();
//   } else {
//      mc2dbg << "Error : Couldn't connect to service center" << endl;
//   }

//   if ( ! result )
//      mc2dbg << "Error : Could not log in" << endl;
   
   return true;
}

bool
CIMDSMSTalker::logLine(bool sending,
                       const char* senderPhone,
                       const char* recipientPhone,
                       int nbrBytes,
                       const byte* contents)
{
   JTCSynchronized synchronize(*m_monitor);
   return SMSTalker::logLine(sending, senderPhone, recipientPhone,
                             nbrBytes, contents);
}

bool
CIMDSMSTalker::reconnect()
{
   m_lastOKResponseTime = TimeUtility::getCurrentTime();
   mc2dbg << "[CIMD]: Entering reconnect()" << endl;
   static const int bufSize = 1024;
   byte* buffer = new byte[bufSize];
   m_socket->close();
   bool result = m_socket->open();

   const char* action = "opened";
   
   if ( result && m_bindAddress ) {
      mc2log << info << "[CIMD]: Socket opened - trying to bind to "
             << MC2CITE(m_bindAddress) << endl;
      result = m_socket->bind(m_bindAddress);
      action = "bound";
   }
   
   if ( result ) {
      mc2log << info << "[CIMD]: Socket " << action
             << " - trying to connect to "
             << prettyPrintIP(m_ip) << ":" << m_port << endl;
      errno = 0;
      result = m_socket->connect(m_ip, m_port);
   } else {
      mc2log << warn << "[CIMD]: Socket open failed" << endl;
   }
   if ( result ) {
      int nbrSecs = 3;
      mc2log << info << "[CIMD]: Connected to "
             << prettyPrintIP(m_ip) << ":" << m_port
             << " - waiting "
             << 2*nbrSecs << " s for server banner" << endl;
      try {
         JTCThread::sleep(nbrSecs*1000);
      } catch ( JTCInterruptedException& ) {}
      errno = 0;
      int length = m_socket->readMaxBytes(buffer, bufSize, nbrSecs*1000*1000);
      if ( length >= 0 ) {
         if ( length < 1024 ) {
            buffer[length] = '\0';
         } else {
            buffer[length-1] = '\0';
         }
         mc2log << info << "[CIMD]: Server says: \"" << buffer << "\""
                << endl;
      } else {
         if ( errno ) {
            mc2log << warn << "[CIMD]: Read failed (" << strerror(errno)
                   << ")" << endl;
         } else {
            mc2log << warn << "[CIMD]: Read failed - timeout" << endl;
         }
      }
   } else {
      mc2log << error << "[CIMD]: Connect to"
             << prettyPrintIP(m_ip) << ":" << m_port << endl
             << " failed (" << strerror(errno)
             << ')' << endl;
   }
   delete [] buffer;
   return result;
}


bool
CIMDSMSTalker::login()
{
   mc2log << info << "[CIMD]: Entering login()" << endl;
   int packetNumber = 0;
   byte buffer[256];
   {
      // Lock out the other threads.
      JTCSynchronized synchronized(*m_monitor);
      // Restart packet numbering
      resetPacketNbr();
      int length =
         CIMDUtil::makeLoginRequest((char*)buffer,
                                    packetNumber = getNextPacketNbr(),
                                    (char*)m_userName,
                                    (char*)m_password);
      // Send the request      
      clearEventQueue();
      CIMDEvent* event = sendAndWait(m_socket, buffer, length, packetNumber);
      bool retVal = false;
      if ( event != NULL &&
           event->m_parserResult.m_resultCode == CIMDParserResult::RESP_LOGIN)
         retVal = true;

      if ( retVal && event->m_parserResult.m_packetNumber == packetNumber)
         retVal = true;
      else
         retVal = false;
      
      delete event;
      return retVal;
   }
}


SMSSendReplyPacket*
CIMDSMSTalker::sendSMS(SMSSendRequestPacket* p)
{
   yield();
   // Lock out the message-fetching thread.
   JTCSynchronized synchronize(*m_monitor);
   uint32 startTime = TimeUtility::getCurrentTime();
   clearEventQueue();
   int dataLength = 0;
   byte* data = NULL;
   char* senderPhone   = NULL;
   char* recipientPhone = NULL;
   int encodingType = CONVERSION_NO;
   int smsNbr = -1;
   int nbrParts = -1;
   int partNbr = -1;
   SMSSendRequestPacket::smsType_t smsType;
   static const int bufSize = 1024*10;
   char buffer[bufSize];
   int pktNbr = 0;
   char* href = NULL;
   char* message = NULL;

   smsType = p->getSMSType();

   switch (smsType) {
      case SMSSendRequestPacket::SMSTYPE_TEXT:
      case SMSSendRequestPacket::SMSTYPE_TEXT_HEX:
         p->getPacketContents(encodingType,
                              senderPhone,
                              recipientPhone,
                              dataLength,
                              data,
                              smsNbr,
                              nbrParts,
                              partNbr);

         logLine(true, senderPhone, recipientPhone, dataLength, data);

#ifdef DEBUG_LEVEL_1
         mc2dbg1 << "CIMDSMSTalker got an SMSRequestPacket: " << endl
                 << "\tSenderPhone = " << senderPhone << endl
                 << "\tRecipientPhone = " << recipientPhone << endl
                 << "\tDataLength = " << dataLength << endl
                 << "\tData = " << endl;
         HEXDUMP(mc2dbg1, data, dataLength, "\t       ");
         mc2dbg1 << endl
                 << "\017"; // Reset terminal if there were ugly characters.
#endif
         // check length
         if ( ((smsType == SMSSendRequestPacket::SMSTYPE_TEXT) 
               && (dataLength > 160))  ||
              ((smsType == SMSSendRequestPacket::SMSTYPE_TEXT_HEX)
               && (dataLength > 320)) ) {
            mc2log << error << "[CIMD] Too long SMS. Can't send more than "
                   << "160 characters" << endl;
            SMSSendReplyPacket* rp = (SMSSendReplyPacket*)p;
            rp->setSubType(Packet::PACKETTYPE_SMSREPLY);
            rp->setStatus(StringTable::MESSAGE_TOO_LONG);
            delete [] data;
            return rp; // RETURNS EARLY
         }
      break;
      case SMSSendRequestPacket::SMSTYPE_WAP_PUSH_SI:
         p->getPacketContents(senderPhone,
                              recipientPhone,
                              href,
                              message);

         logLine(true, senderPhone, recipientPhone, 11, (byte*)"WAP_PUSH_SI");

         mc2dbg1 << "[CIMD] Got an SMSRequestPacket with WAP Push SI: " << endl
                 << "\tSenderPhone = " << senderPhone << endl
                 << "\tRecipientPhone = " << recipientPhone << endl
                 << "\tmessage = " << message << endl
                 << "\thref = " << href << endl;
      break;
      default:
         mc2log << error << "[CIMD] Unknown sms type (" << smsType << ")!" 
                << endl;
      break;
   }

   // error checks that aren't SMS type specific
   if ( strlen(recipientPhone) == 0 ) {
      mc2log << error << "No recipient phone in CIMDSMSTalker "
                         "- can't send SMS to no-one" << endl;
      SMSSendReplyPacket* rp = (SMSSendReplyPacket*)p;
      rp->setSubType(Packet::PACKETTYPE_SMSREPLY);
      rp->setStatus(StringTable::NO_RECIPIENT_PHONE);
      delete [] data;
      return rp; // RETURNS EARLY
   }
   // Simple check for country code.
   if ( recipientPhone[0] == '0' ) {
      mc2log << error << "[CIMD]: Recipient phone begins with 0 - no "
                         "country code" << endl;
      SMSSendReplyPacket* rp = (SMSSendReplyPacket*)p;
      rp->setSubType(Packet::PACKETTYPE_SMSREPLY);
      rp->setStatus(StringTable::WRONG_FORMAT_FOR_ADDRESS);
      delete [] data;
      return rp; // RETURNS EARLY      
   }
   int length = -1;
   switch (smsType) {
      case SMSSendRequestPacket::SMSTYPE_TEXT:
      case SMSSendRequestPacket::SMSTYPE_TEXT_HEX:
         length = CIMDUtil::makeSendRequestText(buffer,
                                                pktNbr=getNextPacketNbr(),
                                                recipientPhone,
                                                data,
                                                dataLength,
                                                smsNbr,
                                                nbrParts,
                                                partNbr,
                                                smsType);
      break;
      case SMSSendRequestPacket::SMSTYPE_WAP_PUSH_SI:
         length = CIMDUtil::makeSendRequestWAPPushSI(buffer,
                                                pktNbr=getNextPacketNbr(),
                                                recipientPhone,
                                                href,
                                                message,
                                                smsNbr,
                                                nbrParts,
                                                partNbr);
      break;
   }

   if ( length == -1) {
      // Rewind packet number!! Or else the CIMD will be confused.
      m_currentPacket -= 2;
      mc2log << error << "[CIMD]: SMS could not be formatted for sending "
             << " (rewinding packet number)" << endl;
      SMSSendReplyPacket* rp = (SMSSendReplyPacket*)p;
      rp->setSubType(Packet::PACKETTYPE_SMSREPLY);
      rp->setStatus(StringTable::NOTOK);
      delete [] data;
      return rp; // RETURNS EARLY      
   }
   if ( length == -2) {  // can be returned by makeSendRequestWAPPushSI
      // Rewind packet number!! Or else the CIMD will be confused.
      m_currentPacket -= 2;      
      mc2log << error << "[CIMD]: Wap Push SI message is too long, could not "
             << "be sent"
             << " (rewinding packet number)" << endl;
      SMSSendReplyPacket* rp = (SMSSendReplyPacket*)p;
      rp->setSubType(Packet::PACKETTYPE_SMSREPLY);
      rp->setStatus(StringTable::MESSAGE_TOO_LONG);
      delete [] data;
      return rp; // RETURNS EARLY      
   }
   
   bool success = false;
   CIMDEvent* event = sendAndWait(m_socket, (byte*)buffer, length, pktNbr);
   
   if ( event != NULL ) {
      m_lastOKResponseTime = TimeUtility::getCurrentTime();
      if ( event->m_parserResult.m_error != 0 ) {
         mc2log << warn << "SMSC Says ERROR: " 
                << event->m_parserResult.m_error
                << " - "
                << CIMDUtil::errorToText(event->m_parserResult.m_error)
                << endl;
         if ( event->m_parserResult.m_error == 1 )
            mc2log << warn
                   << "   Maybe someone else is logged in. Look for 900:103"
                   << " in the log" << endl;
      } else if ( event->m_parserResult.m_resultCode ==
                  CIMDParserResult::RESP_SUBMIT) {
         // TODO: Add check for replynumber here
         success = true;
      }
      delete event;
   }
   
   SMSSendReplyPacket *rp = (SMSSendReplyPacket*)p;
   rp->setSubType(Packet::PACKETTYPE_SMSREPLY);
   if ( success )
      rp->setStatus(StringTable::OK);
   else
      rp->setStatus(StringTable::NOTOK);
   
   mc2dbg << "Sending took " << TimeUtility::getCurrentTime() - startTime
          << "ms" << endl;
   delete [] data;
   return rp;
}


CIMDEvent*
CIMDSMSTalker::sendAndWait(TCPSocket* socket,
                           byte* buffer,
                           int bufLen,
                           int &pktNumber)
{
   int nbrRetries = 0;
   bool noNack = false;
#ifdef DEBUG_LEVEL_4
   mc2dbg4 << "CIMDSMSTalker, sendAndWait(), going to send:" << endl;
   HEXDUMP(mc2dbg2, buffer, bufLen, "\t       ");
#endif
   while ( nbrRetries < 3 && ! noNack ) {
      errno = 0;
      int socketRes = socket->writeAll( buffer, bufLen );
      // If the write failed - return NULL
      if ( socketRes < 0 ) {
         mc2dbg << "[CIMD]: sendAndWait(...) - write returned -1 ("
                << strerror(errno) << ")" << endl;
         return NULL; // NB. Early return.
      }    
#ifdef DEBUG_LEVEL_2
      mc2dbg << "Send : ";
      for ( int duke = 0; duke < socketRes; duke++)
        if ( buffer[duke] != '\r' )
          mc2dbg << buffer[duke];
      mc2dbg << endl;
#endif
      // Wait for the answer
      CIMDEvent* event = waitForEvent();
      if ( event != NULL && event->m_parserResult.m_resultCode ==
           CIMDParserResult::RESP_NACK) {
            handleNack( event->m_parserResult.m_packetNumber );
            CIMDUtil::replacePktNbr(buffer,
                                    bufLen,
                                    pktNumber = getNextPacketNbr());
            delete event;
      } else {
         noNack = true;
         return event; // NB! Early return.
      }      
      nbrRetries++;
   }
   return NULL;
}


bool CIMDSMSTalker::waitForEvent( CIMDParserResult::result_t typeOfMess)
{
   mc2dbg << "Entering waitForEvent(CIMDParserResult::result_t)"
          << endl;
   mc2dbg << "waiting for " << (long) typeOfMess << endl;
   bool retVal = false;
   CIMDEvent* event = waitForEvent();   
   if ( event != NULL ) {
      CIMDParserResult::result_t result = event->m_parserResult.m_resultCode;
      if ( typeOfMess == result )
         retVal = true;
      delete event;
   }
   
   mc2dbg << "I will return " << retVal << endl;
   mc2dbg << "Exiting waitForEvent" << endl;
   return retVal;
}


void
CIMDSMSTalker::handleNack(int packetNumber)
{
   mc2dbg4 << "handleNack(" << packetNumber << ")" << endl;
   m_currentPacket = packetNumber;
}


CIMDEvent*
CIMDSMSTalker::waitForEvent()
{
   const uint32 waitTime = 10000;
   JTCSynchronized synchronized(*this);
   DEBUG1(mc2dbg << "Entering waitForEvent()" << endl);
   
   CIMDEvent* event = NULL;
   bool found = false;
   uint32 time = TimeUtility::getCurrentTime();

   // This loop is here because sometimes wait can exit without a notify.
   while ( ( TimeUtility::getCurrentTime() - time ) < waitTime && ! found ) {
      try {
         wait(10000);
      } catch (JTCInterruptedException& ) {
         mc2dbg << "Interrupted !! " << endl;
      }    
      if ( !m_eventQueue.empty() ) {
         DEBUG2(mc2dbg << "Found event in queue" << endl);
         event = m_eventQueue.front();
         m_eventQueue.erase( m_eventQueue.begin() );
         found = true;
      }
   }
   DEBUG2(mc2dbg << " Exiting waitForEvent()" << endl);
   return event;
}


void CIMDSMSTalker::postEvent( CIMDEvent *e)
{
   JTCSynchronized synchronized(*this);
   DEBUG2(mc2dbg << "Entering postEvent" << endl);
   DEBUG2(mc2dbg << "Posting " << (long)(e->m_parserResult.m_resultCode)
               << endl;);
   m_eventQueue.push_back( e );
   notifyAll(); // Maybe we could skip the loop int waitForEvent if we
                // do not have notifyAll but notify here. But it works
                // like this so... 
   DEBUG2(mc2dbg << "Exiting postEvent" << endl);
}


void CIMDSMSTalker::clearEventQueue()
{
   JTCSynchronized synchronized(*this);
   DEBUG2(mc2dbg << "clearing event queue" << endl);
   while ( !m_eventQueue.empty() ) {
      try {
         wait(1);
         CIMDEvent* event = m_eventQueue.front();
         m_eventQueue.erase( m_eventQueue.begin() );
         delete event;      
      } catch (JTCInterruptedException& ) {
         mc2dbg << "InterruptedException in clearEventQueue()" << endl;
      }
   }
}
uint32 CIMDSMSTalker::checkStatus()
{
   return StringTable::OK;
}


void
CIMDSMSTalker::run()
{
   mc2dbg << "[CIMD] Starting..." << endl;
   const int bufLength = 2048;
   char* buffer = new char[bufLength];
   int  alivePacketNbr = -1;

   reconnect();
   login();
   while ( ! terminated ) {
      int result = readToETX(buffer, bufLength);
      if ( result > 0 ) {
         CIMDParserResult result = m_cimdParser.parse(buffer);
         mc2dbg << "[CIMD Error code is: " << result.m_error << endl;
         if ( result.m_resultCode == CIMDParserResult::MESSAGE_DELIVERY ) {
            m_lastOKResponseTime = TimeUtility::getCurrentTime();
            CIMDDeliveryThread* thread =
               new CIMDDeliveryThread(this,
                                      m_processor,
                                      m_monitor,
                                      m_socket,
                                      result);
            thread->start();
         } else {
            if ( result.m_resultCode != CIMDParserResult::RESP_ALIVE_ACK &&
                 result.m_resultCode != CIMDParserResult::RESP_NACK) {
               // Do not post "Alive acks" and negative acks.
               postEvent(new CIMDEvent(result));
            } else {
               // Acks cannot be handled (don't know if it is because of
               // the CIMDSMSTalker or the SMSC).
               // If we resend a packet now, we will only get a nack as
               // response anyway so we reconnect.
               if ( result.m_error != 0 ||
                    result.m_resultCode == CIMDParserResult::RESP_NACK) {
                  if ( result.m_error != 0 ) {
                     // ERROR 
                     mc2log << warn
                            << "[CIMD]: Got error in ack for alivePacket"
                            << endl;
                  } else {
                     // NACK
                     mc2log << warn
                            << "[CIMD]: Got NACK response from server "
                               "- can't handle"
                               " that!" << endl;;
                  }
                  mc2dbg << "[CIMD]: Waiting 30 sec"
                         << " before reconnecting." << endl;
                  JTCThread::sleep(30000);
                  reconnect();
                  // This will only almost work!!!
                  // Nobody is running to verify the answers.
                  // Will timeout instead.
                  login();
               } else {
                  m_lastOKResponseTime = TimeUtility::getCurrentTime();
                  mc2log << info << "[CIMD] Got ACK for alive packet"
                         << endl;
               }                  
               alivePacketNbr = -1;
            }
         }
      } else {
         if ( result != -2 ) { /* No timeout - probably disconnected  */
            alivePacketNbr = -1;
            mc2log << warn << "[CIMD] Read returned -1. Waiting 30 sec"
                   " before reconnecting (errno: " << strerror(errno) 
                   << ")" << endl;
            JTCThread::sleep(30000);
            reconnect();
            // This will only almost work!!!
            // Nobody is running to verify the answers. Will timeout instead.
            login();
         } else {
            /* Timeout */
            const uint32 maxTime = 60000;
            if ( TimeUtility::getCurrentTime() - m_lastOKResponseTime > maxTime ) {
               mc2log << warn << "[CIMD]: More than " << maxTime << " ms "
                      << " have elapsed without any replies. Waiting 30 s "
                      << " then reconnecting" << endl;
               JTCThread::sleep(30000);
               reconnect();
               // This will only almost work!!!
               // Nobody is running to verify the answers.
               // Will timeout instead.
               login();
            } else {
               /* Send "Alive" 40 */
               alivePacketNbr = getNextPacketNbr();
               int len = CIMDUtil::makeAliveRequest(buffer,
                                                    alivePacketNbr);
               mc2log << info << "[CIMD] Sending alive packet" << endl;
               errno = 0;
               if ( m_socket->writeAll( (byte*)buffer, len ) < 0 ) {
                  mc2dbg << "[CIMD] Problem sending alive packet"
                     " - should reconnect soon (errno: "
                         << strerror(errno) << ")" << endl;
               }
            }
            /* Now we can get an ack for this message. Hopefully it will
               not interfere with the other processes */
         }
      }
   }
}


int
CIMDSMSTalker::readToETX(char* buffer, int bufLength)
{
   mc2dbg2 << "[CIMD] readToETX()" << endl;
   // A temporary buffer
   byte tempBuf;
   // The current position in the buffer
   char* bufPos = buffer;
   // The length of the buffer.
   int   bufLen = 0;
   int   result = 0;
   do {
      // Read one byte into the buffer. Timeout is 40 seconds.
      errno = 0;
      result = m_socket->readMaxBytes(&tempBuf, 1, 40*1000*1000);
      // Check if we still are connected.
      if ( result > 0 ) {
         *bufPos++ = (char)tempBuf;
         bufLen++;
      } else {
         if ( errno ) {
            mc2dbg << "[CIMD]: readToETX failed, errno: "
                   << strerror(errno) << endl;
         } else {
            mc2dbg << "[CIMD]: readToETX failed - timeout" << endl;
         }
      }
   } while ( (result > 0) &&
             ((char)tempBuf != ETX) &&
             (bufLen < (bufLength - 1)));

   // Add zero
   *bufPos = '\0';
   
   if ( result > 0 )
      return bufLen;
   else
      return result;
   
}


/***********************************************************************
 *  Delivery thread starts here
 **********************************************************************/

CIMDDeliveryThread::CIMDDeliveryThread(CIMDSMSTalker* cimdTalker,
                                       SMSProcessor* smsProcessor,
                                       JTCMonitor* monitor,
                                       TCPSocket* socket,
                                       CIMDParserResult& parserResult)
      : m_parserResult(parserResult)
{
   m_smsProcessor = smsProcessor;
   m_smsTalker    = cimdTalker;
   m_monitor      = monitor;
   m_socket       = socket;
}


void
CIMDDeliveryThread::run()
{
   // Lock out sendSMS / wait for sendSMS to finish
   JTCSynchronized synchronized(*m_monitor);
   uint32 startTime = TimeUtility::getCurrentTime();
   m_smsTalker->clearEventQueue();
   mc2dbg << "[CIMDDeliveryThread] Starting ..." << endl;
   const int bufferSize = 1024;
   char* buffer = new char[bufferSize];

   // Send response to server
   char* recipientPhone = m_parserResult.m_recipientPhone;
   char* senderPhone    = m_parserResult.m_senderPhone;
   int   dataLength     = m_parserResult.m_firstParam;
   byte* data           = (byte*)m_parserResult.m_stringParam;
   int pktNumber        = m_parserResult.m_packetNumber;
   int smsNumber        = m_parserResult.m_smsNumber;
   int nbrParts         = m_parserResult.m_maxParts;
   int partNbr          = m_parserResult.m_partNumber;

   // Log receiving line.
   m_smsTalker->logLine(false, senderPhone, recipientPhone, dataLength, data);
   
#ifdef DEBUG_LEVEL_2
   mc2dbg << "--SMS data: (not converted)" << endl;
   for(int i=0; i < dataLength; i++) {
      mc2dbg << hex << setfill('0') << setw(2) << (int)data[i] << dec << " ";
   }
   mc2dbg << endl << "--End SMS data" << endl;

   mc2dbg << "--SMS Length = " << dataLength << endl
          << "--End SMS data" << endl;
#endif
   
   int length = CIMDUtil::makeDeliverResponse(buffer, pktNumber);
   m_socket->writeAll( (byte*)buffer, length );
   buffer[length] = '\0';
   DEBUG2(mc2dbg << "Sent : " << buffer << endl);

   DEBUG2(mc2dbg << "Recipientphone :" << recipientPhone << endl);
   DEBUG2(mc2dbg << "Sender phone :" << senderPhone << endl);
   
   SMSSendRequestPacket *srp = new SMSSendRequestPacket();
   srp->fillPacket(CONVERSION_NO,
                   senderPhone, recipientPhone,
                   dataLength, data, SMSSendRequestPacket::SMSTYPE_TEXT,
                   smsNumber, nbrParts, partNbr);

   m_smsProcessor->SMSReceived(srp);

   mc2dbg << "Receiving took " << TimeUtility::getCurrentTime() - startTime
        << "ms" << endl;

   DEBUG1(mc2dbg << "CIMDReceiverThread exiting" << endl);
   delete [] buffer;
}





