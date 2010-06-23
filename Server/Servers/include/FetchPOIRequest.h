/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FETCH_POI_REQUEST_H
#define FETCH_POI_REQUEST_H

#include "config.h"

#include "MC2Coordinate.h"
#include "LangTypes.h"
#include "MC2String.h"

#include <memory>

class MapRequester;
class SearchResultRequest;

/**
 * @namespace Holds various utilities to fetch pois data.
 */
namespace POIFetch {

/**
 * POI fetch request.
 * Fetch pois using coordinate and translate the poi strings to
 * a specific language.
 */
class Request {
public:
   /**
    * @param center Centrum point for searching.
    * @param radiusInMeters Radius in meters.
    * @param startIndex Start offset of search result.
    * @param endIndex End offset of search result.
    * @param language Translate strings to this language.
    */
   Request( const MC2Coordinate& center,
            uint32 radiusInMeters,
            uint32 startIndex,
            uint32 endIndex,
            const LangType& language,
            const MC2String& searchString ):
      m_center( center ),
      m_radius( radiusInMeters ),
      m_startIndex( startIndex ),
      m_endIndex( endIndex ),
      m_language( language ),
      m_searchString( searchString ) {
   }

   /// @return Center coordinate.
   const MC2Coordinate& getCenter() const {
      return m_center;
   }

   /// @return Radius in meters.
   uint32 getRadius() const {
      return m_radius;
   }

   /// @return Start index.
   uint32 getStartIndex() const {
      return m_startIndex;
   }

   /// @return Ending index.
   uint32 getEndIndex() const {
      return m_endIndex;
   }

   /// @return The language to translate poi strings to.
   const LangType& getLanguage() const {
      return m_language;
   }

   /// @return the item string to search for.
   const MC2String& getSearchString() const {
      return m_searchString;
   }

private:
   Request();

   /// Center point for search.
   const MC2Coordinate m_center;
   /// Radius in meters.
   const uint32 m_radius;
   /// The start offset index
   const uint32 m_startIndex;
   /// The end offset index
   const uint32 m_endIndex;
   /// The language in which the hits will be translated to.
   const LangType m_language;
   /// The item string to search for
   const MC2String m_searchString;
};

/// Type returned by fetchPOIs
typedef std::auto_ptr< SearchResultRequest > SearchResult;

/**
 * Search for pois within request radius.
 * @param requester Sends the required packets to various modules.
 * @param request @see Request
 * @return Hits within the requested radius.
 */
SearchResult fetchPOIs( MapRequester& requester, const Request& request );


} // POIFetch

#endif // FETCH_POI_REQUEST_H

