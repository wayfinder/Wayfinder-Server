/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MULTICAST_H
#define MULTICAST_H

#include "config.h"
#include "Properties.h"
#include "sockets.h" // inet_addr
#include "ModuleTypes.h"

class IPnPort;

/**
 *    Class that encapsulates ip and port settings for modules. The 
 *    values of the settings are initialized when the first call to 
 *    getIP() or getPort() is made. This way a newly created module is 
 *    able to use an optional property file. \\
 *    
 *    How to add entries for a new module:
 *    \begin{enumerate}
 *       \item Add constant to \emph{property_t}. This is also the constant 
 *             that will be used when retrieving the value at run-time.
 *       \item Add string that identifies the ip in a property file to the 
 *             \emph{m_szProperyIP} array.
 *       \item Add string that identifies the port in a property file to 
 *             the \emph{m_szProperyPort} array.
 *       \item Add default ip to the \emph{m_szDefaultIP} array.
 *       \item Add default port to the \emph{m_szDefaultPort} array.
 *    \end{enumerate}
 *
 */
class MultiCastProperties
{
public:

      /**
       *    Get ip for a module.
       *    @param moduleType Module type.
       *    @param leader     True if leader. False for available.
       *    @return The ip as a string.
       */
      static const char* getIP( moduletype_t moduleType, bool leader );

      /**
       *    Return ntohl( inet_addr( getIP( moduleType, leader ) )
       */
      static uint32 getNumericIP( moduletype_t moduleType, bool leader );

      /**
       *    Get port for a module.
       *    @param moduleType Module type.
       *    @param leader     True if leader. False for available.
       */
      static uint32 getPort( moduletype_t moduleType, bool leader );

      /**
       * Gets main port for a module.
       * This one uses the getPort and changes the port number to something
       * else so it can not collide with avail and leader ports
       * @param moduleType the type of module to get the port for
       * @return port number
       */
       static uint32 getModulePort( moduletype_t moduleType );

      /**
       *    Returns MODULE_TYPE_MAX_MODULE_NUMBER if not found.
       */
      static moduletype_t getTypeFromPort( uint32 port );

      static uint16 changeMapSetPort( uint16 port );
      static IPnPort changeMapSetAddr( const IPnPort& org );
   protected:
      /**
       *    Set values to the property arrays.
       */
      static void initialize();

      /// Initially false but set to true at the first call to initialize().
      static bool m_bInitialized;

      /// This is where the ip addresses are stored
      static char c_moduleTypeIPs[ MODULE_TYPE_MAX_MODULE_NUMBER][2][30];
      /// This is where the ports are stored
      static uint32 c_moduleTypePorts[ MODULE_TYPE_MAX_MODULE_NUMBER][2];
};

#endif // MULTICAST_H
