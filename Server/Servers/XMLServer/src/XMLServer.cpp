/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserThreadGroup.h"
#include "XMLParserThread.h"
#include "CommandlineOptionHandler.h"
#include "SSLSocket.h"
#include "HttpInterfaceFactory.h"
#include "Properties.h"
#include "HttpInterfaceRequest.h"
#include "XMLInit.h"
#include "TimeUtility.h"
#include "PropertyHelper.h"
#include "PTServer.h"
#include "ServerTypes.h"

#include <signal.h>
#include <sys/stat.h>

int main( int argc, char** argv ) try {
   // Initialize random number generator
   srand( TimeUtility::getRealTime() );
   
   // Should be uint16 but CommandlineOptionHandler doesn't support that.
   uint32 port = 0;
   uint32 unsecPort = 0;

   bool noSSL = false;
   bool hostnameLookups = false;
   uint32 periodicTrafficUpdateInterval = MAX_UINT32;

   // Just for property file handling
   //---------------------------------------------------------------------
   auto_ptr<CommandlineOptionHandler> 
      realCoh( new CommandlineOptionHandler( argc, argv ) );
   CommandlineOptionHandler& coh = *realCoh;
   coh.setSummary("");

   
   // noSSL
   coh.addOption( "-s", "--nossl",
                  CommandlineOptionHandler::presentVal,
                  1, &noSSL, "F",
                  "Set if no ssl should be used.");
   
   // hostnameLookups
   coh.addOption( "-l", "--lookuphostnames",
                  CommandlineOptionHandler::presentVal,
                  1, &hostnameLookups, "F",
                  "Set if client hosts name should be looked up.");
   // HttpsPort
   coh.addOption( "-n", "--port",
                  CommandlineOptionHandler::uint32Val,
                  1, &port, "11122",
                  "Set the preferred secured listen port. "
                  "Default value is 11122." );
   // HttpPort
   coh.addOption( "-u", "--unsecport",
                  CommandlineOptionHandler::uint32Val,
                  1, &unsecPort, "12211",
                  "Set the preferred unsecured listen port. "
                  "Default value is 12211.");

   // CL_periodicTrafficUpdateInterval
   coh.addUINT32Option( "", "--ptui", 1, 
                        &periodicTrafficUpdateInterval, MAX_UINT32,
                        "Sets the server-set value for the periodic "
                        "traffic informations update interval. If not "
                        "set the value will be set by the "
                        "NAV_PERIODIC_TRAFFIC_UPDATE_INTERVAL in the "
                        "mc2.prop file." );

   // initialize subsystems and cleaners
   PropertyHelper::PropertyInit propInit;
   XMLTool::XMLInit xmlInit;
   SSLCleaner sslClean;

   PTServer* ptserver = new PTServer( "XMLServer", realCoh.release(),
                                      ServerTypes::XML );

   ptserver->parseCommandLine();

   if ( periodicTrafficUpdateInterval == MAX_UINT32 ) {
      periodicTrafficUpdateInterval = Properties::getUint32Property( 
         "NAV_PERIODIC_TRAFFIC_UPDATE_INTERVAL" );
   }

   TCPSocket* listenSock = NULL;
#ifdef USE_SSL
   if ( noSSL ) {
      if ( ptserver->getHttpsFD() != 0 ) {
         // Use the already opened fd 
         listenSock = new TCPSocket( ptserver->getHttpsFD(), DEFAULT_BACKLOG );
      } else {
         listenSock = new TCPSocket();
      }
   } else {
      ptserver->setupCTX();

      if ( ptserver->getHttpsFD() != 0 ) {
         // Use the already opened fd 
         listenSock = new SSLSocket( ptserver->getHttpsFD(), NULL, 
                                     ptserver->getCTX() );
      } else {
         listenSock = new SSLSocket( ptserver->getCTX() );
      }
   }
#else
   listenSock = new TCPSocket();
#endif
   if ( ptserver->getHttpsFD() == 0 ) {
      if ( ! listenSock->open() ) {
         PANIC( "XMLServer failed to open listensocket", "" );
      }
   } else {
      uint32 sIP = 0;
      uint16 sPort = 0;
      listenSock->getSockName( sIP, sPort );
      port = sPort;
   }
   mc2log << info << "Trying to get port " << port << "..." << endl;
   if ( ! listenSock->listenDuration( port, 10/*s*/, 
                                      ptserver->getHttpsFD() == 0 ) ) {
      PANIC( "XMLServer failed to listen on listensocket", "" );
   } else {
      mc2log << info << "XMLServer: listensocket opened, port " << port 
             << endl;
   }

   TCPSocket* unsecSock = NULL;
   if ( ptserver->getHttpFD() == 0 ) {
      unsecSock = new TCPSocket();
      if ( ! unsecSock->open() ) {
         PANIC( "XMLServer failed to open unsecure socket", "" );
      }
   } else {
      unsecSock = new TCPSocket( ptserver->getHttpFD(), DEFAULT_BACKLOG );
      uint32 sIP = 0;
      uint16 sPort = 0;
      unsecSock->getSockName( sIP, sPort );
      unsecPort = sPort;
   }

   mc2log << info << "Trying to get port " << unsecPort << "..." << endl;
   if ( ! unsecSock->listenDuration( unsecPort, 10/*s*/, 
                                     ptserver->getHttpFD() == 0 ) ) {
      PANIC( "XMLServer failed to listen on unsecure socket", "" );
   } else {
      mc2log << info << "XMLServer: unsecuresocket opened, port " 
             << unsecPort << endl;      
   }
   
   uint32 minNbrThreads = min( ptserver->getMinNbrThreads(), 
                               ptserver->getMaxNbrThreads() );

   MC2String httpLogFilename = Properties::getProperty( "XMLHTTP_LOG",
                                                        "" );
   mc2log << "[XMLServer] logging xmlhttp to file " << httpLogFilename << endl;

   XMLParserThreadGroup*  group = 
      new XMLParserThreadGroup( hostnameLookups, 
                                httpLogFilename.c_str(),
                                "XMLParserThreadGroup",
                                minNbrThreads, 
                                ptserver->getMaxNbrThreads() );
   ISABThreadGroupHandle groupHandle = group;

   ptserver->setGroup( group );
   ptserver->addInterfaceFactory( new HttpInterfaceFactory( ptserver->getGroup(),
                                                           listenSock ) );
   ptserver->addInterfaceFactory( new HttpInterfaceFactory( ptserver->getGroup(), 
                                                           unsecSock ) );
   
   group->setPeriodicTrafficUpdateInterval( periodicTrafficUpdateInterval );

   // Set timeout higher
   HttpInterfaceRequest::setMaxSocketWaitTime( 180000000 );
   // Set nbr reuses high so less connection close from server
   HttpInterfaceRequest::setMaxSocketReuses( 1024 );

   ptserver->start();
   ptserver->callGotoWork();

   mc2log << info << "XMLServer done." << endl;

   delete ptserver;
   ptserver = NULL;
   groupHandle = NULL;
   group = NULL;

   return 0;
} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "XMLServer: " << e.what() << endl;
   return 1;
} catch ( const ComponentException& e ) {
   mc2log << error << "XMLServer Component error: " << e.what() << endl;
   return 2;
}
