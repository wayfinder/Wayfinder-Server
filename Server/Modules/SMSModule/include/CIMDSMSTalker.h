/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CIMDSMSTALKER_H
#define CIMDSMSTALKER_H

#include "SMSTalker.h"
#include "ISABThread.h"
#include "CIMDParser.h"
#include "SMSProcessor.h"
#include "Types.h"

class CIMDEvent;
class CIMDDeliveryThread;
class TCPSocket;
      
/**
 *    CIMDSMSTalker.
 *
 */
class CIMDSMSTalker : public ISABMonitor, 
                      public ISABThread, 
                      public SMSTalker {
public:

     /**
      *     Create a new CIMDSMSTalker.
      *     @param ip       The ip number to connect to.
      *     @param port     The port to connect to.
      *     @param userName The user name to use when logging in. If empty 
      *                     a default value will be used.
      *     @param password The password to send to the SMSC when logging 
      *                     in. If empty a default value will be used.
      *     @param bindAddress Address to bind to or NULL if 0.0.0.0.
      */
     CIMDSMSTalker(const char* ip,
                   uint16 port,
                   const char* userName,
                   const char* password,
                   const char* bindAddress);
     
     /**
      *     Delete this CIMDSMSTalker.
      */
     virtual ~CIMDSMSTalker();

   /**
    *   Puts a line in the logfile.
    *   @param senderPhone    Sender phone number.
    *   @param recipientPhone Recipient phone number.
    *   @param nbrBytes       The number of bytes in the message.
    *   @param contents       The message.
    *   @return True if succesfully written to the file.
    */
   bool logLine(bool sending,
                const char* senderPhone,
                const char* recipientPhone,
                int nbrBytes,
                const byte* contents);
     
      /**
       *   The run function for the receiver. (thread)
       */
      virtual void run();

      /**
       *   Initializes the CIMDSMSTalker
       *   @return True if the initialization was succesful.
       */
      virtual bool init(SMSProcessor* processor);

      
      /**
       *   Send an SMS.
       *   @param p A packet containing the SMS to send.
       */
      virtual SMSSendReplyPacket* sendSMS(SMSSendRequestPacket* p);

      
      /**
       *   Returns the status of the CIMDSMSTalker.
       */
      virtual uint32 checkStatus();

  private:

      /**
       *    Reconnects to the SMSC and tries to login.
       */
      bool reconnect();

      /**
       *    Tries to log in to the SMSC.
       */
      bool login();

      
      /**
       *    Read a maximum of bufLenth characters into buffer.
       */
      int readToETX(char* buffer, int bufLength);

      /**
       *   Compares result to the first result in the eventQueue.
       *   @param result The result to wait for.
       *   @return True if the result was what we were waiting for. False
       *           if the result was something else or there was a timeout.
       */
      bool waitForEvent(CIMDParserResult::result_t result);

      
      /**
       *   Waits for an event in the eventQueue and returns it.
       *   @return The next element in eventQueue and NULL if timeout.
       */
      CIMDEvent* waitForEvent();


      /**
       *   Sends the buffer to the socket and waits for the answer.
       *   @return NULL if timeout or if send returned -1.
       */
      CIMDEvent* sendAndWait(TCPSocket* socket,
                             byte* buffer,
                             int bufLen,
                             int &packetNbr);
      
      /**
       *   Puts an event in the eventQueue.
       */
      void postEvent(CIMDEvent* e);

      /**
       *    Clears the event queue.
       */
      void clearEventQueue();

      /**
       *    Parser for modem responses.
       */
      CIMDParser m_cimdParser;

      typedef std::vector < CIMDEvent* > CIMDEventVector;

      /**
       *
       */
      CIMDEventVector m_eventQueue;

      /**
       *    A monitor that will keep the sending and receiving threads from
       *    disturbing each other.
       */
      JTCMonitor* m_monitor;

      
      /**
       *    The delay between sending SMS:s so that the telephone won't
       *    be so surprised when a new SMS is on its way in.
       */
      static const uint32 m_timeBetweenSMS = 1000; // This is one second.
      
      /**
       *    The IP to the CIMD server.
       */
      uint32 m_ip;

      
      /**
       *    The port to connect to.
       */
      uint16 m_port;

      /**
       *    The socket used for communicating with the SMSC.
       */
      TCPSocket* m_socket;

      /**
       *    The username used when logging in to the CIMD server.
       */
      char* m_userName;
      
      /**
       *    The password.
       */
      char* m_password;
      
      /** 
       *    The current packetnumber when sending.
       */
      byte m_currentPacket;

      /// Time for last response that was ok
      uint32 m_lastOKResponseTime;
      
      /**
       *    Address to bind to if not 0.0.0.0;
       */
      char* m_bindAddress;

      /** 
       *    Handle negative acknowledgement from SMSC.
       *    @param packetNumber The number of the nacked packet.
       */
      void handleNack(int packetNumber);

      
      /** 
       *    @return The packet number for the next message to
       *            the SMSC. The number is increased by 2.
       */
      int getNextPacketNbr() {
         byte retVal = m_currentPacket;
         m_currentPacket += 2;
         return retVal;
      };

      /** 
       *    Reset the packet number to 1.
       */
      void resetPacketNbr() {
         m_currentPacket = 1;
      };
      
      /** 
       */
      friend class CIMDDeliveryThread;
};


/**
 *    CIMDEvent.
 *
 */
class CIMDEvent {

   /**
    *    Create a new CIMDEvent.
    */
   CIMDEvent(CIMDParserResult result) : m_parserResult(result)
   {
   };

   /**
    */
   CIMDParserResult m_parserResult;
   
   /**
    */
   friend class CIMDSMSTalker;

   /**
    */
   friend class CIMDDeliveryThread;
};

/**
 *    CIMDDeliveryThread.
 *
 */
class CIMDDeliveryThread : public ISABThread {
   public:
      /**
       */
      CIMDDeliveryThread(CIMDSMSTalker* cimdTalker,
                         SMSProcessor* smsProcessor,
                         JTCMonitor* monitor,
                         TCPSocket* socket,
                         CIMDParserResult& parserResult);

      /**
       */
      void run();
   private:
      /**
       */
      SMSProcessor* m_smsProcessor;

      /**
       */
      CIMDSMSTalker* m_smsTalker;

      /**
       */
      JTCMonitor* m_monitor;

      /**
       */
      CIMDParserResult m_parserResult;

      /**
       */
      TCPSocket* m_socket;
};

#endif

