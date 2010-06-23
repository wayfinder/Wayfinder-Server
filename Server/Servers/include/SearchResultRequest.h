/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHRESULTREQUEST_H
#define SEARCHRESULTREQUEST_H

#include "config.h"

#include <vector>

#include "Request.h"

class VanillaMatch;
class OverviewMatch;
class SearchRequestParameters;

/**
 *   Request that the various request that return search
 *   matches should inherit from.
 *   <br />
 *   @see ClosestSearchRequest
 *   @see SearchRequest
 *   @see ProximityRequest
 */
class SearchResultRequest : public RequestWithStatus {
public:

   /**
    *   SearchResultRequest
    */
   SearchResultRequest(const RequestData& parentOrID) :
      RequestWithStatus(parentOrID) {}
   
   /**
    *   Returns a vector containing the vanilla matches.
    *   Will be unusable when the request is deleted.
    *   If they are to be used after the request is deleted,
    *   then they must be cloned since they can point into
    *   data owned by the request.
    */
   virtual const vector<VanillaMatch*>& getMatches() const = 0;

   /**
    *   Returns the total number of matches that can be found.
    */
   virtual uint32 getTotalNbrMatches() const {
      return getMatches().size();
   }

   /**
    *   Returns the index in the vector of match number
    *   wantedIdx.
    */
   virtual int translateMatchIdx( int wantedIdx ) const {
      int size = getMatches().size();
      return ((wantedIdx) >= 0 ? MIN( wantedIdx, size ) :0);
   }
   

   /**
    *   Returns the SearchParameters used by the request.
    */
   virtual const SearchRequestParameters& getSearchParameters() const = 0;

   /**
    *   Returns the number of overview matches found.
    *   Default is zero.
    */
   virtual uint32 getNbrOverviewMatches() const { return 0; }

   /**
    *   Returns overview match at position i.
    *   Default implementation returns NULL.
    */
   virtual OverviewMatch* getOverviewMatch( uint32 i ) const { return NULL; }

};

#endif
