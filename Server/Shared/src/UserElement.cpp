/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UserElement.h"


// === UserElement ========================================

UserElement::UserElement(UserConstants::UserItemType type)
      : VectorElement()
{
   m_type = type;
   m_removed = false;
   setOk( false );
}


UserElement::UserElement( const UserElement& el ) {
   m_removed = el.m_removed;
   m_ok = el.m_ok;
   m_type = el.m_type;
}


UserElement::~UserElement()
{
   // Nothing to do
}
   

UserConstants::UserItemType
UserElement::getType()
{
   return m_type;
}


void 
UserElement::packInto( Packet* p, int& pos ) {
   DEBUG1(mc2log << error << "UserElement::packInto Called! " 
          << "Don't do it again or I will ******* **** <beeep>." << endl;);
}


uint32 
UserElement::getSize() const
{
   return sizeof(bool) + sizeof(uint32);
}


void
UserElement::remove()
{
   m_removed = true;
}


bool
UserElement::removed() const
{
   return m_removed;
}


uint32
UserElement::sqlString( char* target, const char* str ) {
   int pos = 0;
   int to = 0;
   char ch;

   // Copy string   
   ch = str[pos];
   while ( ch != '\0' ) {
      if ( ch == '\'' ) {
         target[ to ] = '\\';
         to++;
      }
      target[ to ] = ch;
      to++;
      pos++;
      ch = str[pos];
   }
   target[ to ] = '\0';

   return to;
}

uint32
UserElement::sqlString( char* target, const MC2String& str ) {
   return sqlString( target, str.c_str() );
}


void
UserElement::makeSQLOnlyDate( uint32 time, char* dateStr) {
   struct tm *tm_struct;
   time_t rtime = time;
   struct tm result;
   tm_struct = gmtime_r( &rtime, &result );
   sprintf( dateStr, "'%.4d-%.2d-%.2d'" /*"%Y-%m-%d"*/, 
           tm_struct->tm_year + 1900, tm_struct->tm_mon + 1,
           tm_struct->tm_mday );
}
