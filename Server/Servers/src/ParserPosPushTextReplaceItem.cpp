/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ParserPosPushTextReplaceItem.h"
#include "SendDataPacket.h"
#include "PacketContainer.h"
#include "ParserThread.h"
#include "DeleteHelpers.h" // STLUtility
#include "StringUtility.h"
#include "UserConstants.h"
#include "UserData.h"
#include "STLStringUtility.h"
#include "UserTrackElement.h"
#include <regex.h>


ParserPosPushTextReplaceItem::ParserPosPushTextReplaceItem( 
   ParserThread* thread,
   ParserThreadGroup* group,
   const URL& peer,
   vector< UserUser* >& users,
   const char* postitionRequestData,
   GfxUtility::coordianteRepresentations coordSys,
   const char* expectedReplyExp,
   bool keepPositions )
      : ParserPosPushItem( thread, group ),
        m_peer( peer ), m_users( users ), 
        m_postitionRequestData( postitionRequestData ),
        m_coordSys( coordSys ), m_expectedReplyExp( expectedReplyExp ),
        m_keepPositions( keepPositions )
{
}

ParserPosPushTextReplaceItem::~ParserPosPushTextReplaceItem() {
   STLUtility::deleteValues( m_users );
}

int
ParserPosPushTextReplaceItem::positionsReceived( 
   const vector< UserTrackElement* >& p,
   const UserItem* user,
   const ClientSetting* clientSetting )
{
   const UserUser* u = user->getUser();
   int res = 0;
   bool matching = false;
   for ( uint32 i = 0 ; i < m_users.size() && !matching ; ++i ) {
      const UserUser* userPattern = m_users[ i ];
      for ( uint32 i = 0 ; i < UserConstants::USER_NBRFIELDS ; i++ ) {
         if ( userPattern->changed( UserConstants::UserDataField( i ) ) ) {
            char pattern[ 4096 ];
            char userField[ 4096 ];
            pattern[ 0 ] = '\0';
            userField[ 0 ] = '\0';
            userPattern->printValue( 
               pattern, UserConstants::UserDataField( i ), false );
            u->printValue( 
               userField, UserConstants::UserDataField( i ), false );

            int c = (REG_EXTENDED|REG_ICASE);
            if ( !(*pattern == '\0' && *userField == '\0') &&
                 StringUtility::regexp( pattern, userField, c , 0 ) ) {
               matching = true;
               break;
            }
         }
      } // For all fields in user pattern
   } // For all user patterns

   if ( matching ) {
      // [UIN], [USERID]
      // [BEGINCOORD]
      //  [DIST], [SPEED], [TIME], [LAT], [LON], [ANGLE]
      // [ENDCOORD]
      // Currently not supported is, [ALT]
      MC2String UINStr( STLStringUtility::uint2str( u->getUIN() ) );
      MC2String userIDStr( u->getLogonID() );
      MC2String reqData( m_postitionRequestData );
      const MC2String coordBeginStr( "[BEGINCOORD]" );
      MC2String::size_type coordBeginPos = reqData.find( coordBeginStr );
      const MC2String coordEndStr( "[ENDCOORD]" );
      MC2String::size_type coordEndPos = reqData.find( coordEndStr );
      if ( coordBeginPos == MC2String::npos || 
           coordEndPos   == MC2String::npos ) {
         mc2log << warn << "PPPTRI BEGINCOORD or ENDCOORD missing! "
                << reqData << endl;
         return -1;
      }
      const uint32 coordDataLen = coordEndPos + coordEndStr.size() 
         - coordBeginPos;
      MC2String coordDataStr( reqData, 
                              coordBeginPos + coordBeginStr.size(),
                              coordDataLen - coordBeginStr.size() -
                              coordEndStr.size() );
      MC2String allCoordData;
      
      for ( uint32 i = 0 ; i < p.size() ; ++i ) {
         const UserTrackElement *t = p[ i ];
         MC2String distStr( STLStringUtility::int2str( t->getDist() ) );
         MC2String speedStr( STLStringUtility::int2str( t->getSpeed() ) );
         MC2String timeStr( STLStringUtility::uint2str( t->getTime() ) );
         const uint32 coordStrLen = 50;
         char latStr[ coordStrLen ];
         GfxUtility::printLatLon( latStr, t->getLat(), true, 
                                  u->getLanguage(), m_coordSys );
         char lonStr[ coordStrLen ];
         GfxUtility::printLatLon( lonStr, t->getLon(), false, 
                                  u->getLanguage(), m_coordSys );
         MC2String angleStr( STLStringUtility::int2str( t->getHeading() ) );

         MC2String creqData( coordDataStr );
         STLStringUtility::replaceString( creqData, "[UIN]", UINStr );
         STLStringUtility::replaceString( creqData, "[USERID]", userIDStr );
         STLStringUtility::replaceString( creqData, "[DIST]", distStr );
         STLStringUtility::replaceString( creqData, "[SPEED]", speedStr );
         STLStringUtility::replaceString( creqData, "[TIME]", timeStr );
         STLStringUtility::replaceString( creqData, "[LAT]", latStr );
         STLStringUtility::replaceString( creqData, "[LON]", lonStr );
         STLStringUtility::replaceString( creqData, "[ANGLE]", angleStr );
         allCoordData.append( creqData );
      }

      reqData.replace( coordBeginPos, coordDataLen, allCoordData );
      mc2dbg2 << "Sending " << reqData << endl << " To " << m_peer << endl;

      PacketContainer* cont = m_thread->putRequest( 
         new SendDataRequestPacket(
            m_peer,
            reinterpret_cast< const byte* > ( reqData.c_str() ),
            reqData.size(), m_expectedReplyExp.c_str(),
            SendDataRequestPacket::http ),
         MODULE_TYPE_COMMUNICATION );

      if ( cont != NULL ) {
         if ( static_cast<ReplyPacket*>( cont->getPacket() )->getStatus() 
              == StringTable::OK ) {
            if ( !m_keepPositions ) {
               res = 1;
            }
         } else {
               res = -1;
         }
      } else {
         res = -2;
      }
      
      delete cont;
   } else { // End if matching this
      res = -3;
   }

   return res;
}
