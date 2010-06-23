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

#include "UserFavorites.h"
#include "StringUtility.h"
#include "STLStringUtility.h"
#include "UserFavoritesPacket.h"
#include "MC2CRC32.h"
#include "DataBuffer.h"


namespace {
/** pushes different types of either strings or longs
 *
 *  Used to help sort out size of final buffer without
 *  adding objects more than once which reduces errors/bugs.
 */
class TypePusher {
public:
   TypePusher():m_totalSize( 0 ) { }

   /// add a new type
   template <typename T>
   void push( T type );

   const vector<uint32>& getLongs() const { 
      return m_longs;
   }

   const vector<const char*>& getStrings() const {
      return m_strings;
   }

   uint32 getTotalSize() const { return m_totalSize; }

private:
   std::vector<uint32> m_longs;
   std::vector<const char*> m_strings;
   uint32 m_totalSize;
};
// specialized for const char* string
template <>
void TypePusher::push<const char*>(const char* type ) {
   m_strings.push_back( type );
   m_totalSize += strlen( type ) + 1; // one extra for ending \0
}

// specialized for unsigned int
template <>
void TypePusher::push<uint32>(uint32 type ) {
   m_longs.push_back( type );
   m_totalSize += sizeof( uint32 );
}

// writing signed int as unsigned 
template <>
void TypePusher::push<int32>(int32 type ) {
   TypePusher::push<uint32>(type);
}

} // anonymous namespace

UserFavorite::UserFavorite(const uint32 favID) 
{
   m_selfAlloc = false;
   m_favID = favID;
   m_lat = MAX_INT32;
   m_lon = MAX_INT32;
   m_name = NULL;
   m_shortName = NULL;
   m_description = NULL;
   m_category = NULL;
   m_mapIconName = NULL;
}

UserFavorite::UserFavorite(const Packet* packet, int& pos, bool onlyID) 
{
   m_selfAlloc = false;
   m_favID = packet->incReadLong(pos);
   if (!onlyID) {
      m_lat = packet->incReadLong(pos);
      m_lon = packet->incReadLong(pos);
      packet->incReadString(pos, m_name);
      packet->incReadString(pos, m_shortName);
      packet->incReadString(pos, m_description);
      packet->incReadString(pos, m_category);
      packet->incReadString(pos, m_mapIconName);
      uint32 nbrInfos =  packet->incReadLong( pos );
      m_infos.resize( nbrInfos );
      for ( uint32 i = 0 ; i < nbrInfos ; ++i ) {
         m_infos[ i ].load( packet, pos );
      }
   } else {
      m_lat = MAX_INT32;
      m_lon = MAX_INT32;
      m_name = NULL;
      m_shortName = NULL;
      m_description = NULL;
      m_category = NULL;
      m_mapIconName = NULL;
   }
}


bool
UserFavorite::hasInfoType( ItemInfoEnums::InfoType type ) const {
   for ( InfoVect::const_iterator it = m_infos.begin() ; 
         it != m_infos.end() ; ++it ) 
   {
      if ( it->getInfoType() == type ) {
         return true;
      }
   }
   return false;
}


bool readEscStr( const char*& str, MC2String& val, char esc ) {
   bool ok = true;
   char l = '\0';
   char c = '\0';
   do {
      l = c;
      c = *str;
      if ( c == esc ) {
         // Ok test esc
         if ( l != '\\' ) {
            // Done
            c = '\0'; // End loop
         } else { // Escaped 
            val += c;
            c = ' ';
         }
      } else if ( c == '\\' ) {
         if ( l != '\\' ) {
            // Just one backslash, do nothing yet
         } else { // Double backslash, add one and stop escaping
            val += c;
            c = ' ';
         }
      } else if ( c == '\0' ) {
         // Done but error
         ok = false;
         mc2dbg1 << "makeInfoVect bad str not ended by " << esc << " got ("
                 << val << ") starting " << "at \"" << str << endl;
      } else {
         val += c;
      }
      
      str++;
   } while ( c != esc && c != '\0' );
   
   return ok;
}


void addEscStr( const char* str, MC2String& val ) {
   while ( *str != '\0' ) {
      if ( *str == '\\' ) {
         val.append( "\\\\" ); // Two backslashes
      } else if ( *str == ':' ) {
         val.append( "\\:" );
      } else if ( *str == ';' ) {
         val.append( "\\;" );
      } else {
         val += *str;
      }
      str++;
   }
}


bool
UserFavorite::makeInfoVect( const char* str, InfoVect& v ) {
   // [id:keyStr:valueStr;]*
   bool ok = true;
   const char* pos = str;
   uint32 len = strlen( str );
   char* tmpPtr = NULL;
   while ( uint32(pos - str) < len && ok ) {
      // Read element
      uint32 id = strtoul( pos, &tmpPtr, 0 );
      if ( tmpPtr != pos && *tmpPtr == ':' && 
           id < ItemInfoEnums::last_before_bool ) 
      {
         // ok
         pos = tmpPtr + 1;
         MC2String key;
         MC2String val;
         ok = readEscStr( pos, key, ':' );
         if ( ok ) {
            ok = readEscStr( pos, val, ';' );
         } else {
            mc2dbg << "makeInfoVect bad key at " << pos << endl;
         }
         if ( ok ) {
            v.push_back( ItemInfoEntry( key, val, 
                                        ItemInfoEnums::InfoType( id ) ) );
         } else {
            mc2dbg << "makeInfoVect bad val at " << pos << endl;
         }
      } else {
         mc2dbg << "makeInfoVect bad id (" << id << ") starting at \"" 
                << pos << endl;
      }
   } // End while more in string and ok

   return ok;
}


void
UserFavorite::makeInfoStr( const InfoVect& v, MC2String& str ) {
   for ( InfoVect::const_iterator it = v.begin() ; it != v.end() ; ++it ) {
      // id:keyStr:valueStr;
      STLStringUtility::uint2str( it->getInfoType(), str );
      str.append( ":" );
      // esc backslashes and colons in key and value strings
      addEscStr( it->getKey(), str );
      str.append( ":" );
      addEscStr( it->getVal(), str );
      str.append( ";" );
   }
}


void 
UserFavorite::init( const uint32 favID, 
                    const int32 lat, const int32 lon, 
                    const char* name,  const char* shortName,
                    const char* description, const char* category,
                    const char* mapIconName )
{
   m_selfAlloc = true;
   m_favID = favID;
   m_lat = lat; 
   m_lon = lon;
   m_name = StringUtility::newStrDup(name);
   m_shortName = StringUtility::newStrDup(shortName);
   m_description = StringUtility::newStrDup(description);
   m_category = StringUtility::newStrDup(category);
   m_mapIconName = StringUtility::newStrDup(mapIconName);
}


UserFavorite::UserFavorite( const uint32 favID, 
                            const int32 lat, const int32 lon, 
                            const char* name,  const char* shortName,
                            const char* description, const char* category,
                            const char* mapIconName,
                            const char* infoStr )
{
   init( favID, lat, lon, name, shortName, description, category, 
         mapIconName );
   // Parse infoStr and add to m_infos
   if ( !makeInfoVect( infoStr, m_infos ) ) {
      mc2log << fatal << "[UF:UF] Bad infoStr \"" << infoStr << "\"" 
             << endl;
      exit( 1 );
   }
}


UserFavorite::UserFavorite( const uint32 favID, 
                            const int32 lat, const int32 lon, 
                            const char* name,  const char* shortName,
                            const char* description, const char* category,
                            const char* mapIconName,
                            const InfoVect& v )
{
   init( favID, lat, lon, name, shortName, description, category, 
         mapIconName );
   m_infos = v;
}


UserFavorite::UserFavorite(UserFavorite* fav, bool onlyID)
{
   m_selfAlloc = true;
   m_favID = fav->m_favID;
   if (!onlyID) {
      m_lat = fav->m_lat; 
      m_lon = fav->m_lon;
      m_name = StringUtility::newStrDup(fav->m_name);
      m_shortName = StringUtility::newStrDup(fav->m_shortName);
      m_description = StringUtility::newStrDup(fav->m_description);
      m_category = StringUtility::newStrDup(fav->m_category);
      m_mapIconName = StringUtility::newStrDup(fav->m_mapIconName);
      m_infos = fav->m_infos;
   } else {
      m_lat = MAX_INT32;
      m_lon = MAX_INT32;
      m_name = NULL;
      m_shortName = NULL;
      m_description = NULL;
      m_category = NULL;
      m_mapIconName = NULL;
   }
}

void
UserFavorite::store(Packet* packet, int& pos, bool onlyID) const
{
   packet->incWriteLong(pos, m_favID);
   if (!onlyID) {
      packet->incWriteLong(   pos, m_lat );
      packet->incWriteLong(   pos, m_lon );
      packet->incWriteString( pos, m_name );
      packet->incWriteString( pos, m_shortName );
      packet->incWriteString( pos, m_description );
      packet->incWriteString( pos, m_category );
      packet->incWriteString( pos, m_mapIconName );
      packet->incWriteLong(   pos, m_infos.size() );
      for ( InfoVect::const_iterator it = m_infos.begin() ; 
            it != m_infos.end() ; ++it )
      {
         (*it).save( packet, pos );
      }
   }
}

UserFavorite::~UserFavorite()
{
   if (m_selfAlloc) {
      delete [] m_name;
      delete [] m_shortName;
      delete [] m_description;
      delete [] m_category;
      delete [] m_mapIconName;
   }
}

UserFavorite::UserFavorite() 
{
   m_favID = MAX_UINT32;
}


uint32 
UserFavorite::getSize() const {
   uint32 infoSize = 0;
   for ( InfoVect::const_iterator it = m_infos.begin() ; 
         it != m_infos.end() ; ++it )
   {
      infoSize += (*it).getSizeInPacket();
   }
   uint32 stringSize = 0;
   stringSize += (m_name == NULL ? 0 : strlen( m_name )) + 1;
   stringSize += (m_shortName == NULL ? 0 : strlen( m_shortName )) + 1;
   stringSize += (m_description == NULL ? 0 : strlen( m_description )) + 1;
   stringSize += (m_category == NULL ? 0 : strlen( m_category )) + 1;
   stringSize += (m_mapIconName == NULL ? 0 : strlen( m_mapIconName )) + 1;
   return (4 * 3 + 31) /* MAX size for members */ + stringSize + infoSize 
      + 3/*Padding*/ ;
}

uint32 
UserFavorite::getCRC() const {
   ::TypePusher pusher;
   pusher.push( getID() );
   pusher.push( getLat() );
   pusher.push( getLon() );
   pusher.push( getName() );
   pusher.push( getDescription() );
   pusher.push( getCategory() );
   pusher.push( getShortName() );
   pusher.push( getMapIconName() );
   
   UserFavorite::InfoVect::const_iterator it = getInfos().begin();
   UserFavorite::InfoVect::const_iterator itEnd = getInfos().end();
   for ( ; it != itEnd; ++it ) {
      pusher.push( (*it).getKey() );
      pusher.push( (*it).getVal() );
      pusher.push<uint32>( (*it).getInfoType() );
   }

   DataBuffer buff( pusher.getTotalSize() );
   buff.fillWithZeros();

   // write strings
   for ( uint32 i = 0, n = pusher.getStrings().size(); i < n; ++i ) {
      buff.writeNextString( pusher.getStrings()[ i ] );
   }

   // write longs
   for ( uint32 i = 0, n = pusher.getLongs().size(); i < n; ++i ) {
      buff.writeNextLong( pusher.getLongs()[ i ] );
   }

   return MC2CRC32::crc32( buff.getBufferAddress(), buff.getCurrentOffset() );
}

// ----------------------------------------------------------------------
// UserFavoritesList
// ----------------------------------------------------------------------


UserFavoritesList::~UserFavoritesList()
{
   // delete all favorites in the list
   iterator i;

   i = begin();
   while (i != end()) {
      delete (*i);
      erase(i++);
   }
}

void
UserFavoritesList::store(Packet* packet, int& pos, bool onlyID)
{
   iterator i;

   packet->incWriteLong(pos, size());
   for (i = begin(); i != end(); i++) {
      if (packet->getBufSize() - pos < (*i)->getSize() ) {
         mc2dbg8 << "UserFavoritesList::store(): packet too small (" << pos
                 << "), doubling size (2 * " << packet->getBufSize() << ")"
                 << endl;
         packet->setLength(pos);
         packet->resize(packet->getBufSize()*2);
      }
      mc2dbg8 << "UserFavoritesList::store(): storing favorite id: " 
              << (*i)->getID() << endl;
      (*i)->store(packet, pos, onlyID);
   }
}

void
UserFavoritesList::restore(const Packet* packet, int& pos, bool onlyID)
{
   uint32 sizeInPacket = packet->incReadLong(pos);
   
   for (uint32 i = 0; i < sizeInPacket; i++)
      push_back(new UserFavorite(packet, pos, onlyID));
}

void
UserFavoritesList::addFavorite(UserFavorite* userFav) 
{
   push_back(userFav);
}

void
UserFavoritesList::addFavorite(const uint32 favID) 
{
   push_back(new UserFavorite(favID));
}

void
UserFavoritesList::addFavorite( const uint32 favID, 
                                const int32 lat, const int32 lon, 
                                const char* name,  const char* shortName,
                                const char* description, 
                                const char* category,
                                const char* mapIconName,
                                const char* infoStr )
{
   push_back( 
      new UserFavorite( favID, lat, lon, name, shortName, description,
                        category, mapIconName, infoStr ) );
}

bool UserFavorites_compareIDs(UserFavorite* a, UserFavorite* b) {
   return a->getID() < b->getID();
}

void 
UserFavoritesList::sortByID()
{

   sort(UserFavorites_compareIDs);
}
