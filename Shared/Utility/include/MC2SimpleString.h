/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2SIMPLESTRING
#define MC2SIMPLESTRING

#include <string.h>

#ifdef __SYMBIAN32__
   #include "mcostream.h"
#else
   #include <iostream>
   using namespace std;
#endif

#include <algorithm>
   
/**
 *    Helper class to use when sending a lot of strings
 *    around and avoidance of ownership problems are to
 *    be avoided without having to use the more advanced
 *    functionality supplied by MC2String.
 */
class MC2SimpleString {
public:
   /**
    *   Creates a copy of the supplied string and stores it
    *   inside.
    */
   inline MC2SimpleString(const char* str = "");

   /**
    *   Creates a copy of the supplied MC2SimpleString.
    */
   inline MC2SimpleString(const MC2SimpleString& other);

   /**
    *   Destructor. Deletes the internal char-array.
    */
   inline ~MC2SimpleString();
   
   /**
    *   Returns a pointer to the string owned by this object.
    */
   inline const char* c_str() const;

   /**
    *   Assignment operator.
    */
   inline MC2SimpleString& operator=(const MC2SimpleString& other);
   
   /**
    *   Assignment operator.
    */
   inline MC2SimpleString& operator=(const char* str);   
   
   /**
    *   Comparison function which can be used when the string
    *   is put into e.g. maps.
    */
   inline bool operator<(const MC2SimpleString& other) const;
   inline bool operator<(const char* str) const;

   /**
    *   Returns true if the two strings are lexically the same.
    */
   inline bool operator==(const MC2SimpleString& other) const;

   /**
    *   Returns true if the two strings are lexically the same.
    */
   inline bool operator==(const char* other ) const;

   /**
    *   Returns false if the two strings are lexically the same.
    */
   inline bool operator!=(const MC2SimpleString& other) const;

   /**
    *   Returns false if the two strings are lexically the same.
    */
   inline bool operator!=(const char* other) const;

   /**
    *   Returns the character at position i.
    */
   inline char operator[] (int i) const;
   
   /**
    *   Returns the length of the string.
    */
   unsigned int length() const;
   
   /**
    *   Returns true if the string is empty.
    */
   bool empty() const;
   
   /**
    *   Prints the string on the stream.
    */
   inline friend ostream& operator<<(ostream& s, const MC2SimpleString& str);

   /**
    *   Swaps the two strings.
    */
   inline void swap( MC2SimpleString& other );
   
   /**
    *   Class to use for comparisons
    */
   class StrCmp {
   public:
      inline bool operator()(const MC2SimpleString& a,
                             const MC2SimpleString& b ) const;
      inline bool operator()(const MC2SimpleString* a,
                             const MC2SimpleString* b) const;
   };
   
private:
   /**
    *   Initializes the MC2SimpleString.
    */
   void init(const char* str);

   /**
    *   Initializes the MC2SimpleString.
    */
   void init(const MC2SimpleString& other);

protected:

   /**
    *   Does nothing. Used by MC2SimpleStringNoCopy.
    */
   MC2SimpleString(int, int, int) {}
   
   /**
    *   The string itself.
    */
   char* m_str;

   /**
    *   The length of the string.
    */
   unsigned int m_len;
};

/// Swaps the two strings.
namespace std {
   inline static void swap( MC2SimpleString& str1, MC2SimpleString& str2 );
   inline static void iter_swap( MC2SimpleString* str1, MC2SimpleString* str2 );
};

/**
 *   Same as MC2SimpleString but does not copy the string sent in.
 *   Will delete it though.
 */
class MC2SimpleStringNoCopy : public MC2SimpleString {
public:
   /**
    *   Creates a new MC2SimpleStringNoCopy.
    *   @param str String that will be owned by the new string from now on.
    *   @param len The length of the string. If not supplied it will be
    *              calculated using strlen.
    */
   MC2SimpleStringNoCopy( char* str, int len = -1 )
      : MC2SimpleString( 0,0,0 ) {
      
      m_str = str;      
      if ( len < 0 ) {
         m_len = strlen(str);
      } else {
         m_len = len;
      }
   }
};

inline
MC2SimpleString::MC2SimpleString(const char* str)
{
   init(str);
}

inline
MC2SimpleString::MC2SimpleString(const MC2SimpleString& other)
{
   init(other);
}

inline MC2SimpleString&
MC2SimpleString::operator=(const char* str)
{
   delete [] m_str;
   init(str);
   return *this;
}

inline MC2SimpleString&
MC2SimpleString::operator=(const MC2SimpleString& other)
{
   if ( &other != this ) {
      delete [] m_str;
      init(other);
   }
   return *this;
}

inline
MC2SimpleString::~MC2SimpleString()
{
   delete [] m_str;
}

inline const char*
MC2SimpleString::c_str() const
{
   return m_str;
}

inline unsigned int
MC2SimpleString::length() const
{
   return m_len;
}

inline bool 
MC2SimpleString::empty() const
{
   return length() == 0; 
}

inline bool
MC2SimpleString::operator<(const MC2SimpleString& other) const
{
   return strcmp(m_str, other.m_str) < 0;
}

inline bool
MC2SimpleString::operator<(const char* str) const
{
   return strcmp(m_str, str) < 0;
}

inline bool
MC2SimpleString::operator==(const char* other ) const
{
   return strcmp(m_str, other) == 0;
}

inline bool
MC2SimpleString::operator==(const MC2SimpleString& other) const
{   
   return strcmp(m_str, other.m_str) == 0;
}

inline bool
MC2SimpleString::operator!=(const MC2SimpleString& other) const
{
   return !( (*this) == other);
}

inline bool
MC2SimpleString::operator!=(const char* other) const
{
   return !( (*this) == other);
}

inline char
MC2SimpleString::operator[] (int i) const
{
   return m_str[i];
}

inline
ostream& operator<<(ostream& s, const MC2SimpleString& str)
{   
   return s << str.c_str();
}
   
inline void 
MC2SimpleString::swap( MC2SimpleString& other )
{
   std::swap( m_str, other.m_str );
   std::swap( m_len, other.m_len );
}

inline bool
MC2SimpleString::StrCmp::operator()(const MC2SimpleString& a,
                                    const MC2SimpleString& b ) const
{
   return a < b;
}
inline bool
MC2SimpleString::StrCmp::operator()(const MC2SimpleString* a,
                                    const MC2SimpleString* b) const
{
   return *a < *b;
}
   
inline void 
std::swap( MC2SimpleString& str1, MC2SimpleString& str2 )
{
   str1.swap( str2 );
}

inline void
std::iter_swap( MC2SimpleString* str1, MC2SimpleString* str2 )
{
   str1->swap( *str2 );
}


#endif
