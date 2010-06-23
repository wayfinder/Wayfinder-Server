/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRAFFICTHREAD
#define TRAFFICTHREAD

#include "ParserThread.h"

#include <queue>
#include <memory>

class TrafficHandler;
class TrafficFeed;
class TrafficParser;
class GenericServer;
class BasicMapRequester;
class NotifyPipe;

/**
 * A thread that will wait for incomming data from our provider.
 * When data is recieved it send it to the provider specific parser. After
 * that it will send the parsed traffic situations to the Handler that will
 * process the situations and send them to the InfoModule when its done.
 */
class TrafficThread: public ISABThread {
public:
   /**
    * Constructor for the TrafficThread.
    *
    * @param group The thread group this thread will belong to.
    * @param provider The provider of the feed
    */
   TrafficThread( const MC2String& provider,
                  TrafficParser* parser, 
                  TrafficFeed* feed,
                  NotifyPipe& shutdownPipe );

   /**
    * Destructor for the TrafficThread.
    */
   virtual ~TrafficThread();

   /// This will start the thread.
   virtual void run();

private:

   /// The provider of the feed.
   MC2String m_provider;

   /// Communicates packets to modules
   auto_ptr<BasicMapRequester> m_moduleCom;

   /// The handler for this thread
   TrafficHandler* m_handler;

   /// The parser.
   TrafficParser* m_parser;
   auto_ptr<TrafficFeed> m_feed;
   NotifyPipe& m_shutdownPipe;
};

#endif //TRAFFICTHREAD
