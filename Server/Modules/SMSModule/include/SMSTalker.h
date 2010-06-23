/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SMSTALKER_H
#define SMSTALKER_H

#include "config.h"
#include "SMSPacket.h"
#include "MC2String.h"
#include <stdio.h>

class SMSProcessor; // Forward declaration

/**
 *    Abstract superclass to all SMS sender/receivers.
 *
 */
class SMSTalker {
   public:
      /**
       *   Creates a new SMSTalker
       */
      SMSTalker();

      /**
       *   I am the Destructor.
       */
      virtual ~SMSTalker();
      
      /**
       *   Sets new log path and (re)opens log file.
       *   @return True if logfile could be opened.
       */
      bool openLogFile(const char* smsLogPath);

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
       *    Returns the filename that we should be logging to.
       */
      const char* getLogFileName() const;
      
      /**
       *   Initializes the SMSTalker. Copies the processor to
       *   m_processor. Override this method in the subclasses.
       *   @return True.
       */
      virtual bool init(SMSProcessor* processor) {
         m_processor = processor;
         return true;
      }
      
      /**
       *   Send an SMS.
       *   @param p A packet containing the SMS to send.
       */
      virtual SMSSendReplyPacket* sendSMS(SMSSendRequestPacket* p) = 0;
      
      /**
       *   Get the status of the SMSTalker.
       *   @return   The status of this SMSTalker.
       */
      virtual uint32 checkStatus() = 0;

      /**
       *   Set the telephone numbers of this talker.
       *   @param numberOfPhones 
       *   @param phones         
       */
      virtual void setPhoneNumbers(int numberOfPhones, char** phones);

      /**
       *   Sets the servicenames of this talker.
       *   @param numberOfServices  
       *   @param services          
       */
      virtual void setServiceNames(int numberOfServices, char** services);

      /**
       *    Get the string contating the given phonenumber.
       *    @return Phone number number phoneNumberNumber. If
       *             phoneNumberNumber is too large NULL is returned.
       */
      virtual char* getPhoneNumber(int phoneNumberNumber) {
         if ( phoneNumberNumber < m_numberOfPhones )
            return m_phoneNumbers[phoneNumberNumber];
         else
            return NULL;
      }

  protected:

      /**
       *   Creates the filename of the log. 
       */
      MC2String createLogFileName(const char* inpath);
      
      /**
       *   The processor to send incoming SMS:s to.
       */
      SMSProcessor* m_processor;

      /**
       *   The number of phonenumbers of this talker.
       */
      int m_numberOfPhones;

      /**
       *   The number of servicenames for this talker.
       */
      int m_numberOfServices;

      /**
       *   The phonenumbers of this talker.
       */
      char** m_phoneNumbers;

      /**
       *   The servicenames of this talker.
       */
      char** m_serviceNames;

      /**
       *   Current file that we should be logging to.
       */
      char* m_logFileName;

      /**
       *   File pointer to the currently used logfile.
       */
      FILE* m_logFile;
};

#endif

