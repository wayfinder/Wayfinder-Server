/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TrafficThread.h"

#include "TrafficParser.h"
#include "TrafficHandler.h"

#include "RealTrafficIPC.h"
#include "TrafficSituation.h"
#include "DeleteHelpers.h"

#include "TrafficFeed.h"
#include "NotifyPipe.h"

#include "BasicMapRequester.h"

TrafficThread::TrafficThread( const MC2String& provider,
                              TrafficParser* parser,
                              TrafficFeed* feed,
                              NotifyPipe& shutdownPipe ): 
   m_provider( provider ),
   m_moduleCom( new BasicMapRequester( ServerTypes::TRISS ) ),
   m_handler( new TrafficHandler( provider,
                                  new RealTrafficIPC( *m_moduleCom ) ) ),
   m_parser( parser ),
   m_feed( feed ),
   m_shutdownPipe( shutdownPipe )
{

}

TrafficThread::~TrafficThread() {
   delete m_handler;
   delete m_parser;
}

void TrafficThread::run() {

   mc2log << "[TrafficThread] started." << endl;

   if ( m_feed->eos() ) {
      mc2log << "[TrafficThread] No feed." << endl;
      terminated = true;
      m_shutdownPipe.notify();
      return;
   }

   m_moduleCom->start();
   m_handler->setup();

   while ( !terminated ) {
      MC2String data;

      // read the file 
      if ( ! m_feed->getData( data ) ) {
         // End of stream?
         if ( m_feed->eos() ) {
            // nothing more in the stream, terminate thread.
            terminated = true;
            break;
         }

         mc2log << warn << "[TrafficThread] No data in feed." << endl;
         continue;
      }

      // file read, now parse it
      STLUtility::AutoContainer< TrafficHandler::SitCont > trafficSits;
      m_parser->parse( data, trafficSits );

      // data parsed, send it to the traffic handler.
      if ( ! m_handler->processSituations( trafficSits ) ) {
         mc2log << warn << "[TrafficThread] Failed to process situaitons."
                << endl;
      }

   }

   m_moduleCom->stop();
   m_shutdownPipe.notify();

   mc2log << "[TrafficThread] shutting down." << endl;

}

