/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCH_SORTING_H
#define SEARCH_SORTING_H

#include "config.h"

#include <vector>

#include "SearchTypes.h"

class MC2Coordinate;

class MatchLink;

class SearchSorting {
public:

   /**
    *   Sorts a vector of SearchMatches. If it does not link - add more
    *   types to the dummy function in the cpp-file.
    *   @param matches Vector of matches to sort.
    *   @param sorting The type of sorting to use.
    *   @param nbrSorted The number of sorted hits when sorting
    *                    bestmatches.
    *   @return The number of matches that were sorted.
    */
   template<class SEARCHMATCH>
   static int sortSearchMatches(vector<SEARCHMATCH*>& matches,
                                SearchTypes::SearchSorting sorting,
                                int nbrSorted = -1);

   /**
    *   Returns true if a is less than b in the current sorting.
    *   Should be removed and replaced by a function that can merge
    *   two vectors of Matches instead.
    *   @param a One MatchLink.
    *   @param b The other MatchLink.
    *   @param sorting Current sorting.
    */
   static bool isLessThan( const MatchLink* a,
                           const MatchLink* b,
                           SearchTypes::SearchSorting sorting);

   /**
    *    Merges two vectors of sorted matches.
    *    Target will be sorted and sources empty.
    *    @param target Matches are put here. Will contain all matches.
    *    @param source Matches are taken from here. Will be empty.
    *    @param sorting Sorting to use.
    */
   template<class SEARCHMATCH>
      static void mergeSortedMatches(vector<SEARCHMATCH*>& target,
                                     vector<SEARCHMATCH*>& source,
                                     SearchTypes::SearchSorting sorting,
                                     int nbrSorted = -1);
   
   /**
    * Sort matches by distance.
    */
   template < typename SEARCHMATCH >
      static void sortMatches( const MC2Coordinate& coord,
                               vector< SEARCHMATCH* >& matches );

private:
   /**
    *   Sorts a VECTOR of SearchMatches* or SearchableLinks*.
    */
   template<class VECTOR, class SEARCHMATCH>
   static int sortMatches(VECTOR& matches,
                          SEARCHMATCH* searchMatch,
                          SearchTypes::SearchSorting sorting,
                          int nbrSorted);

};

#endif
