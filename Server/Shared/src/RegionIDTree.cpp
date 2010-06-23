/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RegionIDTree.h"

void
RegionIDTree::fixupTopLevel()
{
   // Go through all regions and save regions that are inside
   // other regions.
   set<uint32> regionsInsideOtherRegions;
   for( overviewKeyed_t::const_iterator it(m_byOverview.begin());
        it != m_byOverview.end();
        ++it ) {
      // Mapid is region id.
      regionsInsideOtherRegions.insert( it->second.getMapID() );
   }

   // Go through the  overview keyed regions
   // and check which ones that do not exist
   // in any other region (incl MAX_UINT32).
   // and change the  ones that  are inside
   // to belong to MAX_UINT32.
   
   overviewKeyed_t copyOfByOverview( m_byOverview );
   m_byOverview.clear();
   
   for( overviewKeyed_t::iterator it( copyOfByOverview.begin());
        it != copyOfByOverview.end();
        ++it ) {
      if ( regionsInsideOtherRegions.find( it->first ) ==
           regionsInsideOtherRegions.end() ) {
         // This region is not inside a region that is inside a region
         // (including MAX_UINT32).
         addRegion( MAX_UINT32, it->second.getMapID());
      } else {
         addRegion( it->first, it->second.getMapID());
      }
   }
}
