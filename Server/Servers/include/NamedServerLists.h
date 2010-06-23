/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAMEDSERVERLISTS_H
#define NAMEDSERVERLISTS_H

#include "config.h"
#include "MC2String.h"

#include <map>

class NamedServerList;


/**
 * This class holds named server lists. Each list of servers contained
 * in this class is keyed to a name and a type. Using the name and
 * type a serverlist can be retrieved.
 *
 */
class NamedServerLists {
   public:
      /** 
       * The key type.
       */
      typedef std::pair<MC2String, MC2String> server_key_t;


      /**
       * The value type.
       */
      typedef NamedServerList* server_value_t;


      /** 
       * The map type.
       */
      typedef std::map<server_key_t, server_value_t> server_map_t;


      /** 
       * Constructor.
       */
      NamedServerLists();


      /**
       * Destructor. 
       */
      ~NamedServerLists();


      /**
       * Parses the content of an already read file and constructs the
       * map of <name,type> -> list.
       * 
       * The list follows this syntax:
       * # comment
       * : <name>
       * <type> <list>
       * <type2> <list2>
       *
       * @param serverFile The contents of the serverfile as one long 
       *                   string. 
       */
      void parseServerFile( const char* serverFile );


      /**
       * Fetches the list associated with a name and a key.
       * If no list is found it returns NULL.
       * @param name The name.
       * @param type The type.
       * @return The server list or NULL.
       */
      const char* getServerList( const char* name, 
                                 const char* type ) const;


      /**
       * Finds the list associated with a name and a key.
       * If no list is found it returns the empty string.
       * @param name The name.
       * @param type The type.
       * @return The server list or the empty string.
       */
      const MC2String& findServerList( const MC2String& name, 
                                       const MC2String& type ) const;


      /**
       * Finds the list associated with a name and a key.
       * 
       * @param name The name.
       * @param type The type.
       * @return The server list or NULL.
       */
      const NamedServerList* findNamedServerList( 
         const MC2String& name, const MC2String& type ) const;


      /**
       * Finds the CRC of the list associated with a name and a key.
       * If no list is found it returns 0.
       * @param name The name.
       * @param type The type.
       * @return The CRC of the server list or 0.
       */    
      uint32 getServerListsCRC( const MC2String& listName,
                               const MC2String& type ) const;


      /**
       * Adds a new server list, unless the name - type pair is already
       * present.
       *
       * @param name The name of the new list.
       * @param type The type of the new list.
       * @param serverlist The new list.
       * @return True if the list was added, false if the 
       *         name - type pair was already present.
       */
      bool addServerList( const MC2String& name, const MC2String& type,
                          const MC2String& serverlist );


      /**
       * Adds a new server list. If the name - type pair is already
       * present the new list replaces the old list. 
       *
       * @param name The name of the new list.
       * @param type The type of the new list.
       * @param serverlist The new list.
       */
      bool replaceServerList( const MC2String& name, const MC2String& type,
                              const MC2String& serverlist );


      /**
       * Outputs the whole table to a stream.
       * @param stream The stream that will receive the output. 
       */
      std::ostream& output( std::ostream& stream ) const;


   private:
      /**
       * Parses a "group line" of a server file. 
       *
       * A group line matches this regexp: '^:[ ]*\([^ \n]\)\n', the
       * group name is the kept part.
       *
       * @param name The name of the group if the line was succesfully
       *             parsed.
       * @param data The data that should be parsed.
       * @param lineNum The line number of the data. Used for log messages.
       * @return True if parsed, false if the parse failed. 
       */
      static bool parseGroup( MC2String& name, const char* data );


      /**
       * Parses a serverlist line of the serverlist file.
       *
       */
      int parseAndInsertListLine( const MC2String& name, 
                                  const char* data );


      /**
       * Make a NamedServerList from a server list string.
       *
       * @param serverList The server list string.
       * @param type The type of server list.
       * @return The NamedServerList pointer, NULL if not ok server list
       *          string.
       */
      NamedServerList* checkAndCreateServersList( 
         const char* serverList, const char* type ) const;


      /** 
       * The map that holds the list.
       */
      server_map_t m_lists;


      /** 
       * The empty string sometimes returned by findServerList. It needs
       * to be a member so that we can return a const reference
       * to an empty string.
       */
      static const MC2String m_emptyString;


   public:
      /// The Default Group.
      static const MC2String DefaultGroup;


      /// The Navigator Server Type.
      static const MC2String NavServerType;


      /// The Short Navigator Server Type.
      static const MC2String NavServerShortType;


      /// The Http Navigator server type.
      static const MC2String HttpServerType;


      /// The XML server type.
      static const MC2String XmlServerType;
};


#endif // NAMEDSERVERLISTS_H

