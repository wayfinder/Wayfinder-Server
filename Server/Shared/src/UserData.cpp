/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "UserData.h"
#include "UserPacket.h"
#include "StringUtility.h"
#include "RegionIDs.h"
#include "MapRights.h"

#include "UserElement.h"
#include "UserLicenceKey.h"
#include "UserRight.h"
#include "UserIDKey.h"

#include "TimeUtility.h"
#include "MapBits.h"
#include "STLUtility.h"

#include <typeinfo>

UserItem::UserItem(uint32 ID)
 : CacheElement(ID, USER_ELEMENT_TYPE, SERVER_TYPE)
{
   m_removed = false;

   m_valid = false;

   m_userUser = NULL;
   m_timeStamp = TimeUtility::getRealTime();
}


UserItem::UserItem( const UserReplyPacket* p ) 
      : CacheElement( p->getUIN(), 
                      USER_ELEMENT_TYPE, 
                      SERVER_TYPE) 
{
   m_removed = false;
   m_userUser = NULL;
   m_timeStamp = TimeUtility::getRealTime();
   
   if ( p->getStatus() == StringTable::OK ) {
      m_valid = true;

      switch ( p->getSubType() ) {
         case Packet::PACKETTYPE_USERGETDATAREPLY : {
            if ( !decodePacket( (GetUserDataReplyPacket*) p ) ) {
               m_valid = false;
               mc2dbg1 << "UserItem::UserItem( p ) " 
                      << " decoding failed" << endl;
            }
            break;
         }
         default: 
            mc2dbg1 << "UserItem::UserItem( p ) "
                   << (int) p->getSubType() << " subtype not supported"
                   << endl;
            m_valid = false;
            break;
      }
   } else {
      m_valid = false;  
      mc2dbg1 << "UserItem::UserItem( UserReplyPacket* p ) "
              << "Packet is NOT OK!" << endl;
   }
}


UserItem::UserItem( UserUser* userUser ) 
      : CacheElement( userUser->getUIN(), 
                      USER_ELEMENT_TYPE, 
                      SERVER_TYPE) 
{
   m_removed = false;
   m_userUser = userUser;   
   m_valid = true;
   m_timeStamp = TimeUtility::getRealTime();
}


UserItem::UserItem( const UserItem& userItem )
   : CacheElement(userItem)
{
   m_removed = userItem.m_removed;
   m_userUser = new UserUser(* userItem.m_userUser);
   m_valid = userItem.m_valid;
   m_timeStamp = userItem.m_timeStamp;
}


UserItem::~UserItem()
{
   delete m_userUser;
}


bool
UserItem::decodePacket( GetUserDataReplyPacket* p ) {
   int pos = USER_REQUEST_HEADER_SIZE;
   if ( (uint32)(pos + 3) >= p->getLength() ) {
      DEBUG1(mc2log << error << "UserItem::decodePacket not room for header" 
             << endl;);
      return false;
   }
   uint16 nbrFields = p->getNbrFields();

// This error check commented
//    if ( nbrFields > 5000 ) {
//       DEBUG1(mc2log << error << "UserItem::decodePacket more than 5000 fields"
//              << " giving up" << endl;);
//       return false;
//    }

   pos = p->getFirstElementPosition();
   UserElement* elem = NULL;
   for ( uint16 i = 0 ; i < nbrFields ; i++ ) {
      elem = p->getNextElement( pos );
      if ( elem != NULL ) {
         switch ( elem->getType() ) {
            case UserConstants::TYPE_USER:
               delete m_userUser;
               m_userUser = dynamic_cast<UserUser*>(elem);
               if ( elem == NULL ) {
                  m_valid = false;
               } else {
                  m_valid = true;
               }
               break;
            case UserConstants::TYPE_CELLULAR: 
               if ( m_userUser != NULL ) {
                  UserCellular* cellular = dynamic_cast<UserCellular*> 
                     ( elem );
                  if ( cellular != NULL )
                     m_userUser->addElement( cellular );
                  else {
                     DEBUG1(mc2log << error << "UserItem::decodePacket " 
                             << "cellular not cellular!" << endl;)
                     delete elem;
                     elem = NULL;
                  }
               } else {
                  DEBUG1(mc2log << error << "UserItem::decodePacket " 
                          << "UserCellular but no UserUser!" << endl;);
                  delete elem;
                  elem = NULL;
               }
               break;
            case UserConstants::TYPE_BUDDY:
               if ( m_userUser != NULL ) {
                  UserBuddyList* buddy = dynamic_cast<UserBuddyList*> 
                     ( elem );
                  if ( buddy != NULL )
                     m_userUser->addElement( buddy );
                  else {
                     DEBUG1(mc2log << error << "UserItem::decodePacket " 
                            << "buddy not buddy!" << endl;);
                     delete elem;
                     elem = NULL;
                  }
               } else {
                  DEBUG1(mc2log << error << "UserItem::decodePacket " 
                          << "UserBuddyList but no UserUser!" << endl;);
                  delete elem;
                  elem = NULL;
               }
               break;
            case UserConstants::TYPE_NAVIGATOR:
               if ( m_userUser != NULL ) {
                  UserNavigator* navigator = dynamic_cast<UserNavigator*> 
                     ( elem );
                  if ( navigator != NULL )
                     m_userUser->addElement( navigator );
                  else {
                     DEBUG1(mc2log << error << "UserItem::decodePacket " 
                             << "navigator not navigator!" << endl;);
                     delete elem;
                     elem = NULL;
                  }
               } else {
                  DEBUG1(mc2log << error << "UserItem::decodePacket " 
                         << "UserNavigator but no UserUser!" << endl;);
                  delete elem;
                  elem = NULL;
               }
               break;
            case UserConstants::TYPE_LICENCE_KEY: 
               if ( m_userUser != NULL ) {
                  UserLicenceKey* licence = dynamic_cast<UserLicenceKey*> 
                     ( elem );
                  if ( licence != NULL )
                     m_userUser->addElement( licence );
                  else {
                     mc2log << error << "UserItem::decodePacket " 
                            << "licence not licence!" << endl;
                     delete elem;
                     elem = NULL;
                  }
               } else {
                  mc2log << error << "UserItem::decodePacket " 
                         << "UserLicenceKey but no UserUser!" << endl;
                  delete elem;
                  elem = NULL;
               }
               break;
            case UserConstants::TYPE_REGION_ACCESS: 
               if ( m_userUser != NULL ) {
                  UserRegionAccess* access =dynamic_cast<UserRegionAccess*>
                     ( elem );
                  if ( access != NULL )
                     m_userUser->addElement( access );
                  else {
                     mc2log << error << "UserItem::decodePacket " 
                            << "region access not region access!" << endl;
                     delete elem;
                     elem = NULL;
                  }
               } else {
                  mc2log << error << "UserItem::decodePacket " 
                         << "UserRegionAccess but no UserUser!" << endl;
                  delete elem;
                  elem = NULL;
               }
               break;
            case UserConstants::TYPE_RIGHT: 
               if ( m_userUser != NULL ) {
                  UserRight* r = dynamic_cast<UserRight*> ( elem );
                  if ( r != NULL ) {
                     m_userUser->addElement( r );
                  } else {
                     mc2log << error << "UserItem::decodePacket " 
                            << "user right not user right!" << endl;
                     delete elem;
                     elem = NULL;
                  }
               } else {
                  mc2log << error << "UserItem::decodePacket " 
                         << "UserRight but no UserUser!" << endl;
                  delete elem;
                  elem = NULL;
               }
               break;
            case UserConstants::TYPE_WAYFINDER_SUBSCRIPTION :
               if ( m_userUser != NULL ) {
                  UserWayfinderSubscription* subscr = 
                     dynamic_cast<UserWayfinderSubscription*> ( elem );
                  if ( subscr != NULL )
                     m_userUser->addElement( subscr );
                  else {
                     mc2log << error << "UserItem::decodePacket " 
                            << "WayfinderSubscription not "
                            << "WayfinderSubscription!" << endl;
                     delete elem;
                     elem = NULL;
                  }
               } else {
                  mc2log << error << "UserItem::decodePacket " 
                         << "UserWayfinderSubscription but no UserUser!"
                         << endl;
                  delete elem;
                  elem = NULL;
               }
               break;
            case UserConstants::TYPE_TOKEN :
               if ( m_userUser != NULL ) {
                  UserToken* t = 
                     dynamic_cast<UserToken*> ( elem );
                  if ( t != NULL )
                     m_userUser->addElement( t );
                  else {
                     mc2log << error << "UserItem::decodePacket " 
                            << "UserToken not UserToken!" << endl;
                     delete elem;
                     elem = NULL;
                  }
               } else {
                  mc2log << error << "UserItem::decodePacket " 
                         << "UserToken but no UserUser!"
                         << endl;
                  delete elem;
                  elem = NULL;
               }
               break;
            case UserConstants::TYPE_PIN :
               if ( m_userUser != NULL ) {
                  UserPIN* t = 
                     dynamic_cast<UserPIN*> ( elem );
                  if ( t != NULL )
                     m_userUser->addElement( t );
                  else {
                     mc2log << error << "UserItem::decodePacket " 
                            << "UserPIN not UserPIN!" << endl;
                     delete elem;
                     elem = NULL;
                  }
               } else {
                  mc2log << error << "UserItem::decodePacket " 
                         << "UserPIN but no UserUser!"
                         << endl;
                  delete elem;
                  elem = NULL;
               }
               break;
            case UserConstants::TYPE_ID_KEY :
               if ( m_userUser != NULL ) {
                  UserIDKey* t = dynamic_cast<UserIDKey*> ( elem );
                  if ( t != NULL )
                     m_userUser->addElement( t );
                  else {
                     mc2log << error << "UserItem::decodePacket " 
                            << "UserIDKey not UserIDKey!" << endl;
                     delete elem;
                     elem = NULL;
                  }
               } else {
                  mc2log << error << "UserItem::decodePacket " 
                         << "UserIDKey but no UserUser!"
                         << endl;
                  delete elem;
                  elem = NULL;
               }
               break;
            case UserConstants::TYPE_LAST_CLIENT :
               if ( m_userUser != NULL ) {
                  UserLastClient* t = dynamic_cast<UserLastClient*> ( 
                     elem );
                  if ( t != NULL )
                     m_userUser->addElement( t );
                  else {
                     mc2log << error << "UserItem::decodePacket " 
                            << "UserLastClient not UserLastClient!" 
                            << endl;
                     delete elem;
                     elem = NULL;
                  }
               } else {
                  mc2log << error << "UserItem::decodePacket " 
                         << "UserLastClient but no UserUser!"
                         << endl;
                  delete elem;
                  elem = NULL;
               }
               break;
            default:
               DEBUG1(mc2log << warn << "UserItem::decodePacket Unknown type "
                      << (uint32) elem->getType() << " skipping element" 
                      << endl;);
               delete elem;
               elem = NULL;
               break;
         }
         
      } else {
         DEBUG2(mc2log << warn << "UserItem::decodePacket getNextElement "
                << "returned NULL" << endl;);
      }

   }
   return true;
   PANIC("UserItem::decodePacket ", "Not implemented");
}
   

uint32 
UserItem::getSize() const
{
   uint32 size;
   
   // Booleans m_removed and m_valid
   size = sizeof(bool)*( 1 + 1 );
   
   if ( m_userUser != NULL )
      size += m_userUser->getSize();

   return size;
}


uint32 
UserItem::getUIN() const {
   return getID();
}


void
UserItem::remove()
{
   m_removed = true;
}


bool
UserItem::removed() const
{
   return m_removed;
}


// === UserUser ===========================================
void
UserUser::addUserElement( UserElement* el )
{
   m_userElements[el->getType()].push_back(el);
}

void
UserUser::addElement( UserElement* elem ) {
   addUserElement( elem );
}

namespace {
   /// Empty vector to reference when requesting a non-existant range.
   static vector<UserElement*> emptyUserElementVector(0);
   /// Empty vector to reference when requesting a non-existant range.
   static const vector<UserElement*> constEmptyUserElementVector(0);
}

UserUser::userElRange_t
UserUser::getElementRange( UserConstants::UserItemType type )
{
#if 1
   userElMap_t::iterator it = m_userElements.find( type );
   if ( it == m_userElements.end() ) {
      return userElRange_t( emptyUserElementVector.begin(),
                            emptyUserElementVector.end() );
   } else {
      return userElRange_t( it->second.begin(), it->second.end() );
   }
#else
   // Following row will insert empty vector if needed.
   vector<UserElement*>& tmp = m_userElements[type];
   return userElRange_t( tmp.begin(),
                         tmp.end() );
#endif
}

UserUser::constUserElRange_t
UserUser::getElementRange( UserConstants::UserItemType type ) const
{
   userElMap_t::const_iterator it = m_userElements.find( type );
   if ( it == m_userElements.end() ) {
      return constUserElRange_t( constEmptyUserElementVector.begin(),
                                 constEmptyUserElementVector.end() );
   } else {
      return constUserElRange_t( it->second.begin(), it->second.end() );
   }
}

// Private procedure to set everything to something.
void
UserUser::initUser() {
   mc2dbg4 << "UserUser::initUser()" << endl;

   m_logonID = new char[1];
   m_logonID[0] = '\0';
   m_firstname = new char[1];
   m_firstname[0] = '\0';
   m_initials = new char[1];
   m_initials[0] = '\0';
   m_lastname = new char[1];
   m_lastname[0] = '\0';
   m_session = new char[1];
   m_session[0] = '\0';
   m_measurementSystem = UserConstants::MEASUREMENTTYPE_METRIC;
   m_language = StringTable::ENGLISH;
   m_birthDate= new char[1];
   m_birthDate[0] = '\0';
   m_routeImageType = UserConstants::ROUTEIMAGETYPE_JAVA_APPLET;
   m_validDate = 0;
   m_gender = UserConstants::GENDERTYPE_MALE;
   
   m_lastdest_mapID = MAX_UINT32;
   m_lastdest_itemID = MAX_UINT32;
   m_lastdest_offset = MAX_UINT16;
   m_lastdest_time = MAX_UINT32;
   m_lastdest_string = new char[1];
   m_lastdest_string[0] = '\0';
   m_lastorig_mapID = MAX_UINT32;
   m_lastorig_itemID = MAX_UINT32;
   m_lastorig_offset = MAX_UINT16;
   m_lastorig_time = MAX_UINT32;
   m_lastorig_string = new char[1];
   m_lastorig_string[0] = '\0';

   m_searchType = SearchTypes::CloseMatch;
   m_searchSubstring = SearchTypes::BeginningOfWord;
   m_searchSorting = SearchTypes::ConfidenceSort;
   m_searchObject = SEARCH_STREETS | SEARCH_COMPANIES | SEARCH_MISC
      | SEARCH_MUNICIPALS | SEARCH_BUILT_UP_AREAS | SEARCH_ZIP_CODES;
   m_searchDbMask = SearchTypes::DefaultDB;
   
   m_routingCostA = 0;
   m_routingCostB = 1;
   m_routingCostC = 0;
   m_routingCostD = 0;
   m_routingType = 0;
   m_routingVehicle = ItemTypes::passengerCar;
   
   m_editMapRights = 0;
   m_editDelayRights = 0;
   m_editUserRights = 0;

   m_wapService = false;
   m_htmlService = false;
   m_operatorService = false;
   m_smsService = false;

   m_default_country = new char[1];
   m_default_country[0] = '\0';
                               
   m_default_municipal = new char[1];
   m_default_municipal[0] = '\0';

   m_default_city = new char[1];
   m_default_city[0] = '\0';

   m_navService = false;
   m_operatorComment = StringUtility::newStrDup( "" );
   m_emailAddress = StringUtility::newStrDup( "" );
   m_address1 = StringUtility::newStrDup( "" );
   m_address2 = StringUtility::newStrDup( "" );
   m_address3 = StringUtility::newStrDup( "" );
   m_address4 = StringUtility::newStrDup( "" );
   m_address5 = StringUtility::newStrDup( "" );
   m_routeTurnImageType = UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM;
   m_externalXmlService = false;
   m_transactionBased = UserConstants::NO_TRANSACTIONS;
   m_deviceChanges = -1;
   m_supportComment = StringUtility::newStrDup( "" );
   m_postalCity = StringUtility::newStrDup( "" );
   m_zipCode = StringUtility::newStrDup( "" );
   m_companyName = StringUtility::newStrDup( "" );
   m_companyReference = StringUtility::newStrDup( "" );
   m_companyVATNbr = StringUtility::newStrDup( "" );
   m_emailBounces = 0;
   m_addressBounces = 0;
   m_customerContactInfo = StringUtility::newStrDup( "" );

   m_nbrMunicipal = 0;
   m_municipal = new uint32[1];
   
   uint32 i;
   for ( i = 0 ; i < UserConstants::TABLE_NBR ; i++ ) {
      m_valid[i] = false;
   }

   for ( i = 0 ; i < UserConstants::USER_NBRFIELDS ; i++ ) {
      m_changed[i] = false;
   }
}


UserUser::UserUser( uint32 UIN )
      : UserElement( UserConstants::TYPE_USER )
{
   mc2dbg4 << "UserUser::UserUser()" << endl;
   m_UIN = UIN;

   setOk( true );
   initUser();
}


UserUser::UserUser( uint32 UIN,
                    const char* logonID,
                    const char* firstname,
                    const char* initials,
                    const char* lastname,
                    const char* session,
                    UserConstants::MeasurementType measurementSystem,
                    StringTable::languageCode language,
                    uint32 lastdest_mapID,
                    uint32 lastdest_itemID,
                    uint16 lastdest_offset,
                    uint32 lastdest_time,
                    const char* lastdest_string,
                    uint32 lastorig_mapID,
                    uint32 lastorig_itemID,
                    uint16 lastorig_offset,
                    uint32 lastorig_time,
                    const char* lastorig_string,
                    const char* birthDate,
                    uint8 searchType,
                    uint8 searchSubstring,
                    uint8 searchSorting,
                    uint32 searchObject,
                    uint8 dbMask,
                    byte routingCostA,
                    byte routingCostB,
                    byte routingCostC,
                    byte routingCostD,
                    byte routingType,
                    ItemTypes::vehicle_t routingVehicle,
                    uint32 editMapRights,
                    uint32 editDelayRights,
                    uint32 editUserRights,
                    bool wapService,
                    bool htmlService,
                    bool operatorService,
                    int32 nbrMunicipal,
                    uint32* municipal,
                    UserConstants::RouteImageType routeImageType,
                    uint32 validDate,
                    UserConstants::GenderType gender,
                    bool smsService,
                    const char* default_country,
                    const char* default_municipal,
                    const char* default_city,
                    bool navService,
                    const char* operatorComment,
                    const char* emailAddress,
                    const char* address1,
                    const char* address2,
                    const char* address3,
                    const char* address4,
                    const char* address5,
                    UserConstants::RouteTurnImageType routeTurnImageType,
                    bool externalXmlService,
                    UserConstants::transactionBased_t transactionBased,
                    int32 deviceChanges,
                    const char* supportComment,
                    const char* postalCity,
                    const char* zipCode,
                    const char* companyName,
                    const char* companyReference,
                    const char* companyVATNbr,
                    int32 emailBounces,
                    int32 addressBounces,
                    const char* customerContactInfo )
      : UserElement( UserConstants::TYPE_USER )
{
   m_UIN = UIN;
   m_logonID = new char[strlen(logonID) + 1];
   strcpy( m_logonID, logonID);
   m_firstname = new char[strlen(firstname) + 1];
   strcpy( m_firstname, firstname);
   m_initials = new char[strlen(initials) + 1];
   strcpy( m_initials, initials);
   m_lastname = new char[strlen(lastname) + 1];
   strcpy( m_lastname, lastname);
   m_session = new char[strlen(session) + 1];
   strcpy( m_session, session);
   m_measurementSystem = measurementSystem;
   m_language = language;
   m_birthDate = new char[strlen(birthDate) + 1];
   strcpy( m_birthDate, birthDate);
   m_routeImageType = routeImageType;
   m_validDate = validDate;
   m_gender = gender;
   
   m_lastdest_mapID = lastdest_mapID;
   m_lastdest_itemID = lastdest_itemID;
   m_lastdest_offset = lastdest_offset;
   m_lastdest_time = lastdest_time;
   m_lastdest_string = new char[strlen(lastdest_string) + 1];
   strcpy( m_lastdest_string, lastdest_string);
   m_lastorig_mapID = lastorig_mapID;
   m_lastorig_itemID = lastorig_itemID;
   m_lastorig_offset = lastorig_offset;
   m_lastorig_time = lastorig_time;
   m_lastorig_string = new char[strlen(lastorig_string) + 1];
   strcpy( m_lastorig_string, lastorig_string);
   
   m_searchObject = searchObject;
   m_searchType = searchType;
   m_searchSubstring = searchSubstring;
   m_searchSorting = searchSorting;
   m_searchDbMask = dbMask;
   
   m_routingCostA = routingCostA;
   m_routingCostB = routingCostB;
   m_routingCostC = routingCostC;
   m_routingCostD = routingCostD;
   m_routingType = routingType;
   m_routingVehicle = routingVehicle;
   
   m_editMapRights = editMapRights;
   m_editDelayRights = editDelayRights;
   m_editUserRights = editUserRights;

   m_wapService = wapService;
   m_htmlService = htmlService;
   m_operatorService = operatorService;
   m_smsService = smsService;

   m_default_country = new char[strlen(default_country) + 1];
   strcpy( m_default_country, default_country);
   m_default_municipal = new char[strlen(default_municipal) + 1];
   strcpy( m_default_municipal, default_municipal);
   m_default_city = new char[strlen(default_city) + 1];
   strcpy( m_default_city, default_city);

   m_navService = navService;
   m_operatorComment = StringUtility::newStrDup( operatorComment );
   m_emailAddress = StringUtility::newStrDup( emailAddress );
   m_address1 = StringUtility::newStrDup( address1 );
   m_address2 = StringUtility::newStrDup( address2 );
   m_address3 = StringUtility::newStrDup( address3 );
   m_address4 = StringUtility::newStrDup( address4 );
   m_address5 = StringUtility::newStrDup( address5 );
   m_routeTurnImageType = routeTurnImageType;
   m_externalXmlService = externalXmlService;
   m_transactionBased = transactionBased;
   m_deviceChanges = deviceChanges;
   m_supportComment = StringUtility::newStrDup( supportComment );
   m_postalCity = StringUtility::newStrDup( postalCity );
   m_zipCode = StringUtility::newStrDup( zipCode );
   m_companyName = StringUtility::newStrDup( companyName );
   m_companyReference = StringUtility::newStrDup( companyReference );
   m_companyVATNbr = StringUtility::newStrDup( companyVATNbr );
   m_emailBounces = emailBounces;
   m_addressBounces = addressBounces;
   m_customerContactInfo = StringUtility::newStrDup( customerContactInfo );

   m_nbrMunicipal = nbrMunicipal;
   m_municipal = municipal;
   uint32 j = 0;
   for (  j = 0 ; j < UserConstants::TABLE_NBR ; j++ ) {
      m_valid[j] = true;
   }

   for ( j = 0 ; j < UserConstants::USER_NBRFIELDS ; j++ ) {
      m_changed[j] = false;
   }

   setOk( true );
}


UserUser::UserUser(const UserReplyPacket* p, int& pos)
      : UserElement( UserConstants::TYPE_USER )
{
   mc2dbg4 << "UserUser::UserUser(Packet*, " << pos << ")" << endl;
   
   DEBUG4(p->dump(););
   
   initUser();
   
   setOk( readFromPacket( p, pos ) );
}


bool
UserUser::readFromPacket( const Packet* p , int& pos ) {
   int32 startPos = pos;

   uint32 type = p->incReadLong( pos );

   if ( type == UserConstants::TYPE_USER ) {
      uint32 size = p->incReadShort( pos );
      if ( (12+75 < size) && ((startPos - 6 + size) < p->getLength()) ) { 
         // 12 is the two start TYPE_USER and size (Really 16 with padding)
         uint32 tmpID = p->incReadLong( pos );
         if ( tmpID != reinterpret_cast<const UserReplyPacket*>(p)->getUIN() ) {
            DEBUG1(mc2log << error << "UserUser::UserUser( p, pos ) " 
                   << "Not correct UIN confused, giving up: " << tmpID
                   << "!=" 
                   << reinterpret_cast<const UserReplyPacket*>(p)->getUIN() 
                   << endl;);
            return false;
         }
         m_UIN = tmpID;
         m_measurementSystem = 
            UserConstants::MeasurementType( p->incReadLong( pos ) );
         m_language = StringTable::languageCode( p->incReadLong( pos ) );

         m_lastdest_mapID = p->incReadLong( pos );
         m_lastdest_itemID = p->incReadLong( pos );
         m_lastdest_offset = p->incReadShort( pos );         
         m_lastdest_time = p->incReadLong( pos );         

         m_lastorig_mapID = p->incReadLong( pos );
         m_lastorig_itemID = p->incReadLong( pos );
         m_lastorig_offset = p->incReadShort( pos );
         m_lastorig_time = p->incReadLong( pos );
         m_searchObject = p->incReadLong( pos );

         m_searchType = p->incReadByte( pos );
         m_searchSubstring = p->incReadByte( pos );
         m_searchSorting = p->incReadByte( pos );
         m_searchDbMask = p->incReadByte( pos );

         m_routingCostA = p->incReadByte( pos );
         m_routingCostB = p->incReadByte( pos );
         m_routingCostC = p->incReadByte( pos );
         m_routingCostD = p->incReadByte( pos );
         m_routingVehicle = ItemTypes::vehicle_t( p->incReadLong( pos ) );
         m_routingType = p->incReadByte( pos );

         m_routeImageType =
            UserConstants::RouteImageType( p->incReadLong( pos ) );

         m_gender = UserConstants::GenderType( p->incReadLong( pos ) );
         
         m_validDate = p->incReadLong( pos );


         m_editMapRights = p->incReadLong( pos );
         m_editDelayRights = p->incReadLong( pos );
         m_editUserRights = p->incReadLong( pos );

         m_wapService = p->incReadByte( pos ) != 0;
         m_htmlService = p->incReadByte( pos )  != 0;
         m_operatorService = p->incReadByte( pos )  != 0;
         m_smsService = p->incReadByte( pos )  != 0;

   
         m_nbrMunicipal = p->incReadLong( pos );
         if ( m_nbrMunicipal > 1000 ) {
            DEBUG1(mc2log << error <<  "UserUser::UserUser( p, pos ) " 
                   << "More than 1000 municipals! " << m_nbrMunicipal
                   << " confused giving up" << endl;);
            return false;
         }
         delete [] m_municipal; // Delete default empty vector
         m_municipal = new uint32[m_nbrMunicipal+1];
         for ( int32 i = 0 ; i < m_nbrMunicipal ; i++ ) {
            m_municipal[i] = p->incReadLong( pos );
         }

         char* str;
         int strLen;
         int tmpLen;
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_logonID;
         m_logonID = new char[strLen + 1];
         strcpy( m_logonID, str );

         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_firstname;
         m_firstname = new char[strLen + 1];
         strcpy( m_firstname, str );

         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_initials;
         m_initials = new char[strLen + 1];
         strcpy( m_initials, str );
         
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_lastname;
         m_lastname = new char[strLen + 1];
         strcpy( m_lastname, str );
         
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_session;
         m_session = new char[strLen + 1];
         strcpy( m_session, str );
         
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_lastdest_string;
         m_lastdest_string = new char[strLen + 1];
         strcpy( m_lastdest_string, str );

         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_lastorig_string;
         m_lastorig_string = new char[strLen + 1];
         strcpy( m_lastorig_string, str );

         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn<< "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_birthDate;
         m_birthDate = new char[strLen + 1];
         strcpy( m_birthDate, str );

         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_default_country;
         m_default_country = new char[strLen +1];
         strcpy( m_default_country, str );
         
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_default_municipal;
         m_default_municipal = new char[strLen +1];
         strcpy( m_default_municipal, str );
         
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_default_city;
         m_default_city = new char[strLen +1];
         strcpy( m_default_city, str );

         // navService
         m_navService = p->incReadByte( pos ) != 0;
         
         // operatorComment
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_operatorComment;
         m_operatorComment = StringUtility::newStrDup( str );

         // emailAddress
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_emailAddress;
         m_emailAddress = StringUtility::newStrDup( str );

         // address1
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_address1;
         m_address1 = StringUtility::newStrDup( str );

         // address2
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_address2;
         m_address2 = StringUtility::newStrDup( str );

         // address3
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_address3;
         m_address3 = StringUtility::newStrDup( str );

         // address4
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_address4;
         m_address4 = StringUtility::newStrDup( str );

         // address5
         tmpLen = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( tmpLen != strLen ) {
            DEBUG1(mc2log << warn << "UserUser(p,pos) " << "StrLenError"
                   << endl);
            return false;
         }
         delete [] m_address5;
         m_address5 = StringUtility::newStrDup( str );

         // routeTurnImageType
         m_routeTurnImageType = UserConstants::RouteTurnImageType( 
            p->incReadByte( pos ) );

         // externalXmlService
         m_externalXmlService = p->incReadByte( pos ) != 0;

         // transactionBased
         m_transactionBased = UserConstants::transactionBased_t(
            p->incReadByte( pos ) );

         // deviceChanges
         m_deviceChanges = p->incReadLong( pos );

         // supportComment
         delete [] m_supportComment;
         m_supportComment = StringUtility::newStrDup(
            p->incReadString( pos ) );

         // postalCity
         delete [] m_postalCity;
         m_postalCity = StringUtility::newStrDup( 
            p->incReadString( pos ) );

         // zipCode
         delete [] m_zipCode;
         m_zipCode = StringUtility::newStrDup( p->incReadString( pos ) );

         //companyName 
         delete [] m_companyName;
         m_companyName = StringUtility::newStrDup(
            p->incReadString( pos ) );

         // companyReference
         delete [] m_companyReference;
         m_companyReference = StringUtility::newStrDup(
            p->incReadString( pos ) );

         // companyVATNbr
         delete [] m_companyVATNbr;
         m_companyVATNbr = StringUtility::newStrDup(
            p->incReadString( pos ) );

         // emailBounces
         m_emailBounces = p->incReadLong( pos );

         // addressBounces
         m_addressBounces = p->incReadLong( pos );

         // customerContactInfo
         delete [] m_customerContactInfo;
         m_customerContactInfo = StringUtility::newStrDup(
            p->incReadString( pos ) );

       
         // Read Type last
         type = p->incReadLong( pos );
         if ( type != UserConstants::TYPE_USER ) {
            DEBUG1(mc2log << error  
                   << "UserUser::UserUser( UserReplyPacket* p, pos ) "
                   << "Not TYPE_USER last, not OK" << endl;);
            return false;
         }
         
         // Done
         mc2dbg4 << "UserUser::UserUser( UserReplyPacket* p, pos ) "
                 << " done at pos " << pos << endl;
         return true;
      } else {
         DEBUG1(mc2log << warn 
                << "UserUser::UserUser( UserReplyPacket* p, pos ) "
                << "Not size for data!! Nothing read" << endl;);
         return false;
      }
   } else {
      DEBUG1( mc2log << warn 
              << "UserUser::UserUSER(  UserReplyPacket* p, pos ) "
              << "Not TYPE_USER type!!!!! " << (int) type << endl;);
      return false;
   }   
}


// Deletes all allocated variables
UserUser::~UserUser()
{
   mc2dbg8 << "UserUser::~UserUser()" << endl;
   
   // Delete all strings
   delete [] m_logonID;
   delete [] m_firstname;  
   delete [] m_initials;
   delete [] m_lastname;
   delete [] m_session;
   delete [] m_lastdest_string;
   delete [] m_lastorig_string;
   delete [] m_birthDate;
   delete [] m_default_country;
   delete [] m_default_municipal;
   delete [] m_default_city;
   delete [] m_operatorComment;
   delete [] m_emailAddress;
   delete [] m_address1;
   delete [] m_address2;
   delete [] m_address3;
   delete [] m_address4;
   delete [] m_address5;
   delete [] m_supportComment;
   delete [] m_postalCity;
   delete [] m_zipCode;
   delete [] m_companyName;
   delete [] m_companyReference;
   delete [] m_companyVATNbr;
   delete [] m_customerContactInfo;


   // Delete all elements
   for ( userElMap_t::iterator it = m_userElements.begin();
         it != m_userElements.end();
         ++it ) {
      STLUtility::deleteValues( it->second );
   }
   // Delete m_municipal
   delete [] m_municipal;
}


UserUser::UserUser( const UserUser& user )
      : UserElement( user ) ,
        m_userRightPerMap ( user.m_userRightPerMap ),
        m_userRightsPerTopRegion ( user.m_userRightsPerTopRegion )
   
 {
   m_UIN = user.m_UIN;
   m_logonID = StringUtility::newStrDup( user.m_logonID );
   m_firstname = StringUtility::newStrDup( user.m_firstname );
   m_initials = StringUtility::newStrDup( user.m_initials );
   m_lastname = StringUtility::newStrDup( user.m_lastname );
   m_session = StringUtility::newStrDup( user.m_session );
   m_measurementSystem = user.m_measurementSystem;
   m_language = user.m_language;

   m_lastdest_mapID = user.m_lastdest_mapID;
   m_lastdest_itemID = user.m_lastdest_itemID;
   m_lastdest_offset = user.m_lastdest_offset;
   m_lastdest_time = user.m_lastdest_time;
   m_lastdest_string = StringUtility::newStrDup( user.m_lastdest_string );
   m_lastorig_mapID = user.m_lastorig_mapID;
   m_lastorig_itemID = user.m_lastorig_itemID;
   m_lastorig_offset = user.m_lastorig_offset;
   m_lastorig_time = user.m_lastorig_time;
   m_lastorig_string = StringUtility::newStrDup( user.m_lastorig_string );
   m_birthDate = StringUtility::newStrDup( user.m_birthDate );
   m_routeImageType = user.m_routeImageType;
   m_validDate = user.m_validDate;
   m_gender = user.m_gender;

   m_searchType = user.m_searchType;
   m_searchSubstring = user.m_searchSubstring;
   m_searchSorting = user.m_searchSorting;
   m_searchObject = user.m_searchObject;
   m_searchDbMask = user.m_searchDbMask;

   m_routingCostA = user.m_routingCostA;
   m_routingCostB = user.m_routingCostB;
   m_routingCostC = user.m_routingCostC;
   m_routingCostD = user.m_routingCostD;
   m_routingVehicle = user.m_routingVehicle;
   m_routingType = user.m_routingType;
   
   m_editMapRights = user.m_editMapRights;
   m_editDelayRights = user.m_editDelayRights;
   m_editUserRights = user.m_editUserRights;
   m_wapService = user.m_wapService;
   m_htmlService = user.m_htmlService;
   m_operatorService = user.m_operatorService;
   m_smsService = user.m_smsService;

   m_default_country = StringUtility::newStrDup( user.m_default_country );
   m_default_municipal = StringUtility::newStrDup( user.m_default_municipal );
   m_default_city = StringUtility::newStrDup( user.m_default_city );

   m_navService = user.m_navService;
   m_operatorComment = StringUtility::newStrDup( user.m_operatorComment );
   m_emailAddress = StringUtility::newStrDup( user.m_emailAddress );
   m_address1 = StringUtility::newStrDup( user.m_address1 );
   m_address2 = StringUtility::newStrDup( user.m_address2 );
   m_address3 = StringUtility::newStrDup( user.m_address3 );
   m_address4 = StringUtility::newStrDup( user.m_address4 );
   m_address5 = StringUtility::newStrDup( user.m_address5 );
   m_routeTurnImageType = user.m_routeTurnImageType;
   m_externalXmlService = user.m_externalXmlService;
   m_transactionBased = user.m_transactionBased;
   m_deviceChanges = user.m_deviceChanges;
   m_supportComment = StringUtility::newStrDup( user.m_supportComment );
   m_postalCity = StringUtility::newStrDup( user.m_postalCity );
   m_zipCode = StringUtility::newStrDup( user.m_zipCode );
   m_companyName = StringUtility::newStrDup( user.m_companyName );
   m_companyReference = StringUtility::newStrDup( 
      user.m_companyReference );
   m_companyVATNbr = StringUtility::newStrDup( user.m_companyVATNbr );
   m_emailBounces = user.m_emailBounces;
   m_addressBounces = user.m_addressBounces;
   m_customerContactInfo = StringUtility::newStrDup( 
      user.m_customerContactInfo );
   
   m_nbrMunicipal = user.m_nbrMunicipal;
   m_municipal = new uint32[ m_nbrMunicipal ];
   int32 i = 0;
   for ( i = 0; i < m_nbrMunicipal ; i++ ) {
      m_municipal[ i ] = user.m_municipal[ i ];
   }
   
   for ( i = 0 ; i < UserConstants::USER_NBRFIELDS ; i++ ) {
      m_changed[ i ] = user.m_changed[ i ];
   }

   for ( i = 0 ; i < UserConstants::TYPE_NBR ; i++ ) {
      m_valid[ i ] = user.m_valid[ i ];
   }
   
   for ( userElMap_t::const_iterator it = user.m_userElements.begin();
         it != user.m_userElements.end();
         ++it ) {
      for( vector<UserElement*>::const_iterator jt = it->second.begin();
           jt != it->second.end(); ++jt ) {
      UserElement* el = *jt;
      if ( (&typeid(*el) == &typeid(UserUser)) ) {
         el = new UserUser( *(static_cast<UserUser*>( el ) ) );
      } else if ( &typeid(*el) == &typeid(UserCellular) ) {
         el = new UserCellular( *(static_cast<UserCellular*>( el ) ) );
      } else if ( &typeid( *el ) == &typeid( UserBuddyList ) ) {
         el = new UserBuddyList( *(static_cast<UserBuddyList*>( el ) ) );
      } else if (  &typeid( *el ) == &typeid( UserNavigator ) ) {
         el = new UserNavigator( *(static_cast<UserNavigator*>( el ) ) );
      } else if ( &typeid(*el) == &typeid(UserLicenceKey) ) {
         el = new UserLicenceKey( *(static_cast<UserLicenceKey*>( el ) ) );
      } else if ( &typeid(*el) == &typeid(UserRegionAccess) ) {
         el = new UserRegionAccess( 
            *(static_cast<UserRegionAccess*>( el ) ) );
      } else if ( &typeid(*el) == &typeid(UserRight) ) {
         el = new UserRight( *(static_cast<UserRight*>( el ) ) );
      } else if ( &typeid(*el) == &typeid(UserWayfinderSubscription) ) {
         el = new UserWayfinderSubscription( 
            *(static_cast<UserWayfinderSubscription*>( el ) ) );
      } else if ( &typeid(*el) == &typeid(UserToken) ) {
         el = new UserToken( *(static_cast<UserToken*>( el ) ) );
      } else if ( &typeid(*el) == &typeid(UserPIN) ) {
         el = new UserPIN( *(static_cast<UserPIN*>( el ) ) );
      } else if ( &typeid(*el) == &typeid(UserIDKey) ) {
         el = new UserIDKey( *(static_cast<UserIDKey*>( el ) ) );
      } else if ( &typeid(*el) == &typeid(UserLastClient) ) {
         el = new UserLastClient( *(static_cast<UserLastClient*>( el ) ) );
      } else {
         mc2log << error << "UserUser::UserUser unknown UserElement!" << endl;
      }
      addUserElement( el );
      }
   }

}


uint32 
UserUser::getSize() const
{
   mc2dbg4 << "UserUser::getSize()" << endl;
   return (18 + m_nbrMunicipal)*sizeof( uint32 ) 
                               // UIN, 3*2 lastSearch, measurement
                              // language, 3 edit, 1 vehcle, nbrMunicipals,
                             // searchObject
      + 2*sizeof( uint16 )   // 1*2 lastSearch
      + 8*sizeof( uint8 )   // Search[Type, StringPart, Sorting], 5 routing
      + 4*sizeof( bool )   // wapService, htmlService, operatorService
      + 7*sizeof( uint16 )// 7 string lengths
      + strlen( m_logonID ) + 1 + strlen(  m_firstname ) + 1
      + strlen( m_initials ) + 1 + strlen( m_lastname ) + 1
      + strlen( m_session ) + 1
      + strlen( m_lastdest_string ) + 1 + strlen( m_lastorig_string ) + 1 +
      strlen( m_birthDate ) + 1 + strlen( m_default_country ) + 1 +
      strlen( m_default_municipal ) + 1 + strlen( m_default_city ) + 1
      + 1 /*navService*/ + strlen( m_operatorComment ) + 3 + 
      strlen( m_emailAddress ) + 3 + strlen( m_address1 ) + 3
      + strlen( m_address2 ) + 3 + strlen( m_address3 ) + 3 
      + strlen( m_address4 ) + 3 + strlen( m_address5 ) + 3 
      + 1 /*routeTurnImageType*/ + 1 /*externalXmlService*/ 
      + 1 /*transactionBased*/
      + 7 /*padding + deviceChanges */
      + strlen( m_supportComment ) + 1 + strlen( m_postalCity ) + 1 
      + strlen( m_zipCode ) + 1 + strlen( m_companyName ) + 1 
      + strlen( m_companyReference ) + 1 + strlen( m_companyVATNbr ) + 1 
      + 7 /*padding + emailBounces */ + 4 /*addressBounces*/
      + strlen( m_customerContactInfo ) + 1 
      ;
}


void
UserUser::packInto( Packet* p, int& pos ) {
   mc2dbg4 << "UserUser::packInto( p, " << pos << " )" << endl;

   uint32 size = getSize();
   size += 16 + 8; // Header and footer

   uint32 logonIDSize = strlen( m_logonID );
   uint32 firstnameSize = strlen(  m_firstname );
   uint32 initialsSize = strlen( m_initials );
   uint32 lastnameSize = strlen( m_lastname );
   uint32 sessionSize = strlen( m_session );
   uint32 lastdest_stringSize = strlen( m_lastdest_string );
   uint32 lastorig_stringSize = strlen( m_lastorig_string );
   uint32 birthDateSize = strlen( m_birthDate );
   uint32 default_countrySize = strlen( m_default_country );
   uint32 default_municipalSize = strlen( m_default_municipal );
   uint32 default_citySize = strlen( m_default_city );
   uint32 operatorCommentSize = strlen( m_operatorComment );
   uint32 emailAddressSize = strlen( m_emailAddress );
   uint32 address1Size = strlen( m_address1 );
   uint32 address2Size = strlen( m_address2 );
   uint32 address3Size = strlen( m_address3 );
   uint32 address4Size = strlen( m_address4 );
   uint32 address5Size = strlen( m_address5 );

   
   p->incWriteLong( pos, UserConstants::TYPE_USER );
   p->incWriteShort( pos, size );
   p->incWriteLong( pos, UserConstants::TYPE_USER );
   p->incWriteShort( pos, size );
   
   p->incWriteLong( pos, m_UIN );
   p->incWriteLong( pos, m_measurementSystem );
   p->incWriteLong( pos, m_language );

   p->incWriteLong( pos, m_lastdest_mapID );
   p->incWriteLong( pos, m_lastdest_itemID  );
   p->incWriteShort( pos, m_lastdest_offset );
   p->incWriteLong( pos, m_lastdest_time );

   p->incWriteLong( pos, m_lastorig_mapID );
   p->incWriteLong( pos, m_lastorig_itemID  );
   p->incWriteShort( pos, m_lastorig_offset );
   p->incWriteLong( pos, m_lastorig_time );

   p->incWriteLong( pos, m_searchObject );

   p->incWriteByte( pos, m_searchType );
   p->incWriteByte( pos, m_searchSubstring );
   p->incWriteByte( pos, m_searchSorting );
   p->incWriteByte( pos, m_searchDbMask );

   p->incWriteByte( pos, m_routingCostA );
   p->incWriteByte( pos, m_routingCostB );
   p->incWriteByte( pos, m_routingCostC );
   p->incWriteByte( pos, m_routingCostD );
   p->incWriteLong( pos, m_routingVehicle );
   p->incWriteByte( pos, m_routingType );

   p->incWriteLong( pos, m_routeImageType);
   p->incWriteLong( pos, m_gender );
   p->incWriteLong( pos, m_validDate );

   p->incWriteLong( pos, m_editMapRights );
   p->incWriteLong( pos, m_editDelayRights );
   p->incWriteLong( pos, m_editUserRights );

   p->incWriteByte( pos, m_wapService );
   p->incWriteByte( pos, m_htmlService );
   p->incWriteByte( pos, m_operatorService );
   p->incWriteByte( pos, m_smsService );

   p->incWriteLong( pos, m_nbrMunicipal );
   for ( int32 i = 0 ;  i < m_nbrMunicipal ; i++ ) {
      p->incWriteLong( pos, m_municipal[i] ); 
   }
   
   p->incWriteShort( pos, logonIDSize );
   p->incWriteString( pos, m_logonID );

   p->incWriteShort( pos, firstnameSize );
   p->incWriteString( pos, m_firstname );

   p->incWriteShort( pos, initialsSize );
   p->incWriteString( pos, m_initials );

   p->incWriteShort( pos, lastnameSize );
   p->incWriteString( pos, m_lastname );

   p->incWriteShort( pos, sessionSize );
   p->incWriteString( pos, m_session );

   p->incWriteShort( pos, lastdest_stringSize);
   p->incWriteString( pos, m_lastdest_string );

   p->incWriteShort( pos, lastorig_stringSize);
   p->incWriteString( pos, m_lastorig_string );

   p->incWriteShort( pos, birthDateSize);
   p->incWriteString( pos, m_birthDate );
   
   p->incWriteShort( pos, default_countrySize );
   p->incWriteString( pos, m_default_country );

   p->incWriteShort( pos, default_municipalSize );
   p->incWriteString( pos, m_default_municipal );

   p->incWriteShort( pos, default_citySize );
   p->incWriteString( pos, m_default_city ); 

   p->incWriteByte( pos, m_navService );

   p->incWriteShort( pos, operatorCommentSize );
   p->incWriteString( pos,  m_operatorComment);

   p->incWriteShort( pos, emailAddressSize );
   p->incWriteString( pos, m_emailAddress );

   p->incWriteShort( pos, address1Size );
   p->incWriteString( pos, m_address1 );

   p->incWriteShort( pos, address2Size );
   p->incWriteString( pos, m_address2 );

   p->incWriteShort( pos, address3Size );
   p->incWriteString( pos, m_address3 );

   p->incWriteShort( pos, address4Size );
   p->incWriteString( pos, m_address4 );

   p->incWriteShort( pos, address5Size );
   p->incWriteString( pos, m_address5 );

   p->incWriteByte( pos, m_routeTurnImageType );

   p->incWriteByte( pos, m_externalXmlService );

   p->incWriteByte( pos, m_transactionBased );

   p->incWriteLong( pos, m_deviceChanges );
   p->incWriteString( pos, m_supportComment );
   p->incWriteString( pos, m_postalCity );
   p->incWriteString( pos, m_zipCode );
   p->incWriteString( pos, m_companyName );
   p->incWriteString( pos, m_companyReference );
   p->incWriteString( pos, m_companyVATNbr );
   p->incWriteLong( pos, m_emailBounces );
   p->incWriteLong( pos, m_addressBounces );
   p->incWriteString( pos, m_customerContactInfo );

   p->incWriteLong( pos, UserConstants::TYPE_USER );

   mc2dbg8 << "Internal UserUser should end @ " << pos << endl;

   p->incWriteLong( pos, UserConstants::TYPE_USER );

   mc2dbg8 << "Next should start at @ " << pos << endl;

   // Set length
   p->setLength( pos );
}


void
UserUser::addChanges(Packet* p, int& position)
{
   mc2dbg4 << "UserUser::addChanges pos " << position << endl;
   uint8 nbrChanges = 0;
   for (uint8 i = 0; i < UserConstants::USER_NBRFIELDS; i++)
      if (m_changed[i])
         nbrChanges++;
   if (nbrChanges>0) {
      p->incWriteLong(position, UserConstants::TYPE_USER);
      p->incWriteLong(position, m_UIN);
      p->incWriteLong(position, UserConstants::TYPE_USER);
      p->incWriteLong(position, m_UIN);
      if (removed()) {
         p->incWriteByte(position, UserConstants::ACTION_DELETE);
      }
      else if (m_UIN == 0) {
         p->incWriteByte(position, UserConstants::ACTION_NEW);
         p->incWriteByte(position, UserConstants::USER_NBRFIELDS - 1);
         p->incWriteByte(position, UserConstants::USER_LOGONID);
         p->incWriteString(position, m_logonID);
         p->incWriteByte(position, UserConstants::USER_FIRSTNAME);
         p->incWriteString(position, m_firstname);
         p->incWriteByte(position, UserConstants::USER_INITIALS);
         p->incWriteString(position, m_initials);
         p->incWriteByte(position, UserConstants::USER_LASTNAME);
         p->incWriteString(position, m_lastname);
         p->incWriteByte(position, UserConstants::USER_SESSION);
         p->incWriteString(position, m_session);
         p->incWriteByte(position, UserConstants::USER_MEASUREMENT_SYSTEM);
         p->incWriteLong(position, m_measurementSystem);
         p->incWriteByte(position, UserConstants::USER_LANGUAGE);
         p->incWriteLong(position, m_language);
         // Dest
         p->incWriteByte(position, UserConstants::USER_LASTDEST_MAPID);
         p->incWriteLong(position, m_lastdest_mapID);
         p->incWriteByte(position, UserConstants::USER_LASTDEST_ITEMID);
         p->incWriteLong(position, m_lastdest_itemID);
         p->incWriteByte(position, UserConstants::USER_LASTDEST_OFFSET);
         p->incWriteShort(position, m_lastdest_offset);
         p->incWriteByte(position, UserConstants::USER_LASTDEST_TIME);
         p->incWriteLong(position, m_lastdest_time);
         p->incWriteByte(position, UserConstants::USER_LASTDEST_STRING);
         p->incWriteString(position, m_lastdest_string);
          // Birthdate
         p->incWriteByte(position, UserConstants::USER_BIRTHDATE);
         p->incWriteString(position, m_birthDate);
          // Orig
         p->incWriteByte(position, UserConstants::USER_LASTORIG_MAPID);
         p->incWriteLong(position, m_lastorig_mapID);
         p->incWriteByte(position, UserConstants::USER_LASTORIG_ITEMID);
         p->incWriteLong(position, m_lastorig_itemID);
         p->incWriteByte(position, UserConstants::USER_LASTORIG_OFFSET);
         p->incWriteShort(position, m_lastorig_offset);
         p->incWriteByte(position, UserConstants::USER_LASTORIG_TIME);
         p->incWriteLong(position, m_lastorig_time);
         p->incWriteByte(position, UserConstants::USER_LASTORIG_STRING);
         p->incWriteString(position, m_lastorig_string); 
         // Search
         p->incWriteByte(position, UserConstants::USER_SEARCH_OBJECTS);
         p->incWriteLong(position, m_searchObject);
         p->incWriteByte(position, UserConstants::USER_SEARCH_TYPE);
         p->incWriteByte(position, m_searchType);
         p->incWriteByte(position, UserConstants::USER_SEARCH_SUBSTRING);
         p->incWriteByte(position, m_searchSubstring);
         p->incWriteByte(position, UserConstants::USER_SEARCH_SORTING);
         p->incWriteByte(position, m_searchSorting);
         p->incWriteByte(position, UserConstants::USER_SEARCH_DBMASK);
         p->incWriteByte(position, m_searchDbMask);
         
         // Route
         p->incWriteByte(position, UserConstants::USER_ROUTING_COST_A);
         p->incWriteByte(position, m_routingCostA);
         p->incWriteByte(position, UserConstants::USER_ROUTING_COST_B);
         p->incWriteByte(position, m_routingCostB);
         p->incWriteByte(position, UserConstants::USER_ROUTING_COST_C);
         p->incWriteByte(position, m_routingCostC);
         p->incWriteByte(position, UserConstants::USER_ROUTING_COST_D);
         p->incWriteByte(position, m_routingCostD);
         p->incWriteByte(position, UserConstants::USER_ROUTING_VEHICLE);
         p->incWriteLong(position, m_routingVehicle);
         p->incWriteByte(position, UserConstants::USER_ROUTING_TYPE);
         p->incWriteByte(position, m_routingType);
         // Routeimagetype
         p->incWriteByte(position, UserConstants::USER_ROUTEIMAGETYPE);
         p->incWriteLong(position, m_routeImageType);
         // Gender
         p->incWriteByte(position, UserConstants::USER_GENDER);
         p->incWriteLong(position, m_gender);
         // Validdate
         p->incWriteByte(position, UserConstants::USER_VALIDDATE);
         p->incWriteLong(position, m_validDate);
         // Edit rights
         p->incWriteByte(position, UserConstants::USER_EDIT_MAP_RIGHTS);
         p->incWriteLong(position, m_editMapRights);
         p->incWriteByte(position, UserConstants::USER_EDIT_DELAY_RIGHTS);
         p->incWriteLong(position, m_editDelayRights);
         p->incWriteByte(position, UserConstants::USER_EDIT_USER_RIGHTS);
         p->incWriteLong(position, m_editUserRights);
         // LoginRights
         p->incWriteByte(position, UserConstants::USER_WAP_SERVICE);
         p->incWriteByte(position, m_wapService);
         p->incWriteByte(position, UserConstants::USER_HTML_SERVICE);
         p->incWriteByte(position, m_htmlService);
         p->incWriteByte(position, UserConstants::USER_OPERATOR_SERVICE);
         p->incWriteByte(position, m_operatorService);
         p->incWriteByte(position, UserConstants::USER_SMS_SERVICE);
         p->incWriteByte(position, m_smsService);
         // Default search setings
         p->incWriteByte( position, UserConstants::USER_DEFAULT_COUNTRY );
         p->incWriteString( position, m_default_country );
         p->incWriteByte( position, UserConstants::USER_DEFAULT_MUNICIPAL );
         p->incWriteString( position, m_default_municipal );
         p->incWriteByte( position, UserConstants::USER_DEFAULT_CITY );
         p->incWriteString( position, m_default_city );
         // Municipals
         p->incWriteByte(position, UserConstants::USER_NBRMUNICIPAL);
         p->incWriteLong(position, m_nbrMunicipal);
         p->incWriteByte(position, UserConstants::USER_MUNICIPAL);
         for ( int32 i = 0 ; i < m_nbrMunicipal ; i++ ) {
            p->incWriteLong(position, m_municipal[i]);
         }

         p->incWriteByte( position, UserConstants::USER_NAV_SERVICE );
         p->incWriteByte( position, m_navService );
         p->incWriteByte( position, UserConstants::USER_OPERATOR_COMMENT );
         p->incWriteString( position,  m_operatorComment);
         p->incWriteByte( position, UserConstants::USER_EMAILADDRESS );
         p->incWriteString( position, m_emailAddress );
         p->incWriteByte( position, UserConstants::USER_ADDRESS1 );
         p->incWriteString( position, m_address1 );
         p->incWriteByte( position, UserConstants::USER_ADDRESS2 );
         p->incWriteString( position, m_address2 );
         p->incWriteByte( position, UserConstants::USER_ADDRESS3 );
         p->incWriteString( position, m_address3 );
         p->incWriteByte( position, UserConstants::USER_ADDRESS4 );
         p->incWriteString( position, m_address4 );
         p->incWriteByte( position, UserConstants::USER_ADDRESS5 );
         p->incWriteString( position, m_address5 );
         p->incWriteByte( position, 
                          UserConstants::USER_ROUTETURNIMAGETYPE );
         p->incWriteByte( position, m_routeTurnImageType );
         p->incWriteByte( position, 
                          UserConstants::USER_EXTERNALXMLSERVICE );
         p->incWriteByte( position, m_externalXmlService );
         p->incWriteByte( position, 
                          UserConstants::USER_TRANSACTIONBASED );
         p->incWriteByte( position, m_transactionBased );
         p->incWriteByte( position, UserConstants::USER_DEVICECHANGES );
         p->incWriteLong( position, m_deviceChanges );
         p->incWriteByte( position, UserConstants::USER_SUPPORTCOMMENT );
         p->incWriteString( position, m_supportComment );
         p->incWriteByte( position, UserConstants::USER_POSTALCITY );
         p->incWriteString( position, m_postalCity );
         p->incWriteByte( position, UserConstants::USER_ZIPCODE );
         p->incWriteString( position, m_zipCode );
         p->incWriteByte( position, UserConstants::USER_COMPANYNAME );
         p->incWriteString( position, m_companyName );
         p->incWriteByte( position, UserConstants::USER_COMPANYREFERENCE );
         p->incWriteString( position, m_companyReference );
         p->incWriteByte( position, UserConstants::USER_COMPANYVATNBR );
         p->incWriteString( position, m_companyVATNbr );
         p->incWriteByte( position, UserConstants::USER_EMAILBOUNCES );
         p->incWriteLong( position, m_emailBounces );
         p->incWriteByte( position, UserConstants::USER_ADDRESSBOUNCES );
         p->incWriteLong( position, m_addressBounces );
         p->incWriteByte( position, 
                          UserConstants::USER_CUSTOMERCONTACTINFO );
         p->incWriteString( position, m_customerContactInfo );
         
      }
      else {
         p->incWriteByte(position, UserConstants::ACTION_CHANGE);
         p->incWriteByte(position, nbrChanges);
         if (m_changed[UserConstants::USER_LOGONID]) {
            p->incWriteByte(position, UserConstants::USER_LOGONID);
            p->incWriteString(position, m_logonID);
         }
         if (m_changed[UserConstants::USER_FIRSTNAME]) {
            p->incWriteByte(position, UserConstants::USER_FIRSTNAME);
            p->incWriteString(position, m_firstname);
         }
         if (m_changed[UserConstants::USER_INITIALS]) {
            p->incWriteByte(position, UserConstants::USER_INITIALS);
            p->incWriteString(position, m_initials);
         }
         if (m_changed[UserConstants::USER_LASTNAME]) {
            p->incWriteByte(position, UserConstants::USER_LASTNAME);
            p->incWriteString(position, m_lastname);
         }
         if (m_changed[UserConstants::USER_SESSION]) {
            p->incWriteByte(position, UserConstants::USER_SESSION);
            p->incWriteString(position, m_session);
         }
         if (m_changed[UserConstants::USER_MEASUREMENT_SYSTEM]) {
            p->incWriteByte(position, 
                            UserConstants::USER_MEASUREMENT_SYSTEM);
            p->incWriteLong(position, m_measurementSystem);
         }
         if (m_changed[UserConstants::USER_LANGUAGE]) {
            p->incWriteByte(position, UserConstants::USER_LANGUAGE);
            p->incWriteLong(position, m_language);
         }
         // Dest
         if (m_changed[UserConstants::USER_LASTDEST_MAPID]) {
            p->incWriteByte(position, UserConstants::USER_LASTDEST_MAPID);
            p->incWriteLong(position, m_lastdest_mapID);
         }
         if (m_changed[UserConstants::USER_LASTDEST_ITEMID]) {
            p->incWriteByte(position, UserConstants::USER_LASTDEST_ITEMID);
            p->incWriteLong(position, m_lastdest_itemID);
         }
         if (m_changed[UserConstants::USER_LASTDEST_OFFSET]) {
            p->incWriteByte(position, UserConstants::USER_LASTDEST_OFFSET);
            p->incWriteShort(position, m_lastdest_offset);
         }
         if (m_changed[UserConstants::USER_LASTDEST_TIME]) {
            p->incWriteByte(position, UserConstants::USER_LASTDEST_TIME);
            p->incWriteLong(position, m_lastdest_time);
         }
         if (m_changed[UserConstants::USER_LASTDEST_STRING]) {
            p->incWriteByte(position, UserConstants::USER_LASTDEST_STRING);
            p->incWriteString(position, m_lastdest_string);
         }
         // Birthdate
         if (m_changed[UserConstants::USER_BIRTHDATE]) {
            p->incWriteByte(position, UserConstants::USER_BIRTHDATE);
            p->incWriteString(position, m_birthDate);
         }
         // Orig
         if (m_changed[UserConstants::USER_LASTORIG_MAPID]) {
            p->incWriteByte(position, UserConstants::USER_LASTORIG_MAPID);
            p->incWriteLong(position, m_lastorig_mapID);
         }
         if (m_changed[UserConstants::USER_LASTORIG_ITEMID]) {
            p->incWriteByte(position, UserConstants::USER_LASTORIG_ITEMID);
            p->incWriteLong(position, m_lastorig_itemID);
         }
         if (m_changed[UserConstants::USER_LASTORIG_OFFSET]) {
            p->incWriteByte(position, UserConstants::USER_LASTORIG_OFFSET);
            p->incWriteShort(position, m_lastorig_offset);
         }
         if (m_changed[UserConstants::USER_LASTORIG_TIME]) {
            p->incWriteByte(position, UserConstants::USER_LASTORIG_TIME);
            p->incWriteLong(position, m_lastorig_time);
         }
         if (m_changed[UserConstants::USER_LASTORIG_STRING]) {
            p->incWriteByte(position, UserConstants::USER_LASTORIG_STRING);
            p->incWriteString(position, m_lastorig_string); 
         }
         // Search
         if (m_changed[UserConstants::USER_SEARCH_OBJECTS]) {
            p->incWriteByte(position, UserConstants::USER_SEARCH_OBJECTS);
            p->incWriteLong(position, m_searchObject);
         }
         if (m_changed[UserConstants::USER_SEARCH_TYPE]) {
            p->incWriteByte(position, UserConstants::USER_SEARCH_TYPE);
            p->incWriteByte(position, m_searchType);
         }
         if (m_changed[UserConstants::USER_SEARCH_SUBSTRING]) {
            p->incWriteByte(position, 
                            UserConstants::USER_SEARCH_SUBSTRING);
            p->incWriteByte(position, m_searchSubstring);
         }
         if (m_changed[UserConstants::USER_SEARCH_SORTING]) {
            p->incWriteByte(position, UserConstants::USER_SEARCH_SORTING);
            p->incWriteByte(position, m_searchSorting);
         }
         if (m_changed[UserConstants::USER_SEARCH_DBMASK]) {
            p->incWriteByte(position, UserConstants::USER_SEARCH_DBMASK);
            p->incWriteByte(position, m_searchDbMask);
         }
         // Route
         if (m_changed[UserConstants::USER_ROUTING_COST_A]) {
            p->incWriteByte(position, UserConstants::USER_ROUTING_COST_A);
            p->incWriteByte(position, m_routingCostA);
         }
         if (m_changed[UserConstants::USER_ROUTING_COST_B]) {
            p->incWriteByte(position, UserConstants::USER_ROUTING_COST_B);
            p->incWriteByte(position, m_routingCostB);
         }
         if (m_changed[UserConstants::USER_ROUTING_COST_C]) {
            p->incWriteByte(position, UserConstants::USER_ROUTING_COST_C);
            p->incWriteByte(position, m_routingCostC);
         }
         if (m_changed[UserConstants::USER_ROUTING_COST_D]) {
            p->incWriteByte(position, UserConstants::USER_ROUTING_COST_D);
            p->incWriteByte(position, m_routingCostD);
         }
         if (m_changed[UserConstants::USER_ROUTING_VEHICLE]) {
            p->incWriteByte(position, UserConstants::USER_ROUTING_VEHICLE);
            p->incWriteLong(position, m_routingVehicle);
         }
         if (m_changed[UserConstants::USER_ROUTING_TYPE]) {
            p->incWriteByte(position, UserConstants::USER_ROUTING_TYPE);
            p->incWriteByte(position, m_routingType);
         }
         // Routeimagetype
         if (m_changed[UserConstants::USER_ROUTEIMAGETYPE]) {
            p->incWriteByte(position, UserConstants::USER_ROUTEIMAGETYPE);
            p->incWriteLong(position, m_routeImageType);
         }
         // Gender
         if (m_changed[UserConstants::USER_GENDER]) {
            p->incWriteByte(position, UserConstants::USER_GENDER);
            p->incWriteLong(position, m_gender);
         }
         // Validdate
         if (m_changed[UserConstants::USER_VALIDDATE] ) {
            p->incWriteByte(position, UserConstants::USER_VALIDDATE);
            p->incWriteLong(position, m_validDate);
         }
         // Edit rights
         if (m_changed[UserConstants::USER_EDIT_MAP_RIGHTS]) {
            p->incWriteByte(position, UserConstants::USER_EDIT_MAP_RIGHTS);
            p->incWriteLong(position, m_editMapRights);
         }
         if (m_changed[UserConstants::USER_EDIT_DELAY_RIGHTS]) {
            p->incWriteByte(position, 
                            UserConstants::USER_EDIT_DELAY_RIGHTS);
            p->incWriteLong(position, m_editDelayRights);
         }
         if (m_changed[UserConstants::USER_EDIT_USER_RIGHTS]) {
            p->incWriteByte(position, 
                            UserConstants::USER_EDIT_USER_RIGHTS);
            p->incWriteLong(position, m_editUserRights);
         }
         // Login rights
         if (m_changed[UserConstants::USER_WAP_SERVICE]) {
            p->incWriteByte(position, UserConstants::USER_WAP_SERVICE);
            p->incWriteByte(position, m_wapService);
         }
         if (m_changed[UserConstants::USER_HTML_SERVICE]) {
            p->incWriteByte(position, UserConstants::USER_HTML_SERVICE);
            p->incWriteByte(position, m_htmlService);
          }
         if (m_changed[UserConstants::USER_OPERATOR_SERVICE]) {
            p->incWriteByte(position, 
                            UserConstants::USER_OPERATOR_SERVICE);
            p->incWriteByte(position, m_operatorService);
         }
         if (m_changed[UserConstants::USER_SMS_SERVICE]) {
            p->incWriteByte(position, UserConstants::USER_SMS_SERVICE);
            p->incWriteByte(position, m_smsService);
         }
         // Default search settings
         if ( m_changed[UserConstants::USER_DEFAULT_COUNTRY] ) {
            p->incWriteByte( position, UserConstants::USER_DEFAULT_COUNTRY );
            p->incWriteString( position, m_default_country );
         }
         if ( m_changed[UserConstants::USER_DEFAULT_MUNICIPAL] ) {
            p->incWriteByte( position,
                             UserConstants::USER_DEFAULT_MUNICIPAL );
            p->incWriteString( position, m_default_municipal );
         }
         if ( m_changed[UserConstants::USER_DEFAULT_CITY] ) {
            p->incWriteByte( position,
                             UserConstants::USER_DEFAULT_CITY );
            p->incWriteString( position, m_default_city );
         }
         // Municipals
         if (m_changed[UserConstants::USER_NBRMUNICIPAL]) {
            p->incWriteByte(position, UserConstants::USER_NBRMUNICIPAL);
            p->incWriteLong(position, m_nbrMunicipal);
            p->incWriteByte(position, UserConstants::USER_MUNICIPAL);
            for ( int32 i = 0 ; i < m_nbrMunicipal ; i++ ) {
               p->incWriteLong(position, m_municipal[i]);
            }
         }
         if ( m_changed[ UserConstants::USER_NAV_SERVICE ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_NAV_SERVICE );
            p->incWriteByte( position, m_navService );
         }
         if ( m_changed[ UserConstants::USER_OPERATOR_COMMENT ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_OPERATOR_COMMENT );
            p->incWriteString( position, m_operatorComment );
         }
         if ( m_changed[ UserConstants::USER_EMAILADDRESS ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_EMAILADDRESS );
            p->incWriteString( position, m_emailAddress );
         }
         if ( m_changed[ UserConstants::USER_ADDRESS1 ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_ADDRESS1 );
            p->incWriteString( position, m_address1 );
         }
         if ( m_changed[ UserConstants::USER_ADDRESS2 ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_ADDRESS2 );
            p->incWriteString( position, m_address2 );
         }
         if ( m_changed[ UserConstants::USER_ADDRESS3 ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_ADDRESS3 );
            p->incWriteString( position, m_address3 );
         }
         if ( m_changed[ UserConstants::USER_ADDRESS4 ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_ADDRESS4 );
            p->incWriteString( position, m_address4 );
         }
         if ( m_changed[ UserConstants::USER_ADDRESS5 ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_ADDRESS5 );
            p->incWriteString( position, m_address5 );
         }
         if ( m_changed[ UserConstants::USER_ROUTETURNIMAGETYPE ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_ROUTETURNIMAGETYPE );
            p->incWriteByte( position, m_routeTurnImageType );
         }
         if ( m_changed[ UserConstants::USER_EXTERNALXMLSERVICE ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_EXTERNALXMLSERVICE );
            p->incWriteByte( position, m_externalXmlService );
         }
         if ( m_changed[ UserConstants::USER_TRANSACTIONBASED ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_TRANSACTIONBASED );
            p->incWriteByte( position, m_transactionBased );
         }
         if ( m_changed[ UserConstants::USER_DEVICECHANGES ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_DEVICECHANGES );
            p->incWriteLong( position, m_deviceChanges );
         }
         if ( m_changed[ UserConstants::USER_SUPPORTCOMMENT ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_SUPPORTCOMMENT );
            p->incWriteString( position, m_supportComment );
         }
         if ( m_changed[ UserConstants::USER_POSTALCITY ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_POSTALCITY );
            p->incWriteString( position, m_postalCity );
         }
         if ( m_changed[ UserConstants::USER_ZIPCODE ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_ZIPCODE );
            p->incWriteString( position, m_zipCode );
         }
         if ( m_changed[ UserConstants::USER_COMPANYNAME ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_COMPANYNAME );
            p->incWriteString( position, m_companyName );
         }
         if ( m_changed[ UserConstants::USER_COMPANYREFERENCE ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_COMPANYREFERENCE );
            p->incWriteString( position, m_companyReference );
         }
         if ( m_changed[ UserConstants::USER_COMPANYVATNBR ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_COMPANYVATNBR );
            p->incWriteString( position, m_companyVATNbr );
         }
         if ( m_changed[ UserConstants::USER_EMAILBOUNCES ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_EMAILBOUNCES );
            p->incWriteLong( position, m_emailBounces );
         }
         if ( m_changed[ UserConstants::USER_ADDRESSBOUNCES ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_ADDRESSBOUNCES );
            p->incWriteLong( position, m_addressBounces );
         }
         if ( m_changed[ UserConstants::USER_CUSTOMERCONTACTINFO ] ) {
            p->incWriteByte( position,
                             UserConstants::USER_CUSTOMERCONTACTINFO );
            p->incWriteString( position, m_customerContactInfo );
         }
      }
      p->incWriteLong(position, UserConstants::TYPE_USER);
      p->incWriteLong(position, UserConstants::TYPE_USER);
      for (uint8 i = 0; i < UserConstants::USER_NBRFIELDS; i++)
         m_changed[i] = false;
      p->setLength( position );
   } else {
      mc2dbg4 << "UserUser::addChanges not changes to add!" << endl;
   }
}


bool
UserUser::readChanges( const Packet* p, int& pos, 
                       UserConstants::UserAction& action ) {
   mc2dbg4 << "UserUser::readChanges pos " << pos << endl;
   if ( p->getLength() - pos - 5 > 0 ) {
      uint32 type;
      uint32 UIN;

      type = p->incReadLong( pos );
      UIN = p->incReadLong( pos );
      if ( type == UserConstants::TYPE_USER && UIN == m_UIN ) {
         action = UserConstants::UserAction(p->incReadByte( pos ) );
         switch ( action ) {
            case UserConstants::ACTION_NEW:
               m_UIN = 0; // Indicate new
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_DELETE:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_USER ) {
                  remove();
                  return true;
               } else {
                  DEBUG1(mc2log << warn
                         << "UserUser::readChanges not Type user" << 
                         " after action delete" << endl;);
                  return false;
               }
               break;
            case UserConstants::ACTION_CHANGE:
               return ( readDataChanges( p, pos ) );
               break;
            case UserConstants::ACTION_NOP:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_USER ) {
                  return true;
               } else {
                  DEBUG1(mc2log << warn
                         << "UserUser::readChanges not Type user"
                         " after action NOP" << endl;);
                  return false;
               }
               break;
            default:
               DEBUG1(mc2log << warn
                      << "UserUser::readChanges unknown action: "
                      << (int)action << endl;);
         };
         return false;
      } else {
         DEBUG1(mc2log << error << "UserUser::readChanges not correct type or "
                << " not correct UIN, giving up." << UIN << endl;);
      return false;  
      }
   } else {
      DEBUG1(mc2log << error << "UserUser::readChanges no space for changes "
             << "giving up." << endl;);
      return false;
   }

}


bool 
UserUser::readDataChanges( const Packet* p, int& pos ) {
   if ( p->getLength() - pos - 1 > 0 ) {
      byte nbrFields;

      nbrFields = p->incReadByte( pos );

      // tmpData
      char* logonID = m_logonID;
      uint32 logonIDSize;
      char* firstname = m_firstname;
      uint32 firstnameSize;
      char* initials = m_initials;
      uint32 initialsSize;
      char* lastname = m_lastname;
      uint32 lastnameSize;
      char* session = m_session;
      uint32 sessionSize;
      UserConstants::MeasurementType measurementSystem =m_measurementSystem;
      StringTable::languageCode language = m_language;
      uint32 lastdest_mapID = m_lastdest_mapID;
      uint32 lastdest_itemID = m_lastdest_itemID;
      uint16 lastdest_offset = m_lastdest_offset;
      uint32 lastdest_time = m_lastdest_time;
      char* lastdest_string = m_lastdest_string;
      uint32 lastdest_stringSize;
      uint32 lastorig_mapID = m_lastorig_mapID;
      uint32 lastorig_itemID = m_lastorig_itemID;
      uint16 lastorig_offset = m_lastorig_offset;
      uint32 lastorig_time = m_lastorig_time;
      char* lastorig_string = m_lastorig_string;
      uint32 lastorig_stringSize;
      char* birthDate = m_birthDate;
      uint32 birthDateSize;
      uint8 searchType = m_searchType;
      uint8 searchSubstring = m_searchSubstring;
      uint8 searchSorting = m_searchSorting;
      uint32 searchObject = m_searchObject;
      uint8 searchDbMask = m_searchDbMask;
      
      uint8 routingCostA = m_routingCostA;
      uint8 routingCostB = m_routingCostB;
      uint8 routingCostC = m_routingCostC;
      uint8 routingCostD = m_routingCostD;
      ItemTypes::vehicle_t routingVehicle = m_routingVehicle; 
      uint8 routingType = m_routingType;

      UserConstants::RouteImageType routeImageType = m_routeImageType;
      uint32 validDate = m_validDate;
      UserConstants::GenderType gender = m_gender;
      
      uint32 editMapRights = m_editMapRights;
      uint32 editDelayRights = m_editDelayRights;
      uint32 editUserRights = m_editUserRights;
      
      bool wapService = m_wapService;
      bool htmlService = m_htmlService;
      bool operatorService = m_operatorService;
      bool smsService = m_smsService;

      char* default_country= m_default_country;
      uint32 default_countrySize;
      char* default_municipal = m_default_municipal;
      uint32 default_municipalSize;
      char* default_city = m_default_city;
      uint32 default_citySize;

      bool navService = m_navService;
      char* operatorComment = m_operatorComment;
      uint32 operatorCommentSize = 0;
      char* emailAddress = m_emailAddress;
      uint32 emailAddressSize = 0;
      char* address1 = m_address1;
      uint32 address1Size = 0;
      char* address2 = m_address2;
      uint32 address2Size = 0;
      char* address3 = m_address3;
      uint32 address3Size = 0;
      char* address4 = m_address4;
      uint32 address4Size = 0;
      char* address5 = m_address5;
      uint32 address5Size = 0;
      UserConstants::RouteTurnImageType routeTurnImageType = 
         m_routeTurnImageType;
      bool externalXmlService = m_externalXmlService;
      UserConstants::transactionBased_t transactionBased =
         m_transactionBased;
      int32 deviceChanges = m_deviceChanges;
      char* supportComment = m_supportComment;
      char* postalCity = m_postalCity;
      char* zipCode = m_zipCode;
      char* companyName = m_companyName;
      char* companyReference = m_companyReference;
      char* companyVATNbr = m_companyVATNbr;
      int32 emailBounces = m_emailBounces;
      int32 addressBounces = m_addressBounces;
      char* customerContactInfo = m_customerContactInfo;
      
      int32 nbrMunicipal = m_nbrMunicipal;
      uint32* municipal = m_municipal;

      byte field;
      bool ok = true;
      bool municipalchange = false; // If municipals has changed
      for ( uint8 i = 0 ; 
            (i < nbrFields) && ok && (pos < (int32)p->getLength()) ;
            i ++ ) 
      {
         field = p->incReadByte( pos );
         switch ( field ) {
            case UserConstants::USER_LOGONID :
               m_changed[ UserConstants::USER_LOGONID ] = true;
               logonIDSize = p->incReadString( pos, logonID );
               break;
            case UserConstants::USER_FIRSTNAME :
               m_changed[ UserConstants::USER_FIRSTNAME ] = true;
               firstnameSize = p->incReadString( pos, firstname );
               break;
            case UserConstants::USER_INITIALS :
               m_changed[ UserConstants::USER_INITIALS ] = true;
               initialsSize = p->incReadString( pos, initials );
               break;
            case UserConstants::USER_LASTNAME :
               m_changed[ UserConstants::USER_LASTNAME ] = true;
               lastnameSize = p->incReadString( pos, lastname );
               break;
            case UserConstants::USER_SESSION :
               m_changed[ UserConstants::USER_SESSION ] = true;
               sessionSize = p->incReadString( pos, session );
               break;
            case UserConstants::USER_MEASUREMENT_SYSTEM :
               m_changed[ UserConstants::USER_MEASUREMENT_SYSTEM ] = true;
               measurementSystem =  UserConstants::MeasurementType( 
                  p->incReadLong( pos ) );
               break;
            case UserConstants::USER_LANGUAGE :
               m_changed[ UserConstants::USER_LANGUAGE ] = true;
               language = StringTable::languageCode( 
                  p->incReadLong( pos ) );
               break;
            case UserConstants::USER_LASTDEST_MAPID :
               m_changed[ UserConstants::USER_LASTDEST_MAPID ] = true;
               lastdest_mapID = p->incReadLong( pos );
               break;
            case UserConstants::USER_LASTDEST_ITEMID :
               m_changed[ UserConstants::USER_LASTDEST_ITEMID ] = true;
               lastdest_itemID = p->incReadLong( pos );
               break;
            case UserConstants::USER_LASTDEST_OFFSET :
               m_changed[ UserConstants::USER_LASTDEST_OFFSET ] = true;
               lastdest_offset = p->incReadShort( pos );
               break;
            case UserConstants::USER_LASTDEST_TIME :
               m_changed[ UserConstants::USER_LASTDEST_TIME ] = true;
               lastdest_time = p->incReadLong( pos );
               break;
            case UserConstants::USER_LASTDEST_STRING :
               m_changed[ UserConstants::USER_LASTDEST_STRING ] = true;
               lastdest_stringSize = p->incReadString( pos, 
                                                       lastdest_string );
               break;
            case UserConstants::USER_LASTORIG_MAPID :
               m_changed[ UserConstants::USER_LASTORIG_MAPID ] = true;
               lastorig_mapID = p->incReadLong( pos );
               break;
            case UserConstants::USER_LASTORIG_ITEMID :
               m_changed[ UserConstants::USER_LASTORIG_ITEMID ] = true;
               lastorig_itemID = p->incReadLong( pos );
               break;
            case UserConstants::USER_LASTORIG_OFFSET :
               m_changed[ UserConstants::USER_LASTORIG_OFFSET ] = true;
               lastorig_offset = p->incReadShort( pos );
               break;
            case UserConstants::USER_LASTORIG_TIME :
               m_changed[ UserConstants::USER_LASTORIG_TIME ] = true;
               lastorig_time = p->incReadLong( pos );
               break;
            case UserConstants::USER_LASTORIG_STRING :
               m_changed[ UserConstants::USER_LASTORIG_STRING ] = true;
               lastorig_stringSize = p->incReadString( pos, 
                                                       lastorig_string );
               break;
            case UserConstants::USER_BIRTHDATE :
               m_changed[ UserConstants::USER_BIRTHDATE ] = true;
               birthDateSize = p->incReadString( pos, birthDate );
               break;
            case UserConstants::USER_SEARCH_OBJECTS :
               m_changed[ UserConstants::USER_SEARCH_OBJECTS ] = true;
               searchObject = p->incReadLong( pos );
               break;
            case UserConstants::USER_SEARCH_TYPE :
               m_changed[ UserConstants::USER_SEARCH_TYPE ] = true;
               searchType = p->incReadByte( pos );
               break;
            case UserConstants::USER_SEARCH_SUBSTRING :
               m_changed[ UserConstants::USER_SEARCH_SUBSTRING ] = true;
               searchSubstring = p->incReadByte( pos );
               break;
            case UserConstants::USER_SEARCH_SORTING :
               m_changed[ UserConstants::USER_SEARCH_SORTING ] = true;
               searchSorting = p->incReadByte( pos );
               break;
            case UserConstants::USER_SEARCH_DBMASK :
               m_changed[ UserConstants::USER_SEARCH_DBMASK ] = true;
               searchDbMask = p->incReadByte( pos );
               break;
            case UserConstants::USER_ROUTING_COST_A :
               m_changed[ UserConstants::USER_ROUTING_COST_A ] = true;
               routingCostA = p->incReadByte( pos );
               break;
            case UserConstants::USER_ROUTING_COST_B :
               m_changed[ UserConstants::USER_ROUTING_COST_B ] = true;
               routingCostB = p->incReadByte( pos );
               break;
            case UserConstants::USER_ROUTING_COST_C :
               m_changed[ UserConstants::USER_ROUTING_COST_C ] = true;
               routingCostC = p->incReadByte( pos );
               break;
            case UserConstants::USER_ROUTING_COST_D :
               m_changed[ UserConstants::USER_ROUTING_COST_D ] = true;
               routingCostD = p->incReadByte( pos );
               break;
            case UserConstants::USER_ROUTING_VEHICLE :
               m_changed[ UserConstants::USER_ROUTING_VEHICLE ] = true;
               routingVehicle = ItemTypes::vehicle_t( 
                  p->incReadLong( pos ) );
               break;
            case UserConstants::USER_ROUTING_TYPE :
               m_changed[ UserConstants::USER_ROUTING_TYPE ] = true;
               routingType = p->incReadByte( pos );
               break;
            case UserConstants::USER_ROUTEIMAGETYPE :
               m_changed[ UserConstants::USER_ROUTEIMAGETYPE ] = true;
               routeImageType =  UserConstants::RouteImageType( 
                  p->incReadLong( pos ) );
               break;
            case UserConstants::USER_GENDER :
               m_changed[ UserConstants::USER_GENDER ] = true;
               gender =  UserConstants::GenderType( 
                  p->incReadLong( pos ) );
               break;
            case UserConstants::USER_VALIDDATE :
               m_changed[ UserConstants::USER_VALIDDATE ] = true;
               validDate = p->incReadLong( pos );
               break;
            case UserConstants::USER_EDIT_MAP_RIGHTS :
               m_changed[ UserConstants::USER_EDIT_MAP_RIGHTS ] = true;
               editMapRights = p->incReadLong( pos );
               break;
            case UserConstants::USER_EDIT_DELAY_RIGHTS :
               m_changed[ UserConstants::USER_EDIT_DELAY_RIGHTS ] = true;
               editDelayRights = p->incReadLong( pos );
               break;
            case UserConstants::USER_EDIT_USER_RIGHTS :
               m_changed[ UserConstants::USER_EDIT_USER_RIGHTS ] = true;
               editUserRights = p->incReadLong( pos );
               break;
            case UserConstants::USER_WAP_SERVICE :
               m_changed[ UserConstants::USER_WAP_SERVICE ] = true;
               wapService = p->incReadByte( pos ) != 0;
               break;
            case UserConstants::USER_HTML_SERVICE :
               m_changed[ UserConstants::USER_HTML_SERVICE ] = true;
               htmlService = p->incReadByte( pos ) != 0;
               break;
            case UserConstants::USER_OPERATOR_SERVICE :
               m_changed[ UserConstants::USER_OPERATOR_SERVICE ] = true;
               operatorService = p->incReadByte( pos ) != 0;
               break;
            case UserConstants::USER_SMS_SERVICE :
               m_changed[ UserConstants::USER_SMS_SERVICE ] = true;
               smsService = p->incReadByte( pos ) != 0;
               break;
            case UserConstants::USER_DEFAULT_COUNTRY :
               m_changed[ UserConstants::USER_DEFAULT_COUNTRY ] = true;
               default_countrySize = p->incReadString( pos,
                                                       default_country );
               break;
            case UserConstants::USER_DEFAULT_MUNICIPAL :
               m_changed[ UserConstants::USER_DEFAULT_MUNICIPAL ] = true;
               default_municipalSize = p->incReadString( pos,
                                                         default_municipal );
               break;
            case UserConstants::USER_DEFAULT_CITY :
               m_changed[ UserConstants::USER_DEFAULT_CITY ] = true;
               default_citySize = p->incReadString( pos, default_city );
               break; 
            case UserConstants::USER_NBRMUNICIPAL : // Set municipalchange
               m_changed[ UserConstants::USER_NBRMUNICIPAL ] = true;
               nbrMunicipal = p->incReadLong( pos );
               municipalchange = true;
               break;
            case UserConstants::USER_MUNICIPAL :  {// check municipalchange
               if ( !municipalchange ) {
                  DEBUG1(mc2log << warn << "UserUser::readDataChanges " 
                         << " municipals changed but not nbrmunicipals"
                         << endl;);
                  ok = false;
                  break;
               }
               m_changed[ UserConstants::USER_MUNICIPAL ] = true;
               municipalchange = false; // OK
               // Read nbrMunicipals municipals
               municipal = new uint32[ nbrMunicipal ];
               for ( int32 i = 0 ; i < nbrMunicipal ; i++ ) {
                  municipal[ i ] = p->incReadLong( pos ); 
               }
               break;
            }
            case UserConstants::USER_NAV_SERVICE:
               m_changed[ UserConstants::USER_NAV_SERVICE ] = true;
               navService = p->incReadByte( pos ) != 0;
               break;
            case UserConstants::USER_OPERATOR_COMMENT:
               m_changed[ UserConstants::USER_OPERATOR_COMMENT ] = true;
               operatorCommentSize = p->incReadString( pos, 
                                                       operatorComment );
               break;
            case UserConstants::USER_EMAILADDRESS:
               m_changed[ UserConstants::USER_EMAILADDRESS ] = true;
               emailAddressSize = p->incReadString( pos, 
                                                    emailAddress );
               break;
            case UserConstants::USER_ADDRESS1:
               m_changed[ UserConstants::USER_ADDRESS1 ] = true;
               address1Size = p->incReadString( pos, 
                                        address1 );
               break;
            case UserConstants::USER_ADDRESS2:
               m_changed[ UserConstants::USER_ADDRESS2 ] = true;
               address2Size = p->incReadString( pos, address2 );
               break;
            case UserConstants::USER_ADDRESS3:
               m_changed[ UserConstants::USER_ADDRESS3 ] = true;
               address3Size = p->incReadString( pos, address3 );
               break;
            case UserConstants::USER_ADDRESS4:
               m_changed[ UserConstants::USER_ADDRESS4 ] = true;
               address4Size = p->incReadString( pos, address4 );
               break;
            case UserConstants::USER_ADDRESS5:
               m_changed[ UserConstants::USER_ADDRESS5 ] = true;
               address5Size = p->incReadString( pos, address5 );
               break;
            case UserConstants::USER_ROUTETURNIMAGETYPE:
               m_changed[ UserConstants::USER_ROUTETURNIMAGETYPE ] = true;
               routeTurnImageType = UserConstants::RouteTurnImageType(
                  p->incReadByte( pos ) );
               break;
            case UserConstants::USER_EXTERNALXMLSERVICE:
               m_changed[ UserConstants::USER_EXTERNALXMLSERVICE ] = true;
               externalXmlService = p->incReadByte( pos ) != 0;
               break;
            case UserConstants::USER_TRANSACTIONBASED:
               m_changed[ UserConstants::USER_TRANSACTIONBASED ] = true;
               transactionBased = UserConstants::transactionBased_t( 
                  p->incReadByte( pos ) );
               break;
            case UserConstants::USER_DEVICECHANGES:
               m_changed[ UserConstants::USER_DEVICECHANGES ] = true;
               deviceChanges = p->incReadLong( pos );
               break;
            case UserConstants::USER_SUPPORTCOMMENT:
               m_changed[ UserConstants::USER_SUPPORTCOMMENT ] = true;
               p->incReadString( pos, supportComment );
               break;
            case UserConstants::USER_POSTALCITY:
               m_changed[ UserConstants::USER_POSTALCITY ] = true;
               p->incReadString( pos, postalCity );
               break;
            case UserConstants::USER_ZIPCODE:
               m_changed[ UserConstants::USER_ZIPCODE ] = true;
               p->incReadString( pos, zipCode );
               break;
            case UserConstants::USER_COMPANYNAME:
               m_changed[ UserConstants::USER_COMPANYNAME ] = true;
               p->incReadString( pos, companyName );
               break;
            case UserConstants::USER_COMPANYREFERENCE:
               m_changed[ UserConstants::USER_COMPANYREFERENCE ] = true;
               p->incReadString( pos, companyReference );
               break;
            case UserConstants::USER_COMPANYVATNBR:
               m_changed[ UserConstants::USER_COMPANYVATNBR ] = true;
               p->incReadString( pos, companyVATNbr );
               break;
            case UserConstants::USER_EMAILBOUNCES:
               m_changed[ UserConstants::USER_EMAILBOUNCES ] = true;
               emailBounces = p->incReadLong( pos );
               break;
            case UserConstants::USER_ADDRESSBOUNCES:
               m_changed[ UserConstants::USER_ADDRESSBOUNCES ] = true;
               addressBounces = p->incReadLong( pos );
               break;
            case UserConstants::USER_CUSTOMERCONTACTINFO:
               m_changed[ UserConstants::USER_CUSTOMERCONTACTINFO ] = true;
               p->incReadString( pos, customerContactInfo );
               break;
            default:
               mc2log << warn  << "UserUser::readDataChanges unknown " 
                      << "fieldType: " << (int)field << endl;
               ok = false;
               break;
         };
      }      
      if ( municipalchange ) ok = false;

      uint32 type = p->incReadLong( pos );
      if ( type != UserConstants::TYPE_USER ) {
         DEBUG1(mc2log << warn
                << "UserUser::readDataChanges Not User type after data"
                << endl;);
         ok = false;
      }
      if ( ok ) {
         // Transfer changes to membervariables
         if (m_changed[UserConstants::USER_LOGONID]) {
            setLogonID( logonID );
         }
         if (m_changed[UserConstants::USER_FIRSTNAME]) {
            setFirstname( firstname );
         }
         if (m_changed[UserConstants::USER_INITIALS]) {
            setInitials( initials );
         }
         if (m_changed[UserConstants::USER_LASTNAME]) {
            setLastname( lastname );
         }
         if (m_changed[UserConstants::USER_SESSION]) {
            setSession ( session );
         }
         if (m_changed[UserConstants::USER_MEASUREMENT_SYSTEM]) {
            setMeasurementSystem( measurementSystem );
         }
         if (m_changed[UserConstants::USER_LANGUAGE]) {
            setLanguage( language );
         }
         // Dest
         if (m_changed[UserConstants::USER_LASTDEST_MAPID]) {
            setLastdest_mapID( lastdest_mapID ); 
         }
         if (m_changed[UserConstants::USER_LASTDEST_ITEMID]) {
            setLastdest_itemID( lastdest_itemID );
         }
         if (m_changed[UserConstants::USER_LASTDEST_OFFSET]) {
            setLastdest_offset( lastdest_offset );
         }
         if (m_changed[UserConstants::USER_LASTDEST_TIME]) {
            setLastdest_time( lastdest_time );
         }
         if (m_changed[UserConstants::USER_LASTDEST_STRING]) {
            setBrand( lastdest_string );
         }
         // Birthdate
         if (m_changed[UserConstants::USER_BIRTHDATE]) {
            setBirthDate( birthDate );
         }
         // Orig
         if (m_changed[UserConstants::USER_LASTORIG_MAPID]) {
            setLastorig_mapID( lastorig_mapID ); 
         }
         if (m_changed[UserConstants::USER_LASTORIG_ITEMID]) {
            setLastorig_itemID( lastorig_itemID );
         }
         if (m_changed[UserConstants::USER_LASTORIG_OFFSET]) {
            setLastorig_offset( lastorig_offset );
         }
         if (m_changed[UserConstants::USER_LASTORIG_TIME]) {
            setLastorig_time( lastorig_time );
         }
         if (m_changed[UserConstants::USER_LASTORIG_STRING]) {
            setBrandOrigin( lastorig_string );
         }
         // Search
         if (m_changed[UserConstants::USER_SEARCH_OBJECTS]) {
            m_searchObject = searchObject;
         }
         if (m_changed[UserConstants::USER_SEARCH_TYPE]) {
            setSearch_type( searchType );
         }
         if (m_changed[UserConstants::USER_SEARCH_SUBSTRING]) {
            setSearch_substring( searchSubstring ); 
         }
         if (m_changed[UserConstants::USER_SEARCH_SORTING]) {
            setSearch_sorting( searchSorting );
         }
         if (m_changed[UserConstants::USER_SEARCH_DBMASK]) {
            setSearch_DbMask( searchDbMask );
         }
         // Route
         if (m_changed[UserConstants::USER_ROUTING_COST_A]) {
            setRouting_costA( routingCostA );
         }
         if (m_changed[UserConstants::USER_ROUTING_COST_B]) {
            setRouting_costB( routingCostB );
         }
         if (m_changed[UserConstants::USER_ROUTING_COST_C]) {
            setRouting_costC( routingCostC );
         }
         if (m_changed[UserConstants::USER_ROUTING_COST_D]) {
            setRouting_costD( routingCostD );
         }
         if (m_changed[UserConstants::USER_ROUTING_VEHICLE]) {
            setRouting_vehicle( routingVehicle );
         }
         if (m_changed[UserConstants::USER_ROUTING_TYPE]) {
            setRouting_type( routingType );
         }
         // Route image type
         if (m_changed[UserConstants::USER_ROUTEIMAGETYPE]) {
            setRouteImageType( routeImageType );
         }
         // Gender
         if (m_changed[UserConstants::USER_GENDER]) {
            setGender( gender );
         }
         // valid date
         if (m_changed[UserConstants::USER_VALIDDATE]) {
            setValidDate( validDate );
         }
         // Edit rights
         if (m_changed[UserConstants::USER_EDIT_MAP_RIGHTS]) {
            setEditMapRights( editMapRights );
         }
         if (m_changed[UserConstants::USER_EDIT_DELAY_RIGHTS]) {
            setEditDelayRights( editDelayRights );
         }
         if (m_changed[UserConstants::USER_EDIT_USER_RIGHTS]) {
            setEditUserRights( editUserRights );
         }
         // Login rights
         if (m_changed[UserConstants::USER_WAP_SERVICE]) {
            setWAPService( wapService );
         }
         if (m_changed[UserConstants::USER_HTML_SERVICE]) {
            setHTMLService( htmlService );
         }
         if (m_changed[UserConstants::USER_OPERATOR_SERVICE]) {
            setOperatorService( operatorService );
         }
         if (m_changed[UserConstants::USER_SMS_SERVICE]) {
            setSMSService( smsService );
         }
         // Default search settings
         if ( m_changed[UserConstants::USER_DEFAULT_COUNTRY] ) {
            setDefaultCountry( default_country );
         }
         if( m_changed[UserConstants::USER_DEFAULT_MUNICIPAL] ) {
            setDefaultMunicipal( default_municipal );
         }
         if( m_changed[UserConstants::USER_DEFAULT_CITY] ) {
            setDefaultCity( default_city );
         } 
         // Municipals
         if (m_changed[UserConstants::USER_NBRMUNICIPAL]) {
            setMunicipal( municipal, nbrMunicipal );
         }
         if ( m_changed[ UserConstants::USER_NAV_SERVICE ] ) {
            setNavService( navService );
         }
         if ( m_changed[ UserConstants::USER_OPERATOR_COMMENT ] ) {
            setOperatorComment( operatorComment );
         }
         if ( m_changed[ UserConstants::USER_EMAILADDRESS ] ) {
            setEmailAddress( emailAddress );
         }
         if ( m_changed[ UserConstants::USER_ADDRESS1 ] ) {
            setAddress1( address1 );
         }
         if ( m_changed[ UserConstants::USER_ADDRESS2 ] ) {
            setAddress2( address2 );
         }
         if ( m_changed[ UserConstants::USER_ADDRESS3 ] ) {
            setAddress3( address3 );
         }
         if ( m_changed[ UserConstants::USER_ADDRESS4 ] ) {
            setAddress4( address4 );
         }
         if ( m_changed[ UserConstants::USER_ADDRESS5 ] ) {
            setAddress5( address5 );
         }
         if ( m_changed[ UserConstants::USER_ROUTETURNIMAGETYPE ] ) {
            setRouteTurnImageType( routeTurnImageType );
         }
         if ( m_changed[ UserConstants::USER_EXTERNALXMLSERVICE ] ) {
            setExternalXmlService( externalXmlService );
         }
         if ( m_changed[ UserConstants::USER_TRANSACTIONBASED ] ) {
            setTransactionBased( transactionBased );
         }
         if ( m_changed[ UserConstants::USER_DEVICECHANGES ] ) {
            setDeviceChanges( deviceChanges );
         }
         if ( m_changed[ UserConstants::USER_SUPPORTCOMMENT ] ) {
            setSupportComment( supportComment );
         }
         if ( m_changed[ UserConstants::USER_POSTALCITY ] ) {
            setPostalCity( postalCity );
         }
         if ( m_changed[ UserConstants::USER_ZIPCODE ] ) {
            setZipCode( zipCode );
         }
         if ( m_changed[ UserConstants::USER_COMPANYNAME ] ) {
            setCompanyName( companyName );
         }
         if ( m_changed[ UserConstants::USER_COMPANYREFERENCE ] ) {
            setCompanyReference( companyReference );
         }
         if ( m_changed[ UserConstants::USER_COMPANYVATNBR ] ) {
            setCompanyVATNbr( companyVATNbr );
         }
         if ( m_changed[ UserConstants::USER_EMAILBOUNCES ] ) {
            setEmailBounces( emailBounces );
         }
         if ( m_changed[ UserConstants::USER_ADDRESSBOUNCES ] ) {
            setAddressBounces( addressBounces );
         }
         if ( m_changed[ UserConstants::USER_CUSTOMERCONTACTINFO ] ) {
            setCustomerContactInfo( customerContactInfo );
         }
         
         return ok;
      } else {
         DEBUG1(mc2log << error << "UserUser::readDataChanges failed"<< endl;);
         if ( m_changed[ UserConstants::USER_MUNICIPAL ] ) {
            delete [] municipal;
         }
         return false;
      }
   } else {
      DEBUG1(mc2log << error
             << "UserUser::readDataChanges no space for changes giving up."
             << endl;);
      return false;
   }
}


uint32 
UserUser::getUIN() const {
   return m_UIN;
}


const char*
UserUser::getLogonID() const
{
   return m_logonID;
}


void
UserUser::setLogonID(const char* logonID)
{
   delete [] m_logonID;
   m_logonID = new char[strlen(logonID) + 1];
   strcpy( m_logonID, logonID );
   m_changed[ UserConstants::USER_LOGONID ] = true;
}


const char*
UserUser::getFirstname() const
{
   return m_firstname;
}


void
UserUser::setFirstname(const char* firstname)
{
   delete [] m_firstname;
   m_firstname = new char[strlen(firstname) + 1];
   strcpy( m_firstname, firstname );
   m_changed[ UserConstants::USER_FIRSTNAME ] = true;   
}


const char*
UserUser::getInitials() const
{
   return m_initials;
}


void
UserUser::setInitials(const char* initials)
{
   delete [] m_initials;
   m_initials = new char[strlen(initials) + 1];
   strcpy( m_initials, initials );
   m_changed[ UserConstants::USER_INITIALS ] = true;      
}


const char*
UserUser::getLastname() const
{
   return m_lastname;
}


void
UserUser::setLastname(const char* lastname)
{
   delete [] m_lastname;
   m_lastname = new char[strlen(lastname) + 1];
   strcpy( m_lastname, lastname );
   m_changed[ UserConstants::USER_LASTNAME ] = true;       
}


const char*
UserUser::getSession() const
{
   return m_session;
}


void
UserUser::setSession(const char* session)
{
   delete [] m_session;
   m_session = new char[strlen(session) + 1];
   strcpy( m_session, session );
   m_changed[ UserConstants::USER_SESSION ] = true;       
}


UserConstants::MeasurementType
UserUser::getMeasurementSystem() const
{
   return m_measurementSystem;
}


void
UserUser::setMeasurementSystem(
   UserConstants::MeasurementType measurementSystem)
{
   m_measurementSystem = measurementSystem;
   m_changed[ UserConstants::USER_MEASUREMENT_SYSTEM ] = true;
}


StringTable::languageCode
UserUser::getLanguage() const
{
   return m_language;
}


void
UserUser::setLanguage(StringTable::languageCode language)
{
   m_language = language;
   m_changed[ UserConstants::USER_LANGUAGE ] = true; 
}


uint32
UserUser::getLastdest_mapID() const
{
   return m_lastdest_mapID;
}


void
UserUser::setLastdest_mapID(uint32 lastdest_mapID)
{
   m_lastdest_mapID = lastdest_mapID;
   m_changed[ UserConstants::USER_LASTDEST_MAPID ] = true;   
}


uint32
UserUser::getLastdest_itemID() const
{
   return m_lastdest_itemID;
}


void
UserUser::setLastdest_itemID(uint32 lastdest_itemID)
{
   m_lastdest_itemID = lastdest_itemID;
   m_changed[ UserConstants::USER_LASTDEST_ITEMID ] = true;   
}


uint32
UserUser::getLastdest_offset() const
{
   return m_lastdest_offset;
}


void
UserUser::setLastdest_offset(uint32 lastdest_offset)
{
   m_lastdest_offset = lastdest_offset;
   m_changed[ UserConstants::USER_LASTDEST_OFFSET ] = true;   
}


uint32
UserUser::getLastdest_time() const
{
   return m_lastdest_time;
}


void
UserUser::setLastdest_time(uint32 lastdest_time)
{
   m_lastdest_time = lastdest_time;
   m_changed[ UserConstants::USER_LASTDEST_TIME ] = true;   
}


const char*
UserUser::getBrand() const
{
   return m_lastdest_string;
}


void
UserUser::setBrand(const char* lastdest_string)
{
   delete [] m_lastdest_string;
   m_lastdest_string = new char[strlen(lastdest_string) + 1];
   strcpy( m_lastdest_string,  lastdest_string);
   m_changed[ UserConstants::USER_LASTDEST_STRING ] = true; 
}


const char*
UserUser::getBirthDate() const
{
   return m_birthDate;
}


void
UserUser::setBirthDate(const char* birthDate)
{
   delete [] m_birthDate;
   m_birthDate = new char[strlen(birthDate) + 1];
   strcpy( m_birthDate,  birthDate);
   m_changed[ UserConstants::USER_BIRTHDATE ] = true; 
}


uint32
UserUser::getLastorig_mapID() const
{
   return m_lastorig_mapID;
}


void
UserUser::setLastorig_mapID(uint32 lastorig_mapID)
{
   m_lastorig_mapID = lastorig_mapID;
   m_changed[ UserConstants::USER_LASTORIG_MAPID ] = true;   
}


uint32
UserUser::getLastorig_itemID() const
{
   return m_lastorig_itemID;
}


void
UserUser::setLastorig_itemID(uint32 lastorig_itemID)
{
   m_lastorig_itemID = lastorig_itemID;
   m_changed[ UserConstants::USER_LASTORIG_ITEMID ] = true;   
}


uint32
UserUser::getLastorig_offset() const
{
   return m_lastorig_offset;
}


void
UserUser::setLastorig_offset(uint32 lastorig_offset)
{
   m_lastorig_offset = lastorig_offset;
   m_changed[ UserConstants::USER_LASTORIG_OFFSET ] = true;   
}


uint32
UserUser::getLastorig_time() const
{
   return m_lastorig_time;
}


void
UserUser::setLastorig_time(uint32 lastorig_time)
{
   m_lastorig_time = lastorig_time;
   m_changed[ UserConstants::USER_LASTORIG_TIME ] = true;   
}


const char*
UserUser::getBrandOrigin() const
{
   return m_lastorig_string;
}


void
UserUser::setBrandOrigin(const char* lastorig_string)
{
   delete [] m_lastorig_string;
   m_lastorig_string = new char[strlen(lastorig_string) + 1];
   strcpy( m_lastorig_string, lastorig_string );
   m_changed[ UserConstants::USER_LASTORIG_STRING ] = true; 
}


uint8
UserUser::getRouting_costA() const
{
   return m_routingCostA;
}


void
UserUser::setRouting_costA(uint8 routingCostA)
{
   m_routingCostA = routingCostA;
   m_changed[ UserConstants::USER_ROUTING_COST_A ] = true;   
}


uint8
UserUser::getRouting_costB() const
{
   return m_routingCostB;
}


void
UserUser::setRouting_costB(uint8 routingCostB)
{
   m_routingCostB = routingCostB;
   m_changed[ UserConstants::USER_ROUTING_COST_B ] = true;   
}


uint8
UserUser::getRouting_costC() const
{
   return m_routingCostC;
}


void
UserUser::setRouting_costC(uint8 routingCostC)
{
   m_routingCostC = routingCostC;
   m_changed[ UserConstants::USER_ROUTING_COST_C ] = true;   
}


uint8
UserUser::getRouting_costD() const
{
   return m_routingCostD;
}


void
UserUser::setRouting_costD(uint8 routingCostD)
{
   m_routingCostD = routingCostD;
   m_changed[ UserConstants::USER_ROUTING_COST_D ] = true;   
}


uint8
UserUser::getRouting_type() const
{
   return m_routingType;
}


void
UserUser::setRouting_type(uint8 routingType)
{
   m_routingType = routingType;
   m_changed[ UserConstants::USER_ROUTING_TYPE ] = true;   
}


ItemTypes::vehicle_t
UserUser::getRouting_vehicle() const
{
   return m_routingVehicle;
}


void
UserUser::setRouting_vehicle( ItemTypes::vehicle_t routing_vechicle )
{
   m_routingVehicle = routing_vechicle;
   m_changed[ UserConstants::USER_ROUTING_VEHICLE ] = true;
}


UserConstants::RouteImageType
UserUser::getRouteImageType() const
{
   return m_routeImageType;
}


void
UserUser::setRouteImageType( UserConstants::RouteImageType routeImageType )
{
   m_routeImageType = routeImageType;
   m_changed[ UserConstants::USER_ROUTEIMAGETYPE ] = true;
}


UserConstants::GenderType
UserUser::getGender() const
{
   return m_gender;
}


void
UserUser::setGender( UserConstants::GenderType gender )
{
   m_gender = gender;
   m_changed[ UserConstants::USER_GENDER ] = true;
}


uint32
UserUser::getValidDate() const
{
   return m_validDate;
}


void
UserUser::setValidDate( uint32 validDate )
{
   m_validDate = validDate;
   m_changed[ UserConstants::USER_VALIDDATE ] = true;   
}


uint8
UserUser::getSearch_type() const
{
   return m_searchType;
}


void
UserUser::setSearch_type(uint8 search_type)
{
   m_searchType = search_type;
   m_changed[ UserConstants::USER_SEARCH_TYPE ] = true;   
}


uint8
UserUser::getSearch_substring() const
{
   return m_searchSubstring;
}


void
UserUser::setSearch_substring(uint8 search_substring)
{
   m_searchSubstring = search_substring;
   m_changed[ UserConstants::USER_SEARCH_SUBSTRING ] = true;   
}


uint8
UserUser::getSearch_sorting() const
{
   return m_searchSorting;
}


void
UserUser::setSearch_sorting(uint8 search_sorting)
{
   m_searchSorting = search_sorting;
   m_changed[ UserConstants::USER_SEARCH_SORTING ] = true;   
}


uint32 
UserUser::getSearchForTypes() const {
   return m_searchObject & (~SEARCH_ALL_REGION_TYPES);
}


void 
UserUser::setSearchForTypes( uint32 searchTypes ) {
   m_searchObject = (m_searchObject & SEARCH_ALL_REGION_TYPES) | 
      (searchTypes & (~SEARCH_ALL_REGION_TYPES));
   m_changed[ UserConstants::USER_SEARCH_OBJECTS ] = true;
}


uint32 
UserUser::getSearchForLocationTypes() const {
   return m_searchObject & SEARCH_ALL_REGION_TYPES;
}


void 
UserUser::setSearchForLocationTypes( uint32 searchTypes ) {
   m_searchObject = (m_searchObject & (~SEARCH_ALL_REGION_TYPES)) |
      (searchTypes & SEARCH_ALL_REGION_TYPES);
   m_changed[ UserConstants::USER_SEARCH_OBJECTS ] = true;
}


uint8
UserUser::getSearch_DbMask() const
{
   return m_searchDbMask;
}

void
UserUser::setSearch_DbMask(uint8 newSearchDbMask)
{
   m_searchDbMask = newSearchDbMask;
   m_changed[UserConstants::USER_SEARCH_DBMASK] = true;
}

uint32
UserUser::getEditMapRights() const {
   return m_editMapRights;
}


void
UserUser::setEditMapRights( uint32 editMapRights ) {
   m_editMapRights = editMapRights;
   m_changed[ UserConstants::USER_EDIT_MAP_RIGHTS ] = true;
}


uint32
UserUser::getEditDelayRights() const {
   return m_editDelayRights;
}


void
UserUser::setEditDelayRights( uint32 editDelayRights ) {
   m_editDelayRights = editDelayRights;
   m_changed[ UserConstants::USER_EDIT_DELAY_RIGHTS ] = true;  
}


uint32
UserUser::getEditUserRights() const {
   return m_editUserRights;
}


void
UserUser::setEditUserRights( uint32 editUserRights ) {
   m_editUserRights = editUserRights;
   m_changed[ UserConstants::USER_EDIT_USER_RIGHTS ] = true;
}


bool 
UserUser::getWAPService() const {
   return m_wapService;
}


void
UserUser::setWAPService(bool wapService) {
   m_wapService = wapService;
   m_changed[ UserConstants::USER_WAP_SERVICE ] = true;
}


bool 
UserUser::getHTMLService() const {
   return m_htmlService;
}


void
UserUser::setHTMLService(bool htmlService) {
   m_htmlService = htmlService;
   m_changed[ UserConstants::USER_HTML_SERVICE ] = true;
}


bool 
UserUser::getOperatorService() const {
   return m_operatorService;
}


void
UserUser::setOperatorService(bool operatorService) {
   m_operatorService = operatorService;
   m_changed[ UserConstants::USER_OPERATOR_SERVICE ] = true;
}


bool 
UserUser::getSMSService() const {
   return m_smsService;
}


void
UserUser::setSMSService(bool smsService) {
   m_smsService = smsService;
   m_changed[ UserConstants::USER_SMS_SERVICE ] = true;
}


const char*
UserUser::getDefaultCountry() const {
   return m_default_country;
}


void
UserUser::setDefaultCountry( const char* default_country ) {
   delete [] m_default_country;
   m_default_country = new char[strlen(default_country) + 1];
   strcpy( m_default_country, default_country );
   m_changed[ UserConstants::USER_DEFAULT_COUNTRY ] = true; 
}


const char*
UserUser::getDefaultMunicipal() const {
   return m_default_municipal;
}


void
UserUser::setDefaultMunicipal( const char* default_municipal ) {
   delete [] m_default_municipal;
   m_default_municipal = new char[strlen(default_municipal) + 1];
   strcpy( m_default_municipal, default_municipal );
   m_changed[ UserConstants::USER_DEFAULT_MUNICIPAL ] = true;
}

const char*
UserUser::getDefaultCity() const {
   return m_default_city;
}


void
UserUser::setDefaultCity( const char* default_city ) {
   delete [] m_default_city;
   m_default_city = new char[strlen(default_city) + 1];
   strcpy( m_default_city, default_city );
   m_changed[ UserConstants::USER_DEFAULT_CITY ] = true;
}


uint32
UserUser::getNumMunicipal() const
{
   return m_nbrMunicipal;
}


uint32
UserUser::getMunicipal(uint32 index) const
{
   DEBUG1(if ( (int32)index >= m_nbrMunicipal )
          mc2log << error << "UserUser::getMunicipal INDEX OUT BOUNDS ERROR " 
          << index << endl; );
   return m_municipal[ index ];
}


void
UserUser::setMunicipal( uint32* municipal, uint32 nbrMunicipal ) {
   delete [] m_municipal;
   m_municipal = municipal;
   m_nbrMunicipal = nbrMunicipal;
   m_changed[ UserConstants::USER_MUNICIPAL ] = true;
}


void 
UserUser::setNavService( bool value ) {
   m_navService = value;
   m_changed[ UserConstants::USER_NAV_SERVICE ] = true;
}


bool 
UserUser::getNavService() const {
   return m_navService;
}


void 
UserUser::setOperatorComment( const char* str ) {
   delete [] m_operatorComment;
   m_operatorComment = StringUtility::newStrDup( str );
   m_changed[ UserConstants::USER_OPERATOR_COMMENT ] = true;
}


const char* 
UserUser::getOperatorComment() const {
   return m_operatorComment;
}


void 
UserUser::setEmailAddress( const char* str ) {
   delete [] m_emailAddress;
   m_emailAddress = StringUtility::newStrDup( str );
   m_changed[ UserConstants::USER_EMAILADDRESS ] = true;
}


const char* 
UserUser::getEmailAddress() const {
   return m_emailAddress;
}


void 
UserUser::setAddress1( const char* str ) {
   delete [] m_address1;
   m_address1 = StringUtility::newStrDup( str );
   m_changed[ UserConstants::USER_ADDRESS1 ] = true;
}

 
const char* 
UserUser::getAddress1() const {
   return m_address1;
}


void 
UserUser::setAddress2( const char* str ) {
   delete [] m_address2;
   m_address2 = StringUtility::newStrDup( str );
   m_changed[ UserConstants::USER_ADDRESS2 ] = true;
}

 
const char* 
UserUser::getAddress2() const {
   return m_address2;
}


void 
UserUser::setAddress3( const char* str ) {
   delete [] m_address3;
   m_address3 = StringUtility::newStrDup( str );
   m_changed[ UserConstants::USER_ADDRESS3 ] = true;
}

 
const char* 
UserUser::getAddress3() const {
   return m_address3;
}


void 
UserUser::setAddress4( const char* str ) {
   delete [] m_address4;
   m_address4 = StringUtility::newStrDup( str );
   m_changed[ UserConstants::USER_ADDRESS4 ] = true;
}

 
const char* 
UserUser::getAddress4() const {
   return m_address4;
}


void 
UserUser::setAddress5( const char* str ) {
   delete [] m_address5;
   m_address5 = StringUtility::newStrDup( str );
   m_changed[ UserConstants::USER_ADDRESS5 ] = true;
}

 
const char* 
UserUser::getAddress5() const {
   return m_address5;
}


void 
UserUser::setRouteTurnImageType( UserConstants::RouteTurnImageType type ) {
   m_routeTurnImageType = type;
   m_changed[ UserConstants::USER_ROUTETURNIMAGETYPE ] = true;
}


UserConstants::RouteTurnImageType  
UserUser::getRouteTurnImageType() const {
   return m_routeTurnImageType;
}


void 
UserUser::setExternalXmlService( bool value ) {
   m_externalXmlService = value;
   m_changed[ UserConstants::USER_EXTERNALXMLSERVICE ] = true;
}


bool 
UserUser::getExternalXmlService() const {
   return m_externalXmlService;
}


void 
UserUser::setTransactionBased( UserConstants::transactionBased_t value ) {
   m_transactionBased = value;
   m_changed[ UserConstants::USER_TRANSACTIONBASED ] = true;
}


UserConstants::transactionBased_t
UserUser::getTransactionBased() const {
   return m_transactionBased;
}


int32
UserUser::getDeviceChanges() const {
   return m_deviceChanges;
}


void
UserUser::setDeviceChanges( int32 nbr ) {
   m_changed[ UserConstants::USER_DEVICECHANGES ] = true;
   m_deviceChanges = nbr;
}


bool
UserUser::mayChangeDevice() const {
   return (getDeviceChanges() > 0) || (getDeviceChanges() == -1);
}


bool
UserUser::useDeviceChange() {
   if ( getDeviceChanges() > 0 ) {
      setDeviceChanges( getDeviceChanges() -1 );
      return true;
   } else {
      return false;
   }
}

const char*
UserUser::getSupportComment() const {
   return m_supportComment;
}


void
UserUser::setSupportComment( const char* s ) {
   delete [] m_supportComment;
   m_supportComment = StringUtility::newStrDup( s );
   m_changed[ UserConstants::USER_SUPPORTCOMMENT ] = true;
}


const char*
UserUser::getPostalCity() const {
   return m_postalCity;
}


void
UserUser::setPostalCity( const char* s ) {
   delete [] m_postalCity;
   m_postalCity = StringUtility::newStrDup( s );
   m_changed[ UserConstants::USER_POSTALCITY ] = true;
}


const char*
UserUser::getZipCode() const {
   return m_zipCode;
}


void
UserUser::setZipCode( const char* s ) {
   delete [] m_zipCode;
   m_zipCode = StringUtility::newStrDup( s );
   m_changed[ UserConstants::USER_ZIPCODE ] = true;
}


const char*
UserUser::getCompanyName() const {
   return m_companyName;
}


void
UserUser::setCompanyName( const char* s ) {
   delete [] m_companyName;
   m_companyName = StringUtility::newStrDup( s );
   m_changed[ UserConstants::USER_COMPANYNAME ] = true;
}


const char*
UserUser::getCompanyReference() const {
   return m_companyReference;
}


void
UserUser::setCompanyReference( const char* s ) {
   delete [] m_companyReference;
   m_companyReference = StringUtility::newStrDup( s );
   m_changed[ UserConstants::USER_COMPANYREFERENCE ] = true;
}


const char*
UserUser::getCompanyVATNbr() const {
   return m_companyVATNbr;
}


void
UserUser::setCompanyVATNbr( const char* s ) {
   delete [] m_companyVATNbr;
   m_companyVATNbr = StringUtility::newStrDup( s );
   m_changed[ UserConstants::USER_COMPANYVATNBR ] = true;
}


int32
UserUser::getEmailBounces() const {
   return m_emailBounces;
}


void
UserUser::setEmailBounces( int32 nbr ) {
   m_changed[ UserConstants::USER_EMAILBOUNCES ] = true;
   m_emailBounces = nbr;
}


int32
UserUser::getAddressBounces() const {
   return m_addressBounces;
}


void 
UserUser::setAddressBounces( int32 nbr ) {
   m_changed[ UserConstants::USER_ADDRESSBOUNCES ] = true;
   m_addressBounces = nbr;
}


const char*
UserUser::getCustomerContactInfo() const {
   return m_customerContactInfo;
}


void
UserUser::setCustomerContactInfo( const char* s ) {
   delete [] m_customerContactInfo;
   m_customerContactInfo = StringUtility::newStrDup( s );
   m_changed[ UserConstants::USER_CUSTOMERCONTACTINFO ] = true;
}


MC2String
UserUser::getCustomerContactInfoField( const MC2String& name ) const {
   char* pos = strstr( m_customerContactInfo, name.c_str() );
   if ( pos != NULL && pos[ 1 ] == '=' ) {
      char* endPos = strchr( pos + 1, ';' );
      if ( endPos != NULL ) {
         return MC2String( pos + 1, (endPos - pos) - 1 );
      } else {
         return MC2String( pos + 1 );
      }
   } else {
      return "";
   }
}


void
UserUser::setCustomerContactInfoField( const MC2String& name, 
                                       const MC2String& value )
{
   MC2String customerContactInfo( m_customerContactInfo );
   MC2String::size_type findPos = customerContactInfo.find( name + "=" );
   if ( findPos != MC2String::npos ) {
      // Replace value
      MC2String::size_type startPos = findPos + name.size() + 1;
      MC2String::size_type endPos = customerContactInfo.find( 
         ';', startPos );
      customerContactInfo.replace( startPos, endPos - startPos, value );
   } else {
      // Add it
      customerContactInfo.append( name );
      customerContactInfo.append( "=" );
      customerContactInfo.append( value );
      customerContactInfo.append( ";" );
   }
   setCustomerContactInfo( customerContactInfo.c_str() );
}


bool
UserUser::setCustomerContactInfoField( const MC2String& param ) {
   // opt_in_prod_info=1
   bool ok = false;
   MC2String::size_type findPos = param.find( '=' );
   if ( findPos != MC2String::npos ) {
      MC2String name(  param.substr( 0, findPos ) );
      MC2String value( param.substr( findPos + 1 ) );
      setCustomerContactInfoField( name, value );
      ok = true;
   }

   return ok;
}


uint32 
UserUser::getNbrOfType( UserConstants::UserItemType type ) const 
{
   userElMap_t::const_iterator it = m_userElements.find( type );
   if ( it == m_userElements.end() ) {
      return 0;
   } else {
      return it->second.size();
   }
}


UserElement*
UserUser::getElementOfType( uint32 index, 
                            UserConstants::UserItemType type ) const 
{
   userElMap_t::const_iterator it = m_userElements.find( type );
   if ( it == m_userElements.end() ) {
      return NULL;
   } else {
      return it->second[index];
   }
}


void
UserUser::deleteElementOftype( uint32 index,
                               UserConstants::UserItemType type )
{
   userElMap_t::iterator it = m_userElements.find( type );

   delete it->second[index];
   
   it->second.erase( it->second.begin() + index );
}


UserElement* 
UserUser::getElementOfTypeWithID( UserConstants::UserItemType type,
                                  uint32 id ) const
{
   userElMap_t::const_iterator it = m_userElements.find( type );

   if ( it != m_userElements.end() ) {
      for( int i = 0, n = it->second.size(); i < n; ++i ) {
         if ( it->second[i]->getID() == id ) {
            return it->second[i];
         }
      }
   }
   return NULL;
}

uint32 
UserUser::getNbrElements() const
{
   uint32 sum = 0;
   for ( userElMap_t::const_iterator it = m_userElements.begin();
         it != m_userElements.end();
         ++it ) {
      sum += it->second.size();
   }
   return sum;
}


uint32
UserUser::getAllElements( vector<UserElement*>& dest ) const
{
   dest.reserve( dest.size() + getNbrElements() );
   for ( userElMap_t::const_iterator it = m_userElements.begin();
         it != m_userElements.end();
         ++it ) {
      dest.insert( dest.end(), it->second.begin(), it->second.end() );
   }
   return dest.size();
}

uint32
UserUser::getAllChangedElements( vector<UserElement*>& dest ) const
{
   dest.reserve( dest.size() + getNbrElements() );
   for ( userElMap_t::const_iterator it = m_userElements.begin();
         it != m_userElements.end();
         ++it ) {
      for ( int i = 0, n = it->second.size(); i < n; ++i ) {
         if ( it->second[i]->isChanged() ) {
            dest.push_back( it->second[i] );
         }
      }
   }
   return dest.size();
}

bool
UserUser::hasChangedElement() const
{
   for ( userElMap_t::const_iterator it = m_userElements.begin();
         it != m_userElements.end();
         ++it ) {
      for ( int i = 0, n = it->second.size(); i < n; ++i ) {
         if ( it->second[i]->isChanged() ) {
            return true;
         }
      }
   }
   return false;
}

uint32 
UserUser::getNbrChanged() const {
   uint32 nbr = 0;
   for ( uint32 i = 0 ; i < UserConstants::USER_NBRFIELDS ; i++ ) {
      if ( changed( UserConstants::UserDataField( i ) ) ) {
         nbr++;
      }
   }
   return nbr;
}


uint32
UserUser::printValue( char* target, UserConstants::UserDataField field,
                      bool stringInQuotes ) 
   const
{
   switch ( field ) {
      case UserConstants::USER_UIN : {
         sprintf( target, "%u", getUIN() );
         } break;
      case UserConstants::USER_LOGONID : {
         if ( stringInQuotes ) strcat( target, "'" );
         // Keeping this because its so funny!
         char* lowerLogonID = StringUtility::newStrDup(
            StringUtility::copyLower(MC2String(getLogonID())).c_str() ); 
         sqlString( target + (stringInQuotes ? 1: 0), lowerLogonID );
         if ( stringInQuotes ) strcat( target, "'" );
         delete [] lowerLogonID;
         } break;
      case UserConstants::USER_FIRSTNAME :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getFirstname() ); 
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_INITIALS :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getInitials() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_LASTNAME :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getLastname() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_SESSION :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getSession() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_MEASUREMENT_SYSTEM :
         sprintf( target, "%d", getMeasurementSystem() );
         break;
      case UserConstants::USER_LANGUAGE :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), 
            StringTable::getString(
            (StringTable::getLanguagesAsStringCode())[ getLanguage() ],
            StringTable::ENGLISH ) );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_LASTDEST_MAPID :
         sprintf( target, "%d", getLastdest_mapID() );
         break;
      case UserConstants::USER_LASTDEST_ITEMID :
         sprintf( target, "%d", getLastdest_itemID() );
         break;
      case UserConstants::USER_LASTDEST_OFFSET :
         sprintf( target, "%hd", (int16)getLastdest_offset() );
         break;
      case UserConstants::USER_LASTDEST_TIME :
         sprintf( target, "%d", getLastdest_time() );
         break;
      case UserConstants::USER_LASTDEST_STRING :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), 
                    getBrand() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_BIRTHDATE :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getBirthDate() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_LASTORIG_MAPID :
         sprintf( target, "%d", getLastorig_mapID() );
         break;
      case UserConstants::USER_LASTORIG_ITEMID :
         sprintf( target, "%d", getLastorig_itemID() );
         break;
      case UserConstants::USER_LASTORIG_OFFSET :
         sprintf( target, "%hd", (int16)getLastorig_offset() );
         break;
      case UserConstants::USER_LASTORIG_TIME :
         sprintf( target, "%d", getLastorig_time() );
         break;
      case UserConstants::USER_LASTORIG_STRING :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), 
                    getBrandOrigin() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_SEARCH_TYPE :
         sprintf( target, "%hd", getSearch_type() );
         break;
      case UserConstants::USER_SEARCH_SUBSTRING :
         sprintf( target, "%hd", getSearch_substring() );
         break;
      case UserConstants::USER_SEARCH_SORTING :
         sprintf( target, "%hd", getSearch_sorting() );
         break;
      case UserConstants::USER_SEARCH_OBJECTS :
         sprintf( target, "%d", m_searchObject );
         break;
      case UserConstants::USER_SEARCH_DBMASK :
         sprintf( target, "%hd", getSearch_DbMask() );
         break;
      case UserConstants::USER_ROUTING_COST_A :
         sprintf( target, "%hd", getRouting_costA() );
         break;
      case UserConstants::USER_ROUTING_COST_B :
         sprintf( target, "%hd", getRouting_costB() );
         break;
      case UserConstants::USER_ROUTING_COST_C :
         sprintf( target, "%hd", getRouting_costC() );
         break;
      case UserConstants::USER_ROUTING_COST_D :
         sprintf( target, "%hd", getRouting_costD() );
         break;
      case UserConstants::USER_ROUTING_TYPE :
         sprintf( target, "%hd", getRouting_type() );
         break;
      case UserConstants::USER_ROUTING_VEHICLE :
         sprintf( target, "%d", getRouting_vehicle() );
         break;
      case UserConstants::USER_ROUTEIMAGETYPE :
         sprintf( target, "%d", getRouteImageType() );
         break;
      case UserConstants::USER_GENDER :
         sprintf( target, "%d", getGender() );
         break;
      case UserConstants::USER_VALIDDATE :
         makeSQLOnlyDate(getValidDate(), target);
         break;
      case UserConstants::USER_EDIT_MAP_RIGHTS :
         sprintf( target, "%d", getEditMapRights() );
         break;
      case UserConstants::USER_EDIT_DELAY_RIGHTS :
         sprintf( target, "%d", getEditDelayRights() );
         break;
      case UserConstants::USER_EDIT_USER_RIGHTS :
         sprintf( target, "%d", getEditUserRights() );
         break;
      case UserConstants::USER_WAP_SERVICE :
         sprintf( target, "'%c'", getWAPService() ? 't' : 'f' );
         break;
      case UserConstants::USER_HTML_SERVICE :
         sprintf( target, "'%c'", getHTMLService() ? 't' : 'f' );
         break;
      case UserConstants::USER_OPERATOR_SERVICE :
         sprintf( target, "'%c'", getOperatorService() ? 't' : 'f' );
         break;
      case UserConstants::USER_SMS_SERVICE :
         sprintf( target, "'%c'", getSMSService() ? 't' : 'f' );
         break;
      case UserConstants::USER_DEFAULT_COUNTRY :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), 
                    getDefaultCountry() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_DEFAULT_MUNICIPAL :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), 
                    getDefaultMunicipal() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_DEFAULT_CITY :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), 
                    getDefaultCity() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_NBRMUNICIPAL : 
         sprintf( target, "%d", getNumMunicipal() );
         break;
      case UserConstants::USER_MUNICIPAL : {
         if ( stringInQuotes ) strcat( target, "'" );
         uint32 nbrMunicipal = getNumMunicipal();
         char cstr[40];
         for ( uint32 i = 0 ; i < nbrMunicipal ; i++ ) {
            sprintf( cstr, "%d:", getMunicipal( i ) );
            strcat( target, cstr );
         }
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      }
      case UserConstants::USER_NAV_SERVICE :
         sprintf( target, "'%c'", getNavService() ? 't' : 'f' );
         break;
      case UserConstants::USER_OPERATOR_COMMENT :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), 
                    getOperatorComment() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_EMAILADDRESS :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getEmailAddress() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_ADDRESS1 :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getAddress1() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_ADDRESS2 :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getAddress2() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_ADDRESS3 :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getAddress3() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_ADDRESS4 :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getAddress4() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_ADDRESS5 :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getAddress5() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_ROUTETURNIMAGETYPE :
         sprintf( target, "%hd", getRouteTurnImageType() );
         break;
      case UserConstants::USER_EXTERNALXMLSERVICE :
         sprintf( target, "'%c'", getExternalXmlService() ? 't' : 'f' );
         break;
      case UserConstants::USER_TRANSACTIONBASED :
         sprintf( target, "'%hd'", getTransactionBased() );
         break;
      case UserConstants::USER_DEVICECHANGES :
         sprintf( target, "%d", getDeviceChanges() );
         break;
      case UserConstants::USER_SUPPORTCOMMENT :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), 
                    getSupportComment() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_POSTALCITY :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getPostalCity() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_ZIPCODE :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getZipCode() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_COMPANYNAME :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), getCompanyName() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_COMPANYREFERENCE :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), 
                    getCompanyReference() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_COMPANYVATNBR :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0), 
                    getCompanyVATNbr() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;
      case UserConstants::USER_EMAILBOUNCES :
         sprintf( target, "%d", getEmailBounces() );
         break;
      case UserConstants::USER_ADDRESSBOUNCES :
         sprintf( target, "%d", getAddressBounces() );
         break;
      case UserConstants::USER_CUSTOMERCONTACTINFO :
         if ( stringInQuotes ) strcat( target, "'" );
         sqlString( target + (stringInQuotes ? 1 : 0),
                    getCustomerContactInfo() );
         if ( stringInQuotes ) strcat( target, "'" );
         break;


      default:
         mc2log << error << "UserUser::printValue unknown " 
                << "fieldType: " << (int)field << endl;
         break;
   };
   return strlen( target );
}


bool 
UserUser::isChanged() const {
   return getNbrChanged() > 0 || removed();
}


UserEnums::URType
UserUser::makeURType() const
{
   // I am assuming that there are no rights for this user.
   MC2_ASSERT( getNbrOfType( UserConstants::TYPE_RIGHT ) == 0 );
   
   UserEnums::URType ur;

   // Check if the UserWayfinderSubscription is something,
   // if it is -> add navservice and the correct
   // gold etc.
   bool foundSubscription = false;
   if ( getNbrOfType( UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) > 0  ) {
      // Can only have one
      uint8 userWFType = static_cast< UserWayfinderSubscription*> ( 
            getElementOfType( 
               0, UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) )
         ->getWayfinderType();
      foundSubscription = true;
      // Convert the old subscription type to URType
      ur |= UserEnums::wfstAsLevel(
         WFSubscriptionConstants::subscriptionsTypes ( userWFType ) );
   }
   
   if ( getNavService() || foundSubscription ) {
      ur |= UserEnums::UR_WF;
      ur |= UserEnums::UR_MAPDL_PREGEN;
      ur |= UserEnums::UR_MAPDL_CUSTOM;
      ur |= UserEnums::UR_SPEEDCAM;
      ur |= UserEnums::UR_TRAFFIC;
   }
   if ( getExternalXmlService() ) {
      ur |= UserEnums::UR_XML;
      ur |= UserEnums::UR_SPEEDCAM;      
      ur |= UserEnums::UR_TRAFFIC;
   }
   if ( getHTMLService() ) {
      ur |= UserEnums::UR_MYWAYFINDER;
      ur |= UserEnums::UR_MAPDL_PREGEN;
      ur |= UserEnums::UR_MAPDL_CUSTOM;
      ur |= UserEnums::UR_SPEEDCAM;
      ur |= UserEnums::UR_TRAFFIC;
   }
   
   return ur;
}

MapRights
UserUser::makeMapRights( const RegionIDs& regionInfo,
                         uint32 regionID, 
                         uint32 now ) const
{
   //fetch all rights
   constUserElRange_t rights = getElementRange( UserConstants::TYPE_RIGHT );
   if ( rights.first != rights.second ){ 
      //we have rights
      MapRights mapRights; //collect maprights here
      for ( constUserElRange_t::first_type it = rights.first; 
            it != rights.second; ++it ) {
         const UserRight& r = static_cast<const UserRight&>( **it );
         if ( r.isDeleted() || ! r.checkAccessAt( now ) ) {
            continue; //if the right is not active we go no further.
         }
         if ( r.getRegionID() == regionID ||   //IF the ids match OR ...
              regionID == MAX_UINT32  ||       //the regionID is special OR ..
              r.getRegionID() == MAX_INT32  || //the rights region ID is special ..
              //... OR the rights ID is a region_group AND the
              //regionID is an ordnary region contained in the
              //region_group ....
              ( ( regionInfo.isRegionGroupID( r.getRegionID() ) &&
                  ! regionInfo.isRegionGroupID( regionID ) ) &&
                ( regionInfo.isTheRegionInGroup(r.getRegionID(), regionID ) ) ) ) {
            mapRights |= toMapRight( r.getUserRightType() ); //...add the rights
         }
      }
      // As of MyWf 2.0 no MYWAYFINDER right is needed so BASIC_MAP
      // The whole concept of BASIC_MAP in MapRights is horribly broken,
      // it doesn't check which service (WF/MyWf) and client type it is...
      return mapRights | MapRights( MapRights::BASIC_MAP );
   } else {
      //no rights, use region access OLD SCHOOL!
      constUserElRange_t regionAccess = 
         getElementRange( UserConstants::TYPE_REGION_ACCESS );
      for( constUserElRange_t::first_type it = regionAccess.first;
           it != regionAccess.second; ++it ) {
         const UserRegionAccess& region = 
            static_cast<const UserRegionAccess&>( **it );
         if( region.getRegionID() == regionID || 
             region.getRegionID() == MAX_INT32 ||
             MAX_INT32 == regionID ){
            if( region.getStartTime() <= now && region.getEndTime() >= now ){
               return toMapRight( makeURType() ); //default
            }
         } else if( regionInfo.isRegionGroupID( region.getRegionID() ) &&
                    ! regionInfo.isRegionGroupID( regionID ) ) {
            vector<uint32> regionIDs;
            regionInfo.addRegionIDsFor( regionIDs, region.getRegionID() );
            for( vector<uint32>::iterator ti = regionIDs.begin();
                 ti != regionIDs.end(); ++ti ) {
               if ( *ti == regionID && 
                    region.getStartTime() <= now && 
                    region.getEndTime() >= now ) {
                  return toMapRight( makeURType() ); //default
               }
            }
         }
      }
   }
   // As of MyWf 2.0 no MYWAYFINDER right is needed so BASIC_MAP
   return MapRights() | MapRights( MapRights::BASIC_MAP );
}

MapRights UserUser::getAllMapRights( uint32 now ) const {
   MapRights mapRights; //collect maprights here
    //fetch all rights
   constUserElRange_t rights = getElementRange( UserConstants::TYPE_RIGHT );
   if ( rights.first != rights.second ){ 
      //we have rights

      for ( constUserElRange_t::first_type it = rights.first; 
            it != rights.second; ++it ) {
         const UserRight& r = static_cast<const UserRight&>( **it );
         if ( r.isDeleted() || ! r.checkAccessAt( now ) ) {
            continue;
         }

         mapRights |= toMapRight( r.getUserRightType() );

      }
   }
   return mapRights;
}
void
UserUser::updateRegionRightsCache( const RegionIDs& regionInfo,
                                   uint32 now,
                                   const vector<uint32>& topRegionIDs )
{
   mc2dbg4 << "[UserUser::updateRegionRightsCache]" << endl;
   uint32 startTime = TimeUtility::getCurrentTime();
   //will contain all regions that will be checked for rights.
   set<uint32> interestingRegions;

   //this scope extracts all known region_groups and the regions they
   //contain into the interestingRegions set.
   {
      vector<uint32> allGroups;
      
      regionInfo.addAllRegionGroups( allGroups ); //all group ids here
      for( vector<uint32>::const_iterator it = allGroups.begin();
           it != allGroups.end();
           ++it ) {
         //extract the regions contained in the group into interestingRegions
         vector<uint32> ordinary;
         regionInfo.addRegionIDsFor( ordinary, *it );
         interestingRegions.insert( ordinary.begin(),
                                    ordinary.end() );      
      }
      //add all the region group ids as well.
      interestingRegions.insert( allGroups.begin(), allGroups.end() );
   }
   //and add all topregion ids listed in topRegionIDs.
   interestingRegions.insert( topRegionIDs.begin(), topRegionIDs.end() );
   

   // Clear the old ones
   m_userRightsPerTopRegion.clear();
   for ( set<uint32>::const_iterator it = interestingRegions.begin();
         it != interestingRegions.end();
         ++it ) {
      //what rights does the user have for this region?
      m_userRightsPerTopRegion[*it] |= makeMapRights( regionInfo, *it, now );
   }
   mc2dbg << "[UserUser]: [" << getLogonID() << "]"
          << m_userRightsPerTopRegion.size()
          << " user rights regions created from "
          << getNbrOfType( UserConstants::TYPE_RIGHT ) << " rights in "
          << (TimeUtility::getCurrentTime() - startTime ) << " ms" << endl;
            
}

void
UserUser::getMapRightsForMap( uint32 mapID,
                              UserUser::regionRightMap_t& rights,
                              const MapRights& mask ) const
{
   if( mapID == MAX_UINT32 ) {
      for ( mapRightMap_t::const_iterator it = m_userRightPerMap.begin();
            it != m_userRightPerMap.end();
            ++it ) {
         // TODO: Put these in vector
         MapRights curRight( it->second.second & mask );
         if ( curRight ) {
            rights.insert( make_pair( it->first,
                                      curRight ) );
         }
      }
      return;
   }
   
   // Just add the one map if it is found
   mapRightMap_t::const_iterator findit = m_userRightPerMap.find( mapID );
   if ( m_userRightPerMap.end() != findit ) {
      MapRights curRight( findit->second.second & mask );
      if ( curRight ) {
         rights.insert( make_pair( findit->first,
                                   curRight ) );
      }
   } else if ( MapBits::isOverviewMap( mapID ) ) {
      // Add all the overview maps.
      for ( mapRightMap_t::const_iterator it = m_userRightPerMap.begin();
            it != m_userRightPerMap.end();
            ++it ) {
         if ( MapBits::isOverviewMap( it->first ) ) {
            MapRights curRight( it->second.second & mask );
            if ( curRight ) {
               rights.insert( make_pair( it->first,
                                         curRight ) );
            }
         }
      }
   }   
}

MapRights
UserUser::getMapRightsForMap( uint32 mapID,
                              const MapRights& mask ) const
{
   MapRights right;
   UserUser::regionRightMap_t rights;
   getMapRightsForMap( mapID, rights, mask );
   for ( UserUser::regionRightMap_t::const_iterator it = rights.begin() ;
         it != rights.end() ; ++it ) {
      right |= it->second;
   }

   return right;
}

bool
UserUser::hasAnyRightIn( const MapRights& rightsToFind ) const
{
   for ( regionRightMap_t::const_iterator it =
            m_userRightsPerTopRegion.begin();
         it != m_userRightsPerTopRegion.end();
         ++it ) {
      if ( rightsToFind & it->second ){
         return true;
      }
   }
   return false;
}

const UserUser::regionRightMap_t&
UserUser::getRegionRightsCache() const
{
   return m_userRightsPerTopRegion;
}


void
UserUser::swapInMapRightsCache( mapRightMap_t& newRights )
{
   m_userRightPerMap.swap( newRights );
}

uint32 
UserUser::getHighestVersionLock() const
{
   uint32 highest = 0;
   const time_t now = time( NULL );
   const constUserElRange_t range = getElementRange( UserConstants::TYPE_RIGHT );
   for(constUserElRange_t::first_type it = range.first; it != range.second; ++it){
      const UserRight& right = static_cast<const UserRight&>(**it);
      if( right.notDeletedAndValidAt( now ) ){
         highest = max( highest, right.getVersionLock( 0 ) );
      }
   }
   if( highest == 0 ){
      //no version lock was found. Let the user do everything!
      highest = MAX_UINT32;
   }
   return highest;
}

bool 
UserUser::hasActive( const UserEnums::URType& ur ) const
{
   const constUserElRange_t range = getElementRange( UserConstants::TYPE_RIGHT );
   const time_t now = time( NULL );
   for(constUserElRange_t::first_type it = range.first;
       it != range.second; ++it){
      const UserRight& right = static_cast<const UserRight&>(**it);
      if( right.notDeletedAndValidAt( now ) ) {
         if( ur == right.getUserRightType() ){
            return true;
         }
      }
   }
   return false;
}

UserRight*
UserUser::getMatchingRight( const UserRight* r ) {
   userElRange_t range = getElementRange( UserConstants::TYPE_RIGHT );
   for( userElRange_t::first_type it = range.first ;
        it != range.second ; ++it ){
      UserRight* right = static_cast< UserRight* > ( *it );
      if ( right->getUserRightType() == r->getUserRightType() &&
           right->getRightTimeLength() == r->getRightTimeLength() &&
           right->getRegionID() == r->getRegionID() ) {
         return right;
      }
   }
   return NULL;
}


// === UserCellular =======================================

UserCellular::UserCellular( uint32 ID )
      : UserElement(UserConstants::TYPE_CELLULAR)
{
   m_id = ID;
   m_phoneNumber = new char[1];
   m_phoneNumber[0] = '\0';
   m_smsParams = 0;
   m_maxSearchHitsWap = 6;
   m_maxRouteLinesWap = 50;
   m_eol = UserConstants::EOLTYPE_NOT_DEFINED;
   m_smsLineLength = MAX_UINT8;
   m_model = new CellularPhoneModel();
   m_modelName = StringUtility::newStrDup( m_model->getName() );
   // smssms
   m_smsService = true;
   // Positioning attributes
   m_positioningActive = false;
   m_typeOfPositioning = UserConstants::POSTYPE_NO_POSITIONING;
   m_posUserName = new char[1];
   m_posUserName[0] = '\0';
   m_posPassword = new char[1];
   m_posPassword[0] = '\0';
   m_lastpos_lat = 0;
   m_lastpos_long = 0;
   m_lastpos_innerRadius = 0;
   m_lastpos_outerRadius = 0;
   m_lastpos_startAngle = 0;
   m_lastpos_stopAngle = 0;
   m_lastpos_time = 0;
   
   setOk( true );

   for (uint8 i = 0; i < UserConstants::CELLULAR_NBRFIELDS; i++)
      m_changed[i] = false;
}


UserCellular::UserCellular( uint32 ID,
                            const char* phoneNumber,
                            uint8 smsParams,
                            uint16 maxSearchHitsWap,
                            uint16 maxRouteLinesWap,
                            bool smsService,
                            CellularPhoneModel* model,
                            bool positioningActive,
                            UserConstants::posType typeOfPositioning,
                            const char* posUserName,
                            const char* posPassword,
                            uint32 lastpos_lat,
                            uint32 lastpos_long,
                            uint32 lastpos_innerRadius,
                            uint32 lastpos_outerRadius,
                            uint32 lastpos_startAngle,
                            uint32 lastpos_stopAngle,
                            uint32 lastpos_time,
                            UserConstants::EOLType eol,
                            uint8 smsLineLength)
      : UserElement( UserConstants::TYPE_CELLULAR )
{
   m_id = ID;
   m_phoneNumber = new char[strlen(phoneNumber)+1];
   strcpy(m_phoneNumber, phoneNumber);
   // m_model = model;
   m_smsParams = smsParams;
   m_maxSearchHitsWap = maxSearchHitsWap;
   m_maxRouteLinesWap = maxRouteLinesWap;
   // smssms
   m_smsService = smsService;
   m_eol = eol;
   m_smsLineLength = smsLineLength;
   m_positioningActive = positioningActive;
   m_typeOfPositioning = typeOfPositioning;
   m_posUserName = new char[strlen(posUserName)+1];
   strcpy( m_posUserName, posUserName );
   m_posPassword = new char[strlen(posPassword)+1];
   strcpy( m_posPassword, posPassword );
   m_lastpos_lat = lastpos_lat;
   m_lastpos_long = lastpos_long;
   m_lastpos_innerRadius = lastpos_innerRadius;
   m_lastpos_outerRadius = lastpos_outerRadius;
   m_lastpos_startAngle = lastpos_startAngle;
   m_lastpos_stopAngle = lastpos_stopAngle;
   m_lastpos_time = lastpos_time;
   m_model = model;
   if ( m_model != NULL )
      m_modelName = StringUtility::newStrDup( m_model->getName() );
   else 
      m_modelName = StringUtility::newStrDup( "UNKNOWN" );
   
   setOk( true );

   for (uint8 i = 0; i < UserConstants::CELLULAR_NBRFIELDS; i++)
      m_changed[i] = false;
}


UserCellular::UserCellular(  const UserReplyPacket* p, int& pos )
      : UserElement( UserConstants::TYPE_CELLULAR )
{
   int32 startPos = pos;
   uint32 type = p->incReadLong( pos );

   if ( type == UserConstants::TYPE_CELLULAR ) {
      uint32 size = p->incReadShort( pos );
      if ( (6+12 + 8*sizeof(uint32)+sizeof(bool) < size)
           && ( (startPos - 3 + size) < p->getLength()) ) {
         // 6 is the two start TYPE_CELLULAR and size
         // 11 is the data read here
         m_id = p->incReadLong( pos );
         m_smsParams = p->incReadByte( pos );
         m_maxSearchHitsWap = p->incReadShort( pos );
         m_maxRouteLinesWap = p->incReadShort( pos );
         // smssms
         m_smsService = p->incReadByte( pos ) != 0;
         m_eol = UserConstants::EOLType( p->incReadLong( pos ) );
         m_smsLineLength = p->incReadByte( pos );
         m_positioningActive = p->incReadByte( pos )!=0;
         m_typeOfPositioning = UserConstants::posType( p->incReadLong( pos ) );
         m_lastpos_lat = p->incReadLong( pos );
         m_lastpos_long = p->incReadLong( pos );
         m_lastpos_innerRadius = p->incReadLong( pos );
         m_lastpos_outerRadius = p->incReadLong( pos );
         m_lastpos_startAngle = p->incReadLong( pos );
         m_lastpos_stopAngle = p->incReadLong( pos );
         m_lastpos_time = p->incReadLong( pos );
         char* str;
         uint32 strLen;
         uint32 strSize;
         strSize = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( strSize != strLen ) {
            DEBUG1(mc2log << error << "UserCellular::UserCellular( p, pos ) "
                   << "Strlen error" << endl;);
            return; 
         }
         m_phoneNumber = new char[strLen + 1];
         strcpy( m_phoneNumber, str );
         
         strSize = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         
         m_posUserName = new char[strLen + 1];
         strcpy( m_posUserName, str);

         strSize = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         
         m_posPassword = new char[strLen + 1];
         strcpy( m_posPassword, str);

         if ( 6+13+strLen < size ) { 
           
            
            // Read modelname
            p->incReadShort( pos ); // String length
            p->incReadString( pos, str );
            m_modelName = StringUtility::newStrDup( str );
            
            // Read Cellular phone model
            type = p->incReadLong( pos );
            uint16 size = p->incReadShort( pos );
            if ( type == (MAX_UINT8 / 2) && 
                 pos - 6 + size < (int32)p->getLength() )
            {
               m_model = new CellularPhoneModel( p, pos );
               if ( !m_model->getValid() ) {
                  DEBUG1(mc2log << warn
                         << "UserCellular::UserCellular( p, pos ) "
                         << "Cellular model not OK!" << endl;);
                  return;
               }
               type = p->incReadLong( pos );
               if ( type != UserConstants::TYPE_CELLULAR ) {
                  DEBUG1(mc2log << warn
                         << "UserCellular::UserCellular( p, pos ) "
                         << "Data not ended by TYPE_CELLULAR!!!" << endl;);
                  return;
               }
               // DONE
            } else {
               DEBUG1(mc2log << warn << "UserCellular::UserCellular( p, pos ) "
                      << "No room for Cellular model!" << endl;);
               return;
            }
         } else {
            m_phoneNumber[0] = '\0';
            DEBUG1(mc2log << warn << 
                   "UserCellular::UserCellular( UserReplyPacket* p, pos ) "
                   << "Not space for phonenumber!!" << endl;);
            return;
         }
      } else {
         DEBUG1(mc2log << warn  
                << "UserCellular::UserCellular( UserReplyPacket* p, pos ) "
                << "Not size for data!! Nothing read" << endl;);
         return;
      }
   } else {
      DEBUG1( mc2log << warn 
              << "UserCellular::UserCellular(  UserReplyPacket* p, pos ) "
              << "Not TYPE_CELLULAR type!!!!!" << endl;);
      return;
   }

   for (uint8 i = 0; i < UserConstants::CELLULAR_NBRFIELDS; i++)
      m_changed[i] = false;

   setOk( true );
}


UserCellular::UserCellular( const UserCellular& cel ) : UserElement( cel )
{
   m_id = cel.m_id;
   m_phoneNumber = StringUtility::newStrDup( cel.m_phoneNumber );
   m_smsParams = cel.m_smsParams;
   m_maxSearchHitsWap = cel.m_maxSearchHitsWap;
   m_maxRouteLinesWap = cel.m_maxRouteLinesWap;
   // smssms
   m_smsService = cel.m_smsService;
   m_eol = cel.m_eol;
   m_smsLineLength = cel.m_smsLineLength;
   m_positioningActive = cel.m_positioningActive;
   m_typeOfPositioning = cel.m_typeOfPositioning;
   m_posUserName = StringUtility::newStrDup(cel.m_posUserName);
   m_posPassword =  StringUtility::newStrDup(cel.m_posPassword);
   m_modelName = StringUtility::newStrDup( cel.m_modelName );
   m_lastpos_lat = cel.m_lastpos_lat;
   m_lastpos_long = cel.m_lastpos_long;
   m_lastpos_innerRadius = cel.m_lastpos_innerRadius;
   m_lastpos_outerRadius = cel.m_lastpos_outerRadius;
   m_lastpos_startAngle = cel.m_lastpos_startAngle;
   m_lastpos_stopAngle = cel.m_lastpos_stopAngle;
   m_lastpos_time = cel.m_lastpos_time;
   m_model = new CellularPhoneModel( *cel.m_model );
   
   uint32 i = 0;
   for ( i = 0 ; i < UserConstants::CELLULAR_NBRFIELDS ; i++ ) {
      m_changed[ i ] = cel.m_changed[ i ];
   }
}


UserCellular::~UserCellular()
{
   delete [] m_phoneNumber;
   delete [] m_modelName;
   delete m_model;
   delete [] m_posUserName;
   delete [] m_posPassword;
}


void 
UserCellular::packInto( Packet* p, int& pos ) {
   uint16 phoneLen = strlen(m_phoneNumber);
   uint16 nameLen = strlen(m_modelName);
   uint16 posUserLen = strlen(m_posUserName);
   uint16 posPassLen = strlen(m_posPassword);
   uint16 size = 24 + 8*sizeof( uint32 ) + sizeof( bool ) +
      phoneLen + 1 + nameLen + 1 + posUserLen + 1 + posPassLen + 1;
   
   p->incWriteLong( pos, UserConstants::TYPE_CELLULAR );
   p->incWriteShort( pos, size );
   p->incWriteLong( pos, UserConstants::TYPE_CELLULAR );
   p->incWriteShort( pos, size );

   p->incWriteLong( pos, m_id );
   p->incWriteByte( pos, m_smsParams );
   p->incWriteShort( pos, m_maxSearchHitsWap );
   p->incWriteShort( pos, m_maxRouteLinesWap );
   // smssms
   p->incWriteByte( pos, m_smsService );
   p->incWriteLong( pos, m_eol ); 
   p->incWriteByte( pos, m_smsLineLength );
   p->incWriteByte( pos, m_positioningActive );
   p->incWriteLong( pos, m_typeOfPositioning );
   p->incWriteLong( pos, m_lastpos_lat );
   p->incWriteLong( pos, m_lastpos_long );
   p->incWriteLong( pos, m_lastpos_innerRadius );
   p->incWriteLong( pos, m_lastpos_outerRadius );
   p->incWriteLong( pos, m_lastpos_startAngle );
   p->incWriteLong( pos, m_lastpos_stopAngle );
   p->incWriteLong( pos, m_lastpos_time );
   p->incWriteShort( pos, phoneLen );
   p->incWriteString( pos, m_phoneNumber );
   p->incWriteShort( pos, posUserLen );
   p->incWriteString( pos, m_posUserName );
   p->incWriteShort( pos, posPassLen );
   p->incWriteString( pos, m_posPassword );
   p->incWriteShort( pos, nameLen );
   p->incWriteString( pos, m_modelName );
   
   m_model->packInto( p, pos );

   p->incWriteLong( pos, UserConstants::TYPE_CELLULAR );
   p->incWriteLong( pos, UserConstants::TYPE_CELLULAR );

   // Set length 
   p->setLength( pos );
}


uint32 UserCellular::getSize() const
{
   return (strlen(m_phoneNumber) + 1 + strlen(m_modelName) + 1 +
           strlen(m_posUserName) + 1 + strlen(m_posPassword) + 1) 
      * sizeof(char)
      + 10 * sizeof(uint32) + 1 * sizeof(uint8) + 6 * sizeof(uint16) 
      + 3 * sizeof(bool) + 16 + 2*4 + 3;
}


void UserCellular::addChanges(Packet* p, int& position)
{
   mc2dbg4 << "UserCellular::addChanges " << endl;
   uint8 nbrChanges = 0;
   for (uint8 i = 0; i < UserConstants::CELLULAR_NBRFIELDS; i++)
      if (m_changed[i])
         nbrChanges++;
   if ( nbrChanges > 0 || removed() ) {
      p->incWriteLong(position, UserConstants::TYPE_CELLULAR);
      p->incWriteLong(position, m_id);
      p->incWriteLong(position, UserConstants::TYPE_CELLULAR);
      p->incWriteLong(position, m_id);
      if (removed()) {
         p->incWriteByte(position, UserConstants::ACTION_DELETE);
      }
      else if (m_id == 0) {
         p->incWriteByte(position, UserConstants::ACTION_NEW);
         // Not id and userUIN
         p->incWriteByte(position, UserConstants::CELLULAR_NBRFIELDS - 2);
         p->incWriteByte(position, UserConstants::CELLULAR_PHONENUMBER);
         p->incWriteString(position, m_phoneNumber);
         p->incWriteByte(position, UserConstants::CELLULAR_MODEL);
         p->incWriteString(position, m_modelName);
         p->incWriteByte(position, UserConstants::CELLULAR_SMSPARAMS);
         p->incWriteByte(position, m_smsParams);
         p->incWriteByte(position, UserConstants::CELLULAR_EOL_TYPE);
         p->incWriteLong(position, m_eol);
         p->incWriteByte(position, UserConstants::CELLULAR_CHARS_PER_LINE);
         p->incWriteByte(position, m_smsLineLength);
         p->incWriteByte(position, 
                         UserConstants::CELLULAR_MAXSEARCHHITSWAP);
         p->incWriteShort(position, m_maxSearchHitsWap);
         p->incWriteByte(position, 
                         UserConstants::CELLULAR_MAXROUTELINESWAP);
         p->incWriteShort(position, m_maxRouteLinesWap);
         // smssms
         p->incWriteByte(position, UserConstants::CELLULAR_SMSSERVICE);
         p->incWriteByte(position, m_smsService);
         //positioning
         p->incWriteByte(position, UserConstants::CELLULAR_POSACTIVE );
         p->incWriteByte(position, m_positioningActive);
         p->incWriteByte(position, UserConstants::CELLULAR_TYPE_OF_POS);
         p->incWriteLong(position, m_typeOfPositioning);
         p->incWriteByte(position, UserConstants::CELLULAR_POS_USERNAME);
         p->incWriteString(position, m_posUserName);
         p->incWriteByte(position, UserConstants::CELLULAR_POS_PASSWORD);
         p->incWriteString(position, m_posPassword);
         p->incWriteByte(position, UserConstants::CELLULAR_LASTPOS_LAT);
         p->incWriteLong(position, m_lastpos_lat);
         p->incWriteByte(position, UserConstants::CELLULAR_LASTPOS_LONG);
         p->incWriteLong(position, m_lastpos_long);
         p->incWriteByte(position,
                         UserConstants::CELLULAR_LASTPOS_INNERRADIUS);
         p->incWriteLong(position, m_lastpos_innerRadius);
         p->incWriteByte(position,
                         UserConstants::CELLULAR_LASTPOS_OUTERRADIUS);
         p->incWriteLong(position, m_lastpos_outerRadius);
         p->incWriteByte(position,
                         UserConstants::CELLULAR_LASTPOS_STARTANGLE);
         p->incWriteLong(position, m_lastpos_startAngle);
         p->incWriteByte(position,
                         UserConstants::CELLULAR_LASTPOS_STOPANGLE);
         p->incWriteLong(position, m_lastpos_stopAngle);
         p->incWriteByte(position,
                         UserConstants::CELLULAR_LASTPOS_TIME);
         p->incWriteLong(position, m_lastpos_time);
      }
      else {
         p->incWriteByte(position, UserConstants::ACTION_CHANGE);
         p->incWriteByte(position, nbrChanges);
         if (m_changed[UserConstants::CELLULAR_PHONENUMBER]) {
            p->incWriteByte(position, UserConstants::CELLULAR_PHONENUMBER);
            p->incWriteString(position, m_phoneNumber);
         }
         if (m_changed[UserConstants::CELLULAR_MODEL]) {
            p->incWriteByte(position, UserConstants::CELLULAR_MODEL);
            p->incWriteString(position, m_modelName);
         }
         if ( m_changed[ UserConstants::CELLULAR_SMSPARAMS ] ) {
            p->incWriteByte(position, UserConstants::CELLULAR_SMSPARAMS);
            p->incWriteByte(position, m_smsParams);
         }
         if (m_changed[UserConstants::CELLULAR_MAXSEARCHHITSWAP]) {
            p->incWriteByte(position, 
                            UserConstants::CELLULAR_MAXSEARCHHITSWAP);
            p->incWriteShort(position, m_maxSearchHitsWap);
         }
         if (m_changed[UserConstants::CELLULAR_MAXROUTELINESWAP]) {
            p->incWriteByte(position, 
                            UserConstants::CELLULAR_MAXROUTELINESWAP);
            p->incWriteShort(position, m_maxRouteLinesWap);
         }
         // smssms
         if (m_changed[UserConstants::CELLULAR_SMSSERVICE]) {
            p->incWriteByte(position, UserConstants::CELLULAR_SMSSERVICE);
            p->incWriteByte(position, m_smsService);
         }
         if (m_changed[UserConstants::CELLULAR_EOL_TYPE]) {
            p->incWriteByte(position, UserConstants::CELLULAR_EOL_TYPE);
            p->incWriteLong(position, m_eol);
         }
         if (m_changed[UserConstants::CELLULAR_CHARS_PER_LINE]) {
            p->incWriteByte(position, 
                            UserConstants::CELLULAR_CHARS_PER_LINE);
            p->incWriteByte(position, m_smsLineLength);
         }
         if (m_changed[UserConstants::CELLULAR_POSACTIVE]) {
            p->incWriteByte(position, 
                            UserConstants::CELLULAR_POSACTIVE);
            p->incWriteByte(position, m_positioningActive);
         }
         if (m_changed[UserConstants::CELLULAR_TYPE_OF_POS]) {
            p->incWriteByte(position, UserConstants::CELLULAR_TYPE_OF_POS);
            p->incWriteLong(position, m_typeOfPositioning);
         }
         if (m_changed[UserConstants::CELLULAR_POS_USERNAME]) {
            p->incWriteByte(position, UserConstants::CELLULAR_POS_USERNAME);
            p->incWriteString(position, m_posUserName);
         }
         if (m_changed[UserConstants::CELLULAR_POS_PASSWORD]) {
            p->incWriteByte(position, UserConstants::CELLULAR_POS_PASSWORD);
            p->incWriteString(position, m_posPassword);
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_LAT]) {
            p->incWriteByte(position, UserConstants::CELLULAR_LASTPOS_LAT);
            p->incWriteLong(position, m_lastpos_lat);
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_LONG]) {
            p->incWriteByte(position, UserConstants::CELLULAR_LASTPOS_LONG);
            p->incWriteLong(position, m_lastpos_long);
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_INNERRADIUS]) {
            p->incWriteByte(position,
                            UserConstants::CELLULAR_LASTPOS_INNERRADIUS);
            p->incWriteLong(position, m_lastpos_innerRadius);
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_OUTERRADIUS]) {
            p->incWriteByte(position,
                            UserConstants::CELLULAR_LASTPOS_OUTERRADIUS);
            p->incWriteLong(position, m_lastpos_outerRadius);
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_STARTANGLE]) {
            p->incWriteByte(position,
                            UserConstants::CELLULAR_LASTPOS_STARTANGLE);
            p->incWriteLong(position, m_lastpos_startAngle);
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_STOPANGLE]) {
            p->incWriteByte(position,
                            UserConstants::CELLULAR_LASTPOS_STOPANGLE);
            p->incWriteLong(position, m_lastpos_stopAngle);
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_TIME]) {
            p->incWriteByte(position,
                            UserConstants::CELLULAR_LASTPOS_TIME);
            p->incWriteLong(position, m_lastpos_time);
         }
         

      }
      p->incWriteLong(position, UserConstants::TYPE_CELLULAR);
      p->incWriteLong(position, UserConstants::TYPE_CELLULAR);
      for (uint8 i = 0; i < UserConstants::CELLULAR_NBRFIELDS; i++)
         m_changed[i] = false;
      p->setLength( position );
   }
}


uint32 
UserCellular::getNbrChanged() const {
   uint32 nbr = 0;
   for ( uint32 i = 0 ; i < UserConstants::CELLULAR_NBRFIELDS ; i++ ) {
      if ( changed( UserConstants::UserCellularField( i ) ) ) {
         nbr++;
      }
   }
   return nbr;
}


bool
UserCellular::readChanges( const Packet* p, int& pos, 
                           UserConstants::UserAction& action )
{
   mc2dbg4 << "UserCellular::readChanges pos " << pos << endl;
   if ( p->getLength() - pos - 5 > 0 ) {
      uint32 type;
      uint32 id;
      
      type = p->incReadLong( pos );
      id = p->incReadLong( pos );
      if ( type == UserConstants::TYPE_CELLULAR && id == m_id ) {
         action = UserConstants::UserAction(p->incReadByte( pos ) );
         switch ( action ) {
            case UserConstants::ACTION_NEW:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_DELETE:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_CELLULAR ) {
                  remove();
                  return true;
               } else {
                  DEBUG1(mc2log << warn
                         << "UserCellular::readChanges not Type "
                         << " Cellular after action delete" << endl;);
                  return false;
               }
               break;
            case UserConstants::ACTION_CHANGE:
               return ( readDataChanges( p, pos ) );
               break;
            case UserConstants::ACTION_NOP:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_CELLULAR ) {
                  return true;
               } else {
                  DEBUG1(mc2log << warn
                         << "UserCellular::readChanges not Type "
                         " Cellular after action NOP" << endl;);
                  return false;
               }
               break;
            default:
               DEBUG1(mc2log << warn <<
                      "UserCellular::readChanges unknown action: "
                      << (int)action << endl;);
         };
         return false;
      } else {
         DEBUG1(mc2log << error
                << "UserCellular::readChanges not correct type or "
                << " not correct id, giving up." << endl;);
         return false;  
      }
   } else {
      DEBUG1(mc2log << error
             << "UserCelllular::readChanges no space for changes "
             << "giving up." << endl;);
      return false;
   }
}


bool 
UserCellular::readDataChanges( const Packet* p, int& pos ) {

   if ( p->getLength() - pos - 1 > 0 ) {
      byte nbrFields;

      nbrFields = p->incReadByte( pos );
      
      // tmpData
      char* phoneNumber = m_phoneNumber;
      char* modelName = m_modelName;
      uint32 phoneNumberSize;
      uint32 modelNameSize;
      uint8 smsParams = m_smsParams;
      uint16 maxSearchHitsWap = m_maxSearchHitsWap;
      uint16 maxRouteLinesWap = m_maxRouteLinesWap;
      // smssms
      bool smsService = m_smsService;
      UserConstants::EOLType eol = m_eol;
      uint8 smsLineLength = m_smsLineLength;
      // positioning
      bool positioningActive = m_positioningActive;
      UserConstants::posType typeOfPositioning = m_typeOfPositioning;
      char* posUserName = m_posUserName;
      char* posPassword = m_posPassword;
      uint32 posUserSize;
      uint32 posPassSize;
      uint32 lastpos_lat = m_lastpos_lat;
      uint32 lastpos_long = m_lastpos_long;
      uint32 lastpos_innerRadius = m_lastpos_innerRadius;
      uint32 lastpos_outerRadius = m_lastpos_outerRadius;
      uint32 lastpos_startAngle = m_lastpos_startAngle;
      uint32 lastpos_stopAngle = m_lastpos_stopAngle;
      uint32 lastpos_time = m_lastpos_time;

      byte field;
      bool ok = true;
      for ( uint8 i = 0 ; 
            (i < nbrFields) && ok && (pos < (int32)p->getLength()) ;
            i ++ ) 
      {
         field = p->incReadByte( pos );
         switch ( field ) {
            case UserConstants::CELLULAR_PHONENUMBER :
               m_changed[ UserConstants::CELLULAR_PHONENUMBER ] = true;
               phoneNumberSize = p->incReadString( pos, phoneNumber );
               break;
            case UserConstants::CELLULAR_MODEL :
               m_changed[ UserConstants::CELLULAR_MODEL ] = true;
               modelNameSize = p->incReadString( pos, modelName );
               break;   
            case UserConstants::CELLULAR_SMSPARAMS :
               m_changed[ UserConstants::CELLULAR_SMSPARAMS ] = true;
               smsParams = p->incReadByte( pos );
               break;
            case UserConstants::CELLULAR_MAXSEARCHHITSWAP :
               m_changed[ UserConstants::CELLULAR_MAXSEARCHHITSWAP ] 
                  = true;
               maxSearchHitsWap = p->incReadShort( pos );
               break;
            case UserConstants::CELLULAR_MAXROUTELINESWAP :
               m_changed[ UserConstants::CELLULAR_MAXROUTELINESWAP ] 
                  = true;
               maxRouteLinesWap = p->incReadShort( pos );
               break;
               // smssms
            case UserConstants::CELLULAR_SMSSERVICE :
               m_changed[ UserConstants::CELLULAR_SMSSERVICE ] = false;
               smsService = p->incReadByte( pos ) != 0;
               break;
            case UserConstants::CELLULAR_EOL_TYPE :
               m_changed[ UserConstants::CELLULAR_EOL_TYPE ] = true;
               eol = UserConstants::EOLType( p->incReadLong( pos ) );
               break;
            case UserConstants::CELLULAR_CHARS_PER_LINE :
               m_changed[ UserConstants::CELLULAR_CHARS_PER_LINE ] = true;
               smsLineLength = p->incReadByte( pos );
               break;
            case UserConstants::CELLULAR_POSACTIVE :
               m_changed[ UserConstants::CELLULAR_POSACTIVE ] = true;
               positioningActive = p->incReadByte( pos )!=0;
               break;
            case UserConstants::CELLULAR_TYPE_OF_POS :
               m_changed[ UserConstants::CELLULAR_TYPE_OF_POS ] = true;
               typeOfPositioning =
                  UserConstants::posType( p->incReadLong( pos ) );
               break;
            case UserConstants::CELLULAR_POS_USERNAME :
               m_changed[ UserConstants::CELLULAR_POS_USERNAME ] = true;
               posUserSize = p->incReadString( pos, posUserName );
               break;
            case UserConstants::CELLULAR_POS_PASSWORD :
               m_changed[ UserConstants::CELLULAR_POS_PASSWORD ] = true;
               posPassSize = p->incReadString( pos, posPassword );
               break;
            case UserConstants::CELLULAR_LASTPOS_LAT :
               m_changed[ UserConstants::CELLULAR_LASTPOS_LAT ] 
                  = true;
               lastpos_lat = p->incReadLong( pos );
               break;
            case UserConstants::CELLULAR_LASTPOS_LONG :
               m_changed[ UserConstants::CELLULAR_LASTPOS_LONG ] 
                  = true;
               lastpos_long = p->incReadLong( pos );
               break;
            case UserConstants::CELLULAR_LASTPOS_INNERRADIUS :
               m_changed[ UserConstants::CELLULAR_LASTPOS_INNERRADIUS ] 
                  = true;
               lastpos_innerRadius = p->incReadLong( pos );
               break;
            case UserConstants::CELLULAR_LASTPOS_OUTERRADIUS :
               m_changed[ UserConstants::CELLULAR_LASTPOS_OUTERRADIUS ] 
                  = true;
               lastpos_outerRadius = p->incReadLong( pos );
               break;
            case UserConstants::CELLULAR_LASTPOS_STARTANGLE :
               m_changed[ UserConstants::CELLULAR_LASTPOS_STARTANGLE ] 
                  = true;
               lastpos_startAngle = p->incReadLong( pos );
               break;
            case UserConstants::CELLULAR_LASTPOS_STOPANGLE :
               m_changed[ UserConstants::CELLULAR_LASTPOS_STOPANGLE ]
                  = true;
               lastpos_stopAngle = p->incReadLong( pos );
               break;
            case UserConstants::CELLULAR_LASTPOS_TIME :
               m_changed[ UserConstants::CELLULAR_LASTPOS_TIME ]
                  = true;
               lastpos_time = p->incReadLong( pos );
               break;
            default:
               DEBUG1(mc2log << error
                      << "UserCellular::readDataChanges unknown " 
                      << "fieldType: " << (int)field << endl;);
               ok = false;
               break;
         };
      }
      uint32 type = p->incReadLong( pos );
      if ( type != UserConstants::TYPE_CELLULAR ) {
         DEBUG1(mc2log << error
                << "UserCellular::readDataChanges Not Cellular "
                << " type after data" << endl;);
         ok = false;
      }
      if ( ok ) {
         // Transfer changes to membervariables
         if (m_changed[UserConstants::CELLULAR_PHONENUMBER]) {
            setPhoneNumber( phoneNumber );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL]) {
            delete [] m_modelName;
            m_modelName = StringUtility::newStrDup( modelName );
         }
         if (m_changed[UserConstants::CELLULAR_SMSPARAMS]) {
            setSMSParams( smsParams );
         }
         if (m_changed[UserConstants::CELLULAR_MAXSEARCHHITSWAP]) {
            setMaxSearchHitsWap( maxSearchHitsWap );
         }
         if (m_changed[UserConstants::CELLULAR_MAXROUTELINESWAP]) {
            setMaxRouteLinesWap( maxRouteLinesWap );
         }
         // smssms
         if (m_changed[UserConstants::CELLULAR_SMSSERVICE]) {
            //setSmsService( smsService );
         }
         if (m_changed[UserConstants::CELLULAR_EOL_TYPE]) {
            setEOLType( eol );
         }
         if (m_changed[UserConstants::CELLULAR_CHARS_PER_LINE]) {
            setSMSLineLength( smsLineLength );
         }
         if (m_changed[UserConstants::CELLULAR_POSACTIVE]) {
            setPosActive( positioningActive );
         }
         if (m_changed[UserConstants::CELLULAR_TYPE_OF_POS]) {
            setTypeOfPos( typeOfPositioning );
         }
         if (m_changed[UserConstants::CELLULAR_POS_USERNAME]) {
            delete [] m_posUserName;
            m_posUserName = StringUtility::newStrDup( posUserName );
         }
         if (m_changed[UserConstants::CELLULAR_POS_PASSWORD]) {
            delete [] m_posPassword;
            m_posPassword = StringUtility::newStrDup( posPassword );
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_LAT]) {
            setLastPosLat( lastpos_lat );
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_LONG]) {
            setLastPosLong( lastpos_long );
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_INNERRADIUS]) {
            setLastPosInnerRadius( lastpos_innerRadius );
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_OUTERRADIUS]) {
            setLastPosOuterRadius( lastpos_outerRadius );
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_STARTANGLE]) {
            setLastPosStartAngle( lastpos_startAngle );
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_STOPANGLE]) {
            setLastPosStopAngle( lastpos_stopAngle );
         }
         if (m_changed[UserConstants::CELLULAR_LASTPOS_TIME]) {
            setLastPosTime( lastpos_time );
         }
         return ok;
      } else {
         DEBUG1(mc2log << error
                << "UserCellular::readDataChanges failed" << endl;);
         return false;
      }
   } else {
      DEBUG1(mc2log << error
             << "UserCellular::readDataChanges no space for changes "
             << "giving up." << endl;);
      return false;
   }
}


const char* UserCellular::getPhoneNumber() const
{
   return m_phoneNumber;
}


void UserCellular::setPhoneNumber(const char* phoneNumber)
{
   delete [] m_phoneNumber;
   m_phoneNumber = new char[strlen(phoneNumber)+1];
   strcpy(m_phoneNumber, phoneNumber);
   m_changed[UserConstants::CELLULAR_PHONENUMBER] = true;
}


uint8 UserCellular::getSMSLineLength() const
{
   if ( m_smsLineLength == MAX_UINT8 ) {
      if ( m_model != NULL ) {
         return m_model->getChars();
      } else {
         return 10;
      }
   } else {
      return m_smsLineLength;
   }
}


uint8 UserCellular::getCellularSMSLineLength() const
{
   return m_smsLineLength;
}


void 
UserCellular::setSMSLineLength( uint8 smsLineLength ) {
   m_smsLineLength = smsLineLength;
   m_changed[UserConstants::CELLULAR_CHARS_PER_LINE] = true;
}

/*
bool UserCellular::getSmsService() const
{
   return m_smsService;
}


void UserCellular::setSmsService(bool smsService)
{
   m_smsService = smsService;
   m_changed[UserConstants::CELLULAR_SMSSERVICE] = true;
}*/


UserConstants::EOLType
UserCellular::getEOLType() const
{
   if ( m_eol == UserConstants::EOLTYPE_NOT_DEFINED ) {
      if ( m_model != NULL ) {
         return m_model->getEOL();
      } else {
         return UserConstants::EOLTYPE_AlwaysCRLF;
      }
   } else {
      return m_eol;
   }
}


UserConstants::EOLType UserCellular::getCellularEOLType() const
{
   return m_eol;
}


void
UserCellular::setEOLType( UserConstants::EOLType eol ) {
   m_eol = eol;
   m_changed[UserConstants::CELLULAR_EOL_TYPE] = true;
}


uint8 
UserCellular::getSMSParams() const {
   return m_smsParams;
}


void
UserCellular::setSMSParams( uint8 smsParams ) {
   m_smsParams = smsParams;
   m_changed[UserConstants::CELLULAR_SMSPARAMS] = true;
}


uint16 UserCellular::getMaxSearchHitsWap() const
{
   return m_maxSearchHitsWap;
}


void UserCellular::setMaxSearchHitsWap(uint16 maxSearchHitsWap)
{
   m_maxSearchHitsWap = maxSearchHitsWap;
   m_changed[UserConstants::CELLULAR_MAXSEARCHHITSWAP] = true;
}


uint16 UserCellular::getMaxRouteLinesWap() const
{
   return m_maxRouteLinesWap;
}


void UserCellular::setMaxRouteLinesWap(uint16 maxRouteLinesWap)
{
   m_maxRouteLinesWap = maxRouteLinesWap;
   m_changed[UserConstants::CELLULAR_MAXROUTELINESWAP] = true;
}


CellularPhoneModel* 
UserCellular::getModel() {
   return m_model;
}

const CellularPhoneModel* 
UserCellular::getModel() const {
   return m_model;
}


void
UserCellular::setModel( CellularPhoneModel* model ) {
   delete m_model;
   m_model = model;
   delete [] m_modelName;
   m_modelName = StringUtility::newStrDup( model->getName() );
   m_changed[UserConstants::CELLULAR_MODEL] = true;
}


bool
UserCellular::getPosActive() const {
   return m_positioningActive;
}


void
UserCellular::setPosActive( bool positioningActive ) {
   m_positioningActive = positioningActive;
   m_changed[UserConstants::CELLULAR_POSACTIVE] = true;
}


UserConstants::posType
UserCellular::getTypeOfPos() const
{
   return m_typeOfPositioning;
}

void
UserCellular::setTypeOfPos( UserConstants::posType typeOfPositioning )  {
   m_typeOfPositioning = typeOfPositioning;
   m_changed[UserConstants::CELLULAR_TYPE_OF_POS] = true;
}

const char*
UserCellular::getPosUserName() const {
   return m_posUserName;
}

void
UserCellular::setPosUserName( const char* posUserName ) {
   delete [] m_posUserName;
   m_posUserName = StringUtility::newStrDup( posUserName );
   m_changed[UserConstants::CELLULAR_POS_USERNAME] = true;
}

const char*
UserCellular::getPosPassword() const {
   return m_posPassword;
}

void
UserCellular::setPosPassword( const char* posPassword ) {
   delete [] m_posPassword;
   m_posPassword = StringUtility::newStrDup( posPassword );
   m_changed[UserConstants::CELLULAR_POS_PASSWORD] = true;
}

int32
UserCellular::getLastPosLat() const {
   return m_lastpos_lat;
}

void
UserCellular::setLastPosLat( int32 lastpos_lat ) {
   m_lastpos_lat = lastpos_lat;
   m_changed[ UserConstants::CELLULAR_LASTPOS_LAT] = true;
}


int32
UserCellular::getLastPosLong() const {
   return m_lastpos_long;
}

void
UserCellular::setLastPosLong( int32 lastpos_long ) {
   m_lastpos_long = lastpos_long;
   m_changed[ UserConstants::CELLULAR_LASTPOS_LONG] = true;
}

uint32
UserCellular::getLastPosInnerRadius() const {
   return m_lastpos_innerRadius;
}

void
UserCellular::setLastPosInnerRadius( uint32 lastpos_innerRadius ) {
   m_lastpos_innerRadius = lastpos_innerRadius;
   m_changed[ UserConstants::CELLULAR_LASTPOS_INNERRADIUS] = true;
}
   

uint32
UserCellular::getLastPosOuterRadius() const {
   return m_lastpos_outerRadius;
}

void
UserCellular::setLastPosOuterRadius( uint32 lastpos_outerRadius ) {
   m_lastpos_outerRadius = lastpos_outerRadius;
   m_changed[ UserConstants::CELLULAR_LASTPOS_OUTERRADIUS] = true;
}


uint32
UserCellular::getLastPosStartAngle() const {
   return m_lastpos_startAngle;
}

void
UserCellular::setLastPosStartAngle( uint32 lastpos_startAngle ) {
   m_lastpos_startAngle = lastpos_startAngle;
   m_changed[ UserConstants::CELLULAR_LASTPOS_STARTANGLE] = true;
}



uint32
UserCellular::getLastPosStopAngle() const {
   return m_lastpos_stopAngle;
}

void
UserCellular::setLastPosStopAngle( uint32 lastpos_stopAngle ) {
   m_lastpos_stopAngle = lastpos_stopAngle;
   m_changed[ UserConstants::CELLULAR_LASTPOS_STOPANGLE] = true;
}


uint32
UserCellular::getLastPosTime() const {
   return m_lastpos_time;
}

bool 
UserCellular::isPosValid( int time_diff ) const {
   if( m_positioningActive ){
      int time = TimeUtility::getRealTime() - m_lastpos_time;
      if( (time >= 0 ) && ( time < time_diff) )
         return true;
   }
   return false;
}

void
UserCellular::setLastPosTime( uint32 lastpos_time ) {
   m_lastpos_time = lastpos_time;
   m_changed[ UserConstants::CELLULAR_LASTPOS_TIME] = true;
}


uint32
UserCellular::printValue( char* target, 
                          UserConstants::UserCellularField field ) const
{
   switch ( field ) {
      case UserConstants::CELLULAR_PHONENUMBER : {
         strcat( target, "'" );
         sqlString( target + 1, getPhoneNumber() );
         strcat( target, "'" );
      }
      break;
      case UserConstants::CELLULAR_MODEL : {
         strcat( target, "'" );
         sqlString( target + 1, m_modelName );
         strcat( target, "'" );
      }
      break;
      case UserConstants::CELLULAR_SMSPARAMS :
         sprintf( target, "%hd", getSMSParams() );
         break;
      case UserConstants::CELLULAR_MAXSEARCHHITSWAP :
         sprintf( target, "%hd", (int16)getMaxSearchHitsWap() );
         break;
      case UserConstants::CELLULAR_MAXROUTELINESWAP :
         sprintf( target, "%hd", (int16)getMaxRouteLinesWap() );
         break;
         //CELLULAR_SMSSERVICE is a fixed vaule, the real value is in UserUser
      case UserConstants::CELLULAR_SMSSERVICE :
           sprintf( target, "'t'");
          break;
      case UserConstants::CELLULAR_EOL_TYPE :
         sprintf( target, "%d", m_eol );
         break;
      case UserConstants::CELLULAR_CHARS_PER_LINE :
         sprintf( target, "%hd", (int16)m_smsLineLength );
         break;
       case UserConstants::CELLULAR_POSACTIVE :
           sprintf( target, "'%c'", getPosActive() ? 't' : 'f' );
           break;  
      case UserConstants::CELLULAR_TYPE_OF_POS :
         sprintf( target, "%d", getTypeOfPos() );
         break;
      case UserConstants::CELLULAR_POS_USERNAME :
         strcat( target, "'" );
         sqlString( target + 1, getPosUserName() );
         strcat( target, "'" );
         break;
      case UserConstants::CELLULAR_POS_PASSWORD : 
         strcat( target, "'" );
         sqlString( target + 1, getPosPassword() );
         strcat( target, "'" );
         break;
      case UserConstants::CELLULAR_LASTPOS_LAT :
         sprintf( target, "%d", m_lastpos_lat );
         break; 
      case UserConstants::CELLULAR_LASTPOS_LONG :
         sprintf( target, "%d", m_lastpos_long );
         break;
      case UserConstants::CELLULAR_LASTPOS_INNERRADIUS :
         sprintf( target, "%d", m_lastpos_innerRadius );
         break;
      case UserConstants::CELLULAR_LASTPOS_OUTERRADIUS :
         sprintf( target, "%d", m_lastpos_outerRadius );
         break;
      case UserConstants::CELLULAR_LASTPOS_STARTANGLE :
         sprintf( target, "%d", m_lastpos_startAngle );
         break;
      case UserConstants::CELLULAR_LASTPOS_STOPANGLE :
         sprintf( target, "%d", m_lastpos_stopAngle );
         break;
      case UserConstants::CELLULAR_LASTPOS_TIME :
         sprintf( target, "%d", m_lastpos_time );
         break;
      default:
         DEBUG1(mc2log << warn << "UserCellular::printValue unknown " 
                << "fieldType: " << (int)field << endl;);
         break;
   };
   return strlen( target );
}


bool 
UserCellular::isChanged() const {
   return getNbrChanged() > 0 || removed();
}



// === CellularPhoneModels ===============================
CellularPhoneModels::CellularPhoneModels() {
   m_searchModel = new CellularPhoneModel();
   m_manufacturers = new StringVector();
   m_valid = true;
}


CellularPhoneModels::CellularPhoneModels( const Packet* p, int& pos,
                                          uint32 nbrCellularPhoneModels ) {
   CellularPhoneModel* model = NULL;
   bool ok = true;

   for ( uint32 i = 0 ; i < nbrCellularPhoneModels && ok ; i++ ) {
      uint32 type;
      uint16 length;
         
      type = p->incReadLong( pos );
      length = p->incReadShort( pos );
      if ( type == (MAX_UINT8 / 2) && 
           pos - 6 + length < (int32)p->getLength() )
      {
         model = new CellularPhoneModel( p, pos );
         ok = model->getValid();
         if ( ok ) {
            if ( addLastIfUnique( model ) == MAX_UINT32 ) {
               delete model;
            }
         } else {
            delete model;
         }
      } else {
         DEBUG1(mc2log << warn << "CellularPhoneModels::CellularPhoneModels " 
                << "Not enough space for CellularPhoneModel." << endl;);
      }
   }

   sort( m_models.begin(), m_models.end(), STLUtility::RefLess() );

   // Extract manufacturers
   uint32 nbrPhones = size();
   m_manufacturers = new StringVector();
     
   for( uint32 i = 0; i < nbrPhones; i++ ) {
      CellularPhoneModel* cellular;
      cellular = getModel( i );
      uint32 j = 0;
      
      for ( j = 0 ; j < m_manufacturers->getSize() ; j++ ) {
         if ( strcmp( m_manufacturers->getElementAt( j ), 
                      cellular->getManufacturer() ) == 0 ) 
         {
            break;
         }
      }
      if ( j >= m_manufacturers->getSize() ) {
         mc2dbg4 << "Adding manufacturer"<< endl;
         m_manufacturers->addLast( StringUtility::newStrDup(
            cellular->getManufacturer() ) );
      }
   }

   setValid( ok );
 
   m_searchModel = new CellularPhoneModel();
}


CellularPhoneModels::~CellularPhoneModels() {
   STLUtility::deleteValues( m_models );
   delete m_searchModel;
   m_manufacturers->deleteAllObjs();
   delete m_manufacturers;
}

uint32 
CellularPhoneModels::addLastIfUnique ( CellularPhoneModel* model ) {
   CellularPhoneModelVector::iterator it = find_if ( m_models.begin(), 
                                                     m_models.end(), 
                                                     STLUtility::RefEqualCmp<CellularPhoneModel>(*model) );
   if( it != m_models.end() ) {
      return MAX_UINT32;
   } else {
      m_models.push_back( model );
      return m_models.size();
   }
}

CellularPhoneModel*
CellularPhoneModels::findModel( const char* name ) {
   m_searchModel->setName( name );

   CellularPhoneModelVector::iterator it = lower_bound( m_models.begin(), 
                                                        m_models.end(), 
                                                        m_searchModel, 
                                                        STLUtility::RefLess() );

   if ( it != m_models.end() && 
        *(*it) == *m_searchModel ) {
      return *it;
   } else {
      return NULL;
   }
}


CellularPhoneModel* 
CellularPhoneModels::getModel( uint32 index ) {
   return static_cast< CellularPhoneModel* > ( getElementAt( index ) );
}


bool 
CellularPhoneModels::addPhoneModel( CellularPhoneModel* model ) {
   uint32 index;

   index = addLastIfUnique( model );
   if ( index != MAX_UINT32 ) {
      sort( m_models.begin(), m_models.end(), STLUtility::RefLess() );
      return true;
   } else {
      return false;
   }
}


const StringVector&
CellularPhoneModels::getManufacturer() const {
   return *m_manufacturers;
}


uint32
CellularPhoneModels::getNbrManufacturers() const {
   return m_manufacturers->getSize();
}


bool 
CellularPhoneModels::getValid() const {
   return m_valid;
}


void 
CellularPhoneModels::setValid( bool valid ) {
   m_valid = valid;
}


// === CellularPhoneModel ================================
CellularPhoneModel::CellularPhoneModel() {
   init();
   setValid( true );
}


void
CellularPhoneModel::init() {
   m_name = StringUtility::newStrDup( "" );
   m_manufacturer = StringUtility::newStrDup( "" );
   m_chars = 10;
   m_eol = UserConstants::EOLTYPE_AlwaysCRLF;
   m_lines = 1;
   m_dynamicWidth = false;
   m_graphicDisplayWidth = 50;
   m_graphicDisplayHeight = 10;
   m_smsCapable = SMSCAPABLE_YES;
   m_smsConcat = SMSCONCATENATION_NO;
   m_smsGraphic = SMSGRAPHICS_NO;
   m_wapCapable = WAPCAPABLE_NO;
   m_wapVersion = new char[1];
   m_wapVersion[0] = '\0';
   m_modelYear = 0;
   m_comment = new char[1];
   m_comment[0] = '\0';
   for (uint32 i = 0; i < UserConstants::CELLULAR_MODEL_NBRFIELDS; i++) {
      m_changed[ i ] = false;
   }
}


CellularPhoneModel::CellularPhoneModel( const char* name, 
                                        const char* manufacturer,
                                        uint8 charsPerLine,
                                        UserConstants::EOLType eol,
                                        uint8 lines,
                                        bool dynamicWidth,
                                        uint16 graphicDisplayWidth,
                                        uint16 graphicDisplayHeight,
                                        SMSCapableType smsCapable,
                                        SMSConcatenationType smsConcat,
                                        SMSGraphicsType smsGraphic,
                                        WAPCapableType wapCapable,
                                        const char* wapVersion,
                                        int16 modelYear,
                                        const char* comment )
{
   m_name = StringUtility::newStrDup( name );
   m_manufacturer = StringUtility::newStrDup( manufacturer );
   m_chars = charsPerLine;
   m_eol = eol;
   m_lines = lines;
   m_dynamicWidth = dynamicWidth;
   m_graphicDisplayWidth = graphicDisplayWidth;
   m_graphicDisplayHeight = graphicDisplayHeight;
   m_smsCapable = smsCapable;
   m_smsConcat = smsConcat;
   m_smsGraphic = smsGraphic;
   m_wapCapable = wapCapable;
   m_wapVersion = StringUtility::newStrDup( wapVersion );
   m_modelYear = modelYear;
   m_comment = StringUtility::newStrDup( comment );
   setValid( true );

}


CellularPhoneModel::CellularPhoneModel( const Packet* p, int& pos ) {
   mc2dbg4 << "CellularPhoneModel::CellularPhoneModel( " << pos
          << " )" << endl;
   if ( pos + 3 < (int32)p->getLength() ) {

      uint32 type;
      uint16 length;
         
      type = p->incReadLong( pos );
      length = p->incReadShort( pos );
      if ( type == (MAX_UINT8 / 2) && 
           pos - 12 + length < (int32)p->getLength() )
      {
         m_chars = p->incReadByte( pos );
         m_eol = UserConstants::EOLType( p->incReadLong( pos ) );
         m_lines = p->incReadByte( pos );
         m_dynamicWidth = p->incReadByte( pos ) != 0;
         m_graphicDisplayWidth = p->incReadShort( pos );
         m_graphicDisplayHeight = p->incReadShort( pos );
         m_smsCapable = CellularPhoneModel::SMSCapableType( 
            p->incReadLong( pos ) );
         m_smsConcat = CellularPhoneModel::SMSConcatenationType( 
            p->incReadLong( pos ) );
         m_smsGraphic = CellularPhoneModel::SMSGraphicsType(
            p->incReadLong( pos ) );
         m_wapCapable = CellularPhoneModel::WAPCapableType( 
            p->incReadLong( pos ) );
         m_modelYear =  p->incReadShort( pos );
         char* str;
         p->incReadString( pos, str );
         m_name = StringUtility::newStrDup( str );
         p->incReadString( pos, str );
         m_manufacturer = StringUtility::newStrDup( str );
         p->incReadString( pos, str );
         m_wapVersion = StringUtility::newStrDup( str );
         p->incReadString( pos, str );
         m_comment = StringUtility::newStrDup( str );
         setValid( true );
         mc2dbg8 << "CellularPhoneModel::CellularPhoneModel( pack ) "
                 << "ends at " << pos << endl;
      } else {
         DEBUG1(mc2log << warn
                << "CellularPhoneModel::CellularPhoneModel( p, pos ) "
                << "Not type at pos: " << pos << " or not enough space "
                " for data." << endl;);
         init();
         setValid( false );
      }
   } else {
      DEBUG1(mc2log << error
             << "CellularPhoneModel::CellularPhoneModel( p, pos ) "
             << "Not enough bytes left in packet for header!" << endl;);
      init();
      setValid( false );
   }
}


CellularPhoneModel::CellularPhoneModel( const CellularPhoneModel& model ) {
   m_name = StringUtility::newStrDup( model.m_name );
   m_manufacturer = StringUtility::newStrDup( model.m_manufacturer );
   m_chars = model.m_chars;
   m_eol = model.m_eol;
   m_lines = model.m_lines;
   m_dynamicWidth = model.m_dynamicWidth;
   m_graphicDisplayWidth = model.m_graphicDisplayWidth;
   m_graphicDisplayHeight = model.m_graphicDisplayHeight;
   m_smsCapable = model.m_smsCapable;
   m_smsConcat = model.m_smsConcat;
   m_smsGraphic = model.m_smsGraphic;
   m_wapCapable = model.m_wapCapable;
   m_wapVersion = StringUtility::newStrDup( model.m_wapVersion );
   m_modelYear = model.m_modelYear;
   m_comment = StringUtility::newStrDup( model.m_comment );
   m_valid = model.m_valid;
}


CellularPhoneModel::~CellularPhoneModel() {
   delete [] m_name;
   delete [] m_manufacturer;
   delete [] m_wapVersion;
   delete [] m_comment;
}


bool
CellularPhoneModel::packInto( Packet* p, int& pos ) {
   mc2dbg4 << "CellularPhoneModel::packInto( " << pos << " )" << endl;
   uint16 size = 37 + strlen( m_name ) + 1 + strlen( m_manufacturer ) + 1 +
      strlen( m_wapVersion ) + 1 + strlen( m_comment ) + 1;

   if ( pos + size < (int32)p->getBufSize() ) {
      p->incWriteLong( pos, MAX_UINT8 / 2 );
      p->incWriteShort( pos, size );
      p->incWriteLong( pos, MAX_UINT8 / 2 );
      p->incWriteShort( pos, size );
      p->incWriteByte( pos, m_chars );
      p->incWriteLong( pos, m_eol );
      p->incWriteByte( pos, m_lines );
      p->incWriteByte( pos, m_dynamicWidth );
      p->incWriteShort( pos, m_graphicDisplayWidth );
      p->incWriteShort( pos, m_graphicDisplayHeight );
      p->incWriteLong( pos, m_smsCapable );
      p->incWriteLong( pos, m_smsConcat );
      p->incWriteLong( pos, m_smsGraphic );
      p->incWriteLong( pos, m_wapCapable );
      p->incWriteShort( pos, m_modelYear );
      p->incWriteString( pos, m_name );
      p->incWriteString( pos, m_manufacturer );
      p->incWriteString( pos, m_wapVersion );
      p->incWriteString( pos, m_comment );
      mc2dbg8 << "CellularPhoneModel::packInto ends at " 
              << pos << endl;
      return true;
   } else {
      return false;
   }   
}


const char*
CellularPhoneModel::getName() const {
   return m_name;
}


void
CellularPhoneModel::setName( const char* name ) {
   delete [] m_name;
   m_name = StringUtility::newStrDup( name );
   m_changed[UserConstants::CELLULAR_MODEL_NAME] = true;
}


const char* 
CellularPhoneModel::getManufacturer() const {
   return m_manufacturer;
}


void
CellularPhoneModel::setManufacturer( const char* manufacturer ) {
   delete [] m_manufacturer;
   m_manufacturer = StringUtility::newStrDup( manufacturer );
   m_changed[UserConstants::CELLULAR_MODEL_MANUFACTURER] = true;
}


uint8
CellularPhoneModel::getChars() const {
   return m_chars;
}


void
CellularPhoneModel::setChars( uint8 chars ) {
   m_chars = chars;
   m_changed[ UserConstants::CELLULAR_MODEL_CHARS_PER_LINE ] = true;
}


UserConstants::EOLType
CellularPhoneModel::getEOL() const {
   return m_eol;
}


void
CellularPhoneModel::setEOL( UserConstants::EOLType eol ) {
   m_eol = eol;
   m_changed[ UserConstants::CELLULAR_MODEL_EOL_TYPE] = true;
}


uint8
CellularPhoneModel::getLines() const {
   return m_lines;
}


void
CellularPhoneModel::setLines( uint8 lines ) {
   m_lines = lines;
   m_changed[UserConstants::CELLULAR_MODEL_LINES] = true;
}


bool
CellularPhoneModel::getDynamicWidth() const {
   return m_dynamicWidth;
}


void
CellularPhoneModel::setDynamicWidth( bool dynamicWidth ) {
   m_dynamicWidth = dynamicWidth;
   m_changed[UserConstants::CELLULAR_MODEL_DYNAMIC_WIDTH] = true;
}


uint16
CellularPhoneModel::getGraphicsWidth() const {
   return m_graphicDisplayWidth;
}


void
CellularPhoneModel::setGraphicsWidth( uint16 graphicDisplayWidth ) {
   m_graphicDisplayWidth = graphicDisplayWidth;
   m_changed[UserConstants::CELLULAR_MODEL_GRAPHIC_WIDTH] = true;
}


uint16
CellularPhoneModel::getGraphicsHeight() const {
   return m_graphicDisplayHeight;
}


void
CellularPhoneModel::setGraphicsHeight( uint16 graphicDisplayHeight ) {
   m_graphicDisplayHeight = graphicDisplayHeight;
   m_changed[UserConstants::CELLULAR_MODEL_GRAPHIC_HEIGHT] = true;
}


CellularPhoneModel::SMSCapableType
CellularPhoneModel::getSMSCapable() const {
   return m_smsCapable;
}


void
CellularPhoneModel::setSMSCapable( 
   CellularPhoneModel::SMSCapableType smsCapable ) {
   m_smsCapable = smsCapable;
   m_changed[UserConstants::CELLULAR_MODEL_SMS_CAPABLE] = true;
}


CellularPhoneModel::SMSConcatenationType
CellularPhoneModel::getSMSConcatenation() const {
   return m_smsConcat;
}



void
CellularPhoneModel::setSMSConcatenation( 
   CellularPhoneModel::SMSConcatenationType smsConcat ) {
   m_smsConcat = smsConcat;
   m_changed[UserConstants::CELLULAR_MODEL_SMS_CONCATENATE] = true;
}


CellularPhoneModel::SMSGraphicsType
CellularPhoneModel::getSMSGraphic() const {
   return m_smsGraphic;
}


void
CellularPhoneModel::setSMSGraphic( 
   CellularPhoneModel::SMSGraphicsType smsGraphic ) {
   m_smsGraphic = smsGraphic;
   m_changed[UserConstants::CELLULAR_MODEL_SMS_GRAPHIC] = true;
}


CellularPhoneModel::WAPCapableType
CellularPhoneModel::getWAPCapable() const {
   return m_wapCapable;
}


void
CellularPhoneModel::setWAPCapable( 
   CellularPhoneModel::WAPCapableType wapCapable ) {
   m_wapCapable = wapCapable;
   m_changed[UserConstants::CELLULAR_MODEL_WAP_CAPABLE] = true;
}


const char*
CellularPhoneModel::getWAPVersion() const {
   return m_wapVersion;
}


void
CellularPhoneModel::setWAPVersion( const char* wapVersion ) {
   delete [] m_wapVersion;
   m_wapVersion = StringUtility::newStrDup( wapVersion );
   m_changed[UserConstants::CELLULAR_MODEL_WAP_VERSION] = true;
}


int16
CellularPhoneModel::getModelYear() const {
   return m_modelYear;
 }


void
CellularPhoneModel::setModelYear( int16 modelYear ) {
   m_modelYear = modelYear;
   m_changed[UserConstants::CELLULAR_MODEL_MODEL_YEAR] = true;
}


const char* 
CellularPhoneModel::getComment() const {
   return m_comment;
}


void
CellularPhoneModel::setComment( const char* comment ) {
   delete [] m_comment;
   m_comment = StringUtility::newStrDup( comment );
   m_changed[UserConstants::CELLULAR_MODEL_COMMENT] = true;
}


bool
CellularPhoneModel::getValid() const {
   return m_valid;
}


void
CellularPhoneModel::setValid( bool valid ) {
   m_valid = valid;
}

void
CellularPhoneModel::addChanges( Packet* p, int& position) {
  mc2dbg4 << "CellularPhoneModel::addChanges " << endl;
   uint8 nbrChanges = 0;
   uint16 size = 37 + strlen( m_name ) + 1 + strlen( m_manufacturer ) + 1 +
      strlen( m_wapVersion ) + 1 + strlen( m_comment ) + 1;
   for (uint8 i = 0; i < UserConstants::CELLULAR_MODEL_NBRFIELDS; i++)
      if (m_changed[i])
         nbrChanges++;
   if (nbrChanges>0) {
      p->incWriteByte(position, MAX_UINT8 / 2 );
      p->incWriteShort(position, size);
      p->incWriteByte(position, MAX_UINT8 / 2 );
      p->incWriteShort(position, size);  

      p->incWriteByte(position, UserConstants::ACTION_CHANGE);
      p->incWriteByte(position, nbrChanges);

      if (m_changed[UserConstants::CELLULAR_MODEL_NAME]) {
         p->incWriteByte(position, UserConstants::CELLULAR_MODEL_NAME);
         p->incWriteString(position, m_name);
         }
      if (m_changed[UserConstants::CELLULAR_MODEL_MANUFACTURER]) {
         p->incWriteByte(position, UserConstants::CELLULAR_MODEL_MANUFACTURER);
         p->incWriteString(position, m_manufacturer);
      }
      if ( m_changed[ UserConstants::CELLULAR_MODEL_CHARS_PER_LINE ] ) {
         p->incWriteByte(position,
                         UserConstants::CELLULAR_MODEL_CHARS_PER_LINE);
         p->incWriteByte(position, m_chars);
      }
      if (m_changed[UserConstants::CELLULAR_MODEL_EOL_TYPE]) {
         p->incWriteByte(position, 
                         UserConstants::CELLULAR_MODEL_EOL_TYPE);
         p->incWriteLong(position, m_eol);
      }
      if (m_changed[UserConstants::CELLULAR_MODEL_LINES]) {
         p->incWriteByte(position, 
                         UserConstants::CELLULAR_MODEL_LINES);
         p->incWriteByte(position, m_lines);
      }
      if (m_changed[UserConstants::CELLULAR_MODEL_DYNAMIC_WIDTH]) {
         p->incWriteByte(position,
                         UserConstants::CELLULAR_MODEL_DYNAMIC_WIDTH);
         p->incWriteByte(position, m_dynamicWidth);
      }
      if (m_changed[UserConstants::CELLULAR_MODEL_GRAPHIC_WIDTH]) {
         p->incWriteByte(position,
                         UserConstants::CELLULAR_MODEL_GRAPHIC_WIDTH);
         p->incWriteShort(position, m_graphicDisplayWidth);
      }
      if (m_changed[UserConstants::CELLULAR_MODEL_GRAPHIC_HEIGHT]) {
         p->incWriteByte(position, 
                         UserConstants::CELLULAR_MODEL_GRAPHIC_HEIGHT);
         p->incWriteShort(position, m_graphicDisplayHeight);
      }
      if (m_changed[UserConstants::CELLULAR_MODEL_SMS_CAPABLE]) {
         p->incWriteByte(position, 
                         UserConstants::CELLULAR_MODEL_SMS_CAPABLE);
         p->incWriteLong(position, m_smsCapable);
      }
      if (m_changed[UserConstants::CELLULAR_MODEL_SMS_CONCATENATE]) {
         p->incWriteByte(position, 
                         UserConstants::CELLULAR_MODEL_SMS_CONCATENATE);
         p->incWriteLong(position, m_smsConcat);
      }
      if (m_changed[UserConstants::CELLULAR_MODEL_SMS_GRAPHIC]) {
         p->incWriteByte(position, 
                         UserConstants::CELLULAR_MODEL_SMS_GRAPHIC);
         p->incWriteLong(position, m_smsGraphic);
      }
      if (m_changed[UserConstants::CELLULAR_MODEL_WAP_CAPABLE]) {
         p->incWriteByte(position, 
                         UserConstants::CELLULAR_MODEL_WAP_CAPABLE);
         p->incWriteLong(position, m_wapCapable);
      }
      if (m_changed[UserConstants::CELLULAR_MODEL_WAP_VERSION]) {
         p->incWriteByte(position, 
                         UserConstants::CELLULAR_MODEL_WAP_VERSION);
         p->incWriteString(position, m_wapVersion );
      }
      if (m_changed[UserConstants::CELLULAR_MODEL_MODEL_YEAR]) {
         p->incWriteByte(position, 
                         UserConstants::CELLULAR_MODEL_MODEL_YEAR);
         p->incWriteShort(position, m_modelYear);
      }
      if (m_changed[UserConstants::CELLULAR_MODEL_COMMENT]) {
         p->incWriteByte(position, 
                         UserConstants::CELLULAR_MODEL_COMMENT);
         p->incWriteString(position, m_comment);
      }
   }
   p->incWriteByte(position, MAX_UINT8 / 2);
   p->incWriteByte(position, MAX_UINT8 / 2);
   for (uint8 j = 0; j < UserConstants::CELLULAR_MODEL_NBRFIELDS; j++)
      m_changed[j] = false;
   p->setLength( position );
}

void 
CellularPhoneModel::readChanges( const Packet* p, int& pos ) {

   mc2dbg8 << "Entered readChanges" << endl;


   if ( p->getLength() - pos - 1 > 0 ) {
      byte nbrFields;
      uint16 size;
      uint32 type;
      byte action;
      
      type = p->incReadLong( pos );
      size = p->incReadShort( pos );

      action = p->incReadByte( pos );
      nbrFields = p->incReadByte( pos );
      
      // tmpData
      char* name = m_name;
      char* manufacturer = m_manufacturer;
      uint8 charsPerLine = m_chars;
      UserConstants::EOLType eol = m_eol;
      uint8 lines = m_lines;
      bool dynamicWidth = m_dynamicWidth;
      uint16 graphicDisplayWidth = m_graphicDisplayWidth;
      uint16 graphicDisplayHeight = m_graphicDisplayHeight;
      SMSCapableType smsCapable = m_smsCapable;
      SMSConcatenationType smsConcat = m_smsConcat;
      SMSGraphicsType smsGraphic = m_smsGraphic;
      WAPCapableType wapCapable = m_wapCapable;
      char* wapVersion = m_wapVersion;
      int16 modelYear = m_modelYear;
      char* comment = m_comment;
      

      byte field;
      bool ok = true;
      for ( uint8 i = 0 ; 
            (i < nbrFields) && ok && (pos < (int32)p->getLength()) ;
            i ++ ) 
      {
         field = p->incReadByte( pos );
         switch ( field ) {
            case UserConstants::CELLULAR_MODEL_NAME :
               m_changed[ UserConstants::CELLULAR_MODEL_NAME ] = true;
               p->incReadString( pos, name );
               break;
            case UserConstants::CELLULAR_MODEL_MANUFACTURER :
               m_changed[ UserConstants::CELLULAR_MODEL_MANUFACTURER ] = true;
               p->incReadString( pos, manufacturer );
               break;   
            case UserConstants::CELLULAR_MODEL_CHARS_PER_LINE :
               m_changed[ UserConstants::CELLULAR_MODEL_CHARS_PER_LINE ]
                  = true;
               charsPerLine = p->incReadByte( pos );
               break;
            case UserConstants::CELLULAR_MODEL_EOL_TYPE :
               m_changed[ UserConstants::CELLULAR_MODEL_EOL_TYPE ] = true;
               eol = UserConstants::EOLType( p->incReadLong( pos ) );
               break;
            case UserConstants::CELLULAR_MODEL_LINES :
               m_changed[ UserConstants::CELLULAR_MODEL_LINES ] = true;
               lines = p->incReadByte( pos );
               break;
            case UserConstants::CELLULAR_MODEL_DYNAMIC_WIDTH :
               m_changed[ UserConstants::CELLULAR_MODEL_DYNAMIC_WIDTH ]
                  = true;
               dynamicWidth = p->incReadByte( pos ) != 0;
               break;
            case UserConstants::CELLULAR_MODEL_GRAPHIC_WIDTH :
               m_changed[ UserConstants::CELLULAR_MODEL_GRAPHIC_WIDTH ]
                  = true;
               graphicDisplayWidth = p->incReadShort( pos );
               break;
            case UserConstants::CELLULAR_MODEL_GRAPHIC_HEIGHT :
               m_changed[ UserConstants::CELLULAR_MODEL_GRAPHIC_HEIGHT ]
                  = true;
               graphicDisplayHeight = p->incReadShort( pos );
               break;
            case UserConstants::CELLULAR_MODEL_SMS_CAPABLE :
               m_changed[ UserConstants::CELLULAR_MODEL_SMS_CAPABLE ]
                  = true;
               smsCapable =
                  CellularPhoneModel::SMSCapableType( p->incReadLong( pos ) );
               break;
            case UserConstants::CELLULAR_MODEL_SMS_CONCATENATE :
               m_changed[ UserConstants::CELLULAR_MODEL_SMS_CONCATENATE ]
                  = true;
               smsConcat =
                  CellularPhoneModel::SMSConcatenationType(
                     p->incReadLong( pos ) );
               break;
            case UserConstants::CELLULAR_MODEL_SMS_GRAPHIC :
               m_changed[ UserConstants::CELLULAR_MODEL_SMS_GRAPHIC ]
                  = true;
               smsGraphic =
                  CellularPhoneModel::SMSGraphicsType( p->incReadLong( pos ) );
               break;
            case UserConstants::CELLULAR_MODEL_WAP_CAPABLE :
               m_changed[ UserConstants::CELLULAR_MODEL_WAP_CAPABLE ]
                  = true;
               wapCapable =
                  CellularPhoneModel::WAPCapableType( p->incReadLong( pos ) );
               break;
            case UserConstants::CELLULAR_MODEL_WAP_VERSION :
               m_changed[ UserConstants::CELLULAR_MODEL_WAP_VERSION ]
                  = true;
               p->incReadString( pos,wapVersion );
               break;
            case UserConstants::CELLULAR_MODEL_MODEL_YEAR :
               m_changed[ UserConstants::CELLULAR_MODEL_MODEL_YEAR ]
                  = true;
               modelYear = p->incReadShort( pos );
               break;
            case UserConstants::CELLULAR_MODEL_COMMENT :
               m_changed[ UserConstants::CELLULAR_MODEL_COMMENT ]
                  = true;
               p->incReadString( pos, comment );
               break;
               
            default:
               DEBUG1(mc2log << warn
                      << "UserCellular::readDataChanges unknown " 
                      << "fieldType: " << (int)field << endl;);
               ok = false;
               break;
         };
      }
      type = p->incReadLong( pos );
      if ( type != ( MAX_UINT8 / 2 ) ) {
         DEBUG1(mc2log << error
                << "UserCellular::readDataChanges Not Cellular "
                "type after data" << endl;);
         ok = false;
      }
      if ( ok ) {
         // Transfer changes to membervariables
         if (m_changed[UserConstants::CELLULAR_MODEL_NAME]) {
            setName( name );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_MANUFACTURER]) {
            setManufacturer( manufacturer );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_CHARS_PER_LINE]) {
            setChars( charsPerLine );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_EOL_TYPE]) {
            setEOL( eol );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_LINES]) {
            setLines( lines );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_DYNAMIC_WIDTH]) {
            setDynamicWidth( dynamicWidth );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_GRAPHIC_WIDTH]) {
            setGraphicsWidth( graphicDisplayWidth );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_GRAPHIC_HEIGHT]) {
           setGraphicsHeight( graphicDisplayHeight );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_SMS_CAPABLE]) {
           setSMSCapable( smsCapable );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_SMS_CONCATENATE]) {
           setSMSConcatenation( smsConcat );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_SMS_GRAPHIC]) {
            setSMSGraphic( smsGraphic );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_WAP_CAPABLE]) {
            setWAPCapable( wapCapable );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_WAP_VERSION]) {
            setWAPVersion( wapVersion );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_MODEL_YEAR]) {
            setModelYear( modelYear );
         }
         if (m_changed[UserConstants::CELLULAR_MODEL_COMMENT]) {
            setComment( comment );
         }
         setValid( ok );
      } else {
         DEBUG1(mc2log << error
                << "CellularPhoneModel::readDataChanges failed" << endl;);
         setValid( false );
      }
   } else {
      DEBUG1(mc2log << error
             << "CellularPhoneModel::readDataChanges no space for changes "
             << "giving up." << endl;);
      setValid( false );
   }
}


uint32 
CellularPhoneModel::getNbrChanged() const {
   uint32 nbr = 0;
   for ( uint32 i = 0 ; i < UserConstants::CELLULAR_MODEL_NBRFIELDS ; i++ ) {
      if ( changed( UserConstants::CellularModelField( i ) ) ) {
         nbr++;
      }
   }
   return nbr;
}


uint32 
CellularPhoneModel::printValue( 
   char* target, UserConstants::CellularModelField field ) const
{
   switch ( field ) {
      case UserConstants::CELLULAR_MODEL_NAME:
         strcat( target, "'" );
         UserElement::sqlString( target + 1, getName() );
         strcat( target, "'" );
         break;
      case UserConstants:: CELLULAR_MODEL_MANUFACTURER:
         strcat( target, "'" );
         UserElement::sqlString( target + 1, getManufacturer() );
         strcat( target, "'" );
         break;
      case UserConstants::CELLULAR_MODEL_CHARS_PER_LINE:
         sprintf( target, "%hd", getChars() );
         break;
      case UserConstants::CELLULAR_MODEL_EOL_TYPE:
         sprintf( target, "%d", getEOL() );
         break;
      case UserConstants::CELLULAR_MODEL_LINES:
         sprintf( target, "%hd", getLines() );
         break;
      case UserConstants::CELLULAR_MODEL_DYNAMIC_WIDTH:
         sprintf( target, "'%c'", getDynamicWidth() ? 't' : 'f' );
         break;
      case UserConstants::CELLULAR_MODEL_GRAPHIC_WIDTH:
         sprintf( target, "%hd", getGraphicsWidth() );
         break;
      case UserConstants::CELLULAR_MODEL_GRAPHIC_HEIGHT:
         sprintf( target, "%hd", getGraphicsHeight() );
         break;
      case UserConstants::CELLULAR_MODEL_SMS_CAPABLE:
         sprintf( target, "%d", getSMSCapable() );
         break;
      case UserConstants::CELLULAR_MODEL_SMS_CONCATENATE:
         sprintf( target, "%d", getSMSConcatenation() );
         break;
      case UserConstants::CELLULAR_MODEL_SMS_GRAPHIC:
         sprintf( target, "%d", getSMSGraphic() );
         break;
      case UserConstants::CELLULAR_MODEL_WAP_CAPABLE:
         sprintf( target, "%d", getWAPCapable() );
         break;
      case  UserConstants::CELLULAR_MODEL_WAP_VERSION:
         strcat( target, "'" );
         UserElement::sqlString( target + 1, getWAPVersion() );
         strcat( target, "'" );
         break;
      case UserConstants::CELLULAR_MODEL_MODEL_YEAR:
         sprintf( target, "%hd", getModelYear() );
         break;
      case UserConstants::CELLULAR_MODEL_COMMENT:
         strcat( target, "'" );
         UserElement::sqlString( target + 1, getComment() );
         strcat( target, "'" );
         break;
      default:
         DEBUG1(mc2log << warn << "CellularPhoneModel::printValue unknown " 
                << "fieldType: " << (int)field << endl;);
         break;
   };

   return strlen( target );
}


// **********************************************************************
// UserBuddyList
// **********************************************************************

const uint32 UserBuddyList::MAX_NBR_BUDDIES = 10;


UserBuddyList::UserBuddyList( uint32 ID ) 
   : UserElement( UserConstants::TYPE_BUDDY ),
   m_buddies( MAX_NBR_BUDDIES )
{
   m_id = ID;
   m_name = StringUtility::newStrDup( "" );

   uint32 i = 0;
   for ( i = 0; i < UserConstants::BUDDY_NBRFIELDS; i++ ) {
      m_changed[ i ] = false;
   }
}


UserBuddyList::UserBuddyList( const UserReplyPacket* p, int& pos ) 
   : UserElement( UserConstants::TYPE_BUDDY ),
   m_buddies( MAX_NBR_BUDDIES ) 
{
   m_name = NULL;
   int32 startPos = pos;
   uint32 type = p->incReadLong( pos );

   if ( type == UserConstants::TYPE_BUDDY ) {
      uint32 size = p->incReadShort( pos );
      if ( (12+38 < size) && ( (startPos - 3 + size) < p->getLength()) ) {
         // 12 is the two start TYPE_BUDDY and size
         // 38 is the data read here ,about
         m_id = p->incReadLong( pos );
         uint32 i = 0;
         for ( i = 0; i < MAX_NBR_BUDDIES ; i++ ) {
            m_buddies.setElementAt( i, p->incReadLong( pos ) );
         }
         // Remove unused buddies from total size.
         for ( i = MAX_NBR_BUDDIES ; 
               i > 0 && m_buddies[ i - 1] == 0 ; 
               i-- ) {
            m_buddies.removeElementAt( i - 1 );
         }
         

         char* str;
         uint32 strLen;
         uint32 strSize;
         strSize = p->incReadShort( pos );
         strLen = p->incReadString( pos, str );
         if ( strSize != strLen ) {
            DEBUG1(mc2log << error << "UserCellular::UserCellular( p, pos ) "
                   << "Strlen error" << endl;);
            return; 
         }
         m_name = new char[strLen + 1];
         if ( (strLen + 1) <= (p->getLength() - pos) ) { 
            strcpy( m_name, str );

            type = p->incReadLong( pos );
            if ( type != UserConstants::TYPE_BUDDY ) {
               DEBUG1(mc2log << error
                      << "UserBUddyList::UserBuddyList( p, pos ) "
                      << "Data not ended by TYPE_BUDDY!!!" << endl;);
               return;
            }
            // DONE
         } else {
            m_name[0] = '\0';
            DEBUG1(mc2log << error << "UserBuddyList::UserBuddyList( p, pos ) "
                   << "Not space for name!!" << endl;);
            return;
         }
      } else {
         DEBUG1(mc2log << error  
                << "UserBuddyList::UserBuddyList( p, pos ) "
                << "Not size for data!! Nothing read" << endl;);
         return;
      }
   } else {
      DEBUG1(mc2log << error   
              << "UserBuddyList::UserBuddyList( p, pos ) "
              << "Not TYPE_BUDDY type!!!!!" << endl;);
      return;
   }

   for ( uint32 i = 0; i < UserConstants::BUDDY_NBRFIELDS; i++) {
      m_changed[ i ] = false;
   }

   setOk( true );
}


UserBuddyList::UserBuddyList( const UserBuddyList& orig ) 
      : UserElement( UserConstants::TYPE_BUDDY ),
        m_buddies( MAX_NBR_BUDDIES )
{
   uint32 i = 0;
   for ( i = 0 ; i < orig.getNbrBuddies() ; i++ ) {
      m_buddies.addLast( orig.getBuddy( i ) );
   }
   for ( i = 0; i < UserConstants::BUDDY_NBRFIELDS; i++ ) {
      m_changed[ i ] = orig.m_changed[ i ];
   }
   m_id = orig.m_id;
   m_name = StringUtility::newStrDup( orig.m_name );
}


UserBuddyList::~UserBuddyList() {
   delete [] m_name;
}


uint32
UserBuddyList::getSize() const {
   return (MAX_NBR_BUDDIES + 1)*sizeof(uint32) + strlen( m_name ) + 1;
}


void
UserBuddyList::packInto( Packet* p, int& pos ) {
   mc2dbg4 << "UserBuddyList::packInto " << endl;
   uint16 nameLen = strlen( m_name );

   uint16 size = getSize();

   p->incWriteLong( pos, UserConstants::TYPE_BUDDY );
   p->incWriteShort( pos, size );
   p->incWriteLong( pos, UserConstants::TYPE_BUDDY );
   p->incWriteShort( pos, size );

   p->incWriteLong( pos, m_id );
   
   for ( uint32 i = 0; i < MAX_NBR_BUDDIES ; i++ ) {
      if ( i < m_buddies.getSize() ) {
         p->incWriteLong( pos, m_buddies[ i ] );
      } else {
         p->incWriteLong( pos, 0 ); 
      }
   }
 
   p->incWriteShort( pos, nameLen );
   p->incWriteString( pos, m_name );

   p->incWriteLong( pos, UserConstants::TYPE_BUDDY );
   p->incWriteLong( pos, UserConstants::TYPE_BUDDY );

   // Set length 
   p->setLength( pos );   
}


void
UserBuddyList::addChanges( Packet* p, int& pos ) {
   mc2dbg4 << "UserBuddyList::addChanges " << endl;
   uint8 nbrChanges = 0;

   for ( uint32 i = 0; i < UserConstants::BUDDY_NBRFIELDS; i++ ) {
      if ( m_changed[ i ] ) {
         nbrChanges++;
      }
   }

   if ( nbrChanges > 0 || removed() ) {
      p->incWriteLong( pos, UserConstants::TYPE_BUDDY );
      p->incWriteLong( pos, m_id );
      p->incWriteLong( pos, UserConstants::TYPE_BUDDY );
      p->incWriteLong( pos, m_id );
      if ( removed() ) { // Removed
         p->incWriteByte( pos, UserConstants::ACTION_DELETE );
      } else if ( m_id == 0) { // New
         p->incWriteByte( pos, UserConstants::ACTION_NEW) ;
         // Nbr Changed Not USERUIN and ID
         p->incWriteByte( pos, UserConstants::BUDDY_NBRFIELDS - 2 );
         p->incWriteByte( pos, UserConstants::BUDDY_NAME );
         p->incWriteString( pos, m_name );
         p->incWriteByte( pos, UserConstants::BUDDY_BUDDIES );
         for ( uint32 i = 0 ; i < MAX_NBR_BUDDIES ; i++ ) {
            p->incWriteLong( pos, m_buddies[ i ] );
         }
      } else { // Changed
         p->incWriteByte( pos, UserConstants::ACTION_CHANGE );
         p->incWriteByte( pos, nbrChanges );
         if ( m_changed[ UserConstants::BUDDY_NAME ]) {
            p->incWriteByte( pos, UserConstants::BUDDY_NAME);
            p->incWriteString( pos, m_name );
         }
         if ( m_changed[ UserConstants::BUDDY_BUDDIES ]) {
            p->incWriteByte( pos, UserConstants::BUDDY_BUDDIES );
            for ( uint32 i = 0 ; i < MAX_NBR_BUDDIES ; i++ ) {
               p->incWriteLong( pos, m_buddies[ i ] );
            }
         }
      }
      p->incWriteLong( pos, UserConstants::TYPE_BUDDY );
      p->incWriteLong( pos, UserConstants::TYPE_BUDDY );
      for (uint8 i = 0; i < UserConstants::BUDDY_NBRFIELDS; i++)
         m_changed[ i ] = false;
      p->setLength( pos );
   }
}


bool 
UserBuddyList::changed( UserConstants::UserBuddyListField field ) const {
   return m_changed[ field ];
}


bool
UserBuddyList::readChanges( const Packet* p, int& pos, 
                            UserConstants::UserAction& action ) 
{
   mc2dbg4 << "UserBuddy::readChanges pos " << pos << endl;

   if ( p->getLength() - pos - 5 > 0 ) {
      uint32 type;
      uint32 id;
      
      type = p->incReadLong( pos );
      id = p->incReadLong( pos );
      if ( type == UserConstants::TYPE_BUDDY && id == m_id ) {
         action = UserConstants::UserAction(p->incReadByte( pos ) );
         switch ( action ) {
            case UserConstants::ACTION_NEW:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_DELETE:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_BUDDY ) {
                  remove();
                  return true;
               } else {
                  DEBUG1(mc2log << error
                         << "UserBuddyList::readChanges not Type "
                         << " BUDDY after action delete" << endl;);
                  return false;
               }
               break;
            case UserConstants::ACTION_CHANGE:
               return ( readDataChanges( p, pos ) );
               break;
            case UserConstants::ACTION_NOP:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_BUDDY ) {
                  return true;
               } else {
                  DEBUG1(mc2log << error
                         << "UserBuddyList::readChanges not Type "
                         << " BUDDY after action NOP" << endl;);
                  return false;
               }
               break;
            default:
               DEBUG1(mc2log << error
                      << "UserBuddyList::readChanges unknown action: "
                      << (int)action << endl;);
         };
         return false;
      } else {
         DEBUG1(mc2log << error
                << "UserBuddyList::readChanges not correct type or "
                << " not correct id, giving up." << endl;);
         return false;  
      }
   } else {
      DEBUG1(mc2log << error
             << "UserBuddyLisy::readChanges no space for changes "
             << "giving up." << endl;);
      return false;
   }
}


uint32 
UserBuddyList::printValue( char* target, 
                           UserConstants::UserBuddyListField field ) const
{
   switch ( field ) {
      case UserConstants::BUDDY_ID :
         sprintf( target, "%d", getID() );
         break;
      case UserConstants::BUDDY_USERUIN :
         sprintf( target, "%u", getUserUIN() );
         break;
      case UserConstants::BUDDY_NAME :
         strcat( target, "'" );
         sqlString( target + 1, getName() );
         strcat( target, "'" );
         break;
      case UserConstants::BUDDY_BUDDIES :
         strcpy( target, "b%d" );
         break;
      default:
         DEBUG1(mc2log << error << "UserBuddyList::printValue unknown " 
                << "fieldType: " << (int)field << endl;);
         break;
   }

   return strlen( target );
}


bool 
UserBuddyList::isChanged() const {
   return getNbrChanged() > 0 || removed();
}


bool 
UserBuddyList::readDataChanges( const Packet* p, int& pos ) {

   if ( p->getLength() - pos - 1 > 0 ) {
      byte nbrFields;

      nbrFields = p->incReadByte( pos );
      
      // tmpData
      char* name = m_name;
      uint32 nameSize;
      uint32 buddies[ MAX_NBR_BUDDIES ];

      byte field;
      bool ok = true;
      for ( uint8 i = 0 ; 
            (i < nbrFields) && ok && (pos < (int32)p->getLength()) ;
            i ++ ) 
      {
         field = p->incReadByte( pos );
         switch ( field ) {
            case UserConstants::BUDDY_NAME :
               m_changed[ UserConstants::BUDDY_NAME ] = true;
               nameSize = p->incReadString( pos, name );
               break;
            case UserConstants::BUDDY_BUDDIES :{
               m_changed[ UserConstants::BUDDY_BUDDIES ] = true;
               for ( uint32 i = 0; i < MAX_NBR_BUDDIES ; i++ ) {
                  buddies[ i ] = p->incReadLong( pos );
               }
               break;   
            }
            default:
               DEBUG1(mc2log << error
                      << "UserBuddyList::readDataChanges unknown " 
                      << "fieldType: " << (int)field << endl;);
               ok = false;
               break;
         };
      }
      uint32 type = p->incReadLong( pos );
      if ( type != UserConstants::TYPE_BUDDY ) {
         DEBUG1(mc2log << error << "UserBuddyList::readDataChanges Not BUDDY "
                <<" type after data" << endl;);
         ok = false;
      }
      if ( ok ) {
         // Transfer changes to membervariables
         if ( m_changed[ UserConstants::BUDDY_NAME ] ) {
            setName( name );
         }
         if ( m_changed[ UserConstants::BUDDY_BUDDIES ] ) {
            for ( uint32 i = 0; i < MAX_NBR_BUDDIES ; i++ ) {
               m_buddies[ i ] = buddies[ i ];
            } 
         }
         return ok;
      } else {
         DEBUG1(mc2log << error << "UserBuddyList::readDataChanges failed"
                << endl;);
         return false;
      }
   } else {
      DEBUG1(mc2log << error
             << "UserBuddyList::readDataChanges no space for changes "
             << "giving up." << endl;);
      return false;
   }
}


void 
UserBuddyList::setID( uint32 ID ) {
   m_id = ID;
   m_changed[ UserConstants::BUDDY_ID ] = true;
}


uint32 
UserBuddyList::getUserUIN() const {
   return m_userUIN;
}


void 
UserBuddyList::setUserUIN( uint32 UIN ) {
   m_userUIN = UIN;
   m_changed[ UserConstants::BUDDY_USERUIN ] = true;
}


const char*
UserBuddyList::getName() const {
   return m_name;
}


void
UserBuddyList::setName( const char* name ) {
   delete [] m_name;
   m_name = StringUtility::newStrDup( name );
   m_changed[ UserConstants::BUDDY_NAME ] = true;
}


uint32 
UserBuddyList::getNbrBuddies() const {
   return m_buddies.getSize();
}


uint32
UserBuddyList::getBuddy( uint32 index ) const {
   if ( index < m_buddies.getSize() ) {
      return m_buddies.getElementAt( index );
   } else {
      return 0;
   }
}


bool
UserBuddyList::addBuddy( uint32 UIN ) {
   if ( m_buddies.getSize() < MAX_NBR_BUDDIES ) {
      bool ok = m_buddies.addLastIfUnique( UIN ) != MAX_UINT32;
      if ( ok ) {
         m_changed[ UserConstants::BUDDY_BUDDIES ] = true;
      }
      return ok;
   } else {
      return false;
   }
}


bool
UserBuddyList::setBuddyAt( uint32 index, uint32 UIN ) {
   if ( index < MAX_NBR_BUDDIES ) {
      m_buddies.setElementAt( index, UIN );
      m_changed[ UserConstants::BUDDY_BUDDIES ] = true;
      return true;
   } else {
      return false;
   } 
}


bool
UserBuddyList::removeBuddyAt( uint32 index ) {
   if ( index < m_buddies.getSize() ) {
      m_buddies.setElementAt( index, 0 ); // 0 == Not used
      m_changed[ UserConstants::BUDDY_BUDDIES ] = true;
      return true;
   } else {
      return false;
   }
}


bool
UserBuddyList::removeBuddyWithUIN( uint32 UIN ) {
   uint32 index = m_buddies.linearSearch( UIN );
   if ( index < m_buddies.getSize() ) {
      m_buddies.setElementAt( index, 0 ); // 0 == Not used
      m_changed[ UserConstants::BUDDY_BUDDIES ] = true;
      return true;
   } else {
      return false;
   }  
}


uint32
UserBuddyList::getNbrChanged() const {
   uint32 nbr = 0;
   for ( uint32 i = 0 ; i < UserConstants::BUDDY_NBRFIELDS ; i++ ) {
      if ( changed( UserConstants::UserBuddyListField( i ) ) ) {
         nbr++;
      }
   }
   return nbr;
}


//********************************************************************** 
// DBUserNavigator
//**********************************************************************

const uint32 DBUserNavigator::m_nbrFields = 9;

const char* DBUserNavigator::m_fieldNames[ m_nbrFields ] = { 
   "id",
   "userUIN",
   "navigatorType",
   "address",
   "unsentDataCount",
   "lastContactDate",
   "lastContactSuccess",
   "lastContactLat",
   "lastContactLon" };

const DBaseElement::element_t DBUserNavigator::m_fieldTypes[ m_nbrFields ]
= {
   DBaseElement::DB_INT_NOTNULL,           // id
   DBaseElement::DB_INT_NOTNULL,           // userUIN
   DBaseElement::DB_INT_NOTNULL,           // navigatorType
   DBaseElement::DB_LARGESTRING_NOTNULL,   // address
   DBaseElement::DB_SMALLINT,              // unsent count
   DBaseElement::DB_INT,                   // last contact date
   DBaseElement::DB_BOOL,                  // last contact success
   DBaseElement::DB_INT,                   // last contact latitude   
   DBaseElement::DB_INT                    // last contact longitude   
};

const char* DBUserNavigator::m_fieldDescriptionStrs[ m_nbrFields ] = {
   "id",
   "userUIN",
   "navigatorType",
   "address",
   "unsentDataCount",
   "lastContactDate",
   "lastContactSuccess",
   "lastContactLat",
   "lastContactLon"
};

const char* DBUserNavigator::m_tableStr = "ISABUserNavigator";

const StringTable::stringCode DBUserNavigator::m_fieldDescriptions[ 
   m_nbrFields ] = {
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING };


DBUserNavigator::DBUserNavigator() 
      : DBaseObject( m_nbrFields )
{
   // Always call init from subclass
   init( m_nbrFields );
}


DBUserNavigator::DBUserNavigator( const DBUserNavigator& copy )
      : DBaseObject( copy )
{
   // Do not call init from copy constructor
}


DBUserNavigator::DBUserNavigator( SQLQuery* pQuery )
      : DBaseObject( m_nbrFields )
{
   // Always call init from subclass
   init( m_nbrFields, pQuery);
}


DBUserNavigator::DBUserNavigator( const Packet* pPacket, int& pos )
   : DBaseObject( m_nbrFields )
{
   // Always call init from subclass
   init( m_nbrFields, pPacket, pos );
}


DBUserNavigator::~DBUserNavigator() {
}


const char*
DBUserNavigator::getTableStr() const {
   return m_tableStr;
}


const char* 
DBUserNavigator::getTableName() {
   return m_tableStr; 
}


const char** 
DBUserNavigator::getFieldDescriptions() const {
   return m_fieldDescriptionStrs;
}


const StringTable::stringCode* 
DBUserNavigator::getFieldStringCode() const {
   return m_fieldDescriptions;
}


const char** 
DBUserNavigator::getFieldNames() const {
   return m_fieldNames;
}


const DBaseElement::element_t* 
DBUserNavigator::getFieldTypes() const {
   return m_fieldTypes;
}


uint32
DBUserNavigator::getNbrChanged() const {
   DBaseElement* pDBElement = NULL;
   uint32 nbrChanges = 0;
   int nbrFields = m_dBaseElm.size();

   for( int i = 0; i < nbrFields; i++ ) {
      pDBElement = static_cast<DBaseElement*>(m_dBaseElm[i]);
      if( pDBElement->changed() ) {
         nbrChanges++;
      }
   }
   
   return nbrChanges;
}


uint32
DBUserNavigator::getID() const {
   return getInt( field_ID );
}


void
DBUserNavigator::setID( uint32 ID ) {
   setInt( field_ID, ID );
}


uint32
DBUserNavigator::getUserUIN() const {
   return getInt( field_userUIN );
}


void
DBUserNavigator::setUserUIN( uint32 UIN ) {
   setInt( field_userUIN, UIN );
}


UserConstants::navigatorType
DBUserNavigator::getNavigatorType() const {
   return UserConstants::navigatorType( getInt( field_navType ) );
}


void
DBUserNavigator::setNavigatorType( UserConstants::navigatorType navType ) {
   setInt( field_navType, navType );
}


const char* 
DBUserNavigator::getAddress() const {
   return getString( field_address );
}


void
DBUserNavigator::setAddress( const char* address ) {
   setString( field_address, address );
}


uint16 
DBUserNavigator::getUnsentCount() const {
   return getSmallInt( field_unsentDataCount );
}


void
DBUserNavigator::setUnsentCount( uint16 count ) {
   setSmallInt( field_unsentDataCount, count );
}


uint32
DBUserNavigator::getLastContactDate() const {
   return getInt( field_lastContactDate );
}


void
DBUserNavigator::setLastContactDate( uint32 date ) {
   setInt( field_lastContactDate, date );
}


bool
DBUserNavigator::getLastContactSuccess() const {
   return getBool( field_lastContactSuccess );
}


void
DBUserNavigator::setLastContactSuccess( bool success ) {
   setBool( field_lastContactSuccess, success );
}


uint32 
DBUserNavigator::getLastContactLat() const {
   return getBool( field_lastContactLatitude );
}


void 
DBUserNavigator::setLastContactLat( uint32 lat ) {
   setInt( field_lastContactLatitude, lat );
}


uint32 
DBUserNavigator::getLastContactLon() const {
   return getBool( field_lastContactLongitude );
}


void 
DBUserNavigator::setLastContactLon( uint32 lon ) {
   setInt( field_lastContactLongitude, lon );
}


//**********************************************************************
// UserNavigator
//**********************************************************************


UserNavigator::UserNavigator( uint32 ID ) 
      : UserElement( UserConstants::TYPE_NAVIGATOR )
{
   m_dbNavigator = new DBUserNavigator();
   setID( ID );
   m_dbNavigator->reset();
}


UserNavigator::UserNavigator( const UserReplyPacket* p, int& pos ) 
      : UserElement( UserConstants::TYPE_NAVIGATOR )
{
   uint32 type = 0;
   uint16 size = 0;

   type = p->incReadLong( pos );
   size = p->incReadShort( pos );
   m_dbNavigator = new DBUserNavigator( p, pos );
   type = p->incReadLong( pos );
   if ( type != UserConstants::TYPE_NAVIGATOR ) {
      mc2log << error <<  "UserNavigator::UserNavigator( p, pos ) "
             << "Data not ended by TYPE_NAVIGATOR!!!" << endl;
   } else {
      setOk( true );
   }
}


UserNavigator::UserNavigator( SQLQuery* pQuery ) 
      : UserElement( UserConstants::TYPE_NAVIGATOR )
{
   m_dbNavigator = new DBUserNavigator( pQuery );
}


UserNavigator::UserNavigator( const UserNavigator& orig ) 
      : UserElement( UserConstants::TYPE_NAVIGATOR )
{
   m_dbNavigator = new DBUserNavigator( *orig.m_dbNavigator );
}


UserNavigator::~UserNavigator() {
   delete m_dbNavigator;
}


uint32 
UserNavigator::getSize() const {
   return 1;
}


void
UserNavigator::packInto( Packet* p, int& pos ) {
   p->incWriteLong( pos, UserConstants::TYPE_NAVIGATOR );
   p->incWriteShort( pos, getSize() );
   p->incWriteLong( pos, UserConstants::TYPE_NAVIGATOR );
   p->incWriteShort( pos, getSize() );
   m_dbNavigator->packInto( p, pos );
   
   p->incWriteLong( pos, UserConstants::TYPE_NAVIGATOR );
   p->incWriteLong( pos, UserConstants::TYPE_NAVIGATOR );
}


void
UserNavigator::addChanges( Packet* p, int& pos ) {
   p->incWriteLong(pos, UserConstants::TYPE_NAVIGATOR);
   p->incWriteLong(pos, getID() );
   p->incWriteLong(pos, UserConstants::TYPE_NAVIGATOR );
   p->incWriteLong(pos, getID() );
   if ( removed() ) {
      p->incWriteByte( pos, UserConstants::ACTION_DELETE );
   } else {
      if ( getID() == 0 ) {
         p->incWriteByte( pos, UserConstants::ACTION_NEW );
      } else {
         p->incWriteByte( pos, UserConstants::ACTION_CHANGE );
      }
   }
   m_dbNavigator->addChanges( p, pos );
   p->incWriteLong(pos, UserConstants::TYPE_NAVIGATOR);
   p->incWriteLong(pos, UserConstants::TYPE_NAVIGATOR);
}


bool
UserNavigator::readChanges( const Packet* p, int& pos, 
                            UserConstants::UserAction& action )
{
   uint32 type = 0;
   type = p->incReadLong( pos);
   p->incReadLong( pos );
   action = UserConstants::UserAction( p->incReadByte( pos ) );
   m_dbNavigator->readChanges( p, pos );
   type = p->incReadLong( pos);

   if ( action == UserConstants::ACTION_DELETE ) {
      remove();
   }
   return (type == UserConstants::TYPE_NAVIGATOR);
}


uint32 
UserNavigator::getNbrChanged() const {
   return m_dbNavigator->getNbrChanged();
}


uint32
UserNavigator::getID() const {
   return m_dbNavigator->getID();
}


void
UserNavigator::setID( uint32 ID ) {
   m_dbNavigator->setID( ID );
}


uint32 
UserNavigator::getUserUIN() const {
   return m_dbNavigator->getUserUIN();
}


void
UserNavigator::setUserUIN( uint32 UIN ) {
   m_dbNavigator->setUserUIN( UIN );
}


UserConstants::navigatorType 
UserNavigator::getNavigatorType() {
   return m_dbNavigator->getNavigatorType();
}


void
UserNavigator::setNavigatorType( UserConstants::navigatorType navType ) {
   m_dbNavigator->setNavigatorType( navType );
}


const char*
UserNavigator::getAddress() const {
   return m_dbNavigator->getAddress();
}


void
UserNavigator::setAddress( const char* address ) {
   m_dbNavigator->setAddress( address );
}


uint16 
UserNavigator::getUnsentCount() const {
   return m_dbNavigator->getUnsentCount();
}


void
UserNavigator::setUnsentCount( uint16 count ) {
   m_dbNavigator->setUnsentCount( count );
}


uint32
UserNavigator::getLastContactDate() const {
   return m_dbNavigator->getLastContactDate();
}


void
UserNavigator::setLastContactDate( uint32 date ) {
   m_dbNavigator->setLastContactDate( date );
}


bool
UserNavigator::getLastContactSuccess() const {
   return m_dbNavigator->getLastContactSuccess();
}


void
UserNavigator::setLastContactSuccess( bool success ) {
   m_dbNavigator->setLastContactSuccess( success );
}


uint32 
UserNavigator::getLastContactLat() const {
   return m_dbNavigator->getLastContactLat();
}


void 
UserNavigator::setLastContactLat( uint32 lat ) {
   m_dbNavigator->setLastContactLat( lat ); 
}


uint32 
UserNavigator::getLastContactLon() const {
   return m_dbNavigator->getLastContactLon();
}


void 
UserNavigator::setLastContactLon( uint32 lon ) {
   m_dbNavigator->setLastContactLon( lon ); 
}


DBUserNavigator* 
UserNavigator::getDB() {
   return m_dbNavigator;
}


bool 
UserNavigator::isChanged() const {
   return getNbrChanged() > 0 || removed();
}


//**********************************************************************
// DBUserNavDestination
//**********************************************************************


const uint32 DBUserNavDestination::m_nbrFields = 11;


const char* DBUserNavDestination::m_fieldNames[ m_nbrFields ] = { 
   "id",
   "navigatorID",
   "sent",
   "createdDate",
   "type",
   "name",
   "senderID",
   "receiverType",
   "receiverAddress", 
   "lat",
   "lon" };

const DBaseElement::element_t DBUserNavDestination::m_fieldTypes[ 
   m_nbrFields ] = {
      DBaseElement::DB_INT_NOTNULL,           // id
      DBaseElement::DB_INT_NOTNULL,           // nav id
      DBaseElement::DB_BOOL_NOTNULL,          // Sent
      DBaseElement::DB_INT_NOTNULL,           // createDate
      DBaseElement::DB_INT_NOTNULL,           // Type
      DBaseElement::DB_LARGESTRING_NOTNULL,   // Name
      DBaseElement::DB_INT_NOTNULL,           // sender ID
      DBaseElement::DB_INT_NOTNULL,           // receiver Type
      DBaseElement::DB_LARGESTRING_NOTNULL,   // receiver Address
      DBaseElement::DB_INT,                   // latitude   
      DBaseElement::DB_INT                    // longitude   
   };

const char* DBUserNavDestination::m_fieldDescriptionStrs[ m_nbrFields ] = {
   "id",
   "navigatorID",
   "sent",
   "createdDate",
   "type",
   "name",
   "senderID",
   "receiverType",
   "receiverAddress",
   "lat",
   "lon"
};

const char* DBUserNavDestination::m_tableStr = 
"ISABUserNavDestination";


const StringTable::stringCode DBUserNavDestination::m_fieldDescriptions[ 
   m_nbrFields ] = {
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING };


DBUserNavDestination::DBUserNavDestination()
    : DBaseObject( m_nbrFields )
{
   // Always call init from subclass
   init( m_nbrFields );
}


DBUserNavDestination::DBUserNavDestination( 
   const DBUserNavDestination& copy )
      : DBaseObject( copy )
{
   // Do not call init from copy constructor
}


DBUserNavDestination::DBUserNavDestination( SQLQuery* pQuery )
      : DBaseObject( m_nbrFields )
{
   // Always call init from subclass
   init( m_nbrFields, pQuery );
}


DBUserNavDestination::DBUserNavDestination( const Packet* pPacket, int& pos )
   : DBaseObject( m_nbrFields )
{
   // Always call init from subclass
   init( m_nbrFields, pPacket, pos );
}


DBUserNavDestination::~DBUserNavDestination() {
}


const char*
DBUserNavDestination::getTableStr() const {
   return m_tableStr;
}


const char* 
DBUserNavDestination::getTableName() {
   return m_tableStr; 
}


const char** 
DBUserNavDestination::getFieldDescriptions() const {
   return m_fieldDescriptionStrs;
}


const StringTable::stringCode* 
DBUserNavDestination::getFieldStringCode() const {
   return m_fieldDescriptions;
}


const char**
DBUserNavDestination::getFieldNames() const {
   return m_fieldNames;
}


const DBaseElement::element_t* 
DBUserNavDestination::getFieldTypes() const {
   return m_fieldTypes;
}


uint32
DBUserNavDestination::getNbrChanged() const {
   DBaseElement* pDBElement = NULL;
   uint32 nbrChanges = 0;
   int nbrFields = m_dBaseElm.size();

   for( int i = 0; i < nbrFields; i++ ) {
      pDBElement = static_cast<DBaseElement*>(m_dBaseElm[i]);
      if( pDBElement->changed() ) {
         nbrChanges++;
      }
   }
   
   return nbrChanges;
}


uint32
DBUserNavDestination::getID() const {
   return getInt( field_ID );
}


void
DBUserNavDestination::setID( uint32 ID ) {
   setInt( field_ID, ID );
}


uint32
DBUserNavDestination::getNavigatorID() const {
   return getInt( field_navID );
}


void
DBUserNavDestination::setNavigatorID( uint32 navID ) {
   setInt( field_navID, navID );
}


bool
DBUserNavDestination::getSent() const {
   return getBool( field_sent );
}


void
DBUserNavDestination::setSent( bool sent ) {
   setBool( field_sent, sent );
}


uint32
DBUserNavDestination::getCreationDate() const {
   return getInt( field_created );
}


void
DBUserNavDestination::setCreationDate( uint32 date ) {
   setInt( field_created, date );
}


UserConstants::navigatorMessageType
DBUserNavDestination::getMessageType() const {
   return UserConstants::navigatorMessageType( getInt( field_type ) );
}


void
DBUserNavDestination::setMessageType( 
   UserConstants::navigatorMessageType type )
{
   setInt( field_type, type );
}


const char* 
DBUserNavDestination::getName() const {
   return getString( field_name );
}


void
DBUserNavDestination::setName( const char* name ) {
   setString( field_name, name );
}


uint32
DBUserNavDestination:: getSenderID() const {
   return getInt( field_senderID );
}


void
DBUserNavDestination::setSenderID( uint32 ID ) {
   setInt( field_senderID, ID );
}


UserConstants::navigatorType
DBUserNavDestination::getNavigatorType() {
   return UserConstants::navigatorType( getInt( field_receiverType ) );
}


void
DBUserNavDestination::setNavigatorType( 
   UserConstants::navigatorType navType )
{
   setInt( field_receiverType, navType ); 
}


const char* 
DBUserNavDestination::getAddress() const {
   return getString( field_receiverAddress );
}


void
DBUserNavDestination::setAddress( const char* address ) {
   setString( field_receiverAddress, address );
}


uint32
DBUserNavDestination::getLat() const {
   return getInt( field_latitude ); 
}


void
DBUserNavDestination::setLat( uint32 lat ) {
   setInt( field_latitude, lat );
}


uint32
DBUserNavDestination::getLon() const {
   return getInt( field_longitude ); 
}


void
DBUserNavDestination::setLon( uint32 lon ) {
   setInt( field_longitude, lon );
}



// ---- DebitElement ----------------------------------------------------


DebitElement::DebitElement( uint32 messageID,
                            uint32 debInfo,
                            uint32 time,
                            uint32 operationType,
                            uint32 sentSize,
                            const char* userOrigin,
                            const char* serverID,
                            const char* operationDescription )
      : m_messageID( messageID ), m_sentSize( sentSize ),
        m_debitInfo( debInfo ), m_time( time ),
        m_operationType( operationType ),
        m_userOrigin( StringUtility::newStrDup( userOrigin ) ),
        m_serverID( StringUtility::newStrDup( serverID ) ),
        m_description( StringUtility::newStrDup( operationDescription ) )
{
}


DebitElement::DebitElement() 
      : m_messageID( 0 ), m_sentSize( 0 ),
        m_debitInfo( 0 ), m_time( 0 ),
        m_operationType( 0 ), 
        m_userOrigin( NULL ),
        m_serverID( NULL ),
        m_description( NULL ) 
{
}


DebitElement::~DebitElement() {
   delete [] m_userOrigin;
   delete [] m_serverID;
   delete [] m_description;
}


uint32 
DebitElement::getSize() const {
   return 5*4 + /* messageID debInfo time operationType sentSize */
      strlen( getUserOrigin() ) + 1 + strlen( getServerID() ) + 1 + 
      strlen( getDescription() ) + 1 + 3 /* 3 possible padding */ ;
}


void 
DebitElement::packInto( Packet* p, int& pos ) const {
   if ( pos + getSize() >= p->getBufSize() ) {
      p->resize( p->getBufSize()*2 + getSize() );
   }

   p->incWriteLong( pos, getMessageID() );
   p->incWriteLong( pos, getDebitInfo() );
   p->incWriteLong( pos, getTime() );
   p->incWriteLong( pos, getOperationType() );
   p->incWriteLong( pos, getSentSize() );
   p->incWriteString( pos, getUserOrigin() );
   p->incWriteString( pos, getServerID() );
   p->incWriteString( pos, getDescription() );
   p->setLength( pos );
}


void 
DebitElement::readFromPacket( const Packet* p, int& pos ) {
   m_messageID = p->incReadLong( pos );
   m_debitInfo = p->incReadLong( pos );
   m_time = p->incReadLong( pos );
   m_operationType = p->incReadLong( pos );
   m_sentSize = p->incReadLong( pos );
   char* tmpStr = NULL;
   p->incReadString( pos, tmpStr );
   m_userOrigin = StringUtility::newStrDup( tmpStr );
   p->incReadString( pos, tmpStr );
   m_serverID = StringUtility::newStrDup( tmpStr );
   p->incReadString( pos, tmpStr );
   m_description = StringUtility::newStrDup( tmpStr );
}


// **********************************************************************
// UserRegionAccess
// **********************************************************************


UserRegionAccess::UserRegionAccess( uint32 id,
                                    uint32 regionID,
                                    uint32 startTime,
                                    uint32 endTime )
      : UserElement( UserConstants::TYPE_REGION_ACCESS ),
        m_regionID( regionID ), m_startTime( startTime ),
        m_endTime( endTime )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0; i < UserConstants::USER_REGION_ACCESS_NBRFIELDS ; 
         i++ )
   {
      m_changed[ i ] = false;
   }
}


UserRegionAccess::UserRegionAccess( uint32 id ) 
      : UserElement( UserConstants::TYPE_REGION_ACCESS ),
        m_regionID( MAX_INT32 ), m_startTime( 0 ),
        m_endTime( MAX_INT32 )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0; i < UserConstants::USER_REGION_ACCESS_NBRFIELDS ; 
         i++ )
   {
      m_changed[ i ] = false;
   }
}


UserRegionAccess::UserRegionAccess( const UserReplyPacket* p, int& pos ) 
      : UserElement( UserConstants::TYPE_REGION_ACCESS ) 
{
   readFromPacket( p, pos );
}


UserRegionAccess::UserRegionAccess( const UserRegionAccess& user ) 
      : UserElement( UserConstants::TYPE_REGION_ACCESS )
{
   m_id = user.m_id;
   m_regionID = user.m_regionID;
   m_startTime = user.m_startTime;
   m_endTime = user.m_endTime;
   for ( uint8 i = 0; i < UserConstants::USER_REGION_ACCESS_NBRFIELDS ; 
         i++ )
   {
      m_changed[ i ] = user.m_changed[ i ];
   }
}


UserRegionAccess::~UserRegionAccess() {
   // Nothing newed in constructor (yet)
}


uint32
UserRegionAccess::getSize() const {
   return 16 + 4*4 + 2*4 + 3;
}


void
UserRegionAccess::packInto( Packet* p, int& pos ) {
   uint32 size = getSize();

   p->incWriteLong( pos, UserConstants::TYPE_REGION_ACCESS );
   p->incWriteShort( pos, size );
   p->incWriteLong( pos, UserConstants::TYPE_REGION_ACCESS );
   p->incWriteShort( pos, size );

   p->incWriteLong( pos, m_id );
   p->incWriteLong( pos, m_regionID );
   p->incWriteLong( pos, m_startTime );
   p->incWriteLong( pos, m_endTime );

   p->incWriteLong( pos, UserConstants::TYPE_REGION_ACCESS );
   p->incWriteLong( pos, UserConstants::TYPE_REGION_ACCESS );

   // Set length 
   p->setLength( pos );
}


void
UserRegionAccess::addChanges( Packet* p, int& position ) {
   uint8 nbrChanges = 0;
   for ( uint8 i = 0; i < UserConstants::USER_REGION_ACCESS_NBRFIELDS ; 
         i++ )
      if ( m_changed[ i ] )
         nbrChanges++;
   
   if ( nbrChanges > 0 || removed() || m_id == 0 ) {
      p->incWriteLong( position, UserConstants::TYPE_REGION_ACCESS );
      p->incWriteLong( position, m_id );
      p->incWriteLong( position, UserConstants::TYPE_REGION_ACCESS );
      p->incWriteLong( position, m_id );
      if ( removed() ) {
         p->incWriteByte( position, UserConstants::ACTION_DELETE );
      } else if ( m_id == 0 ) {
         p->incWriteByte(position, UserConstants::ACTION_NEW);
         // Not id and not userUIN
         // Nbr changed
         p->incWriteByte( position, 
                          UserConstants::USER_REGION_ACCESS_NBRFIELDS -2 );
         p->incWriteByte( position, 
                          UserConstants::USER_REGION_ACCESS_REGION_ID );
         p->incWriteLong( position, m_regionID );
         p->incWriteByte( position, 
                          UserConstants::USER_REGION_ACCESS_START_TIME );
         p->incWriteLong( position, m_startTime );
         p->incWriteByte( position, 
                          UserConstants::USER_REGION_ACCESS_END_TIME );
         p->incWriteLong( position, m_endTime );
      } else {
         p->incWriteByte( position, UserConstants::ACTION_CHANGE );
         p->incWriteByte( position, nbrChanges );
         
         if ( m_changed[ UserConstants::USER_REGION_ACCESS_REGION_ID ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_REGION_ACCESS_REGION_ID );
            p->incWriteLong( position, m_regionID );
         }
         if ( m_changed[ UserConstants::USER_REGION_ACCESS_START_TIME ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_REGION_ACCESS_START_TIME);
            p->incWriteLong( position, m_startTime );
         }
         if ( m_changed[ UserConstants::USER_REGION_ACCESS_END_TIME ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_REGION_ACCESS_END_TIME );
            p->incWriteLong( position, m_endTime );
         }
      } // End else changed

      p->incWriteLong( position, UserConstants::TYPE_REGION_ACCESS );
      p->incWriteLong( position, UserConstants::TYPE_REGION_ACCESS );
      for ( uint8 i = 0; i < UserConstants::USER_REGION_ACCESS_NBRFIELDS ;
            i++ )
      {
         m_changed[ i ] = false;
      }
      p->setLength( position );
   } else {
      mc2log << warn << "UserRegionAccess::addChanges but there is no "
             << "changes to save?" << endl;
   }
}



bool
UserRegionAccess::readChanges( const Packet* p, int& pos, 
                               UserConstants::UserAction& action )
{
   if ( p->getLength() - pos - 5 > 0 ) {
      uint32 type;
      uint32 id;
      
      type = p->incReadLong( pos );
      id = p->incReadLong( pos );
      if ( type == UserConstants::TYPE_REGION_ACCESS && id == m_id ) {
         action = UserConstants::UserAction( p->incReadByte( pos ) );
         switch ( action ) {
            case UserConstants::ACTION_NEW:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_DELETE:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_REGION_ACCESS ) {
                  remove();
                  return true;
               } else {
                  mc2log << warn
                         << "UserRegionAccess::readChanges not Type "
                         << " Region access after action delete" << endl;
                  return false;
               }
               break;
            case UserConstants::ACTION_CHANGE:
               return ( readDataChanges( p, pos ) );
               break;
            case UserConstants::ACTION_NOP:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_REGION_ACCESS ) {
                  return true;
               } else {
                  mc2log << warn
                         << "UserRegionAccess::readChanges not Type "
                         << " region access after action NOP" << endl;
                  return false;
               }
               break;
            default:
               mc2log << warn
                      << "UserRegionAccess::readChanges unknown action: "
                      << (int)action << endl;
         }
         return false;
      } else {
         mc2log << error
                << "UserRegionAccess::readChanges not correct type or "
                << " not correct id, giving up." << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserRegionAccess::readChanges no space for changes "
             << "giving up." << endl;
      return false;
   }
}


bool
UserRegionAccess::readDataChanges( const Packet* p, int& pos ) {
   if ( p->getLength() - pos - 1 > 0 ) {
      byte nbrFields;

      nbrFields = p->incReadByte( pos );

      // tmpdata
      uint32 regionID = m_regionID;
      uint32 startTime = m_startTime;
      uint32 endTime = m_endTime;

      byte field;
      bool ok = true;
      for ( uint8 i = 0 ; 
            (i < nbrFields) && ok && (pos < (int32)p->getLength()) ;
            i ++ ) 
      {
         field = p->incReadByte( pos );
         switch ( field ) {
            case UserConstants::USER_REGION_ACCESS_REGION_ID :
               m_changed[ UserConstants::USER_REGION_ACCESS_REGION_ID ] = 
                  true;
               regionID = p->incReadLong( pos );
               break;
            case UserConstants::USER_REGION_ACCESS_START_TIME :
               m_changed[ UserConstants::USER_REGION_ACCESS_START_TIME ] = 
                  true;
               startTime = p->incReadLong( pos );
               break;
            case UserConstants::USER_REGION_ACCESS_END_TIME :
               m_changed[ UserConstants::USER_REGION_ACCESS_END_TIME ] = 
                  true;
               endTime = p->incReadLong( pos );
               break;

            default:
               mc2log << error
                      << "UserRegionAccess::readDataChanges unknown " 
                      << "fieldType: " << (int)field << endl;
               ok = false;
               break;
         }
      }

      uint32 type = p->incReadLong( pos );
      if ( type != UserConstants::TYPE_REGION_ACCESS ) {
         mc2log << error
                << "UserRegionAccess::readDataChanges Not REGION_ACCESS "
                << " type after data" << endl;
         ok = false;
      }
      if ( ok ) {
         // Transfer changes to membervariables
         if ( m_changed[ UserConstants::USER_REGION_ACCESS_REGION_ID ] ) {
           m_regionID = regionID;
         }
         if ( m_changed[ UserConstants::USER_REGION_ACCESS_START_TIME ] ) {
           m_startTime = startTime;
         }
         if ( m_changed[ UserConstants::USER_REGION_ACCESS_END_TIME ] ) {
           m_endTime = endTime;
         }
         return ok;
      } else {
         mc2log << error
                << "UserRegionAccess::readDataChanges failed" << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserRegionAccess::readDataChanges no space for changes" 
             << endl;
      return false;
   }
}


void
UserRegionAccess::readFromPacket( const Packet* p, int& pos ) {
   uint32 type = p->incReadLong( pos );

   if ( type == UserConstants::TYPE_REGION_ACCESS ) {
      uint32 size = p->incReadShort( pos );
      
      if ( (getSize() - 6) <= size ) {
         m_id = p->incReadLong( pos );
         m_regionID = p->incReadLong( pos );
         m_startTime = p->incReadLong( pos );
         m_endTime = p->incReadLong( pos );

         type = p->incReadLong( pos );
         if ( type != UserConstants::TYPE_REGION_ACCESS ) {
            mc2log << warn
                   << "UserRegionAccess::readFromPacket( "
                   << "Packet* p, int& pos )"
                   << "Data not ended by TYPE_REGION_ACCESS_KEY!!!"
                   << endl;
            return;
         }
         // DONE
      } else {
         mc2log << warn  
                << "UserRegionAccess::readFromPacket(Packet* p, int& pos)"
                << " Not size for data!! Nothing read" << endl;
         return;
      }
   } else {
      mc2log << warn 
             << "UserRegionAccess::readFromPacket( const Packet* p, int& pos )"
             << "Not TYPE_REGION_ACCESS_KEY type!!!!!" << endl;
      return;
   }

   setOk( true );

   for ( uint8 i = 0; i < UserConstants::USER_REGION_ACCESS_NBRFIELDS ; 
         i++ )
   {
      m_changed[ i ] = false;
   }
}


uint32
UserRegionAccess::getNbrChanged() const {
   uint32 nbrChanged = 0;
   for ( uint8 i = 0; i < UserConstants::USER_REGION_ACCESS_NBRFIELDS ; 
         i++ )
   {
      if ( m_changed[ i ] ) {
         nbrChanged++;
      }
   }
   return nbrChanged;
}


uint32
UserRegionAccess::printValue( 
   char* target, UserConstants::UserRegionAccessField field ) const
{
   switch ( field ) {
      case UserConstants::USER_REGION_ACCESS_ID :
      case UserConstants::USER_REGION_ACCESS_USERUIN :
         mc2log << error << "UserRegionAccess::printValue asked to print "
                << "ID or userUIN, not supported." << endl;
         MC2_ASSERT( false );
         break;
      case UserConstants::USER_REGION_ACCESS_REGION_ID :
         sprintf( target, "%d", getRegionID() );
         break;
      case UserConstants::USER_REGION_ACCESS_START_TIME :
         sprintf( target, "%d", getStartTime() );
         break;
      case UserConstants::USER_REGION_ACCESS_END_TIME :
         sprintf( target, "%d", getEndTime() );
         break;

      default:
         mc2log << error << "UserRegionAccess::printValue unknown " 
                << "fieldType: " << (int)field << endl;
         break;
   }

   return strlen( target );
}


bool
UserRegionAccess::changed( 
   UserConstants::UserRegionAccessField field ) const
{
   return m_changed[ field ];
}


bool
UserRegionAccess::isChanged() const {
   return getNbrChanged() > 0 || m_id == 0 || removed();
}

// **********************************************************************
// UserWayfinderSubscription
// **********************************************************************


UserWayfinderSubscription::UserWayfinderSubscription( uint32 id,
                                                      byte type )
      : UserElement( UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ),
        m_wayfinderType( type )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0 ; 
         i < UserConstants::USER_WAYFINDER_SUBSCRIPTION_NBRFIELDS ; 
         i++ )
   {
      m_changed[ i ] = false;
   }
}


UserWayfinderSubscription::UserWayfinderSubscription( uint32 id ) 
      : UserElement( UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ),
        m_wayfinderType( MAX_BYTE )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0 ; 
         i < UserConstants::USER_WAYFINDER_SUBSCRIPTION_NBRFIELDS ; 
         i++ )
   {
      m_changed[ i ] = false;
   }
}


UserWayfinderSubscription::UserWayfinderSubscription( 
   const UserReplyPacket* p, int& pos ) 
      : UserElement( UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) 
{
   readFromPacket( p, pos );
}


UserWayfinderSubscription::UserWayfinderSubscription( 
   const UserWayfinderSubscription& user ) 
      : UserElement( UserConstants::TYPE_WAYFINDER_SUBSCRIPTION )
{
   m_id = user.m_id;
   m_wayfinderType = user.m_wayfinderType;
      for ( uint8 i = 0 ; 
         i < UserConstants::USER_WAYFINDER_SUBSCRIPTION_NBRFIELDS ; 
         i++ )
   {
      m_changed[ i ] = user.m_changed[ i ];
   }
}


UserWayfinderSubscription::~UserWayfinderSubscription() {
   // Nothing newed in constructor (yet)
}


uint32
UserWayfinderSubscription::getSize() const {
   return 16 + 4*1 + 1*1 + 2*4 + 3;
}


void
UserWayfinderSubscription::packInto( Packet* p, int& pos ) {
   uint32 size = getSize();

   p->incWriteLong( pos, UserConstants::TYPE_WAYFINDER_SUBSCRIPTION );
   p->incWriteShort( pos, size );
   p->incWriteLong( pos, UserConstants::TYPE_WAYFINDER_SUBSCRIPTION );
   p->incWriteShort( pos, size );

   p->incWriteLong( pos, m_id );
   p->incWriteByte( pos, m_wayfinderType );

   p->incWriteLong( pos, UserConstants::TYPE_WAYFINDER_SUBSCRIPTION );
   p->incWriteLong( pos, UserConstants::TYPE_WAYFINDER_SUBSCRIPTION );

   // Set length 
   p->setLength( pos );
}


void
UserWayfinderSubscription::addChanges( Packet* p, int& position ) {
   uint8 nbrChanges = 0;
   for ( uint8 i = 0 ; 
         i < UserConstants::USER_WAYFINDER_SUBSCRIPTION_NBRFIELDS ; 
         i++ )
      if ( m_changed[ i ] )
         nbrChanges++;
   
   if ( nbrChanges > 0 || removed() || m_id == 0 ) {
      p->incWriteLong( position, 
                       UserConstants::TYPE_WAYFINDER_SUBSCRIPTION );
      p->incWriteLong( position, m_id );
      p->incWriteLong( position, 
                       UserConstants::TYPE_WAYFINDER_SUBSCRIPTION );
      p->incWriteLong( position, m_id );
      if ( removed() ) {
         p->incWriteByte( position, UserConstants::ACTION_DELETE );
      } else if ( m_id == 0 ) {
         p->incWriteByte(position, UserConstants::ACTION_NEW);
         // Not id and not userUIN
         // Nbr changed
         p->incWriteByte( 
            position, 
            UserConstants::USER_WAYFINDER_SUBSCRIPTION_NBRFIELDS -2 );
         p->incWriteByte( 
            position, 
            UserConstants::USER_WAYFINDER_SUBSCRIPTION_TYPE );
         p->incWriteByte( position, m_wayfinderType );
      } else {
         p->incWriteByte( position, UserConstants::ACTION_CHANGE );
         p->incWriteByte( position, nbrChanges );
         
         if ( m_changed[ 
                 UserConstants::USER_WAYFINDER_SUBSCRIPTION_TYPE ] ) {
            p->incWriteByte( 
               position, 
               UserConstants::USER_WAYFINDER_SUBSCRIPTION_TYPE );
            p->incWriteByte( position, m_wayfinderType );
         }
      } // End else changed

      p->incWriteLong( position, 
                       UserConstants::TYPE_WAYFINDER_SUBSCRIPTION );
      p->incWriteLong( position, 
                       UserConstants::TYPE_WAYFINDER_SUBSCRIPTION );
      for ( uint8 i = 0 ; 
            i < UserConstants::USER_WAYFINDER_SUBSCRIPTION_NBRFIELDS ;
            i++ )
      {
         m_changed[ i ] = false;
      }
      p->setLength( position );
   } else {
      mc2log << warn << "UserWayfinderSubscription::addChanges but there "
             << "is no changes to save?" << endl;
   }
}



bool
UserWayfinderSubscription::readChanges( const Packet* p, int& pos, 
                                        UserConstants::UserAction& action )
{
   if ( p->getLength() - pos - 5 > 0 ) {
      uint32 type;
      uint32 id;
      
      type = p->incReadLong( pos );
      id = p->incReadLong( pos );
      if ( type == UserConstants::TYPE_WAYFINDER_SUBSCRIPTION && 
           id == m_id ) 
      {
         action = UserConstants::UserAction( p->incReadByte( pos ) );
         switch ( action ) {
            case UserConstants::ACTION_NEW:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_DELETE:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) {
                  remove();
                  return true;
               } else {
                  mc2log << warn
                         << "UserWayfinderSubscription::readChanges not "
                         << "Type Wayfinder Subscription after action "
                         << "delete" << endl;
                  return false;
               }
               break;
            case UserConstants::ACTION_CHANGE:
               return ( readDataChanges( p, pos ) );
               break;
            case UserConstants::ACTION_NOP:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) {
                  return true;
               } else {
                  mc2log << warn
                         << "UserWayfinderSubscription::readChanges not "
                         << "Type Wayfinder Subscription "
                         << "after action NOP" << endl;
                  return false;
               }
               break;
            default:
               mc2log << warn
                      << "UserWayfinderSubscription::readChanges unknown "
                      << "action: " << (int)action << endl;
         }
         return false;
      } else {
         mc2log << error
                << "UserWayfinderSubscription::readChanges not correct "
                << "type or not correct id, giving up." << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserWayfinderSubscription::readChanges no space for "
             << "changes giving up." << endl;
      return false;
   }
}


bool
UserWayfinderSubscription::readDataChanges( const Packet* p, int& pos ) {
   if ( p->getLength() - pos - 1 > 0 ) {
      byte nbrFields;

      nbrFields = p->incReadByte( pos );

      // tmpdata
      byte type = m_wayfinderType;

      byte field;
      bool ok = true;
      for ( uint8 i = 0 ; 
            (i < nbrFields) && ok && (pos < (int32)p->getLength()) ;
            i ++ ) 
      {
         field = p->incReadByte( pos );
         switch ( field ) {
            case UserConstants::USER_WAYFINDER_SUBSCRIPTION_TYPE :
               m_changed[ UserConstants::USER_WAYFINDER_SUBSCRIPTION_TYPE ]
                  = true;
               type = p->incReadByte( pos );
               break;

            default:
               mc2log << error
                      << "UserWayfinderSubscription::readDataChanges "
                      << "unknown fieldType: " << (int)field << endl;
               ok = false;
               break;
         }
      }

      uint32 packetType = p->incReadLong( pos );
      if ( packetType != UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) {
         mc2log << error
                << "UserWayfinderSubscription::readDataChanges Not "
                << "WAYFINDER_SUBSCRIPTION type after data" << endl;
         ok = false;
      }
      if ( ok ) {
         // Transfer changes to membervariables
         if ( m_changed[ 
                 UserConstants::USER_WAYFINDER_SUBSCRIPTION_TYPE ] ) 
         {
           m_wayfinderType = type;
         }
         return ok;
      } else {
         mc2log << error
                << "UserWayfinderSubscription::readDataChanges failed" 
                << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserWayfinderSubscription::readDataChanges no space for "
             << "changes" << endl;
      return false;
   }
}


void
UserWayfinderSubscription::readFromPacket( const Packet* p, int& pos ) {
   uint32 type = p->incReadLong( pos );

   if ( type == UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) {
      uint32 size = p->incReadShort( pos );
      
      if ( (getSize() - 6) <= size ) {
         m_id = p->incReadLong( pos );
         m_wayfinderType = p->incReadByte( pos );

         type = p->incReadLong( pos );
         if ( type != UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) {
            mc2log << warn
                   << "UserWayfinderSubscription::readFromPacket( "
                   << "Packet* p, int& pos )"
                   << "Data not ended by TYPE_WAYFINDER_SUBSCRIPTION!!!"
                   << endl;
            return;
         }
         // DONE
      } else {
         mc2log << warn  
                << "UserWayfinderSubscription::readFromPacket"
                << "(Packet* p, int& pos)"
                << " Not size for data!! Nothing read" << endl;
         return;
      }
   } else {
      mc2log << warn 
             << "UserWayfinderSubscription::readFromPacket"
             << "( Packet* p, int& pos )"
             << "Not TYPE_WAYFINDER_SUBSCRIPTION type!!!!!" << endl;
      return;
   }
   
   setOk( true );

   for ( uint8 i = 0 ; 
         i < UserConstants::USER_WAYFINDER_SUBSCRIPTION_NBRFIELDS ; 
         i++ )
   {
      m_changed[ i ] = false;
   }
}


uint32
UserWayfinderSubscription::getNbrChanged() const {
   uint32 nbrChanged = 0;
   for ( uint8 i = 0 ; 
         i < UserConstants::USER_WAYFINDER_SUBSCRIPTION_NBRFIELDS ; 
         i++ )
   {
      if ( m_changed[ i ] ) {
         nbrChanged++;
      }
   }
   return nbrChanged;
}


uint32
UserWayfinderSubscription::printValue( 
   char* target, 
   UserConstants::UserWayfinderSubscriptionField field ) const
{
   switch ( field ) {
      case UserConstants::USER_WAYFINDER_SUBSCRIPTION_ID :
      case UserConstants::USER_WAYFINDER_SUBSCRIPTION_USERUIN :
         mc2log << error << "UserWayfinderSubscription::printValue asked "
                << "to print ID or userUIN, not supported." << endl;
         MC2_ASSERT( false );
         break;
      case UserConstants::USER_WAYFINDER_SUBSCRIPTION_TYPE :
         sprintf( target, "%hd", getWayfinderType() );
         break;

      default:
         mc2log << error << "UserWayfinderSubscription::printValue unknown"
                << " fieldType: " << (int)field << endl;
         break;
   }

   return strlen( target );
}


bool
UserWayfinderSubscription::changed( 
   UserConstants::UserWayfinderSubscriptionField field ) const
{
   return m_changed[ field ];
}


bool
UserWayfinderSubscription::isChanged() const {
   return getNbrChanged() > 0 || m_id == 0 || removed();
}


// **********************************************************************
// UserToken
// **********************************************************************


UserToken::UserToken( uint32 id, uint32 createTime, byte age, 
                      const char* token, const MC2String& group )
      : UserElement( UserConstants::TYPE_TOKEN ),
        m_createTime( createTime ), m_age( age ), m_token( token ),
        m_group( group )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0 ; i < UserConstants::USER_TOKEN_NBRFIELDS ; ++i ) {
      m_changed[ i ] = false;
   }
}


UserToken::UserToken( uint32 id ) 
      : UserElement( UserConstants::TYPE_TOKEN ),
        m_createTime( 0 ), m_age( 0 ), m_token( "" ), m_group( "" )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0 ; i < UserConstants::USER_TOKEN_NBRFIELDS ; ++i ) {
      m_changed[ i ] = false;
   }
}


UserToken::UserToken( const Packet* p, int& pos ) 
      : UserElement( UserConstants::TYPE_TOKEN ) 
{
   readFromPacket( p, pos );
}


UserToken::UserToken( const UserToken& user ) 
      : UserElement( UserConstants::TYPE_TOKEN )
{
   m_id = user.m_id;
   m_createTime = user.m_createTime;
   m_age = user.m_age;
   m_token = user.m_token;
   m_group = user.m_group;
   for ( uint8 i = 0 ; i < UserConstants::USER_TOKEN_NBRFIELDS ; ++i ) {
      m_changed[ i ] = user.m_changed[ i ];
   }
}


UserToken::~UserToken() {
   // Nothing newed in constructor (yet)
}


uint32
UserToken::getSize() const {
   return 16 + 2*4 + 1*1 + m_token.size() + 1 + m_group.size() + 1 
      + 2*4 + 3;
}


void
UserToken::packInto( Packet* p, int& pos ) {
   uint32 size = getSize();

   p->incWriteLong( pos, UserConstants::TYPE_TOKEN );
   p->incWriteShort( pos, size );
   p->incWriteLong( pos, UserConstants::TYPE_TOKEN );
   p->incWriteShort( pos, size );

   p->incWriteLong( pos, m_id );
   p->incWriteLong( pos, m_createTime );
   p->incWriteByte( pos, m_age );
   p->incWriteString( pos, m_token.c_str() );
   p->incWriteString( pos, m_group.c_str() );

   p->incWriteLong( pos, UserConstants::TYPE_TOKEN );
   p->incWriteLong( pos, UserConstants::TYPE_TOKEN );

   // Set length 
   p->setLength( pos );
}


void
UserToken::addChanges( Packet* p, int& position ) {
   uint8 nbrChanges = 0;
   for ( uint8 i = 0 ; i < UserConstants::USER_TOKEN_NBRFIELDS ; ++i )
      if ( m_changed[ i ] )
         nbrChanges++;
   
   if ( nbrChanges > 0 || removed() || m_id == 0 ) {
      p->incWriteLong( position, 
                       UserConstants::TYPE_TOKEN );
      p->incWriteLong( position, m_id );
      p->incWriteLong( position, 
                       UserConstants::TYPE_TOKEN );
      p->incWriteLong( position, m_id );
      if ( removed() ) {
         p->incWriteByte( position, UserConstants::ACTION_DELETE );
      } else if ( m_id == 0 ) {
         p->incWriteByte(position, UserConstants::ACTION_NEW);
         // Not id and not userUIN
         // Nbr changed
         p->incWriteByte( 
            position, UserConstants::USER_TOKEN_NBRFIELDS -2 );
         p->incWriteByte( 
            position, UserConstants::USER_TOKEN_CREATE_TIME );
         p->incWriteLong( position, m_createTime );
         p->incWriteByte( position, UserConstants::USER_TOKEN_AGE );
         p->incWriteByte( position, m_age );
         p->incWriteByte( position, UserConstants::USER_TOKEN_TOKEN );
         p->incWriteString( position, m_token.c_str() );
         p->incWriteByte( position, UserConstants::USER_TOKEN_GROUP );
         p->incWriteString( position, m_group.c_str() );
      } else {
         p->incWriteByte( position, UserConstants::ACTION_CHANGE );
         p->incWriteByte( position, nbrChanges );
         
         if ( m_changed[ UserConstants::USER_TOKEN_CREATE_TIME ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_TOKEN_CREATE_TIME );
            p->incWriteLong( position, m_createTime );
         }
         if ( m_changed[ UserConstants::USER_TOKEN_AGE ] ) {
            p->incWriteByte( position, UserConstants::USER_TOKEN_AGE );
            p->incWriteByte( position, m_age );
         }
         if ( m_changed[ UserConstants::USER_TOKEN_TOKEN ] ) {
            p->incWriteByte( position, UserConstants::USER_TOKEN_TOKEN );
            p->incWriteString( position, m_token.c_str() );
         }
         if ( m_changed[ UserConstants::USER_TOKEN_GROUP ] ) {
            p->incWriteByte( position, UserConstants::USER_TOKEN_GROUP );
            p->incWriteString( position, m_group.c_str() );
         }
         
      } // End else changed

      p->incWriteLong( position, UserConstants::TYPE_TOKEN );
      p->incWriteLong( position, UserConstants::TYPE_TOKEN );
      for ( uint8 i = 0 ; i < UserConstants::USER_TOKEN_NBRFIELDS ; ++i ) {
         m_changed[ i ] = false;
      }
      p->setLength( position );
   } else {
      mc2log << warn << "UserToken::addChanges but there "
             << "is no changes to save?" << endl;
   }
}


bool
UserToken::readChanges( const Packet* p, int& pos, 
                        UserConstants::UserAction& action )
{
   if ( p->getLength() - pos - 5 > 0 ) {
      uint32 type;
      uint32 id;
      
      type = p->incReadLong( pos );
      id = p->incReadLong( pos );
      if ( type == UserConstants::TYPE_TOKEN && id == m_id ) {
         action = UserConstants::UserAction( p->incReadByte( pos ) );
         switch ( action ) {
            case UserConstants::ACTION_NEW:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_DELETE:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_TOKEN ) {
                  remove();
                  return true;
               } else {
                  mc2log << warn
                         << "UserToken::readChanges not "
                         << "Type Token after action delete" << endl;
                  return false;
               }
               break;
            case UserConstants::ACTION_CHANGE:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_NOP:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_TOKEN ) {
                  return true;
               } else {
                  mc2log << warn
                         << "UserToken::readChanges not "
                         << "Type Token after action NOP" << endl;
                  return false;
               }
               break;
            default:
               mc2log << warn
                      << "UserToken::readChanges unknown "
                      << "action: " << (int)action << endl;
         }
         return false;
      } else {
         mc2log << error
                << "UserToken::readChanges not correct "
                << "type or not correct id, giving up." << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserToken::readChanges no space for "
             << "changes giving up." << endl;
      return false;
   }
}


bool
UserToken::readDataChanges( const Packet* p, int& pos ) {
   if ( p->getLength() - pos - 1 > 0 ) {
      byte nbrFields;

      nbrFields = p->incReadByte( pos );

      // tmpdata
      uint32 createTime = m_createTime;
      byte age = m_age;
      MC2String token = m_token;
      MC2String group = m_group;

      byte field;
      bool ok = true;
      for ( uint8 i = 0 ; 
            (i < nbrFields) && ok && (pos < (int32)p->getLength()) ; ++i ) 
      {
         field = p->incReadByte( pos );
         switch ( field ) {
            case UserConstants::USER_TOKEN_AGE :
               m_changed[ UserConstants::USER_TOKEN_AGE ] = true;
               age = p->incReadByte( pos );
               break;
            case UserConstants::USER_TOKEN_CREATE_TIME :
               m_changed[ UserConstants::USER_TOKEN_CREATE_TIME ] = true;
               createTime = p->incReadLong( pos );
               break;
            case UserConstants::USER_TOKEN_TOKEN :
               m_changed[ UserConstants::USER_TOKEN_TOKEN ] = true;
               token = p->incReadString( pos );
               break;
            case UserConstants::USER_TOKEN_GROUP :
               m_changed[ UserConstants::USER_TOKEN_GROUP ] = true;
               group = p->incReadString( pos );
               break;

            default:
               mc2log << error
                      << "UserToken::readDataChanges "
                      << "unknown fieldType: " << (int)field << endl;
               ok = false;
               break;
         }
      }

      uint32 packetType = p->incReadLong( pos );
      if ( packetType != UserConstants::TYPE_TOKEN ) {
         mc2log << error
                << "UserToken::readDataChanges Not "
                << "TOKEN type after data" << endl;
         ok = false;
      }
      if ( ok ) {
         // Transfer changes to membervariables
         if ( m_changed[ UserConstants::USER_TOKEN_CREATE_TIME ] ) {
            m_createTime = createTime;
         }
         if ( m_changed[ UserConstants::USER_TOKEN_AGE ] ) {
           m_age = age;
         }
         if ( m_changed[ UserConstants::USER_TOKEN_TOKEN ] ) {
            m_token = token;
         }
         if ( m_changed[ UserConstants::USER_TOKEN_GROUP ] ) {
            m_group = group;
         }

         return ok;
      } else {
         mc2log << error << "UserToken::readDataChanges failed" << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserToken::readDataChanges no space for changes" << endl;
      return false;
   }
}


void
UserToken::readFromPacket( const Packet* p, int& pos ) {
   uint32 type = p->incReadLong( pos );

   if ( type == UserConstants::TYPE_TOKEN ) {
      uint32 size = p->incReadShort( pos );
      
      if ( (getSize() - 6) <= size ) {
         m_id = p->incReadLong( pos );
         m_createTime = p->incReadLong( pos );
         m_age = p->incReadByte( pos );
         m_token = p->incReadString( pos );
         m_group = p->incReadString( pos );

         type = p->incReadLong( pos );
         if ( type != UserConstants::TYPE_TOKEN ) {
            mc2log << warn
                   << "UserToken::readFromPacket( "
                   << "Packet* p, int& pos )"
                   << "Data not ended by TYPE_TOKEN!!!"
                   << endl;
            return;
         }
         // DONE
      } else {
         mc2log << warn  
                << "UserToken::readFromPacket"
                << "(Packet* p, int& pos)"
                << " Not size for data!! Nothing read" << endl;
         return;
      }
   } else {
      mc2log << warn 
             << "UserToken::readFromPacket"
             << "( Packet* p, int& pos )"
             << "Not TYPE_TOKEN type!!!!!" << endl;
      return;
   }
   
   setOk( true );

   for ( uint8 i = 0 ; i < UserConstants::USER_TOKEN_NBRFIELDS ; ++i ) {
      m_changed[ i ] = false;
   }
}


uint32
UserToken::getNbrChanged() const {
   uint32 nbrChanged = 0;
   for ( uint8 i = 0 ; i < UserConstants::USER_TOKEN_NBRFIELDS ; ++i ) {
      if ( m_changed[ i ] ) {
         nbrChanged++;
      }
   }
   return nbrChanged;
}


uint32
UserToken::printValue( char* target, UserConstants::UserTokenField field )
   const
{
   switch ( field ) {
      case UserConstants::USER_TOKEN_ID :
      case UserConstants::USER_TOKEN_USERUIN :
         mc2log << error << "UserToken::printValue asked "
                << "to print ID or userUIN, not supported." << endl;
         MC2_ASSERT( false );
         break;
      case UserConstants::USER_TOKEN_CREATE_TIME :
         sprintf( target, "%d", getCreateTime() );
         break;
      case UserConstants::USER_TOKEN_AGE :
         sprintf( target, "%hd", getAge() );
         break;
      case UserConstants::USER_TOKEN_TOKEN :
         strcat( target, "'" );
         sqlString( target + 1, getToken() );
         strcat( target, "'" );
         break;
      case UserConstants::USER_TOKEN_GROUP :
         strcat( target, "'" );
         sqlString( target + 1, getGroup().c_str() );
         strcat( target, "'" );
         break;

      default:
         mc2log << error << "UserToken::printValue unknown"
                << " fieldType: " << (int)field << endl;
         break;
   }

   return strlen( target );
}


bool
UserToken::changed( 
   UserConstants::UserTokenField field ) const
{
   return m_changed[ field ];
}


bool
UserToken::isChanged() const {
   return getNbrChanged() > 0 || m_id == 0 || removed();
}


// **********************************************************************
// UserPIN
// **********************************************************************


UserPIN::UserPIN( uint32 id, const char* PIN, const char* comment )
      : UserElement( UserConstants::TYPE_PIN ),
        m_PIN( PIN ), m_comment( comment )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0 ; i < UserConstants::USER_PIN_NBRFIELDS ; ++i ) {
      m_changed[ i ] = false;
   }
}


UserPIN::UserPIN( uint32 id ) 
      : UserElement( UserConstants::TYPE_PIN ),
        m_PIN( "" ), m_comment( "" )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0 ; i < UserConstants::USER_PIN_NBRFIELDS ; ++i ) {
      m_changed[ i ] = false;
   }
}


UserPIN::UserPIN( const Packet* p, int& pos ) 
      : UserElement( UserConstants::TYPE_PIN ) 
{
   readFromPacket( p, pos );
}


UserPIN::UserPIN( const UserPIN& user ) 
      : UserElement( UserConstants::TYPE_PIN )
{
   m_id = user.m_id;
   m_PIN = user.m_PIN;
   m_comment = user.m_comment;
   for ( uint8 i = 0 ; i < UserConstants::USER_PIN_NBRFIELDS ; ++i ) {
      m_changed[ i ] = user.m_changed[ i ];
   }
}


UserPIN::~UserPIN() {
   // Nothing newed in constructor (yet)
}


uint32
UserPIN::getSize() const {
   return 16 + 1*4 + m_PIN.size() + 1 + m_comment.size() + 1 + 2*4 + 3;
}


void
UserPIN::packInto( Packet* p, int& pos ) {
   uint32 size = getSize();

   p->incWriteLong( pos, UserConstants::TYPE_PIN );
   p->incWriteShort( pos, size );
   p->incWriteLong( pos, UserConstants::TYPE_PIN );
   p->incWriteShort( pos, size );

   p->incWriteLong( pos, m_id );
   p->incWriteString( pos, m_PIN.c_str() );
   p->incWriteString( pos, m_comment.c_str() );

   p->incWriteLong( pos, UserConstants::TYPE_PIN );
   p->incWriteLong( pos, UserConstants::TYPE_PIN );

   // Set length 
   p->setLength( pos );
}


void
UserPIN::addChanges( Packet* p, int& position ) {
   uint8 nbrChanges = 0;
   for ( uint8 i = 0 ; i < UserConstants::USER_PIN_NBRFIELDS ; ++i )
      if ( m_changed[ i ] )
         nbrChanges++;
   
   if ( nbrChanges > 0 || removed() || m_id == 0 ) {
      p->incWriteLong( position, 
                       UserConstants::TYPE_PIN );
      p->incWriteLong( position, m_id );
      p->incWriteLong( position, 
                       UserConstants::TYPE_PIN );
      p->incWriteLong( position, m_id );
      if ( removed() ) {
         p->incWriteByte( position, UserConstants::ACTION_DELETE );
      } else if ( m_id == 0 ) {
         p->incWriteByte(position, UserConstants::ACTION_NEW);
         // Not id and not userUIN
         // Nbr changed
         p->incWriteByte( 
            position, UserConstants::USER_PIN_NBRFIELDS -2 );
         p->incWriteByte( position, UserConstants::USER_PIN_PIN );
         p->incWriteString( position, m_PIN.c_str() );
         p->incWriteByte( position, UserConstants::USER_PIN_COMMENT );
         p->incWriteString( position, m_comment.c_str() );
      } else {
         p->incWriteByte( position, UserConstants::ACTION_CHANGE );
         p->incWriteByte( position, nbrChanges );
         
         if ( m_changed[ UserConstants::USER_PIN_PIN ] ) {
            p->incWriteByte( position, UserConstants::USER_PIN_PIN );
            p->incWriteString( position, m_PIN.c_str() );
         }
         if ( m_changed[ UserConstants::USER_PIN_COMMENT ] ) {
            p->incWriteByte( position, UserConstants::USER_PIN_COMMENT );
            p->incWriteString( position, m_comment.c_str() );
         }
         
      } // End else changed

      p->incWriteLong( position, UserConstants::TYPE_PIN );
      p->incWriteLong( position, UserConstants::TYPE_PIN );
      for ( uint8 i = 0 ; i < UserConstants::USER_PIN_NBRFIELDS ; ++i ) {
         m_changed[ i ] = false;
      }
      p->setLength( position );
   } else {
      mc2log << warn << "UserPIN::addChanges but there "
             << "is no changes to save?" << endl;
   }
}


bool
UserPIN::readChanges( const Packet* p, int& pos, 
                        UserConstants::UserAction& action )
{
   if ( p->getLength() - pos - 5 > 0 ) {
      uint32 type;
      uint32 id;
      
      type = p->incReadLong( pos );
      id = p->incReadLong( pos );
      if ( type == UserConstants::TYPE_PIN && id == m_id ) {
         action = UserConstants::UserAction( p->incReadByte( pos ) );
         switch ( action ) {
            case UserConstants::ACTION_NEW:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_DELETE:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_PIN ) {
                  remove();
                  return true;
               } else {
                  mc2log << warn
                         << "UserPIN::readChanges not "
                         << "Type PIN after action delete" << endl;
                  return false;
               }
               break;
            case UserConstants::ACTION_CHANGE:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_NOP:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_PIN ) {
                  return true;
               } else {
                  mc2log << warn
                         << "UserPIN::readChanges not "
                         << "Type PIN after action NOP" << endl;
                  return false;
               }
               break;
            default:
               mc2log << warn
                      << "UserPIN::readChanges unknown "
                      << "action: " << (int)action << endl;
         }
         return false;
      } else {
         mc2log << error
                << "UserPIN::readChanges not correct "
                << "type or not correct id, giving up." << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserPIN::readChanges no space for "
             << "changes giving up." << endl;
      return false;
   }
}


bool
UserPIN::readDataChanges( const Packet* p, int& pos ) {
   if ( p->getLength() - pos - 1 > 0 ) {
      byte nbrFields;

      nbrFields = p->incReadByte( pos );

      // tmpdata
      MC2String PIN = m_PIN;
      MC2String comment = m_comment;

      byte field;
      bool ok = true;
      for ( uint8 i = 0 ; 
            (i < nbrFields) && ok && (pos < (int32)p->getLength()) ; ++i ) 
      {
         field = p->incReadByte( pos );
         switch ( field ) {
            case UserConstants::USER_PIN_PIN :
               m_changed[ UserConstants::USER_PIN_PIN ] = true;
               PIN = p->incReadString( pos );
               break;
            case UserConstants::USER_PIN_COMMENT :
               m_changed[ UserConstants::USER_PIN_COMMENT ] = true;
               comment = p->incReadString( pos );
               break;

            default:
               mc2log << error
                      << "UserPIN::readDataChanges "
                      << "unknown fieldType: " << (int)field << endl;
               ok = false;
               break;
         }
      }

      uint32 packetType = p->incReadLong( pos );
      if ( packetType != UserConstants::TYPE_PIN ) {
         mc2log << error
                << "UserPIN::readDataChanges Not "
                << "PIN type after data" << endl;
         ok = false;
      }
      if ( ok ) {
         // Transfer changes to membervariables
         if ( m_changed[ UserConstants::USER_PIN_PIN ] ) {
            m_PIN = PIN;
         }
         if ( m_changed[ UserConstants::USER_PIN_COMMENT ] ) {
            m_comment = comment;
         }

         return ok;
      } else {
         mc2log << error << "UserPIN::readDataChanges failed" << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserPIN::readDataChanges no space for changes" << endl;
      return false;
   }
}


void
UserPIN::readFromPacket( const Packet* p, int& pos ) {
   uint32 type = p->incReadLong( pos );

   if ( type == UserConstants::TYPE_PIN ) {
      uint32 size = p->incReadShort( pos );
      
      if ( (getSize() - 6) <= size ) {
         m_id = p->incReadLong( pos );
         m_PIN = p->incReadString( pos );
         m_comment = p->incReadString( pos );

         type = p->incReadLong( pos );
         if ( type != UserConstants::TYPE_PIN ) {
            mc2log << warn
                   << "UserPIN::readFromPacket( "
                   << "Packet* p, int& pos )"
                   << "Data not ended by TYPE_PIN!!!"
                   << endl;
            return;
         }
         // DONE
      } else {
         mc2log << warn  
                << "UserPIN::readFromPacket"
                << "(Packet* p, int& pos)"
                << " Not size for data!! Nothing read" << endl;
         return;
      }
   } else {
      mc2log << warn 
             << "UserPIN::readFromPacket"
             << "( Packet* p, int& pos )"
             << "Not TYPE_PIN type!!!!!" << endl;
      return;
   }
   
   setOk( true );

   for ( uint8 i = 0 ; i < UserConstants::USER_PIN_NBRFIELDS ; ++i ) {
      m_changed[ i ] = false;
   }
}


uint32
UserPIN::getNbrChanged() const {
   uint32 nbrChanged = 0;
   for ( uint8 i = 0 ; i < UserConstants::USER_PIN_NBRFIELDS ; ++i ) {
      if ( m_changed[ i ] ) {
         nbrChanged++;
      }
   }
   return nbrChanged;
}


uint32
UserPIN::printValue( char* target, UserConstants::UserPINField field )
   const
{
   switch ( field ) {
      case UserConstants::USER_PIN_ID :
      case UserConstants::USER_PIN_USERUIN :
         mc2log << error << "UserPIN::printValue asked "
                << "to print ID or userUIN, not supported." << endl;
         MC2_ASSERT( false );
         break;
      case UserConstants::USER_PIN_PIN :
         strcat( target, "'" );
         sqlString( target + 1, getPIN() );
         strcat( target, "'" );
         break;
      case UserConstants::USER_PIN_COMMENT :
         strcat( target, "'" );
         sqlString( target + 1, getComment() );
         strcat( target, "'" );
         break;

      default:
         mc2log << error << "UserPIN::printValue unknown"
                << " fieldType: " << (int)field << endl;
         break;
   }

   return strlen( target );
}


bool
UserPIN::changed( 
   UserConstants::UserPINField field ) const
{
   return m_changed[ field ];
}


bool
UserPIN::isChanged() const {
   return getNbrChanged() > 0 || m_id == 0 || removed();
}


// **********************************************************************
// UserLastClient
// **********************************************************************


UserLastClient::UserLastClient( 
   uint32 id, const MC2String& clientType, 
   const MC2String& clientTypeOptions, 
   const MC2String& version, const MC2String& extra,
   const MC2String& origin, bool history,
   uint32 changerUIN, uint32 changeTime )
      : UserElement( UserConstants::TYPE_LAST_CLIENT ),
        m_clientType( clientType ), 
        m_clientTypeOptions( clientTypeOptions ), m_version( version ),
        m_extra( extra ), m_origin( origin ), m_history( history ),
        m_changerUIN( changerUIN ), m_changeTime( changeTime )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0 ; i < UserConstants::USER_LAST_CLIENT_NBRFIELDS ; 
         ++i ) 
   {
      m_changed[ i ] = false;
   }
}


UserLastClient::UserLastClient( uint32 id ) 
      : UserElement( UserConstants::TYPE_LAST_CLIENT )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0 ; i < UserConstants::USER_LAST_CLIENT_NBRFIELDS ; 
         ++i ) 
   {
      m_changed[ i ] = false;
   }
}


UserLastClient::UserLastClient( const Packet* p, int& pos ) 
      : UserElement( UserConstants::TYPE_LAST_CLIENT ) 
{
   readFromPacket( p, pos );
}


UserLastClient::UserLastClient( const UserLastClient& user ) 
      : UserElement( UserConstants::TYPE_LAST_CLIENT )
{
   m_id = user.m_id;
   m_clientType = user.m_clientType;
   m_clientTypeOptions = user.m_clientTypeOptions;
   m_version = user.m_version;
   m_extra = user.m_extra;
   m_origin = user.m_origin;
   m_history = user.m_history;
   m_changerUIN = user.m_changerUIN;
   m_changeTime = user.m_changeTime;

   for ( uint8 i = 0 ; i < UserConstants::USER_LAST_CLIENT_NBRFIELDS ; 
         ++i ) 
   {
      m_changed[ i ] = user.m_changed[ i ];
   }
}


UserLastClient::~UserLastClient() {
   // Nothing newed in constructor (yet)
}


uint32
UserLastClient::getSize() const {
   return 16 + m_clientType.size() + 1 + m_clientTypeOptions.size() + 1 
      + m_version.size() + 1 + m_extra.size() + 1 + m_origin.size() + 1 
      + 3*4 + 2*4 + 3;
}


void
UserLastClient::packInto( Packet* p, int& pos ) {
   uint32 size = getSize();

   p->incWriteLong( pos, UserConstants::TYPE_LAST_CLIENT );
   p->incWriteShort( pos, size );
   p->incWriteLong( pos, UserConstants::TYPE_LAST_CLIENT );
   p->incWriteShort( pos, size );

   p->incWriteLong( pos, m_id );
   p->incWriteString( pos, m_clientType.c_str() );
   p->incWriteString( pos, m_clientTypeOptions.c_str() );
   p->incWriteString( pos, m_version.c_str() );
   p->incWriteString( pos, m_extra.c_str() );
   p->incWriteString( pos, m_origin.c_str() );
   p->incWriteByte( pos, m_history );
   p->incWriteLong( pos, m_changerUIN );
   p->incWriteLong( pos, m_changeTime );

   p->incWriteLong( pos, UserConstants::TYPE_LAST_CLIENT );
   p->incWriteLong( pos, UserConstants::TYPE_LAST_CLIENT );

   // Set length 
   p->setLength( pos );
}


void
UserLastClient::addChanges( Packet* p, int& position ) {
   uint8 nbrChanges = 0;
   for ( uint8 i = 0 ; i < UserConstants::USER_LAST_CLIENT_NBRFIELDS ; 
         ++i ) 
   {
      if ( m_changed[ i ] ) {
         nbrChanges++;
      }
   }
   
   if ( nbrChanges > 0 || removed() || m_id == 0 ) {
      p->incWriteLong( position, UserConstants::TYPE_LAST_CLIENT );
      p->incWriteLong( position, m_id );
      p->incWriteLong( position, UserConstants::TYPE_LAST_CLIENT );
      p->incWriteLong( position, m_id );
      if ( removed() ) {
         p->incWriteByte( position, UserConstants::ACTION_DELETE );
      } else if ( m_id == 0 ) {
         p->incWriteByte(position, UserConstants::ACTION_NEW);
         // Not id and not userUIN
         // Nbr changed
         p->incWriteByte( 
            position, UserConstants::USER_LAST_CLIENT_NBRFIELDS -2 );
         p->incWriteByte( position, 
                          UserConstants::USER_LAST_CLIENT_CLIENT_TYPE );
         p->incWriteString( position, m_clientType.c_str() );
         p->incWriteByte( 
            position, 
            UserConstants::USER_LAST_CLIENT_CLIENT_TYPE_OPTIONS );
         p->incWriteString( position, m_clientTypeOptions.c_str() );
         p->incWriteByte( position, 
                          UserConstants::USER_LAST_CLIENT_VERSION );
         p->incWriteString( position, m_version.c_str() );
         p->incWriteByte( position, 
                          UserConstants::USER_LAST_CLIENT_EXTRA );
         p->incWriteString( position, m_extra.c_str() );
         p->incWriteByte( position, 
                          UserConstants::USER_LAST_CLIENT_ORIGIN );
         p->incWriteString( position, m_origin.c_str() );
      } else {
         p->incWriteByte( position, UserConstants::ACTION_CHANGE );
         p->incWriteByte( position, nbrChanges );
         
         if ( m_changed[ UserConstants::USER_LAST_CLIENT_CLIENT_TYPE ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_LAST_CLIENT_CLIENT_TYPE );
            p->incWriteString( position, m_clientType.c_str() );
         }
         if ( m_changed[ 
                 UserConstants::USER_LAST_CLIENT_CLIENT_TYPE_OPTIONS ] ) 
         {
            p->incWriteByte( 
               position, 
               UserConstants::USER_LAST_CLIENT_CLIENT_TYPE_OPTIONS );
            p->incWriteString( position, m_clientTypeOptions.c_str() );
         }
         if ( m_changed[ UserConstants::USER_LAST_CLIENT_VERSION ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_LAST_CLIENT_VERSION );
            p->incWriteString( position, m_version.c_str() );
         }
         if ( m_changed[ UserConstants::USER_LAST_CLIENT_EXTRA ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_LAST_CLIENT_EXTRA );
            p->incWriteString( position, m_extra.c_str() );
         }
         if ( m_changed[ UserConstants::USER_LAST_CLIENT_ORIGIN ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_LAST_CLIENT_ORIGIN );
            p->incWriteString( position, m_origin.c_str() );
         }
      } // End else changed

      p->incWriteLong( position, UserConstants::TYPE_LAST_CLIENT );
      p->incWriteLong( position, UserConstants::TYPE_LAST_CLIENT );
      for ( uint8 i = 0 ; i < UserConstants::USER_LAST_CLIENT_NBRFIELDS ; 
            ++i )
      {
         m_changed[ i ] = false;
      }
      p->setLength( position );
   } else {
      mc2log << warn << "UserLastClient::addChanges but there "
             << "is no changes to save?" << endl;
   }
}


bool
UserLastClient::readChanges( const Packet* p, int& pos, 
                        UserConstants::UserAction& action )
{
   if ( p->getLength() - pos - 5 > 0 ) {
      uint32 type;
      uint32 id;
      
      type = p->incReadLong( pos );
      id = p->incReadLong( pos );
      if ( type == UserConstants::TYPE_LAST_CLIENT && id == m_id ) {
         action = UserConstants::UserAction( p->incReadByte( pos ) );
         switch ( action ) {
            case UserConstants::ACTION_NEW:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_DELETE:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_LAST_CLIENT ) {
                  remove();
                  return true;
               } else {
                  mc2log << warn
                         << "UserLastClient::readChanges not "
                         << "Type LAST_CLIENT after action delete" << endl;
                  return false;
               }
               break;
            case UserConstants::ACTION_CHANGE:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_NOP:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_LAST_CLIENT ) {
                  return true;
               } else {
                  mc2log << warn
                         << "UserLastClient::readChanges not "
                         << "Type LAST_CLIENT after action NOP" << endl;
                  return false;
               }
               break;
            default:
               mc2log << warn
                      << "UserLastClient::readChanges unknown "
                      << "action: " << (int)action << endl;
         }
         return false;
      } else {
         mc2log << error
                << "UserLastClient::readChanges not correct "
                << "type or not correct id, giving up." << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserLastClient::readChanges no space for "
             << "changes giving up." << endl;
      return false;
   }
}


bool
UserLastClient::readDataChanges( const Packet* p, int& pos ) {
   if ( p->getLength() - pos - 1 > 0 ) {
      byte nbrFields;

      nbrFields = p->incReadByte( pos );

      // tmpdata
      MC2String clientType = m_clientType;
      MC2String clientTypeOptions = m_clientTypeOptions;
      MC2String version = m_version;
      MC2String extra = m_extra;
      MC2String origin = m_origin;

      byte field;
      bool ok = true;
      for ( uint8 i = 0 ; 
            (i < nbrFields) && ok && (pos < (int32)p->getLength()) ; ++i ) 
      {
         field = p->incReadByte( pos );
         switch ( field ) {
            case UserConstants::USER_LAST_CLIENT_CLIENT_TYPE :
               m_changed[ UserConstants::USER_LAST_CLIENT_CLIENT_TYPE ] = 
                  true;
               clientType = p->incReadString( pos );
               break;
            case UserConstants::USER_LAST_CLIENT_CLIENT_TYPE_OPTIONS :
               m_changed[ 
                  UserConstants::USER_LAST_CLIENT_CLIENT_TYPE_OPTIONS ] = 
                  true;
               clientTypeOptions = p->incReadString( pos );
               break;
            case UserConstants::USER_LAST_CLIENT_VERSION :
               m_changed[ UserConstants::USER_LAST_CLIENT_VERSION ] = true;
               version = p->incReadString( pos );
               break;
            case UserConstants::USER_LAST_CLIENT_EXTRA :
               m_changed[ UserConstants::USER_LAST_CLIENT_EXTRA ] = true;
               extra = p->incReadString( pos );
               break;
            case UserConstants::USER_LAST_CLIENT_ORIGIN :
               m_changed[ UserConstants::USER_LAST_CLIENT_ORIGIN ] = true;
               origin = p->incReadString( pos );
               break;

            default:
               mc2log << error
                      << "UserLastClient::readDataChanges "
                      << "unknown fieldType: " << (int)field << endl;
               ok = false;
               break;
         }
      }

      uint32 packetType = p->incReadLong( pos );
      if ( packetType != UserConstants::TYPE_LAST_CLIENT ) {
         mc2log << error
                << "UserLastClient::readDataChanges Not "
                << "LAST_CLIENT type after data" << endl;
         ok = false;
      }
      if ( ok ) {
         // Transfer changes to membervariables
         if ( m_changed[ UserConstants::USER_LAST_CLIENT_CLIENT_TYPE ] ) {
            m_clientType = clientType;
         }
         if ( m_changed[ 
                 UserConstants::USER_LAST_CLIENT_CLIENT_TYPE_OPTIONS ] ) 
         {
            m_clientTypeOptions = clientTypeOptions;
         }
         if ( m_changed[ UserConstants::USER_LAST_CLIENT_VERSION ] ) {
            m_version = version;
         }
         if ( m_changed[ UserConstants::USER_LAST_CLIENT_EXTRA ] ) {
            m_extra = extra;
         }
         if ( m_changed[ UserConstants::USER_LAST_CLIENT_ORIGIN ] ) {
            m_origin = origin;
         }

         return ok;
      } else {
         mc2log << error << "UserLastClient::readDataChanges failed"
                << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserLastClient::readDataChanges no space for changes" 
             << endl;
      return false;
   }
}


void
UserLastClient::readFromPacket( const Packet* p, int& pos ) {
   uint32 type = p->incReadLong( pos );

   if ( type == UserConstants::TYPE_LAST_CLIENT ) {
      uint32 size = p->incReadShort( pos );
      
      if ( (getSize() - 6) <= size ) {
         m_id     = p->incReadLong( pos );
         m_clientType = p->incReadString( pos );
         m_clientTypeOptions = p->incReadString( pos );
         m_version = p->incReadString( pos );
         m_extra = p->incReadString( pos );
         m_origin = p->incReadString( pos );
         m_history = p->incReadByte( pos );
         m_changerUIN = p->incReadLong( pos );
         m_changeTime = p->incReadLong( pos );

         type = p->incReadLong( pos );
         if ( type != UserConstants::TYPE_LAST_CLIENT ) {
            mc2log << warn
                   << "UserLastClient::readFromPacket( "
                   << "Packet* p, int& pos )"
                   << "Data not ended by TYPE_LAST_CLIENT!!!"
                   << endl;
            return;
         }
         // DONE
      } else {
         mc2log << warn  
                << "UserLastClient::readFromPacket"
                << "(Packet* p, int& pos)"
                << " Not size for data!! Nothing read" << endl;
         return;
      }
   } else {
      mc2log << warn 
             << "UserLastClient::readFromPacket"
             << "( Packet* p, int& pos )"
             << "Not TYPE_LAST_CLIENT type!!!!!" << endl;
      return;
   }
   
   setOk( true );

   for ( uint8 i = 0 ; i < UserConstants::USER_LAST_CLIENT_NBRFIELDS ; 
         ++i ) 
   {
      m_changed[ i ] = false;
   }
}


void
UserLastClient::setClientType( const MC2String& clientType ) {
   m_clientType = clientType;
   m_changed[ UserConstants::USER_LAST_CLIENT_CLIENT_TYPE ] = true;
}


void
UserLastClient::setClientTypeOptions( const MC2String& clientTypeOptions )
{
   m_clientTypeOptions = clientTypeOptions;
   m_changed[ UserConstants::USER_LAST_CLIENT_CLIENT_TYPE_OPTIONS ] = true;
}


void
UserLastClient::setVersion( const MC2String& version ) {
   m_version = version;
   m_changed[ UserConstants::USER_LAST_CLIENT_VERSION ] = true;
}


void
UserLastClient::setExtra( const MC2String& extra ) {
   m_extra = extra;
   m_changed[ UserConstants::USER_LAST_CLIENT_EXTRA ] = true;
}


void
UserLastClient::setOrigin( const MC2String& origin ) {
   m_origin = origin;
   m_changed[ UserConstants::USER_LAST_CLIENT_ORIGIN ] = true;
}


uint32
UserLastClient::getNbrChanged() const {
   uint32 nbrChanged = 0;
   for ( uint8 i = 0 ; i < UserConstants::USER_LAST_CLIENT_NBRFIELDS ; 
         ++i ) 
   {
      if ( m_changed[ i ] ) {
         nbrChanged++;
      }
   }
   return nbrChanged;
}


uint32
UserLastClient::printValue( char* target, 
                            UserConstants::UserLastClientField field )
   const
{
   switch ( field ) {
      case UserConstants::USER_LAST_CLIENT_ID :
      case UserConstants::USER_LAST_CLIENT_USERUIN :
         mc2log << error << "UserLastClient::printValue asked "
                << "to print ID or userUIN, not supported." << endl;
         MC2_ASSERT( false );
         break;
      case UserConstants::USER_LAST_CLIENT_CLIENT_TYPE :
         strcat( target, "'" );
         sqlString( target + 1, getClientType().c_str() );
         strcat( target, "'" );
         break;
      case UserConstants::USER_LAST_CLIENT_CLIENT_TYPE_OPTIONS :
         strcat( target, "'" );
         sqlString( target + 1, getClientTypeOptions().c_str() );
         strcat( target, "'" );
         break;
      case UserConstants::USER_LAST_CLIENT_VERSION :
         strcat( target, "'" );
         sqlString( target + 1, getVersion().c_str() );
         strcat( target, "'" );
         break;
      case UserConstants::USER_LAST_CLIENT_EXTRA :
         strcat( target, "'" );
         sqlString( target + 1, getExtra().c_str() );
         strcat( target, "'" );
         break;
      case UserConstants::USER_LAST_CLIENT_ORIGIN :
         strcat( target, "'" );
         sqlString( target + 1, getOrigin().c_str() );
         strcat( target, "'" );
         break;

      default:
         mc2log << error << "UserLastClient::printValue unknown"
                << " fieldType: " << (int)field << endl;
         break;
   }

   return strlen( target );
}


bool
UserLastClient::changed( UserConstants::UserLastClientField field ) const {
   return m_changed[ field ];
}


bool
UserLastClient::isChanged() const {
   return getNbrChanged() > 0 || m_id == 0 || removed();
}
