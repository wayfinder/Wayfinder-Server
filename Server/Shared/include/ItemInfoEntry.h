/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMINFO_ENTRY_H
#define ITEMINFO_ENTRY_H

#include "config.h"

#include "ItemInfoEnums.h"
#include "MC2String.h"

class Packet;

/**
 *   One row of ItemInfo.
 *   This is how I think it works:
 *   ItemInfo consists of a key and a value.
 *   If the key is known  to be any of the types
 *   in ItemInfoEnums, it should be set so that
 *   clients can call phone numbers, surf to web
 *   pages etc.
 *   WARNING! (In the future). If read from a packet, ItemInfoEntry has
 *   strings that point into the packet. Use copy constructor
 *   to get rid of these.
 */
class ItemInfoEntry {
public:

   /**
    *   Creates empty undefined ItemInfoEntry.
    */
   inline ItemInfoEntry();
   
   /**
    *   Constructs one ItemInfoEntry.
    *   Strings are copied, so it is safe to use.
    *   @param key      The key.
    *   @param val      The value.
    *   @param infotype The type of info that the key represents.
    */
   inline ItemInfoEntry( const MC2String& key,
                         const MC2String& val,
                         ItemInfoEnums::InfoType infoType );

   /**
    *   Returns the key,
    */
   inline const char* getKey() const;
   
   /**
    *   Returns the value.
    */
   inline const char* getVal() const;

   /**
    *   Returns the type.
    */
   inline ItemInfoEnums::InfoType getInfoType() const;

   /**
    *   Returns the size of this ItemInfoEntry in a Packet.
    */
   int getSizeInPacket() const;

   /**
    *   Saves the info in a packet.
    */
   int save( Packet* p, int& pos ) const;

   /**
    *   Loads the info from a packet.
    */
   int load( const Packet* p, int& pos );

   bool operator == ( const ItemInfoEntry& other ) const {
      return
         m_key == other.m_key &&
         m_val == other.m_val &&
         m_infoType == other.m_infoType;
   }

   bool operator != ( const ItemInfoEntry& other ) const {
      return ! ( *this == other );
   }

private:
   MC2String m_key;
   MC2String m_val;
   ItemInfoEnums::InfoType m_infoType;
   
};

inline
ItemInfoEntry::ItemInfoEntry( const MC2String& key,
                              const MC2String& val,
                              ItemInfoEnums::InfoType infoType ) :
      m_key( key ),
      m_val( val ),
      m_infoType( infoType )
{
}

inline
ItemInfoEntry::ItemInfoEntry()
{
}

inline
const char*
ItemInfoEntry::getKey() const
{
   return m_key.c_str();
}

inline
const char*
ItemInfoEntry::getVal() const
{
   return m_val.c_str();
}

inline
ItemInfoEnums::InfoType
ItemInfoEntry::getInfoType() const
{
   return m_infoType;
}

#endif
