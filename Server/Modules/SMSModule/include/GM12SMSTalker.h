/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GM12SMSTALKER_H
#define GM12SMSTALKER_H

#include "HayesSMSTalker.h"
#include "ISABThread.h"
#include "ModemParser.h"

class GM12Event; // Forward declaration
class GM12ReceiverThread; // Forward declaration

/**
 *    GM12SMSTalker. 
 *    SMSTalker for GM12 and GC25 telephones.
 *
 */
class GM12SMSTalker : public ISABMonitor, ISABThread, public HayesSMSTalker {
public:
   
   /**
    *   Create a new GM12SMSTalker.
    *   @param port The serial port that the phone module is connected to.
    *   @param pin The pincode of the SIM-card.
    */
   GM12SMSTalker(const char* port,
                 const char* pin);
   
   /**
    *    Delete this object.
    */
   virtual ~GM12SMSTalker();
   
   /**
    *   The run function for the receiver. (thread)
    */
   virtual void run();
   
   /**
    *   Initializes the GM12SMSTalker
    *   @return True if the initialization was succesful.
    */
   virtual bool init(SMSProcessor* processor);
   
   
   /**
    *   Send an SMS.
    *   @param p A packet containing the SMS to send.
    */
   virtual SMSSendReplyPacket* sendSMS(SMSSendRequestPacket* p);
   
   
   /**
    *   Returns the status of the GM12SMSTalker.
    */
   virtual uint32 checkStatus();
   
private:
   
   /**
    *    Read a maximum of bufLenth characters into buffer.
    */
   int readLine(char* buffer, int bufLength);
   
   /**
    *   Compares result to the first result in the eventQueue.
    *   @param result The result to wait for.
    *   @return True if the result was what we were waiting for. False
    *           if the result was something else or there was a timeout.
    */
   bool waitForEvent(ModemParserResult::result_t result);
   
   /**
    *   Waits for a GM12Event in the eventQueue.
    *   @return GM12Event or NULL.
    */      
   GM12Event* waitForEvent();
   
   /**
    *   Puts an event in the eventQueue.
    */
   void postEvent(GM12Event* e);
   
   /**
    *    Clears the event queue.
    */
   void clearEventQueue();
   
   /**
    *    The pincode of the SIM-card or NULL if pin shouldn't be used.
    */
   char* m_pin;
   
   /**
    *    Parser for modem responses.
    */
   ModemParser modemParser;
   
   /**
    */
   std::vector < GM12Event* > m_eventQueue;
   
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
    */
   friend class GM12ReceiverThread;
};


/**
 *    Event sent from the reading thread to the other two threads.
 *    Represents a result from the modem.
 *
 */
class GM12Event {

   /**
    *    Creates a new event containing a <code>ModemParserResult</code>.
    *    @param result The result to carry.
    */
   GM12Event(ModemParserResult result) : m_parserResult(result)
   {
   };

   /**
    *    The result that the <code>GM12Event</code> carries.
    */
   ModemParserResult m_parserResult;
   
   /**
    */
   friend class GM12SMSTalker;

   /**
    */
   friend class GM12ReceiverThread;
};


/**
 *    GM12ReceiverThread.
 *
 */
class GM12ReceiverThread : public JTCThread {

  public:
   /**
    *    The constructor.
    *    @param gm12smstalker The GM12SMSTalker that called the function.
    *    @param smsProcessor  The SMSProcessor for deliverance of the SMS.
    *    @param monitor       The monitor to lock out incoming SMS:s.
    *    @param memoryIndex   The index to send to the phone in "CGMR".
    */
   GM12ReceiverThread(GM12SMSTalker *gm12smstalker,
                      SMSProcessor* smsProcessor,
                      JTCMonitor* monitor,
                      SerialComm* serial,
                      int memoryIndex);

   /**
    *    The thread itself. Gets an SMS from the phone and then
    *    deletes it from the memory in the phone. After that it
    *    invokes smsProcessor->SMSReceived( ... ). Then it dies
    *    the horrible ThreadDeath. ( or at least it exits from run).
    */
   void run();

  private:

   /**
    *   The monitor provided by GM12SMStalker that will keep us from
    *   disturbing the sending of SMS:s in GM12SMSTalker::sendSMS.
    */
   JTCMonitor* m_monitor;

   /**
    */
   GM12SMSTalker* m_smsTalker;

   /**
    */
   int m_memoryIndex;

   /**
    */
   SerialComm* m_serial;

   /**
    *   The SMSProcessor
    */
   SMSProcessor* m_smsProcessor;
};

#endif

