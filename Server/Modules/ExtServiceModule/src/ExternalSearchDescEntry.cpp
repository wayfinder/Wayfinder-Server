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

#include "ExternalSearchDescEntry.h"
#include "Packet.h"

ExternalSearchDescEntry::ExternalSearchDescEntry( int id,
                                                  const MC2String& name,
                                                  val_type_t type,
                                                  uint32 required )
      : m_id( id ),
        m_name( name ),
        m_type( type ),
        m_required( required )
{
}

void
ExternalSearchDescEntry::addChoiceParam ( int value,
                                          const MC2String& name )
{
   MC2_ASSERT( m_type == choice_type );
   m_choices.push_back( make_pair( value, name ) );
}

const
ExternalSearchDescEntry::choice_vect_t&
ExternalSearchDescEntry::getChoices() const
{
   return m_choices;
}

int
ExternalSearchDescEntry::save( Packet* packet, int& pos ) const
{
   int start_pos = pos;
   packet->incWriteLong( pos, m_id );
   packet->incWriteLong( pos, m_type );
   packet->incWriteLong( pos, m_required );
   packet->incWriteString( pos, m_name.c_str() );
   packet->incAlignWriteLong( pos );
   for( choice_vect_t::const_iterator it = m_choices.begin();
        it != m_choices.end();
        ++it ) {
      packet->incWriteLong( pos, it->first );
   }
   for( choice_vect_t::const_iterator it = m_choices.begin();
        it != m_choices.end();
        ++it ) {
      packet->incWriteString( pos, it->second.c_str() );
   }
   packet->incAlignWriteLong( pos );
   return pos - start_pos;
}

void
ExternalSearchDescEntry::print( ostream& o ) const
{
   o << m_name
     << ", id = " << m_id
     << ", type = " << m_type << ", required = " << m_required << endl;
   if ( m_type == choice_type ) {
      o << "  ";
      for( choice_vect_t::const_iterator it = m_choices.begin();
           it != m_choices.end();
           ++it ) {
         o << "(" << it->second << ", " << it->first << ")" << endl;
      }
      o << endl;
   }
}
