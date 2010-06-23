/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STRING_CACHE_ELEMENT_H
#define STRING_CACHE_ELEMENT_H

#include "CacheElement.h"
#include "NotCopyable.h"

class TCPSocket;

/**
 * StringTableCacheElement.
 *
 */
class StringTableCacheElement : public CacheElement, private NotCopyable
{
public:
   StringTableCacheElement( uint32 mapID,
                            CACHE_TYPE cache_type );

   StringTableCacheElement(uint32 mapID, uint32 mapIP, 
                           uint16 mapPort, CACHE_TYPE cache_type);
   
   StringTableCacheElement(uint32 mapID, char* hostName, 
                           uint16 hostPort, CACHE_TYPE cache_type);

   StringTableCacheElement( uint32 mapID,
                           byte* data,
                           int length,
                           int nbrStrings,
                           CACHE_TYPE cache_type);

   virtual ~StringTableCacheElement();

   inline uint32 getSize() const;
   inline byte* getStringTableData() const;
   inline char** getStringVector() const;
   inline uint32 getNbrStrings() const;
   inline void addLast( char* newString );

private:
   void readStringTable( uint32 mapIP, uint16 mapPort );
   void readStringTable( char* hostName, uint16 hostPort );
   void readStringTable( byte* data, int length );
   void readStringTable( TCPSocket& sock );

   char** m_strVector;
   uint32 m_nbrStrings;
   byte*  m_stringData;
   uint32 m_stringSize;
};

/*
 * Inlines.
 */

inline uint32 StringTableCacheElement::getSize() const
{
   return m_stringSize;
}

inline byte* StringTableCacheElement::getStringTableData() const
{
   return m_stringData;
}

inline char** StringTableCacheElement::getStringVector() const
{
   return m_strVector;
}

inline uint32 StringTableCacheElement::getNbrStrings() const
{
   return m_nbrStrings;
}  

#endif
