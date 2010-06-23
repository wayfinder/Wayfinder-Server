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
#include "VersionLockParser.h"
#include <regex.h>
#include "ScopedArray.h"
#include "StringUtility.h"

void 
VersionLockParser::VersionLockClientTypes::add( const MC2String& client_type )
{
   m_clientTypes.insert( client_type );
}


void 
VersionLockParser::VersionLockClientTypes::add( const VersionLockClientTypes& other )
{
   MC2_ASSERT( m_versionLock == other.m_versionLock );
   if( m_versionLock == other.m_versionLock ){
      m_clientTypes.insert( other.m_clientTypes.begin(), 
                            other.m_clientTypes.end() );
   }
}

bool 
VersionLockParser::VersionLockClientTypes::matches( const char* client_type ) const
{
   bool match_found = false;
   for(set<MC2String>::iterator it = m_clientTypes.begin();
             !match_found && it != m_clientTypes.end(); ++it){
      regex_t re;
      int flags = (REG_EXTENDED|REG_NOSUB|REG_NEWLINE);
      int res = regcomp( &re, it->c_str(), flags );
      if( res != 0 ){
         mc2log << error << "client_type regexp '" << *it 
                << "' failed to compile, error code " << res << endl;
         continue;
      }
      
      res = regexec( &re, client_type, 0, NULL, 0 );
      regfree( &re );
      
      match_found = (res == 0); //match found?
   }
   return match_found;
}

void VersionLockParser::VersionLockClientTypes::print(ostream& stream) const
{
   stream << "LOCK: " << m_versionLock;
   for(set<MC2String>::iterator it = m_clientTypes.begin(); 
       it != m_clientTypes.end(); ++it){
      stream << "\n- " << *it;
   }
   stream << endl;
}

void VersionLockParser::parseVersionLock( const char* data )
{
   int lock = 0;                            //the lock version
   int len = 0;                             //temp variable for parsing
   //tmp string large enough for parsing
   ScopedArray<char> tmp ( new char[strlen(data) + 1] );
   const char* str = data;                  //raw data iterator
   //read tag and version lock value
   if( 1 == sscanf( str, "VersionLock%*[ \t:]%i%n", &lock, &len ) ){
      str += len;                           //jump past read data.
      VersionLockClientTypes vlt( lock );   
      //read one clienttype expression at a time, ignoring leading whitespace
      while( 1 == sscanf(str, "%*[ \t,]%[^,\n]%n", tmp.get(), &len) ){
         str += len;                        //jump past read data
         StringUtility::trimEnd(tmp.get()); //trim whitespace from end of string
         vlt.add( tmp.get() );              //store client epression
      }
      //is thie versionlock value already in the container?
      VersionHolder::iterator it = m_versionLockClientTypes.find( lock );
      if( it != m_versionLockClientTypes.end() ){
         //Since set:::iterator is actually a const iterator we remove
         //the old object from the container and add its contents to
         //the new object that will be added later.
         VersionLockClientTypes old = *it;
         m_versionLockClientTypes.erase(it);
         vlt.add( old );
      }
      m_versionLockClientTypes.insert( vlt );
   }
}

uint32 VersionLockParser::findVersionLock( const char* client_type )
{
   typedef std::vector<const VersionLockClientTypes*> MatchCont;
   MatchCont matches;
   for(VersionHolder::iterator it = m_versionLockClientTypes.begin();
       it != m_versionLockClientTypes.end(); ++it){
      if( it->matches( client_type ) ){
         matches.push_back( &*it );
      }
   }
   if( matches.size() > 1 ){
      mc2log << fatal << "Client type '" << client_type 
             << "' version locked to " << matches.size() 
             << " different versions; ";
      for(MatchCont::iterator i = matches.begin(); i != matches.end(); ++i){
         mc2log << (*i)->VersionLock() << ", ";
      }
      mc2log << endl;
      exit( 1 );
   }
   return matches.empty() ? 0 : matches.front()->VersionLock();
}


ostream& VersionLockParser::operator<<(ostream& stream) const
{
   for(VersionHolder::iterator it = m_versionLockClientTypes.begin();
       it != m_versionLockClientTypes.end(); ++it){
      it->print(stream);
   }
   return stream;
}

      
