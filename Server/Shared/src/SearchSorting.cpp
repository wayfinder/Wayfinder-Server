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


#include "SearchSorting.h"
#include "StringUtility.h"
#include "SearchReplyPacket.h"
#include "GfxUtility.h"

#include <algorithm>

using namespace std;

/**
 *    Functor used for alphabetical sorting.
 *    <br />
 *    FIXME: Make these functors private and use vectors instead.
 */
template <class SEARCHMATCH> class AlphaComparator {
public:
   bool operator()(const SEARCHMATCH* a, const SEARCHMATCH* b) const {
      int res = StringUtility::strcasecmp(a->getAlphaSortingName(),
                                          b->getAlphaSortingName());
      if ( res == 0 ) {
         res = StringUtility::strcasecmp(a->getLocationName(),
                                         b->getLocationName());
      }
      return res < 0;      
   }
   
};

/**
 *   Functor used for sorting hits according to confidence.
 *   If the points are the same a DistanceComparator will
 *   be used.
 */
template <class SEARCHMATCH> class ConfidenceComparator {
public:
   bool operator()(const SEARCHMATCH* a, const SEARCHMATCH* b) const {
      if ( a->getPoints() == b->getPoints() ) {
         return m_extraComp(a,b);
      } else {
         return a->getPoints() > b->getPoints();
      }
   }
   
private:
   /** Comparator for alpha */
   AlphaComparator<SEARCHMATCH> m_extraComp;
   
};

/**
 *   Functor used for sorting hits according to distance.
 *   If the distances are the same the alphabetical order will
 *   be used.
 */
template <class SEARCHMATCH> class DistanceComparator {
public:
   bool operator()(const SEARCHMATCH* a, const SEARCHMATCH* b) const {
      if ( a->getDistance() == b->getDistance() ) {
         return m_extraComparator(a,b);
      } else {
         return  a->getDistance() < b->getDistance();
      }
   }
private:
   /// Comparator for confidence.
   ConfidenceComparator<SEARCHMATCH> m_extraComparator;
};


// ------------------- Class SearchSorting ------------------

template<class VECTOR, class SEARCHMATCH>
int
SearchSorting::sortMatches(VECTOR& matches,
                           SEARCHMATCH* searchMatch,
                           SearchTypes::SearchSorting sorting,
                           int nbrSorted)
{
   uint32 startTime = TimeUtility::getCurrentTime();
   int size = matches.size();
   if ( nbrSorted < 0 ) {
      nbrSorted = MaxSortingLimit;
   }
   mc2dbg8 << "[SearchSorting]: Sorting using "
           << SearchTypes::getSortingString(sorting)
           << " nbrSorted = " << nbrSorted
           << ", size = " << size
           << endl;

   // In the case of BestMatchesSort and BestDistanceSort
   // nbrSorted should probably be set and otherwise ignored.   
   switch ( sorting ) {
      case SearchTypes::DistanceSort:
      case SearchTypes::BestDistanceSort:
         if ( size <= nbrSorted ) {
            stable_sort(matches.begin(), matches.end(),
                        DistanceComparator<SEARCHMATCH>());
         } else {            
            partial_sort( matches.begin(),
                          matches.begin() + nbrSorted,
                          matches.end(),
                          DistanceComparator<SEARCHMATCH>() );
         }
         break;
      case SearchTypes::BestMatchesSort:
      case SearchTypes::ConfidenceSort:
         // Update the distance
         for( typename VECTOR::iterator it = matches.begin();
              it != matches.end();
              ++it ) {
            (*it)->getPointInfo().setDistanceMeters((*it)->getDistance());
         }
         if ( size <= nbrSorted ) {
            sort( matches.begin(), matches.end(),
                  ConfidenceComparator<SEARCHMATCH>() );
         } else {
            partial_sort( matches.begin(),
                          matches.begin() + nbrSorted,
                          matches.end(),
                          ConfidenceComparator<SEARCHMATCH>() );
         }
         break;
      case SearchTypes::AlphaSort:
         if ( size <= nbrSorted ) {
            sort ( matches.begin(), matches.end(),
                   AlphaComparator<SEARCHMATCH>());
         } else {
            partial_sort( matches.begin(),
                          matches.begin() + nbrSorted,
                          matches.end(),
                          AlphaComparator<SEARCHMATCH>() );
         }
         break;
      default:
         mc2log << warn << "[SearchSorting]: Sorting type "
                << SearchTypes::getSortingString(sorting)
                << " not implemented " << endl;
         
   };
   uint32 stopTime = TimeUtility::getCurrentTime();
   mc2dbg8 << "[SearchSort]: Sorted " << MIN(size, nbrSorted)
           << " elements in " << (stopTime-startTime) << " ms"
           << endl;
   if ( size <= nbrSorted ) {
      return size;
   } else {
      return nbrSorted;
   }
}

template<class SEARCHMATCH>
void
SearchSorting::mergeSortedMatches(vector<SEARCHMATCH*>& target,
                                  vector<SEARCHMATCH*>& source,
                                  SearchTypes::SearchSorting sorting,
                                  int nbrSorted)
{
   
#if 0
   // Optimize this later.
   target.insert( target.end(), source.begin(), source.end());
   source.clear();   
   sortMatches(target, (SEARCHMATCH*)NULL, sorting, nbrSorted);
#else
   // How about now?
   vector<SEARCHMATCH*> new_target( source.size() + target.size() );
   // Urgg. This is probably why I haven't done this until now
   
   switch ( sorting ) {
      case SearchTypes::DistanceSort:
      case SearchTypes::BestDistanceSort:
         // Update the distance
         // Should be no need to update the distance in
         // the target since it should be the result of
         // a previous merge where  the source has been
         // updated already.
         for( typename vector<SEARCHMATCH*>::iterator it = source.begin();
              it != source.end();
              ++it ) {
            (*it)->getPointInfo().setDistanceMeters((*it)->getDistance());
         }
         merge( source.begin(), source.end(),
                target.begin(), target.end(),
                new_target.begin(), DistanceComparator<SEARCHMATCH>() );
         break;
      case SearchTypes::BestMatchesSort:
      case SearchTypes::ConfidenceSort:
         merge( source.begin(), source.end(),
                target.begin(), target.end(),
                new_target.begin(), ConfidenceComparator<SEARCHMATCH>() );
         break;
      case SearchTypes::AlphaSort:
      default:
         merge( source.begin(), source.end(),
                target.begin(), target.end(),
                new_target.begin(), AlphaComparator<SEARCHMATCH>() );
         break;
   }

   // Put the new target in target and clear the source.
   new_target.swap( target );
   source.clear();
#endif
}

// ugly workaround for linker problems in el5 that are ok in el4 and el6 beta.
#if ARCH_OS_LINUX_RH_REL == 5 
void
SearchSorting::mergeSortedMatches(vector<VanillaMatch*>& target,
                                  vector<VanillaMatch*>& source,
                                  SearchTypes::SearchSorting sorting,
                                  int nbrSorted)
{
   
#if 0
   // Optimize this later.
   target.insert( target.end(), source.begin(), source.end());
   source.clear();   
   sortMatches(target, (SEARCHMATCH*)NULL, sorting, nbrSorted);
#else
   // How about now?
   vector<VanillaMatch*> new_target( source.size() + target.size() );
   // Urgg. This is probably why I haven't done this until now
   
   switch ( sorting ) {
      case SearchTypes::DistanceSort:
      case SearchTypes::BestDistanceSort:
         // Update the distance
         // Should be no need to update the distance in
         // the target since it should be the result of
         // a previous merge where  the source has been
         // updated already.
         for( vector<VanillaMatch*>::iterator it = source.begin();
              it != source.end();
              ++it ) {
            (*it)->getPointInfo().setDistanceMeters((*it)->getDistance());
         }
         merge( source.begin(), source.end(),
                target.begin(), target.end(),
                new_target.begin(), DistanceComparator<VanillaMatch>() );
         break;
      case SearchTypes::BestMatchesSort:
      case SearchTypes::ConfidenceSort:
         merge( source.begin(), source.end(),
                target.begin(), target.end(),
                new_target.begin(), ConfidenceComparator<VanillaMatch>() );
         break;
      case SearchTypes::AlphaSort:
      default:
         merge( source.begin(), source.end(),
                target.begin(), target.end(),
                new_target.begin(), AlphaComparator<VanillaMatch>() );
         break;
   }

   // Put the new target in target and clear the source.
   new_target.swap( target );
   source.clear();
#endif
}
#endif

/// Dummy function for linking. Add more types here if needed.
namespace {
   void dummy1()
   {
      SearchTypes::SearchSorting sorting = SearchTypes::AlphaSort;
      vector<OverviewMatch*> vect1;
      vector<OverviewMatch*> vect3;
      SearchSorting::mergeSortedMatches(vect1, vect3, sorting);
      SearchSorting::sortSearchMatches(vect1, sorting, 10);
      vector<VanillaMatch*> vect2;
      vector<VanillaMatch*> vect4;
      SearchSorting::mergeSortedMatches(vect2, vect4, sorting);
      SearchSorting::sortSearchMatches(vect2, sorting, 10);
   }
}

template<class SEARCHMATCH>
int
SearchSorting::sortSearchMatches(vector<SEARCHMATCH*>& matches,
                                 SearchTypes::SearchSorting sorting,
                                 int nbrSorted)
{
   // Never call the dummy function. Just make it link.
   if ( false ) {
      dummy1();
   }

   // Real function is here.
   return sortMatches(matches, (SEARCHMATCH*)NULL, sorting, nbrSorted);
}

// ugly workaround for linker problems in el5 that are ok in el4 and el6 beta.
#if ARCH_OS_LINUX_RH_REL == 5 
int
SearchSorting::sortSearchMatches(vector<OverviewMatch*>& matches,
                                 SearchTypes::SearchSorting sorting,
                                 int nbrSorted)
{
   // Real function is here.
   return sortMatches(matches, (OverviewMatch*)NULL, sorting, nbrSorted);
}
#endif

bool
SearchSorting::isLessThan( const MatchLink* a,
                           const MatchLink* b,
                           SearchTypes::SearchSorting sorting)
{
   switch ( sorting ) {
      case SearchTypes::BestMatchesSort:
      case SearchTypes::ConfidenceSort:
         return
            ConfidenceComparator<SearchMatch>()(a->getMatch(), b->getMatch());
      case SearchTypes::AlphaSort:
         return
            AlphaComparator<SearchMatch>()(a->getMatch(), b->getMatch());
      default:
         return false;
   }
}


template <>
void SearchSorting::
sortMatches< VanillaMatch >( const MC2Coordinate& origin,
                             vector< VanillaMatch* >& matches ) {

   for ( vector< VanillaMatch*>::size_type i = 0; i < matches.size(); ++i ) {
      VanillaMatch* match = matches[ i ];
      double distanceSq =
         GfxUtility::squareP2Pdistance_linear( origin.lat,
                                               origin.lon,
                                               match->getCoords().lat,
                                               match->getCoords().lon );
      match->setDistance( (uint32)( ::sqrt( distanceSq ) ) );
   }

   SearchSorting::sortSearchMatches( matches, SearchTypes::DistanceSort );
}
