/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavStringTable.h"
#include "STLUtility.h"

/*------------------------------------------------------------*/

NavStringElement::NavStringElement(const char* str)
{
   if ( str != NULL ) {
      m_string = new char[strlen(str) + 1];      
      strcpy(m_string, str);
      m_length = strlen(str);
   } else {
      m_string = new char[1];
      m_string[0] = 0;
      m_length = 0;
   }
}

NavStringElement::~NavStringElement()
{
   delete [] m_string;
}

const char*
NavStringElement::getStringRef() const
{
   return m_string;
}

const int
NavStringElement::getStringLen() const
{
   return m_length;
}

/*------------------------------------------------------------*/

NavStringTable::NavStringTable()
{
   mc2dbg4 << "m_table.size = " << m_table.size() << endl;
   m_size = 0;
}

NavStringTable::~NavStringTable()
{
   STLUtility::deleteValues( m_table );
}

int
NavStringTable::addString(const char* str)
{
   // Create a new element which we can use when searching.
   NavStringElement* el = new NavStringElement(str); 
   for(uint32 i=0; i < m_table.size(); ++i) {
      if ( *el == *m_table[i] ) {
         delete el;
         return i;
      }
   }
   // Didn't find it.
   mc2dbg4 << "Adding \"" << str << "\" to table at pos "
           << m_table.size() << endl;
   m_table.push_back(el);
   m_size += el->getStringLen();
   return m_table.size() - 1;
}

const char*
NavStringTable::getStringAt(int idx) const
{
   return m_table[idx]->getStringRef();
}

int
NavStringTable::getSize() const
{
   return m_table.size();
}

int
NavStringTable::getCharSize() const
{
   return m_size;
}
