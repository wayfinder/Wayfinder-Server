/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavInfoHandler.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "UserData.h"
#include "NavUtil.h"
#include "SearchMatch.h"
#include "ItemInfoRequest.h"
#include "InfoTypeConverter.h"
#include "ItemDetailEnums.h"
#include "ItemInfoEntry.h"
#include "SearchParserHandler.h"

namespace {

void addStringAndIndex( const MC2String& str, NParam& indexTable, NParam& stringTable ) {
   uint16 index = stringTable.getLength();
   stringTable.addString( str );
   indexTable.addUint32( index );
}

// Append reviews and images to the result
void appendResources( NParamBlock& rparams, const SearchMatch* match, 
                      const SearchParserHandler& searchHandler) {
   const VanillaCompanyMatch* poiMatch = 
      dynamic_cast< const VanillaCompanyMatch* >( match );

   if ( poiMatch != NULL ) {

      // String table
      NParam& strings = rparams.addParam( NParam( 7103 ) );

      // Add images 
      const VanillaCompanyMatch::ImageURLs& imageURLs = poiMatch->getImageURLs();
      if ( !imageURLs.empty() ) {
         NParam& images = rparams.addParam( NParam( 7101 ) );
         addStringAndIndex( searchHandler.getProviderName( *match ),
                            images, strings );
         addStringAndIndex( searchHandler.getProviderImageName( *match ),
                            images, strings );
         images.addUint16( imageURLs.size() );
         
         // Add the image urls
         for ( VanillaCompanyMatch::ImageURLs::const_iterator it = imageURLs.begin();
               it != imageURLs.end(); ++it ) {
            addStringAndIndex( *it, images, strings );
         }
      }

      // Add review group
      const VanillaCompanyMatch::Reviews& reviews = poiMatch->getReviews();
      if ( !reviews.empty() ) {
         NParam& reviewTable = rparams.addParam( NParam( 7102 ) );
         addStringAndIndex( searchHandler.getProviderName( *match ),
                            reviewTable, strings );
         addStringAndIndex( searchHandler.getProviderImageName( *match ),
                            reviewTable, strings );
         reviewTable.addUint16( reviews.size() );


         // Add the reviews
         for ( VanillaCompanyMatch::Reviews::const_iterator it = reviews.begin();
               it != reviews.end(); ++it ) {
            reviewTable.addByte( it->getRating() ); 
            addStringAndIndex( it->getDate(), reviewTable, strings );
            addStringAndIndex( it->getReviewer(), reviewTable, strings );
            addStringAndIndex( it->getReviewText(), reviewTable, strings );            
         }         
      }
   }   
}

}

std::auto_ptr<InfoTypeConverter> NavInfoHandler::m_infoTypeConverter(new InfoTypeConverter());

NavInfoHandler::NavInfoHandler( InterfaceParserThread* thread,
                                NavParserThreadGroup* group )
      : NavHandler( thread, group )
{
   m_expectations.push_back( 
      ParamExpectation( 1200, NParam::Uint16_array, 4, 4 ) );
   m_expectations.push_back( ParamExpectation( 1201, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1202, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1203, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1204, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1205, NParam::Uint32 ) );
   m_expectations.push_back( 
      ParamExpectation( 1206, NParam::Uint32_array, 8, 8 ) );
   m_expectations.push_back( ParamExpectation( 1207, NParam::Uint16 ) );
   m_expectations.push_back( 
      ParamExpectation( 1208, NParam::Byte_array, 2, 2 ) );
   m_expectations.push_back( ParamExpectation( 7000, NParam::String ) );
}



bool
NavInfoHandler::handleInfo( UserItem* userItem, 
                            NavRequestPacket* req, 
                            NavReplyPacket* reply )
{
   if ( !checkExpectations( req->getParamBlock(), reply ) ) {
      return false;
   }
   bool ok = true;

   // The params
   const NParamBlock& params = req->getParamBlock();
   NParamBlock& rparams = reply->getParamBlock();
   // The user
   UserUser* user = userItem->getUser();


   // Start parameter printing
   mc2log << info << "handleInfo:";

   StringTable::languageCode language = user->getLanguage();
   if ( params.getParam( 6 ) ) {
      language = NavUtil::mc2LanguageCode( 
         params.getParam( 6 )->getUint16() );
      mc2log << " Lang " << StringTable::getString( 
         StringTable::getLanguageAsStringCode( language ), 
         StringTable::ENGLISH );
   }
   LangTypes::language_t lang = ItemTypes::getLanguageCodeAsLanguageType( 
      language );
   
   // For all 1800 parameters
   vector< const NParam* > infos;
   params.getAllParams( 1800, infos );
   for ( uint32 i = 0 ; i < infos.size() ; ++i ) {
#ifdef MC2_UTF8
      bool mc2UsesLatin1 = false;
#else
      bool mc2UsesLatin1 = true;
#endif
      // That string should not be converted. It can be in utf-8 or
      // latin1.
      SearchMatch* match = SearchMatch::createMatch( 
         infos[ i ]->getString( mc2UsesLatin1 ).c_str() );
      mc2log << " ID: " << infos[ i ]->getString( mc2UsesLatin1 );
//         "a:37:1A2:0:1" );

      ItemInfoRequest* ir = new ItemInfoRequest( 
         m_thread->getNextRequestID(),
         m_thread->getTopRegionRequest());
      if ( match == NULL ) {
         mc2log << " Failed to make out id " 
                << MC2CITE( infos[ i ]->getString( mc2UsesLatin1 ) )
                << " ";  
      } else if ( ! ir->setItem( *match, lang ) ) {
         mc2log << " Not id nor coords valid for id " 
                << MC2CITE( infos[ i ]->getString(
                               m_thread->clientUsesLatin1()) ) << " ";
      }

      // Wait for the answer
      m_thread->putRequest( ir );

      if ( ! ir->replyDataOk() ) {
         mc2log << " FAILED! ";
      } else {
         // Add 1900 parameter
         NParam& p = rparams.addParam( NParam( 1900 ) );
         p.addString( infos[ i ]->getString(
                         m_thread->clientUsesLatin1()).c_str(),
                      m_thread->clientUsesLatin1());
         if ( ir->getNbrReplyItems() > 0 ) {
            uint8 nbrTuples = MIN( ir->getReplyNbrFields( 0 ), MAX_BYTE );
            p.addByte( nbrTuples );
            for ( uint32 j = 0 ; j < nbrTuples ; ++j ) {
               // Type
               p.addByte( infoTypeToAdditionalInfoType( 
                             ir->getReplyItemType( 0, j ), 
                             req->getReqVer() ) );
               // Key
               p.addString( ir->getReplyItemFieldname( 0, j ), 
                             m_thread->clientUsesLatin1() );

               // Value
               p.addString( ir-> getReplyItemValue( 0, j ),
                            m_thread->clientUsesLatin1() );
            }
         } else {
            mc2log << " ItemInfoRequest has no reply item! ";
            p.addByte( 0 );
         }
      }

      delete match;
      delete ir;
   } // End for all 1800 params

   mc2log << endl;


   return ok;
}

uint16
NavInfoHandler::infoTypeToAdditionalInfoType( ItemInfoEnums::InfoType type,
                                              byte reqVer ) {
   uint16 res = m_infoTypeConverter->infoTypeToAdditionalInfoType( type );
   if ( reqVer < 2 && type == ItemInfoEnums::image_url ) {
      res = 0x02; // url as the client can handle that
   }
   return res;
}

ItemInfoEnums::InfoType
NavInfoHandler::additionalInfoTypeToInfoType( uint16 type, byte reqVer ) {
   return m_infoTypeConverter->additionalInfoTypeToInfoType( type );
}


bool
NavInfoHandler::handleDetail( UserItem* userItem, 
                              NavRequestPacket* req, 
                              NavReplyPacket* reply )
{
   if ( !checkExpectations( req->getParamBlock(), reply ) ) {
      return false;
   }
   bool ok = true;

   // The params
   const NParamBlock& params = req->getParamBlock();
   NParamBlock& rparams = reply->getParamBlock();
   // The user
   UserUser* user = userItem->getUser();


   // Start parameter printing
   mc2log << info << "[NavInfoHandler]::handleDetail";

   StringTable::languageCode language = user->getLanguage();
   if ( params.getParam( 6 ) ) {
      language = NavUtil::mc2LanguageCode( 
         params.getParam( 6 )->getUint16() );
      mc2log << " Lang=" << StringTable::getString( 
                StringTable::getLanguageAsStringCode( language ), 
                StringTable::ENGLISH );
   }
   LangTypes::language_t lang = 
      ItemTypes::getLanguageCodeAsLanguageType( language );
   
   // For all 7000 parameters
   vector< const NParam* > infos;
   params.getAllParams( 7000, infos );
   for ( uint32 i = 0 ; i < infos.size() ; ++i ) {
#ifdef MC2_UTF8
      bool mc2UsesLatin1 = false;
#else
      bool mc2UsesLatin1 = true;
#endif
      // That string should not be converted. It can be in utf-8 or
      // latin1.
      std::auto_ptr< SearchMatch > match( SearchMatch::createMatch( 
         infos[ i ]->getString( mc2UsesLatin1 ).c_str() ) );
      mc2log << " ID=" << infos[ i ]->getString( mc2UsesLatin1 ) << endl;

      std::auto_ptr< ItemInfoRequest > itemInfoReq(   
         new ItemInfoRequest( m_thread->getNextRequestID(), 
                              m_thread->getTopRegionRequest() ) );
      if ( match.get() == NULL ) {
         mc2log << "[NavInfoHandler]::handleDetail Failed to make out id " 
                << MC2CITE( infos[ i ]->getString( mc2UsesLatin1 ) )
                << endl;
      } else if ( ! itemInfoReq->setItem( *match, lang ) ) {
         mc2log << "[NavInfoHandler]::handleDetail Not id nor coords "
            "valid for id " << MC2CITE( infos[ i ]->getString(
                                        m_thread->clientUsesLatin1()) ) << endl;
      }

      // Put the request
      m_thread->putRequest( itemInfoReq.get() );
      
      if ( itemInfoReq->replyDataOk() ) {

         // Add 7100 parameter
         NParam& p = rparams.addParam( NParam( 7100 ) );
         p.addString( infos[ i ]->getString(
                         m_thread->clientUsesLatin1()).c_str(),
                      m_thread->clientUsesLatin1());
         
         uint8 nbrTuples = MIN( itemInfoReq->getReplyNbrFields( 0 ), MAX_BYTE );
         if ( nbrTuples > 0 ) {
            for ( uint32 matchIdx = 0 ; 
                  matchIdx < itemInfoReq->getNbrReplyItems() ; matchIdx++ ) {
               SearchMatch* infoMatch = itemInfoReq->getMatches()[ matchIdx ];
               if ( itemInfoReq->getMatches()[ matchIdx ]->getName()[ 0 ] == '\0' ) {
                  // Use the input
                  infoMatch = match.get();
               } 
               
               infoMatch->mergeToSaneItemInfos( 
                  lang, m_thread->getSearchHandler().getTopRegionForMatch( infoMatch ) );               
               
               const SearchMatch::ItemInfoVector& items = infoMatch->getItemInfos();
               p.addUint16( items.size() );
               for ( SearchMatch::ItemInfoVector::const_iterator it = items.begin();
                     it != items.end() ; ++it) {
                  // Type
                  MC2String type = ItemDetailEnums::poiDetailAsString( 
                     ItemDetailEnums::poiInfoToPoiDetail( 
                        it->getInfoType() ) );
                  p.addUint16( ItemDetailEnums::poiInfoToPoiDetail(
                                  it->getInfoType() ) );

                  // Content type
                  p.addByte( ItemDetailEnums::getContentTypeForPoiInfo(
                                it->getInfoType() ) );
                  // Key
                  MC2String key = it->getKey();
                  p.addString( key, m_thread->clientUsesLatin1() );
                  
                  // Value
                  MC2String value = it->getVal();
                  p.addString( value, m_thread->clientUsesLatin1() );
               }

               if ( req->getReqVer() >= 2 ) {
                  // Add images, add reviews
                  appendResources( rparams, infoMatch, m_thread->getSearchHandler() );
               }
            }
            mc2log << info << "[NavInfoHandler]::handleDetail "
               "POIInfo OK " << itemInfoReq->getNbrReplyItems() << " infos" << endl;            
         } else {
            mc2log << "[NavInfoHandler]::handleDetail "
               "ItemInfoRequest has no reply item!" << endl;
            p.addUint16( 0 ); // No infos found.
         }
      } else {
         mc2log << error << "[NavInfoHandler]::handleDetail "
            "POI Detail Request FAILED!" << endl;
         reply->setStatusCode( NavPacket::NAV_INVALID );
         ok = false;
      }
   }
   
   return ok;
}

