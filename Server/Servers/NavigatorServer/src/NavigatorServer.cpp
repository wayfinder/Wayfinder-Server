/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavParserThreadGroup.h"
#include "CommandlineOptionHandler.h"
#include "TcpInterfaceFactory.h"
#include "Properties.h"
#include "SSLSocket.h"
#include "HttpInterfaceRequest.h"
#include "ngpmaker.h"
#include "PropertyHelper.h"
#include "XMLInit.h"
#include "PTServer.h"

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

void signalHandler( int iMsg ) {
}

/**
 *   Thread in which to read the latest news images on whopper.
 */
class LatestNewsReaderThread : public ISABThread {
public:

   LatestNewsReaderThread( NavParserThreadGroup& grp,
                           const char* param ) : m_grp(grp),
                                                 m_param(param ? param : "")
      {}
      
   void run() {
      if ( ! m_param.empty() ) {
         m_grp.setLatestNews( m_param.c_str() );
      }
   }

private:
   /// NavParserThreadGroup
   NavParserThreadGroup& m_grp;
   
   /// Param to setLatestNews
   MC2String m_param;

};


/**
 * Nav version with LatestNewsReaderThread in init.
 */
class NavPTServer : public PTServer {
public:
   /**
    * @see PTServer constructor.
    */
   NavPTServer( char*& CL_latestNews, const MC2String& name,
                CommandlineOptionHandler* handler,
                ServerTypes::servertype_t type,
                bool pushService = false )
         : PTServer( name, handler, type, pushService ),
           m_CL_latestNews( CL_latestNews ) 
   {}

   /**
    * @see PTServer
    */
   virtual void init() {
      // TODO: This is not the best solution for the dependencies at startup
      // Set this here so LatestNewsReaderThread can be started, but it is 
      // check in PTServer::init() and set there if not set here (see XS)...
      m_group->setClientSettingStorage( m_clientSettings );
#if 1
      // THREAD HERE
      (new LatestNewsReaderThread( *static_cast<NavParserThreadGroup*> ( 
                                      m_group ), m_CL_latestNews ) )->start();
#else
      static_cast<NavParserThreadGroup*> ( m_group )->setLatestNews( 
         m_CL_latestNews );
#endif
      PTServer::init();
   }

private:
   char*& m_CL_latestNews;
};


int main(int argc, char** argv)
try {
   // Initialize random number generator
   srand( TimeUtility::getRealTime() );
   
   mc2log << info << "Navigatorserver - initializing" << endl;

   //For the commandline
   char* CL_port;
   char* CL_saveSimulatorRoute;
   char* CL_alternativeServersList;
   bool  CL_forceRedirect = false;
   char* CL_currentVersion;
   bool  CL_user_track = false;
   char* CL_latestNews;
   char* CL_categories;
   char* CL_longAlternativeServersList;
   char* CL_httpPort;
   char* CL_httpAlternativeServersList;
   bool  CL_ngpmaker = false;
   uint32 CL_periodicTrafficUpdateInterval = MAX_UINT32;
   
   // Parse the commandline
   auto_ptr<CommandlineOptionHandler> 
      realCoh( new CommandlineOptionHandler( argc, argv ) );

   CommandlineOptionHandler& coh = *realCoh;
   coh.setSummary("This server is for the Navigator.");
   
   coh.addOption( "-d", "--port",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_port, "\0",
                  "Set the port to listen for requests on. " );

   coh.addOption("-L", "--save-simulator-route",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_saveSimulatorRoute, "",
                  "Save the routes taken as MapViewer simulator routes,"
                  "(Default is not to save. Not used in eBox)");

   coh.addOption( "", "--alternativeServersList",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_alternativeServersList, "",
                  "A list of servers that clients should use, "
                  "include this server if this should be used. "
                  "Format: host:port[[,|;]host:port]*" );

   coh.addOption( "", "--force_redirect",
                  CommandlineOptionHandler::presentVal,
                  1, &CL_forceRedirect, "F",
                  "Send alternativeServersList and redirect status to "
                  "all who connects." );

   coh.addOption( "", "--current_client_versions",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_currentVersion, "",
                  "A list of version of clients "
                  "Format: Program version;Client type;"
                  "Client type options;Resource version"
                  "[,Program version;Client type;Client type options;"
                  "Resource version]*"
                  "\r\nExample: 1.1.2;wf-s-60-m;N7650;1.0.2" );

   coh.addOption( "-f", "--user_track",
                  CommandlineOptionHandler::presentVal,
                  1, &CL_user_track, "F",
                  "Store user track positions, default is not to "
                  "store." );
   
   coh.addOption( "", "--latest_news",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_latestNews, "",
                  "A path to the latest new files. The files must be "
                  "named like \"client-type '_' lang '_' WFSubscr_t '_' "
                  "DaysLeft _ '.' ext\".\r\n"
                  "client-type is the client type identificatin string."
                  "\r\n"
                  "lang is the iso-639-1 string for the language.\r\n"
                  "WFSubscr_t is the subscription type {t|s|g}.\r\n"
                  "DaysLeft is the number of days left, inf|0|7|14|30.\r\n"
                  "ext is the file type extension for the client-type."
                  "\r\n");

   coh.addOption( "", "--categories",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_categories, "",
                  "A dir with a list of category files. The files must be "
                  "named like \"'wfcat' _ lang '.txt'\" where\r\n"
                  "lang is the iso-639-1 string for the language.\r\n");

   coh.addOption( "", "--longAlternativeServersList",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_longAlternativeServersList, "",
                  "A list of servers that clients should use, "
                  "include this server if this should be used. "
                  "Allowed to be longer than 255 chars. Is only used for "
                  "protoVer >= 0xa.\r\n"
                  "Format: host:port[[,|;]host:port]*" );

   // the "--unsecport" in XMLServer
   // Http port
   coh.addOption( "", "--httpport",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_httpPort, "",
                  "Set the port to listen for http requests on." );

   coh.addOption( "", "--httpAlternativeServersList",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_httpAlternativeServersList, "",
                  "A list of servers that clients should use, "
                  "include this server if this should be used. "
                  "This list is for clients communicating over HTTP.\r\n"
                  "Format: host:port[[,|;]host:port]*" );


   // CL_ngpmaker
   coh.addOption( "", "--ngpmaker",
                  CommandlineOptionHandler::presentVal,
                  1, &CL_ngpmaker, "F",
                  "Run the ngpmaker not NavigatorServer." );

   //uint32 CL_periodicTrafficUpdateInterval = MAX_UINT32;
   coh.addUINT32Option( "", "--ptui", 1, 
                        &CL_periodicTrafficUpdateInterval, MAX_UINT32,
                        "Sets the server-set value for the periodic "
                        "traffic informations update interval. If not "
                        "set the value will be set by the "
                        "NAV_PERIODIC_TRAFFIC_UPDATE_INTERVAL in the "
                        "mc2.prop file." );



   PTServer* ptserver = new NavPTServer( CL_latestNews, "NavigatorServer",
                                         realCoh.release(),
                                         ServerTypes::NAVIGATOR );
   ptserver->parseCommandLine();

   // Run the ngpmaker then exit
   if ( CL_ngpmaker ) {
      ngpmaker n( argc, argv );
      n.doIt();
      exit( 0 );
   }

   // Try properties if no alternativeServersList set on commandline
   if ( CL_alternativeServersList == NULL ) {
      CL_alternativeServersList = NewStrDup::newStrDupNull( 
         Properties::getProperty( "NAV_ALTERNATIVE_SERVERS_LIST" ) );
   }

   if ( CL_longAlternativeServersList == NULL ) {
      CL_longAlternativeServersList = NewStrDup::newStrDupNull( 
         Properties::getProperty( "NAV_LONG_ALTERNATIVE_SERVERS_LIST" ) );
   }

   if ( CL_httpAlternativeServersList == NULL ) {
      CL_httpAlternativeServersList = NewStrDup::newStrDupNull( 
         Properties::getProperty( "NAV_HTTP_ALTERNATIVE_SERVERS_LIST" ) );
   }
   

   if ( ptserver->getHttpsFD() != 0 ) {
      // Not used close it
      ::close( ptserver->getHttpsFD() );
      ptserver->setHttpsFD( 0 );
   }

   if ( CL_categories ) {
      Properties::insertProperty( "WF_CATEGORIES_DIR", CL_categories );
   }

   if ( CL_periodicTrafficUpdateInterval == MAX_UINT32 ) {
      CL_periodicTrafficUpdateInterval = 
         Properties::getUint32Property("NAV_PERIODIC_TRAFFIC_UPDATE_INTERVAL");
   }

   // initialize subsystem and delete helpers
   PropertyHelper::PropertyInit propInit;
   XMLTool::XMLInit xmlInit;
   SSLCleaner sslCleaner;

   // NavParserThreadGroup
   // TODO: Add command line agruments for queue-sizes.
   int minNbrThreads = min( ptserver->getMinNbrThreads(), 
                            ptserver->getMaxNbrThreads() );

   NavParserThreadGroup* group = 
      new NavParserThreadGroup( "NavParserThreadGroup", 
                                minNbrThreads, ptserver->getMaxNbrThreads() );
   ISABThreadGroupHandle groupHandle = group;

   ptserver->setGroup( group );

   {
      bool oneAdded = false;
      if ( CL_port != NULL ) {
         oneAdded = true;
         ptserver->
            addInterfaceFactory( new 
                                 TcpInterfaceFactory( group, 
                                                      CL_port ) );
      }

      // Set timeout high so Wayfinder client times out first
      HttpInterfaceRequest::setMaxSocketWaitTime( 180000000 );
      // Set nbr reuses high so less connection close from server
      HttpInterfaceRequest::setMaxSocketReuses( 1024 );
      if ( ptserver->getHttpFD() != 0 ) {
         ptserver->
            addInterfaceFactory( new TcpInterfaceFactory( group, 
                                                          ptserver->getHttpFD(),
                                                          true/*http*/ ));
      } else if ( CL_httpPort != NULL ) {
         ptserver->
            addInterfaceFactory( new TcpInterfaceFactory( group, 
                                                          CL_httpPort, 
                                                          true/*http*/ ));
      } else if ( ! oneAdded ) {
         mc2log << fatal << "NavigatorServer no ports to listen "
                << "on, exiting!" << endl;
         exit( 1 );
      }
   }

   group->setSimulatedRouteFileName( CL_saveSimulatorRoute );
   group->setCategories( CL_categories );
   group->setAlternativeServersList( CL_alternativeServersList );
   group->setLongAlternativeServersList( CL_longAlternativeServersList );
   group->setHttpAlternativeServersList( CL_httpAlternativeServersList );
   group->setForceRedirect( CL_forceRedirect );
   group->setCurrentVersion( CL_currentVersion );
   group->setNoUserTrack( !CL_user_track );
   group->setPeriodicTrafficUpdateInterval( CL_periodicTrafficUpdateInterval );

   ptserver->setupCTX();

   ptserver->start();
   ptserver->callGotoWork();

   mc2log << info << "NavigatorServer done" << endl;

   delete ptserver;
   ptserver = NULL;
   groupHandle = NULL;

   return 0;

} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "NavigatorServer: " << e.what() << endl;
   return 1;
} catch ( const ComponentException& e ) {
   mc2log << error << "NavigagorServer: " << e.what() << endl;
   return 2;
}
