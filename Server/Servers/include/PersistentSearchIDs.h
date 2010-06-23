/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PERSISTENT_SEARCH_IDS_H
#define PERSISTENT_SEARCH_IDS_H

#include "config.h"
#include "MC2String.h"
#include <memory>

// Forwards
class SearchMatch;
class SearchParserHandler;
class LangType;
class MC2Coordinate;


/**
 * The functions for "persistent" search ids.
 */
namespace PersistentSearchIDs {

/**
 * Check if an itemID string is a "persistent" id.
 *
 * @param itemIDStr The item id string.
 * @return True if the itemIDStr is a "persistent" id, false if not.
 */
bool isPersistentID( const MC2String& itemIDStr );


/**
 * Get the "persistent" itemID string for the match.
 *
 * @param match The match to make "persistent" id for.
 * @param searchHandler A SearchParserHandler to use.
 * @return The item id string for match.
 */
MC2String makeID( const SearchMatch& match,
                  const SearchParserHandler& searchHandler );

/**
 * Get a SearchMatch from an itemID string, fills in coordinates
 * searchType, from headingID, and name in result.
 *
 * @param itemIDStr The "persistent" item id string.
 * @param searchHandler A SearchParserHandler to use.
 * @param coord Set to the coordinate of the id.
 * @param name Set to the name of the id.
 * @param headingID Set to the heading id of the id.
 * @param result Set to the result, set if function returns true.
 * @return True if item was gotten ok.
 */
bool getMatchData( const MC2String& itemIDStr,
                   const SearchParserHandler& searchHandler,
                   MC2Coordinate& coord,
                   MC2String& name,
                   uint32& headingID );

} // End namespace PersistentSearchIDs

#endif // PERSISTENT_SEARCH_IDS_H

