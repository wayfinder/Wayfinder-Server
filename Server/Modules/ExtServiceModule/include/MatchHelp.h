/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TALKERUTILITY_MATCHHELP_H
#define TALKERUTILITY_MATCHHELP_H

#include "ExternalSearchRequestData.h"

namespace TalkerUtility {

template <typename T>
size_t cutMatches( typename vector<T>::size_type begin,
                   typename vector<T>::size_type end,
                   vector<T>& array,
                   typename vector<T>::size_type pageSize ) {
   
   const typename vector<T>::size_type beginIdx = begin % pageSize;

   if ( beginIdx >= array.size() ) {
      array.clear();
   } else {
      // Does the range span more than one page?
      if ( ( begin / pageSize ) != ( end / pageSize ) ) {
         // erase everything in the beginning
         array.erase( array.begin(), array.begin() + beginIdx );
      } else {
         typename vector<T>::size_type endIdx = end % pageSize;
         if ( endIdx >= array.size() )  {
            // array is not large enough, remove in the beginning
            // and keep the end
            array.erase( array.begin(), array.begin() + beginIdx );
         } else {
            // keep the middle
            array = vector<T>( array.begin() + beginIdx, 
                               array.begin() + endIdx );
         }
      }
   }
   return beginIdx;
}

/** 
 * Cut out a section of matches we are interested in
 * @return start offset for matches.This is not the start in the vector.
 */
template <typename T>
int cutMatches( const ExternalSearchRequestData& searchData,
                vector<T>& matches,
                typename vector<T>::size_type pageHits ) {
   return cutMatches( searchData.getStartHitIdx(),
                      searchData.getEndHitIdx(),
                      matches,
                      pageHits );
}

}


#endif // TALKERUTILITY_MATCHHELP_H
