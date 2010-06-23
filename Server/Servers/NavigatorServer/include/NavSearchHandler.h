/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVSEARCHHANDLER_H
#define NAVSEARCHHANDLER_H

#include "config.h"
#include "NavHandler.h"
#include "SearchTypes.h"
#include "SearchMatch.h"

class NavRequestPacket;
class NavReplyPacket;
class UserItem;
class NavUserHelp;
class SearchResultRequest;


/**
 * Class handling NavServerProt, v10+, searches.
 *
 */
class NavSearchHandler : public NavHandler {
   public:
      /**
       * Constructor.
       */
      NavSearchHandler( InterfaceParserThread* thread,
                        NavParserThreadGroup* group,
                        NavUserHelp* userHelp );


      /**
       * Handles a search packet.
       *
       * @param userItem The user.
       * @param req The request to search.
       * @param reply Set status of if problem.
       * @return True if searched ok, false if not and then reply's status
       *         is set.
       */
      bool handleSearch( UserItem* userItem, 
                         NavRequestPacket* req, NavReplyPacket* reply );


      /**
       * Converts a Nav sorting type to mc2 sorting type.
       *
       * @param navSortType The Nav sorting type.
       * @return The mc2 sorting type.
       */
      SearchTypes::SearchSorting mc2SortType( byte navSortType ) const;


      /**
       * MC2 type to Nav region type.
       */
      static uint16 mc2TypeToNavRegionType( uint32 type );


      /**
       * MC2 type to Nav item type.
       */
      static uint8 mc2TypeToNavItemType( uint32 type );


      /**
       * MC2 item type to Nav sub type.
       */
      static uint8 mc2ItemTypeToNavSubType( ItemTypes::itemType type );

      /**
       * Add area and item params.
       *
       * @param rparams Where to add the params.
       * @param sr The request with the matches.
       * @param addArea If to add area matches or not.
       * @param maxNbrMatches The maximum number of matches to add.
       * @param areaStartIndex The index for the first area match to add.
       * @param itemStartIndex The index for the first item match to add.
       * @param regionParamID The paramID to add regions too.
       * @param areaParamID The paramID to add area matches too.
       * @param areaIndexParamID The paramID for start index and total 
       *                         number matches.
       * @param itemParamID The paramID to add item matches too.
       * @param itemIndexParamID The paramID for start index and total 
       *                         number matches.
       * @param areaIndexParam The param to use instead of 
       *                       areaIndexParamID. Index is added before 
       *                       matches.
       * @param areaParam The param to use instead of areaIndexParam.
       * @param itemIndexParam The param to use instead of 
       *                       itemIndexParamID. Index is added before 
       *                       matches.
       * @param itemParam The param to use instead of itemIndexParam.
       * @param regionParam The param to use instead of regionParamID.
       * @param version 0 is orig, 1 is with image name and with 32 bit 
       *                region Search location type.
       * @param addAreaIndex If to add areaIndex, default true.
       * @param addItemIndex If to add itemIndex, default true.
       * @param headingID The current heading id for combined/compact search,
       *                  used for retrieving default image name for match.
       * @return If added any matches.
       */
      bool addMatches( NParamBlock& rparams, const SearchResultRequest* sr,
                       bool addArea, uint32 maxNbrMatches, 
                       uint32 areaStartIndex, uint32 itemStartIndex, 
                       uint16 regionParamID, 
                       uint16 areaParamID, uint16 areaIndexParamID,
                       uint16 itemParamID, uint16 itemIndexParamID,
                       NParam* areaIndexParam = NULL, 
                       NParam* areaParam = NULL,
                       NParam* itemIndexParam = NULL, 
                       NParam* itemParam = NULL,
                       NParam* regionParam = NULL,
                       int version = 0,
                       bool addAreaIndex = true,
                       bool addItemIndex = true,
                       uint32 headingID = MAX_UINT32 );

   private:
      /**
       * Adds a Nav region to regionSearchMap and pregion.
       *
       * @param match Might add fake region to match if no regions but have
       *              location name. This for external search.
       */
      void addRegionTo( SearchMatch* match, 
                        map< const SearchMatch*, uint16,
                        SearchMatchIDAndNameLessThan >& regionSearchMap, 
                        NParam& pregion,
                        int version ) const;

      /// A NavUserHelp.
      NavUserHelp* m_userHelp;
};


#endif // NAVSEARCHHANDLER_H

