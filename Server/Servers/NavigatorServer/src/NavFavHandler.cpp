/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavFavHandler.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "UserData.h"
#include "NavUtil.h"
#include "NavUserHelp.h"
#include "UserFavoritesRequest.h"
#include "MC2Coordinate.h"
#include "NavInfoHandler.h"
#include "STLStringUtility.h"
#include "ParserUserHandler.h"
#include "BitUtility.h"
#include "TimeUtility.h"

NavFavHandler::NavFavHandler( InterfaceParserThread* thread,
                                NavParserThreadGroup* group,
                              NavUserHelp* userHelp )
      : NavHandler( thread, group ),
        m_userHelp( userHelp )
{
   m_expectations.push_back( 
      ParamExpectation( 1600, NParam::Int32_array, 16, 16 ) );
   m_expectations.push_back(
      ParamExpectation( 1601, NParam::Int32_array, 8, 8 ) );
   m_expectations.push_back( ParamExpectation( 1602, NParam::Uint16 ) );
   m_expectations.push_back( ParamExpectation( 1603, NParam::Uint16 ) );
   m_expectations.push_back( ParamExpectation( 1604, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 1605, NParam::String ) );
   m_expectations.push_back( 
      ParamExpectation( 1606, NParam::Uint16_array, 4, 4 ) );
   m_expectations.push_back( 
      ParamExpectation( 1607, NParam::Uint16_array, 4, 4 ) );
   m_expectations.push_back( ParamExpectation( 1608, NParam::Byte ) );
   m_expectations.push_back( 
      ParamExpectation( 1609, NParam::Byte_array, 10, 10 ) );
   m_expectations.push_back( 
      ParamExpectation( 1610, NParam::Byte_array, 6, 6 ) );
}


NavFavHandler::~NavFavHandler() {
}


bool
NavFavHandler::handleFav( UserItem* userItem, 
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
   uint32 startTime = TimeUtility::getCurrentTime();


   // Start parameter printing
   mc2log << info << "handleFav:";

   mc2log << " Ver " << int(req->getReqVer());

   StringTable::languageCode language = user->getLanguage();
   if ( params.getParam( 6 ) ) {
      language = NavUtil::mc2LanguageCode( 
         params.getParam( 6 )->getUint16() );
      mc2log << " Lang " << StringTable::getString( 
         StringTable::getLanguageAsStringCode( language ), 
         StringTable::ENGLISH );
   }
   

   bool noSync = false;
   UserFavoritesRequest* ureq = new UserFavoritesRequest( 
      m_thread->getNextRequestID(), userItem->getUIN() );   

   if ( params.getParam( 4800 ) ) {
      noSync = BitUtility::getBit( params.getParam( 4800 )->getUint32(), 0 );
      // Bit 1 fetchAutoDest not supported anymore 20051011
      mc2log << " noSync " << BP(noSync);
   }
   if ( noSync ) {
      ureq->setNoSync();
   }

   if ( params.getParam( 4801 ) ) {
      const NParam* pa = params.getParam( 4801 );
      mc2log << " nbrSyncIds " << (pa->getLength() / 4);
      for ( uint16 i = 0 ; i < pa->getLength() / 4 ; ++i ) {
         ureq->addFavSync( pa->getUint32Array( i ) );
         mc2dbg8 << " ID " << pa->getUint32Array( i );
      }
   }

   if ( params.getParam( 4802 ) ) {
      const NParam* pd = params.getParam( 4802 );
      mc2log << " nbrDelIds " << (pd->getLength() / 4);
      for ( uint16 i = 0 ; i < pd->getLength() / 4 ; ++i ) {
         ureq->addFavSync( pd->getUint32Array( i ) );
         ureq->addFavDelete( pd->getUint32Array( i ) );
         mc2dbg8 << " ID " << pd->getUint32Array( i );
      }
   }

   if ( params.getParam( 4803 ) ) {
      const NParam* pa = params.getParam( 4803 );
      uint32 pos = 0;
      uint32 nbrAdd = 0;
      while ( pos < pa->getLength() ) {
         pos += 4; // ID
         int32 navLat = pa->getUint32( pos ); pos += 4;
         int32 navLon  = pa->getUint32( pos ); pos += 4;
         MC2Coordinate coord( Nav2Coordinate( navLat, navLon ) );
         MC2String name = 
            pa->incGetString( m_thread->clientUsesLatin1(), pos ); 
         MC2String shortName = 
            pa->incGetString( m_thread->clientUsesLatin1(), pos ); 
         MC2String description =
            pa->incGetString( m_thread->clientUsesLatin1(), pos ); 
         MC2String category = 
            pa->incGetString( m_thread->clientUsesLatin1(), pos ); 
         MC2String mapIconName =
            pa->incGetString( m_thread->clientUsesLatin1(),
                              pos );
         UserFavorite::InfoVect v;
         if ( req->getReqVer() > 1 ) { // reqVer 2+
            // Read infos
            uint16 nbrInfos = pa->getUint16( pos ); pos += 2;

            ItemInfoEnums::InfoType type;
            MC2String key;
            MC2String val;
            for ( uint32 i = 0 ; i < nbrInfos ; ++i ) {
               type = NavInfoHandler::additionalInfoTypeToInfoType(
                  pa->incGetUint16( pos ), req->getReqVer() > 3 ? 2 : 1 ); 
               key = pa->incGetString( m_thread->clientUsesLatin1(), pos );
               val = pa->incGetString( m_thread->clientUsesLatin1(), pos );
               v.push_back( ItemInfoEntry( key, val, type ) );
            }
         }

         ureq->addFavNew( 
            new UserFavorite( 0, coord.lat, coord.lon, name.c_str(),
                              shortName.c_str(), description.c_str(),
                              category.c_str(), mapIconName.c_str(), 
                              v ) );
         mc2dbg8 << " Coord " << coord << " name " << name << " shortName "
                 << shortName << " description " << description 
                 << " category " << category << " mapIconName "
                 << mapIconName;
         mc2dbg8 << " " << v.size() << " infos ";
         for ( UserFavorite::InfoVect::const_iterator it = v.begin() ; 
               it != v.end() ; ++it )
         {
            mc2dbg8 << " " << ItemInfoEnums::infoTypeToString( 
               LangTypes::english, (*it).getInfoType() ) << ":" 
                    << (*it).getKey() << ":" << (*it).getVal() << ";";
         }
         ++nbrAdd;
      }
      mc2log << " nbrAddFavs " << nbrAdd;
   }   

   mc2log << endl;

   // XXX: Check if backup user
   // if ( !rd.backupUser ) { ... } else { status = NOT_ON_BACKUP_SERVER }

   m_thread->putRequest( ureq );

   if ( ureq->getStatus() == StringTable::OK ) {
      mc2log << info << "handleFav: Reply";
      const UserFavorite* fav = NULL;

      fav = ureq->getAddFav();
      if ( fav != NULL ) {
         NParam pa( 4900 );
         uint32 nbrAdd = 0;
         while ( fav != NULL ) {
            // Check if not to add this favorite
            bool locFav = false;
            uint32 decLen = strlen( fav->getDescription() ) + 1;
            char locUserID[ decLen ];
            char locPIN[ decLen ];
            if ( sscanf( fav->getDescription(), "LOC:%[^:\n]:%[^:\n]", 
                         locUserID, locPIN ) == 2 ) 
            {
               // The temporary Locator favorites.
               locFav = true;
            }
            if ( req->getReqVer() == 1 && 
                 (fav->hasInfoType( ItemInfoEnums::special_flag) || 
                  locFav)  )
            {
               mc2dbg8 << " Skipping ID " << fav->getID();
               fav = ureq->getAddFav();
               continue;
            }

            if ( pa.getLength() + fav->getSize() + 4
                 > MAX_UINT16 ) 
            {
               rparams.addParam( pa );
               pa.clear();
            }
            pa.addUint32( fav->getID() );
            Nav2Coordinate coord( MC2Coordinate( 
                                     fav->getLat(), fav->getLon() ) );
            pa.addInt32( coord.nav2lat );
            pa.addInt32( coord.nav2lon );
            pa.addString( fav->getName(), m_thread->clientUsesLatin1() );
            pa.addString( fav->getShortName(), 
                          m_thread->clientUsesLatin1() );
            MC2String desc( fav->getDescription() );
            if ( req->getReqVer() == 1 ) { 
               // Add infos to description and cut at 255 bytes.
               for ( uint16 i = 0 ; i < fav->getInfos().size() ; ++i ) {
                  if ( !desc.empty() ) {
                     desc.append( ", " );
                  }
                  desc.append( fav->getInfos()[ i ].getKey() );
                  desc.append( ": " );
                  desc.append( fav->getInfos()[ i ].getVal() );
               }
               if ( desc.size() > 255 ) {
                  desc.erase( 255 );
               }
            }
            pa.addString( desc,
                          m_thread->clientUsesLatin1() );
            pa.addString( fav->getCategory(),
                          m_thread->clientUsesLatin1() );
            pa.addString( fav->getMapIconName(), 
                          m_thread->clientUsesLatin1() );
            if ( req->getReqVer() > 1 ) { // reqVer 2+
               // Write infos
               uint16 size = MIN( fav->getInfos().size() + (locFav ? 1 : 0),
                                  MAX_UINT16 );
               pa.addUint16( size );
               uint16 i = 0;
               // The temporary Locator favorites.
               if ( locFav ) {
                  i = 1;
                  pa.addUint16( ItemInfoEnums::special_flag );
                  pa.addString( "", m_thread->clientUsesLatin1() );
                  pa.addString( "", m_thread->clientUsesLatin1() );
               }
               // Add fav infos
               for ( /**/ ; i < size ; ++i ) {
                  pa.addUint16( fav->getInfos()[ i ].getInfoType() );
                  pa.addString( fav->getInfos()[ i ].getKey(), 
                                m_thread->clientUsesLatin1() );
                  pa.addString( fav->getInfos()[ i ].getVal(), 
                                m_thread->clientUsesLatin1() );
               }
            }

            mc2dbg8<< " ID " << fav->getID() << " coord (" << fav->getLat()
                   << "," << fav->getLon() << ")"
                   << " name " << fav->getName() << " description "
                   << desc << " category " 
                   << fav->getCategory() << " shortname " 
                   << fav->getShortName() << " mapiconname " 
                   << fav->getMapIconName();
            if ( req->getReqVer() > 1 ) { // reqVer 2+
               mc2dbg8 << " " << fav->getInfos().size() << " infos ";
               for ( UserFavorite::InfoVect::const_iterator it = 
                        fav->getInfos().begin() ; 
                     it != fav->getInfos().end() ; ++it )
               {
                  mc2dbg8 << ItemInfoEnums::infoTypeToString( 
                     LangTypes::english, (*it).getInfoType() ) << ":" 
                              << (*it).getKey() << ":" << (*it).getVal()
                              << ";";
               }
            }
            fav = ureq->getAddFav();
            ++nbrAdd;
         }
         mc2log << " nbrFavAdd " << nbrAdd;
         rparams.addParam( pa );
      }

      fav = ureq->getDelFav();
      if ( fav != NULL ) {
         NParam& pd = rparams.addParam( NParam( 4901 ) );
         uint32 nbrDel = 0;
         while ( fav != NULL ) {
            mc2dbg8 << " " << fav->getID();
            pd.addUint32( fav->getID() );
            fav = ureq->getDelFav();
            ++nbrDel;
         }
         mc2log << " nbrFavDel " << nbrDel;
      }

      if ( req->getReqVer() > 2 ) { // 3+
         MC2String crcStr;
         STLStringUtility::uint2strAsHex( m_thread->getUserHandler()->
            getUserFavCRC( user->getUIN() ), crcStr );
         mc2log << " crc " << crcStr;
         rparams.addParam( NParam( 4903, crcStr, 
                                   m_thread->clientUsesLatin1() ) );
      }

      mc2log << endl;
      
   } else {
      mc2log << warn << "handleFav: UserFavoritesRequest failed ";
      if ( ureq->getStatus() == StringTable::UNKNOWN && 
           (TimeUtility::getCurrentTime() - startTime) > 3000 )
      {
         reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
         mc2log << "Timeout";
      } else {
         reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         mc2log << "Error";
      }
      mc2log << endl;
   }


   delete ureq;

   return ok;
}
