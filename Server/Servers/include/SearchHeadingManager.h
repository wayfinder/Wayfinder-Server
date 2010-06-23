/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCH_HEADING_MANAGER_H
#define SEARCH_HEADING_MANAGER_H

#include "config.h"

#include "CompactSearch.h"

#include <memory>

class Requester;

/**
 * Fetches and holds the search headings from ExtServiceModule.
 * Thread safe.
 */
class SearchHeadingManager {
public:
   SearchHeadingManager();
   ~SearchHeadingManager();

   /**
    * Whether search headings should be updated, either if crc mismatch or
    * update time for headings was reached.
    * @param headingCRC A checksum to test whether the headings are the same.
    * @return True if headings needs to be updated.
    */
   bool shouldUpdateSearchHeadings( uint32 headingCRC ) const;

   /**
    * Get the headings and the associated checksum.
    * @param requster Sends search heading request to module.
    * @param headings Contains the new headings.
    * @param headingCRC Checksum for the new headings.
    */
   void getSearchHeadings( Requester& requester,
                           CompactSearchHitTypeVector& headings,
                           uint32& headingCRC );

private:

   struct Impl;
   /// Hidden Impl.
   auto_ptr< Impl > m_impl;
};

#endif // SEARCH_HEADING_MANAGER_H
