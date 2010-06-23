/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAMESSERVERGROUP_H
#define NAMESSERVERGROUP_H


#include "config.h"
#include "MC2String.h"


/**
 * Class for holding a server group.
 *
 */
class NamedServerGroup {
   public:
      enum server_group_t {
         /// Server
         Server = 0,
         /// Backup
         Backup = 1,
         /// Config
         Config = 2,
         /// Map
         Map    = 3,
         /// Tile Map
         TMap   = 4,
      };


      /**
       * Constructor.
       */
      NamedServerGroup( server_group_t type, 
                        uint32 switch_group_threshold );


      /**
       * Destructor.
       */
      ~NamedServerGroup();


      typedef vector< MC2String > server_vector_t;


      /**
       * Get the servers.
       */
      const server_vector_t& getServers() const;


      /**
       * Get the type of group.
       */
      server_group_t getType() const;


      /**
       * Get the number of servers to test in the group before switching 
       * to next.
       */
      uint32 getSwitchGroupThreshold() const;


      /**
       * Set the type of group.
       */
      void setType( server_group_t t );


      /**
       * Get type as string.
       */
      static const char* getTypeAsString( server_group_t t );


   private:
      /// The type.
      server_group_t m_type;


      /// The switch_group_threshold.
      uint32 m_switch_group_threshold;


      /// The servers.
      server_vector_t m_servers;


      /// Friend NamesServerLists
      friend class NamedServerLists;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline NamedServerGroup::server_group_t
NamedServerGroup::getType() const {
   return m_type;
}


inline uint32
NamedServerGroup::getSwitchGroupThreshold() const {
   return m_switch_group_threshold;
}


inline void
NamedServerGroup::setType( server_group_t t ) {
   m_type = t;
}


inline const NamedServerGroup::server_vector_t&
NamedServerGroup::getServers() const {
   return m_servers;
}


#endif // NAMESSERVERGROUP_H

