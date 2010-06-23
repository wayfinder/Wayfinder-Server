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
#include "NamedServerLists.h"
#include <iterator>
#include "StringUtility.h"
#include "MC2CRC32.h"
#include "NamedServerList.h"
#include "NamedServerGroup.h"
#include "STLStringUtility.h"
#include "Properties.h"

const MC2String NamedServerLists::m_emptyString = "";
const MC2String NamedServerLists::DefaultGroup = "default";
const MC2String NamedServerLists::NavServerType = "ns";
const MC2String NamedServerLists::NavServerShortType = "ns_short";
const MC2String NamedServerLists::HttpServerType = "http";
const MC2String NamedServerLists::XmlServerType = "xml";


//namespace std since the types we will output (std::pair) are in that
//namespace.
namespace std{
   /**
    * Prints a key type value to an ostream.
    */
   std::ostream&
   operator<<( std::ostream& stream, 
               const NamedServerLists::server_key_t& value ) 
   {
      return stream << "[" << value.first << "," << value.second << "]";
   }
   
   /**
    * Prints a value type value to an ostream.
    */
   std::ostream&
   operator<<( std::ostream& stream, 
               const NamedServerLists::server_value_t& value )
   {
      return stream << value->getServerListString() << " (0x" << hex 
                    << value->getServerListCRC() << dec << ")";
   }
   
   /**
    * Prints a map value type (key and value) value to an ostream.
    */
   std::ostream& 
   operator<<( std::ostream& stream, 
               const NamedServerLists::server_map_t::value_type& value )
   {
      return stream << value.first << " -> " << value.second;
   }
}


/**
 * Prints a NamedServerLists object to an ostream. 
 */
std::ostream&
operator<<( std::ostream& stream,
            const NamedServerLists& obj )
{
   return obj.output( stream );
}


std::ostream& 
NamedServerLists::output( std::ostream& stream ) const {
   std::copy( m_lists.begin(), m_lists.end(), 
              std::ostream_iterator<server_map_t::value_type>( 
                 stream, "\n" ) );
   return stream;
}


NamedServerLists::NamedServerLists() {
}


NamedServerLists::~NamedServerLists() {
   // Delete all newed
   for( server_map_t::iterator it = m_lists.begin(); it != m_lists.end() ;
        ++it )
   {
      delete it->second;
   }
}


bool
NamedServerLists::parseGroup( MC2String& name, const char* data ) {
   bool retval = false;
   const char* lineend = strchr( data, '\n' );
   int len = ( lineend != NULL ) ? ( lineend - data ) : strlen( data );
   char name_c[ len ];
   *name_c = '\0';
   int res = sscanf( data , "%*c%*[ ]%[^ \n]%*[ ]\n", name_c );
   if ( ( res == 1 ) && ( *name_c != '\0' ) ) {
      //ok
      name = name_c;
      retval = true;
   }
   return retval;
}


int 
NamedServerLists::parseAndInsertListLine( const MC2String& name, 
                                          const char* data)
{
   const char* lineend = strchr( data, '\n' );
   int len = ( lineend != NULL ) ? ( lineend - data ) : strlen( data );
   char serverlist[ len ];
   serverlist[ 0 ] = '\0';
   char servertype[ len ];
   servertype[ 0 ] = '\0';
   //serverlist names may only contain alpanumerics and hyphen
   int res = sscanf( data, "%[a-zA-Z0-9-_]%*[ ]%[^\n]\n", 
                     servertype, serverlist );
   if ( res == 2 && strlen( serverlist ) && strlen( servertype ) ) {
      //ok
      mc2dbg4 << "[NSL] new List " << name << " type " << servertype 
             << endl;

      NamedServerList* l = checkAndCreateServersList( 
         serverlist, servertype );
      if ( l != NULL ) {
         //serverlist parsed ok
         // Sanity check for short list
         if ( NavServerShortType == servertype && 
              l->getServerListString().size() > 255 )
         {
            mc2log << warn << "[NSL] " << NavServerShortType 
                   << " too long " << strlen( serverlist ) << " max 255." 
                   << endl;
            return -1; //serverlist corrupted
         } else {
            m_lists[ make_pair( MC2String( name ), 
                                MC2String( servertype ) ) ] = l;
            return 0; //ok
         }
      } else {
         return -1; //serverlist corrupted
      }
   } else {
      return -2; //line unparseable
   }
   return -2; // wtf? gcc 4.3.1 is strange
}


NamedServerList*
NamedServerLists::checkAndCreateServersList( 
   const char* serverList, const char* type ) const 
{
   int length = strlen( serverList );
   uint32 crc = MC2CRC32::crc32( (byte*)serverList, length );
   NamedServerList* l = new NamedServerList( serverList, crc );
   NamedServerGroup* g = NULL;
   int pos = 0;
   while ( pos < length && l != NULL ) {
      char proto[ length ];
      char host[ length ];
      char endChar = '\0';
      uint16 port;
      char remainder[ length ];
      char path[ length ];
      proto[ 0 ] = '\0';
      path[ 0 ] = '\0';
      uint32 protoLength = 0;
      int sres = -1;
      
      sres = sscanf( serverList + pos, "%[^:/]:%hu/%[^,;]%s", 
                     host, &port, path, remainder );
      if ( sres >= 3 ) {
         sres = sres -1;
      } else {
         sres = sscanf( serverList + pos, "%[^:/]:%hu%s", 
                        host, &port, remainder );
      }
      
      int urlRes = 0;
      if ( sres < 2 ) {
         urlRes = sscanf( serverList + pos, "%[^:]://%[^/:]:%hu/%[^,;]%s", 
                          proto, host, &port, path, remainder );
         if ( urlRes >= 4 ) {
            sres = urlRes - 2;
         } else {
            urlRes = sscanf( serverList + pos, "%[^:]://%[^/:]:%hu%s", 
                             proto, host, &port, remainder );
            if ( urlRes >= 3 ) {
               sres = urlRes - 1;
            }
         }
         if ( sres >= 2 ) {
            protoLength = strlen( proto ) + 3;
         }
      }

      if ( sres >= 2 ) {
//         mc2log << "[NSL] sscanf retuned " << sres << endl;
         // Ok skipp past this host
         int pathLen = strlen( path );
         if ( pathLen > 0 ) pathLen += 1; // The /
         char portTmp[ length ];
         int portLen = sprintf( portTmp, "%hu", port ) + 1/*:*/;
         int hostLen = strlen( host );
         pos += protoLength + hostLen + portLen + pathLen;
         if ( sres > 2 ) {
            endChar = serverList[ pos ];
            if ( endChar == ',' || endChar == ';' ) {
               pos += 1; // ',' or ';'
            } else {
               mc2log << fatal << "Not ',' or ';' after "
                      << "serverList entry, "
                      << "perhaps missing ',' or ';'. Error at: " 
                   << (serverList + pos) << endl;
               delete l;
               l = NULL;
            }
         } else { // sres == 2
            if ( pos != length ) {
               mc2log << fatal << "Last serverList entry not "
                      << "last, perhaps missing ',' or ';'" << endl;
               delete l;
               l = NULL;
            }
         }

         if ( l != NULL ) {
            if ( g == NULL ) {
               // Add new group
               l->m_groups.push_back( 
                  NamedServerGroup( NamedServerGroup::Server, 
                                    3/*Nbr svrs to test in group*/ ) );
               g = &l->m_groups.back();
               mc2dbg4 << "[NSL]  new group" << endl;
            }
            // Add host and port to g
            MC2String protoStr;
            if ( urlRes >= 3 ) {
               // If bhttp:// first the set backup, chttp:// -> config
               protoStr = proto;
               if ( protoStr == "bhttp" ) {
                  g->setType( NamedServerGroup::Backup );
                  protoStr.erase( 0, 1 );
               } else if ( protoStr == "chttp" ) {
                  g->setType( NamedServerGroup::Config );
                  protoStr.erase( 0, 1 );
               } else if ( protoStr == "mhttp" ) {
                  g->setType( NamedServerGroup::Map );
                  protoStr.erase( 0, 1 );
               } else if ( protoStr == "mthttp" ) {
                  g->setType( NamedServerGroup::TMap );
                  protoStr.erase( 0, 2 );
               } else if ( protoStr == "http" ) {
               } else if ( protoStr == "https" ) {
               } else if ( protoStr == "b" ) {
                  g->setType( NamedServerGroup::Backup );
                  protoStr.erase( 0, 1 );
               } else if ( protoStr == "m" ) {
                  g->setType( NamedServerGroup::Map );
                  protoStr.erase( 0, 1 );
               } else if ( protoStr == "mt" ) {
                  g->setType( NamedServerGroup::TMap );
                  protoStr.erase( 0, 2 );
               } else if ( protoStr == "c" ) {
                  g->setType( NamedServerGroup::Config );
                  protoStr.erase( 0, 1 );
               }
            }
            if ( !protoStr.empty() ) {
               protoStr += "://";
            }
            // Path, if any
            MC2String pathStr( path );
            if ( !pathStr.empty() ) {
               pathStr.insert( 0, "/" );
            }
            MC2String portStr;
            STLStringUtility::int2str( port, portStr );
            g->m_servers.push_back( protoStr + host + ":" + portStr + 
                                    pathStr );
            mc2dbg4 << "[NSL]   new " << NamedServerGroup::getTypeAsString(
               g->getType() ) << " " 
                   << (protoStr + host + ":" + portStr + pathStr ) << endl;
            if ( endChar == ';' ) {
               // New group
               g = NULL;
            }
         }
      } else if ( sscanf( serverList + pos, ":%s", remainder ) == 1 ) {
         const NamedServerList* o = findNamedServerList( remainder, type );
         delete l;
         if ( o != NULL ) {
            l = new NamedServerList( *o );
            mc2dbg4 << "[NSL]  Server list reference " <<  remainder 
                   << " " << type << endl;
         } else {
            mc2log << fatal << "serverList No such server list \""
                   << remainder << "\" Error at: " 
                   << (serverList + pos) << endl;
            l = NULL;
         }
         pos = length;
      } else {
         mc2log << fatal << "serverList "
                << "not valid! host:port[[,|;]host:port]*. Error at: " 
                << (serverList + pos) << endl;
         delete l;
         l = NULL;
      }
   }

   // Make server list string without bhttp just http
   if ( l != NULL ) {
      MC2String srvlst;
      for ( NamedServerList::group_vector_t::const_iterator git = 
               l->getGroups().begin() ; git != l->getGroups().end() ;
            ++git )
      {
         if ( git != l->getGroups().begin() ) {
            srvlst += ";";
         }
         for ( NamedServerGroup::server_vector_t::const_iterator sit =
                  (*git).getServers().begin() ; 
               sit != (*git).getServers().end() ; ++sit )
         {
            if ( sit != (*git).getServers().begin() ) {
               srvlst += ",";
            }
            srvlst += *sit;
         }
      }
      l->setServerListString( srvlst );
   }
   
   return l;
}



void 
NamedServerLists::parseServerFile(const char* data)
{
   int pos = 0;
   int line = 0;
   MC2String name = "";
   while ( StringUtility::trimStart( data + pos )[ 0 ] != '\0' ) {
      ++line;
      if ( StringUtility::trimStart( data + pos )[ 0 ] == '#' ) {
         //Comment
      } else if ( data[ pos ] == '\0' ) {
         //EOF
      } else if ( data[ pos ] == '\n' || 
                  (data[ pos ] == '\r' && data[ pos+1 ] == '\n' ) ) {
         //empty line
      } else if ( data[ pos ] == ':' ) {
         //new group 
         if ( parseGroup(name, data + pos) ) {
            mc2dbg4 << "New serverlist name: " << name << endl;
         } else {
            mc2log << fatal << "NamedServerList line" << line << " : "
                   << MC2CITE( data + pos ) << " unparseable." << endl;
            exit( 1 );
         }         
      } else {
         // get line
         int parseresult = parseAndInsertListLine( name, data + pos );
         switch ( parseresult ) {
         case 0: //ok;
            break;
         case -1: //
            mc2log << fatal << "Serverlist on line " << line << " : " 
                   << MC2CITE( data + pos ) << " is not correct " << endl;
            exit( 1 );
            break;
         case -2:
            mc2log << fatal << "NamedServerList line " << line << " : "
                   << MC2CITE( data + pos ) << " unparseable." << endl;
            exit( 1 );
            break;
         }            
      }

      char* eolPos = StringUtility::strchr( data + pos, '\n' );
      if ( eolPos != NULL ) {
         pos = eolPos - data + 1;
      } else {
         mc2log << fatal << "No eol at line " << MC2CITE( data + pos ) 
                << " at line " << line << endl;
         exit( 1 );
      }

   }

   // WF_INSTANCE property sets "default" serverlists
   MC2String instance = Properties::getProperty( "WF_INSTANCE", "" );
   if ( !instance.empty() ) {
      // Find and copy instance's server list to default
      // For all serverlists
      for ( server_map_t::const_iterator it = m_lists.begin() ; 
            it != m_lists.end() ; ++it )
      {
         if ( (*it).first.first == instance ) {
            mc2dbg4 << "Copying " << (*it).first.first << " " 
                   << (*it).first.second << " to default" << endl;
            m_lists[ make_pair( DefaultGroup, 
                                (*it).first.second ) ] = 
               new NamedServerList( *(*it).second );
         }
      }
   }

//   mc2log << "Named Server lists: \n" << *this;
//   mc2log.flush();

} // parseServerFile


bool 
NamedServerLists::addServerList( const MC2String& name, 
                                 const MC2String& type,
                                 const MC2String& serverlist )
{
   bool ret = ( 0 == getServerListsCRC( name, type ) );
   if ( ret ) {
      replaceServerList( name, type, serverlist );
   }
   return ret;
}


bool
NamedServerLists::replaceServerList( const MC2String& name, 
                                     const MC2String& type,
                                     const MC2String& serverlist )
{
   NamedServerList* l = checkAndCreateServersList( serverlist.c_str(),
                                                   type.c_str() );
   if ( l != NULL ) {
      m_lists[ make_pair( name, type ) ] = l;
      return true;
   } else {
      mc2log << error << "NamedServerLists::replaceServerList bad "
             << "serverList \"" << serverlist << "\"" << endl;
      return false;
   }
}


const char*
NamedServerLists::getServerList( const char* name, 
                                 const char* type ) const 
{
   const MC2String& ret = findServerList( name, type );
   if( ret.length() > 0 ){
      return ret.c_str();
   } else {
      return NULL;
   }
}


const NamedServerList* 
NamedServerLists::findNamedServerList( 
   const MC2String& name, const MC2String& type ) const
{
   typedef server_map_t::const_iterator const_iterator;
   // check for instance specific list first
   const_iterator result = m_lists.find( make_pair( name + ":" + 
                  Properties::getProperty("WF_INSTANCE", ""), type ) );
   if ( m_lists.end() != result ) {
      return result->second;
   } else {
      result = m_lists.find( make_pair( name, type ) );
      if ( m_lists.end() != result ) {
         return result->second;
      }
   }
   return NULL;
}


const MC2String&
NamedServerLists::findServerList( const MC2String& name, 
                                  const MC2String& type ) const 
{
   const NamedServerList* l = findNamedServerList( name, type );
   if ( l != NULL ) {
      return l->getServerListString();
   }
   return m_emptyString;
}


uint32 
NamedServerLists::getServerListsCRC( const MC2String& name,
                                     const MC2String& type ) const
{
   typedef server_map_t::const_iterator const_iterator;
   const_iterator result = m_lists.find( make_pair( name, type ) );
   if ( m_lists.end() != result ) {
      return result->second->getServerListCRC();
   }
   return 0;
}
