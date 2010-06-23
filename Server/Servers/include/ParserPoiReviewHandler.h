/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSERPOIREVIEWHANDLER_H
#define PARSERPOIREVIEWHANDLER_H

#include "config.h"
#include "ParserHandler.h"
#include "MC2String.h"
#include "PoiReviewEnums.h"
#include "LangTypes.h"
#include <vector>

class UserUser;
class ClientSetting;
class PoiReviewItem;


/**
 * Handles reviews of pois in ParserThread.
 *
 */
class ParserPoiReviewHandler : public ParserHandler {
   public:
      /**
       * Constructor.
       */
      ParserPoiReviewHandler( ParserThread* thread,
                              ParserThreadGroup* group );


      /**
       * Add poi review, or just grade one if review_id set but not title and
       * text. Modify a review by setting review_id and title and text.
       *
       * @param uin The user requesting this.
       * @param poiID The map's id of the poi, for this map release.
       * @param grade The grade of the poi.
       * @param lang The language of the review, only used if title and
       *             text is set.
       * @param review_id The id of a existing review to grade or change.
       * @param title Text with the title of the review.
       * @param text Text with the review text.
       * @return 0 if ok, -1 if error, -2 if timeout, -3 if no such review,
       *         -4 if not allowed.
       */
      int poiReview( uint32 uin,
                     const MC2String& poiID,
                     uint32 grade,
                     LangTypes::language_t lang,
                     MC2String& out_review_id,
                     const MC2String& review_id = MC2String(),
                     const MC2String& title = MC2String(), 
                     const MC2String& text = MC2String() );

      /**
       * Delete a review, should only be done by owner or admin.
       *
       * @param uin The user requesting this.
       * @param review_id The id of the review to delete.
       * @return 0 if ok, -1 if error, -2 if timeout, -3 if no such review,
       *         -4 if not allowed.
       */
      int deleteReview( uint32 uin, const MC2String& review_id );

      /**
       * List review from user, review_id or poiID. The first non empty/0 
       * one is used.
       *
       * @param review_id The review to list.
       * @param detail The level of details.
       * @param lang The language of the review?
       * @param review A new item on which load is called if ok else NULL.
       * @return 0 if ok, -1 if error, -2 if timeout, -3 if no review found.
       */
      int listReviews( uint32 ownerUIN,
                       const MC2String& review_id,
                       const MC2String& poiID,
                       PoiReviewEnums::reviewDetails_t detail, 
                       LangTypes::language_t lang,
                       vector<PoiReviewItem*>& reviews );

   private:
};


#endif // PARSERPOIREVIEWHANDLER_H

