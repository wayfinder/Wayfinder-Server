/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ParserUserHandler.h"
#include <iostream>
#include <functional>
#include <algorithm>
#include "ParserThreadGroup.h"
#include "Properties.h"
#include "UserRight.h"
#include "UserData.h"
#include "UserLicenceKey.h"
#include "UserPacket.h"
#include "SendEmailPacket.h"
#include "MimeMessage.h"
#include "IPnPort.h"
#include "ParserActivationHandler.h"
#include "MC2CRC32.h"
#include "UserFavorites.h"
#include "UserFavoritesRequest.h"
#include "ClientSettings.h"
#include "InterfaceRequestData.h"
#include "LicenceToPacket.h"
#include "UserRightHelper.h"
#include "SQLDataContainer.h"
#include "GetStoredUserDataPacket.h"
#include "SetStoredUserDataPacket.h"
#include "IDKeyToPacket.h"
#include "UserIDKey.h"
#include "STLStringUtility.h"

#include <boost/lexical_cast.hpp>

using namespace UserRightHelper;

const MC2String ParserUserHandler::imeiType = "imei";
const MC2String ParserUserHandler::imsiType = "imsi";
const MC2String ParserUserHandler::iPhoneDevIDType = "iphone_dev_id";
const MC2String ParserUserHandler::customerMSISDNType = "customer_msisdn";

// Valid hardware key types are:
// The types are in priority order.
static const char* validTypesArray[] = { 
   ParserUserHandler::iPhoneDevIDType.c_str(),
   ParserUserHandler::imeiType.c_str(), 
   "bbpin", 
   "btmac", 
   "esn", 
   ParserUserHandler::imsiType.c_str(), 
   "phone_msisdn",
   ParserUserHandler::customerMSISDNType.c_str() };
const set< MC2String > ParserUserHandler::validHardwareKeyTypes(
   validTypesArray, validTypesArray + NBR_ITEMS( validTypesArray ) );


static const byte* nullByte = reinterpret_cast<const byte*>( "" );


/**
 * ostream formatter of licencekeys.
 */
struct printLicence {
   /**
    * Print a licencekey by pointer and length.
    * @param key Pointer to the first byte.
    * @param len The number of bytes. 
    */
   printLicence(const uint8* key, uint32 len) : 
      begin(key), end(key + len)
   { }

   /**
    * Print a UserLicenceKey object.
    * @param ul Pointer to the UserLicenceKey object.
    */
   printLicence(const UserLicenceKey* ul) :
      begin( ul ? ul->getLicenceKey() : nullByte ),
      end( ul ? begin + ul->getLicenceLength() : nullByte )
   {}

   /**
    * Print a UserLicenceKey object.
    * @param ul Reference to the UserLicenceKey object.
    */
   printLicence(const UserLicenceKey& ul) :
      begin( ul.getLicenceKey() ),
      end( begin + ul.getLicenceLength() )
   {}

   /**
    * Do the actual formatting and printing.
    * @param out The stream to print on.
    * @return The stream.
    */
   ostream& print(ostream& out) const
   {
      const bool hexPrint = 
         (end == std::find_if( begin, end, ptr_fun( ::isprint ) ));

      if ( hexPrint ) {
         out << hex << setfill( '0' );
         out.setf( ios::uppercase );
         for ( const byte* it = begin ; it != end ; ++it ) {
            out << setw( 2 ) << int( *it );
         }
         out << dec << endl;
         out.unsetf( ios::uppercase ); // Reset to default
      } else {
         for ( const byte* it = begin ; it != end ; ++it ) {
            out << *it;
         }
      }
      return out;
   }

private:
   /** The start iterator of the licence.*/
   const byte* const begin;
   /** The end iterator of the licence.*/
   const byte* const end;
};

/**
 * Stream operator for printLicence.
 * @param out The stream to print on.
 * @param pl The printLicence object.
 * @return The stream. 
 */
ostream& operator<<(ostream& out, const printLicence& pl)
{
   return pl.print( out );
}

/**
 * Method outside of this class
 */
ostream& operator<<( ostream& out, const UserLicenceKey& ul ) {
   out << ul.getKeyType() << ":";
   return printLicence( &ul ).print( out );
}


ParserUserHandler::ParserUserHandler( ParserThread* thread,
                                      ParserThreadGroup* group )
      : ParserHandler( thread, group )
{
}

 
int
ParserUserHandler::createWayfinderUser( 
   MC2String& passwd,
   const UserLicenceKey* userKey, 
   UserItem*& userItem,
   uint32 startTime,
   uint32 endTime,
   uint32 regionID,
   LangTypes::language_t lang, 
   const char* clientType,
   const char* clientTypeOptions,
   const uint32* programV,
   const MC2String& activationCode,
   const ExtraTypes& extraTypes,
   WFSubscriptionConstants::subscriptionsTypes createWFST,
   int32 transactionDays,
   const MC2String& brand,
   const char* fixedUserName,
   const char* extraComment,
   const UserElementVector& extraElements,
   const char* userNamePrefix,
   const char* email,
   const char* name, 
   const MC2String& opt_in_name,
   const MC2String& opt_in_value,
   const MC2String& extraRights )
{
   int status = 0;

   StringTable::languageCode language = 
      ItemTypes::getLanguageTypeAsLanguageCode( lang );
   // Create new user with random logonID and password
   // Retry if logonID not unique

   // We hope srand has been called with good value in startup

   if ( passwd.empty() ) {
      makePassword( passwd );
   }

   bool ok = true;
   // Check if licenceKey is already used! Before adding it to new user.
   if ( userKey != NULL ) {
      uint32 nbrUsers = 0;
      ok = m_thread->getUserFromUserElement( 
         userKey, userItem, nbrUsers, true );
      if ( !ok || nbrUsers != 0 ) {
         ok = false;
         status = 3;
         if ( nbrUsers != 0 ) {
            mc2log << warn << "createWayfinderUser: Licence key already used "
                   << printLicence( userKey ) << " returning error." << endl;
         } else {
            mc2log << warn << "createWayfinderUser: Licence key check "
                   << "failed returning error." << endl;
         }
         m_thread->releaseUserItem( userItem );
         userItem = NULL;
         return status;
      }
   }

   const ClientSetting* clientSetting = m_group->getSetting(
      clientType, clientTypeOptions );

   bool createdUser = false;
   do {
      MC2String userName( "WL" ); // Wayfinder Light
      if ( Properties::getProperty( "WAYFINDER_CREATE_USER_PREFIX" ) ) {
         userName = Properties::getProperty( 
            "WAYFINDER_CREATE_USER_PREFIX" );
      }
      if ( userNamePrefix ) { // Override for prefix
         userName = userNamePrefix;
      }
      for ( uint32 i = 0 ; i < 6 ; ++i ) {
         userName += char( 65 + 
                           uint32( ( 25.0*rand() / (RAND_MAX + 1.0) ) ) );
      }
      if ( !userNamePrefix && fixedUserName ) {
         userName = fixedUserName;
      }
      mc2dbg4 << "Trying to create Wayfinder user with logonID \""
              << userName << "\" password \"" << passwd << "\"" << endl;
      
      UserUser* user = new UserUser( 0 );
      user->setLogonID( userName.c_str() );
      // Add licence
      if ( userKey != NULL ) {
         UserLicenceKey* userLicence = new UserLicenceKey( 0 );
         userLicence->setProduct( userKey->getProduct() );
         userLicence->setKeyType( userKey->getKeyType() );
         userLicence->setLicence( userKey->getLicenceKeyStr() );
         user->addElement( userLicence );
      }
      // Set operatorcomment
      char dateStr[ 11 ];
      char timeStr[ 9 ];
      uint32 now = TimeUtility::getRealTime();
      StringUtility::makeDateStr( now, dateStr, timeStr );
      MC2String operatorcomment( "Created " );
      operatorcomment.append( m_thread->getServerType() );
      operatorcomment.append( " " );
      operatorcomment.append( dateStr );
      operatorcomment.append( " " );
      operatorcomment.append( timeStr );
      operatorcomment.append( "[UTC]" );
      if ( !activationCode.empty() ) {
         operatorcomment.append( " AC: " );
         operatorcomment.append( activationCode );
      }
      if ( userKey != NULL ) {
         operatorcomment.append( " BK: " );
         for ( uint32 i = 0 ; i < userKey->getLicenceLength() ; ++i ) {
            operatorcomment += userKey->getLicenceKey()[ i ];
         }
      }
      operatorcomment.append( " " );
      operatorcomment.append( 
         WFSubscriptionConstants::subscriptionTypeToString(
            WFSubscriptionConstants::subscriptionsTypes( 
               createWFST ) ) );
      // param_client_type
      if ( clientType != NULL || programV != NULL ) {
         operatorcomment.append( " (" );
      }
      if ( clientType != NULL ) {
         operatorcomment.append( clientType );
      }
      // param_client_type_options
      if ( clientType != NULL && 
           clientTypeOptions != NULL && clientTypeOptions[ 0 ] != '\0' ) 
      {
         operatorcomment.append( " opt: " );
         operatorcomment.append( clientTypeOptions );
      }
      // param_program_version
      if ( programV != NULL ) {
         operatorcomment.append( " v" );
         char tmpStr[ 256 ];
         sprintf( tmpStr, "%u", programV[ 0 ] );
         operatorcomment.append( tmpStr ); 
         operatorcomment.append( "." );
         sprintf( tmpStr, "%u", programV[ 1 ] );
         operatorcomment.append( tmpStr ); 
         operatorcomment.append( "." );
         sprintf( tmpStr, "%u", programV[ 2 ] );
         operatorcomment.append( tmpStr ); 
      }
      if ( clientType != NULL || programV != NULL ) {
         operatorcomment.append( ")" );
      }

      if ( extraComment ) {
         operatorcomment.append( "; " );
         operatorcomment.append( extraComment );
      }
      user->setOperatorComment( operatorcomment.c_str() );
      // Email
      if ( email != NULL && *email != '\0' ) {
         user->setEmailAddress( email );
      }
      // Name
      if ( name != NULL && *name != '\0' ) {
         // Split name on space and see
         vector<MC2String> splitStr;
         StringUtility::tokenListToVector( splitStr, name, ' ' );
         if ( splitStr.size() > 1 ) {
            user->setFirstname( splitStr[ 0 ].c_str() );
            user->setLastname( splitStr[ 1 ].c_str() );
         } else if ( splitStr.size() > 0 ) {
            user->setFirstname( splitStr[ 0 ].c_str() );
         }
      }

      // Add create right.
      MC2String origin( "AUTO: " );
      origin.append( operatorcomment );
      if ( clientSetting->usesRights() ) {
         user->addElement( 
            new UserRight( 0, now, UserEnums::URType( 
                              UserEnums::wfstAsLevel( createWFST ), 
                              UserEnums::UR_WF ),
                           regionID, startTime, 
                           endTime, false, origin.c_str() ) );
      }
      // And all extraTypes
      for ( uint32 i = 0 ; i < extraTypes.size() ; ++i ) {
         uint32 setEndTime = endTime;
         if ( extraTypes[ i ].second.isSet ) {
            setEndTime = ParserUserHandler::addTime( 
               extraTypes[ i ].second.year,
               extraTypes[ i ].second.month,
               extraTypes[ i ].second.day,
               extraTypes[ i ].second.explic,
               startTime );
         }
         uint32 setRegionID = regionID;
         if ( extraTypes[ i ].second.regionID != MAX_UINT32 ) {
            setRegionID = extraTypes[ i ].second.regionID;
         }
         user->addElement( 
            new UserRight( 0, now, extraTypes[ i ].first,
                           setRegionID, startTime, setEndTime, 
                           false, origin.c_str() ) ); 
      }
      for ( uint32 i = 0 ; i < extraElements.size() ; ++i ) {
         user->addElement( extraElements[ i ] );
      }

      // Check TransactionDays
      if ( transactionDays > 0 ) {
         // Transaction days based user
         user->setTransactionBased( UserConstants::TRANSACTION_DAYS );
         // Days added below (when UIN is set)
      }


      // Set eternal validtime
      user->setValidDate( 0 );
      // Set language
      user->setLanguage( language );

      // Brand user (With hot metal iron brander)
      if ( brand[ 0 ] != '\0' ) {
         // brand_origin
         user->setBrandOrigin( brand.c_str() );
         // And brand
         user->setBrand( brand.c_str() );
      }
      if ( !opt_in_name.empty() ) {
         user->setCustomerContactInfoField( opt_in_name, opt_in_value );
      }

      if ( ! extraRights.empty() ) {
         MC2String ac = activationCode;
         if ( ac.empty() ) {
            ac = "Extra Rights: " + extraRights;
         }

         m_thread->getActivationHandler()->
            addUserRights( user,
                           // ignore phone and model and topRegionID
                           "", "", MAX_INT32,
                           extraRights, ac,
                           now );
      }

      uint32 uin = m_thread->createUser( user, passwd.c_str(), 
                                         NULL/*changer*/ );
      if ( uin == 0 ) {
         // Try again with new logonID
      } else if ( uin == MAX_UINT32 ) {
         // Comm error
         status = 2;
         ok = false;
      } else if ( uin == MAX_UINT32-1 ) {
         // Error
         status = 3;
         ok = false;
      } else if ( uin == MAX_UINT32-2 ) {
         // Non unique UserIDKey error
         status = 4;
         ok = false;
      } else {
         // Bingo!
         createdUser = true;
         // Add transaction days
         if ( transactionDays > 0 ) {
            // Transaction days based user
            uint32 curTime = 0;
            StringTable::stringCode res = StringTable::OK;
            int retries = 4;
            do {
               res = m_thread->getAndChangeTransactionDays(
                  uin, false/*check*/, transactionDays, curTime );
               retries--;
               if ( res != StringTable::OK ) {
                  ISABThread::sleep( 1000 ); // ms
               }
            } while ( res != StringTable::OK && retries > 0 );
            if ( res != StringTable::OK ) {
               mc2log << error << "createWayfinderUser: failed to add "
                      << transactionDays
                      << " Transaction Days to " << user->getLogonID() 
                      << "(" << uin << ") DELETING USER!" << endl;
               DeleteUserRequestPacket* p = new DeleteUserRequestPacket(
                  0,0, uin );
               PacketContainer* pc = m_thread->putRequest( 
                  p, MODULE_TYPE_USER );
               if ( pc == NULL || static_cast<ReplyPacket*>(
                       pc->getPacket() )->getStatus() != StringTable::OK )
               {
                  mc2log << "createWayfinderUser: failed to delete user "
                         << user->getLogonID() << "(" << uin << ")" 
                         << endl;
               }
               delete pc;
               uin = 0;
               createdUser = false; // No, not ok
               ok = false;
               status = 3;
            } // End if res != StringTable::OK
         } // End if transactionDays > 0


         if ( ok && createdUser ) {            
            
            // Get user (to test that it works!)
            if ( m_thread->getUser( uin, userItem, true ) ) {
               if ( userItem != NULL ) {
                  // DONE!
               } else {
                  // But I just created it!?
                  mc2log << error << "createWayfinderUser: created \"" 
                         << userName
                         << "\" uin " << uin << " ok but can't get user "
                         << "from database now!" << endl;
                  status = 3;
                  ok = false;
               }
            } else {
               // Arrghhh 
               // Comm error
               status = 2;
               ok = false;
            }
         } // End if createdUser
      }

      delete user;
   } while ( ok && !createdUser && fixedUserName == NULL );

   return status;
}

   
int 
ParserUserHandler::createWayfinderUser( 
   MC2String& passwd,
   const UserLicenceKey* userKey, 
   UserItem*& userItem,
   LangTypes::language_t lang, 
   const char* clientType,
   const char* clientTypeOptions,
   const uint32* programV,
   const MC2String& activationCode,
   const ExtraTypes& extraTypes,
   const char* fixedUserName,
   const char* extraComment,
   const UserElementVector& extraElements )
{
   int status = 0;

   // Make create access settings.
   uint32 now = TimeUtility::getRealTime();
   const ClientSetting* navSetting = m_group->getSetting(
      clientType, clientTypeOptions );
   uint32 startTime = now - 24*60*60;
   uint32 endTime = now;

   m_group->getCreateWFTime( navSetting, endTime );

   
   WFSubscriptionConstants::subscriptionsTypes createWFST = 
      WFSubscriptionConstants::subscriptionsTypes(
         navSetting->getCreateLevel() );
   uint32 regionID = navSetting->getCreateRegionID();
   int32 transactionDays = navSetting->getCreateTransactionDays();

   status = m_thread->getUserHandler()->createWayfinderUser(
      passwd, userKey, userItem, startTime, endTime, regionID, lang,
      clientType, clientTypeOptions, programV, activationCode, 
      extraTypes, createWFST, transactionDays, navSetting->getBrand(),
      fixedUserName, extraComment, extraElements,
      NULL,  // userNamePrefix
      NULL, NULL, // email, name
      "", "", // opt in name, opt in value
      navSetting->getExtraRights() );

   mc2dbg2 << "createWayfinderUser: creating with extra rights: "
           << navSetting->getExtraRights() << endl;

   return status;
}


int
ParserUserHandler::createWayfinderUserWithAutoAC( 
   MC2String& passwd,
   MC2String& server,
   const UserLicenceKey* userKey, 
   UserItem*& userItem,
   uint32 startTime,
   uint32 endTime,
   uint32 regionID,
   LangTypes::language_t lang, 
   const char* clientType,
   const char* clientTypeOptions,
   const uint32* programV,
   const MC2String& activationCode,
   const ExtraTypes& extraTypes,
   WFSubscriptionConstants::subscriptionsTypes createWFST,
   int32 transactionDays,
   const MC2String& brand,
   const char* fixedUserName,
   const char* extraComment,
   const UserElementVector& extraElements,
   const char* userNamePrefix,
   const char* email,
   const char* name, 
   const MC2String& opt_in_name,
   const MC2String& opt_in_value,
   const MC2String& extraRights,
   const UserLicenceKey* autoKey )
{
   int status = 0;

   // Create new user with random logonID and password

   if ( clientTypeOptions == NULL ) {
      clientTypeOptions = "";
   }
   const ClientSetting* clientSetting = m_group->getSetting(
      clientType, clientTypeOptions );

   // Check for IMEI Activation code
   MC2String licenceKeyStr;
   if ( autoKey != NULL ) {
      MC2String imei15digts;
      if ( autoKey->extract15Digits( imei15digts ) ) {
         licenceKeyStr.append( "IMEI" );
         licenceKeyStr.append( imei15digts );
      } else {
         // Not even 14 digits
         mc2log << "createWayfinderUser: Not even 14 digits in IMEI key "
                << MC2CITE( autoKey->getLicenceKeyStr() ) << " not using it."
                << endl;
      }
   }
   MC2String rights;
   uint32 ownerUIN = 0;
   int acres = -5;
   if ( !licenceKeyStr.empty() ) {
      acres = m_thread->getActivationHandler()->getActivationData(
         licenceKeyStr.c_str(), rights, ownerUIN, server );
   }

   if ( acres == 0 && !server.empty() ) {
      // Redirect to right server
      status = 4;
   } else if ( acres == -1 || acres == -2 ) {
      mc2log << warn << "createWayfinderUser: failed to get "
             << "activation code \"" << licenceKeyStr << "\" Error:";
      status = 3;
      if ( acres == -2 ) {
         mc2log << " Timeout";
         status = 2;
      } else {
         mc2log << " Error";
      }
      mc2log << endl;
   } else if ( !clientSetting->getNotCreateWLUser() ) {
      // Check if we may call createWayfinderUser if not then
      // UNAUTHORIZED.
      int res = m_thread->getUserHandler()->createWayfinderUser( 
         passwd, userKey, userItem, startTime, endTime, regionID, lang,
         clientType, clientTypeOptions, programV, activationCode, extraTypes,
         createWFST, transactionDays, brand, fixedUserName, extraComment,
         extraElements, userNamePrefix, email, name,
         opt_in_name, opt_in_value, extraRights );
      if ( res != 0 ) {
         mc2log << warn << "createWayfinderUser: failed to "
                << "create Wayfinder user for licence: "
                << printLicence( userKey ) << endl;
         status = 3;
      } else {
         // userItem is set
         mc2log << info << "createWayfinderUser: created "
                << "Wayfinder user with logonID \""
                << userItem->getUser()->getLogonID() << "\""
                << "(" << userItem->getUIN() << ")"
                << " with binary key: " << printLicence( userKey )
                << endl;
         // Use the IMEI activation code
         if ( acres == 0 && ownerUIN == 0 ) {
            // Aha, it exists!
            MC2String userAgent( 
               makeUserAgentString( 
                  clientType, clientTypeOptions, programV ) );
            MC2String userInput( "Auto IMEI check on create user" );
                  
            mc2log << "createWayfinderUser: Trying upgrade for "
                   << "licence key " << licenceKeyStr << endl;
            uint32 ownerUIN = 0;
            MC2String serverStr;
            int res = m_thread->getActivationHandler()->activateUser(
               userItem->getUser(), clientSetting,
               licenceKeyStr.c_str(), 
               ""/*Phonenumber*/, MAX_INT32/*Topregionid*/, 
               m_thread->getPeerIP(), 
               userAgent.c_str(), userInput.c_str(), 
               "UNKNOWN"/*Phonemodel*/, ownerUIN, serverStr,
               true/*allowSpecialCodes*/ );

            if ( res == 0 || res == -2 ) {
               // Get user so all ids are set correctly
               uint32 uin = userItem->getUIN();
               m_thread->releaseUserItem( userItem );
               userItem = NULL;
               if ( m_thread->getUser( uin, userItem, true ) ) {
                  if ( userItem != NULL ) {
                     // DONE!
                  } else {
                     // But I just created it!?
                     mc2log << error << "createWayfinderUser: had "
                            << " uin " << uin << " ok but can't get "
                            << "user from database now! Licence "
                            << printLicence( userKey ) << endl;
                     status = 3;
                  }
               } else {
                  // Arrghhh 
                  // Comm error
                  mc2log << warn << "createWayfinderUser: failed to get "
                         << "IMEI activated user, uin " << uin 
                         << endl;
                  status = 2;
               }
            }
         } // End if res == 0 && ownerUIN == 0 from getActivationData
      } // End else when created wf user ok
   } else { // End if not not create user allowed
      // May not create wayfinder user.
      mc2log << info << "createWayfinderUser: No user with licence and "
             << "may not create new, clientType " << clientType 
             << " licence: " << printLicence( userKey ) << endl;
      status = 5;
   }

   return status;
}


uint32
ParserUserHandler::addTime( int32 years, int32 months, int32 days, 
                            uint32 explicitDate, uint32 now )
{
   uint32 endTime = now;
   if ( explicitDate != MAX_UINT32 ) {
      endTime = explicitDate;
   } else { // Use years months and days
      if ( years != 0 ) {
         endTime = StringUtility::addYears( endTime, years );
      }
      if ( months != 0 ) {
         endTime = StringUtility::addMonths( endTime, months );
      }
      if ( days != 0 ) {
         endTime += 24*60*60 * days;
      }
      if ( years == 0 && months == 0 && days == 0 ) {
         // Inf access time. Even for whole life.
         endTime = 0;
      }
   }

   return endTime;
}


bool
ParserUserHandler::getHardwareIdAndTime( const UserIDKey* el, uint32& time,
                                         MC2String& id )
{
   bool ok = false;

   if ( el->getIDType() == UserIDKey::hardware_id_and_time ) {
      MC2String::size_type findPos = el->getIDKey().find( ':' );
      if ( findPos != MC2String::npos ) {
         time = strtol( el->getIDKey().substr( 
                           findPos + 1 ).c_str(), NULL, 0 );
         ok = true;
         id = el->getIDKey().substr( 0, findPos );
      }
   }

   return ok;
}

bool
ParserUserHandler::getServiceIdResAndTime( const UserIDKey* el, uint32& time,
                                           MC2String& id, uint32& res )
{
   bool ok = false;

   if ( el->getIDType() == UserIDKey::service_id_and_time ) {
      vector<MC2String> parts( STLStringUtility::explode( ":", 
                                                          el->getIDKey() ) );
      if ( parts.size() == 3 ) {
         id = parts[ 0 ];
         time = strtol( parts[ 1 ].c_str(), NULL, 0 );
         res = strtol( parts[ 2 ].c_str(), NULL, 0 );
         ok = true;
      }
   }

   return ok;
}


UserIDKey*
ParserUserHandler::getIDKey( const UserUser* user, uint32 type,
                             const MC2String& key )
{
   UserUser::constUserElRange_t els = user->getElementRange( 
      UserConstants::TYPE_ID_KEY );
   for ( UserUser::userElVect_t::const_iterator it = els.first ; 
         it != els.second ; ++it )
   {
      const UserIDKey* el = static_cast< const UserIDKey* > ( *it );
      if ( el->getIDType() == UserIDKey::idKey_t( type ) &&
           el->getIDKey() == key ) 
      {
         return const_cast< UserIDKey* > ( el );
      }
   }
   return NULL;
}


UserIDKey*
ParserUserHandler::getIDKey( const UserUser* user, 
                             const UserIDKey* idKey )
{
   return getIDKey( user, idKey->getIDType(), idKey->getIDKey() );
}


uint32
ParserUserHandler::removeAllUseIDKeyOfType( UserUser* user, uint32 type ) {
   uint32 nbrRemoved = 0;
   
   UserUser::userElRange_t els = user->getElementRange( 
      UserConstants::TYPE_ID_KEY );
   for ( UserUser::userElVect_t::iterator it = els.first ; 
         it != els.second ; ++it )
   {
      UserIDKey* el = static_cast< UserIDKey* > ( *it );
      if ( el->getIDType() == UserIDKey::idKey_t( type ) ) {
         el->remove();
         ++nbrRemoved;
      }
   }

   return nbrRemoved;
}


bool
ParserUserHandler::checkAndUpdateLastClientAndUsage( 
   const UserUser* user,
   UserItem*& userItem,
   const MC2String& clientType, 
   const MC2String& clientTypeOptions, 
   const MC2String& version, const MC2String& extra,
   const IPnPort& origin )
{
   bool updated = false;

   // First check if last client needs to be updated
   UserUser::constUserElRange_t els = user->getElementRange(
      UserConstants::TYPE_LAST_CLIENT );

   bool lastClientMatches = false;
   for ( UserUser::userElVect_t::const_iterator it = els.first ; 
         it != els.second ; ++it ) {
      const UserLastClient* el = 
         static_cast< const UserLastClient* > ( *it );
      // Check if matches
      // And if not empty origin to update all without ip set.
      if ( !el->isHistory() &&
           el->getClientType() == clientType &&
           el->getClientTypeOptions() == clientTypeOptions &&
           el->getVersion() == version &&
           !el->getOrigin().empty() ) {
         lastClientMatches = true;
         break;
      }
   }

   // Check if latest usage needs to be updated
   bool updateLatestUsage = checkLatestUsage( user, clientType );

   if ( !lastClientMatches || updateLatestUsage ) {
      // Update!
      UserUser* cuser = new UserUser( *user );

      if ( !lastClientMatches ) {
         // Update the LastClient
         UserUser::userElRange_t els = cuser->getElementRange( 
            UserConstants::TYPE_LAST_CLIENT );
         for ( UserUser::userElVect_t::iterator it = els.first ; 
               it != els.second ; ++it ) {
            UserLastClient* el = static_cast<UserLastClient*> ( *it );
            if ( !el->isHistory() ) {
               el->remove();
            }
         }
         cuser->addElement( 
            new UserLastClient( 0, clientType, clientTypeOptions,
                                version, extra, origin.toString() ) );
      }

      if ( updateLatestUsage ) {
         // Update latest usage of the server
         bool clientTypeFound = false;
         MC2String storedClient;
         
         // Get all id keys
         UserUser::userElRange_t els = cuser->getElementRange(
            UserConstants::TYPE_ID_KEY );
         
         for ( UserUser::userElVect_t::iterator it = els.first ; 
               it != els.second ; ++it ) {
            UserIDKey* el = static_cast< UserIDKey* > ( *it );
            if ( el->getIDType() == UserIDKey::client_type_and_time ) {
               const MC2String& keyValue = el->getIDKey();
               MC2String::size_type findPos = keyValue.find( ':' );
               if ( findPos != MC2String::npos ) {
                  storedClient = keyValue.substr( 0, findPos );
               }
               
               if( storedClient == clientType ) {
                  // Update is needed
                  uint32 now = TimeUtility::getRealTime();
                  MC2String newKey( clientType + ":" + 
                                    boost::lexical_cast< MC2String > ( now ) );
                  el->setIDKey( newKey );
                  
                  clientTypeFound = true;
                  break;
               }
            }
         }
         
         if ( ! clientTypeFound ) {
            // First time usage of client type. Add usage key for it
            uint32 now = TimeUtility::getRealTime();
            MC2String newKey( clientType + ":" + 
                              boost::lexical_cast< MC2String > ( now ) );
            cuser->addElement(
               new UserIDKey( 0, newKey, UserIDKey::client_type_and_time ) );
         }
      }
      

      if ( m_thread->changeUser( cuser, NULL ) ) {
         // Changed ok
         UserItem* returnItem = NULL;
         uint32 startTime = TimeUtility::getCurrentTime();
         bool commOk = m_thread->getUser( 
            cuser->getUIN(), returnItem, true );
         uint32 endTime = TimeUtility::getCurrentTime();
         
         if ( !commOk ) {
            mc2log << warn << "checkAndUpdateLastClientAndUsage: Get uptodate "
                   << " user ("
                   << cuser->getUIN() << ") Error: ";
            if ( endTime - startTime > 3000 ) {
               mc2log << "Timeout";
            } else {
               mc2log << "Error";
            }
            mc2log << endl;
         } else {
            userItem = returnItem;
            updated = true;
         }
      } // Else failed to change user, try again next time
      
      delete cuser;
   } // End if not lastClientMacthes or updateLatesetUsage

   return updated;  
}


bool
ParserUserHandler::checkIronTrialTime( const UserUser* user, 
                                       const ClientSetting* clientSetting,
                                       UserItem*& setUserItem )
{
   bool ok = true;
   uint32 now = TimeUtility::getRealTime();
   bool changed = false;
   
   UserUser* cuser = new UserUser( *user );

   if ( m_thread->checkIfIronClient( clientSetting ) ) {
      MC2String origin( "AUTO: IRON time" );
      // Iron, make sure user has Iron right(s) (WF IRON and MyWF GOLD)
      if ( !m_thread->checkAccessToService( 
              cuser, UserEnums::UR_WF, UserEnums::UR_IRON,
              clientSetting->getCreateRegionID()) )
      {
         cuser->addElement( new UserRight(
            0, now, UserEnums::URType( UserEnums::UR_IRON,
                                       UserEnums::UR_WF ),
            clientSetting->getCreateRegionID(),
            now, 0/*Lifetime*/, false, origin.c_str() ) );
         changed = true;
         // Also special Earth clients with Extra rights added too.
         if ( ! clientSetting->getExtraRights().empty() ) {
            const MC2String& extraRights = clientSetting->getExtraRights();
            MC2String ac = "Extra Rights: " + extraRights;
            m_thread->getActivationHandler()->
               addUserRights( cuser,
                              // ignore phone and model and topRegionID
                              "", "", MAX_INT32,
                              extraRights, ac, now );
         }
      }
      if ( !m_thread->checkAccessToService( 
              cuser, UserEnums::UR_MYWAYFINDER, UserEnums::UR_GOLD,
              clientSetting->getCreateRegionID() ) )
      {
         cuser->addElement( new UserRight(
            0, now, UserEnums::URType( UserEnums::UR_GOLD,
                                       UserEnums::UR_MYWAYFINDER ),
            clientSetting->getCreateRegionID(),
            now, 0/*Lifetime*/, false, origin.c_str() ) );
         changed = true;
      }
   } else {
      // Check if to add the initial access now
      // AUTO Silver deletes trial so check for deleted too.
      // Also if initial right is deleted it is by purpose.
      UserUser::constUserElRange_t els = cuser->getElementRange(
         UserConstants::TYPE_RIGHT );
      UserEnums::URType initialWf( UserEnums::wfstAsLevel( 
                                      clientSetting->getCreateLevel() ),
                                   UserEnums::UR_WF );
      uint32 createStartTime = now - 24*60*60;
      uint32 createEndTime = now;
      uint32 createRegionID = clientSetting->getCreateRegionID();
      m_group->getCreateWFTime( clientSetting, createEndTime );
      bool noInitial = true;
      // Extra rights
      const MC2String& extraRights = clientSetting->getExtraRights();
      MC2String ac = "Extra Rights: " + extraRights;
      UserUser initialRightsUser( 0 );
      if ( !clientSetting->getExtraRights().empty() ) {
         m_thread->getActivationHandler()->addUserRights( 
            &initialRightsUser, "", "", MAX_UINT32, 
            clientSetting->getExtraRights(), ac, now );
      }
      
      // Find out what user has
      for ( UserUser::userElVect_t::const_iterator it = els.first ; 
            it != els.second ; ++it )
      {
         const UserRight* r = static_cast< const UserRight* > ( *it );
         if ( r->getUserRightType() == initialWf && 
              sameTimeSpan( r, clientSetting ) && 
              r->getRegionID() == createRegionID ) {
            noInitial = false;
         } else {
            vector<UserRight*> sameRights;
            m_thread->getMatchingRights( sameRights, &initialRightsUser, 
                                         r->getUserRightType() );
            for ( uint32 i = 0 ; i < sameRights.size() ; ++i ) {
               if ( r->getRegionID() == sameRights[ i ]->getRegionID() ) {
                  // Ok, both has it, indicate by removing it from
                  // initialRightsUser
                  sameRights[ i ]->remove();
               }
            }
         }
      }
      vector< const UserRight* > extraRightsToAdd;
      UserUser::constUserElRange_t extr = initialRightsUser.getElementRange(
         UserConstants::TYPE_RIGHT );
      for ( UserUser::userElVect_t::const_iterator it = extr.first ; 
            it != extr.second ; ++it ) {
         if ( !(*it)->removed() ) {
            extraRightsToAdd.push_back( static_cast<const UserRight*>( *it ) );
         }
      }

      if ( noInitial ) {
         MC2String origin( "AUTO: " );
         origin.append( WFSubscriptionConstants::subscriptionTypeToString(
                           initialWf.getLevelAsWFST() ) );
         origin.append( " time" );
         // No initial rights ever then add
         cuser->addElement( new UserRight(
            0, now, initialWf, createRegionID, createStartTime, createEndTime,
            false, origin.c_str() ) );
         changed = true;
         // Also special clients with Extra rights added too.
         if ( ! clientSetting->getExtraRights().empty() ) {
            m_thread->getActivationHandler()->
               addUserRights( cuser,
                              // ignore phone and model and topRegionID
                              "", "", MAX_INT32,
                              extraRights, ac, now );
         }
      } else if ( !extraRightsToAdd.empty() ) {
         // Add the missing initial rights
         for ( uint32 i = 0 ; i < extraRightsToAdd.size() ; ++i ) {
            cuser->addElement( new UserRight( *extraRightsToAdd[ i ] ) );
         }
         changed = true;
      }
   }

   if ( changed ) {
      if ( !m_thread->changeUser( cuser, NULL/*changer*/ ) ) {
         mc2log << warn << "checkIronTrialTime: " 
                << "failed to change user " << user->getLogonID()
                << "(" << user->getUIN() << ")" << endl;
      }

      // Update to new user with correct IDs
      UserItem* returnItem = NULL;
      uint32 startTime = TimeUtility::getCurrentTime();
      bool commOk = m_thread->getUser( 
         cuser->getUIN(), returnItem, true );
      uint32 endTime = TimeUtility::getCurrentTime();
         
      if ( !commOk ) {
         mc2log << warn << "checkIronTrialTime: Get uptodate "
                << " user (" << cuser->getUIN() << ") Error: ";
         if ( endTime - startTime > 3000 ) {
            mc2log << "Timeout";
         } else {
            mc2log << "Error";
         }
         mc2log << endl;
      } else {
         setUserItem = returnItem;
      }
   }

   delete cuser;

   return ok;
}

bool
ParserUserHandler::checkLatestUsage(
   const UserUser*& user,
   const MC2String& clientType ) {
   
   bool updateNeeded = false;
   bool clientTypeFound = false;
   uint32 time = 0;
   MC2String storedClient;

   // Get all id keys
   UserUser::constUserElRange_t els = user->getElementRange(
      UserConstants::TYPE_ID_KEY );

   for ( UserUser::userElVect_t::const_iterator it = els.first ; 
         it != els.second ; ++it ) {
      const UserIDKey* el = static_cast< const UserIDKey* > ( *it );
      if ( el->getIDType() == UserIDKey::client_type_and_time ) {
         const MC2String& keyValue = el->getIDKey();
         MC2String::size_type findPos = keyValue.find( ':' );
         if ( findPos != MC2String::npos ) {
            time = strtol( keyValue.substr( 
                              findPos + 1 ).c_str(), NULL, 0 );
            storedClient = keyValue.substr( 0, findPos );
         }

         if( storedClient == clientType ) {
            // Client already saved. Check date if update is needed
            uint32 currentTime = TimeUtility::getRealTime();
            if ( currentTime - time >= 86400 ) {
               // It is 24 h or more since last usage, update the value
               updateNeeded = true;
            }
            
            clientTypeFound = true;
            break;
         }
      }
   }
   
   return updateNeeded || !clientTypeFound;
}
   

void
ParserUserHandler::makePassword( MC2String& passwd ) {
   for ( uint32 i = 0 ; i < 6 ; ++i ) {
      passwd += char( 65 + uint32( ( 25.0*rand() / (RAND_MAX + 1.0) ) ) );
   }
}


uint32
ParserUserHandler::getUserFavCRC( uint32 uin) {
   UserFavoritesRequest req( m_thread->getNextRequestID(), uin );

   m_thread->putRequest( &req );

   vector<uint32> crcArray;
   if ( req.getStatus() == StringTable::OK ) {
      // get all CRCs
      const UserFavorite* fav = req.getAddFav();
      while( fav != NULL) {
         crcArray.push_back( fav->getCRC() );
         fav = req.getAddFav();
      }
   }

   // calculate one CRC from all CRCs
   return MC2CRC32::crc32( reinterpret_cast<const byte*>(&crcArray[0]), 
                           crcArray.size() * sizeof( uint32 ) );
}


int
ParserUserHandler::licenceTo( const UserUser* user, 
                              const UserLicenceKey* licence )
{
   int res = 0;
   // Check if all is ok first
   if ( findUserLicenceKey( user, licence ) != NULL ) {
      return res;
   }

   if ( res == 0 ) {
      // Check device changes
      if ( !user->mayChangeDevice() ) {
         mc2log << warn << "licenceTo: User may not change BK, User "
                << user->getLogonID() 
                << "(" << user->getUIN() << ")" ;
         mc2log << endl;
         res = -3;
      }
   }

   if ( res == 0 ) {
      // Send licence and uin to UM and let it fix it.
      // No concurrency issues if two calls to licenceTo at the same time
      // with the same user(s) involved
      auto_ptr<PacketContainer> pc( 
         m_thread->putRequest( 
            new LicenceToRequestPacket( user->getUIN(), licence ),
            MODULE_TYPE_USER ) );
      uint32 status = pc.get() != NULL ? 
         static_cast< ReplyPacket* > ( pc->getPacket() )->getStatus() :
         StringTable::TIMEOUT_ERROR;
      if ( status == StringTable::OK ) {
         // OK!
         LicenceToReplyPacket* r = static_cast< LicenceToReplyPacket* > ( 
            pc->getPacket() );
         // Clean some user's from cache (this and possibly owner of key)
         m_group->removeUserFromCache( user->getUIN() );
         if ( r->getOwnerUIN() != user->getUIN() && r->getOwnerUIN() != 0 ) {
            m_group->removeUserFromCache( r->getOwnerUIN() );
         }
         mc2log << info << "licenceTo: added licence to user " 
                << user->getLogonID() << "(" << user->getUIN() 
                << ") licence " << printLicence( licence ) << endl;
      } else {
         mc2log << info << "licenceTo: failed to add licence to user " 
                << user->getLogonID() << "(" << user->getUIN() << ")" 
                << " will try again next session licence "
                << printLicence( licence ) << " Error: ";
         if ( status == StringTable::NOT_ALLOWED ) {
            // Not allowed to move from current owner
            res = -3;
            mc2log << "Owner of BK may not change BK";
         } else if ( status == StringTable::TIMEOUT_ERROR ) {
            res = -2;
            mc2log << "Timeout";
         } else {
            res = -1;
            mc2log << "Error";
         }
         mc2log << endl;
      }
   } // Else already error

   return res;
}

uint32
ParserUserHandler::getOldestRightTime( const UserUser* user ) const {
   uint32 oldestTime = MAX_UINT32;

   UserUser::constUserElRange_t els = user->getElementRange( 
      UserConstants::TYPE_RIGHT );
   for ( UserUser::userElVect_t::const_iterator it = els.first ; 
         it != els.second ; ++it ) {
      const UserRight* r = static_cast< const UserRight* > ( *it );
      if ( r->getAddTime() < oldestTime ) {
         oldestTime = r->getAddTime();
      }
   }

   return oldestTime;
}

uint32 
ParserUserHandler::getEndTimeForUser( 
   const UserUser* user, const ClientSetting* clientSetting,
   UserEnums::userRightLevel levelmask ) const 
{
   int32 endTime = MAX_INT32;
   if ( user->getValidDate() != 0 ) { // 0 means inf
      endTime = user->getValidDate();
   }
   // Also check region accesses
   int32 highestRegionTime = 0;
   if ( user->getNbrOfType( UserConstants::TYPE_RIGHT ) > 0 ) {
      UserEnums::URType type( levelmask,
                              UserEnums::UR_WF );
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_RIGHT ) ; i++ ) 
      {
         const UserRight* r = static_cast<const UserRight*> ( 
            user->getElementOfType( i, UserConstants::TYPE_RIGHT ) );
         if ( !r->isDeleted() && 
              r->getUserRightType().levelAndServiceMatchMask( type ) &&
              validFor( r, clientSetting ) ) {
            if ( int32(r->getEndTime()) > highestRegionTime ) {
               highestRegionTime = int32(r->getEndTime());
            } else if ( r->getEndTime() == 0 ) {
               // Lifetime
               highestRegionTime = MAX_INT32;
            }
         }
      }
   } else { // XXX: Old region way
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( 
            UserConstants::TYPE_REGION_ACCESS ) ; i++ )
   {
      UserRegionAccess* region = static_cast< UserRegionAccess* > (
         user->getElementOfType( 
            i, UserConstants::TYPE_REGION_ACCESS ) );
      if ( int32(region->getEndTime()) > highestRegionTime ) {
         highestRegionTime = int32(region->getEndTime());
      }
   }
   } // End else region way
   if ( highestRegionTime < endTime ) {
      endTime = highestRegionTime;
   }
   // If client type is to be blocked any time soon
   if ( int32(clientSetting->getBlockDate()) < endTime ) {
      endTime = clientSetting->getBlockDate();
   }

   return endTime;
}


uint32
ParserUserHandler::getNbrDaysLeftForUser( 
   const UserUser* user, const ClientSetting* clientSetting,
   uint32 maxNbrDays ) const
{
   uint32 now = TimeUtility::getRealTime();
   int32 endTime = getEndTimeForUser( user, clientSetting, 
                                      UserEnums::ALL_LEVEL_MASK );
   uint32 days = MAX_UINT32;

   if ( endTime - int32(now) <= int32(maxNbrDays)*24*3600 ) {
      if ( endTime < int32(now) ) {
         endTime = int32(now);
      }
      days = uint32( rint( float64( endTime - int32( now ) ) / 
                           float64( 24*3600 ) ) );
   }

   return days;
}

int
ParserUserHandler::updateUser( UserItem*& userItem ) {
   int res = 0;

   UserItem* setUserItem = NULL;
   uint32 startTime = TimeUtility::getCurrentTime();
   if ( m_thread->getUser( userItem->getUIN(), setUserItem, true ) ) {
      if ( setUserItem != NULL ) {
         m_thread->releaseUserItem( userItem );
         userItem = setUserItem;
      } else {
         mc2log << warn << "[PUH] updateUser no "
                << "user!?" << endl;
         res = -1;
      }
   } else {
      res = -1;
      mc2log << warn << "[PUH] updateUser "
             << "failed to get uptodate user: ";
      uint32 endTime = TimeUtility::getCurrentTime();
      if ( (endTime-startTime) > 2000 ) {
         res = -2;
         mc2log << "Timeout";
      } else {
         mc2log << "Error";
      }
      mc2log << endl;
   }

   return res;
}

bool
ParserUserHandler::makeLicenceKeyStr( MC2String& str,
                                      const MC2String& hardwareType, 
                                      const MC2String& hardwareKey ) const 
{
   bool ok = true;

   if ( ParserUserHandler::validHardwareKeyTypes.find( hardwareType ) != 
        ParserUserHandler::validHardwareKeyTypes.end() ) {
      if ( hardwareType != "imei" ) {
         // Separator
         str.append( hardwareType );
         str.append( ":" );
      }
      str.append( hardwareKey );
   } else {
      ok = false;
      mc2log << warn << "[PU]:makeLicenceKeyStr Licence type is "
             << "not valid type. " << MC2CITE( hardwareType ) << endl;
   }

   return ok;
}

bool
ParserUserHandler::makeLicenceKeyStr( MC2String& str,
                                      const UserLicenceKey& hwKey ) const {
   return makeLicenceKeyStr( str, hwKey.getKeyType(), 
                             hwKey.getLicenceKeyStr() );
}


bool
ParserUserHandler::setBestLicenceKey( const vector< UserLicenceKey >& hwKeys, 
                                      UserLicenceKey& hwKey ) {
   if ( !hwKeys.empty() ) {
      uint32 bestIndex = NBR_ITEMS( validTypesArray );
      UserLicenceKeyVect::const_iterator findIt = hwKeys.begin();
      for ( UserLicenceKeyVect::const_iterator it = hwKeys.begin();
            it != hwKeys.end() ; ++it ) {
         for ( uint32 i = 0 ; i < NBR_ITEMS( validTypesArray ) ; ++i ) {
            if ( it->getKeyType() == validTypesArray[ i ] ) {
               if ( i < bestIndex ) {
                  bestIndex = i;
                  findIt = it;
               }
               break;
            }
         }
      }
      hwKey = *findIt;
      
      return true;
   } else {
      return false;
   }
}


UserLicenceKey* 
ParserUserHandler::findUserLicenceKey( UserUser* user, 
                                       const UserLicenceKey* userKey ) const
{
   UserLicenceKey findKey( *userKey );
   // Check trimmed IMEI?
   if ( userKey->isIMEIKey() ) {
      MC2String imei15digts;
      if ( userKey->extract15Digits( imei15digts ) && 
           imei15digts != userKey->getLicenceKeyStr() ) {
         findKey.setLicence( imei15digts );
      } // End if needs to check trimmed IMEI
   } // End if is IMEI key

   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_LICENCE_KEY ) ; ++i )
   {
      UserLicenceKey* licence = static_cast< UserLicenceKey* > ( 
         user->getElementOfType( i, UserConstants::TYPE_LICENCE_KEY ) );
      if ( findKey.compare( *licence ) ) {
         // Found it!
         return licence;
      }
   } // End for all user's licences

   return NULL;
}


const UserLicenceKey*
ParserUserHandler::findUserLicenceKey( 
   const UserUser* user, const UserLicenceKey* userKey ) const
{
   return findUserLicenceKey( const_cast<UserUser*>( user ), userKey );
}


int
ParserUserHandler::getNbrProductKeys( const UserUser* user, 
                                      const MC2String& product ) const {
   return UserLicenceKeyProductCounter( product ).getNbrKeys(
      user->getElementRange( UserConstants::TYPE_LICENCE_KEY ) );
}

ConstUserLicenceKeyPVect
ParserUserHandler::getProductKeys( const UserUser* user,
                                   const MC2String& product ) const {
   ConstUserLicenceKeyPVect res;
   UserUser::constUserElRange_t els = user->getElementRange( 
      UserConstants::TYPE_LICENCE_KEY );
   for ( UserUser::userElVect_t::const_iterator it = els.first ; 
         it != els.second ; ++it ) {
      if ( static_cast< const UserLicenceKey* > ( *it )->getProduct() == 
           product ) {
         res.push_back( static_cast< const UserLicenceKey* > ( *it ) );
      }
   }
   return res;
}

UserLicenceKeyPVect
ParserUserHandler::getProductKeys( UserUser* user,
                                   const MC2String& product ) const {
   UserLicenceKeyPVect res;
   UserUser::userElRange_t els = user->getElementRange( 
      UserConstants::TYPE_LICENCE_KEY );
   for ( UserUser::userElVect_t::iterator it = els.first ; 
         it != els.second ; ++it ) {
      if ( static_cast< UserLicenceKey* > ( *it )->getProduct() == 
           product ) {
         res.push_back( static_cast< UserLicenceKey* > ( *it ) );
      }
   }
   return res;
}

MC2String
ParserUserHandler::makeUserAgentString( const char* clientType,
                                        const char* clientTypeOptions,
                                        const uint32* cpvArray )
{
   MC2String userAgent;
   char tmpStr[ 256 ];
   bool nothingAdded = true;

   if ( clientType != NULL ) {
      userAgent.append( clientType );
      nothingAdded = false;
   }
   if ( cpvArray != NULL ) {
      if ( !nothingAdded ) {
         userAgent.append( ", " );
      }
      sprintf( tmpStr, "%u", cpvArray[ 0 ] );
      userAgent.append( tmpStr ); 
      userAgent.append( "." );
      sprintf( tmpStr, "%u", cpvArray[ 1 ] );
      userAgent.append( tmpStr ); 
      userAgent.append( "." );
      sprintf( tmpStr, "%u", cpvArray[ 2 ] );
      userAgent.append( tmpStr );
      nothingAdded = false;
   }
   if ( clientTypeOptions != NULL ) {
      if ( !nothingAdded ) {
         userAgent.append( ", " );
      }
      userAgent.append( clientTypeOptions );
      nothingAdded = false;
   }
   if ( nothingAdded ) {
      userAgent.append( "none" );
   }

   return userAgent;
}

bool
ParserUserHandler::getUserFromLicenceKeys( const UserLicenceKeyVect& hwKeys,
                                           UserItem*& hardwareUser, 
                                           uint32& nbrUsers,
                                           bool wipeFromCache )
{
   bool ok = true;
   const UserLicenceKey* firstKey = NULL;
   const UserLicenceKey* hwKey = NULL;
   for ( uint32 i = 0 ; i < NBR_ITEMS( validTypesArray ) && ok ; ++i ) {
      hwKey = InterfaceRequestData::getLicenceKeyType(
         hwKeys, validTypesArray[ i ] );
      if ( hwKey != NULL ) {
         if ( firstKey == NULL ) {
            firstKey = hwKey;
         }
         UserLicenceKey userKey( *hwKey );

         nbrUsers = 0;
         if ( !m_thread->getUserFromUserElement( 
                 &userKey, hardwareUser, nbrUsers, true, wipeFromCache ) ) {
            ok = false;
            mc2log << "ParserUserHandler::getUserFromLicenceKeys "
                   << "getUserFromUserElement failed for '" 
                   << userKey << "'" << endl;
         } else if ( nbrUsers == 1 && hardwareUser == NULL ) {
            ok = false;
            mc2log  << "ParserUserHandler::getUserFromLicenceKeys Failed to "
                    << "get user for key (" << userKey << ")" << endl;
         } else if( nbrUsers > 1 ) {
            // More than one user
            mc2log << warn << "XMLParserThread::checkAuthorization"
                   << "More than one user, " << nbrUsers << ", with the same "
                   << "licence key '" << userKey << "'" << endl;
            ok = false;
         } else {
            // Ok
            if( nbrUsers == 0 ) {
               hardwareUser = NULL;
               mc2dbg << "ParserUserHandler::getUserFromLicenceKeys No user "
                      << "found for licence key '" << userKey << "'" 
                      << endl;
            } else {
               mc2log << "ParserUserHandler::getUserFromLicenceKeys Found "
                      << "user "
                      << hardwareUser->getUIN() << " using licence key '" 
                      << userKey << "'" << endl;
               // Found!
               break;
            }
         }
      } // End if have key of type
   } // End for all key Types in prio order

   //// Check if highest prio key is used
   //if ( ok && hardwareUser != NULL && firstKey != NULL && hwKey != firstKey ) {
   //   licenceTo( hardwareUser->getUser(), firstKey );
   //   // If this fails we try again so ignore errors
   //}

   return ok;
}

bool
ParserUserHandler::getUserFromIDKeyValue( const MC2String idKeyValue, 
                                          UserItem*& userItem, 
                                          uint32& nbrUsers,
                                          bool useCache,
                                          bool wipeFromCache )
{
   UserIDKey userIDKey( MAX_UINT32 );
   userIDKey.setIDKey( idKeyValue.c_str() );
   userIDKey.setIDType( UserIDKey::account_id_key );
   return m_thread->getUserFromUserElement( 
      &userIDKey, userItem, nbrUsers, useCache, wipeFromCache );
}

ParserUserHandler::errorCode
ParserUserHandler::idKeyTo( const UserUser* user, const UserIDKey* idKey,
                            const UserIDKey* removeIDkey ) {
   errorCode res = OK;
   // Check if all is ok first
   if ( getIDKey( user, idKey ) != NULL ) {
      return res;
   }

   // Send idKey and UIN to UM and let it fix it.
   auto_ptr<PacketContainer> pc( 
      m_thread->putRequest( 
         new IDKeyToRequestPacket( user->getUIN(), idKey, removeIDkey ),
         MODULE_TYPE_USER ) );
   uint32 status = pc.get() != NULL ? 
      static_cast< ReplyPacket* > ( pc->getPacket() )->getStatus() :
      StringTable::TIMEOUT_ERROR;
   if ( status == StringTable::OK ) {
      // OK!
      IDKeyToReplyPacket* r = static_cast< IDKeyToReplyPacket* > ( 
         pc->getPacket() );
      // Clean some user's from cache (this and possibly owner of key)
      m_group->removeUserFromCache( user->getUIN() );
      if ( r->getOwnerUIN() != user->getUIN() && r->getOwnerUIN() != 0 ) {
         m_group->removeUserFromCache( r->getOwnerUIN() );
      }
      mc2log << info << "idKeyTo: added idKey to user " 
             << user->getLogonID() << "(" << user->getUIN() 
             << ") idKey " << *idKey << endl;
   } else {
      mc2log << info << "idKeyTo: failed to add idKey to user " 
             << user->getLogonID() << "(" << user->getUIN() << ")" 
             << " idKey " << idKey << " Error: ";
      if ( status == StringTable::TIMEOUT_ERROR ) {
         res = TIMEOUT;
         mc2log << "Timeout";
      } else {
         res = ERROR;
         mc2log << "Error";
      }
      mc2log << endl;
   }

   return res;
}


int
ParserUserHandler::getStoredUserData( uint32 uin, const MC2String& key,
                                      MC2String& value ) {
   int res = 0;

   auto_ptr< RequestPacket > 
      studReqPacket( new GetStoredUserDataRequestPacket( uin ) );
   vector< SQLDataContainer::NameAndIsNum > rowNames;
   rowNames.push_back( SQLDataContainer::NameAndIsNum( "userUIN", true) );
   rowNames.push_back( SQLDataContainer::NameAndIsNum( "dataKey", false ) );
   
   vector< MC2String > rowValues;
   rowValues.push_back( STLStringUtility::uint2str( uin ) );
   rowValues.push_back( key );

   SQLDataContainer cont( rowNames, rowValues );
   int pos = USER_REQUEST_HEADER_SIZE;
   cont.save( studReqPacket.get(), pos );

   auto_ptr< PacketContainer >
      pc( m_thread->putRequest( studReqPacket.release(), MODULE_TYPE_USER ) );
   if ( pc.get() != NULL && static_cast< ReplyPacket* > ( 
           pc->getPacket() )->getStatus() == StringTable::OK ) {
      // Add data to reply
      pos = USER_REPLY_HEADER_SIZE;
      cont.load( pc->getPacket(), pos );
      MC2String keyName;
      // Check if the key and value exist and add them
      if( cont.getCol( 0, "dataKey", keyName ) && 
          cont.getCol( 0, "dataValue", value ) ) {
         // value is set
      } else {
         // No such key in user
         res = 1;
      }
   } else {
      res = -1;
      if ( pc.get() == NULL ) {
         res = -2;
      }
   }

   return res;
}

int
ParserUserHandler::setStoredUserData( uint32 uin, const MC2String& key,
                                      const MC2String& value ) {
   int res = 0;

   auto_ptr< RequestPacket > 
      studReqPacket( new SetStoredUserDataRequestPacket( uin ) );
   
   vector< SQLDataContainer::NameAndIsNum > rowNames;
   rowNames.push_back( SQLDataContainer::NameAndIsNum( "userUIN", true) );
   rowNames.push_back( SQLDataContainer::NameAndIsNum( "dataKey", false ) );
   rowNames.push_back( SQLDataContainer::NameAndIsNum( "dataValue", false ) );
   
   vector< MC2String > rowValues;
   rowValues.push_back( STLStringUtility::uint2str( uin ) );
   rowValues.push_back( key );
   rowValues.push_back( value );

   SQLDataContainer cont( rowNames, rowValues );
   int pos = USER_REQUEST_HEADER_SIZE;
   cont.save( studReqPacket.get(), pos );

   auto_ptr< PacketContainer >
      pc( m_thread->putRequest( studReqPacket.release(), MODULE_TYPE_USER ) );
   if ( pc.get() != NULL && static_cast< ReplyPacket* > ( 
           pc->getPacket() )->getStatus() == StringTable::OK ) {
      // Ok
   } else {
      res = -1;
      if ( pc.get() == NULL ) {
         res = -2;
      }
   }

   return res;
}

bool
ParserUserHandler::sameTimeSpan( const UserRight* r,
                                 const ClientSetting* clientSetting ) const {
   uint32 endTime = r->getStartTime() + 24*60*60;
   m_group->getCreateWFTime( clientSetting, endTime );

   return abs(int32(endTime) - int32(r->getEndTime())) < 2*24*60*60 ;
}
