/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PTSERVER_H
#define PTSERVER_H

#include "config.h"

#include "Component.h"
#include "MC2String.h"
#include "ScopedArray.h"
#include "ISABThread.h"
#include "GenericServer.h"
#include "ServerTypes.h"

#include <set>

class SSL_CONTEXT;
class GenericServer;
class ParserThreadGroup;
class InterfaceFactory;
class InterfaceParserThreadGroup;


/**
 * Common class for the "real" servers, such as XML and Navigator.
 *
 */
class PTServer: public GenericServer {
public:
   /**
    * @param name the name of this component
    * @param handler command line options
    * @param type the server type, XML, NAVIGATOR etc
    */
   PTServer( const MC2String& name,
             CommandlineOptionHandler* handler,
             ServerTypes::servertype_t type,
             bool pushService = false );
   virtual ~PTServer();

   /**
    * @param group the main group
    */
   void setGroup( InterfaceParserThreadGroup* group ) { 
      m_group = group;
   }

   /// @return the main interface thread group
   InterfaceParserThreadGroup* getGroup() {
      return m_group;
   }
   /**
    * Adds an interface to this server. The interface will be destroyed
    * by this instance.
    * @param factory an interface factor to add
    */
   void addInterfaceFactory( InterfaceFactory* factory );
   /// @see Component
   void parseCommandLine() throw ( ComponentException );
   /// @see Component
   virtual void init();

   /// @see Component
   int handleCommand( const char* input );

   /// @see Component
   void helpCommands();

   /// set up ssl context
   void setupCTX();

   /// @return ssl context pointer
   SSL_CONTEXT* getCTX() { return m_ctx.get(); }
   /// @return http file descriptor
   int getHttpFD() const { return m_httpFD; }
   /// sets the https file descriptor
   void setHttpsFD( int fd ) { m_httpsFD = fd; }
   /// @return https file descriptor
   int getHttpsFD() const { return m_httpsFD; }
   /// @return the minimum number of threads to start
   uint32 getMinNbrThreads() const { return m_minNbrThreads; }
   /// @return the maximum number of threads to start
   uint32 getMaxNbrThreads() const { return m_maxNbrThreads; }
   /// @return name of the certifaction file
   const char* getCertFilename() const { return m_certFile; }

protected:
   InterfaceParserThreadGroup* m_group; ///< main thread group
   set< InterfaceFactory* > m_interfaceFactories; ///< all interface factories
   ScopedArray<char> m_clientSettingsData;  ///< client settings data
   ScopedArray<char> m_namedServerlistsData; ///< server list data
   char* m_CL_clientSettings;  ///< client settings data from command line
   char* m_CL_serverLists; ///< server list data from command line
   char* m_clientSettings;
   char* m_namedServerlists;
   char* m_certFile; /// < certification file for http
   uint32 m_minNbrThreads; ///< the minimum number of threads
   uint32 m_maxNbrThreads; ///< maximum number of threads
   int m_httpFD; ///< http file descriptor
   int m_httpsFD; ///< https file descriptor 
   auto_ptr<SSL_CONTEXT> m_ctx; ///< ssl context
};

#endif
