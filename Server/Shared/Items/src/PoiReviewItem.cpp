/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PoiReviewItem.h"
#include "Packet.h"
#include "StringUtility.h"

void
PoiReviewItem::save( Packet* p, int& pos ) const {
   p->incWriteString( pos, m_poiID.c_str() );
   p->incWriteLong( pos, m_totalGrades );
   p->incWriteLong( pos, m_totalVotes );
   p->incWriteLong( pos, m_poiReviews.size() );
   for ( uint32  i = 0 ; i < m_poiReviews.size() ; ++i ) {
      m_poiReviews[ i ].save( p ,pos );
   }
}

void
PoiReviewItem::load( const Packet* p, int& pos ) {
   m_poiID = p->incReadString( pos );
   m_totalGrades = p->incReadLong( pos );
   m_totalVotes = p->incReadLong( pos );
   uint32 n = p->incReadLong( pos );
   m_poiReviews.resize( n );
   for ( uint32  i = 0 ; i < m_poiReviews.size() ; ++i ) {
      m_poiReviews[ i ].load( p ,pos );
   }
}

uint32
PoiReviewItem::getSizeAsBytes() const {
   uint32 size = 3*4 /*total*2, nbr reviews*/ + m_poiID.size() + 1 + 
      3/*possible padding*/;
   
   for ( uint32  i = 0 ; i < m_poiReviews.size() ; ++i ) {
      size += m_poiReviews[ i ].getSizeAsBytes();
   }
   return size;
}

bool
PoiReviewItem::operator < ( const PoiReviewItem& o ) const {
   return m_poiID < o.m_poiID;
}

void
PoiReviewDetail::save( Packet* p, int& pos ) const {
   p->incWriteLong( pos, m_ownerUIN );
   p->incWriteLong( pos, m_grade );
   p->incWriteLong( pos, m_lang );
   p->incWriteLong( pos, m_time );
   p->incWriteString( pos, m_review_id.c_str() );
   p->incWriteString( pos, m_title.c_str() );
   p->incWriteString( pos, m_text.c_str() );
   p->incWriteString( pos, m_logonID.c_str() );
   p->incWriteString( pos, m_firstname.c_str() );
   p->incWriteString( pos, m_lastname.c_str() );
}

void
PoiReviewDetail::load( const Packet* p, int& pos ) {
   m_ownerUIN = p->incReadLong( pos );
   m_grade = p->incReadLong( pos );
   m_lang = LangTypes::language_t( p->incReadLong( pos ) );
   m_time = p->incReadLong( pos );
   m_review_id = p->incReadString( pos );
   m_title = p->incReadString( pos );
   m_text = p->incReadString( pos );
   m_logonID = p->incReadString( pos );
   m_firstname = p->incReadString( pos );
   m_lastname = p->incReadString( pos );
}

uint32
PoiReviewDetail::getSizeAsBytes() const {
   uint32 size = 4*4/*uin,lang,grade,time*/ + m_review_id.size() + 1 
      + m_title.size() +1 + m_text.size() +1 
      + m_logonID.size() +1 + m_firstname.size() +1 + m_lastname.size() +1 
      + 3/*possible padding*/;

   return size;
}

uint32
PoiReviewDetail::addSQLData( char* target ) const {
   int pos = 0;
//    // ID
//    pos += sprintf( target + pos, "%d, ", ID ); // m_review_id 
   // UIN
   pos += sprintf( target + pos, "%u, ", m_ownerUIN );
   // Grade
   pos += sprintf( target + pos, "%d, ", m_grade );
   // Lang
   pos += sprintf( target + pos, "%d, ", uint32(m_lang) );
   // Title
   pos += sprintf( target + pos, "'%s', ", StringUtility::SQLEscapeSecure( 
                      m_title ).c_str() );
   // Text (no , as this is last)
   pos += sprintf( target + pos, "'%s' ", StringUtility::SQLEscapeSecure( 
                      m_text ).c_str() );

   return pos;
}

