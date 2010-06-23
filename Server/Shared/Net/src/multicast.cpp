/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "multicast.h"

#include "StringUtility.h"
#include "MC2String.h"
#include "Properties.h"
#include "IPnPort.h"
#include "PropertyHelper.h"
#include "NetUtility.h"

#include <map>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>


char MultiCastProperties::
c_moduleTypeIPs[ MODULE_TYPE_MAX_MODULE_NUMBER][2][30];

uint32
MultiCastProperties::c_moduleTypePorts[ MODULE_TYPE_MAX_MODULE_NUMBER][2];

bool
MultiCastProperties::m_bInitialized = false;

const char*
MultiCastProperties::getIP( moduletype_t moduleType, bool leader )
{
   if ( ! m_bInitialized ) {
      initialize();
   }

   const char* retVal = c_moduleTypeIPs[ moduleType ][ leader ];

   mc2dbg8 << "[MultiCastProperties]: getIP("
           << ModuleTypes::moduleTypeToString(moduleType)
           << ", " << (leader ? "true" : "false" ) << ") = "
           << retVal
           << endl;
   
   return retVal;
}

uint32
MultiCastProperties::getPort( moduletype_t moduleType, bool leader )
{
   if ( ! m_bInitialized ) {
      initialize();
   }

   uint32 retVal = c_moduleTypePorts[ moduleType][leader];
   
   mc2dbg8 << "[MultiCastProperties]: getPort("
           << ModuleTypes::moduleTypeToString(moduleType)
           << ", " << (leader ? "true" : "false" ) << ") = "
           << retVal
           << endl;

   
   return retVal;
}

uint32
MultiCastProperties::getModulePort( moduletype_t moduleType ) {
   return 3000 + getPort( moduleType, true ); // get leader port
}

moduletype_t
MultiCastProperties::getTypeFromPort( uint32 port )
{
   for ( int i = 0; i < MODULE_TYPE_MAX_MODULE_NUMBER; ++i ) {
      for ( int leader = 0; leader < 2; ++leader ) {
         if ( getPort( moduletype_t(i), leader ) == port ) {
            return moduletype_t(i);
         }
      }
   }
   return MODULE_TYPE_MAX_MODULE_NUMBER;
}

uint32
MultiCastProperties::getNumericIP( moduletype_t moduleType, bool leader )
{
   return ntohl( inet_addr( getIP( moduleType, leader ) ) );
}

static MC2String getPropName( int moduleType,
                              bool leader,
                              bool ip )
{
   MC2String moduleName (
      ModuleTypes::moduleTypeToString( moduletype_t( moduleType ) ) );
   MC2String leaderOrAvail = leader ? "_LEADER" : "_AVAILABLE";
   MC2String portorip = ip ? "_IP" : "_PORT";
   return StringUtility::copyUpper( moduleName + leaderOrAvail + portorip );
}

void MultiCastProperties::initialize()
{
   // Make the start address. Byte 1 is reserved for mapSet.
   uint32 startAddr = NetUtility::aton( "225.1.0.1" );
   MC2_ASSERT( startAddr != MAX_UINT32 );

   // Init module types
   for ( int i = 0; i < MODULE_TYPE_MAX_MODULE_NUMBER; ++i ) {
      if ( i == MODULE_TYPE_INVALID ) {
         // No need to complain about this one.
         continue;
      }
      try {
         // get "module type"_MODULE_NET property
         MC2String netPropName = StringUtility::
            copyUpper( ModuleTypes::
                       moduleTypeToString( moduletype_t( i ) ) ) +
                       "_MODULE_NET";
         uint32 net = PropertyHelper::get<uint32>( netPropName.c_str() );

         // module = 1 = leader, 0 = avail
         for ( int module = 0; module < 2; ++module ) {
            //
            //   +-+-+-+-+----------------------+
            //   |1 1 1 0|  MULTICAST Address   | 225.0.0.0 - 239.255.255.255
            //   +-+-+-+-+----------------------+
            //
            // mod_offset = 2*i + module + ( net << 16 )
            // port_mod_offset = 2*i + module + net * 80
            // The address will be 225.1.0.1 + mod_offset
            // Example ( net = 1 ):
            //  225.1.0.1 = first module type, leader, 
            //  225.1.0.2 = first module type, avail,
            //
            // The port is determined by 5000 + port_mod_offset
            // Example ( net = 1 ):
            //  5080 = first module type, avail
            //  5081 = first module type, leader
            //  5082 = second module type, avail
            //  5083 = second module type, leader
            //  ...
            uint32 mod_base        = 2*i + module;
            uint32 mod_offset      = mod_base + ( net << 16 );
            // Room for some modules. Currently there were 18
            uint32 port_mod_offset = mod_base + net * (40 * 2);
            uint32 addr = startAddr + mod_offset;
            MC2_ASSERT( NetUtility::isMulticast( addr ) );
            // For mapSets. They will use those bits.
            MC2_ASSERT( ( addr & ( 7 << 8 )) == 0 );

            strncpy( c_moduleTypeIPs[ i ][ module ], 
                     NetUtility::ip2str( addr ).c_str(), 30 );

            c_moduleTypePorts[ i ][ module == 0 ] = 5000 + port_mod_offset;
            MC2_ASSERT( c_moduleTypePorts[ i ][ module == 0 ] < 8000 );
            // For mapSets. They will use those bits.
            MC2_ASSERT((c_moduleTypePorts[ i ][ module == 0 ] & (7<<13)) == 0);
            // The "own-port"
            MC2_ASSERT( c_moduleTypePorts[ i ][ module == 0 ] + 3000 < 65535 );
         }
      } catch ( const PropertyException& e )  { 

         // old code, parse the following properties:
         // ( * = module type )
         // *_LEADER_IP 
         // *_AVAILABLE_IP
         // *_LEADER_PORT 
         // *_AVAILABLE_PORT
         // *_MODULE_PORT


         mc2log << warn << e.what() 
                << " . Will try to use old obsolete property"
                << endl;
            
         // For leader and available
         for( int j = 0; j < 2; ++j ) {
            bool leader = j == 0;
            // Get IP property.
            MC2String ipProp = getPropName( i, leader, true );
            char defaultIP[30];
            sprintf( defaultIP, "225.2.2.%u", 50 + i + j );

            const char* ip = Properties::getProperty( ipProp.c_str(),
                                                      defaultIP );
         
            strcpy( c_moduleTypeIPs[i][leader], ip );

            mc2dbg8 << "[MultiCastProperties]: "
                    << ipProp << " = " << ip << endl;
              
            // Get the port property.
            MC2String portProp = getPropName( i, leader, false );
         
            uint32 port = Properties::getUint32Property( portProp.c_str(),
                                                         5050 + i + j );
            mc2dbg8 << "[MultiCastProperties]: "
                    << portProp << " = " << port << endl;


         
            c_moduleTypePorts[i][leader] = port;         
         }
      }

   }
   
   m_bInitialized = true;
}


uint16 MultiCastProperties::changeMapSetPort( uint16 port ) 
{
   return changeMapSetAddr( IPnPort( 0, port ) ).getPort();
}

IPnPort MultiCastProperties::changeMapSetAddr( const IPnPort& addr ) 
{
   uint32 mapSet = Properties::getMapSet();
   return IPnPort( addr.getIP() + (mapSet << 8),
                   addr.getPort() | ( mapSet << 13 ) );
}
