/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UserEnums.h"
#include "UserEnumNames.h"
#include "Packet.h"
#include "STLStringUtility.h"
#include "BitUtility.h"


WFSubscriptionConstants::subscriptionsTypes 
UserEnums::URType::getLevelAsWFST() const {
   WFSubscriptionConstants::subscriptionsTypes type = 
      WFSubscriptionConstants::subscriptionsTypes( MAX_BYTE );

   uint32 l = level() & ALL_LEVEL_MASK;
   if ( l != 0 ) {
      if ( l == uint32(UR_TRIAL) ) {
         return WFSubscriptionConstants::TRIAL;
      } else if ( l == UR_SILVER ) {
         return WFSubscriptionConstants::SILVER;
      } else if ( l == UR_GOLD ) {
         return WFSubscriptionConstants::GOLD;
      } else if ( l == UR_IRON ) {
         return WFSubscriptionConstants::IRON;
      } else if ( l == UR_LITHIUM ) {
         return WFSubscriptionConstants::LITHIUM;
      } else if ( l == UR_CESIUM ) {
         return WFSubscriptionConstants::CESIUM;
      } else if ( l == UR_WOLFRAM ) {
         return WFSubscriptionConstants::WOLFRAM;
      } else {
         return type;
      }
   } else {
      return type;
   }
}


const UserEnums::URType UserEnums::defaultMask = UserEnums::URType(
   ALL_LEVEL_MASK, 
   userRightService( (UR_WF|UR_MYWAYFINDER|UR_XML) ) );

const UserEnums::URType UserEnums::allMask = UserEnums::URType::getAllMask();

const UserEnums::URType UserEnums::emptyMask = UserEnums::URType();

UserEnums::userRightLevel 
UserEnums::wfstAsLevel( 
   WFSubscriptionConstants::subscriptionsTypes wfst ) 
{
   switch( wfst ) {
      case WFSubscriptionConstants::TRIAL :
         return UR_TRIAL;
      case WFSubscriptionConstants::SILVER :
         return UR_SILVER;
      case WFSubscriptionConstants::GOLD :
         return UR_GOLD;
      case WFSubscriptionConstants::IRON :
         return UR_IRON;
      case WFSubscriptionConstants::LITHIUM :
         return UR_LITHIUM;
      case WFSubscriptionConstants::CESIUM :
         return UR_CESIUM;
      case WFSubscriptionConstants::WOLFRAM:
         return UR_WOLFRAM;
      default:
         return UR_TRIAL;
   }

   return UR_TRIAL;
}


MC2String&
UserEnums::URType::phpPrint( MC2String& tmp ) const {
   tmp.clear();
   return STLStringUtility::int2str( getAsInt(), tmp );
}


bool 
UserEnums::URType::load(const Packet* p, int& pos) {
   p->incReadLong( pos, first );
   p->incReadLong( pos, second );
   return true;
}


bool 
UserEnums::URType::save(Packet* p, int &pos) const {
   p->incWriteLong( pos, first );
   p->incWriteLong( pos, second ); 
   return true;  
}


int
UserEnums::URType::getSizeInPacket() const {
   return 8;
}


MC2String
UserEnums::getName( const UserEnums::URType& type,
                    LangTypes::language_t lang )
{
   return UserEnumNames::getName( type, lang );
}



bool
UserEnums::isBitRight( userRightService r ) {
   return BitUtility::nbrBits( r ) == 1;
}


bool
UserEnums::isTrafficRight( userRightService r ) {
   switch ( r ) {
      case UR_TRAFFIC :
      case UR_FREE_TRAFFIC :
      case UR_WF_TRAFFIC_INFO_TEST :
         return true;
      default:
         return false;
   }
}
