/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDSTRINGITEMVECTOR_H
#define EXPANDSTRINGITEMVECTOR_H

#include "config.h"
#include "ExpandStringItem.h"
#include <vector>

/**
 *    This vector should be used to get SMSes from a route consisting of
 *    a couple of ExpandStringItems. By calling the getRouteAsSMS-method
 *    some SMSes describing the route will be returned.
 *
 */
class ExpandStringItemVector { 
   public:

      /**
       *    Create a new vector for the ExpandStringItems.
       *    @param nbrItems      The initial size of the vector.
       */
      ExpandStringItemVector(uint32 nbrItems);

      /**
       *    Delete this vector.
       */
      virtual ~ExpandStringItemVector();
      
      /**
       *    Get the route that is stored in this vector (as ExpandStringItems)
       *    as SMSes.
       *
       *    Notice that the SMSes that are returned must be deleted by the
       *    caller!
       *    
       *    @param destVector             The SMSes will be inserted into
       *                                  this vector. <I><B>NB!</B> The
       *                                  elements that are inserted into
       *                                  this vector are allocated here but
       *                                  must be deleted by the caller!</I>
       *    @param totRouteLength         The length of the route (meters).
       *    @param totRouteTime           The estimated time to follow the 
       *                                  route.
       *    @param totRouteStandStillTime The estimated stand-still time when
       *                                  following the route.
       *    @param language               The language in the SMS. Must be an
       *                                  SMS-language (e.g. SMSISH_ENG or
       *                                  SMSISH_SWE).
       *    @param eol                    The type of end-of-line that should
       *                                  be used.
       *    @param maxLineLength          The maximum length of one row in the
       *                                  SMS.
       *    @param concatinatedSMS        True if the SMSes will be 
       *                                  concatinated in the phone, false 
       *                                  otherwise.
       *    @param preText                Optional parameter with user-defined
       *                                  text that should be written in front
       *                                  of the route-description.
       *    @param postText               Optional parameter with user-defined
       *                                  text that should be written after
       *                                  the route-description.
       *    @return  True will be returned if the destVector is filled,
       *             false otherwise.
       */
      bool getRouteAsSMS(vector<char*>& destVector, 
                         uint32 totRouteLength, 
                         uint32 totRouteTime, 
                         uint32 totRouteStandStillTime, 
                         StringTable::languageCode language,
                         UserConstants::EOLType eol, 
                         int maxLineLength,
                         bool concatinatedSMS,
                         const char* preText = NULL, 
                         const char* postText = NULL);

   void addLast( ExpandStringItem* item ) {
      m_items.push_back( item );
   }

   void deleteAllObjs();

private:
   std::vector< ExpandStringItem* > m_items;
};

#endif

