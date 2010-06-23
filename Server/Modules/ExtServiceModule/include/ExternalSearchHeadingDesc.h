/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTERNAL_SEARCH_HEADING_DESC_H
#define EXTERNAL_SEARCH_HEADING_DESC_H

#include "config.h"
#include "CompactSearch.h"

/**
 * Creates header list for Power Search(tm).
 */
class ExternalSearchHeadingDesc {
public:

   /** 
    * Special headings such as favorites and phonebook.
    * Do not change these numbers.
    */
   enum SpecialHeadings {
      FAVORITES_HEADING = 100, ///< favorites
      PHONEBOOK_HEADING,        ///< phonebook in the client
      ADVERTISEMENT_HEADING = 1000 ///< advertisement heading
   };

   ExternalSearchHeadingDesc();

   void getHeadings( CompactSearchHitTypeVector& other, uint32& crc ) {
      other = m_headings;
      crc = getHeadingsCRC();
   }

private:
   /// Get CRC for the search headings
   uint32 getHeadingsCRC();

   /// contains all search headings
   CompactSearchHitTypeVector m_headings;

   // the crc of the headings vector
   uint32 m_crc;
};

#endif // EXTERNAL_SEARCH_HEADING_DESC_H
