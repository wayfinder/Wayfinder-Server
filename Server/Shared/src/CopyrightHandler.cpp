/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CopyrightHandler.h"
#include "MapGenEnums.h"
#include "MapModuleNoticeContainer.h"
#include "LangTypes.h"
#include "Sort1st.h"

#include <algorithm>

struct gtFirst {

   bool operator() (const std::pair<int64,int>& p1, 
                    const std::pair<int64,int>& p2 ) const
   {
      return p1.first > p2.first;
   }
};

CopyrightHandler::CopyrightHandler() {
}

CopyrightHandler::
CopyrightHandler( const MapModuleNoticeContainer& notice,
                  const LangType& language ) {
   buildCopyrights( notice, language );
}

CopyrightHandler::CopyrightHandler( const CopyrightHolder& holder ):
   m_copyrights( holder ) {
}

/// merges two copyright trees into one tree
void CopyrightHandler::merge( const CopyrightHolder& other ) {
   // offset for the merge
   int32 offset = m_copyrights.m_boxes.size();

   typedef CopyrightHolder::mapIt_t MapIt;

   // copy the ids and add offset so they point
   // to the correct place in the CopyrightNotice vector.
   MapIt mapIt = other.m_boxesByParent.begin();
   MapIt mapItEnd = other.m_boxesByParent.end();
   for ( ; mapIt != mapItEnd; ++mapIt ) {
      // now lets offset the ids and insert them
      // and dont offset the special ID = MAX_INT32
      int32 index = (*mapIt).first == MAX_INT32 ? 
         MAX_INT32 : (*mapIt).first + offset;
      m_copyrights.m_boxesByParent.
         push_back( make_pair( index, (*mapIt).second + offset ) );
   }

   // copy all boxes
   m_copyrights.m_boxes.insert( m_copyrights.m_boxes.end(),
                                other.m_boxes.begin(),  
                                other.m_boxes.end() );

   // merge copyright strings if the merged copyright is not 
   // the first copyright holder
   if ( offset != 0 ) {
      mergeCopyrightStrings( m_copyrights.m_boxes.begin() + offset, other );
   } else {
      m_copyrights.m_copyrightStrings = other.m_copyrightStrings;
      m_copyrights.m_strIdxToSupplier = other.m_strIdxToSupplier;
   }

   // default coverages is 10 percent
   m_copyrights.m_minCovPercent = 10;
   // Set copyright to what you have
   m_copyrights.m_copyrightHead = "© ";

   //   m_copyrights.dump();
}

void CopyrightHandler::
mergeCopyrightStrings( vector<CopyrightNotice>::iterator startIt,
                       const CopyrightHolder& other ) {
   const vector<MC2SimpleString>& copyrightStrings = other.m_copyrightStrings;

   for ( ; startIt != m_copyrights.m_boxes.end(); ++startIt ) {

      // search for a copyright string from the "other" copyrights
      // in the m_copyrights
      vector<MC2SimpleString>::iterator stringIt = 
         find( m_copyrights.m_copyrightStrings.begin(),
               m_copyrights.m_copyrightStrings.end(),
               copyrightStrings[ (*startIt).getCopyrightID() ] );

      if ( stringIt == m_copyrights.m_copyrightStrings.end() ) {
         // did not find any copyright with the same name, lets
         // add it and set new copyright id
         m_copyrights.m_copyrightStrings.
            push_back( copyrightStrings[ (*startIt).getCopyrightID() ] );

         // save supplier ID before changing the original copyright ID
         CopyrightHolder::StringIndexToSupplier::const_iterator 
            suppIt = other.m_strIdxToSupplier.find( (*startIt).m_copyrightID );
         CopyrightHolder::SupplierID supplier = (*suppIt).second;

         // set new copy right id
         (*startIt).m_copyrightID = m_copyrights.m_copyrightStrings.size() -1;

         // update string index to supplier id in the current copyright
         m_copyrights.m_strIdxToSupplier[ (*startIt).m_copyrightID ] = 
            supplier;
      } else {
         // found an already existing copyright string with the 
         // same name, update copyright ID and there is no need 
         // to set string index to supplier ID here since it already
         // exist.
         (*startIt).m_copyrightID = 
            distance( m_copyrights.m_copyrightStrings.begin(),
                      stringIt );
      }
   }
}

void CopyrightHandler::sort() {
   std::sort( m_copyrights.m_boxesByParent.begin(),
              m_copyrights.m_boxesByParent.end() );
}

CopyrightHandler::Coverage
CopyrightHandler::
recursiveCheckCoverage( const MC2BoundingBox& screenBox, 
                        int id,
                        CoverageByCopyrightID& covByCopyrightId ) const {

   const CopyrightNotice& curNotice = m_copyrights.m_boxes[ id ];
   
   MC2BoundingBox intersection;
   if ( ! curNotice.getBox().getInterSection( screenBox, intersection ) ) {
      // no overlapping
      return 0;
   }

   // Calculate the coverage as area.
   Coverage coverage = (Coverage) intersection.getLonDiff() * 
      (Coverage) intersection.getHeight();
   mc2dbg << "coverage (copyright): " << coverage << endl;
      
   Coverage covForChildren = 0;
   CopyrightHolder::range_t range = 
      std::equal_range( m_copyrights.m_boxesByParent.begin(),
                        m_copyrights.m_boxesByParent.end(), 
                        id, 
                        STLUtility::
                        makeSort1st( m_copyrights.m_boxesByParent ) );

   for ( CopyrightHolder::mapIt_t it = range.first; 
         it != range.second; ++it ) {

      int childID = it->second;
      covForChildren += recursiveCheckCoverage( intersection, 
                                                childID, 
                                                covByCopyrightId );
      if ( covForChildren == coverage ) {
         break;
      }
      if ( covForChildren > coverage ) {
         mc2log << error << "[CopyrightHolder] covForChildren > coverage! "
                << " covForChildren = " << covForChildren 
                << " coverage = " << coverage << endl;
      }
      
   }

   if ( curNotice.isOverlappingBox() ) {
      // Box is only used as bounding box for children and
      // does not contain any copyright info of it's own.
      // Return the copyright coverage for the children.
      return covForChildren;
   }
      
   // Calculate the current coverage, 
   // by subtracting any coverage already covered by a child.
   Coverage curCov = coverage - covForChildren;
      
   mc2dbg << "curCov (copyright): " << curCov << endl;
   mc2dbg << "covForChildren (copyright): " << covForChildren << endl;
      
   // Update coverage.
   covByCopyrightId[ curNotice.getCopyrightID() ].first += curCov;

   return coverage;
}

MC2String
CopyrightHandler::
getCopyrightString( const MC2BoundingBox& box ) const {

   // Get the root boxes. (Parent is MAX_INT32)
  
   CopyrightHolder::range_t range = 
      std::equal_range( m_copyrights.m_boxesByParent.begin(),
                        m_copyrights.m_boxesByParent.end(), 
                        MAX_INT32, 
                        STLUtility::
                        makeSort1st( m_copyrights.m_boxesByParent ) );

   // First is coverage area, second is copyright id.
   CoverageByCopyrightID covByCopyrightId;
   covByCopyrightId.resize( m_copyrights.m_copyrightStrings.size() );
   for ( uint32 i = 0; i < covByCopyrightId.size(); ++i ) {
      covByCopyrightId[ i ] = std::make_pair( 0, i );
   }

   for ( CopyrightHolder::mapIt_t it = range.first;
         it != range.second; ++it ) {
      recursiveCheckCoverage( box, it->second, covByCopyrightId );
   }
 
   // Will contain the coverage area in first and copyright id in second.
   // Will be sorted in coverage order.
   std::sort( covByCopyrightId.begin(), covByCopyrightId.end(), gtFirst() );
   
   // The result should be in covByCopyrightId.
   float32 totalArea = 0;
   for ( uint32 i = 0; i < covByCopyrightId.size(); ++i ) {
      totalArea += (float32) covByCopyrightId[ i ].first;
   }
  
   if ( totalArea < 0.000001 ) {
      // No copyright area found. Return to avoid divide by zero.
      return m_copyrights.m_copyrightHead.c_str();
   }

   // First add the copyright head.
   MC2String copyrightString = m_copyrights.m_copyrightHead.c_str();
   for ( uint32 i = 0; i < covByCopyrightId.size(); ++i ) {
      int percent = int( (float32)covByCopyrightId[ i ].first / 
                         totalArea * 100 );

      if ( percent >= m_copyrights.m_minCovPercent ) {
         copyrightString += ",";
         copyrightString += m_copyrights.
            m_copyrightStrings[ covByCopyrightId[ i ].second ].c_str();
      }

      mc2dbg << "Copyright Id: " << i << ", " 
             << m_copyrights.m_copyrightStrings[ covByCopyrightId[ i ].second ]
             << ", area: " << covByCopyrightId[ i ].first 
             << ", percent: " << percent
             << ", total area: " << totalArea << endl;

   }

   return copyrightString;
}

CopyrightNotice::CopyrightID 
CopyrightHandler::addCopyright( const MC2SimpleString& copyrightString ) {

   // see if there is a copyright string in the
   // the copyright strings vector first, and if it is then
   // assign the copyright ID to the position 
   vector<MC2SimpleString>::iterator copyrightIt = 
      std::find( m_copyrights.m_copyrightStrings.begin(), 
                 m_copyrights.m_copyrightStrings.end(),
                 copyrightString );

   CopyrightNotice::CopyrightID copyrightID = 0;
   if ( copyrightIt == m_copyrights.m_copyrightStrings.end() ) {
      // did not find copyright string, add it as new and
      // set copyrightID as the last one
      copyrightID = m_copyrights.m_copyrightStrings.size();
      m_copyrights.m_copyrightStrings.push_back( copyrightString );
   } else {
      // found a copyright string, lets use the same id
      copyrightID = std::distance( m_copyrights.m_copyrightStrings.begin(), 
                                   copyrightIt );
   }
   return copyrightID;
}

namespace {
/// @return copyright string for map supplier
MC2SimpleString 
getCopyrightString( MapGenEnums::mapSupplier supplierID,
                    const LangType& language, 
                    const MapModuleNoticeContainer& mmnotice ) {
   const NameCollection& names = mmnotice.getMapSupNames( supplierID );
   return names.getBestName( language )->getName();

}
}

void CopyrightHandler::
buildCopyrights( const MapModuleNoticeContainer& mmnotice,
                 const LangType& language ) {
   const MapModuleNoticeContainer::coverageTree_t& covtree =
      mmnotice.getMapSupCoverageTree();
   const MapModuleNoticeContainer::mapSupCoverage_t& supcov =
      mmnotice.getMapSupCoverageAreas();

   typedef MapModuleNoticeContainer::coverageTree_t::const_iterator
      coverageTreeConstIt;
   typedef MapModuleNoticeContainer::mapSupCoverage_t::const_iterator
      coverageAreaConstIt;

   m_copyrights.m_boxes.reserve( supcov.size() );
   m_copyrights.m_boxesByParent.reserve( supcov.size() );
   coverageTreeConstIt treeIt = covtree.begin();
   for ( coverageAreaConstIt it = supcov.begin();
         it != supcov.end(); ++it, ++treeIt ) {
      // setup a copyright notice
      CopyrightNotice copNotice;
      copNotice.m_box = (*it).first;
      MapGenEnums::mapSupplier supplier = (*it).second;
      copNotice.m_copyrightID = 
         addCopyright( ::getCopyrightString( supplier, language, 
                                             mmnotice ) );

      m_copyrights.m_strIdxToSupplier[ copNotice.m_copyrightID ] = supplier;

      // at this point the CopyrightNotice is ready
      m_copyrights.m_boxes.push_back( copNotice );
      // the client assumes int32s so;
      // convert uint32's MAX_UINT32 to MAX_INT32
      // I think we can assume we dont go over MAX_UINT32 / 2 
      int32 index = ( (*treeIt).first == MAX_UINT32 ) ? MAX_INT32 : 
         static_cast<int32>( (*treeIt).first );
      m_copyrights.m_boxesByParent.
         push_back( make_pair( index, (*treeIt).second ) );
   }
   // Set copyright to what you have
   m_copyrights.m_copyrightHead = "© ";
}
