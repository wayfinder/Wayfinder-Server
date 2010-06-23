/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GET_SEARCH_ITEM_H
#define GET_SEARCH_ITEM_H

#include "config.h"
#include "MC2String.h"
#include <memory>

// Forwards
class Requester;
class LangType;
class SearchMatch;

/**
 * The functions for retreiving SearchMatches from SearchModule
 * if you only have the ID of the match.
 */
namespace GetSearchItem {

/**
 * Get the SearchMatch for the item id as string, see SearchMatch.
 *
 * @param requester The thing that can send Requests.
 * @param itemIDStr The item id string.
 * @param language The language requested.
 * @param result Set to the result, set if function returns true.
 * @return True if item was gotten ok.
 */
bool request( Requester& requester,
              const MC2String& itemIDStr,
              const LangType& language,
              auto_ptr<SearchMatch>& result );

/**
 * Get the SearchMatch from the SearchModule for the SearchMatch.
 *
 * @param requester The thing that can send Requests.
 * @param inputMatch The item to get the search match for.
 * @param language The language requested.
 * @param result Set to the result, set if function returns true.
 * @return True if item was gotten ok.
 */
bool request( Requester& requester,
              const SearchMatch* inputMatch,
              const LangType& language,
              auto_ptr<SearchMatch>& result );

} // End namespace GetSearchItem

#endif // GET_SEARCH_ITEM_H

