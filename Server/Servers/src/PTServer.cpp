/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PTServer.h"
#include "FileUtils.h"
#include "GenericServer.h"
#include "CommandlineOptionHandler.h"
#include "InterfaceFactory.h"
#include "DeleteHelpers.h"
#include "SelectableInterfaceIO.h"
#include "ThreadRequestHandler.h"
#include "InterfaceParserThreadGroup.h"
#include "SSLSocket.h"
#include "NewStrDup.h"
#include "TimedOutSocketLogger.h"

#include <unistd.h>

PTServer::PTServer( const MC2String& name,
                    CommandlineOptionHandler* handler,
                    ServerTypes::servertype_t type,
                    bool pushService ):
   GenericServer( name.c_str(), handler, type, pushService ),
   m_group( NULL ),
   m_CL_clientSettings( NULL ),
   m_CL_serverLists( NULL ),
   m_clientSettings( NULL ),
   m_namedServerlists( NULL ), 
   m_certFile( NULL ),
   m_minNbrThreads( 5 ),
   m_maxNbrThreads( 20 ),
   m_httpFD( -1 ),
   m_httpsFD( -1 )
{
   getCache()->setMaxSize( 1000000 );

   // setup client settings and server list options
   handler->addOption( "", "--client_settings",
                       CommandlineOptionHandler::stringVal,
                       1, &m_CL_clientSettings, "",
                       "File with the client type settings." );

   handler->addOption( "", "--server_lists", 
                       CommandlineOptionHandler::stringVal,
                       1, &m_CL_serverLists, "",
                       "File with named server lists. If any client type "
                       "specified in --client_settings uses a named server "
                       "list, that list must be specified in this file." );

   handler->addOption( "", "--minnumberthreads",
                       CommandlineOptionHandler::uint32Val,
                       1, &m_minNbrThreads, "5",
                       "The minimum number of threads that handles requests in"
                       " parallel."
                       "(Default is 5 threads)" );

   handler->addOption( "", "--maxnumberthreads",
                       CommandlineOptionHandler::uint32Val,
                       1, &m_maxNbrThreads, "20",
                       "The maximum number of threads that handles requests in"
                       " parallel. Started on high load."
                       "(Default is 20 threads)" );

   handler->addOption( "", "--certfile",
                       CommandlineOptionHandler::stringVal,
                       1, &m_certFile, "httpd.pem",
                       "Set the certificate file. "
                       "Default value is \"httpd.pem\".");

   handler->addOption( "", "--httpfd",
                       CommandlineOptionHandler::integerVal,
                       1, &m_httpFD, "0",
                       "Set the HTTP fd, used i.s.o. --httpport.");

   handler->addOption( "", "--httpsfd",
                       CommandlineOptionHandler::integerVal,
                       1, &m_httpsFD, "0",
                       "Set the HTTPS fd, secured port.");
}

PTServer::~PTServer() {
   try {
      m_group->destroy();
      if ( ! m_group->isDestroyed() ) {
         m_group->destroy();
      }
   } catch ( const JTCException& e ) {
      mc2log << fatal << "PTServer shutdown: " << e << endl;
   }
   STLUtility::deleteValues( m_interfaceFactories );
#ifdef USE_SSL
   if ( m_ctx.get() != NULL ) {
      m_ctx->exitCleanup();
   }
   ERR_remove_state( 0 );
#endif
   mc2dbg << "PTServer dead." << endl;
}

namespace {
bool replaceFileNameWithContent( const char* name, char*& ptr, 
                                 const char* errorTag ) 
{
   if ( ! FileUtils::getFileContent( name, ptr ) ) {
      mc2dbg << "Failed to get content for " << errorTag << endl;
      return false;
   }

   return true;
}

}

void 
PTServer::addInterfaceFactory( InterfaceFactory* factory ) {
   m_interfaceFactories.insert( factory );
}

int
PTServer::handleCommand( const char* input ) {
   MC2String token( input );
   int ret = 0;

   if ( token == "status" ) {
      cout << " Status:";
      m_group->printStatus( cout );
      cout << endl;
   } else {
      ret = Component::handleCommand( input );
   }

   if ( ret == -1 ) {
      m_group->shutDown();

      // must wait for group threads to finish before shutting down the server
      // due to relation beteen thread request handler and generic server
      mc2dbg << "[PTServer] waiting for all threads in group to end." << endl;
      while ( m_group->activeCount() > 0 ) {
         usleep( 500 );
      }
      // now we can shutdown the server safely
      GenericServer::shutdown();
      
   }
   return ret;
}

void
PTServer::helpCommands() {
   // Called by Component so subclasses can print their command help to cout.
}

void 
PTServer::parseCommandLine() throw ( ComponentException ) {

   Component::parseCommandLine();
   
   const char* defaultClientSettings = "# EOF\r\n";

   // parse server list and client settings

   if ( m_CL_serverLists != NULL ) {
      char* t = NULL;
      bool ok = ::replaceFileNameWithContent( 
         m_CL_serverLists, t, "ServerLists" );
      if ( !ok ) {
         MC2String err( "Problem with ServerLists file " );
         err += MC2String("\"") + m_CL_serverLists + "\"";
         throw ComponentException( err );
      } else {
         m_namedServerlistsData.reset( t );
      }
      m_namedServerlists = t;
   }

   if ( m_CL_clientSettings == NULL ) {
       // allocate on heap since CommandlineOptionHandler::deleteTheStrings
       // will delete the string
       m_CL_clientSettings = NewStrDup::newStrDup( defaultClientSettings );
   } else { // Read file
      char* t = NULL;
      bool ok = ::replaceFileNameWithContent( 
         m_CL_clientSettings, t, "ClientSettings" );
      if ( !ok ) {
         MC2String err( "Problem with ClientSettings file " );
         err += MC2String("\"") + m_CL_clientSettings + "\"";
         throw ComponentException( err );
      } else {
         m_clientSettingsData.reset( t );
      }
      m_clientSettings = t;
   }
}

void
PTServer::setupCTX() {
   // setup SSL context

#ifdef USE_SSL
   const char pathSep = '/';
   const char* path = "./";
   const char* certFilename = m_certFile;
   char* pos = StringUtility::strrchr( certFilename, pathSep );
   if ( pos != NULL ) {
      // Split certFile in path and file
      *pos = '\0';
      certFilename = ++pos;
      path = m_certFile; // original filename
   }
   m_ctx.reset( SSLSocket::
                makeNewCTX( SSLSocket::SSL_SOCKET_CLIENTS_AND_SERVERS,
                            path,
                            certFilename,
                            certFilename,
                            certFilename,
                            NULL) );
#endif

}

void
PTServer::init() {
   // init common group elements

   m_group->setShutdownPipe( getShutdownPipe() );

   if ( m_group->getClientSettingStorage() == NULL ) {
      m_group->setClientSettingStorage( m_clientSettings );
   }
   m_group->setNamedServerLists( m_namedServerlists );
#ifdef USE_SSL
   m_group->setSSLContext( m_ctx.get() );
#endif
   ThreadRequestHandler* handler = 
      new ThreadRequestHandler( this, m_group );

   handler->start();
   m_group->start( handler,
                   new SelectableInterfaceIO( m_group ), 
                   m_interfaceFactories, 
                   new TimedOutSocketLogger( m_group ));

   for_each( m_interfaceFactories.begin(), m_interfaceFactories.end(),
             mem_fun( &InterfaceFactory::start ) );
}

