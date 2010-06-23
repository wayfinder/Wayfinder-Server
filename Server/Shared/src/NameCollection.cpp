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

#include "NameCollection.h"
#include "NameUtility.h"
#include "MC2String.h"
#include "Name.h"
#include "ScopedArray.h"
#include <sstream>

#include "Packet.h"
#include "DataBuffer.h"

Name NameCollection::m_missingName( "", LangTypes::english );

NameCollection::NameCollection(const NameCollection& other)
{
   reserve( other.size() );
   for( const_iterator it = other.begin();
        it != other.end();
        ++it ) {
      push_back( new Name(**it) );
   }
}

const NameCollection&
NameCollection::operator=( const NameCollection& other )
{
   if ( this != &other ) {
      // Delete the contents of vector
      for ( iterator it = begin(); it != end(); ++it ) {
         delete (*it);
      }
      // Fill it with new names.
      reserve( other.size() );
      for( const_iterator it = other.begin();
           it != other.end();
           ++it ) {
         push_back( new Name(**it) );
      } 
   }
   return *this;
}

NameCollection::~NameCollection()
{
   for ( iterator it = begin(); it != end(); ++it ) {
      delete (*it);
   }
}

set<const Name*> 
NameCollection::getNames( LangTypes::language_t lang, 
                          ItemTypes::name_t type ) const 
{
   set<const Name*> results;
   for ( const_iterator it = begin(); it != end(); ++it ) {
      if ( ( lang == (*it)->getLanguage() ) &&
           ( (type == ItemTypes::invalidName ) ||
             (type == (*it)->getType() ) ) ) {
         results.insert( *it );
      }
   }
   
   return results;
}

ostream& 
operator<< ( ostream& stream,
             const NameCollection& collection )
{
   stringstream strstr;
   for ( uint32 i = 0 ; i < collection.getSize() ; ++i ) {
      if ( i != 0 ) {
         // Add separator
         strstr << "; ";
      }
      strstr << *collection.getName( i );
   }
   strstr << ends;
   stream << strstr.str();
   return stream;
}


const Name* 
NameCollection::getBestName( LangTypes::language_t lang ) const
{
   // Simply uses NameUtility::getBestName
   
   // Create a uint32 array from the Name:s
   ScopedArray<uint32> nameArray( new uint32[ getSize() ] );
   
   for ( size_type i = 0; i < getSize(); ++i ) {
      const Name* name = getName( i );
      nameArray[ i ] = CREATE_NEW_NAME( name->getLanguage(), 
                                        name->getType(), 
                                        i ); 
   }
   
   int result = NameUtility::getBestName( getSize(), 
                                          nameArray.get(), 
                                          lang, 
                                          NULL, 
                                          false );
   if ( result >= 0 ) {
      return getName( result );
   }
   else {
      return &m_missingName;
   }
}

// --- DataBuffer and Packet functions.

int
NameCollection::getSizeInDataBuffer() const
{
   int totalSize = 0;
   
   totalSize += 4; // Size
   totalSize += 4; // Version
   totalSize += 4; // Nbr names

   for ( const_iterator it = begin();
         it != end();
         ++it ) {
      totalSize += 4; // Type 
      totalSize += 4; // Language
      totalSize += strlen((*it)->getName()) + 1; // Name + \0
   }
   AlignUtility::alignLong(totalSize);
   return totalSize;
}

bool
NameCollection::save(DataBuffer* buf) const
{
   uint32 beforeOffset = buf->getCurrentOffset();
   uint32 sizeInDataBuffer = getSizeInDataBuffer();
   
   buf->writeNextLong( sizeInDataBuffer ); // Total size
   
#ifdef MC2_UTF8
   buf->writeNextLong(1); // Version
#else
   buf->writeNextLong(0); // Version
#endif
   
   buf->writeNextLong( size() ); // Nbr names
   for ( const_iterator it = begin();
         it != end();
         ++it ) {
      buf->writeNextLong( (*it)->getLanguage() );
      buf->writeNextLong( (*it)->getType() );
   }

   // Write the strings separetely to save some space.
   for ( const_iterator it = begin();
         it != end();
         ++it ) {
         buf->writeNextString( (*it)->getName() );
   }
   // Align the buffer.
   buf->alignToLongAndClear();
   
   uint32 afterOffset = buf->getCurrentOffset();

   if ( (afterOffset - beforeOffset ) != sizeInDataBuffer ) {
      mc2log << fatal << "[NameColl]: offsetDiff = "
             << (afterOffset - beforeOffset ) << ", sizeindb = "
             << sizeInDataBuffer << endl;
      abort();
   }
   
   return true;
}

bool
NameCollection::load(DataBuffer* buf)
{
   int size = buf->readNextLong();
   int version = buf->readNextLong(); // Version.
   switch ( version ) {
      case 0:
      case 1: {
         int nbrItems = buf->readNextLong();
         vector<uint32> langs;
         langs.reserve(nbrItems);
         vector<uint32> types;
         types.reserve(nbrItems);

         // Read languages and types
         for ( int i = 0 ; i < nbrItems; ++i ) {
            langs.push_back(buf->readNextLong());
            types.push_back(buf->readNextLong());
         }

         // Read the names
         vector<MC2String> names;
         names.reserve(nbrItems);
         // Now read the names.
         if ( version == 0 ) {
            // Version 0 uses latin-1
            for ( int i = 0; i < nbrItems; ++i ) {            
               names.push_back( UTF8Util::isoToMc2( buf->readNextString() ) );
            }                        
         } else {
            // From version 1 we use utf8
            for ( int i = 0; i < nbrItems; ++i ) {            
               names.push_back( UTF8Util::utf8ToMc2( buf->readNextString() ) );
            }            
         }
         
         MC2_ASSERT( langs.size() == uint32(nbrItems) );
         MC2_ASSERT( types.size() == uint32(nbrItems) );
         MC2_ASSERT( names.size() == uint32(nbrItems) );

         // Create and add the names
         for ( int i = 0; i < nbrItems; ++i ) {
            Name* newName = new Name( names[i].c_str(),
                                      LangTypes::language_t(langs[i]),
                                      ItemTypes::name_t(types[i]));
            push_back(newName);
         }
         // Align the buffer
         buf->alignToLong();
         return true;
      }
      default:
         mc2log << error << "NameCollection::load - unknown version "
                << version << " skipping." << endl;
         buf->readPastBytes( size - 4 ); // Version already read
         return false;
   }
   return false;
}

int
NameCollection::getSizeInPacket() const
{
   int totalSize = 0;
   
   totalSize += 4; // Nbr names

   for ( const_iterator it = begin();
         it != end();
         ++it ) {
      totalSize += 4; // Type 
      totalSize += 4; // Language
      totalSize += strlen((*it)->getName()) + 1; // Name + \0
   }
   AlignUtility::alignLong(totalSize);
   return totalSize;
}

bool
NameCollection::save(Packet* p, int& pos) const
{
   uint32 beforeOffset = pos;
   uint32 sizeInPacket = getSizeInPacket();

   // Double the packet size if needed.
   p->updateSize( sizeInPacket, p->getBufSize() + sizeInPacket );

   // Version and stuff are not needed in packets, since
   // they won't be stored on disk.
   p->incWriteLong( pos, size() );           // Nbr names
   for ( const_iterator it = begin();
         it != end();
         ++it ) {
      p->incWriteLong( pos, (*it)->getLanguage() );
      p->incWriteLong( pos, (*it)->getType() );
   }

   // Write the strings separetely to save some alignment space.
   for ( const_iterator it = begin();
         it != end();
         ++it ) {
         p->incWriteString( pos, (*it)->getName() );
   }
   
   // Align the position.
   AlignUtility::alignLong(pos);
   
   uint32 afterOffset = pos;

   if ( (afterOffset - beforeOffset ) != sizeInPacket ) {
      mc2log << fatal << "[NameColl]: offsetDiff = "
             << (afterOffset - beforeOffset ) << ", size in pack = "
             << sizeInPacket << endl;
      abort();
   }
   
   return true;
}

bool
NameCollection::load(const Packet* p, int& pos)
{
   // Read the number of items.
   int nbrItems = p->incReadLong(pos);
   
   vector<uint32> langs;
   langs.reserve(nbrItems);
   vector<uint32> types;
   types.reserve(nbrItems);
   
   // Read languages and types
   for ( int i = 0 ; i < nbrItems; ++i ) {
      langs.push_back(p->incReadLong(pos) );
      types.push_back(p->incReadLong(pos) );      
   }
   
   // Read the names
   vector<MC2String> names;
   names.reserve(nbrItems);
   // Now read the names.
   for ( int i = 0; i < nbrItems; ++i ) {
      char* name = NULL;
      p->incReadString(pos, name);
      names.push_back(MC2String(name)); // Create string and add.
   }
   
   MC2_ASSERT( langs.size() == uint32(nbrItems) );
   MC2_ASSERT( types.size() == uint32(nbrItems) );
   MC2_ASSERT( names.size() == uint32(nbrItems) );
   
   // Create and add the names
   for ( int i = 0; i < nbrItems; ++i ) {
      Name* newName = new Name( names[i].c_str(),
                                LangTypes::language_t(langs[i]),
                                ItemTypes::name_t(types[i]));
      push_back(newName);
   }
   // Align the position
   AlignUtility::alignLong(pos);
   return true;
}




