/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ClientRights.h"

#include "UserRight.h"
#include "UserData.h"
#include "TimeUtility.h"

namespace ClientRights {

#define URM( a ) userRight.getUserRightType().levelAndServiceMatch( \
   UserEnums::URType( userRight.getUserRightType().level(), a ) )
#define URMM( a ) userRight.getUserRightType().levelAndServiceMatchMask( \
   UserEnums::URType( userRight.getUserRightType().level(), a ) )

bool canUseRight( Rights right, const UserRight& userRight, uint32 now ) {

   // This function does not return the complete truth, since 
   // WF_GOLD etc. isn't used. Add that too.

   // if deleted or the right is accessed now, then we can not use right now.
   if ( userRight.isDeleted() || ! userRight.checkAccessAt( now ) ) {
      return false;
   }

   bool useRight = false;
   switch ( right ) {
      case CONNECT_GPS:
         useRight = URM( UserEnums::UR_USE_GPS );
         break;
      case ROUTE:
         useRight = URM( UserEnums::UR_ROUTE );
         break;
      case POSITIONING:
         useRight = URMM( UserEnums::UR_POSITIONING );
         break;
      case FLEET:
         useRight = URMM( UserEnums::UR_FLEET );
         break;
      case TRAFFIC:
         useRight = URMM( UserEnums::UR_ALL_TRAFFIC_MASK );
         break;
      case NBR_RIGHTS:
         MC2_ASSERT( false );
         break;
         // NOTE!: If you are adding more here dont forget to fill 
         //        the list in getClientRights
   }

   return useRight;
}

bool canUseRight( Rights right, const UserRight& userRight ) {
   uint32 now = TimeUtility::getRealTime();

   return canUseRight( right, userRight, now );
}

ClientRights
getClientRights( const UserUser& user ) { 
   // the rights we want to check against
   const Rights rights[] = { 
      ROUTE,
      CONNECT_GPS,
      POSITIONING,
      FLEET,
      TRAFFIC,
   };

   CHECK_ARRAY_SIZE( rights, NBR_RIGHTS );


   ClientRights clientRights;

   uint32 now = TimeUtility::getRealTime();
   UserUser::constUserElRange_t iteratorRange = 
      user.getElementRange( UserConstants::TYPE_RIGHT );
   UserUser::userElVect_t::const_iterator rightIt = iteratorRange.first;
   // for each client rights;
   //   check and see if we can use it with current user right.
   //   and if we can use it then add to clientrights set
   for ( ; rightIt != iteratorRange.second; ++rightIt ) {

      UserRight& userRight = static_cast<UserRight&>( *(*rightIt) );
      for ( uint32 i = 0; i < sizeof( rights ) / sizeof( rights[ 0 ] ); ++i ){
         if ( canUseRight( rights[ i ], userRight, now ) ) {
            clientRights.insert( rights[ i ] );
         }
      }
   }

   return clientRights;
}

}
