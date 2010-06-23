/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COPYRIGHTHANDLER_H
#define COPYRIGHTHANDLER_H

#include "CopyrightHolder.h"
#include "MC2String.h"

class MapModuleNoticeContainer;
class LangType;

/**
 * Builds a copyright holder from map module notice container
 * or from a set of other copyright holders.
 *
 */
class CopyrightHandler {
public:

   CopyrightHandler();
   explicit CopyrightHandler( const CopyrightHolder& holder );
   /// build copyrights from map module notice container
   CopyrightHandler( const MapModuleNoticeContainer& notice,
                     const LangType& language ); 

   /// merges another copyright holder into current copyright
   void merge( const CopyrightHolder& other );
   /// sorts the copyright ID tree
   void sort();

   /// @return copyrights
   const CopyrightHolder& getCopyrightHolder() const {
      return m_copyrights;
   }
   MC2String getCopyrightString( const MC2BoundingBox& box ) const;

private:
   /**
    * Merge copyright id strings starting from startIt with their ids 
    * pointing into copyrightStrings.
    * @param startIt start point into m_copyright.m_boxes of notices to 
    *                find copyright string duplicates for.
    * @param other The other holder to merge
    */
   void mergeCopyrightStrings( vector<CopyrightNotice>::iterator startIt,
                               const CopyrightHolder& other );


   /// builds copyrights from map module notice container
   void buildCopyrights( const MapModuleNoticeContainer& notice,
                         const LangType& language );
   /**
    * Adds copyright string if it is unique, else it will
    * return id of an already existing copyright string.
    * @return copyright ID
    */
   int addCopyright( const MC2SimpleString& copyrightString );
   typedef int64 Coverage;
   typedef std::pair< Coverage, CopyrightNotice::CopyrightID > CoverageCopyrightPair;
   typedef std::vector< CoverageCopyrightPair > CoverageByCopyrightID;

   Coverage
   recursiveCheckCoverage( const MC2BoundingBox& screenBox, 
                           int id, 
                           CoverageByCopyrightID& covByCopyrightId ) const;

   CopyrightHolder m_copyrights;
};

#endif // COPYRIGHTHANDLER_H
