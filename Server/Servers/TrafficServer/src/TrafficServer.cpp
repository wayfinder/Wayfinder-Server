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

#include "GenericServer.h"
#include "CommandlineOptionHandler.h"
#include "TrafficThread.h"
#include "ThreadRequestHandler.h"

// Parsers
#include "DatexIITrafficParser.h"

// Feeds
#include "TCPTrafficFeed.h"
#include "FileTrafficFeed.h"

#include "ParserThreadGroup.h"
#include "PropertyHelper.h"
#include "XMLInit.h"
#include "NotifyPipe.h"
#include "TrafficInfoParserThreadGroup.h"

#include "SSLSocket.h"
#include "NotifyPipe.h"

class TrafficServerComponent: public Component {
public:
   explicit TrafficServerComponent( CommandlineOptionHandler* coh ):
      Component( "TrafficServer", coh ) {
   }

   void setWorkThread( ISABThreadHandle work ) {
      m_workThread = work;
   }

   void init() {
      Component::init();
      m_workThread->start();
   }

private:
   ISABThreadHandle m_workThread;
};

/// Creates a traffic parser from a specific format
TrafficParser* createParser( uint32 format ) {
   switch ( format ) {
      case 0:
         return new DatexIITrafficParser( "SRA_AS" );
      case 1:
         return new DatexIITrafficParser( "SRA_EIS" );
      case 2:
         return new DatexIITrafficParser( "SRA_RWS" );
      case 3:
         return new DatexIITrafficParser( "SRA_TMS" );
   }

   return NULL;
}

int main( int argc, char** argv ) {
   uint32 format = MAX_UINT32;
   uint32 port = 0;
   PropertyHelper::PropertyInit propInit;

   auto_ptr<CommandlineOptionHandler>
      coh( new CommandlineOptionHandler( argc, argv ) );
   coh->setSummary( "The TrafficServer has a listening socket that "
                   "accepts traffic information files. Each received "
                   "file will be processed and the resulting traffic "
                   "situations sent to the InfoModule.\n\n"
                   "Optionally it can be run with one or more files "
                   "specified on the commandline. Each file will be "
                   "read and processed in turn. When all files have "
                   "been processed, the TrafficServer will exit. " );

   coh->addOption( "-n", "--TrafficServerPort",
                  CommandlineOptionHandler::uint32Val,
                  1, &port, "0",
                  "Sets the preferred TrafficServerPort.");

   typedef pair< MC2String, MC2String > OptionPair;
   const OptionPair options[] = {
      make_pair( "--datexii_as",  "Vägverket DatexII Accident Service" ),
      make_pair( "--datexii_eis", "Vägverket DatexII Emergency Info Service" ),
      make_pair( "--datexii_rws", "Vägverket DatexII Road Work Service" ),
      make_pair( "--datexii_tmx", "Vägverket DatexII Traffic Message Service" )
   };

   coh->addSingleChoiceOption( "-f", 
                              options,
                              options + sizeof (options) / sizeof (*options),
                              &format, MAX_UINT32,
                              "Sets the type of data to read." );
   coh->setTailHelp("[files]");
   if ( ! coh->parse() ) {
      mc2log << error << "TrafficServer: Error on commandline (-h for help)"
             << endl;
      return 2;
   }

   if ( ! Properties::setPropertyFileName( coh->getPropertyFileName() ) ) {
      mc2log << error << "TrafficServer: No such file or directory: '"
             << coh->getPropertyFileName() << "'" << endl;
      return 3;
   }


   // Initialize the XML system
   XMLTool::XMLInit xmlInit;
   SSLCleaner sslInit;
   ISABThreadInitialize threadInit;

   TrafficParser* parser = createParser( format );

   if ( parser == NULL ) {
      mc2log << error << "Invalid parser choice!" << endl;
      return 4;
   }

   TrafficFeed* feed = NULL;

   TrafficServerComponent trafficServer( coh.get() );

   if ( port != 0 ) {
      feed = new TCPTrafficFeed( port,
                                 trafficServer.getShutdownPipe() );
   } else {

      vector<MC2String> files;
      for ( uint32 i = 0; i < coh->getTailLength(); ++i ) {
         files.push_back( coh->getTail( i ) );
      }

      feed = new FileTrafficFeed( files );
   }
   coh.release();

   ISABThreadHandle workThread =
      new TrafficThread( parser->getProvider(),
                         parser, feed,
                         trafficServer.getShutdownPipe() );

   trafficServer.setWorkThread( workThread );
   trafficServer.init();
   trafficServer.gotoWork( threadInit );

   return 0;
}
