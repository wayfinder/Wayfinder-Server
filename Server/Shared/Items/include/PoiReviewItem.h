/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POIREVIEWITEM_H
#define POIREVIEWITEM_H

#include "config.h"
#include "MC2String.h"
#include "LangTypes.h"
#include <vector>


class Packet;


/**
 * Represents a review for a poi.
 *
 */
class PoiReviewDetail {
   public:
      /**
       * Constructor for empty PoiReviewDetail.
       */
      PoiReviewDetail()
         : m_ownerUIN( 0 ), m_grade( MAX_UINT32 ), m_lang( LangTypes::english )
         {}

      /**
       * Constructor.
       */
      PoiReviewDetail( uint32 ownerUIN,
                       uint32 grade,
                       const MC2String& review_id,
                       const MC2String& title, 
                       const MC2String& text,
                       LangTypes::language_t lang,
                       uint32 time = 0,
                       const MC2String& logonID = MC2String(),
                       const MC2String& firstname = MC2String(), 
                       const MC2String& lastname = MC2String() ) 
         : m_ownerUIN( ownerUIN ), m_grade( grade ), m_review_id( review_id ),
           m_title( title ), m_text( text ), m_lang( lang ), m_time( time ),
           m_logonID( logonID ), m_firstname( firstname ), 
           m_lastname( lastname )
         {}

      /**
       * Get the ownerUIN.
       */
      uint32 getOwnerUIN() const;

      /**
       * Get the grade of the poi for this review.
       */
      uint32 getGrade() const;

      /**
       * Get the review_id.
       */
      const MC2String& getReviewID() const;

      /**
       * Get the title.
       */
      const MC2String& getTitle() const;

      /**
       * Get the text.
       */
      const MC2String& getText() const;

      /**
       * Get the lang.
       */
      LangTypes::language_t getLang() const;

      /**
       * Get the time.
       */
      uint32 getTime() const;

      /**
       * Get the logonID.
       */
      const MC2String& getLogonID() const;

      /**
       * Get the firstname.
       */
      const MC2String& getFirstname() const;

      /**
       * Get the lastname.
       */
      const MC2String& getLastname() const;

      /**
       * Pack into a packet.
       */
      void save( Packet* p, int& pos ) const;

      /**
       * Read data from a packet.
       */
      void load( const Packet* p, int& pos );

      /**
       * Return the size when packed into a packet.
       */
      uint32 getSizeAsBytes() const;

      /**
       * Add as part of sql.
       */
      uint32 addSQLData( char* target ) const;

   private:
      /**
       * The owner's UIN.
       */
      uint32 m_ownerUIN;

      /**
       * The grade of the poi for this review.
       */
      uint32 m_grade;

      /**
       * Review id.
       */
      MC2String m_review_id;

      /**
       * The title.
       */
      MC2String m_title;

      /**
       * The text.
       */
      MC2String m_text;

      /**
       * The language.
       */
      LangTypes::language_t m_lang;

      /**
       * The UTC time of the review.
       */
      uint32 m_time;

      /**
       * logonID.
       */
      MC2String m_logonID;

      /**
       * firstname.
       */
      MC2String m_firstname;

      /**
       * lastname.
       */
      MC2String m_lastname;
};


/**
 * Represents a poi's reviews and grades.
 *
 */
class PoiReviewItem {
   public:
      /**
       * Constructor for empty PoiReviewItem.
       */
      PoiReviewItem()
         : m_totalGrades( 0 ), m_totalVotes( 0 )
         {}

      /**
       * Constructor.
       */
      PoiReviewItem( const MC2String& poiID,
                     uint32 totalGrades = 0,
                     uint32 totalVotes = 0 ) 
         : m_poiID( poiID ), m_totalGrades( totalGrades ), 
           m_totalVotes( totalVotes )
         {}

      /**
       * Get the poiID.
       */
      const MC2String& getPoiID() const;

      typedef vector< PoiReviewDetail > PoiReviewC;

      /**
       * Get the sum of grades.
       */
      uint32 getTotalGrades() const;

      /**
       * Get the number of grade votes.
       */
      uint32 getTotalVotes() const;

      /**
       * Get the reviews.
       */
      const PoiReviewC& getReviews() const;

      /**
       * Add a review
       */
      void addReview( PoiReviewDetail p );

      /**
       * Pack into a packet.
       */
      void save( Packet* p, int& pos ) const;

      /**
       * Read data from a packet.
       */
      void load( const Packet* p, int& pos );

      /**
       * Return the size when packed into a packet.
       */
      uint32 getSizeAsBytes() const;

      /**
       * Operator <, uses poiID.
       */
      bool operator < ( const PoiReviewItem& o ) const;

   private:
      /**
       * The poi ID.
       */
      MC2String m_poiID;

      /**
       * The total sum of grades.
       */
      uint32 m_totalGrades;

      /**
       * The total count of grade votes.
       */
      uint32 m_totalVotes;

      /**
       * The reviews.
       */
      PoiReviewC m_poiReviews;
};


// ========================================================================
//                                  Implementation of the inlined methods =

inline const MC2String&
PoiReviewItem::getPoiID() const {
   return m_poiID;
}

inline uint32
PoiReviewItem::getTotalGrades() const {
   return m_totalGrades;
}

inline uint32
PoiReviewItem::getTotalVotes() const {
   return m_totalVotes;
}

inline const PoiReviewItem::PoiReviewC&
PoiReviewItem::getReviews() const {
   return m_poiReviews;
}


inline void
PoiReviewItem::addReview( PoiReviewDetail p ) {
   m_poiReviews.push_back( p );
}

inline uint32
PoiReviewDetail::getOwnerUIN() const {
   return m_ownerUIN;
}

inline uint32
PoiReviewDetail::getGrade() const {
   return m_grade;
}

inline const MC2String&
PoiReviewDetail::getReviewID() const {
   return m_review_id;
}

inline const MC2String&
PoiReviewDetail::getTitle() const {
   return m_title;
}

inline const MC2String&
PoiReviewDetail::getText() const {
   return m_text;
}

inline LangTypes::language_t
PoiReviewDetail::getLang() const {
   return m_lang;
}

inline uint32
PoiReviewDetail::getTime() const {
   return m_time;
}

inline const MC2String&
PoiReviewDetail::getLogonID() const {
   return m_logonID;
}

inline const MC2String&
PoiReviewDetail::getFirstname() const {
   return m_firstname;
}

inline const MC2String&
PoiReviewDetail::getLastname() const {
   return m_lastname;
}

#endif // POIREVIEWITEM_H

