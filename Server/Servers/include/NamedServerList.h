/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAMEDSERVERLIST_H
#define NAMEDSERVERLIST_H


#include "config.h"
#include "MC2String.h"

class NamedServerGroup;


/**
 * Class for holding a server list.
 *
 */
class NamedServerList {
   public:
      /**
       * Constructor.
       */
      NamedServerList( const MC2String& serverList, uint32 crc );


      /**
       * Destructor.
       */
      ~NamedServerList();


      /**
       * Get the server list string.
       */
      const MC2String& getServerListString() const;


      /**
       * Set the server list string.
       */
      void setServerListString( const MC2String& s );


      /**
       * Get the server list crc.
       */
      uint32 getServerListCRC() const;


      typedef vector< NamedServerGroup > group_vector_t;


      /**
       * Get the groups.
       */
      const group_vector_t& getGroups() const;
      

   private:
      /// The server list string.
      MC2String m_serverListStr;


      /// The server list crc.
      uint32 m_serverListCRC;


      /// The groups.
      group_vector_t m_groups;


      /// Friend NamedServerLists
      friend class NamedServerLists;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline const MC2String&
NamedServerList::getServerListString() const {
   return m_serverListStr;
}


inline void
NamedServerList::setServerListString( const MC2String& s ) {
   m_serverListStr = s;
}


inline uint32
NamedServerList::getServerListCRC() const {
   return m_serverListCRC;
}


inline const NamedServerList::group_vector_t&
NamedServerList::getGroups() const {
   return m_groups;
}


#endif // NAMEDSERVERLIST_H

