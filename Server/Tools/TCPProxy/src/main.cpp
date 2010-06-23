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
#include "ProxyThread.h"
#include "HttpProxyThread.h"
#include "TCPSocket.h"
#include "SelectableSelector.h"
#include "CommandlineOptionHandler.h"
#include "FDSelectable.h"


int main( int argc, char** argv ) {
   ISABThreadInitialize init;

   // Options
   uint32 CL_port = 0;
   char* CL_host = NULL;
   uint32 CL_hostPort = 0;
   bool CL_http = 0;
   char* CL_msisdnFile = NULL;
   char* CL_hmccFile = NULL;

   auto_ptr<CommandlineOptionHandler> 
      realCoh( new CommandlineOptionHandler( argc, argv ) );

   CommandlineOptionHandler& coh = *realCoh;
   coh.setSummary( "This proxies connections to another host:port." );
   

   coh.addOption( "-l", "--listen",
                  CommandlineOptionHandler::uint32Val,
                  1, &CL_port, "0",
                  "Sets the port to listen for incoming connections on." );

   coh.addOption( "-s", "--host",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_host, "\0",
                  "Sets the host to proxy connections to." );

   coh.addOption( "-t", "--port",
                  CommandlineOptionHandler::uint32Val,
                  1, &CL_hostPort, "0",
                  "Sets the port to proxy connections to." );

   coh.addOption( "-o", "--http",
                  CommandlineOptionHandler::presentVal,
                  1, &CL_http, "F",
                  "If to use the http inserting proxying." );

   coh.addOption( "-m", "--msisdn",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_msisdnFile, "./msisdn.txt",
                  "If using the http inserting proxying. Then use this file \
                  to will tell what MSISDN to use for that header." );

   coh.addOption( "-hmcc", "--hmcc",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_hmccFile, "./hmcc.txt",
                  "If using the http inserting proxying. Then use this file \
                  to tell what hmcc to ouse for that header." );

   if ( !coh.parse() ) {
      return 1;
   }

   if ( CL_port == 0 || CL_hostPort == 0 || CL_host == NULL || 
        strlen( CL_host ) == 0 ) {
      mc2log << "Needs listen port, host and port." << endl;
      coh.printUsage( mc2log );
      return 1;
   }

   // Make listen socket
   TCPSocket* listenSock = new TCPSocket();
   if ( ! listenSock->listenDuration( CL_port, 10/*s*/ ) ) {
      mc2log << "Failed to listen on port " << CL_port << endl;
      return 1;
   } else {
      mc2log << "Listening on port " << CL_port << endl;
   }
   listenSock->setBlocking( false );

   SelectableSelector selector;

   // Check for shutdown
   FDSelectable stdinSelectable( fileno( stdin ) );
   bool shutdown = false;
   char input[ 2048 ];
   // do non-blocking stdin
   if ( fcntl( fileno( stdin ), F_SETFL, O_NONBLOCK ) == -1 ) {
      mc2log << error << "Failed to set non-blocking stdin" << endl;
   }
   input[ 0 ] = '\0';

   // Make proxy threads for accepted sockets
   while ( !shutdown ) {
      SelectableSelector::selSet readReady;
      SelectableSelector::selSet writeReady;

      selector.addSelectable( listenSock, true, false, false );
      selector.addSelectable( &stdinSelectable, true, false, false );

      SelectableSelector::SelectStatus selStatus =
         selector.select( 5000000, readReady, writeReady );

      if ( selStatus == SelectableSelector::OK ) {
         if ( readReady.find( listenSock ) != readReady.end() ) {
            TCPSocket* newSock = listenSock->accept();
            if ( newSock != NULL ) {
               ProxyThread* thread = NULL;
               if ( CL_http ) {
                  thread = new HttpProxyThread( newSock, CL_host, 
                                                CL_hostPort, CL_msisdnFile,
                                                CL_hmccFile );
               } else {
                  thread = new ProxyThread( newSock, CL_host, CL_hostPort );
               }
               thread->start();
            }
         }
         // Commands
         if ( readReady.find( &stdinSelectable ) != readReady.end() ) {
            fgets( input, 2048, stdin );
            MC2String line( input );
            if ( line.size() > 0 ) {
               line.erase( line.size() - 1 ); // Get rid of newline char
            }
            // Got a line 
            if ( line == "shutdown" ) {
               mc2log << info << "Got shutdown from commandline, "
                      << "shutting down." << endl;
               shutdown = true;
            }
         }
      }
   }

   // Delete some
   delete listenSock;

   // Await termination
   init.waitTermination();

   coh.deleteTheStrings();

   return 0;
}


