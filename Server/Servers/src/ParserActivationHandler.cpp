/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ParserActivationHandler.h"

#include "UserRight.h"
#include "UserData.h"
#include "ParserThread.h"
#include "WFActivationPacket.h"
#include "Properties.h"
#include "ClientSettings.h"
#include "ParserThreadGroup.h"
#include "ServerRegionIDs.h"
#include "STLStringUtility.h"
#include "STLStrComp.h"

#include <regex.h>
#include <map>


/// The right as string to right as enum.
class ParserActivationHandler::RightStrToRightContImpl 
   : public map< MC2String, UserEnums::URType, strNoCaseCompareLess >
{};

#define RSM( a, l, s ) m_rightStrToRightCont->insert( make_pair( \
   MC2String( a ), UserEnums::URType( l, s ) ) )
#define RSMDEMO( a, l, s ) m_rightStrToRightCont->insert( make_pair( \
   MC2String( a ), UserEnums::URType( \
      UserEnums::userRightLevel( l|UserEnums::UR_DEMO ), s ) ) )
#define RSMDUAL( a, l, s ) RSM( a, l, s ) ; \
                           RSMDEMO( MC2String( "DEMO_" ) + a, l, s )

ParserActivationHandler::ParserActivationHandler( 
   ParserThread* thread,
   ParserThreadGroup* group )
      : ParserHandler( thread, group ),
        m_rightStrToRightCont( new RightStrToRightContImpl() )
{
   typedef UserEnums UE;
   // Fill m_rightStrToRightCont
   RSMDUAL( "GOLD",              UE::UR_GOLD,     UE::UR_WF );
   RSMDUAL( "SILVER",            UE::UR_SILVER,   UE::UR_WF );
   RSMDUAL( "TRIAL",             UE::UR_TRIAL,    UE::UR_WF );
   RSMDUAL( "LITHIUM",           UE::UR_LITHIUM,  UE::UR_WF );
   RSMDUAL( "MYWAYFINDER",       UE::UR_GOLD,     UE::UR_MYWAYFINDER );
   RSMDUAL( "MAPDL_PREGEN",      UE::UR_GOLD,     UE::UR_MAPDL_PREGEN );
   RSMDUAL( "MAPDL_CUSTOM",      UE::UR_GOLD,     UE::UR_MAPDL_CUSTOM );
   RSMDUAL( "XML",               UE::UR_GOLD,     UE::UR_XML );
   RSMDUAL( "TRAFFIC",           UE::UR_GOLD,     UE::UR_TRAFFIC );
   RSMDUAL( "FREE_TRAFFIC",      UE::UR_GOLD,     UE::UR_FREE_TRAFFIC );
   RSMDUAL( "SPEEDCAMS",         UE::UR_GOLD,     UE::UR_SPEEDCAM );
   RSMDUAL( "LOCATOR",           UE::UR_NO_LEVEL, UE::UR_POSITIONING );
   RSMDUAL( "FLEET",             UE::UR_NO_LEVEL, UE::UR_FLEET );
   RSMDUAL( "USE_GPS",           UE::UR_NO_LEVEL, UE::UR_USE_GPS );
   RSMDUAL( "ROUTE",             UE::UR_NO_LEVEL, UE::UR_ROUTE );
   RSMDUAL( "POI",               UE::UR_NO_LEVEL, UE::UR_POI );
   RSMDUAL( "SCDB_SPEEDCAM",     UE::UR_NO_LEVEL, UE::UR_SCDB_SPEEDCAM );
   RSMDUAL( "APP_STORE_NAV",     UE::UR_NO_LEVEL, UE::UR_APP_STORE_NAVIGATION );
}


ParserActivationHandler::~ParserActivationHandler() {
}

                                           
int
ParserActivationHandler::activateUser( UserUser* user,
                                       const ClientSetting* clientSetting,
                                       const char* activationCode,
                                       const char* phoneNumber,
                                       uint32 topRegionID,
                                       uint32 ip,
                                       const char* userAgent,
                                       const char* userInput,
                                       const char* cellularModel,
                                       uint32& ownerUIN,
                                       MC2String& serverStr,
                                       bool allowSpecialCodes )
{
   MC2String rightStr;
   ownerUIN = 0;
   serverStr = "";
   uint32 now = TimeUtility::getRealTime();
   MC2String ac; // activation code
   int ret = getActivationCode( user,
                                clientSetting,
                                activationCode,
                                ip,
                                userAgent, userInput,
                                ownerUIN,
                                serverStr,
                                allowSpecialCodes,
                                now,
                                ac, rightStr );

   if ( ret == 0 ) {
      ret = addUserRights( user, 
                           phoneNumber,
                           cellularModel,
                           topRegionID,
                           rightStr, ac, now );
   }

   if ( ret == 0 ) {
      ret = sendActivation( user, ac,
                            ip, 
                            userAgent, userInput );
      m_group->removeUserFromCache( user->getUIN() );
   }
   return ret;
}

int ParserActivationHandler::
getActivationCode( UserUser* user,
                   const ClientSetting* clientSetting,
                   const char* activationCode,
                   uint32 ip,
                   const char* userAgent,
                   const char* userInput,
                   uint32& ownerUIN,
                   MC2String& serverStr,
                   bool allowSpecialCodes,
                   uint32 now,
                   MC2String& ac, 
                   MC2String& rightStr ) 
{
   int ret = 0;
   // Client sends formating chars in code, like '-'. Strip them
   uint32 aPos = 0;
   while ( activationCode[ aPos ] != '\0' ) {
      if ( isalnum( activationCode[ aPos ] ) ) {
         ac += activationCode[ aPos ];
      }
      aPos++;
   }

   if ( !allowSpecialCodes && isSpecialActivationCode( ac ) ) {
      mc2log << info << "PT::activateUser Special (IMEI) AC "
             << "found \"" << ac << "\" Bad code returned" 
             << endl;
      ret = -5;
   }

   MC2String wholeServerStr;
   if ( ret == 0 ) {
      ret = getActivationData( ac.c_str(), rightStr, ownerUIN, 
                               wholeServerStr, true );
   }
   // Just the server part
   vector<MC2String> splitStr;
   StringUtility::tokenListToVector( splitStr, wholeServerStr, ' ',
                                     false, false );
   if ( splitStr.size() > 0 ) {
      serverStr = splitStr[ 0 ];
   } else {
      serverStr = "";
   }

   // Check if Iron client they may not upgrade to GOLD (Navigator).
   if ( ownerUIN == 0 && 
        clientSetting->getCreateLevel() == WFSubscriptionConstants::IRON &&
        (StringUtility::strstr( rightStr.c_str(), "GOLD" ) != NULL ||
         StringUtility::strstr( rightStr.c_str(), "SILVER" ) != NULL) )
   {
      mc2log << info << "PT::activateUser GOLD/SILVER AC and IRON client. "
             << "Code may not be used." 
             << endl;
      ret = -9;
   }

   if ( ret == 0 ) {
      mc2dbg4 << "serverStr " << serverStr << " wholeServerStr " 
              << wholeServerStr << endl;

      if ( !serverStr.empty() ) {
         // If Gold/Silver with access left then activate on this server!
         UserEnums::userRightLevel level = 
            UserEnums::userRightLevel( 
               UserEnums::UR_GOLD|UserEnums::UR_SILVER );
//             UserEnums::userRightLevel( 
//                ~(UserEnums::UR_TRIAL|UserEnums::UR_DEMO) );
         if ( strchr( wholeServerStr.c_str(), '*' ) == NULL &&
              ownerUIN == 0 && m_thread->checkAccessToService( 
                 user, UserEnums::UR_WF, level, 
                 MAX_UINT32/*regionID*/, true/*checkTime*/ ) )
         {
            mc2log << warn << "PT::activateUser Activation Code \""
                   << activationCode << "\" on other server \"" 
                   << serverStr << "\" But user has access left stays "
                   << "on this server and Activates here." << endl;
            // Flags AC as now on this server (but was on other before)
            // and UPDATE WFActivationPacket
            const char* instance = Properties::getProperty( 
               "WF_INSTANCE", "" );
            serverStr = MC2String( instance ) + MC2String( " (" ) + 
               serverStr + ")";
            PacketContainer* pc = m_thread->putRequest( 
               new WFActivationRequestPacket(
                  ac.c_str(), 
                  WFActivationRequestPacket::UPDATE, ip, userAgent,
                  userInput, user->getLogonID(), user->getUIN(), 
                  serverStr.c_str() ),
               MODULE_TYPE_USER );
            
            if ( pc != NULL ) {
               WFActivationReplyPacket* r = 
                  static_cast<WFActivationReplyPacket*>( pc->getPacket() );

               if ( r->getResultCode() != WFResCodes::OK ) {
                  mc2log << warn << "PT::activateUser Update activation "
                         << "code Error \""
                         << WFResCodes::activationResToString( 
                            r->getResultCode() ) << "\"" << endl;
                  if ( r->getResultCode() == 
                       WFResCodes::WRONG_ACTIVATION_CODE ) 
                  {
                     ret = -5;
                  } else {
                     ret = -1;
                  }
               } else {
                  mc2dbg2 << "PT::activateUser Updated AC " << ac << " ok."
                         << endl;
               }
            } else {
               mc2log << warn << "PT::activateUser Use activation code "
                      << " NULL WFActivationReplyPacket" << endl;
               ret = -2;
            }
            delete pc;
         } else {
            mc2log << warn << "PT::activateUser Activation Code \""
                   << activationCode << "\" on other server \"" 
                   << serverStr << "\"";
            if ( ownerUIN != 0 ) {
               mc2log << " And used by (" << ownerUIN << ")";
            }
            mc2log << endl;
            ret = -8;
            // This should only happen on dev-server, UM clears serverstr
            // if it is WF_INSTANCE. But if sharing UM and server has other
            // WF_INSTANCE value this helps by removing redirect loop.
            if ( serverStr == Properties::getProperty( "WF_INSTANCE", "" ))
            {
               ret = -7;
               mc2log << "PT::activateUser redirect to this server? " 
                      << "Returning Creation not allowed." << endl;
            }
         }
      } // End if has serverStr
   }

   if ( ret == 0 ) {
      if ( ownerUIN != 0 ) {
         mc2log << warn << "PT::activateUser Used Activation Code \""
                << activationCode << "\" by (" << ownerUIN << ")" << endl;
         ret = -4;
      }
   }
   return ret;
}

int ParserActivationHandler::
sendActivation( UserUser* user,
                const MC2String& ac,
                uint32 ip, 
                const char* userAgent, 
                const char* userInput ) {

   int ret = 0;

   // Change user
   if ( !m_thread->changeUser( user, user/*changer*/ ) ) {
      mc2log << warn << "PT::activateUser failed to update user"
             << "Activation aborted." << endl;
      ret = -1;
   } else {
      // Use activation code
      PacketContainer* pc = 
         m_thread->
         putRequest( new WFActivationRequestPacket
                     ( ac.c_str(), 
                       WFActivationRequestPacket::USE, 
                       ip, userAgent,
                       userInput, 
                       user->getLogonID(), 
                       user->getUIN(), 
                       ""/*server*/ 
                       ),
                     MODULE_TYPE_USER );
            
      if ( pc != NULL ) {
         WFActivationReplyPacket* r = 
            static_cast<WFActivationReplyPacket*>( pc->getPacket() );

         if ( r->getResultCode() != WFResCodes::OK ) {
            mc2log << warn << "PT::activateUser Use activation code "
                   << " Error \""
                   << WFResCodes::activationResToString( r->getResultCode() ) 
                   << "\"" << endl;

            if ( r->getResultCode() == 
                 WFResCodes::WRONG_ACTIVATION_CODE ) {
               ret = -5;
            } else {
               ret = -1;
            }
         } else {
            mc2log << info << "PT::activateUser Activated ok."
                   << endl;
         }
      } else {
         mc2log << warn << "PT::activateUser Use activation code "
                << " NULL WFActivationReplyPacket" << endl;
         ret = -2;
      }
      delete pc;
   } // End else if changed user ok
   return ret;
}
int
ParserActivationHandler::getActivationData( 
   const char* activationCode, MC2String& rights, uint32& ownerUIN,
   MC2String& server, 
   bool wholeServerStr )
{
   int res = 0;

   // Client sends formating chars in code, like '-'. Strip them
   MC2String ac;
   uint32 aPos = 0;
   while ( activationCode[ aPos ] != '\0' ) {
      if ( isalnum( activationCode[ aPos ] ) ) {
         ac += activationCode[ aPos ];
      }
      aPos++;
   }

   PacketContainer* pc = m_thread->putRequest( 
      new WFActivationRequestPacket(
         ac.c_str(), 
         WFActivationRequestPacket::GET_TIME, 0, "", "", "", 0, "" ),
      MODULE_TYPE_USER );

   if ( pc != NULL ) {
      WFActivationReplyPacket* r = static_cast<WFActivationReplyPacket*>(
         pc->getPacket() );
      rights = r->getRights();
      ownerUIN = r->getOwnerUIN();
      if ( wholeServerStr ) {
         server = r->getServer();
      } else {
         vector<MC2String> splitStr;
         StringUtility::tokenListToVector( splitStr, r->getServer(), ' ',
                                           false, false );
         if ( splitStr.size() > 0 ) {
            server = splitStr[ 0 ];
         } else {
            server = "";
         }
      }
      // Expand CHOOSE_IN region list if symbolic name
      if ( rights.find( "CHOOSE{" ) != MC2String::npos ) {
         vector< MC2String > matches;
         int c = (REG_EXTENDED|REG_ICASE);
         if ( StringUtility::regexp( ".*CHOOSE\\{([^}]*)\\}.*", rights.c_str(),
                                     1/*One substring*/, matches, c ) ) {
            const RegionList* rlist = m_group->getRegionIDs()->getRegionList( 
               matches[ 0 ] );
            if ( rlist != NULL ) {
               // Replace list with ids
               MC2String newRights;
               bool first = true;
               vector< const vector< uint32 >* > th;
               th.push_back( &rlist->getRegions() );
               th.push_back( &rlist->getGroups() );
               for ( uint32 l = 0 ; l < th.size() ; ++l ) {
                  for ( uint32 i = 0  ; i < th[ l ]->size() ; ++i ) {
                     if ( first ) {
                        first = false;
                     } else {
                        newRights.append( "|" );
                     }
                     STLStringUtility::uint2str( (*th[ l ])[ i ],
                                                 newRights );
                  }
               }
               
               STLStringUtility::replaceAllStrings( 
                  rights, matches[ 0 ], newRights );
            } else {
               // Check if only digits and '|'s (and white space)
               if ( !StringUtility::regexp( ".*CHOOSE\\{[0-9| \t]*\\}.*", 
                                            rights.c_str(), c ) ) {
                  mc2log << warn << "PT::getActivationData bad CHOOSE right "
                         << "possibly unknown region list? " << rights << endl;
                  res = -5;
               }
            }
         } else {
            mc2log << warn << "PT::getActivationData bad CHOOSE right "
                   << rights << endl;
            res = -5;
         }
      }
      if ( r->getResultCode() != WFResCodes::OK ) {
         mc2log << warn << "PT::getActivationData Error \""
                << WFResCodes::activationResToString( r->getResultCode() )
                << "\" for AC: " << MC2CITE( ac )<< endl;
         if ( r->getResultCode() == WFResCodes::WRONG_ACTIVATION_CODE ) {
            res = -5;
         } else {
            res = -1;
         }
      }
   } else {
      mc2log << warn << "PT::getActivationData NULL "
             << "WFActivationReplyPacket" << endl;
      res = -2;
   }
   delete pc;

   return res;
}

int 
ParserActivationHandler::addUserRights( UserUser* user,
                                        const char* phoneNumber,
                                        const char* cellularModel,
                                        uint32 topRegionID,
                                        const MC2String& rightStr,
                                        const MC2String& ac,
                                        uint32 now,
                                        uint32 newWriteId) {
   mc2dbg4 << "addUserRights[" << rightStr << "]" << endl;
   int ret = 0;

   vector<UserRight*> rights;
   MC2String firstRightServiceStr;
   bool setDeviceChangeNbr = false;
   int32 deviceChangeNbr = -1;
         
   // Parse rights into rights vector
   int pos = 0;
   // Get all records "*GOLD(12m,2097152)"
   // +GOLD(12m,2097152),SILVER(12m,2097152),SPEEDCAMS(12m,2097152)
   int res = 0;
   char serviceStr[  rightStr.size() + 1 ];
   char timeStr[     rightStr.size() + 1 ];
   char regionIDStr[ rightStr.size() + 1 ];
   char remStr[      rightStr.size() + 1 ];
   UserEnums::userRightLevel level = UserEnums::UR_GOLD;
   UserEnums::userRightService service = UserEnums::UR_WF;
   char limitChar;
   do {
      res = sscanf( rightStr.c_str() + pos, "%[^(](%[^,],%[^)])%s",
                    serviceStr, timeStr, regionIDStr, remStr );
      if ( res >= 3 ) {
         bool ok = true;
         bool add = true;
         MC2String errorStr;
         int hasLCh = 0;
         bool printACOnError = true;
         
         mc2dbg4 << "serviceStr " << serviceStr << " timeStr " << timeStr
                 << " regionIDStr " << regionIDStr << endl;
         
         // LimitChar?
         if ( serviceStr[ 0 ] == '+' || serviceStr[ 0 ] == '*' ||
              serviceStr[ 0 ] == '~' )
         {
            limitChar = serviceStr[ 0 ];
            hasLCh = 1;
         } else {
            limitChar = '\0';
         }
         
         if ( firstRightServiceStr.empty() ) {
            firstRightServiceStr = serviceStr + hasLCh;
         }
         
         // The service string to right
         RightStrToRightContImpl::const_iterator findRight = 
            m_rightStrToRightCont->find( serviceStr + hasLCh );
         if ( findRight != m_rightStrToRightCont->end() ) {
            level   = findRight->second.level();
            service = findRight->second.service();
         } else {
            errorStr = "Bad service string";
            ok = false;
         }
         
         char* tmpPtr = NULL;
         uint32 regionID = strtol( regionIDStr, &tmpPtr, 10 );
         if ( ok && 
              (strncmp( regionIDStr, "CHOOSE{", 7 ) == 0 ||
               strncmp( regionIDStr, "CHOOSE_IN{", 10 ) == 0 ) &&
              regionIDStr[ strlen( regionIDStr ) -1 ] == '}' ) {
            if ( topRegionID == MAX_UINT32 || 
                 topRegionID == MAX_INT32 ) {
               ret = -10;
               ok = false;
               errorStr = "Must choose region";
               printACOnError = false;
            } else {
               // Check to make sure topRegionID is in choices.
               if ( strstr( regionIDStr, STLStringUtility::uint2str( 
                               topRegionID ).c_str() ) != NULL ) {
                  regionID = topRegionID;
               } else {
                  ret = -10;
                  ok = false;
                  errorStr = "Must choose a region from list!";
                  printACOnError = false;
               }
            }
         } else if ( ok && (tmpPtr == NULL || *tmpPtr != '\0') ) {
            ok = false;
            errorStr = "Bad regionID string";
         }
         
         uint32 startTime = now;
         uint32 endTime = now;
         tmpPtr = NULL;
         uint32 timeNbr = strtol( timeStr, &tmpPtr, 10 );
         if ( ok && (tmpPtr == NULL || tmpPtr == timeStr) ) {
            ok = false;
            errorStr = "Bad time string";
         }
         
         if ( ok ) {
            // Check limits and merge times (set startTime).
            if ( user->getNbrOfType(UserConstants::TYPE_RIGHT ) > 0 ) {
               // Use rights
               vector<UserRight*> mrights;
               m_thread->getMatchingRights( 
                  mrights, user, UserEnums::URType( level, service ) );
               
               // For all matching rights
               uint32 endTime = 0;
               for ( uint32 i = 0 ; i < mrights.size() ; ++i ) {
                  if ( mrights[ i ]->getRegionID() != regionID ) {
                     // Not same region don't add time
                  } else if ( /*StringUtility::*/strncmp( 
                                 "AUTO:", mrights[ i ]->getOrigin(), 5 ) == 0 )
                  {
                     // AUTO not to be used after upgrade
                     mrights[ i ]->setDeleted( true );
                  } else if ( mrights[ i ]->getEndTime() >= MAX_INT32 )
                  {
                     // Unlimited nothing for this
                     mrights[ i ]->setDeleted( true );
                  } else if ( pos == 0 /*First right in AC*/ &&
                              mrights[ i ]->getEndTime() == 0 &&
                              mrights[ i ]->getUserRightType().service() 
                              != UserEnums::UR_WF )
                  {
                     // Only for first right in activation
                     // Lifetime you don't need to extend
                     // But with wf8 lifetime UR_WF are ignored so allow!
                     // TODO: If only wf8 client use
                     //       clientSetting->noGoldLifeTime() 
                     errorStr = "Extension of lifetime not allowed for ";
                     errorStr.append( serviceStr + hasLCh );
                     ok = false;
                     ret = -6; // TODO: New error code
                  } else if ( mrights[ i ]->getEndTime() > endTime ) {
                     endTime = mrights[ i ]->getEndTime();
                  }
               }
               
               if ( !ok ) {
                  // Error set already set no more checks needed
               } else if ( limitChar == '+' && mrights.size() == 0 ) {
                  errorStr = "Extension not allowed for ";
                  errorStr.append( serviceStr + hasLCh );
                  ok = false;
                  ret = -6;
               } else if ( limitChar == '*' && mrights.size() > 0 ) {
                  errorStr = "Creation not allowed, already exists, for ";
                  errorStr.append( serviceStr + hasLCh );
                  ok = false;
                  ret = -7;
               } else if ( limitChar == '~' && mrights.size() > 0 ) {
                  // Add if no such right already! 
                  // But not error if already has such right
                  // Skipp this
                  add = false;
               } // Else no limits
               
               if ( endTime > now ) {
                  startTime = endTime;
               }
            } else {
               // If zero rights, in user, use old checks (WFST) 
               // (and possibly Silver may use Gold externsions.)
               // And add old time if any (Arrghhh) but not if
               // (unlimted) trial/silver akm/auto time 
               WFSubscriptionConstants::subscriptionsTypes wfst = 
                  m_thread->getSubscriptionTypeForUser( 
                     user, UserEnums::defaultMask.service() );
               if ( wfst == MAX_BYTE ) {
                  wfst = WFSubscriptionConstants::TRIAL;
               }
               
               if ( wfst == WFSubscriptionConstants::TRIAL ) {
                  if ( limitChar == '+' && level != UserEnums::UR_TRIAL)
                  {
                     errorStr = "Extension not allowed for ";
                     errorStr.append( serviceStr + hasLCh );
                     ok = false;
                     ret = -6;
                  } // Only TRIAL extension allowed
                  // All creations allowed
                  // Don't use time from this
               } else if ( wfst == WFSubscriptionConstants::SILVER ) {
                  if ( limitChar == '+' ) {
                     if ( level == UserEnums::UR_GOLD ) {
                        errorStr = "Extension not allowed for ";
                        errorStr.append( serviceStr + hasLCh );
                        ok = false;
                        ret = -6;
                     } // Else (TRIAL|SILVER) extension ok
                  } else if ( limitChar == '*' ) {
                     if ( level != UserEnums::UR_GOLD ) {
                        errorStr = "Creation not allowed, already exists, for ";
                        errorStr.append( serviceStr + hasLCh );
                        ok = false;
                        ret = -7;
                     }
                  } // Else no limits
                  // Silver Time
                  if ( level == UserEnums::UR_SILVER || 
                       (level == UserEnums::UR_GOLD &&
                        service != UserEnums::UR_WF) ) 
                  {
                     // Add time from this, but not unlimited
                     uint32 endTime = 0;
                     for ( uint32 i = 0 ; 
                           i < user->getNbrOfType( 
                              UserConstants::TYPE_REGION_ACCESS ) ; i++)
                     {
                        UserRegionAccess* r = 
                           static_cast< UserRegionAccess* > ( 
                              user->getElementOfType( 
                                 i, UserConstants::TYPE_REGION_ACCESS));
                        if ( r->getEndTime() >= MAX_INT32 ) {
                           // Unlimited time skipped
                        } else if ( r->getRegionID() == regionID ) {
                           if ( r->getEndTime() > endTime ) {
                              endTime = r->getEndTime();
                           }
                        }
                     } // End for all regions
                     if ( endTime > now ) {
                        // addcompatright up to this time from now
                        MC2String originStr( "FILLER: " );
                        originStr.append( ac );
                        rights.push_back( 
                           new UserRight( 
                              newWriteId, now, 
                              UserEnums::URType( level, service ),
                              regionID, now, endTime, false,
                              originStr.c_str() ) );
                        // Set to start when previous ends
                        startTime = endTime;
                     }
                  } // End if level is silver then check for addtime
                  
               } else if ( wfst == WFSubscriptionConstants::GOLD ) {
                  if ( limitChar == '+' ) {
                     if ( level == UserEnums::UR_TRIAL ) {
                        errorStr = "Extension not allowed for ";
                        errorStr.append( serviceStr + hasLCh );
                        ok = false;
                        ret = -6;
                     } // Else (SILVER|GOLD) extension ok
                  } else if ( limitChar == '*' ) {
                     errorStr = "Creation not allowed, already "
                        "exists, for ";
                     errorStr.append( serviceStr + hasLCh );
                     ok = false;
                     ret = -7;
                  } // Else no limits
                  // Gold Time
                  if ( level == UserEnums::UR_GOLD ||
                       level == UserEnums::UR_SILVER )
                  {
                     // Add time from this, but not unlimited
                     uint32 endTime = 0;
                     for ( uint32 i = 0 ; 
                           i < user->getNbrOfType( 
                              UserConstants::TYPE_REGION_ACCESS ) ; i++)
                     {
                        UserRegionAccess* r = 
                           static_cast< UserRegionAccess* > ( 
                              user->getElementOfType( 
                                 i, UserConstants::TYPE_REGION_ACCESS));
                        if ( r->getEndTime() >= MAX_INT32 ) {
                           // Unlimited time skipped
                        } else if ( r->getRegionID() == regionID ) {
                           if ( r->getEndTime() > endTime ) {
                              endTime = r->getEndTime();
                           }
                        }
                     } // End for all regions
                     
                     if ( endTime > now ) {
                        // addcompatright up to this time from now
                        MC2String originStr( "FILLER: " );
                        originStr.append( ac );
                        rights.push_back( 
                           new UserRight( 
                              newWriteId, now, 
                              UserEnums::URType( level, service ),
                              regionID, now, endTime, false,
                              originStr.c_str() ) );
                        // Set to start when previous ends
                        startTime = endTime;
                     }
                  } // End if level is gold then check for addtime
               } // End if gold         
            } // End use old WFST/region access
         }

         if ( ok && add ) {
            bool endTimeSetToLessThanStartTime = false;
            if ( *tmpPtr == 'm' ) {
               endTime = StringUtility::addMonths( startTime, timeNbr );
            } else if ( *tmpPtr == 'd' ) {
               endTime = startTime + timeNbr*24*60*60;
            } else if ( *tmpPtr == 'y' ) {
               endTime = StringUtility::addYears( startTime, timeNbr );
            } else if (  *tmpPtr == '\0' && timeNbr == 0 ) {
               endTime = 0; // Lifetime
               endTimeSetToLessThanStartTime = true;
            } else {
               errorStr = "Bad time unit";
               ok = false;
            }
            if ( ok ) {
               // Overflow check
               if ( endTime >= startTime ) {
                  // Ok
                  if ( endTime > MAX_INT32 ) {// Not more than MAX_INT32
                     mc2log << error << "PT::activateUser endTime "
                            << endTime << "(" << startTime << "+" 
                            << timeNbr << *tmpPtr << ") too large, "
                            << "setting " << "MAX_INT32" << endl;
                     endTime = MAX_INT32;
                  }
               } else if ( !endTimeSetToLessThanStartTime ) {
                  mc2log << error << "PT::activateUser endTime "
                         << endTime << "(" << startTime << "+" 
                         << timeNbr << *tmpPtr << ") will overflow, "
                         << "setting " << "MAX_INT32" << endl;
                  endTime = MAX_INT32;
               }
            } // End if ok (to do overflow check)
         } // End if ok and add (to make endtime)
         
         if ( ok && add ) {
            // Add the right
            MC2String originStr( "AC: " );
            originStr.append( ac );
            
            rights.push_back( 
               new UserRight( newWriteId, now, 
                              UserEnums::URType( level, service ),
                              regionID, startTime, endTime, false,
                              originStr.c_str() ) );   
         } // If if ok and add then add right
            
         if ( ok ) {
            // Skipp past ','
            MC2String::size_type findPos = rightStr.find( ",", pos );
            if ( findPos != MC2String::npos ) {
               pos = findPos + 1;
            } else {
               ok = false;
               errorStr = "Coudn't find ',' in part!";
               res = 0;
            }
            
            // Then skipp past ')'
            if ( ok ) {
               findPos = rightStr.find( ")", pos );
               if ( findPos != MC2String::npos ) {
                  pos = findPos + 1;
               } else {
                  ok = false;
                  errorStr = "Coudn't find ')' in part!";
                  res = 0;
               }
            }
            
            // Then possibly an ','
            if ( ok && rightStr[ pos ] == ',' ) {
               pos++;
            }   
         }

         if ( !ok ) {
            mc2log << error << "PT::activateUser Error: \""
                   << errorStr << "\" for Activation code \""
                   << ac << "\"";
            if ( printACOnError ) {
               mc2log << "\" Rights str \""
                      << rightStr << "\" Error at \"" 
                      << (rightStr.c_str() + pos) << "\"";
            }
            mc2log << endl;
            res = 0;
            if ( ret == 0 ) {
               // Set to error if not set
               ret = -1;
            }
         }
         
      } else if ( (res = sscanf( rightStr.c_str() + pos, 
                                 "%[^(](%[^)])%s",
                                 serviceStr, timeStr, remStr ) ) >= 2 )
      {
         // Special, special for you only.
         if ( strcmp( serviceStr, "DEVICE_CHANGE" ) == 0 ) {
            bool ok = true;
            char* tmpPtr = NULL;
            int32 nbr = strtol( timeStr, &tmpPtr, 10 );
            if ( ok && (tmpPtr == NULL || tmpPtr == timeStr) ) {
               ok = false;
               mc2log << error << "PT::activateUser Error: \""
                      << "Bad special time string\" for Activation "
                      << "code \"" << ac << "\" Rights str \""
                      << rightStr << "\" Error at \"" 
                      << (rightStr.c_str() + pos) << "\"" << endl;
               ret = -1;
            }
            
            if ( ok ) {
               setDeviceChangeNbr = true;
               int32 newNbr = nbr;
               if ( nbr == -1 ) {
                  // OK set it to -1
               } else {
                  // Add?
                  if ( user->getDeviceChanges() >= 0 ) {
                     newNbr += user->getDeviceChanges();
                  }
               }
               deviceChangeNbr = newNbr;
            }
         } else if ( strcmp( serviceStr, "VERSION_LOCK" ) == 0 ) {
            // VERSION_LOCK(6000) -> 6000 regionID! endTime = 0
            bool ok = true;
            char* tmpPtr = NULL;
            int32 nbr = strtol( timeStr, &tmpPtr, 10 );
            if ( ok && (tmpPtr == NULL || tmpPtr == timeStr) ) {
               ok = false;
               mc2log << error << "PT::activateUser Error: \""
                      << "Bad VERSION_LOCK time string\" for "
                      << "Activation code \"" << ac 
                      << "\" Rights str \"" << rightStr 
                      << "\" Error at \"" << (rightStr.c_str() + pos)
                      << "\"" << endl;
               ret = -1;
            }

            if ( ok && nbr ) {
               MC2String originStr( "AC: " );
               originStr.append( ac );
               rights.push_back( 
                  new UserRight( 
                     newWriteId, now, 
                     UserEnums::URType( UserEnums::UR_NO_LEVEL, 
                                        UserEnums::UR_VERSION_LOCK ),
                     nbr, now, 0, false,
                     originStr.c_str() ) );
            }
         } else {
            ret = -1;
            mc2log << error << "PT::activateUser Special Error: "
                   <<" for Activation code \""
                   << ac << "\" Rights str \""
                   << rightStr << "\" Error at \"" 
                   << (rightStr.c_str() + pos) << "\"" << endl;
         }
      } else {
         mc2log << error << "PT::activateUser "
                << "Bad Rights string for Activation code \""
                << ac << "\" Rights str \""
                << rightStr << "\" Error at \"" 
                << (rightStr.c_str() + pos) << "\"" << endl;
         ret = -1;
      }
   } while( res >= 3 && pos < int32(rightStr.size()) );
   

   // Fixup cellular
   if ( ret == 0 && phoneNumber[ 0 ] != '\0' ) {
      // Check if already present in this user.
      if ( m_thread->getUsersCellular( user, phoneNumber ) != NULL ) {
         // Check model
         UserCellular* cellular = m_thread->getUsersCellular( 
            user, phoneNumber );
         if ( strcmp( cellular->getModel()->getName(), 
                      cellularModel ) != 0 )
         {
            // Change Model
            CellularPhoneModel* model = new CellularPhoneModel(
               *cellular->getModel() );
            model->setName( cellularModel );
            cellular->setModel( model ); // Owns model now
         }
      } else {
         // Other has it?
         UserItem* userItem = NULL;
         if ( m_thread->getUserFromCellularNumber( 
                 phoneNumber, userItem, true ) )
         {
            // Ok check it
            if ( userItem != NULL && 
                 userItem->getUIN() != user->getUIN() )
            {
               // In other user, just nag don't do anything
               mc2log << warn << "PT::activateUser phonenumber "
                      << "already used " << MC2CITE( phoneNumber )
                      << " by " << userItem->getUser()->getLogonID()
                      << "(" << userItem->getUIN() << ")" << endl;
               
            } // No one has phonenumber or this user has it -> ok
            if ( userItem == NULL ) {
               // Add it
               UserCellular* cellular = new UserCellular( 0 );
               cellular->setPhoneNumber( phoneNumber );
               CellularPhoneModel* model = new CellularPhoneModel();
               model->setManufacturer( "UNKNOWN" );
               model->setName( cellularModel );
               cellular->setModel( model );
               user->addElement( cellular );
            }
         } else {
            mc2log << info << "PT::activateUser "
                   << "getUserFromCellularNumber( " << phoneNumber 
                   << ") failed " << endl;
            ret = -1;
         }
         m_thread->releaseUserItem( userItem );
      } // End else if phonenumber not in user   
   } // End if ret == 0 for cellular fixup
   
   
   // Add to user
   if ( ret == 0 ) {
      // If old WFST/access on GOLD but Silver AC now add GOLD filler.
      if ( (firstRightServiceStr == "SILVER" ||
            firstRightServiceStr == "DEMO_SILVER") &&
           user->getNbrOfType(UserConstants::TYPE_RIGHT ) == 0 &&
           m_thread->getSubscriptionTypeForUser( 
              user, UserEnums::defaultMask.service() ) == 
           WFSubscriptionConstants::GOLD )
      {
         // Add time from this, but not unlimited
         for ( uint32 i = 0 ; 
               i < user->getNbrOfType( 
                  UserConstants::TYPE_REGION_ACCESS ) ; i++)
         {
            UserRegionAccess* r = 
               static_cast< UserRegionAccess* > ( 
                  user->getElementOfType( 
                     i, UserConstants::TYPE_REGION_ACCESS));
            if ( r->getEndTime() >= MAX_INT32 ) {
               // Unlimited time skipped
            } else if ( r->getEndTime() > now ) {
               // addcompatright up to this time from now
               MC2String originStr( "FILLER: For old GOLD" );
               rights.push_back( 
                  new UserRight( 
                     newWriteId, now, 
                     UserEnums::URType( UserEnums::UR_GOLD, 
                                        UserEnums::UR_WF ),
                     r->getRegionID(), now, r->getEndTime(), false,
                     originStr.c_str() ) );
            }
         } // End for all regions   
      } // End if Old GOLD WFST user and Silver AC now
      for ( uint32 i = 0 ; i < rights.size() ; ++i ) {
         user->addElement( rights[ i ] );
      }

      if ( setDeviceChangeNbr ) {
         user->setDeviceChanges( deviceChangeNbr );
      }
      
      // Add this update to operatorComment
      MC2String opCom = user->getOperatorComment();
      opCom.append( "; ");
      opCom.append( firstRightServiceStr ); // First right in rights
      opCom.append( " ");
      char dateStr[ 11 ];
      char timeStr[ 9 ];
      StringUtility::makeDateStr( now, dateStr, timeStr );
      opCom.append( dateStr );
      opCom.append( " " );
      opCom.append( timeStr );
      opCom.append( " " );
      opCom.append( ac );
      user->setOperatorComment( opCom.c_str() );
      
       
   } else {
      // Delete all rights
      for ( uint32 i = 0 ; i < rights.size() ; ++i ) {
         delete rights[ i ];
      }
   }
   
   return ret;
}

bool
ParserActivationHandler::isSpecialActivationCode( 
   const MC2String& ac ) const 
{
   // If like IMEI[0-9]{15,19} then don't allow it
   char tmpIMEIStr[ ac.size() + 1 ];
   tmpIMEIStr[ 0 ] = '\0';
   if ( sscanf( ac.c_str(), "IMEI%[0-9]", tmpIMEIStr ) == 1 &&
        strlen( tmpIMEIStr ) >= 15 && strlen( tmpIMEIStr ) <= 19 )
   {
      return true;
   }

   return false;
}

