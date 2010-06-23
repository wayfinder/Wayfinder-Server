/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERVERRGIONIDS_H
#define SERVERRGIONIDS_H

#include "config.h"
#include "RegionIDs.h"

class TopRegionRequest;

class ServerRegionIDs : public RegionIDs {
public:

   /**
    *    Reads the region_ids.xml file and exits
    *    if it fails.
    */
   ServerRegionIDs();
   
   /**
    * Fill in the region ids that a region group has and that is in
    * the TopRegions supplied.
    *
    * @param regionIDs The vector to add region ids to.
    * @param regionGroupID The region group to add region ids for.
    * @param topReg The TopRegionRequest with the suported region IDs.
    * @return The number of added region ids.
    */
   uint32 addRegionIDsForReq( vector<uint32>& regionIDs, 
                              uint32 regionGroupID, 
                              const TopRegionRequest* topReg ) const;   
};

#endif
