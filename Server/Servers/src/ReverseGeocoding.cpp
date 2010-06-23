/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ReverseGeocoding.h"
#include "CoordinateRequest.h"
#include "ItemTypes.h"
#include "MapRequester.h"
#include "STLUtility.h"
#include "ThreadRequestContainer.h"

#include "TopRegionRequest.h"
#include "TopRegionMatch.h"

namespace ReverseGeocoding {

LookupResult lookup( const MC2Coordinate& position,
                     uint16 angle,
                     StringTable::languageCode language,
                     const set< uint32 >* allowedMaps,
                     MapRequester* requester ) {
   LookupResult result;

   // Get closest item 
   const TopRegionRequest* topReq = requester->getTopRegionRequest();

   auto_ptr<CoordinateRequest> req( 
      new CoordinateRequest( requester->getNextRequestID(),
                             topReq,
                             language ) );
   const byte itemType = ItemTypes::streetSegmentItem;
   req->addCoordinate( position.lat, position.lon, 0, 1, &itemType, angle );
   auto_ptr<ThreadRequestContainer> reqCont( 
      new ThreadRequestContainer( req.get() ) );
   requester->putRequest( reqCont.get() );
   CoordinateReplyPacket* r = req->getCoordinateReply();
   if ( (r != NULL) && (r->getStatus() == StringTable::OK) &&
        (topReq->getStatus() == StringTable::OK) ) {
      // Check if allowed country
      if ( allowedMaps == NULL ||
           STLUtility::has( *allowedMaps, r->getMapID() ) ) {
         // Get the strings from r
         const int MAX_NAME_LENGTH = 256;
         char countryName[MAX_NAME_LENGTH];
         char municipalName[MAX_NAME_LENGTH];
         char cityName[MAX_NAME_LENGTH];
         char districtName[MAX_NAME_LENGTH];
         char name[MAX_NAME_LENGTH];
         r->getStrings( countryName, municipalName, 
                        cityName, districtName, 
                        name, MAX_NAME_LENGTH );
         mc2dbg8 << "Found " << countryName << "." 
                 << municipalName << "."
                 << cityName << "." << name << endl;
         
         result.m_status = LOOKUP_OK;
         
         result.m_country = countryName;
         result.m_municipal = municipalName;
         result.m_city = cityName;
         result.m_district = districtName;
         result.m_street = name;
         
         result.m_mapID = r->getMapID();
         result.m_itemID = r->getItemID();
         result.m_streetSegmentOffset = r->getOffset();

         const TopRegionMatch* topMatch =
            topReq->getCountryForMapID( r->getMapID() );

         if ( topMatch != NULL ) {
            result.m_topRegionID = topMatch->getID();
            // Append state name to country, so we can differentiate the
            // states in USA and India.
            // Requesting the name in english, because the country is in
            // english (for now).
            // We now use ItemTypes::getLanguageCodeAsLanguageType( 
            // language ) and hope that the names from MM is the same as in
            // topregion.
            const char* regionName = topMatch->getName( 
               ItemTypes::getLanguageCodeAsLanguageType( language ) );
            const char* englishRegionName = topMatch->
               getName( LangTypes::english );
            if ( regionName != NULL ) {
               // The regions in India already have "India" prefix, so we
               // replace the country name with the region name.
               // But also "Inde" prefix, in fre, and others...
               // Make sure we do not append the same name twice
               // US regions are now named "USA - Pennsylvania". 20091030
               if ( strcmp( regionName, result.m_country.c_str() ) != 0 ) {
                  result.m_country = regionName;
               }
               // Make sure that the municipal is not the same as the country
               // name.
               if ( strcmp( regionName, result.m_municipal.c_str() ) == 0 ||
                    strcmp( englishRegionName,
                            result.m_municipal.c_str() ) == 0  ) {
                  result.m_municipal.clear();
               }
            }
         }

      } else {
         // Outside allowed area
         result.m_status = OUTSIDE_ALLOWED_AREA;
         result.m_errorString = "Coordinate outside allowed area.";
      }
   } else {
      result.m_status = LOOKUP_FAILED;
      result.m_errorString = "Problem finding street near position: ";
      mc2log << warn << "ReverseGeocoding::lookup CoordinateReplyPacket";
      if ( r != NULL ) {
         mc2log << " not OK: " << StringTable::getString( 
            StringTable::stringCode( 
               static_cast< ReplyPacket* > ( r )->getStatus() ),
            StringTable::ENGLISH ) << endl;
         if ( static_cast< ReplyPacket* > ( r )->getStatus() ==
              StringTable::MAPNOTFOUND ) 
         {
            // Outside maps
            result.m_status = OUTSIDE_MAP_COVERAGE;
            result.m_errorString.append( "Coordinate outside map coverage." );
         } else {
            result.m_errorString.append( 
               StringTable::getString( 
                  StringTable::stringCode( 
                     static_cast< ReplyPacket* > ( r )->getStatus() ),
                  StringTable::ENGLISH ) );
            }
      } else {
         mc2log << " is NULL." << endl;
         result.m_status = TIMEOUT_ERROR;
         result.m_errorString.append( StringTable::getString( 
                                         StringTable::TIMEOUT_ERROR,
                                         StringTable::ENGLISH ) );
      }
   }

   return result;
}

}
