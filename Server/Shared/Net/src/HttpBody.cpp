/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Includes
#include "HttpBody.h"
#include "MC2String.h"
#include "SharedBuffer.h"
#include "StringUtility.h"


HttpBody::HttpBody( const MC2String& Body ) {
   m_body = new char[Body.size() + 1];
   strcpy(m_body, Body.c_str());
   m_pos = Body.size();
   m_size = Body.size() + 1;
   m_allocSize = defaultAllocSize;
   m_binary = false;
   m_charSetMayBeChanged = true;
}


HttpBody::HttpBody() {
   m_allocSize = defaultAllocSize;
   m_body = new char[m_allocSize];
   m_body[0] = '\0';
   m_pos = 0;
   m_size = m_allocSize;
   m_binary = false;
   m_charSetMayBeChanged = true;
}


void
HttpBody::addString(const MC2String& Str) {
   uint32 size = Str.size();
   checkSize( size );
   strcpy(m_body + m_pos, Str.c_str());
   m_pos = m_pos + size;
}


void
HttpBody::addString(const char* Str) {
   uint32 size = strlen( Str );
   checkSize( size );
   strcpy(m_body + m_pos, Str);
   m_pos = m_pos + size;
}


void
HttpBody::addString(const char ch) {
   checkSize( 1 );
   m_body[m_pos] = ch;
   m_pos = m_pos + 1;
}


void
HttpBody::addAndSetCapital( const MC2String& str ) {
   MC2String upper = StringUtility::copyUpper(str);
   uint32 size = upper.size();
   checkSize( size );
   strcpy(m_body + m_pos, upper.c_str());
   m_pos = m_pos + size;
}


void
HttpBody::addAndSetCapital( const char* str ) {
   MC2String upper = StringUtility::copyUpper(MC2String(str));
   uint32 size = upper.size();
   checkSize( size );
   strcpy(m_body + m_pos, upper.c_str());
   m_pos = m_pos + size;
}


HttpBody::~HttpBody() {
   delete [] m_body;
}


void
HttpBody::clear() {
   if ( m_size != m_allocSize ) {
      delete [] m_body;
      m_body = new char[m_allocSize];
      m_size = m_allocSize;
   }
   m_body[0] = '\0';
   m_pos = 0;
   m_binary = false;
}


void
HttpBody::setBody(MC2String* newBody) {
   delete [] m_body;
   m_body = new char[newBody->size() + 1];
   strcpy(m_body, newBody->c_str());
   m_pos = newBody->size();
   m_size = newBody->size() + 1;
}


void 
HttpBody::setBody( const byte* newBody, uint32 length ) {
   delete [] m_body;
   m_body = new char[ length + 1 ]; // Safetybyte
   memcpy(m_body, newBody, length );
   m_pos = length;
   m_size = length + 1; 
   m_binary = true;
}

void
HttpBody::setBody( const SharedBuffer& buf )
{
   setBody( buf.getBufferAddress(), buf.getBufferSize() );
}

void 
HttpBody::setBinary( bool flag ) {
   m_binary = flag;
}

const char*
HttpBody::getCharSet() const
{
   return m_charSet.c_str();
}

void
HttpBody::setCharSet( const char* charSet, bool mayBeConverted )
{
   m_charSet = charSet;
   m_charSetMayBeChanged = mayBeConverted;   
}

bool
HttpBody::charSetMayBeChanged() const
{
   return m_charSetMayBeChanged;
}

const char*
HttpBody::getBody() {
   if ( !m_binary ) {
      m_body[m_pos] = '\0';
   }
   return m_body;
}


uint32
HttpBody::getBodyLength() const {
   return m_pos;
}


void
HttpBody::checkSize( uint32 size ) {
//   DEBUG8(cerr << "HttpBody::CheckSize: " << endl
//          << "size: " << size << endl
//          << "m_pos: " << m_pos << endl
//          << "m_size: " << m_size << endl
//          << "m_allocSize: " << m_allocSize << endl
//          ;);
   if ( (size + m_pos) >= m_size ) {
      uint32 newSize = m_size + MAX(m_allocSize, size + 1);
      char* newBody = new char[ newSize ];
      memcpy( newBody, m_body, m_pos );
      delete [] m_body;
      m_body = newBody;
      m_size = newSize;
   }
}


void 
HttpBody::setAllocationSize(uint32 newSize) {
   if ( newSize > 0 ) {
      m_allocSize = newSize;
   } else {
      m_allocSize = 1;
   }
}
