/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavCombinedSearchHandler.h"
#include "InterfaceParserThread.h"
#include "isabBoxSession.h"
#include "NavPacket.h"
#include "UserData.h"
#include "NavUserHelp.h"
#include "NavRequestData.h"
#include "CompactSearch.h"
#include "SearchMatch.h"
#include "STLUtility.h"
#include "SearchParserHandler.h"
#include "SearchResultRequest.h"
#include "NavSearchHandler.h"
#include "TimeUtility.h"
#include "OperationTypes.h"
#include "NavParserThreadGroup.h"
#include "ExternalSearchRequest.h"
#include "NavParamHelper.h"

#include "HttpInterfaceRequest.h"
#include "HttpHeader.h"
#include "ClientSettings.h"
#include "PurchaseOptions.h"

NavCombinedSearchHandler::NavCombinedSearchHandler( 
   InterfaceParserThread* thread,
   NavParserThreadGroup* group,
   NavUserHelp* userHelp,
   NavSearchHandler* searchHandler )
      : NavHandler( thread, group ),
        m_userHelp( userHelp ), m_searchHandler( searchHandler )
{
   m_expectations.push_back( ParamExpectation( 5600, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 5601, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 5602, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 5603, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 1201, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1202, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1203, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1205, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 1209, NParam::Bool ) );
   m_expectations.push_back( ParamExpectation(    6, NParam::Uint16 ) );
}


bool
NavCombinedSearchHandler::handleCombinedSearch( NavRequestData& rd ) {
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }

   bool ok = true;


   // The user
   UserUser* user = rd.session->getUser()->getUser();
   uint32 startTime = TimeUtility::getCurrentTime();
   uint32 now = TimeUtility::getRealTime();

   // AURA
   set< uint32 >* allowedMaps = NULL;
   if ( ok && !m_thread->getMapIdsForUserRegionAccess( user, allowedMaps,
                                                       now, rd.urmask ) )
   {
      mc2log << warn << "handleCombinedSearch: "
             << "getMapIdsForUserRegionAccess failed. ";
      if ( TimeUtility::getCurrentTime() - startTime > 3000 ) {
         rd.reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
         mc2log << "Timeout";
      } else {
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         mc2log << "Error";
      }
      mc2log << endl;
      ok = false;
   }

   // Start parameter printing
   mc2log << info << "handleCombinedSearch:";
   CompactSearch params;

   params.m_language = rd.clientLang;
   mc2log << " Lang " << StringTable::getString( 
         StringTable::getLanguageAsStringCode( rd.stringtClientLang ), 
         StringTable::ENGLISH );
   if ( getParam( 5600, params.m_startIndex, rd.params ) ) {
      mc2log << " IN " << params.m_startIndex;
   }
   if ( getParam( 5601, params.m_round, rd.params ) ) {
      mc2log << " Round " << params.m_round;
   }
   if ( getParam( 5602, params.m_heading, rd.params ) ) {
      mc2log << " Heading/Type " << params.m_heading;
   }
   if ( getParam( 5603, params.m_maxHits, rd.params ) ) {
      mc2log << " MAX " << params.m_maxHits;
   }
   if ( getParam( 1201, params.m_where, rd.params ) ) {
      mc2log << " A " << params.m_where;
   }
   MC2String areaIDStr;
   getParam( 1202, areaIDStr, rd.params );
   if ( !areaIDStr.empty() && ok ) {
      auto_ptr<SearchMatch> selectedArea( SearchMatch::createMatch(areaIDStr));
      if ( selectedArea.get() == NULL ) {
         mc2log << warn << "handleCombinedSearch: Bad areaID " 
                << MC2CITE( areaIDStr ) << endl;
         ok = false;
         rd.reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_PARAMETER_INVALID );
         rd.reply->setStatusMessage( "Search AreaID" );
      } else {
         // Check if area is in allowed maps
         if ( allowedMaps != NULL && selectedArea->getID().isValid() &&
              allowedMaps->find( selectedArea->getMapID() ) == 
              allowedMaps->end() )
         {
            mc2log << warn << "handleCombinedSearch: selected area "
                   << "unallowed map " << MC2CITE( areaIDStr ) << endl;
            ok = false;
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_OUTSIDE_ALLOWED_AREA );
         }
      }

      if ( ok ) {
         params.m_areaID = selectedArea->getID();
         mc2log << " SA " << params.m_areaID;
      }
   }
   if ( getParam( 1203, params.m_what, rd.params ) ) {
      mc2log << " I " << params.m_what;
   }
   // Search category
   if ( getParam( 1204, params.m_categoryName, rd.params ) ) {
      mc2log << " C " << params.m_categoryName;
   }
   bool haveTopRegion = false;
   if ( getParam( 1205, params.m_topRegionID, rd.params ) ) {
      mc2log << " TID " << MC2HEX( params.m_topRegionID );
      haveTopRegion = true;
   }
   if ( NavParamHelper::getCoordAndAngle( rd.params.getParam( 1000 ), 
                                          params.m_location.m_coord,
                                          params.m_location.m_angle ) ) {
      mc2log << " Pos " << params.m_location.m_coord;
   }
   if ( getParam( 5604, params.m_distance, rd.params ) ) {
      mc2log << " Dist " << params.m_distance;
   } else if ( params.m_location.isValid() ) {
      // default 100km for distance
      params.m_distance = 100000;
      mc2log << " Dist " << params.m_distance;
   }

   CategoryTreeUtils::CategoryID categoryID = CategoryTreeUtils::INVALID_CATEGORY;
   if ( getParam( 5605, categoryID, rd.params ) ) {
      mc2log << " CatID " << categoryID << endl;
      params.m_categoryIDs.push_back( categoryID );
   }

   if ( getParam( 1209, (byte&)params.m_findCategories, rd.params ) ) {
      mc2log << " Find Cat " << params.m_findCategories;
   }

   // strip any leading and trailing white space from input
   params.cleanInput();
  	 
   // Validate input fields
   if ( ! params.validInput() ) {
      rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_PARAMETER_INVALID );
      rd.reply->setStatusMessage( "Invalid input" );
      ok = false;
   }

   // End parameter printing
   mc2log << endl;

   // Check with external access if user is allowed to search
   PurchaseOptions purchaseOptions( rd.clientSetting->getClientType() );
   set< MC2String > checkedServiceIDs;
   if ( ! m_thread->checkService( rd.clientSetting,
                                  rd.ireq->getHttpRequest(),
                                  OperationType::SEARCH_RIGHT,
                                  purchaseOptions,
                                  checkedServiceIDs,
                                  params.m_location.m_coord,
                                  params.m_topRegionID,
                                  rd.clientLang,
                                  params.m_round >= 1 ) ) {
      
      mc2log << warn << "handleCombinedSearch: checkService failed.";
      ok = false;
      rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
      using namespace PurchaseOption;
      if ( purchaseOptions.getReasonCode() == 
           PurchaseOption::NEEDS_TO_BUY_APP_STORE_ADDON ) {
         // Add the packages to reply
         MC2String packageStr( "iPhoneAppStore;" );
         const PurchaseOptions::packageIDs& packages =
            purchaseOptions.getAppPackages();
         for ( PurchaseOptions::packageIDs::const_iterator it = 
                  packages.begin(), end = packages.end() ; it != end ; 
               ++it ) {
            if ( it != packages.begin() ) {
               packageStr.append( "," );
            }
            packageStr.append( *it );
         }
         rd.rparams.addParam( NParam( 33, packageStr ) );
         // Set status, but no url (the url in purchaseOptions isn't 
         // for app store)
         rd.reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_EXPIRED_USER );
      } else {
         mc2log << " General error.";
         if ( purchaseOptions.getReasonCode() == SERVICE_NOT_FOUND ) {
            mc2log << " Service id not possible to purchase..";

            MC2String noServiceIdURL( "http://show_msg/?txt=" );
            noServiceIdURL += StringUtility::URLEncode(
               StringTable::getString( StringTable::WF_NO_BILL_AREA,
                                       rd.clientLang ) );
            rd.rparams.addParam( NParam( 26, 
                                         noServiceIdURL,
                                         m_thread->clientUsesLatin1() ) );
         }
      }

      mc2log << endl;
      return ok;
   }

   // Set end index from max nbr hits and start index
   params.m_endIndex = MAX( params.m_maxHits -1, 0 ) + params.m_startIndex;

   // Check top region
   if ( ok && haveTopRegion && params.m_topRegionID != MAX_UINT32 &&
        !m_thread->checkUserRegionAccess( 
           params.m_topRegionID, user, rd.urmask ) ) {
      mc2log << warn << "handleCombinedSearch: TopRegion " 
             << params.m_topRegionID << " unallowed." << endl;
      ok = false;
      rd.reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_OUTSIDE_ALLOWED_AREA );
   }

   // Make request
   typedef STLUtility::AutoContainerMap< 
      SearchParserHandler::SearchResults > SearchResults;
   SearchResults results;
   if ( ok ) {
      results = m_thread->getSearchHandler().compactSearch( params );
      for ( SearchResults::const_iterator it = results.begin() ; 
            it != results.end() ; ++it ) {
         if ( (*it).second->getSearchResultRequest()->getStatus() != 
              StringTable::OK ) {
            mc2log << "handleCombinedSearch: compactSearch Heading "
                   << (*it).first << " failed: ";
            if ( (*it).second->getSearchResultRequest()->getStatus() == 
                 StringTable::TIMEOUT_ERROR )
            {
               mc2log << " REQUEST_TIMEOUT";
               rd.reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
            } else if ( (*it).second->getSearchResultRequest()->getStatus() == 
                        StringTable::OUTSIDE_ALLOWED_AREA )
            {
               mc2log << " OUTSIDE_ALLOWED_AREA";
               rd.reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_OUTSIDE_ALLOWED_AREA );
            } else { 
               mc2log << " NOT_OK";
               rd.reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_NOT_OK );
            }
            if ( params.m_round == 1 && 
                 (*it).second->getSearchResultRequest()->getStatus() != 
                 StringTable::OUTSIDE_ALLOWED_AREA ) {
               mc2log << " BUT round 1 (ExternalSearch) so we ignore";
               rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_OK );
            } else {
               ok = false;
            }
            mc2log << endl;
         }
      }
   }

   // Put reply together
   if ( ok ) {
      const char* sponsStr = StringTable::
         getString( StringTable::SPONSORED_RESULTS, params.m_language );
      const char* allStr = StringTable::
         getString( StringTable::ALL_RESULTS, params.m_language );
      mc2log << "handleCombinedSearch: Reply:";
      uint32 i = 0;
      if ( ! results.empty() && rd.reqVer >= 2 ) {
         // add "sponsored links" and "all results" titles
         NParam& headingSearchText = rd.rparams.addParam( NParam( 6000 ) );
         headingSearchText.addString( sponsStr, m_thread->clientUsesLatin1() );
         headingSearchText.addString( allStr, m_thread->clientUsesLatin1() );
      }

      for ( SearchResults::const_iterator it = results.begin() ; 
            it != results.end() ; ++it ) {
         const uint32 heading = (*it).first;
         SearchResultRequest* match = 
            (*it).second->getSearchResultRequest();
         mc2log << " Heading " << heading << ":";
         NParam& p = rd.rparams.addParam( NParam( 5700 + i++ ) );
         bool addAreaMatches = false;
         bool addItemMatches = true;
         uint32 listType = 0;
         if ( match->getMatches().empty() &&
              match->getNbrOverviewMatches() > 1 ) {
            addAreaMatches = true;
            addItemMatches = false;
            listType = 0x1;
         }

         p.addUint32( heading );
         p.addUint32( listType );
         p.addUint32( params.m_startIndex );
         p.addUint32( match->getTotalNbrMatches() );
         // add number of top hits
         if ( rd.reqVer >= 2 ) {
            ExternalSearchRequest* req = 
               dynamic_cast<ExternalSearchRequest*>( match );
            if ( req ) {
               // min top hits, still a hack though.
               p.addUint32( min( req->getNbrTopHits(), 
                                 match->getTotalNbrMatches() ) );
            } else {
               p.addUint32( 0 );
            }
            // add "sponsored links" and "all results" titles
            p.addString( sponsStr, m_thread->clientUsesLatin1() );
            p.addString( allStr, m_thread->clientUsesLatin1() );
         }

         NParam& regions = rd.rparams.addParam( NParam( 5700 + i++ ) );
         
         m_searchHandler->addMatches( 
            rd.rparams, match, addAreaMatches,
            params.m_maxHits, 0, params.m_startIndex, 0, 
            0, 0, 0, 0, NULL, &p, NULL, &p, &regions, 
            rd.reqVer, false, false,
            heading );
      }
      mc2log << endl;
   }

   delete allowedMaps;

   return ok;
}
