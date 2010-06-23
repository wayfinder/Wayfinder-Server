/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Types.h"
#include "config.h"
#include "EmailReader.h"
#include "EmailProcessor.h"
#include "JobThread.h"
#include "PacketQueue.h"
#include "CommandlineOptionHandler.h"
#include "EmailSender.h"
#include "StringUtility.h"
#include "PacketReaderThread.h"
#include "MapSafeVector.h"
#include "ModulePacketSenderReceiver.h"
#include "PropertyHelper.h"
#include "Module.h"
#include "StandardEmailSender.h"
#include "DummyEmailSender.h"

#define DEFAULT_SMTP_HOST  "localhost.localdomain"

class EmailModule: public Module {
public:
   EmailModule( int argc, char **argv ):
      Module( MODULE_TYPE_SMTP,
              argc, argv,
              false ) // not using maps
   {}

   void parseCommandLine() throw ( ComponentException ) {
      char* smtphost = NULL;

      m_cmdlineOptHandler->
         addOption( "-s", "--smtphost",
                    CommandlineOptionHandler::stringVal,
                    1, &smtphost, "X",
                    "Set the name of the SMTP-server. This overrides the value "
                    "in the property file. The name of the SMTP-host is set in "
                    "the following order: 1) The value given on the "
                    "commandline 2) Value in the propery-file 3) Default value "
                    "(" DEFAULT_SMTP_HOST ")");
      // Set the value to "X" if not given on commandline, handled below!

      m_cmdlineOptHandler->
         addOption( "-d", "--dummy",
                    CommandlineOptionHandler::presentVal,
                    1, &m_dummy, "F",
                    "Doesn't really send the emails to a server, just silently "
                    "discards them." );
      Module::parseCommandLine();

      // Check the smtphost-value
      if ( (smtphost == NULL) ||
           ( (smtphost != NULL) && (strlen(smtphost) == 1) && 
             (smtphost[0] == 'X'))) {
         // NO value given on commandline
         const char* newSmtphost = Properties::getProperty( "SMTP_HOST",
                                                            DEFAULT_SMTP_HOST );
         m_smtphost = newSmtphost ? newSmtphost : "";
      }
   }

   void init() {
      m_processor.reset( new EmailProcessor( m_loadedMaps.get(), 
                                             createEmailSender() ) ); 

      m_senderReceiver.
         reset( new ModulePacketSenderReceiver( m_preferredPort,
                                                IPnPort( m_leaderIP,
                                                         m_leaderPort ),
                                                IPnPort( m_availIP, 
                                                         m_availPort) ) );
      m_packetReader.
         reset( new PacketReaderThread( static_cast<ModulePacketSenderReceiver&>
                                        (*m_senderReceiver ) ) );
      m_queue.reset( new PacketQueue() );

      m_reader = new EmailReader( m_queue.get(), 
                                  m_senderReceiver->getSendQueue(),
                                  m_loadedMaps.get(), 
                                  m_packetReader.get(),
                                  m_senderReceiver->getPort(),
                                  m_rank );
      Module::init();
   }

   EmailSender* createEmailSender() {
      if ( m_dummy ) {
         return new DummyEmailSender();
      } 
      else {
         return new StandardEmailSender( m_smtphost.c_str() );
      }
   }

private:
   MC2String m_smtphost;
   bool m_dummy;
};

int main( int argc, char** argv ) 
try {

   ISABThreadInitialize init;
   PropertyHelper::PropertyInit propInit;

   EmailModule* emailModule = new EmailModule( argc, argv );
   ISABThreadHandle emailHandle = emailModule;

   emailModule->parseCommandLine();
   emailModule->init();
   emailModule->start();
   emailModule->gotoWork( init );

   return 0;

} catch ( const ComponentException& ex ) {
   mc2log << fatal << "Could not initialize EmailModule. " 
          << ex.what() << endl;
}
