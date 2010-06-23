/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavUserHelp.h"
#include "InterfaceParserThread.h"
#include "NavParserThreadGroup.h"
#include "SinglePacketRequest.h"
#include "AddUserTrackRequest.h"
#include "ClientSettings.h"
#include "NParamBlock.h"
#include "TopRegionRequest.h"
#include "WFResCodes.h"
#include "SendEmailPacket.h"
#include "MimeMessage.h"
#include "Properties.h"
#include "UserPacket.h"
#include "ParserUserHandler.h"
#include "UserRight.h"
#include "UserData.h"
#include "UserLicenceKey.h"
#include "MC2CRC32.h"
#include "NavAuthHandler.h"
#include "NavPacket.h"
#include "ParserTokenHandler.h"
#include "ParserActivationHandler.h"
#include "NamedServerLists.h"
#include "isabBoxSession.h"
#include "ServerRegionIDs.h"
#include "Utility.h"

#include <iomanip>
#include <algorithm>


NavUserHelp::NavUserHelp( InterfaceParserThread* thread,
                          NavParserThreadGroup* group )
      : NavHandler( thread, group )
{
}


uint32 
NavUserHelp::getUIN( const char* identification ) {
   mc2dbg8 << "GET THE UIN" << endl;
   uint32 uin = MAX_UINT32;
   bool timeOut = false;

   if( (identification != NULL) && (strlen(identification) > 0) ){
      // Identification might be ok, send to UM
      char* tmpStr = NULL;
      uint32 uinNbr = strtoul( identification, &tmpStr, 10 );
      if ( (uin == MAX_UINT32 || uin == 0) && !timeOut &&
           identification[ 0 ] != '\0' &&
           tmpStr != NULL && tmpStr[ 0 ] == '\0' && 
           uinNbr != MAX_UINT32 ) 
      {
         // Is it an UIN?
         mc2dbg8 << "Is it an UIN??? " << endl;
         UserItem* userItem = NULL;
         if ( m_thread->getUser( uinNbr, userItem, true ) ) {
            if ( userItem != NULL ) {
               // User!
               uin = uinNbr;
            } else {
               uin = 0;
            }
         } else {
            mc2log << warn << "getUIN getUser " << "timeout." << endl;
            timeOut = true;
         }
         m_thread->releaseUserItem( userItem );
      }

      if ( (uin == MAX_UINT32 || uin == 0) && !timeOut ) {
         // User-logonID?
         mc2dbg8 << "Is it a logonID??? " << endl;
         UserUser* user = new UserUser( MAX_UINT32 );
         user->setLogonID( identification );
         FindUserRequestPacket* fUP = new FindUserRequestPacket( 0,
                                                                 0,
                                                                 user );
         
         // Wait for the answer 
         PacketContainer* answerCont = m_thread->putRequest(
            fUP, MODULE_TYPE_USER );

         if( answerCont != NULL ){
            FindUserReplyPacket* reply =
               static_cast<FindUserReplyPacket*>( 
                  answerCont->getPacket() );
            if( reply != NULL &&
                reply->getStatus() == StringTable::OK  &&
                reply->getNbrUsers() == 1 ){
               uint32* uins = reply->getUINs();
               uin = uins[ 0 ];
               delete [] uins;
               mc2dbg8 << "getUIN" << " UIN: " << uin << endl;
            } else {
               mc2dbg8 << "Unknown user??? " << endl;
               uin = 0;
            }
         } else {
            mc2log << warn << "getUIN getUser " << "timeout." << endl;
            timeOut = true;
         }
         // Cleanup
         delete user;
         delete answerCont;
      }

      // Phonenumbers not supported anymore 20051107
//       if ( (uin == MAX_UINT32 || uin == 0) && !timeOut ) {
//          mc2dbg8 << "Is it a phone number??? " << endl;
//          UserCellular userCell( MAX_UINT32 );
//          userCell.setPhoneNumber( identification );
//          FindUserRequestPacket* fUP = 
//             new FindUserRequestPacket( 0, 0, &userCell );
//          // Wait for the answer 
//          PacketContainer* answerCont = m_thread->putRequest(
//             fUP, MODULE_TYPE_USER );

//          if( answerCont != NULL ){
//             FindUserReplyPacket* reply = static_cast<FindUserReplyPacket*>(
//                answerCont->getPacket() );
//             if( reply != NULL &&
//                 reply->getStatus() == StringTable::OK  &&
//                 reply->getNbrUsers() == 1 ){
//                uint32* uins = reply->getUINs();
//                uin = uins[ 0 ];
//                delete [] uins;
//                mc2dbg8 << "getUIN UIN: " << uin << endl;
//             } else{
//                mc2dbg8 << "Unknown user??? " << endl;
//                uin = 0;
//             }
//          } else{
//             mc2log << warn << "getUIN getUser " << "timeout." << endl;
//             timeOut = true;
//          }

//          delete answerCont;
//       }

   } else{
      mc2log << warn << "getUIN identification string invalid: " 
             << identification << endl;
   }
   return uin;
}


void 
NavUserHelp::storeUserPosition( uint32 UIN, MC2Coordinate pos,
                              const char* source )
{
   storeUserPosition( UIN, pos.lat, pos.lon, 0, "", source );
}


void 
NavUserHelp::storeUserPosition( uint32 UIN, int32 lat, int32 lon, 
                                uint32 navID, const char* identification,
                                const char* source )
{
   mc2dbg2 << "storeUserPosition UIN " << UIN << " lat " << lat
           << " lon " << lon << " navID " << navID << " identification " 
           << identification << endl;
   if ( m_group->getNoUserTrack() ) {
      mc2dbg2 << "storeUserPosition not storing as no "
              << "user tracking is set." << endl;
      return;
   }
   // Soure is 16 chars long in db.
   char sourceDB[ 17 ];
   if ( source[ 0 ] != '\0' ) {
      StringUtility::strlcpy( sourceDB, source, 16 );
   } else {
      sprintf( sourceDB, "Nav: %u", navID );
   }

   AddUserTrackRequest* request = new AddUserTrackRequest(
      m_thread->getNextRequestID(), UIN );
   request->addUserTrackElement( 
      new UserTrackElement( lat, lon, MAX_UINT32, MAX_UINT16, MAX_UINT16,
                            TimeUtility::getRealTime(), sourceDB ) );
         
   // Wait for the answer 
   m_thread->putRequest( request );
   PacketContainer* answerCont = request->getAnswer();

   if( request->getStatus() == StringTable::OK ) {
      mc2dbg2 << "storeUserPosition stored ok." << endl;
   } else {
      mc2log << warn << "NavUserHelp::storeUserPosition failed status: "
             << StringTable::getString( request->getStatus(), 
                                        StringTable::ENGLISH ) << endl;
   }

   delete answerCont;
   delete request;
}

ostream& 
NavUserHelp::printLicence( ostream& out, const uint8* key, uint32 len )
{
   const bool hexPrint = 
      (key + len == std::find_if( key, key + len, ptr_fun( ::isprint ) ));

   if ( hexPrint ) {
      out << hex << setfill( '0' );
      out.setf( ios::uppercase );
      for ( uint32 i = 0 ; i < len ; ++i ) {
         out << setw( 2 ) << int( key[ i ] );
      }
      out << dec << endl;
      out.unsetf( ios::uppercase ); // Reset to default
   } else {
      for ( uint32 i = 0 ; i < len ; ++i ) {
         out << key[ i ];
      }
   }
   return out;
}

ostream&
NavUserHelp::printLicence( ostream& out, uint16 paramID,
                           NParamBlock& params )
{
   const NParam* p = params.getParam( paramID );
   if ( p != NULL ) {
      printLicence( out, p->getBuff(), p->getLength() );
   }
   return out;
}

MC2String& 
NavUserHelp::printLicence( MC2String& out, 
                           const uint8* key, uint32 len )
{
   const bool hexPrint = 
      (key + len == std::find_if( key, key + len, ptr_fun( ::isprint ) ));
   char tmpStr[ 20 ];
      
   if ( hexPrint ) {
      for ( uint32 i = 0 ; i < len ; ++i ) {
         sprintf( tmpStr, "%.2X", key[ i ] );
         out.append( tmpStr );
      }
   } else {
      out.insert( out.size(), reinterpret_cast<const char*>( key ), len );
   }
   return out;
}

ostream& 
NavUserHelp::printLicence( ostream& out, 
                           const UserLicenceKey& licence ) {
   bool hexPrint = false;
   const byte* key = licence.getLicenceKey();
   const uint32 len = licence.getLicenceLength();
   for ( uint32 i = 0 ; i < len ; ++i ) {
      if ( !isprint( key[ i ] ) ) {
         hexPrint = true;
         break;
      }
   }
   out << licence.getKeyType() << ":";
   if ( hexPrint ) {
      out << hex << setfill( '0' );
      out.setf( ios::uppercase );
      for ( uint32 i = 0 ; i < len ; ++i ) {
         out << setw( 2 ) << int( key[ i ] );
      }
      out << dec << endl;
      out.unsetf( ios::uppercase ); // Reset to default
   } else {
      for ( uint32 i = 0 ; i < len ; ++i ) {
         out << key[ i ];
      }
   }
   return out;
}


MC2String&
NavUserHelp::printLicence( MC2String& out, 
                           const UserLicenceKey& licence ) {
   bool hexPrint = false;
   char tmpStr[ 20 ];
   const byte* key = licence.getLicenceKey();
   const uint32 len = licence.getLicenceLength();
   for ( uint32 i = 0 ; i < len ; ++i ) {
      if ( !isprint( key[ i ] ) ) {
         hexPrint = true;
         break;
      }
   }
   out.append( licence.getKeyType() );
   out.append( ":" );
   if ( hexPrint ) {
      for ( uint32 i = 0 ; i < len ; ++i ) {
         sprintf( tmpStr, "%.2X", key[ i ] );
         out.append( tmpStr );
      }
   } else {
         out.insert( out.size(), reinterpret_cast<const char*>( key ), len );
   }
   return out;
}


ostream& 
NavUserHelp::printLicences( ostream& out, 
                            const UserLicenceKeyVect& hwKeys,
                            const MC2String& prefix )
{
   out << prefix;
   for ( UserLicenceKeyVect::const_iterator it = hwKeys.begin() ;
         it != hwKeys.end() ; ++it ) {
      if ( it != hwKeys.begin() ) {
         out << ", ";
      }
      printLicence( out, *it );
   } // End for all hwdKeys
   return out;   
}

MC2String&
NavUserHelp::printLicences( MC2String& out, 
                            const UserLicenceKeyVect& hwKeys,
                            const MC2String& prefix )
{
   out.append( prefix );
   for ( UserLicenceKeyVect::const_iterator it = hwKeys.begin() ;
         it != hwKeys.end() ; ++it ) {
      if ( it != hwKeys.begin() ) {
         out.append( ", " );
      }
      printLicence( out, *it );
   } // End for all hwdKeys
   return out;
}

int
NavUserHelp::checkUserLicence( UserItem* userItem, const byte* key, 
                               uint32 keyLength, const char* userName,
                               const char* userPassword,
                               UserItem*& changeToUserItem,
                               bool noaddLicence )
{
   UserUser* user = new UserUser( *userItem->getUser() );
   int res = 0;
   bool otherUserHasBK = false;

   // If user has no licence then add it!
   if ( user->getNbrOfType( UserConstants::TYPE_LICENCE_KEY ) == 0 &&
        user->mayChangeDevice() )
   {
      // Check if some other user has it!
      // User with lower WFSuscr.Type then move BK
      // to this user.
      bool addLicence = true;
      UserLicenceKey* userKey = 
         new UserLicenceKey( MAX_UINT32 );
      userKey->setLicence( key, keyLength );
      userKey->setKeyType( ParserUserHandler::imeiType );
      UserItem* licUserItem = NULL;
      uint32 nbrUsers = 0;
      if ( m_thread->getUserFromUserElement( 
              userKey, licUserItem, nbrUsers, true ) )
      {
         if ( nbrUsers == 1 && licUserItem == NULL ) {
            // DB comm error
            mc2log << warn << "checkUserLicence: get user with "
                   << "same licence failed. Licence ";
            NavUserHelp::printLicence( mc2log, key, keyLength );
            mc2log << endl;
            // Don't know if ok to add
            addLicence = false;
            res = 3;
         } else if ( nbrUsers == 0 ) {
            // Ok to add to this user
         } else if ( nbrUsers == 1 ) {
            UserUser* licUser = new UserUser( *licUserItem->getUser() );
            // Check if this user is "better"
            byte otherWayfinderType = m_thread->getSubscriptionTypeForUser(
               licUser, UserEnums::UR_WF, false );
            if ( otherWayfinderType == MAX_BYTE ) {
               otherWayfinderType = WFSubscriptionConstants::GOLD;
            }
            byte userWFType = m_thread->getSubscriptionTypeForUser(
               userItem->getUser(), UserEnums::UR_WF, false );
            if ( userWFType == MAX_BYTE ) {
               userWFType = WFSubscriptionConstants::GOLD;
            }
            
            if ( otherWayfinderType < userWFType ) {
               // This is better
               // Remove from other
               for ( uint32 i = 0 ; i < licUser->getNbrOfType( 
                        UserConstants::TYPE_LICENCE_KEY ) ; ++i )
               {
                  UserLicenceKey* licence = static_cast< UserLicenceKey* >(
                     licUser->getElementOfType( 
                        i, UserConstants::TYPE_LICENCE_KEY ) );
                  if ( keyLength == licence->getLicenceLength() &&
                       memcmp( key, licence->getLicenceKey(), 
                               keyLength ) == 0 )
                  {
                     // Remove it
                     licence->remove();
                  } // End If same BK as this
               } // End for all BKs in licUserItem
               // Send changes
               if ( m_thread->changeUser( licUser, NULL/*changer*/ ) ) {
                  mc2log << info << "checkUserLicence: Moved BK from " 
                         << licUser->getLogonID() << "(" 
                         << licUser->getUIN() << ")" << " to " 
                         << user->getLogonID() << "(" << user->getUIN()
                         << ")" << " licence: ";
                  NavUserHelp::printLicence( mc2log, key, keyLength );
                  mc2log << endl;
               } else {
                  mc2log << warn << "checkUserLicence: failed to move BK "
                         << "from " << licUser->getLogonID() << "(" 
                         << licUser->getUIN() << ")" << " to " 
                         << user->getLogonID() << "(" << user->getUIN()
                         << ")" << " licence: ";
                  NavUserHelp::printLicence( mc2log, key, keyLength );
                  mc2log << endl;
                  // Didn't remove don't add
                  addLicence = false;
                  res = 3;
               }
            } else {
               // Other has the key for the phone change to that user!
               mc2log << info << "checkUserLicence: other user has BK and "
                      << "better or equal WFSuscr.type. Changing to that "
                      << "user. other " 
                      << licUser->getLogonID() << "(" << licUser->getUIN()
                      << "), WF " 
                      << WFSubscriptionConstants::subscriptionTypeToString(
                         WFSubscriptionConstants::subscriptionsTypes( 
                            otherWayfinderType ) ) << ", " << "this " 
                      << user ->getLogonID() << "(" << user->getUIN() 
                      << "), WF " 
                      << WFSubscriptionConstants::subscriptionTypeToString(
                         WFSubscriptionConstants::subscriptionsTypes(
                            userWFType ) )
                      << " licence: ";
               NavUserHelp::printLicence( mc2log, key, keyLength );
               mc2log << endl;
               delete user;
               user = new UserUser( *licUser );
               // Set new user to use
               changeToUserItem = new UserItem( *licUserItem );
               addLicence = false; // No need licUser has it allready
            }

            delete licUser; // Newed above (to edit it)
         } else { 
            // Not 0 or 1 user then it is 2+
            mc2log << warn << "checkUserLicence: More "
                   << "than one user, " << nbrUsers << ", with same "
                   << "licence wont add to this user, licence: ";
            NavUserHelp::printLicence( mc2log, key, keyLength );
            mc2log << endl;
            addLicence = false;
            otherUserHasBK = true; // Actually many has it
         }
      } else {
         // Database connection problem
         mc2log << warn << "checkUserLicence: DB check for users with "
                << "same licence failed." << endl;
         res = 3;
         // Don't know if ok to add
         addLicence = false;
      }
      // Gotten above
      m_thread->releaseUserItem( licUserItem );
                        
      if ( addLicence && !noaddLicence ) {
         UserLicenceKey* newEl = new UserLicenceKey( 0 );
         newEl->setLicence( key, keyLength );
         user->addElement( newEl );
         user->useDeviceChange();
         // Save changes in database
         if ( m_thread->changeUser( user, NULL/*changer*/ ) ) {
            mc2log << info << "checkUserLicence: added licence to user " 
                   << user->getLogonID() << "(" << userItem->getUIN() 
                   << ")" << " licence ";
            NavUserHelp::printLicence( mc2log, key, keyLength );
            mc2log << endl;
         } else {
            mc2log << info << "checkUserLicence: failed to add licence to "
                   << "user " << user->getLogonID() << "(" 
                   << userItem->getUIN() << ")" 
                   << " will try again next session licence ";
            NavUserHelp::printLicence( mc2log, key, keyLength );
            mc2log << endl;
         }
      } // End if addLicence and not noaddLicence

      delete userKey;
   } // End if user has zero UserLicenceKeys


   // Now check if licence matches
   bool licenceMatch = false;
   UserLicenceKey* licence = NULL;
   // The developer key to use for developers
   uint32 devKeyLength = 15;
   byte devKey[] = 
      {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
   bool hasDevKey = false;
   
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_LICENCE_KEY ) ; ++i )
   {
      licence = static_cast< UserLicenceKey* > ( 
         user->getElementOfType( i, UserConstants::TYPE_LICENCE_KEY ) );
      mc2dbg8 << "UserLicence: " << endl;
#ifdef DEBUG_LEVEL_8
      NavUserHelp::printLicence( mc2dbg8, licence->getLicenceKey(), 
                                 licence->getLicenceLength() );
#endif
      mc2dbg8 << endl;
      if ( (keyLength == licence->getLicenceLength() &&
            memcmp( key, licence->getLicenceKey(), keyLength ) == 0) )
      {
         licenceMatch = true;
         break;
      }
      // Dev key check
      if ( (licence->getLicenceLength() == devKeyLength &&
            memcmp( licence->getLicenceKey(), devKey, devKeyLength ) == 0 )
           )
      {
         hasDevKey = true;
      }
   } // End for all user's licences

   if ( !licenceMatch && res == 0 && hasDevKey ) {
      // Has dev key and no licence matches check username and password
      if ( userName != NULL && userPassword != NULL ) {
         uint32 UIN = m_thread->authenticateUser( userName, userPassword );
         if ( UIN != 0 && UIN != MAX_UINT32 && UIN != (MAX_UINT32 -1) ){
            // Ok!
            licenceMatch = true;
         } else {
            if ( UIN == 0 || UIN == (MAX_UINT32 -1) ) {
               res = 1;
               mc2log << warn << "checkUserLicence: UNAUTHORIZED User "
                      << "userName " << userName << "(" 
                      << user->getUIN() << ") pass " << userPassword 
                      << " using dev key. Licence: ";
               NavUserHelp::printLicence( mc2log, key, keyLength );
               mc2log << endl;
            } else { // == MAX_UINT32
               mc2log << warn << "checkUserLicence: authenticateUser "
                      << "timeout User userName " << userName << "(" 
                      << user->getUIN() << ") pass " << userPassword 
                      << " using dev key. Licence: ";
               NavUserHelp::printLicence( mc2log, key, keyLength );
               mc2log << endl;
               res = 2;
            }
         }
      } else {
         // Has no userName or userPassword for dev key.
         res = 1;
         mc2log << warn << "checkUserLicence: UNAUTHORIZED User "
                << "userName " << userName << "(" 
                << user->getUIN() << ") pass " << userPassword 
                << " using dev key. Licence: ";
         NavUserHelp::printLicence( mc2log, key, keyLength );
         mc2log << endl;
      }
   } // End has dev key and no licence matches

   if ( !licenceMatch && res == 0 ) {
      mc2log << warn << "checkUserLicence: UNAUTHORIZED";
      if ( otherUserHasBK ) {
         mc2log << " Other has licence";
         res = 4;
      } else {
         mc2log << " no licence match";
         res = 1;
      }
      mc2log << " userName " << userName << "(" 
             << user->getUIN() << ") Licence: ";
      NavUserHelp::printLicence( mc2log, key, keyLength );
      mc2log << endl;
   }

   delete user; // Newed above

   return res;
}


UserEnums::userRightLevel
NavUserHelp::getUrLevel( const ClientSetting* clientSetting ) {
   return m_thread->getUrLevel( clientSetting );
}


UserEnums::URType
NavUserHelp::getUrMask( const ClientSetting* clientSetting ) {
   return UserEnums::URType( getUrLevel( clientSetting ), UserEnums::UR_WF );
}


int
NavUserHelp::checkWFST( byte wayfinderType, UserItem* userItem, 
                        const ClientSetting* clientSetting,
                        byte& userwfst )
{
   int res = 0;   
   uint32 now = TimeUtility::getRealTime();

   if( clientSetting->getVersionLock() /*required*/ >
       userItem->getUser()->getHighestVersionLock() /*user highest*/){
      //if version lock is in effect, return TRIAL
      userwfst = WFSubscriptionConstants::TRIAL;
      return 4;
   }

   if ( userItem->getUser()->getNbrOfType(
           UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) > 0 ||
        userItem->getUser()->getNbrOfType(UserConstants::TYPE_RIGHT ) > 0 )
   {
      UserUser* user = new UserUser( *userItem->getUser() );

      UserEnums::userRightLevel urlevel = getUrLevel( clientSetting );

      userwfst = 
         m_thread->getSubscriptionTypeForUser( user, UserEnums::UR_WF, 
                                               true, urlevel );
      byte highestEver = 
         m_thread->getSubscriptionTypeForUser( user, UserEnums::UR_WF, 
                                               false, urlevel );
      if ( userwfst == MAX_BYTE ) {
         // Set to highest expired one
         userwfst = highestEver;
      }

      // Iron client should be Iron
      if ( m_thread->checkIfIronClient( clientSetting ) ) {
         if ( StringUtility::validEmailAddress( 
                 userItem->getUser()->getEmailAddress() ) ||
              clientSetting->getWFID() )
         {
            // For old clients with valid email or new clients send iron
            userwfst = WFSubscriptionConstants::IRON;
         } else { // Show upgrade view in client so user enters email
            userwfst = WFSubscriptionConstants::TRIAL;
         }
      }
      if ( userwfst != MAX_BYTE && wayfinderType != userwfst ) {
         if ( userwfst == WFSubscriptionConstants::TRIAL &&
              wayfinderType == WFSubscriptionConstants::SILVER 
              && clientSetting->getMatrixOfDoomLevel() == 1 )
         {
            // Update user to silver
            // Set html-access and Silver subscription
            // TODO: Remove htmlsevice and TYPE_WAYFINDER_SUBSCRIPTION
            user->setHTMLService( true );
            if (  user->getNbrOfType( 
                     UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) > 0 )
            {
               static_cast<UserWayfinderSubscription*> ( 
                  user->getElementOfType( 
                     0, 
                     UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) )
                  ->setWayfinderType( 
                     WFSubscriptionConstants::SILVER );
            }

            // Check transactions based
            bool removeInfTimeAccess = false;
            if ( user->getTransactionBased() != 
                 UserConstants::NO_TRANSACTIONS ) 
            {
               mc2log << info << "checkWFST: " 
                      << " changing from "
                      << "transaction based type " 
                      << int(user->getTransactionBased()) << " to "
                      << "NO_TRANSACTIONS" << endl;
               user->setTransactionBased( UserConstants::NO_TRANSACTIONS );
               removeInfTimeAccess = true;
            }

            // Set end time 
            // Move back 24hs to avoid problems with clocks 
            // not being synced
            uint32 startTime = now - 24*60*60;
            UserRegionAccess* changedRegion = NULL;
            // Time when subscription ends from client settings
            uint32 endTime = ParserUserHandler::addTime( 
               clientSetting->getSilverTimeYear(),
               clientSetting->getSilverTimeMonth(),
               clientSetting->getSilverTimeDay(),
               clientSetting->getExplicitSilverTime(),
               now );
            if ( user->getNbrOfType( UserConstants::TYPE_RIGHT ) > 0 ) {
               // Hmm, delete all unlimited and set new Silver rights?
               // Has unlimited trial on bundled-types should stop after 
               // silver time.
               UserEnums::URType trialWf( UserEnums::UR_TRIAL,
                                          UserEnums::UR_WF );
               for ( uint32 i = 0 ; 
                     i < user->getNbrOfType( UserConstants::TYPE_RIGHT ) ; 
                     i++ ) 
               {
                  UserRight* r = static_cast<UserRight*> ( 
                     user->getElementOfType( i, 
                                             UserConstants::TYPE_RIGHT ) );
                  if ( !r->isDeleted() && r->getUserRightType() == trialWf
                       && (r->getEndTime() >= MAX_INT32 || 
                           r->getEndTime() == 0) ) 
                  {
                     r->setDeleted( true );
                  }
               }
               // Add new
               MC2String origin( "AUTO Silver: " );
               origin.append( clientSetting->getClientType() );
               UserRight* newRight = new UserRight(
                  0, now, UserEnums::URType( UserEnums::UR_SILVER, 
                                             UserEnums::UR_WF ),
                  clientSetting->getSilverRegionID(),
                  startTime, endTime, false, origin.c_str() );
               user->addElement( newRight );
               newRight = new UserRight(
                  0, now, UserEnums::URType( UserEnums::UR_GOLD, 
                                             UserEnums::UR_MYWAYFINDER ),
                  clientSetting->getSilverRegionID(),
                  startTime, endTime, false, origin.c_str() );
               user->addElement( newRight );
               
            } else {
            for ( uint32 i = 0 ; 
                  i < user->getNbrOfType( 
                     UserConstants::TYPE_REGION_ACCESS ) ; i++ )
            {
               if ( static_cast< UserRegionAccess* > ( 
                       user->getElementOfType( 
                          i, UserConstants::TYPE_REGION_ACCESS ) )
                    ->getRegionID() == 
                    clientSetting->getSilverRegionID() )
               {
                  // Silver region access
                  changedRegion = static_cast< UserRegionAccess* >
                     ( user->getElementOfType( 
                        i, UserConstants::TYPE_REGION_ACCESS ) );
//                         changedRegion->setEndTime( endTime );
//                         if ( startTime < changedRegion->getStartTime() )
//                         {
//                            changedRegion->setStartTime( startTime );
//                         }
                  // This works, changing doesn't
                  changedRegion->remove();
                  changedRegion = NULL;
               } else if ( 
                  static_cast< UserRegionAccess* > ( 
                     user->getElementOfType( 
                        i, UserConstants::TYPE_REGION_ACCESS ) )
                  ->getRegionID() == MAX_INT32 ||
                  static_cast< UserRegionAccess* > ( 
                     user->getElementOfType( 
                        i, UserConstants::TYPE_REGION_ACCESS ) )
                  ->getRegionID() == 
                  clientSetting->getCreateRegionID() ||
                  (removeInfTimeAccess &&
                   static_cast< UserRegionAccess* > ( 
                      user->getElementOfType( 
                         i, UserConstants::TYPE_REGION_ACCESS ) )
                   ->getEndTime() == MAX_INT32) )
               {
                  // All region access or Created region
                  // Or unlimited time and we wants to remove that
                  // remove it
                  static_cast< UserRegionAccess* > ( 
                     user->getElementOfType( 
                        i, UserConstants::TYPE_REGION_ACCESS ) )->remove();
               }
            }
            if ( changedRegion == NULL ) {
               // WTF? no Silver region access? Add new one.
               changedRegion = new UserRegionAccess( 
                  0, clientSetting->getSilverRegionID(),
                  startTime, endTime );
               user->addElement( changedRegion );
            }
            } // End else use old region access
            // Add this update to operatorComment
            MC2String opCom = user->getOperatorComment();
            opCom.append( "; SILVER ");
            char dateStr[ 11 ];
            char timeStr[ 9 ];
            StringUtility::makeDateStr( now, dateStr, timeStr );
            opCom.append( dateStr );
            opCom.append( " " );
            opCom.append( timeStr );
            user->setOperatorComment( opCom.c_str() );
            // Send changes
            if ( m_thread->changeUser( user, NULL/*changer*/ ) ) {
               mc2log << info << "checkWFST: " 
                      << "upgraded user to SILVER " 
                      << user->getLogonID() << endl;
               userwfst = WFSubscriptionConstants::SILVER;
               res = 4;
            } else {
               mc2log << info << "checkWFST: " 
                      << "failed to update user to SILVER " 
                      << user->getLogonID() << "(" << user->getUIN() << ")"
                      << " will try again next session." << endl;
            }
         } // end trial->silver
         else {
            // Matrix Of DOOM -> send to client
            mc2log << info << "checkWFST: MOD updating "
                   << "client from: " << WFSubscriptionConstants::
               subscriptionTypeToString( WFSubscriptionConstants::
                                         subscriptionsTypes( 
                                            wayfinderType ) )
                   << " to " << WFSubscriptionConstants::
               subscriptionTypeToString( WFSubscriptionConstants::
                                         subscriptionsTypes( 
                                            userwfst ) ) 
                   << endl;
            res = 4;
         }
      } // End if user valid has WFST and client wfsubtype != db wfsubtype

      delete user;
   } // End if have param wf_type and user has wf_type

   return res;
}


MC2String 
NavUserHelp::makeExtraUserInfoStr( const NParamBlock& params ) const {
   MC2String userID;

   bool anyThingAdded = false;
   // Client type
   if ( params.getParam( 4 ) ) {
      userID.append( params.getParam( 4 )->getString(
                        m_thread->clientUsesLatin1()).c_str() );
      anyThingAdded = true;
   }
   // Add client version
   if ( params.getParam( 11 ) ) {
      char verStr[40];
      sprintf( verStr, "%u.%u.%u", 
               params.getParam( 11 )->getUint32Array( 0 ),
               params.getParam( 11 )->getUint32Array( 1 ),
               params.getParam( 11 )->getUint32Array( 2 ) );
      if ( anyThingAdded ) {
         userID += ',';
      }
      userID.append( verStr );
      anyThingAdded = true;
   }

   return userID;
}


uint8
NavUserHelp::handleNavUpgrade( const char* actCode,
                               uint32 topRegionID,
                               const char* phoneNumber,
                               UserUser* user,
                               const ClientSetting* clientSetting,
                               const uint32* cpvArray,
                               uint32 peerIP,
                               const UserLicenceKey& licenceKey,
                               byte& userWFType,
                               bool& topRegionOK,
                               bool& activationCodeOK,
                               bool& phonenumberOK,
                               UserItem*& changeToUserItem,
                               bool mayChangeUser,
                               MC2String& serverStr ) 
{
   uint8 status = NavReplyPacket::NAV_STATUS_OK;

   // WF sends formating chars in code, like '-'. Strip them
   MC2String activationCode;
   uint32 aPos = 0;
   while ( actCode[ aPos ] != '\0' ) {
      if ( isalnum( actCode[ aPos ] ) ) {
         activationCode += actCode[ aPos ];
      }
      aPos++;
   }


   mc2log << info << "handleNavUpgrade: ";
   // Print indata
   mc2log << " TopregionId " << topRegionID
          << " ActivationCode " << activationCode << " (" << actCode << ")"
          << " Phonenumber " << phoneNumber << endl;

   if ( m_thread->getActivationHandler()->isSpecialActivationCode( 
           activationCode ) ) 
   {
      mc2log << info << "handleNavUpgrade: Special (IMEI) AC from client "
             << "found \"" << activationCode << "\" Bad code returned" 
             << endl;
      activationCodeOK = false;
      return status;
   }
   char tmpStr[ 256 ];

   // Check top region id using top region request
   const TopRegionRequest* topRegReq = m_group->getTopRegionRequest( 
      m_thread );
   if ( topRegReq != NULL && topRegReq->getStatus() == StringTable::OK ) {
      topRegionOK = false;
      if ( topRegionID == MAX_INT32 ) {
         topRegionOK = true;
      } else {
         for ( uint32 i = 0 ; i < topRegReq->getNbrTopRegions() ; ++i ) {
            if ( topRegReq->getTopRegion( i )->getID() == topRegionID ) {
               topRegionOK = true;
               break;
            }
         }
         // Check meta regions
         vector< uint32 > regionGroupIDs;
         m_group->getRegionIDs()->addAllRegionGroups( regionGroupIDs );
         for ( uint32 i = 0 ; i < regionGroupIDs.size() ; ++i ) {
            if ( regionGroupIDs[ i ] == topRegionID ) {
               topRegionOK = true;
               break;
            }
         }
      }
   } else if ( topRegReq == NULL ) {
      mc2log << info << "handleNavUpgrade: "
             << "No TopRegionRequest " << endl;
      status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
   } else if ( topRegReq->getStatus() == StringTable::TIMEOUT_ERROR ) {
      mc2log << info << "handleNavUpgrade: "
             << "timeout on TopRegionRequest " << endl;
      status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
   } else {
      mc2log << info << "handleNavUpgrade: "
             << "not ok from TopRegionRequest \"" 
             << StringTable::getString( topRegReq->getStatus(), 
                                        StringTable::ENGLISH )
             << "\" (" << int(topRegReq->getStatus()) << ")" << endl;
      status = NavReplyPacket::NAV_STATUS_NOT_OK;
   }

   if ( topRegionOK && status == NavReplyPacket::NAV_STATUS_OK ) {
      // Handle request
      MC2String userAgent( m_thread->getUserHandler()->makeUserAgentString( 
                              clientSetting->getClientType(),
                              clientSetting->getClientTypeOptions(),
                              cpvArray ) );
      MC2String userInput;
      userInput.append( "Phonenumber: " );
      userInput.append( phoneNumber );
      // Cellular model
      MC2String cellularModel( "6600" );
      if ( false /* Model in client_options */ ) {

      } else {
         cellularModel = clientSetting->getPhoneModel();
      }
      userInput.append( ", Model: " );
      userInput.append( cellularModel );
      userInput.append( ", Top region id: " );
      sprintf( tmpStr, "%u", topRegionID );
      userInput.append( tmpStr );

      // Check phone number here
      //      By removing + () etc and getUserFromCellularNumber
      //      If exists must be same UIN as this user.
      //      Else set status to WRONG_PHONE_NBR.
      char* phoneNbr = StringUtility::cleanPhonenumber( phoneNumber );
      bool noAddPhonenumber = false;
      if ( StringUtility::validPhonenumber( phoneNbr ) ) {
         // Get cellular
         UserItem* userItem = NULL;
         if ( m_thread->getUserFromCellularNumber( 
                 phoneNbr, userItem, true ) ) 
         {
            // Ok check it
            if ( userItem != NULL && userItem->getUIN() != user->getUIN() )
            {
               // Not ok
               mc2log << warn << "handleNavUpgrade: "
                      << "ActivationRequest phonenumber "
                      << "already used " << MC2CITE( phoneNbr ) << endl;
               // No just don't add it phonenumberOK = false;
               noAddPhonenumber = true;
            } // No one has phonenumber or this user has it -> ok
         } else {
            mc2log << info << "phoneNumber: "
                   << "getUserFromCellularNumber failed " << endl;
            status = NavReplyPacket::NAV_STATUS_NOT_OK;
         }
         m_thread->releaseUserItem( userItem );
      } else if ( phoneNbr[ 0 ] == '\0' ) {
         mc2log << info << "handleNavUpgrade: "
                << "ActivationRequest phonenumber empty letting it pass" 
                << endl;
         noAddPhonenumber = true;
      } else {
         mc2log << warn << "handleNavUpgrade: "
                << "ActivationRequest phonenumber not valid "
                << MC2CITE( phoneNbr ) << " from "
                << phoneNumber << endl;
         phonenumberOK = false;
      }

      if ( phonenumberOK && status == NavReplyPacket::NAV_STATUS_OK ) 
      { // Cellular check whent ok
         uint32 ownerUIN = 0;
         int res = m_thread->getActivationHandler()->activateUser( 
            user, clientSetting, activationCode.c_str(), phoneNbr,
            topRegionID, peerIP, userAgent.c_str(), userInput.c_str(),
            cellularModel.c_str(), ownerUIN, serverStr, 
            false/*allowSpecialCodes*/ );

         if ( res == -4 && ownerUIN != 0 && mayChangeUser ) {
            // Change to ownerUIN (do nothing if already is ownerUIN)

            res = 0;
            if ( ownerUIN != user->getUIN() ) {
               // Get owner user
               UserItem* ownerItem = NULL;
               uint32 startTime = TimeUtility::getCurrentTime();
               bool commOk = m_thread->getUser( 
                  ownerUIN, ownerItem, true, true /*Really uptodate*/ );
               uint32 endTime = TimeUtility::getCurrentTime();
                 
               if ( !commOk ) {
                  mc2log << warn << "handleNavUpgrade: Get owner user ("
                         << ownerUIN << ") Error: ";
                  res = -1;
                  if ( endTime - startTime > 3000 ) {
                     status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
                     mc2log << "Timeout";
                     res = -2;
                  } else {
                     status = NavReplyPacket::NAV_STATUS_NOT_OK;
                     mc2log << "Error";
                  }
                  mc2log << endl;
               }

               if ( res == 0 ) {
                  res = m_thread->getUserHandler()->licenceTo( 
                     ownerItem->getUser(), &licenceKey );

                  if ( res == 0 ) {
                     mc2log << warn << "handleNavUpgrade: Changing to "
                            << "owner " 
                            << ownerItem->getUser()->getLogonID() << "("
                            << ownerItem->getUIN() << ") from " 
                            << user->getLogonID() << "(" << user->getUIN()
                            << ")" << endl;
                  } else {
                     status = NavReplyPacket::NAV_STATUS_NOT_OK;
                     if ( res == -2 ) {
                        status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
                     } else if ( res == -3 ) {
                        res = -5; // Used activation code
                        status = 
                           NavReplyPacket::
                           NAV_STATUS_UNAUTH_OTHER_HAS_LICENCE;
                     }
                  }

                  if ( res == 0 ) {
                     // Get user with uptodate ids
                     UserItem* returnItem = NULL;
                     uint32 startTime = TimeUtility::getCurrentTime();
                     bool commOk = m_thread->getUser( 
                        ownerItem->getUIN(), returnItem, true );
                     uint32 endTime = TimeUtility::getCurrentTime();
                 
                     if ( !commOk ) {
                        mc2log << warn << "handleNavUpgrade: Get uptodate "
                               << "owner user ("
                               << ownerUIN << ") Error: ";
                        res = -1;
                        if ( endTime - startTime > 3000 ) {
                           status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
                           mc2log << "Timeout";
                           res = -2;
                        } else {
                           status = NavReplyPacket::NAV_STATUS_NOT_OK;
                           mc2log << "Error";
                        }
                        mc2log << endl;
                     }

                     if ( res == 0 ) {
                        changeToUserItem = returnItem;
                        // user is used below
                        user = returnItem->getUser();
                     }
                  }

               } // End if res == 0

               m_thread->releaseUserItem( ownerItem );
            } else {
               mc2log << info << "handleNavUpgrade: Activation code used "
                      << "by this user, returning ok." << endl;
            }
         } // End if to change to owner of activation code

         if ( res != 0  ) {
            mc2log << info << "handleNavUpgrade: Failed to activate user ";
            status = NavReplyPacket::NAV_STATUS_OK;
            if ( res == -2 ) {
               status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
               mc2log << "Timeout";
            } else if ( res == -3 ) {
               phonenumberOK = false;
               mc2log << "Wrong phone number";
            } else if ( res == -4 ) {
               activationCodeOK = false;
               mc2log << "Used activation code";
            } else if ( res == -5 ) {
               activationCodeOK = false;
               mc2log << "Bad activation code";
            } else if ( res == -6 ) {
               activationCodeOK = false;
               mc2log << "Extension not allowed";
            } else if ( res == -7 ) {
               activationCodeOK = false;
               mc2log << "Creation not allowed";
            } else if ( res == -8 ) {
               // Redirect to serverStr
               status = NavReplyPacket::NAV_STATUS_REDIRECT;
               mc2log  << "Redirecting to " << serverStr;
            } else if ( res == -9 ) {
               activationCodeOK = false;
               mc2log << "Use of code in IRON client not allowed";
            } else if ( res == -10 ) {
               mc2log << "Must choose region";
               status = NavReplyPacket::UPGRADE_MUST_CHOOSE_REGION;
            } else {
               // Else just error
               status = NavReplyPacket::NAV_STATUS_NOT_OK;
            }
            mc2log << endl;
         } else {
            // Else activated ok

            //      External auth?? But they doesn't activate! And its only
            //      to reactivate and that is done automatically in ext
            //      auth java clients. No need to check here.
            if ( m_thread->getTokenHandler()->getNbrTokens( 
                    user, clientSetting, true/*allGroups*/ ) > 0 ) 
            {
               // Remove tokens
               UserUser cuser( *user );
               //      IRON/"" tokens too? but that means that you can't
               //      have java earth and symbian navigator...
               //      but better than not being able to start
               int tres = m_thread->getTokenHandler()->removeAllButToken( 
                  &cuser, "", clientSetting, true/*sendChange*/,
                  true/*allGroups*/ );
               if ( tres == 0 ) {
                  mc2log << info << "handleNavUpgrade: removed "
                         << m_thread->getTokenHandler()->getNbrTokens( 
                            user, clientSetting ) 
                         << " tokens from user successfully " << endl;
               } else {
                  // Error
                  mc2log << warn << "handleNavUpgrade: Failed to remove "
                         << "tokens from user. Error: " ;
                  status = NavReplyPacket::NAV_STATUS_NOT_OK;
                  if ( tres == -2 ) {
                     status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
                     mc2log << "Timeout";
                  } else {
                     mc2log << "Error";
                  }
                  mc2log << endl;
               }

            } // End if has tokens
         }

         userWFType = m_thread->getSubscriptionTypeForUser( 
            user, UserEnums::UR_WF );

      } // End if phonenumberOK from Cellular check
      delete [] phoneNbr;
   } // Endif topregionok and status is ok

   return status;
}


void 
NavUserHelp::handleUploadFiles( UserItem* userItem, const vector<byte>& v,
                                const byte* key, uint32 keyLength )
{
   // [filename string] : [length in decimal string] :
   // [length bytes data]
   // Mailto
   const char* sendTo = "mc2logs@localhost.localdomain";
   MimeMessage* m = new MimeMessage( 
      MimePartApplication::getContentTypeAsString(
         MimePartApplication::
         CONTENT_TYPE_APPLICATION_OCTETSTREAM ) );
   uint32 pos = 0;
   vector<MC2String> files;

   while ( pos < v.size() ) {
      char name[ v.size() + 1 ];
      char lengthStr[ v.size() + 1 ];
      uint32 length = 0;
      char* tmpStr = NULL;
      if ( sscanf( (const char*)&v.front() + pos, 
                   "%[^:]:%[^:]:", name, lengthStr ) == 2 ) 
      {
         pos += strlen( name ) + 1 + strlen( lengthStr ) + 1;
         length = strtoul( lengthStr, &tmpStr, 10 );
         if ( tmpStr != NULL && *tmpStr == '\0' ) {
            MimePartApplication* part = new MimePartApplication( 
               const_cast<byte*>( &v.front() + pos ), 
               length, 
               MimePartApplication::
               CONTENT_TYPE_APPLICATION_OCTETSTREAM,
               name, true );
            MC2String dStr( "attachment; filename=\"" );
            dStr.append( name );
            dStr.append( "\"" );
            part->addHeaderfield( "Content-Disposition",
                                  dStr.c_str() );
            m->add( part );
            pos += length;
            files.push_back( name );
         } else {
            mc2log << warn << "handleUploadFiles file length at "
                   << pos << " of " << v.size() << endl;
            pos = v.size();
         }
      } else {
         mc2log << warn << "handleUploadFiles bad file at " << pos
                << " of " << v.size() << endl;
         pos = v.size();
      }
   }

   bool sentOk = false;
   SendEmailRequestPacket* p = new SendEmailRequestPacket( 0 );
   char* body = m->getMimeMessageBody();
   const char* optionalHeaderTypes[ 2 ] = 
      { MimeMessage::mimeVersionHeader, MimeMessage::contentTypeHeader };
   const char* optionalHeaderValues[ 2 ] = 
      { m->getMimeVersion(), m->getContentType() };
   MC2String sender( Properties::getProperty( 
                        "DEFAULT_RETURN_EMAIL_ADDRESS", 
                        "please_dont_reply@localhost.localdomain" ) );
   MC2String subject( "WF Uploaded files" );
   char tmpStr[20];

   if ( userItem != NULL ) {
      UserUser* user = userItem->getUser();
      sprintf( tmpStr, "%u", user->getUIN() );
      subject.append( " From user " );
      subject.append( user->getLogonID() );
      subject.append( "(" );
      subject.append( tmpStr );
      subject.append( ")" );
   } 

   if ( key != NULL ) {
      subject.append( " Key " );
      NavUserHelp::printLicence( subject, key, keyLength );
   } // End if key != NULL
   
   if ( p->setData( sendTo, sender.c_str(), subject.c_str(), 
                    body,
                    2, optionalHeaderTypes, optionalHeaderValues ) )
   {
      PacketContainer* rp = new PacketContainer( p, 0, 0 , 
                                                 MODULE_TYPE_SMTP );
      PacketContainer* pc = m_thread->putRequest( rp );
      if ( pc != NULL && static_cast< ReplyPacket* >( 
              pc->getPacket() )->getStatus() == StringTable::OK )
      {
         sentOk = true; 
      }
      delete pc;
   } else {
      mc2log << error << "handleUploadFiles "
             << "SendEmailRequestPacket::setData failed." << endl;
   }
         
   delete [] body;
   delete m;

   if ( !sentOk ) {
      Utility::hexDump( 
         mc2log, const_cast<byte*>( &v.front() ), v.size() );
   }

   // Unset triggers here for files in files-vector
   if ( userItem != NULL ) {
      removeUploadFiles( userItem, files );
   } // End if have user
   
}

void
NavUserHelp::removeUploadFiles( UserItem* userItem, 
                                const vector<MC2String>& files ) 
{
   UserUser* user = userItem->getUser();
   MC2String initials = user->getInitials();
   // Remove file-names from Initials
   for ( uint32 i = 0 ; i < files.size() ; ++i ) {
      MC2String::size_type pos = initials.find( files[ i ] );
      if ( pos != MC2String::npos ) {
         // Remove from initials
         uint32 eraseSize = files[ i ].size();
         if ( initials[ pos + files[ i ].size() ] == ',' ) {
            ++eraseSize;
         } else if ( pos + files[ i ].size() ==
                     initials.size() && pos > 0 &&
                     initials[ pos - 1 ] == ',' )
         {
            // Remove preceding ','
            --pos;
            ++eraseSize;
         }
         initials.erase( pos, eraseSize );
      } // Else not present no need to remove
      
   } // End for all file-names
   if ( initials.compare( user->getInitials() ) != 0 ) {
      // Change 
      mc2dbg << "Chaning initials from " << user->getInitials()
             << " to " << initials << endl;
      UserUser* cuser = new UserUser( *user );
      cuser->setInitials( initials.c_str() );
      if ( !m_thread->changeUser( cuser, NULL/*changer*/ ) ) {
         mc2log << warn << "handleUploadFiles " 
                << "failed to remove uploaded files from user."
                << endl;
      }
      delete cuser;
   }
}


MC2Coordinate
NavUserHelp::getCenterpointFor( UserItem* userItem, uint32& scale ) {
   MC2Coordinate cp( 0, 0 ); // Greenwich
   MC2BoundingBox bbox; // Impossible bbox
   UserUser* user = userItem->getUser();
   UserEnums::userRightService service  = UserEnums::UR_WF;
   UserEnums::userRightLevel levels = 
      UserEnums::userRightLevel( 
         UserEnums::ALL_LEVEL_MASK ^ UserEnums::UR_TRIAL );
   byte wfst = m_thread->getSubscriptionTypeForUser( user, service );
   if ( wfst >= WFSubscriptionConstants::IRON ) {
      // No change all levels
   } else if ( wfst >= WFSubscriptionConstants::GOLD ) {
      levels = UserEnums::UR_GOLD;
   } else if ( wfst >= WFSubscriptionConstants::SILVER ) {
      levels = UserEnums::SG_MASK;
   }
   UserEnums::URType urmask( levels, service );

   // Get top regions
   const TopRegionRequest* topRegReq = m_group->getTopRegionRequest( 
      m_thread );
   if ( topRegReq != NULL && topRegReq->getStatus() == StringTable::OK ) {
      for ( uint32 i = 0 ; i < topRegReq->getNbrTopRegions() ; ++i ) {
         if ( m_thread->checkUserRegionAccess( 
                 topRegReq->getTopRegion( i )->getID(), user, urmask ) )
         {
            bbox.update( topRegReq->getTopRegion( i )->getBoundingBox() );
         }
      }
   } else {
      mc2log << error << "NavUserHelp::getCenterpointFor failed to get "
             << "TopRegionRequest " << endl;
   }

   if ( bbox.getCenter().isValid() ) {
      cp = bbox.getCenter();
   }

   // Perhaps set to see complete bbox, when scale can be calculated
   scale = 100000;

   return cp;
}


bool
NavUserHelp::getUserFromParams( NParamBlock& params, MC2String& idStr,
                                UserItem*& userItem, bool useCache,
                                bool wipeFromCache )
{
   bool commOk = true;
   if ( params.getParam( 9 ) != NULL ) { // UIN
      idStr = params.getParam( 9 )->getString(
         m_thread->clientUsesLatin1());
      uint32 UIN = strtoul( idStr.c_str(), NULL, 0 );
      commOk = m_thread->getUser( UIN, userItem, useCache, wipeFromCache );
   } else if ( params.getParam( 8 ) != NULL ) { // LogonID
      idStr = params.getParam( 8 )->getString(
         m_thread->clientUsesLatin1());
      commOk = m_thread->getUser( idStr.c_str(), userItem, 
                                  useCache, wipeFromCache );
   } else if ( params.getParam( 1 ) != NULL  ) { // UIN/logonID or whatever
      idStr = params.getParam( 1 )->getString(
         m_thread->clientUsesLatin1());
      uint32 UIN = getUIN( idStr.c_str() );
      if ( UIN != 0 && UIN != MAX_UINT32 ) {
         commOk = m_thread->getUser( UIN, userItem, useCache, wipeFromCache );
      } else {
         if ( UIN == 0 ) {
            userItem = NULL;
         } else {
            commOk = false;
         }
      }
   }
   
   return commOk;
}


void
NavUserHelp::getUsersPINs( const UserItem* userItem, NParam& p ) {
   const UserUser* userUser = userItem->getUser();

   for ( uint32 i = 0 ;
         i < userUser->getNbrOfType( UserConstants::TYPE_PIN ) ; i++ )
   {
      UserPIN* t = static_cast< UserPIN* >( 
         userUser->getElementOfType( i, UserConstants::TYPE_PIN ) );

      // ID
      p.addUint32( t->getID() );
      // PIN
      p.addString( t->getPIN(), m_thread->clientUsesLatin1() );
      // Comment
      p.addString( t->getComment(), m_thread->clientUsesLatin1() );
   }
}


uint32
NavUserHelp::getUsersPINsCRC( const UserItem* userItem ) {
   NParam p( 0 );
   getUsersPINs( userItem, p );

   uint32 crc = 0;
   if ( p.getLength() > 0 ) {
      crc = MC2CRC32::crc32( 
         p.getBuff(), p.getLength() );
   }

   return crc;
}

bool
NavUserHelp::addServerListParams( 
   NParamBlock& params,
   const ClientSetting& settings,
   bool http, bool addServer, bool printCRC,
   const MC2String& fixedServerListName ) const
{
   bool added = false;
   const MC2String* serverListName = &settings.getServerListName();
   if ( *serverListName == "" ) {
      serverListName = &NamedServerLists::DefaultGroup;
   }
   if ( !fixedServerListName.empty() ) {
      serverListName = &fixedServerListName;
   }

   const MC2String* serverListType =
      &NamedServerLists::NavServerType;
   if ( http ) {
      serverListType = &NamedServerLists::HttpServerType;
   }

   const MC2String& serverList = 
      m_group->getNamedServerList( *serverListName, *serverListType );
   if ( serverList != "" ) {
      uint32 crc = m_group->getNamedServerListCRC( *serverListName, 
                                                   *serverListType );

      if ( addServer ) {
         params.addParam( NParam( 3900, serverList, 
                                  m_thread->clientUsesLatin1() ) );
      }
      if ( printCRC ) {
         mc2log << " ServerListCRC " << MC2HEX( crc ); 
      }
      params.addParam( NParam( 4307, crc ) );
      added = true;
   }

   return added;
}


const ClientSetting* 
NavUserHelp::getClientSetting( const NParamBlock& params,
                               const UserUser* user ) const
{
   const ClientSetting* clientSetting = NULL;
   MC2String clientType = "unknown";
   MC2String clientTypeOptions = "";
   bool hasType = false;

   if ( params.getParam( 4 ) != NULL ) {
      clientType = params.getParam( 4 )->getString(
         m_thread->clientUsesLatin1() );
      hasType = true;
   }
   if ( params.getParam( 5 ) != NULL ) {
      clientTypeOptions = params.getParam( 5 )->getString(
         m_thread->clientUsesLatin1() );
   }

   if ( !hasType ) {
      UserUser::constUserElRange_t ls = user->getElementRange(
         UserConstants::TYPE_LAST_CLIENT );
      if ( ls.first != ls.second ) {
         const UserLastClient* l = static_cast<const UserLastClient*>( 
            *ls.first );
         clientType = l->getClientType();
         clientTypeOptions = l->getClientTypeOptions();
      }
   }
   clientSetting = m_group->getSetting( clientType.c_str(),
                                        clientTypeOptions.c_str() );

   return clientSetting;
}


bool
NavUserHelp::updateSessionUser( UserUser* user, 
                                isabBoxSession* session )
{
   bool ok = true;
   // Change user
   if ( !m_thread->changeUser( user, NULL/*changer*/ ) ) {
      mc2log << warn << "updateSessionUser: " 
             << "failed to change user,"
             << " will try again next session." << endl;
      ok = false;
   }
   // Update to new user with correct IDs
   UserItem* userItem = NULL;
   if ( m_thread->getUser( user->getUIN(), userItem, true ) ) {
      if ( userItem != NULL ) {
         // Update to new user
         session->setUser( new UserItem( *userItem ) );
         user = session->getUser()->getUser();
      } else {
         ok = false;
      }
   } else {
      ok = false;
   }
   m_thread->releaseUserItem( userItem );

   return ok;
}

bool
NavUserHelp::getLicencesAndTypes( const NParamBlock& params, uint16 typeID,
                                  uint16 licenceID, 
                                  vector< UserLicenceKey >& hwKeys,
                                  const MC2String& product ) const
{
   bool ok = true;

   
   vector< const NParam* > licenceIDparams;
   params.getAllParams( licenceID, licenceIDparams );
   vector< const NParam* > typeIDparams;
   params.getAllParams( typeID, typeIDparams );

   if ( licenceIDparams.size() > 1 && 
        licenceIDparams.size() != typeIDparams.size() ) {
      ok = false;
      mc2log << warn << "getLicencesAndTypes Licence type array is "
             << "not same size as licence keys. Keys size " 
             << licenceIDparams.size() << " type size " << typeIDparams.size()
             << endl;
   }

   for ( uint32 i = 0 ; i < licenceIDparams.size() && ok ; ++i ) {
      hwKeys.push_back( UserLicenceKey( MAX_UINT32 ) );
      hwKeys.back().setLicence( licenceIDparams[ i ]->getBuff(), 
                                licenceIDparams[ i ]->getLength() );
      hwKeys.back().setProduct( product ); 
      if ( i < typeIDparams.size() ) {
         ParamExpectation expect( typeID, NParam::String );
         if ( expect.checkParam( typeIDparams[ i ] ) ) {
            MC2String type = typeIDparams[ i ]->getString( 
               m_thread->clientUsesLatin1() );

            if ( ParserUserHandler::validHardwareKeyTypes.find( type ) != 
                 ParserUserHandler::validHardwareKeyTypes.end() ) {
               hwKeys.back().setKeyType( type ); 
            } else {
               ok = false;
               mc2log << warn << "getLicencesAndTypes Licence type is "
                      << "not valid type. Param: ";
               typeIDparams[ i ]->dump( mc2log, true, true );
               mc2log << endl;
            }
         } else {
            ok = false;
            mc2log << warn << "getLicencesAndTypes Licence type is "
                   << "not a string: ";
            typeIDparams[ i ]->dump( mc2log, true, true );
            mc2log << endl;
         }         
      } else {
         // The old clients has IMEI
         hwKeys.back().setKeyType( ParserUserHandler::imeiType );
      }
   }

   return ok;
}
