/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLServerUtility.h"

#include <stdlib.h>
#include <string.h>

namespace XMLServerUtility {

bool readSizeValue( const MC2String& str, uint32& value ) {
   bool ok = true;
   if ( str == "inf" ) {
      value = MAX_UINT32;
   } else {
      char* tmpChrp = NULL;
      uint32 tmpNbr = strtoul( str.c_str(), &tmpChrp, 10 );
      if ( tmpChrp == NULL || tmpChrp[ 0 ] != '\0' ) {
         mc2log << warn 
                << "XMLParserThread::readSizeValue "
                << "str is not a number " << str << endl;
         ok = false;
      } else {
         value = tmpNbr;
      }
   }

   return ok;
}

void 
printWithLineNumbers( ostream& out, const char* str,
                      const char* preStr, 
                      int nbrWidth ) 
{
   int lineNbr = 1;
   const char* pos = str;
   const char* ch = strchr( str, '\n' );
   int lineLength = 0;

   if ( nbrWidth < 0 ) {
      // Calculate a good nbrWidth
      int nbrLines = 0;
      const char* ch = strchr( str, '\n' );
      while ( ch != NULL ) { 
         ++nbrLines; 
         ch = strchr( ch+1, '\n' );
      }
      nbrWidth = 1;
      if ( nbrLines > 0 ) {
         nbrWidth += int( ::log10( nbrLines ) );
      }
   }

   while ( ch != NULL ) {
      out << preStr << setw( nbrWidth ) << lineNbr << ": ";
      lineLength = ch - pos;
      if ( *ch == '\r' ) {
         --lineLength;
      }
      out.write( pos, lineLength );
      out << endl;
      ++lineNbr;
      pos = ch;
      pos += 1; // Skip newline
      ch = strchr( pos, '\n' );
   }

   // Print last line
   out << preStr << setw( nbrWidth ) << lineNbr << ": "
       << pos << endl;
}

const char* 
routeTurnImageTypeToString( UserConstants::RouteTurnImageType type ) {
   switch ( type ) {
   case UserConstants::ROUTE_TURN_IMAGE_TYPE_MAP_IMAGE :
      return "map";
   case UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM :
      return "pictogram";
   case UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_1 :
      return "pictogram_set_1";
   case UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_2 :
      return "pictogram_set_2";
   case UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_3 :
      return "pictogram_set_3";
   case UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_4 :
      return "pictogram_set_4";
   case UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_5 :
      return "pictogram_set_5";
   case UserConstants::ROUTE_TURN_IMAGE_TYPE_NBR :
      mc2log << error << "XMLParserThread::routeTurnImageTypeToString"
             << " got ROUTE_TURN_IMAGE_TYPE_NBR!" << endl;
      MC2_ASSERT( false );
      return "map";
   }

   // Should never reach this, tell to implemnt new choice above
   MC2_ASSERT( false );
   return "map";
}

UserConstants::RouteTurnImageType 
routeTurnImageTypeFromString( const char* str, 
                              UserConstants::RouteTurnImageType defaultType ) {
   if ( strcmp( str, "map" ) == 0 ) {
      return UserConstants::ROUTE_TURN_IMAGE_TYPE_MAP_IMAGE;
   } else if ( strcmp( str, "pictogram" ) == 0 ) {
      return UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM;
   } else if ( strcmp( str, "pictogram_set_1" ) == 0 ) {
      return UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_1;
   } else if ( strcmp( str, "pictogram_set_2" ) == 0 ) {
      return UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_2;
   } else if ( strcmp( str, "pictogram_set_3" ) == 0 ) {
      return UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_3;
   } else if ( strcmp( str, "pictogram_set_4" ) == 0 ) {
      return UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_4;
   } else if ( strcmp( str, "pictogram_set_5" ) == 0 ) {
      return UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_5;
   } else {
      return UserConstants::ROUTE_TURN_IMAGE_TYPE_NBR;
   }
}

const char* 
routeturnTypeToString( ExpandedRouteItem::routeturn_t turn, 
                       bool endOfRoad ) {
   switch ( turn ) {
      case ExpandedRouteItem::UNDEFINED :
         return "no_turn";
      case ExpandedRouteItem::NO_TURN :
         return "no_turn";
      case ExpandedRouteItem::START :
         return "start";
      case ExpandedRouteItem::START_WITH_U_TURN :
         return "start_with_u_turn";
      case ExpandedRouteItem::FINALLY :
         return "finally";
      case ExpandedRouteItem::LEFT :
         if ( endOfRoad ) {
            return "endofroad_left_turn";
         } else {
            return "left";
         }
      case ExpandedRouteItem::AHEAD :
         return "ahead";
      case ExpandedRouteItem::RIGHT :
         if ( endOfRoad ) {
            return "endofroad_right_turn";
         } else {
            return "right";
         }
      case ExpandedRouteItem::U_TURN :
         return "u_turn";
      case ExpandedRouteItem::FOLLOW_ROAD :
         return "followroad";
      case ExpandedRouteItem::ENTER_ROUNDABOUT :
         return "enter_roundabout";
      case ExpandedRouteItem::EXIT_ROUNDABOUT :
         return "exit_roundabout";
      case ExpandedRouteItem::AHEAD_ROUNDABOUT :
         return "ahead_roundabout";
      case ExpandedRouteItem::RIGHT_ROUNDABOUT :
         return "right_roundabout";
      case ExpandedRouteItem::LEFT_ROUNDABOUT :
         return "left_roundabout";
      case ExpandedRouteItem::U_TURN_ROUNDABOUT :
         return "u_turn_roundabout";
      case ExpandedRouteItem::ON_RAMP :
         return "on_ramp";
      case ExpandedRouteItem::OFF_RAMP :
         return "off_ramp";
      case ExpandedRouteItem::OFF_RAMP_LEFT :
         return "off_ramp_left";
      case ExpandedRouteItem::OFF_RAMP_RIGHT :
         return "off_ramp_right";
      case ExpandedRouteItem::ENTER_BUS :
         return "enter_bus";
      case ExpandedRouteItem::EXIT_BUS :
         return "exit_bus";
      case ExpandedRouteItem::CHANGE_BUS :
         return "change_bus";
      case ExpandedRouteItem::KEEP_RIGHT :
         return "keep_right";
      case ExpandedRouteItem::KEEP_LEFT :
         return "keep_left";
      case ExpandedRouteItem::ENTER_FERRY :
         return "enter_ferry";
      case ExpandedRouteItem::EXIT_FERRY :
         return "exit_ferry";
      case ExpandedRouteItem::CHANGE_FERRY :
         return "change_ferry";
      case ExpandedRouteItem::PARK_AND_WALK :
         return "park_car";
       case ExpandedRouteItem::ON_MAIN :
         return "on_main";
      case ExpandedRouteItem::OFF_MAIN :
         return "off_main";
      case ExpandedRouteItem::STATE_CHANGE:
         return "";
      break;
   }

   // Unreachable code
   return "";
}

UserConstants::RouteImageType 
overviewImageTypeFromString( const char* str ) {
   if ( strcmp( str, "image" ) == 0 ) {
      return UserConstants::ROUTEIMAGETYPE_IMAGE;
   } else if ( strcmp( str, "applet" ) == 0 ) {
      return UserConstants::ROUTEIMAGETYPE_JAVA_APPLET;
   } else if ( strcmp( str, "none" ) == 0 ) {
      return UserConstants::ROUTEIMAGETYPE_NONE;
   } else {
      return UserConstants::ROUTEIMAGETYPE_NBR;
   }
}


const char* 
overviewImageTypeToString( UserConstants::RouteImageType type ) {
   switch ( type ) {
      case UserConstants::ROUTEIMAGETYPE_IMAGE :
         return "image";
      case UserConstants::ROUTEIMAGETYPE_JAVA_APPLET :
         return "applet";
      case UserConstants::ROUTEIMAGETYPE_NONE :
         return "none";
      case UserConstants::ROUTEIMAGETYPE_NBR :
         mc2log << error << "XMLParserThread::overviewImageTypeToString "
                << "got ROUTEIMAGETYPE_NBR!" << endl;
         MC2_ASSERT( false );
         return "image";
   }

   // Should never reach this, tell to implemnt new choice above
   MC2_ASSERT( false );
   return "image";
}


UserConstants::transactionBased_t 
transactionBasedTypeFromString( const char* str ) {
   if ( strcmp( str, "no_transactions" ) == 0 ) {
      return UserConstants::NO_TRANSACTIONS;
   } else if ( strcmp( str, "transactions" ) == 0 ) {
      return UserConstants::TRANSACTIONS;
   } else if ( strcmp( str, "transaction_days" ) == 0 ) {
      return UserConstants::TRANSACTION_DAYS;
   } else {
      return UserConstants::NBR_TRANSACTION_T;
   }
}


const char* transactionBasedTypeToString( UserConstants::
                                          transactionBased_t transType ) {
   switch ( transType ) {
      case UserConstants::NO_TRANSACTIONS:
         return "no_transactions";
      case UserConstants::TRANSACTIONS:
         return "transactions";
      case UserConstants::TRANSACTION_DAYS:
         return "transaction_days";
      case UserConstants::NBR_TRANSACTION_T:
         mc2log << error << "XMLParserThread::"
                << "transactionBasedTypeToString got NBR_TRANSACTION_T!"
                << endl;
      MC2_ASSERT( false );
      return "no_transactions";
   }

   // Should never reach this, tell to implemnt new choice above
   MC2_ASSERT( false );
   return "no_transactions";
}

UserIDKey::idKey_t idKeyTypeFromString( const char* str ) {
   if ( strcmp( str, "account" ) == 0 ) {
      return UserIDKey::account_id_key;
   } else if ( strcmp( str, "hardware" ) == 0 ) {
      return UserIDKey::hardware_id_key;
   } else if ( strcmp( str, "hardware_and_time" ) == 0 ) {
      return UserIDKey::hardware_id_and_time;
   } else if ( strcmp( str, "service_id_and_time" ) == 0 ) {
      return UserIDKey::service_id_and_time;
   } else if ( strcmp( str, "client_type_and_time" ) == 0 ) {
      return UserIDKey::client_type_and_time;
   } else {
      return UserIDKey::account_id_key;
   }
}


const char* idKeyTypeToString( UserIDKey::idKey_t type ) {
   switch ( type ) {
      case UserIDKey::account_id_key :
         return "account";
      case UserIDKey::hardware_id_key :
         return "hardware";
      case UserIDKey::hardware_id_and_time :
         return "hardware_and_time";
      case UserIDKey::service_id_and_time :
         return "service_id_and_time";
      case UserIDKey::client_type_and_time :
         return "client_type_and_time";
   }

   // Should never reach this, tell to implement new choice above
   MC2_ASSERT( false );
   return "account";
}

}
