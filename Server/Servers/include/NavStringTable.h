/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVSTRINGTABLE_4711_H
#define NAVSTRINGTABLE_4711_H

#include "config.h"
 

/**
 *    
 */
class NavStringElement {

public:

   /**
    *   Creates a new NavStringElement.
    *   @param str The string to hold.
    */
   NavStringElement(const char* str);

   /**
    *   Destroys the NavStringElement.
    */
   virtual ~NavStringElement();

   /**
    *   Returns a pointer to the string.
    *   @return Pointer to the string.
    */
   const char* getStringRef() const;

   /**
    *   Returns the lenght of the string without '\0'.
    *   @return Lenght of the string.
    */
   const int getStringLen() const;

   /**
    *   @name  operators. 
    *   To make it possible to sort and search for the elements. 
    *   If the elements not should be sorted or searched
    *   for the functions doesn't need to be implemented.
    */
   //@{
   ///   equal
   bool virtual operator == (const NavStringElement& elm) const {
      return ( strcmp( m_string, elm.m_string ) == 0 );
   }
   
   ///   not equal
   bool virtual operator != (const NavStringElement& elm) const {
      return ( strcmp( m_string, elm.m_string ) != 0 );
   }
   
   ///   greater
   bool virtual operator > (const NavStringElement& elm) const {
      return ( strcmp( m_string, elm.m_string ) > 0 );
   }
   
   ///   less
   bool virtual operator < (const NavStringElement& elm) const {
      return ( strcmp( m_string, elm.m_string ) < 0 );
   }
   //@}

private:
   
   /** The string of this element */   
   char* m_string;

   /** The string length without '\0' for this element */
   int m_length;
   
};

class NavStringTable {

public:
   
   /**
    *   Creates a new NavStringTable.
    */
   NavStringTable();

   /**
    *   Deletes all strings and the table.
    */
   virtual ~NavStringTable();

   /**
    *   Adds a string to the table if it doesn't exist already.
    *   @return Index to the position where the string is in the
    *           table.
    */
   int addString(const char* str);

   /** 
    *   Returns the string at index idx.
    *   @param idx The index.
    *   @return A pointer to the string.
    */
   const char* getStringAt(int idx) const;

   /**
    *    Get the element at position pos. {\it {\bf NB!} No check is done 
    *    against writing outside the buffer!}
    *    @param   pos   The position of the element to return.
    */
   inline const char*  operator[](uint32 pos) const {
      return getStringAt(pos);
   }

   
   /**
    *   Returns the number of strings in the stringTable.
    *   @return The number of strings in the stringTable.
    */
   int getSize() const;

      /**
    *   Returns the numbder of chars in strintable without the '\0':s.
    *   @return The number of chars.
    */
   int getCharSize() const;
   
private:

   typedef std::vector<NavStringElement*> NaviStringVector;

   /**
    *   The string table itself.
    */
   NaviStringVector  m_table;

   /**
    *   The number of chars in stringtable (the '\0':s are not included)
    */
   int m_size;
   
};


#endif
