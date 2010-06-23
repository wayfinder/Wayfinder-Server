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
#include "RouteID.h"

#include<stdio.h>

RouteID::RouteID()
{
   m_id           = MAX_UINT32;
   m_creationTime = MAX_UINT32;
}

RouteID::RouteID(uint32 id, uint32 creationTime)
      : m_id(id), m_creationTime(creationTime)
{   
}

int
RouteID::isValid() const
{
   return ( m_id != 0 ) && ( m_creationTime != 0 ) &&
          ( m_id != MAX_UINT32 ) && ( m_creationTime != MAX_UINT32 );
}

RouteID::RouteID(const char* str)
{
   unsigned int id;
   unsigned int creationtime;
   if ( sscanf( str, "%X_%X",
                &id, &creationtime ) == 2 ) {
      // Valid route_id
      m_id = id;
      m_creationTime = creationtime;
   } else {
      mc2dbg2 << "RouteID::RouteID(const char*) "
         "invalid route_id string" << endl;
      m_id           = MAX_UINT32;
      m_creationTime = MAX_UINT32;
   }
}

RouteID::RouteID(int64 nav2routeid)
{
   m_id = int64(MAX_UINT32) & (nav2routeid >> 32);
   m_creationTime = int64(nav2routeid) & int64(MAX_UINT32);
}

int64
RouteID::toNav2Int64() const
{
   return int64( int64(m_id) << 32 ) | int64(m_creationTime);
}
   
bool 
RouteID::operator == ( const RouteID& other ) const
{
   return ( m_id == other.m_id ) && 
          ( m_creationTime == other.m_creationTime );
}

#ifdef MC2_SYSTEM

MC2SimpleString
RouteID::toString() const
{
   char* routeIDStr = new char[64];
   sprintf( routeIDStr, "%X_%X", m_id, m_creationTime );
   return MC2SimpleStringNoCopy( routeIDStr );
}

uint32
RouteID::getRouteIDNbr() const
{
   return m_id;
}

uint32
RouteID::getCreationTime() const
{
   return m_creationTime;
}
#endif // MC2_SYSTEM


