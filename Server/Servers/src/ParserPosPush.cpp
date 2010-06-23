/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ParserPosPush.h"
#include "SendDataPacket.h"
#include "PacketContainer.h"
#include "ParserThread.h"
#include "ParserPosPushTextReplaceItem.h"
#include "UserData.h"
#include "DeleteHelpers.h" // STLUtility
#include "ParserExternalAuth.h"

ParserPosPush::ParserPosPush( ParserThread* thread,
                              ParserThreadGroup* group )
      : ParserHandler( thread, group )
{

   // Example position push
#if 0
   // What user has to match to get this positions forwarded.
   vector< UserUser* > users;
   users.clear();
   users.push_back( new UserUser( 0 ) );
   // Any regexp and field in UserUser can be used.
   users.back()->setBrand( "^SendPositionBrand$" );
   // The request to send
   MC2String httpReq( "POST /positionsupload HTTP/1.1\r\n" );
   // The data to send, the strings inside brackets are replaced by their
   // values, e.g. [LON] is replaced by the longitude.
   // Possible replaces are:
   // [UIN], [USERID]
   // [BEGINCOORD]
   //  [DIST], [SPEED], [TIME], [LAT], [LON], [ANGLE]
   // [ENDCOORD]
   // The substring between [BEGINCOORD] and [ENDCOORD] are repeated
   // for each coordinate.
   httpReq.append( 
      "\r\n"
      "Connection: Close\r\n"
      "\r\n"
      "[BEGINCOORD]userID=[USERID]&distance=[DIST]&speed=[SPEED]&"
      "time=[TIME]&lat=[LAT]&lon=[LON]&angle=[ANGLE]\r\n[ENDCOORD]" );

   m_pushItems.push_back( 
      new ParserPosPushTextReplaceItem( 
         thread, group,
         URL( "https://" ), // The host to send to
         users, 
         httpReq.c_str(),
         GfxUtility::wgs84Deg, // Coordinate format
         "^HTTP/1\\.[01] 200.*OK", // What a successfull reply looks like
         true ) );
#endif

}

ParserPosPush::~ParserPosPush() {
   STLUtility::deleteValues( m_pushItems );
}

int
ParserPosPush::positionsReceived( 
   const vector< UserTrackElement* >& p,
   const UserItem* user,
   const ClientSetting* clientSetting )
{
   int res = 0;

   for ( uint32 i = 0 ; i < m_pushItems.size() ; ++i ) {
      int tempRes = m_pushItems[ i ]->positionsReceived( 
         p, user, clientSetting );
      if ( tempRes != -3 ) {
         res = tempRes;
         break;
      }
   }

   return res;
}
