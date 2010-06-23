/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavOneSearchHandler.h"
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
#include "OneSearchBinaryResult.h"

namespace {
void addDataBuffer( int16 paramID, NParamBlock& rparams, DataBuffer* buffer ) {
   // Add string table
   NParam& bufferPar = rparams.addParam( NParam( paramID ) );
   bufferPar.addByteArray( 
      buffer->getBufferAddress(),
      buffer->getBufferSize() );
}
}

NavOneSearchHandler::NavOneSearchHandler( 
   InterfaceParserThread* thread,
   NavParserThreadGroup* group )
      : NavHandler( thread, group ) {
   m_expectations.push_back( ParamExpectation( 6800, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6801, NParam::Int32_array ) );
   m_expectations.push_back( ParamExpectation( 6802, NParam::Int32_array ) );
   m_expectations.push_back( ParamExpectation( 6803, NParam::Int32 ) );
   m_expectations.push_back( ParamExpectation( 6804, NParam::Int32 ) );
   m_expectations.push_back( ParamExpectation( 6805, NParam::Int32 ) );
   m_expectations.push_back( ParamExpectation( 6806, NParam::Bool ) );
   m_expectations.push_back( ParamExpectation( 6807, NParam::Int32 ) );
   m_expectations.push_back( ParamExpectation( 6808, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6809, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 6810, NParam::Int32 ) );
   m_expectations.push_back( ParamExpectation(    6, NParam::Uint16 ) );
}


bool
NavOneSearchHandler::handleOneSearch( NavRequestData& rd ) {
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }

   // The user
   UserUser* user = rd.session->getUser()->getUser();
   uint32 startTime = TimeUtility::getCurrentTime();
   uint32 now = TimeUtility::getRealTime();

   bool ok = true;

   // AURA
   set< uint32 >* allowedMaps = NULL;
   if ( !m_thread->getMapIdsForUserRegionAccess( user, allowedMaps,
                                                 now, rd.urmask ) )
   {
      mc2log << warn << "handleOneSearch: "
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
   delete allowedMaps;

   // Start parameter printing
   mc2log << info << "handleOneSearch:";
   CompactSearch params;
   params.m_oneResultList = true;
   params.m_itemInfoFilter = ItemInfoEnums::OneSearch_All;

   params.m_language = rd.clientLang;
   mc2log << " Lang " << StringTable::getString( 
         StringTable::getLanguageAsStringCode( rd.stringtClientLang ), 
         StringTable::ENGLISH );
   if ( getParam( 6805, params.m_round, rd.params ) ) {
      mc2log << " Round " << params.m_round;
   }
   if ( getParam( 6804, params.m_maxHits, rd.params ) ) {
      mc2log << " MAX " << params.m_maxHits;
   }
   if ( getParam( 6800, params.m_what, rd.params ) ) {
      mc2log << " What " << params.m_what;
   }
   if ( NavParamHelper::getMC2CoordAndAngle( rd.params.getParam( 6802 ), 
                                             params.m_location.m_coord,
                                             params.m_location.m_angle ) ) {
      mc2log << " Pos " << params.m_location.m_coord;
   }
   if ( getParam( 6803, params.m_distance, rd.params ) ) {
      mc2log << " Dist " << params.m_distance;
   } else if ( params.m_location.isValid() ) {
      // default 100km for distance
      params.m_distance = 100000;
      mc2log << " Dist " << params.m_distance;
   }

   if ( ! getParam( 6806, params.m_includeInfoItem, rd.params ) ) {
      params.m_includeInfoItem = true;
   }

   if ( params.m_includeInfoItem ) {
      mc2log << " include info fields ";
   }

   // Get the sorting type
   params.m_sorting = SearchTypes::DistanceSort;
   int32 sortType;
   if ( getParam( 6807, sortType, rd.params ) ) {
      mc2log << " Sort " << sortType;
      if( sortType == 1 ) {
         params.m_sorting = SearchTypes::AlphaSort;
      }
   }

   // Search category
   const NParam* p = rd.params.getParam( 6801 );
   if ( p != NULL ) {
      uint32 length = p->getLength();
      params.m_categoryIDs.resize( length );
      for ( uint32 i = 0; i < length; ++i ) {         
         params.m_categoryIDs.push_back( p->getInt32( i ) );
      }         
   }

   if ( rd.reqVer >= 2 ) {
      if ( getParam( 6808, params.m_where, rd.params ) ) {
         mc2log << " Where " << params.m_where;
      }

      if ( getParam( 6809, params.m_topRegionID, rd.params ) ) {
         mc2log << " TopRegionID " << params.m_topRegionID;
      }

      int32 searchType;
      if ( getParam( 6810, searchType, rd.params ) ) {
         if ( searchType == 0x1 && params.m_round == 0 ) {
            mc2log << " Address search ";
            params.m_heading = 1; // Address heading
         }
      }            
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

    // Set end index from max nbr hits and start index
   params.m_startIndex = 0;
   params.m_endIndex = params.m_maxHits - 1;

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
            mc2log << "handleOneSearch: compactSearch Heading "
                   << (*it).first << " failed: ";
            if ( (*it).second->getSearchResultRequest()->getStatus() == 
                 StringTable::TIMEOUT_ERROR ) {
               mc2log << " REQUEST_TIMEOUT";
               rd.reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
            } else if ( (*it).second->getSearchResultRequest()->getStatus() == 
                        StringTable::OUTSIDE_ALLOWED_AREA ) {
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
      // one search only returns one heading in the list...
      SearchResults::const_iterator it = results.begin();
      if ( it != results.end() ) {
         const SearchResultRequest* match = 
            (*it).second->getSearchResultRequest();
         const vector<VanillaMatch*>& matches = match->getMatches();
         uint32 numberMatches = MIN( params.m_maxHits, matches.size() );

         rd.rparams.addParam( NParam( 6900, numberMatches ) );
         rd.rparams.addParam( NParam( 6901, match->getTotalNbrMatches() ) );
         
         if ( numberMatches > 0 ) {
            using namespace OneSearchUtils;
            BinarySearchResult binaryResult;
            serializeResults( matches.begin(), 
                              matches.begin() + numberMatches, 
                              m_thread->getSearchHandler(), 
                              &binaryResult );
            
            // Add SearchStringTable
            addDataBuffer( 6902, rd.rparams, binaryResult.m_stringTable.get() );
            
            // Add AreaTable
            addDataBuffer( 6903, rd.rparams, binaryResult.m_areaTable.get() );
            
            // Add infoItemTable
            addDataBuffer( 6904, rd.rparams, binaryResult.m_infoItemTable.get() );
            
            // Add Matches
            addDataBuffer( 6905, rd.rparams, binaryResult.m_matchesTable.get() );
         } else {
            // No hits. Add empty arrays
            rd.params.addParam( NParam( 6902 ) );
            rd.params.addParam( NParam( 6903 ) );
            rd.params.addParam( NParam( 6904 ) );
            rd.params.addParam( NParam( 6905 ) );
         }
      }
   }

   return ok;
}
