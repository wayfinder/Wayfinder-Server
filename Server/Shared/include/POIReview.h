/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POIREVIEW_H
#define POIREVIEW_H

#include "config.h"
#include "MC2String.h"

/**
 * Contains information about a review.
 */
class POIReview {
public:
   
   /**
    * Creates a POIReview
    *
    * @param rating The rating given by the reviewer
    * @param reviewer The user that wrote the review
    * @param date The date of the review
    * @param reviewText The review
    */
   POIReview( int8 rating, 
              MC2String reviewer, 
              MC2String date, 
              MC2String reviewText );

   int8 getRating() const;

   const MC2String& getReviewer() const;

   const MC2String& getDate() const;

   const MC2String& getReviewText() const;

private:
   // The rating
   int8 m_rating;
   
   // The reviewer
   MC2String m_reviewer; 

   // The date of the review
   MC2String m_date;

   // The review text
   MC2String m_reviewText;
};

#endif
