/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MaxCount.h"

#include <algorithm>

namespace TalkerUtility {

uint32 maxCount( uint32 start, uint32 end,
                 uint32 replyHits,
                 uint32 estimatedResultCount,
                 uint32 lastOffset, uint32 pageSize ) {

   uint32 maxHits = std::max( estimatedResultCount, replyHits );

   if ( start >= lastOffset ) {
      if ( estimatedResultCount < lastOffset ) {
         maxHits = estimatedResultCount;
      } else {
         maxHits = lastOffset;
      }
   } else {
      // inside the last page?
      if ( start >= lastOffset - pageSize ) {
         // end index within the last page?
         if ( end < lastOffset ) {
            maxHits = estimatedResultCount;
         } else {
            // end index is outside the last page, clamp
            // max results to last offset
            maxHits = lastOffset;
         }
      } else if ( replyHits < pageSize && end - start > replyHits ) {
         maxHits = replyHits;
      } else {
         maxHits = estimatedResultCount;
      }
   }

   return maxHits;
}

} // TalkerUtility

