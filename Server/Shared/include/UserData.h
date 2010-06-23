/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERDATA_H
#define USERDATA_H

#include "config.h"
#include "StringTable.h"
#include "CacheElement.h"
#include "UserConstants.h"
#include "ItemTypes.h"
#include "Vector.h"
#include "DBaseObject.h"
#include "UserEnums.h"
#include "MapRights.h"
#include "UserElement.h"

#include <map>
#include <vector>

// Forward declaration
class UserRequestPacket;
class UserReplyPacket;
class GetUserDataReplyPacket;

// This file contains the following classes:
class UserItem; 
class UserSubscription;
class UserUser;
class UserCellular;
class CellularModels;
class CellularPhoneModel;
class UserLicenceKey;
class RegionIDs;
class UserRight;


/**
 *    Class holding information about a user. 
 *
 */
class UserItem : public CacheElement
{
  public:
   /**
    * Constructor, without any valid data
    */
   UserItem(uint32 UIN);


   /**
    * Constructor, extracts the information from the UserReplyPacket.
    */
   UserItem( const UserReplyPacket* p );


   /**
    * Constructor, creates a new UserItem with userUser as UserUser.
    */
   UserItem( UserUser* userUser );

   /**
    *    Copy-constructor.
    */
   UserItem( const UserItem& userItem );

   /**
    * Destructor, removes all elements and allocated objects.
    */
   virtual ~UserItem();


   /**
    * Decodes the packet and extracts the information.
    */
   bool decodePacket( GetUserDataReplyPacket* p );


   /**
    *  Returns the size of the allocated variables and elements.
    */
   virtual uint32 getSize() const;


   /**
    * Returns the UIN
    */
   uint32 getUIN() const;


   /**
    * The useruser
    */
   UserUser* getUser() { return m_userUser; }


   /**
    * The useruser
    */
   const UserUser* getUser() const { return m_userUser; }


   /**
    *  Removes the user. Marks the user for deletion. 
    */
   void remove();


   /**
    *  Checks if the item is removed. If its marked for deletetion.
    */
   bool removed() const;


   /**
    * If the UserItem was initialzed properly
    */
   bool getValid() {
      return m_valid;
   }


   /**
    * Get the timestamp of this copy of user.
    */
   uint32 getTimeStamp() const;


  private:
   /// If the User is to be removed from the UserDatabase.
   bool m_removed;

   
   /// If UserUser has been loaded for user.
   bool m_valid;


   /// The UserUser of this UserItem
   UserUser* m_userUser;


   /// The time of creation of this copy of user.
   uint32 m_timeStamp;
};



// ========================================================================
//                                  Implementation of the inlined methods =

inline uint32 
UserItem::getTimeStamp() const {
   return m_timeStamp;
}



/**
 *  Contains data about a User
 *
 *
 *   When packed into or from a packet UserUser looks like this.
 *   The first TYPE_USER and Total size in bytes and the last 
 *   TYPE_USER is read by someone else when read from a packet.
 *   \begin{tabular}{lll}
 *      Pos                         & Size     & Destription \\ \hline
 *      pos                         & 1 bytes  & TYPE_USER \\
 *      +1                          & 2 bytes  & Total size in bytes \\
 *      +3                          & 1 bytes  & TYPE_USER \\
 *      +4                          & 2 bytes  & Total size in bytes \\
 *      +6                          & 4 bytes  & UIN \\
 *      +10                         & 4 bytes  & measurementSystem \\
 *      +14                         & 4 bytes  & language \\
 *      +18                         & 4 bytes  & lastdest_mapID \\
 *      +22                         & 4 bytes  & lastdest_itemID \\
 *      +26                         & 2 bytes  & lastdest_offset \\
 *      +28                         & 4 bytes  & lastdest_time \\
 *      +32                         & 4 bytes  & lastorig_mapID \\
 *      +36                         & 4 bytes  & lastorig_itemID \\
 *      +38                         & 2 bytes  & lastorig_offset \\
 *      +40                         & 4 bytes  & lastorig_time \\
 *      +44                         & 4 bytes  & searchObject \\
 *      +48                         & 1 bytes  & searchType \\
 *      +49                         & 1 bytes  & searchSubstring \\
 *      +50                         & 1 bytes  & searchSorting \\
 *      +51                         & 1 bytes  & database bitmask \\
 *      +52                         & 1 bytes  & routingCostA \\
 *      +53                         & 1 bytes  & routingCostB \\
 *      +54                         & 1 bytes  & routingCostC \\
 *      +55                         & 1 bytes  & routingCostD \\
 *      +56                         & 4 bytes  & routingVehicle
 *      +60                         & 1 bytes  & routingType \\
 *                                  & 4 bytes  & gender \\
 *                                  & 4 bytes  & validDate \\
 *                                  & 4 bytes  & editMapRights \\
 *                                  & 4 bytes  & editDelayRights \\
 *                                  & 4 bytes  & editUserRights \\
 *                                  & 1 bytes  & wapService \\
 *                                  & 1 bytes  & htmlService \\
 *                                  & 1 bytes  & operatorService \\
 *                                  & 1 bytes  & smsService \\
 *                                  & 4 bytes  & nbrMunicipal \\
 *      for nbrMunicipal            & 4 bytes  & municipal \\
 *                                  & 2 bytes  & strLen \\
 *                                  & string   & logonID \\
 *                                  & 2 bytes  & strLen \\
 *                                  & string   & firstname \\
 *                                  & 2 bytes  & strLen \\
 *                                  & string   & initials \\
 *                                  & 2 bytes  & strLen \\
 *                                  & string   & lastname \\
 *                                  & 2 bytes  & strLen \\
 *                                  & string   & session \\
 *                                  & 2 bytes  & strLen \\
 *                                  & string   & lastdest_string \\
 *                                  & 2 bytes  & strLen \\
 *                                  & string   & lastorig_string \\
 *                                  & 2 bytes  & strLen \\
 *                                  & string   & birthDate \\
 *                                  & 2 bytes  & strLen \\
 *                                  & string   & default_country \\
 *                                  & 2 bytes  & strLen \\
 *                                  & string   & default_municipal \\
 *                                  & 2 bytes  & strLen \\
 *                                  & string   & default_city \\
 *                                  & 1 bytes  & navService \\
 *                                  & 2 bytes  & strlen \\
 *                                  & string   & operatorComment \\
 *                                  & 2 bytes  & strlen \\
 *                                  & string   & emailAddress \\
 *                                  & 2 bytes  & strlen \\
 *                                  & string   & address1 \\
 *                                  & 2 bytes  & strlen \\
 *                                  & string   & address2 \\
 *                                  & 2 bytes  & strlen \\
 *                                  & string   & address3 \\
 *                                  & 2 bytes  & strlen \\
 *                                  & string   & address4 \\
 *                                  & 2 bytes  & strlen \\
 *                                  & string   & address5 \\
 *                                  & 1 bytes  & routeTurnImageType \\
 *                                  & 1 bytes  & externalXmlService \\
 *                                  & 1 bytes  & transactionBased \\
 *                                  & 4 bytes  & deviceChanges \\
 *                                  & string   & supportComment \\
 *                                  & string   & postalCity \\
 *                                  & string   & zipCode \\
 *                                  & string   & companyName \\
 *                                  & string   & companyReference \\
 *                                  & string   & companyVATNbr \\
 *                                  & 4 bytes  & emailBounces \\
 *                                  & 4 bytes  & addressBounces\\
 *                                  & string   & customerContactInfo \\
 *     last-1                       & 1 bytes  & TYPE_USER \\
 *     last                         & 1 bytes  & TYPE_USER \\
 *   \end{tabular}
 *
 */
class UserUser : public UserElement
{
  public:
   /// Sets all variables to zero or suitable defaults.
   UserUser( uint32 UIN );

   
   /// Sets all variables
   UserUser( uint32 UIN,
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
             const char* default_country ,
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
             const char* customerContactInfo );


   /// Sets all variables according to parameters in packet.
   UserUser( const UserReplyPacket* p, int& pos );

   /// Copy constructor copies everything.
   UserUser( const UserUser& user );
  

   /// Deletes all allocated variables
   virtual ~UserUser();


   /// Returns the size allocated by the variables
   uint32 getSize() const;


   /// Puts the UserUser into p
   virtual void packInto( Packet* p, int& pos );


   /// Adds all changes in the object to the packet
   virtual void addChanges(Packet* p, int& position);


   /// Reads changes from a packet.
   virtual bool readChanges( const Packet* p, int& pos, 
                             UserConstants::UserAction& action );


   /// Returns the UIN of the User
   uint32 getUIN() const;
   
   /// The loginname of the user
   const char* getLogonID() const;
   /// Copies the string and sets the field as changed
   void setLogonID(const char* logonID);
   /// First name
   const char* getFirstname() const;
   /// Copies the string
   void setFirstname(const char* firstname);
   /// Initials
   const char* getInitials() const;
   /// Copies the string
   void setInitials(const char* initials);
   /// Last name
   const char* getLastname() const;
   /// Copies the string
   void setLastname(const char* lastname);
   /// Session key
   const char* getSession() const;
   /// Copies the string
   void setSession(const char* session);
   /// Length type
   UserConstants::MeasurementType getMeasurementSystem() const;
   /// New length type
   void setMeasurementSystem(
      UserConstants::MeasurementType measurementSystem);
   /// Langugage
   StringTable::languageCode getLanguage() const;
   /// New langugare sets changed
   void setLanguage(StringTable::languageCode language);

   /// Get Lastdest mapID
   uint32 getLastdest_mapID() const;
   /// Set Lastdest mapID
   void setLastdest_mapID(uint32 lastdest_mapID);
   /// Get Lastdest itemID
   uint32 getLastdest_itemID() const;
   /// Set Lastdest itemID
   void setLastdest_itemID(uint32 lastdest_itemID);
   /// Get Lastdest offset
   uint32 getLastdest_offset() const;
   /// Set Lastdest offset
   void setLastdest_offset(uint32 lastdest_offset);
   /// Get Lastdest time
   uint32 getLastdest_time() const;
   /// Set Lastdest time
   void setLastdest_time(uint32 lastdest_time);
   /// Get the brand of the user.
   const char* getBrand() const;
   /// Set the brand of the user.
   void setBrand( const char* str );

   /// The birthnumber of the user
   const char* getBirthDate() const;
   /// Copies the string and sets the field as changed
   void setBirthDate(const char* logonID);
   
   /// Get Lastorig mapID
   uint32 getLastorig_mapID() const;
   /// Set Lastorig mapID
   void setLastorig_mapID(uint32 lastorig_mapID);
   /// Get Lastorig itemID
   uint32 getLastorig_itemID() const;
   /// Set Lastorig itemID
   void setLastorig_itemID(uint32 lastorig_itemID);
   /// Get Lastorig offset
   uint32 getLastorig_offset() const;
   /// Set Lastorig offset
   void setLastorig_offset(uint32 lastorig_offset);
   /// Get Lastorig time
   uint32 getLastorig_time() const;
   /// Set Lastorig time
   void setLastorig_time(uint32 lastorig_time);
   /// Get the original brand.
   const char* getBrandOrigin() const;
   /// Set the original brand.
   void setBrandOrigin( const char* str );

   /// Get Routing costA
   uint8 getRouting_costA() const;
   /// Set Routing costA
   void setRouting_costA( uint8 routingCostA );
   /// Get Routing costB
   uint8 getRouting_costB() const;
   /// Set Routing costB
   void setRouting_costB( uint8 routingCostB );
   /// Get Routing costC
   uint8 getRouting_costC() const;
   /// Set Routing costC
   void setRouting_costC( uint8 routingCostC);
   /// Get Routing costD
   uint8 getRouting_costD() const;
   /// Set Routing costD
   void setRouting_costD( uint8 routingCostD);
   /// Get Routing Type
   uint8 getRouting_type() const;
   /// Set Routing Type
   void setRouting_type( uint8 routingType);
   /// Get Routing Vechicle Type
   ItemTypes::vehicle_t getRouting_vehicle() const;
   /// Set Routing Type
   void setRouting_vehicle( ItemTypes::vehicle_t routing_vechicle );

   /// Get RouteImageType
   UserConstants::RouteImageType getRouteImageType() const;
   /// Set RouteImageType
   void setRouteImageType( UserConstants::RouteImageType routeImageType );

   /// Get gender
   UserConstants::GenderType getGender() const;
   /// Set gender
   void setGender( UserConstants::GenderType gender );

   /// Get valid date
   uint32 getValidDate() const;
   /// Set valid date
   void setValidDate( uint32 validDate );

   /// Get Search type
   uint8 getSearch_type() const;
   /// Set Search type
   void setSearch_type(uint8 newSearch_type);
   /// Get Search substring (Stringpart)
   uint8 getSearch_substring() const;
   /// Set Search Substring (Stringpart)
   void setSearch_substring(uint8 newSearch_substring);
   /// Get Search sorting
   uint8 getSearch_sorting() const;
   /// Set Search sorting
   void setSearch_sorting(uint8 newSearch_sorting);
   /// Get the types to search for in ordinary matches.
   uint32 getSearchForTypes() const;
   /// Set the types to search for in ordinary matches.
   void setSearchForTypes( uint32 searchTypes );
   /// Get the types to search for in overview matches.
   uint32 getSearchForLocationTypes() const;
   /// Set the types to search for in overview matches.
   void setSearchForLocationTypes( uint32 searchTypes );
   /// Get search dbMask
   uint8 getSearch_DbMask() const;
   /// Set search dbMask
   void setSearch_DbMask(uint8 newDBMask);
   
   /// Get EditMapRights
   uint32 getEditMapRights() const;
   /// Set EditMapRights
   void setEditMapRights( uint32 editMapRights );
   /// Get EditDelayRights
   uint32 getEditDelayRights() const;
   /// Set EditDelayRights
   void setEditDelayRights( uint32 editDelayRights );
   /// Get EditUserRights
   uint32 getEditUserRights() const;
   /// Set EditUserRights
   void setEditUserRights( uint32 editUserRights );


   /// Returns wheather User has rights to use WAP
   bool getWAPService() const;
   /// Sets wheather User has rights to use WAP
   void setWAPService(bool wapService);


   /// Returns wheather User has rights to use HTML
   bool getHTMLService() const;
   /// Sets wheather User has rights to use WAP
   void setHTMLService(bool htmlService);


   /// Returns wheather User has rights to use Operatorinterface
   bool getOperatorService() const;
   /// Sets wheather User has rights to use Operatorinterface
   void setOperatorService(bool operatorService);


    /// Returns wheather User has rights to use SMS
   bool getSMSService() const;
   /// Sets wheather User has rights to use SMS
   void setSMSService(bool smsService);

   /// Returns the users default country to search in.
   const char* getDefaultCountry() const;
   /// Sets the country where the users usually looks for destinations.
   void setDefaultCountry( const char* default_country );

   /// Returns the users default municipal to search in.
   const char* getDefaultMunicipal() const;
   /// Sets the municipal where the users usually looks for destinations.
   void setDefaultMunicipal( const char* default_municipal );

   /// Returns the users default city to search in.
   const char* getDefaultCity() const;
   /// Sets the country where the users usually looks for destinations.
   void setDefaultCity( const char* default_city );
   
   /// Get NumMunicipal
   uint32 getNumMunicipal() const;
   /// Get Municipal
   uint32 getMunicipal(uint32 index) const;
   /// Set Municipals
   void setMunicipal( uint32* municipal, uint32 nbrMunicipal );

   /// Sets the UIN
   void setUIN( uint32 UIN ) {
     m_UIN = UIN;
   }

   /**
    * Set the nav service.
    */
   void setNavService( bool value );

   /**
    * Get the nav service.
    */
   bool getNavService() const;

   /**
    * Set the operator comment, not to show to end user.
    */
   void setOperatorComment( const char* str );

   /**
    * Get the operator comment, not to show to end user.
    */
   const char* getOperatorComment() const;

   /**
    * Set the email address.
    */
   void setEmailAddress( const char* str );

   /**
    * Get the email address.
    */
   const char* getEmailAddress() const;

   /**
    * Set address field 1.
    */
   void setAddress1( const char* str );

   /**
    * Get address field 1.
    */
   const char* getAddress1() const;

   /**
    * Set address field 2.
    */
   void setAddress2( const char* str );

   /**
    * Get address field 2.
    */
   const char* getAddress2() const;

   /**
    * Set address field 3.
    */
   void setAddress3( const char* str );

   /**
    * Get address field 3.
    */
   const char* getAddress3() const;

   /**
    * Set address field 4.
    */
   void setAddress4( const char* str );

   /**
    * Get address field 4.
    */
   const char* getAddress4() const;

   /**
    * Set address field 5.
    */
   void setAddress5( const char* str );

   /**
    * Get address field 5.
    */
   const char* getAddress5() const;

   /**
    * Set the route turn image type
    */
   void setRouteTurnImageType( UserConstants::RouteTurnImageType type );

   /**
    * Get the route turn image type
    */
   UserConstants::RouteTurnImageType  getRouteTurnImageType() const;

   /**
    * Set external xml service.
    */
   void setExternalXmlService( bool value );

   /**
    * Get external xml service.
    */
   bool getExternalXmlService() const;

   /**
    * Set transaction based.
    */
   void setTransactionBased( UserConstants::transactionBased_t value );

   /**
    * Get transaction based.
    */
   UserConstants::transactionBased_t getTransactionBased() const;

   /**
    * Get deviceChanges.
    */
   int32 getDeviceChanges() const;

   /**
    * Set deviceChanges.
    */
   void setDeviceChanges( int32 nbr );

   /**
    * If user is allowed to make a device change.
    */
   bool mayChangeDevice() const;

   /**
    * Use up a device change, may decrease deviceChanges but not if 
    * unlimited.
    *
    * @return True if changed user, false if not.
    */
   bool useDeviceChange();

   /**
    * Get supportComment.
    */
   const char* getSupportComment() const;

   /**
    * Set supportComment.
    */
   void setSupportComment( const char* s );
   

   /**
    * Get postalCity.
    */
   const char* getPostalCity() const;

   /**
    * Set postalCity.
    */
   void setPostalCity( const char* s );
   

   /**
    * Get zipCode.
    */
   const char* getZipCode() const;

   /**
    * Set zipCode.
    */
   void setZipCode( const char* s );
   

   /**
    * Get companyName.
    */
   const char* getCompanyName() const;

   /**
    * Set companyName.
    */
   void setCompanyName( const char* s );
   

   /**
    * Get companyReference.
    */
   const char* getCompanyReference() const;

   /**
    * Set companyReference.
    */
   void setCompanyReference( const char* s );
   

   /**
    * Get companyVATNbr.
    */
   const char* getCompanyVATNbr() const;

   /**
    * Set companyVATNbr.
    */
   void setCompanyVATNbr( const char* s );
   
   /**
    * Get emailBounces.
    */
   int32 getEmailBounces() const;

   /**
    * Set emailBounces.
    */
   void setEmailBounces( int32 n );

   /**
    * Get addressBounces.
    */
   int32 getAddressBounces() const;

   /**
    * Set addressBounces.
    */
   void setAddressBounces( int32 n );

   /**
    * Get customerContactInfo.
    */
   const char* getCustomerContactInfo() const;

   /**
    * Set customerContactInfo.
    */
   void setCustomerContactInfo( const char* s );

   /**
    * Set field in customerContactInfo. Like "opt_in_prod_info", "1".
    */
   void setCustomerContactInfoField( const MC2String& name, 
                                     const MC2String& value );

   /**
    * Set field in customerContactInfo. Like "opt_in_prod_info=1".
    *
    * @param param The string that contains name '=' value.
    * @return True if param is ok and info field was set, false if not.
    */
   bool setCustomerContactInfoField( const MC2String& param );

   /**
    * Get field in customerContactInfo.
    */
   MC2String getCustomerContactInfoField( const MC2String& name ) const;

   /// Get number of elements of a certain type
   uint32 getNbrOfType( UserConstants::UserItemType type ) const;
   /// Get an element of a type
   UserElement* getElementOfType( uint32 index, 
                                  UserConstants::UserItemType type ) const;
   /// Remove element of a type
   void deleteElementOftype( uint32 index,
                             UserConstants::UserItemType type );
   
   /**
    * Get element of a type and with an specific id.
    *
    * @param type The type of the element.
    * @param id   The id of the element.
    */
   UserElement* getElementOfTypeWithID( UserConstants::UserItemType type,
                                        uint32 id ) const;


   /// Add an element
   void addElement( UserElement* elem );

   /// Type of vector stored in the map of vectors
   typedef vector<UserElement*> userElVect_t;
      
   /// Return type of getElementRange
   typedef pair<userElVect_t::iterator, userElVect_t::iterator> userElRange_t;
   
   /// Return type of getElementRange
   typedef pair<userElVect_t::const_iterator,
                userElVect_t::const_iterator> constUserElRange_t;
   
   /**
    *   Returns begin in first and end in second for the specified
    *   element type. Will create an empty vector if no elements
    *   of the type exist.
    */
   userElRange_t getElementRange( UserConstants::UserItemType type );

   /**
    *   Returns begin in first and end in second for the specified
    *   element type.
    */
   constUserElRange_t
      getElementRange( UserConstants::UserItemType type ) const;
   
   /**
    *   Put all elements into the supplied vector.
    */
   uint32 getAllElements( vector<UserElement*>& dest ) const;

   /**
    *   Put all the changed elements into the supplied vector.
    */
   uint32 getAllChangedElements( vector<UserElement*>& dest ) const;

   /// Changed a field?
   bool changed( UserConstants::UserDataField field ) const {
      return m_changed[field];
   }

   
   /// Number changed fields
   uint32 getNbrChanged() const;


   /// A value printed into target length is returned
   uint32 printValue( char* target, 
                      UserConstants::UserDataField field,
                      bool stringInQuotes = true ) const;

   
   /**
    * Returns true if element has changes.
    */ 
   virtual bool isChanged() const;

   /// Type of map where the region rights cache is stored.
   typedef map<uint32, MapRights> regionRightMap_t;   

   /**
    *   Returns the rights needed for a module that uses the map
    *   with id <code>mapID</code>. When it is an overview map
    *   that is not fully covered (0x90000000) it will return all
    *   the rights for now.
    *   @param mapID  The interesting map.
    *   @param rights The rights will be put here by.
    *   @param mask   Only rights matching the mask will be added.
    */
   void getMapRightsForMap( uint32 mapID,
                            regionRightMap_t& rights,
                            const MapRights& mask = ~MapRights()) const;

   /**
    * Returns the rights needed for a specific mapID using 
    * the other getMapRightsForMap and merging the regionRightMap_t
    * into one MapRight.
    *
    * @param mapID The interesting map.
    * @param rights The rights will be put here by.
    * @param mask Only rights matching the mask will be added.
    * @return The MapRight for the mapID.
    */
   MapRights getMapRightsForMap( uint32 mapID,
                                 const MapRights& mask = ~MapRights() ) const;

   /**
    * Creates a MapRights object (set to the correct rights for the
    * user and region) for a specified region id. 
    * @param regionInfo Info about the region groups. 
    * @param regionID   The region id for which rights will be found.
    * @param now        The current time.
    * @return The MapRights for the user in the specified region. 
    */
   MapRights makeMapRights( const RegionIDs& regionInfo,
                            uint32 regionId, 
                            uint32 now ) const;
   /**
    * Merges all valid map rights in all regions and returns the value.
    * @param now the current time
    * @return all maprights that are valid now in all regions
    */
   MapRights getAllMapRights( uint32 now ) const;

   /**
    *   Updates internal cache of user rights per region
    *   @param regionInfo   Info about the region groups.
    *   @param now          The current TimeUtility::getRealTime()
    *   @param topRegionIDs The top region ids from the TopRegionRequest.
    *                       These may not be in the groups.
    */
   void updateRegionRightsCache( const RegionIDs& regionInfo,
                                 uint32 now,
                                 const vector<uint32>& topRegionIDs );

   /**
    *   Returns true if the User has any right matching the mask
    *   in the rights per region.
    *   @param mask Mask to use.
    *   @return True if any bit in the mask matches any bit in the
    *           rights
    */
   bool hasAnyRightIn( const MapRights& mask ) const;
   
   /**
    *   Returns a reference to the region rights cache.
    *   Mainly for use int the ParserThread(Group) when making
    *   the map rights cache.
    */
   const regionRightMap_t& getRegionRightsCache() const;

   /// Type of the map to put the map rights cache into.
   typedef map<uint32, pair<uint32, MapRights> > mapRightMap_t;
   
   /**
    *   Set a new map rights cache by swapping the sent in map
    *   with the old one. For use in ParserThred(Group).
    */
   void swapInMapRightsCache( mapRightMap_t& newRights );

   /**
    *   Returns true if the user contains an element that has changed.
    */
   bool hasChangedElement() const;

   /**
    * Finds the valid versionlockright with the highest value.
    * @return The highest versionlock value for the user.
    */
   uint32 getHighestVersionLock() const;

   /**
    * Searches the users rights to see whether there exists a right
    * that is of the specified URType that is active now and not
    * deleted.
    * @param ur The level and service combo to look for
    * @return True of a valid right was found. 
    */
   bool hasActive(const UserEnums::URType& ur) const;

   /**
    * Get the right matching on type, length and region.
    *
    * @return The, first, matching right, NULL if none.
    */
   UserRight* getMatchingRight( const UserRight* r );

  protected:
   /**
    * Get the total number of elements.
    */
   uint32 getNbrElements() const;
   
   /// Makes URType from old region access. User must not have user rights.
   UserEnums::URType makeURType() const;

   /// Adds a UserElement to the map of vectors
   void addUserElement( UserElement* el );
   
   /// UId, allways set
   uint32 m_UIN;
   /// Default ""
   char* m_logonID;
   /// Default "" 
   char* m_firstname;
   /// Default "" 
   char* m_initials;
   /// Default "" 
   char* m_lastname;
   /// Current session key, default ""
   char* m_session;
   ///  UserConstants::MeasurementType 
   UserConstants::MeasurementType m_measurementSystem;
   /// StringTable::languageCode
   StringTable::languageCode m_language;
   /// Default ""
   char* m_birthDate;

   uint32 m_lastdest_mapID;
   uint32 m_lastdest_itemID;
   uint16 m_lastdest_offset;
   uint32 m_lastdest_time;
   char* m_lastdest_string;
   uint32 m_lastorig_mapID;
   uint32 m_lastorig_itemID;
   uint16 m_lastorig_offset;
   uint32 m_lastorig_time;
   char* m_lastorig_string;
   uint8 m_searchType;
   uint8 m_searchSubstring;
   uint8 m_searchSorting;
   uint32 m_searchObject;
   uint8 m_searchDbMask;

   uint8 m_routingCostA;
   uint8 m_routingCostB;
   uint8 m_routingCostC;
   uint8 m_routingCostD;
   uint8 m_routingType;
   ItemTypes::vehicle_t m_routingVehicle;
   UserConstants::RouteImageType m_routeImageType;
   UserConstants::GenderType m_gender;
   uint32 m_validDate;

   uint32 m_editMapRights;
   uint32 m_editDelayRights;
   uint32 m_editUserRights;

   bool m_wapService;
   bool m_htmlService;
   bool m_operatorService;
   bool m_smsService;

   char* m_default_country;
   char* m_default_municipal;
   char* m_default_city;

   int32 m_nbrMunicipal;
   uint32* m_municipal;

   bool m_navService;
   char* m_operatorComment;
   char* m_emailAddress;
   char* m_address1;
   char* m_address2;
   char* m_address3;
   char* m_address4;
   char* m_address5;
   UserConstants::RouteTurnImageType m_routeTurnImageType;
   bool m_externalXmlService;
   UserConstants::transactionBased_t m_transactionBased;
   int32 m_deviceChanges;
   char* m_supportComment;
   char* m_postalCity;
   char* m_zipCode;
   char* m_companyName;
   char* m_companyReference;
   char* m_companyVATNbr;
   int32 m_emailBounces;
   int32 m_addressBounces;
   char* m_customerContactInfo;

   /// This used to be an Objvector
   typedef map<UserConstants::UserItemType, userElVect_t > userElMap_t;
   /// Map of vectors of user elements
   userElMap_t m_userElements;
   
   bool m_changed[UserConstants::USER_NBRFIELDS];

   /**
    *   Map to cache region access in. MapID in first,
    *   overviewmap, UR in second. If no overviewmap
    *   is fully covered, the map itself will be in
    *   second.first.
    *   <br />
    *   Not stored in DB. Relies on that the user is not
    *   used forever in the server.
    */
   mapRightMap_t m_userRightPerMap;

   /**
    *   Map to store the user rights per top region in.
    *   <br />
    *   Not stored in DB. Relies on that the user is not
    *   used forever in the server.
    */
   regionRightMap_t m_userRightsPerTopRegion;

  private:
   /// Sets all variables to 0 or default values.
   void initUser();
   
   
   /// Reads data from packet
   bool readFromPacket( const Packet* p , int& pos );


   /// Reads changes in Users data from packet
   bool readDataChanges( const Packet* p, int& pos );

   
   /// Valid UserElements of time
   bool m_valid[UserConstants::TYPE_NBR];

   
   /// Friend with UserProcessor
   friend class UserProcessor;
};

/**
 *  Contains data about a cellular phone
 *
 *   When packed into or from a packet UserCellular looks like this.
 *   The first TYPE_CELLULAR and Total size in bytes and the last 
 *   TYPE_CELLULAR is read by someone else when read from a packet.
 *   \begin{tabular}{lll}
 *      Pos                         & Size     & Destription \\ \hline
 *      pos                         & 1 bytes  & TYPE_CELLULAR \\
 *      +1                          & 2 bytes  & Total size in bytes \\
 *      +3                          & 1 bytes  & TYPE_CELLULAR \\
 *      +4                          & 2 bytes  & Total size in bytes \\
 *      +6                          & 4 bytes  & ID \\
 *      +10                         & 1 bytes  & smsParams \\
 *      +11                         & 2 bytes  & maxSearchHitsWap \\
 *      +13                         & 2 bytes  & maxRouteLinesWap \\
 *      +15                         & 1 bytes  & smsService \\
 *      +16                         & 1 bytes  & wapService \\
 *      +17                         & 4 bytes  & EOLType \\
 *      +21                         & 1 bytes  & smsLineLength \\
 *      +22                         & 2 bytes  & phoneNumberLength \\
 *      +24                         & string   & phoneNumber \\
 *                                  & 2 bytes  & modelNameLength\\
 *                                  & string   & modelName \\
 *                                  & data     & CellularPhoneModel \\
 *      last-1                      & 1 bytes  & TYPE_CELLULAR \\
 *      last                        & 1 bytes  & TYPE_CELLULAR \\
 *   \end{tabular}
 *
 */
class UserCellular : public UserElement
{
  public:
   /// Creates a cellular phone and sets all variables to zero
   UserCellular( uint32 ID );


   /// Creates a cellular phone and sets all variables
   UserCellular( uint32 ID,
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
                 UserConstants::EOLType eol = 
                    UserConstants::EOLTYPE_NOT_DEFINED,
                 uint8 smsLineLength = MAX_UINT8);


   /// Creates a Cellular from packet
   UserCellular( const UserReplyPacket* p, int& pos );


   /// Copy constructor
   UserCellular( const UserCellular& );


   /// Deletes all allocated variables
   virtual ~UserCellular();


   /// Puts the Cellular into p starting at pos pos
   virtual void packInto( Packet* p, int& pos );


   /// Returns the size allocated by the variables
   uint32 getSize() const;


   /// Adds all changes in the object to the packet
   virtual void addChanges(Packet* p, int& position);


   /// The changes done
   bool changed( UserConstants::UserCellularField field ) const {
      return m_changed[ field ];
   }

   
   /// The number of changed fields
   uint32 getNbrChanged() const;


   /// Reads the changes from a packet
   virtual bool readChanges( const Packet* p, int& pos, 
                             UserConstants::UserAction& action );


   /// Returns the phonenumber
   const char* getPhoneNumber() const;


   /// Sets the phonenumber
   void setPhoneNumber(const char* phoneNumber);


   /// Returns the line length to use for SMS (set by changing model)
   uint8 getSMSLineLength() const;


   /// Returns the line length to use for SMS in the Cellular, not model
   uint8 getCellularSMSLineLength() const;

   /// Sets the line length
   void setSMSLineLength( uint8 smsLineLength );

   
   /// Returns type of eol
   UserConstants::EOLType getEOLType() const;


   /// Returns the UserCellulars type of eol, doesn't check the model
   UserConstants::EOLType getCellularEOLType() const;


   /// Sets the EOL type
   void setEOLType( UserConstants::EOLType eol );

   
   /// Returns the SMSByte
   uint8 getSMSParams() const;

   
   /// Sets the SMSByte
   void setSMSParams( uint8 smsParams );


   /// Returns the maximum amount of search hits to send to the phone
   uint16 getMaxSearchHitsWap() const;


   /// Sets the maximum amount of search hits to send to the phone
   void setMaxSearchHitsWap(uint16 maxSearchHitsWap);


   /// Returns the maximum amount of route lines to send to the phone
   uint16 getMaxRouteLinesWap() const;


   /// Sets the maximum amount of route lines to send to the phone
   void setMaxRouteLinesWap(uint16 maxRouteLinesWap);

 
   /// The const cellular phone model
   const CellularPhoneModel* getModel() const;

   /// The cellular phone model
   CellularPhoneModel* getModel();

    
   /// Sets the cellular phonemodel
   void setModel( CellularPhoneModel* model );


   /// Returns if positioning is active
   bool getPosActive() const;

   /// Sets positioning on/off
   void setPosActive( bool positioningActive );


   /// Returns type of positioning ( Kipling etc )
   UserConstants::posType getTypeOfPos() const;


   /// Sets type of positioning
   void setTypeOfPos( UserConstants::posType typeOfPositioning );


   /// Returns the username in positioningsystem
   const char* getPosUserName() const;


   /// Sets username in the positioningsystem
   void setPosUserName( const char* posUserName );


   /// Returns the password in the positioningsystem
   const char* getPosPassword() const;


   /// Sets password in the possitioningsystem
   void setPosPassword( const char* posPassword );


   /// Returns latitude from the last known position
   int32 getLastPosLat() const;


   /// Set latitude for last known position
   void setLastPosLat( int32 lastpos_lat );
   

   /// Returns longitude from the last known position
   int32 getLastPosLong() const;


   /// Set latitude for last known position
   void setLastPosLong( int32 lastpos_long );

 
   /// Returns inner radius from the last known position
   uint32 getLastPosInnerRadius() const;


   /// Set inner radius for last known position
   void setLastPosInnerRadius( uint32 lastpos_innerRadius );


   /// Returns outer radius from the last known position
   uint32 getLastPosOuterRadius() const;


   /// Set outer radius for last known position
   void setLastPosOuterRadius( uint32 lastpos_outerRadius );


   /// Returns start angle from the last known position
   uint32 getLastPosStartAngle() const;


   /// Set start angle for last known position
   void setLastPosStartAngle( uint32 lastpos_startAngle );


   /// Returns stop angle from the last known position
   uint32 getLastPosStopAngle() const;


   /// Set stop angle for last known position
   void setLastPosStopAngle( uint32 lastpos_stopAngle );


   /// Returns time from the last known position
   uint32 getLastPosTime() const;

   /// retuns true if NOW - lastpostime < time_diff and position is active
   bool isPosValid( int time_diff ) const;

   /// Set time for last known position
   void setLastPosTime( uint32 lastpos_time );
   

   /// Prints the value of field into target.
   uint32 printValue( char* target, 
                      UserConstants::UserCellularField field ) const;


   /**
    * Returns true if element has changes.
    */ 
   virtual bool isChanged() const;


  protected:
   char* m_phoneNumber;
   uint8 m_smsParams;
   uint16 m_maxSearchHitsWap;
   uint16 m_maxRouteLinesWap;
   UserConstants::EOLType m_eol;
   uint8 m_smsLineLength;
   char* m_modelName;
   CellularPhoneModel* m_model;
   // smssms
   bool m_smsService;
   bool m_positioningActive;
   UserConstants::posType m_typeOfPositioning;
   char* m_posUserName;
   char* m_posPassword;
   uint32 m_lastpos_lat;
   uint32 m_lastpos_long;
   uint32 m_lastpos_innerRadius;
   uint32 m_lastpos_outerRadius;
   uint32 m_lastpos_startAngle;
   uint32 m_lastpos_stopAngle;
   uint32 m_lastpos_time;

   bool m_changed[UserConstants::CELLULAR_NBRFIELDS];


  private:
   /// Reads changes in Users data from packet
   bool readDataChanges( const Packet* p, int& pos );
};


/**
 *    Class for holding CellularPhoneModels.
 *
 */
class CellularPhoneModels  {
  public: 
   /// Create empty 
   CellularPhoneModels();


   /// Create list from packet
   CellularPhoneModels( const Packet* p, int& pos, 
                        uint32 nbrCellularPhoneModels );

   
   /// Deletes all phone models and other data.
   virtual ~CellularPhoneModels();

   
   /// Finds a phone model named name, returns NULL if none found.
   CellularPhoneModel* findModel( const char* name );


   /// Returns the model with index index, returns NULL if no such index.
   CellularPhoneModel* getModel( uint32 index );
   
   
   /**
    * Adds a phone model. The model is now owned by this class. 
    * @return true if added ok otherwise false and the model isn't taken.
    */
   bool addPhoneModel( CellularPhoneModel* model );


   /**
    * Returns the different manufacturer's names.
    * If no manufacturers exists the function returns empty 
    * StringVector.
    */
   const StringVector& getManufacturer() const;


   /**
    * Returns the number different manufacturers.
    */
   uint32 getNbrManufacturers() const;
   
   
   /**
    * If initialed ok
    */
   bool getValid() const;

   // Get the number of models
   inline int size() const;
   
   // Get element at pos
   inline CellularPhoneModel* getElementAt( unsigned int pos ) const;
   
  private:
   /// The manufacturers as strings
   StringVector* m_manufacturers;


   /// Set valid
   void setValid( bool valid );

   /// Adds to models if not already in list
   uint32 addLastIfUnique ( CellularPhoneModel* model );
   
   /// The SearchObject
   CellularPhoneModel* m_searchModel;

   /// If initialized ok
   bool m_valid;

   typedef vector < CellularPhoneModel* > CellularPhoneModelVector;

   /// The vector with PhoneModels
   CellularPhoneModelVector m_models;
};


// ========================================================================
//                                  Implementation of the inlined methods =

int 
CellularPhoneModels::size() const {
   return m_models.size();
}

CellularPhoneModel*
CellularPhoneModels::getElementAt(unsigned int pos) const {
   if(pos < m_models.size() ) {
      return m_models[ pos ];
   } else {
      return NULL;
   }
}



/**
 * Contains data about a cellular phone model.
 *
 * When packeed into or from a packet CellularPhoneModel looks like this.
 *  \begin{tabular}{lll}
 *      Pos                         & Size     & Destription \\ \hline
 *      pos                         & 1 bytes  & MAX_UINT8 / 2 \\
 *      +1                          & 2 bytes  & Total size in bytes \\
 *      +3                          & 1 bytes  & MAX_UINT8 / 2 \\
 *      +4                          & 2 bytes  & Total size in bytes \\
 *      +6                          & 1 bytes  & Characters per line \\
 *      +7                          & 4 bytes  & EOLType \\
 *      +11                         & 1 bytes  & Displayable lines \\
 *      +12                         & 1 bytes  & Dynamic width \\
 *      +13                         & 2 bytes  & Graphic Display width \\
 *      +15                         & 2 bytes  & Graphic Display height \\
 *      +17                         & 4 bytes  & SMS capable \\
 *      +21                         & 4 bytes  & SMS concatenated capable\\
 *      +25                         & 4 bytes  & SMS Graphics capable \\
 *      +29                         & 4 bytes  & WAP capable \\
 *      +33                         & 2 bytes  & Model Year \\
 *      +37                         & string   & Name \\
 *                                  & string   & Manufacturer \\
 *                                  & string   & WAP Version \\
 *                                  & string   & Comment \\
 *   \end{tabular}
 *
 */
class CellularPhoneModel  {
  public:
   /** 
    * @name Operator, to search the models.
    */
   //@{
   /// equal
   bool operator == (const CellularPhoneModel& el) const {
      return strcmp( m_name, el.m_name ) == 0;
   }
   /// not equal
   bool operator != (const CellularPhoneModel& el) const {
      return strcmp( m_name, el.m_name ) != 0;
   }
   /// greater than
   bool operator > (const CellularPhoneModel& el) const {
      return strcmp( m_name, el.m_name ) > 0;
   }
   /// less than
   bool operator < (const CellularPhoneModel& el) const {
      return strcmp( m_name, el.m_name ) < 0;
   }
   //@}


   /// SMS capability
   enum SMSCapableType {
      SMSCAPABLE_YES = 0,
      SMSCAPABLE_NO,
      SMSCAPABLE_RECEIVE_ONLY,
   };

   
   /// SMS concatenation
   enum SMSConcatenationType {
      SMSCONCATENATION_YES = 0,
      SMSCONCATENATION_NO,
      SMSCONCATENATION_RECEIVE_ONLY,
   };


   /// SMS Graphic SMSes capable
   enum SMSGraphicsType {
      SMSGRAPHICS_YES = 0,
      SMSGRAPHICS_NO,
   };

   
   /// WAP capable
   enum WAPCapableType {
      WAPCAPABLE_YES_CSD = 0,
      WAPCAPABLE_YES_SMS,
      WAPCAPABLE_YES_GPRS,
      WAPCAPABLE_NO,
   };

   
   /// Creates an empty Phone model
   CellularPhoneModel();

   
   /// Creates an Phone model from arguments
   CellularPhoneModel( const char* name, 
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
                       const char* comment );


    /// Changed a field?
   bool changed( UserConstants::CellularModelField field ) const {
      return m_changed[field];
   }

   /// Creates an Phone model from packet, sets valid.
   CellularPhoneModel( const Packet* p, int& pos );

   
   /// Copy constructor
   CellularPhoneModel( const CellularPhoneModel& model );

   
   /// Removes allocated memory
   virtual ~CellularPhoneModel();


   /// Packs the Phone model into p starting at position pos.
   bool packInto( Packet* p, int& pos );


   /// The name
   const char* getName() const;


   /// Copies the string to name
   void setName( const char* name );
   

   /// The manufacturer 
   const char* getManufacturer() const;


   /// Copies the string
   void setManufacturer( const char* manufacturer );


   /// Get nbr chars per line
   uint8 getChars() const;


   /// Set nbr chars per line
   void setChars( uint8 chars );


   /// EOL type
   UserConstants::EOLType getEOL() const;

   
   /// Set EOL type
   void setEOL( UserConstants::EOLType eol );

   
   /// Get lines
   uint8 getLines() const;


   /// Set lines
   void setLines( uint8 lines );


   /// Get dynamic width(font)
   bool getDynamicWidth() const;

   
   /// Set dynamic width
   void setDynamicWidth( bool dynamicWidth );

   
   /// Get Graphics width
   uint16 getGraphicsWidth() const;


   /// Set Graphics width
   void setGraphicsWidth( uint16 graphicDisplayWidth );


   /// Get Graphics height
   uint16 getGraphicsHeight() const;


   /// Set Graphics Height
   void setGraphicsHeight( uint16 graphicDisplayHeight );

   
   /// Get SMS capable
   SMSCapableType getSMSCapable() const;


   /// Set SMS capable
   void setSMSCapable( SMSCapableType smsCapable );


   /// Get SMS contacetanition
   SMSConcatenationType getSMSConcatenation() const;


   /// Set SMS concatenation
   void setSMSConcatenation( SMSConcatenationType smsConcat );


   /// Get Graphic SMS
   SMSGraphicsType getSMSGraphic() const;


   /// Set the Graphic SMS
   void setSMSGraphic( SMSGraphicsType smsGraphic );


   /// Get WAP capable
   WAPCapableType getWAPCapable() const;

   
   /// Set WAP capable
   void setWAPCapable( WAPCapableType wapCapable );


   /// Get WAP version
   const char* getWAPVersion() const;


   /// Set WAP version, copies the string
   void setWAPVersion( const char* wapVersion );
   
   
   /// Get the model Year
   int16 getModelYear() const;


   /// Set model Year
   void setModelYear( int16 modelYear );


   /// Get comment
   const char* getComment() const;

   
   /// Set comment, the string is copied.
   void setComment( const char* comment );


   /// Get valid, if initialized ok
   bool getValid() const;


   /// Adds changes to packet p
   void addChanges( Packet* p, int& pos);


   /// Reads changes to packet p
   void readChanges ( const Packet* p, int& pos);


   /// Counts number of changes done to CellularPhoneModel
   uint32 getNbrChanged() const; 


   /// Prints the value of field into target.
   uint32 printValue( char* target, 
                      UserConstants::CellularModelField field ) const;


  private:
   /// Initializes all member variables
   void init();

   
   /// Set valid
   void setValid( bool valid );


   /// Name of the model.
   char* m_name;


   /// Manufacturer 
   char* m_manufacturer;


   /// Characters per line
   uint8 m_chars;

   
   /// Type of EOL
   UserConstants::EOLType m_eol;


   /// Displayable lines
   uint8 m_lines;


   /// Dynamic width font or not
   bool m_dynamicWidth;

   
   /// Graphics dislay width
   uint16 m_graphicDisplayWidth;


   /// Graphics display height
   uint16 m_graphicDisplayHeight;


   /// Capable to use SMS
   SMSCapableType m_smsCapable;


   /// Concatenated SMS capable
   SMSConcatenationType m_smsConcat;


   /// Graphic SMS capable
   SMSGraphicsType m_smsGraphic;


   /// WAP capable
   WAPCapableType m_wapCapable;

   
   /// WAP version
   char* m_wapVersion;


   /// Model year
   int16 m_modelYear;


   /// Comment 
   char* m_comment;


   /// Valid data
   bool m_valid;

   /// Changed variables
   bool m_changed[UserConstants::CELLULAR_MODEL_NBRFIELDS];
};

/**
 * Contains data about a list of Buddies.
 *
 */
class UserBuddyList : public UserElement {
   public:
      /**
       * Creates a new buddylist.
       *
       * @param ID The id of the BuddyList, 0 if new and MAX_UINT32 if
       *        search.
       */
      UserBuddyList( uint32 ID );


      /**
       *  Creates a BuddyList from packet.
       */
      UserBuddyList( const UserReplyPacket* p, int& pos );

      
      /**
       * Makes a new BuddyList from orig.
       */
      UserBuddyList( const UserBuddyList& orig );

      
      /**
       * Destructor.
       */ 
      virtual ~UserBuddyList();


      /**
       *  Returns the size of the allocated variables and elements.
       *  Used to make sure that the element fits in a packet.
       *  @return size of the element packed into a packet. If the precise 
       *          size isn't known the maximum possible size is used.
       */
      virtual uint32 getSize() const;


      /**
       * Packs the Element into a packet.
       */
      virtual void packInto( Packet* p, int& pos );


      /**
       * Adds changes to packet.
       * 
       * @param p The packet to add to.
       * @param pos The position to start writing on.
       */
      virtual void addChanges( Packet* p, int& pos );


      /**
       * Reads changes from a packet and returns true if reading was ok.
       */
      virtual bool readChanges( const Packet* p, int& pos, 
                                UserConstants::UserAction& action );


      /// The number of changed fields
      uint32 getNbrChanged() const;


      /**
       * Set the ID.
       * @param ID The new ID.
       */
      virtual void setID( uint32 ID );


      /**
       * Get the owners UIN.
       * @return UIN of the list owner.
       */
      uint32 getUserUIN() const;

      
      /**
       * Set the owners UIN.
       * @param UIN New UIN for the list owner.
       */
      void setUserUIN( uint32 UIN );


      /**
       * Get the name of the buddyList.
       *
       * @return The name of the list.
       */
      const char* getName() const;


      /**
       * Set the name of the buddyList.
       * Copies the string. 
       *
       * @param The new name of the list.
       */
      void setName( const char* name );


      /**
       * Returns the number of buddies.
       */
      uint32 getNbrBuddies() const;


      /**
       * Returns the UIN of buddy with inde xindex. 0 if no such index.
       */ 
      uint32 getBuddy( uint32 index ) const;


      /**
       * Add a buddy to list.
       *
       * @return True if added false if not.
       */
      bool addBuddy( uint32 UIN );


      /**
       * Set a buddy.
       *
       * @param index Index of buddy to set.
       * @param UIN Buddy's UIN.
       */
      bool setBuddyAt( uint32 index, uint32 UIN );


      /**
       * Remove a buddy with index.
       */
      bool removeBuddyAt( uint32 index );


      /**
       * Remove a buddy with UIN.
       */
      bool removeBuddyWithUIN( uint32 UIN );


      /**
       * The maximum number of buddies.
       */
      static const uint32 MAX_NBR_BUDDIES;


      /**
       * Field changed.
       */
      bool changed( UserConstants::UserBuddyListField field ) const; 


      /**
       * Prints SQL value of field. Prints a sprintf format string if 
       * buddies.
       *
       * @param target String to print into.
       * @param field The field to print.
       * @return The number of chars written to string.
       */
      uint32 printValue( char* target, 
                         UserConstants::UserBuddyListField field ) const;


      /**
       * Returns true if element has changes.
       */ 
      virtual bool isChanged() const;


   private:
      /**
       * Reads changes from a packet.
       */
      bool readDataChanges( const Packet* p, int& pos );
      

      /**
       * The buddies.
       */
      Vector m_buddies;

      
      /**
       * Changed fields.
       */
      bool m_changed[ UserConstants::BUDDY_NBRFIELDS ];

      
      /**
       * The UIN of the owner of this list.
       */
      uint32 m_userUIN;

      
      /**
       * The name of the buddyList
       */
      char* m_name;
};


/**
 * DBaseObject class for UserNavigator.
 *
 */
class DBUserNavigator : public DBaseObject {
   public:
      /**
       * Creates new empty DBUserNavigator.
       */
      DBUserNavigator();

      
      /**
       * Copy constructor.
       */
      DBUserNavigator( const DBUserNavigator& copy );


      /**
       * Reads from SQL database.
       */
      DBUserNavigator( SQLQuery* pQuery );

      
      /**
       * Reads from Packet.
       */
      DBUserNavigator( const Packet* pPacket, int& pos );

      
      /**
       * Destructor.
       */
      virtual ~DBUserNavigator();


      /**
       * The table strings, whatever.
       */
      virtual const char* getTableStr() const;


      /**
       * The same as getTableStr but static.
       */
      static const char* getTableName();

      
      /**
       * The descriptions of the fields in ascii.
       */
      virtual const char** getFieldDescriptions() const;


      /**
       * The descriptions of the fields in stringCodes.
       */
      virtual const StringTable::stringCode* getFieldStringCode() const;


      /**
       * The number of changed fields.
       */
      uint32 getNbrChanged() const;


      /**
       * The ID of this UserNavigator.
       */
      uint32 getID() const;


      /**
       * Set the ID.
       * @param ID The new ID.
       */
      void setID( uint32 ID );


      /**
       * Get the owners UIN.
       * @return UIN of the list owner.
       */
      uint32 getUserUIN() const;

      
      /**
       * Set the owners UIN.
       * @param UIN New UIN for the list owner.
       */
      void setUserUIN( uint32 UIN );

      
      /**
       * Type of Navigator. eBox or Navigator.
       */
      UserConstants::navigatorType getNavigatorType() const;


      /**
       * Set type of Navigator.
       */
      void setNavigatorType( UserConstants::navigatorType navType );

      
      /**
       * The address of the UserNavigator, e.g. phonenumber.
       */
      const char* getAddress() const ;


      /**
       * Set the address of the UserNavigator.
       */
      void setAddress( const char* address );


      /**
       * Unsent changed data count.
       */
      uint16 getUnsentCount() const ;


      /**
       * Set unsent changed data count.
       */
      void setUnsentCount( uint16 count );


      /**
       * Date of last contact in UTC.
       */
      uint32 getLastContactDate() const;


      /**
       * Set date of last contact in UTC.
       */
      void setLastContactDate( uint32 date );

      
      /**
       * Last contact successful.
       */
      bool getLastContactSuccess() const;


      /**
       * Set last contact successful.
       */
      void setLastContactSuccess( bool success );


      /**
       * The latitude of last contact.
       */
      uint32 getLastContactLat() const;


      /**
       * Set the latitude of last contact.
       */
      void setLastContactLat( uint32 lat );

      
      /**
       * The longitude of last contact.
       */
      uint32 getLastContactLon() const;


      /**
       * Set the longitude of last contact.
       */
      void setLastContactLon( uint32 lon );


      /**
       * The field to index enum.
       */
      enum fieldNbr {
         field_ID = 0,
         field_userUIN,
         field_navType,
         field_address,
         field_unsentDataCount,
         field_lastContactDate,
         field_lastContactSuccess,
         field_lastContactLatitude,
         field_lastContactLongitude
      };


   protected:
      /**
       * Table field names.
       */
      virtual const char** getFieldNames() const;

      
      /**
       * Type of elemtnts.
       */
      virtual const DBaseElement::element_t* getFieldTypes() const;

      
   private:
      /**
       * The number of fields.
       */
      static const uint32 m_nbrFields;


      /**
       * The SQL string of the name of the table.
       */
      static const char* m_tableStr;
      

      /**
       * The fieldNames
       */
      static const char* m_fieldNames[];


      /**
       * The fieldTypes.
       */
      static const DBaseElement::element_t m_fieldTypes[];

      
      /**
       * The fieldDescriptions.
       */
      static const char* m_fieldDescriptionStrs[];


      /**
       * The descriptions in stringCodes.
       */
      static const StringTable::stringCode m_fieldDescriptions[];


      /**
       * UserProcessor is a friend.
       */
      friend class UserProcessor;
};


/**
 * Contains data about a Navigation unit.
 *
 */
class UserNavigator : public UserElement {
   public:
      /**
       * Creates a new UserNavigator.
       *
       * @param ID The id of the UserNavigator, 0 if new and MAX_UINT32 if
       *        search.
       */
      UserNavigator( uint32 ID );

      
      /**
       * Creates new UserNavigator from packet.
       */
      UserNavigator( const UserReplyPacket* p, int& pos );


      /**
       * Reads from SQL database.
       */
      UserNavigator( SQLQuery* pQuery );


      /**
       * Makes a new UserNavigator from orig.
       */
      UserNavigator( const UserNavigator& orig );


      /**
       * Destructor.
       */
      ~UserNavigator();


      /**
       *  Returns the size of the allocated variables and elements.
       *  Used to make sure that the element fits in a packet.
       *  @return size of the element packed into a packet. If the precise 
       *          size isn't known the maximum possible size is used.
       */
      virtual uint32 getSize() const;


      /**
       * Packs the Element into a packet.
       */
      virtual void packInto( Packet* p, int& pos );


      /**
       * Adds changes to packet.
       * 
       * @param p The packet to add to.
       * @param pos The position to start writing on.
       */
      virtual void addChanges( Packet* p, int& pos );


      /**
       * Reads changes from a packet and returns true if reading was ok.
       */
      virtual bool readChanges( const Packet* p, int& pos, 
                                UserConstants::UserAction& action );
      

      /**
       * The number of changed fields.
       */
      uint32 getNbrChanged() const;


      /**
       * The ID of this UserNavigator.
       */
      virtual uint32 getID() const;


      /**
       * Set the ID.
       * @param ID The new ID.
       */
      virtual void setID( uint32 ID );


      /**
       * Get the owners UIN.
       * @return UIN of the list owner.
       */
      uint32 getUserUIN() const;

      
      /**
       * Set the owners UIN.
       * @param UIN New UIN for the list owner.
       */
      void setUserUIN( uint32 UIN );

      
      /**
       * Type of Navigator. eBox or Navigator.
       */
      UserConstants::navigatorType getNavigatorType();


      /**
       * Set type of Navigator.
       */
      void setNavigatorType( UserConstants::navigatorType navType );

      
      /**
       * The address of the UserNavigator, e.g. phonenumber.
       */
      const char* getAddress() const ;


      /**
       * Set the address of the UserNavigator.
       */
      void setAddress( const char* address );


      /**
       * Unsent changed data count.
       */
      uint16 getUnsentCount() const ;


      /**
       * Set unsent changed data count.
       */
      void setUnsentCount( uint16 count );


      /**
       * Date of last contact in UTC.
       */
      uint32 getLastContactDate() const;


      /**
       * Set date of last contact in UTC.
       */
      void setLastContactDate( uint32 date );

      
      /**
       * Last contact successful.
       */
      bool getLastContactSuccess() const;


      /**
       * Set last contact successful.
       */
      void setLastContactSuccess( bool success );


      /**
       * The latitude of last contact.
       */
      uint32 getLastContactLat() const;


      /**
       * Set the latitude of last contact.
       */
      void setLastContactLat( uint32 lat );

      
      /**
       * The longitude of last contact.
       */
      uint32 getLastContactLon() const;


      /**
       * Set the longitude of last contact.
       */
      void setLastContactLon( uint32 lon );


      /**
       * Get the DBUserNavigator
       */
      DBUserNavigator* getDB();
      

      /**
       * Returns true if element has changes.
       */ 
      virtual bool isChanged() const;


   private:
      DBUserNavigator* m_dbNavigator; 
};


/**
 * DBaseObject class for Navigation destinations.
 *
 */
class DBUserNavDestination : public DBaseObject {
   public:
      /**
       * Creates new empty DBUserNavDestination.
       */
      DBUserNavDestination();

      
      /**
       * Copy constructor.
       */
      DBUserNavDestination( const DBUserNavDestination& copy );


      /**
       * Reads from SQL database.
       */
      DBUserNavDestination( SQLQuery* pQuery );

      
      /**
       * Reads from Packet.
       */
      DBUserNavDestination( const Packet* pPacket, int& pos );

      
      /**
       * Destructor.
       */
      virtual ~DBUserNavDestination();


      /**
       * The table's sql name.
       */
      virtual const char* getTableStr() const;


      /**
       * The same as getTableStr but static.
       */
      static const char* getTableName();

      
      /**
       * The descriptions of the fields in ascii.
       */
      virtual const char** getFieldDescriptions() const;


      /**
       * The descriptions of the fields in stringCodes.
       */
      virtual const StringTable::stringCode* getFieldStringCode() const;


      /**
       * The number of changed fields.
       */
      uint32 getNbrChanged() const;

      
      /**
       * The ID of this DBUserNavDestination.
       */
      uint32 getID() const;


      /**
       * Set the ID.
       * @param ID The new ID.
       */
      void setID( uint32 ID );


      /**
       * Get the owners navigator.
       * @return NavigatorID of the owner.
       */
      uint32 getNavigatorID() const;

      
      /**
       * Set the owners naigator.
       * @param navID The NavigatorID for the owner.
       */
      void setNavigatorID( uint32 navID );

            
      /**
       * Sent if message has been sent.
       */
      bool getSent() const;


      /**
       * Set if message has been sent.
       */
      void setSent( bool sent );


      /**
       * Date of creation in UTC.
       */
      uint32 getCreationDate() const;


      /**
       * Set date of creation in UTC.
       */
      void setCreationDate( uint32 date );


      /**
       * Type of message.
       */
      UserConstants::navigatorMessageType getMessageType() const;


      /**
       * Set the type of message.
       */
      void setMessageType( UserConstants::navigatorMessageType type );


      /**
       * Name of the destination entered by the user.
       */
      const char* getName() const;


      /**
       * Set the name of the destination entered by the user.
       */
      void setName( const char* name );


      /**
       * The NavigatorID of the navigator that initiated the PickMeUp,
       * is only valid if MessageType is NAVIGATORMESSAGETYPE_PICKMEUP.
       */
      uint32 getSenderID() const;


      /**
       * Set the navigatorID of the navigator that initiated the PickMeUp,
       * see getSenderID for more information.
       * @param ID The new ID.
       */
      void setSenderID( uint32 ID );


      /**
       * Type of receiver. eBox or Navigator.
       */
      UserConstants::navigatorType getNavigatorType();


      /**
       * Set type of receiver for this UserNavDestination.
       */
      void setNavigatorType( UserConstants::navigatorType navType );

      
      /**
       * The address of the receiver, e.g. phonenumber.
       */
      const char* getAddress() const;


      /**
       * Set the address of the receiver.
       */
      void setAddress( const char* address );


      /**
       * The latitude of this destination.
       */
      uint32 getLat() const;


      /**
       * Set the latitude of this destination.
       */
      void setLat( uint32 lat );

      
      /**
       * The longitude of this destination.
       */
      uint32 getLon() const;


      /**
       * Set the longitude of this destination.
       */
      void setLon( uint32 lon );


      /**
       * The field to index enum.
       */
      enum fieldNbr {
         field_ID = 0,
         field_navID,
         field_sent,
         field_created,
         field_type,
         field_name,
         field_senderID,
         field_receiverType,
         field_receiverAddress,
         field_latitude,
         field_longitude
      };


   protected:
      /**
       * Table field names.
       */
      virtual const char** getFieldNames() const;

      
      /**
       * Type of elemtnts.
       */
      virtual const DBaseElement::element_t* getFieldTypes() const;

      
   private:
      /**
       * The number of fields.
       */
      static const uint32 m_nbrFields;


      /**
       * The SQL string of the name of the table.
       */
      static const char* m_tableStr;
      

      /**
       * The fieldNames
       */
      static const char* m_fieldNames[];


      /**
       * The fieldTypes.
       */
      static const DBaseElement::element_t m_fieldTypes[];

      
      /**
       * The fieldDescriptions.
       */
      static const char* m_fieldDescriptionStrs[];


      /**
       * The descriptions in stringCodes.
       */
      static const StringTable::stringCode m_fieldDescriptions[];


      /**
       * UserProcessor is a friend.
       */
      friend class UserProcessor;
};


/**
 * Class for holding a debit row.
 *
 */
class DebitElement {
   public:
      /**
       * Constructs a DebitElement.
       */
      DebitElement( uint32 messageID,
                    uint32 debInfo,
                    uint32 time,
                    uint32 operationType,
                    uint32 sentSize,
                    const char* userOrigin,
                    const char* serverID,
                    const char* operationDescription );


      /**
       * Constructs an empty DebitElement, must call readFromPacket before
       * using the DebitElement.
       */
      DebitElement();


      /**
       * Destructor.
       */
      virtual ~DebitElement();


      /**
       * The maximum size the DebitElement will take packed into a Packet.
       *
       * @return The size in a Packet.
       */
      uint32 getSize() const;


      /**
       * Pack the DebitElement into a Packet.
       * May resize the Packet if nessesary.
       *
       * @param p The Packet to pack the DebitElement into, may be resized
       *          to fit the DebitElement.
       * @param pos The position in the Packet.
       */
      void packInto( Packet* p, int& pos ) const;


      /**
       * Reads a DebitElement from p and sets the values in this.
       *
       * @param p The Packet to read the DebitElement from.
       * @param pos The position in the Packet.
       */
      void readFromPacket( const Packet* p, int& pos );


      /**
       * @return The messageID.
       */
      inline uint32 getMessageID() const;

   
      /**
       * @return The sent size.
       */
      inline uint32 getSentSize() const;
   

      /**
       * @return The debInfo number.
       */
      inline uint32 getDebitInfo() const;


      /**
       * @return The timestamp.
       */
      inline uint32 getTime() const;


      /**
       * @return The kind of operation.
       */
      inline uint32 getOperationType() const;


      /**
       * @return The user origin.
       */
      inline const char* getUserOrigin() const;

   
      /**
       * @return The servers ID.
       */
      inline const char* getServerID() const;
   

      /**
       * @return The operationdescription.
       */
      inline const char* getDescription() const;


   private:
      /**
       * The messageID.
       */
      uint32 m_messageID;


      /**
       * The sent size.
       */
      uint32 m_sentSize;


      /**
       * The debInfo number.
       */
      uint32 m_debitInfo;


      /**
       * The timestamp.
       */
      uint32 m_time;


      /**
       * The kind of operation.
       */
      uint32 m_operationType;


      /**
       * The user origin.
       */
      char* m_userOrigin;


      /**
       * The servers ID.
       */
      char* m_serverID;


      /**
       * The operationdescription.
       */
      char* m_description;
};


// ========================================================================
//                                  Implementation of the inlined methods =


uint32 
DebitElement::getMessageID() const {
   return m_messageID;
}


uint32 
DebitElement::getDebitInfo() const {
   return m_debitInfo;
}


uint32 
DebitElement::getTime() const {
   return m_time;
}


uint32 
DebitElement::getOperationType() const {
   return m_operationType;
}


uint32 
DebitElement::getSentSize() const {
   return m_sentSize;
}


const char*
DebitElement::getUserOrigin() const {
   return m_userOrigin;
}


const char*
DebitElement::getServerID() const {
   return m_serverID;
}


const char*
DebitElement::getDescription() const {
   return m_description;
}



/**
 * Class for holding information about user's access to a region.
 *
 *   When packed into or from a packet UserRegionAccess looks like this.
 *   The first TYPE_REGION_ACCESS and Total size in bytes and the last 
 *   TYPE_REGION_ACCESS is read by someone else when read from a packet.
 *   \begin{tabular}{lll}
 *      Pos                         & Size     & Destription \\ \hline
 *      pos                         & 4 bytes  & TYPE_REGION_ACCESS \\
 *      +4                          & 2 bytes  & Total size in bytes \\
 *      +6                          & 4 bytes  & TYPE_REGION_ACCESS \\
 *      +10                         & 2 bytes  & Total size in bytes \\
 *      +12                         & 4 bytes  & UserAccessRegion ID \\
 *      +16                         & 4 bytes  & Region id \\
 *      +20                         & 4 bytes  & Start time \\
 *      +24                         & 4 bytes  & End time \\
 *   \end{tabular}
 *
 */
class UserRegionAccess : public UserElement {
   public:
      /**
       * Constructs a UserRegionAccess.
       *
       * @param id The id of the UserRegionAccess, use 0 if new 
       *           UserRegionAccess.
       * @param regionID The id of the region that the user has access to.
       * @param startTime The time when the access starts, in UTC.
       * @param endTime The time when the access ends, in UTC.
       */
      UserRegionAccess( uint32 id,
                        uint32 regionID,
                        uint32 startTime,
                        uint32 endTime );


      /**
       * Constructs an empty UserRegionAccess, must call readFromPacket 
       * before using the UserRegionAccess.
       */
      UserRegionAccess( uint32 id );


      /**
       * Sets all variables according to parameters in packet.
       */
      UserRegionAccess( const UserReplyPacket* p, int& pos );

   
      /**
       * Copy constructor copies everything.
       */
      UserRegionAccess( const UserRegionAccess& user );


      /**
       * Destructor.
       */
      virtual ~UserRegionAccess();


      /**
       * The maximum size the UserRegionAccess will take packed into a 
       * Packet.
       *
       * @return The size in a Packet.
       */
      uint32 getSize() const;


      /**
       * Pack the UserRegionAccess into a Packet.
       * May resize the Packet if nessesary.
       *
       * @param p The Packet to pack the UserRegionAccess into, may be 
       *          resized to make the UserRegionAccess fit.
       * @param pos The position in the Packet.
       */
      virtual void packInto( Packet* p, int& pos );


      /**
       * Adds all changes in the object to the packet.
       *
       */
      virtual void addChanges( Packet* p, int& position );


      /**
       * Reads changes from a packet.
       */
      virtual bool readChanges( const Packet* p, int& pos, 
                                UserConstants::UserAction& action );


      /**
       * Reads a UserRegionAccess from p and sets the values in this.
       * Sets this to be ok if read ok.
       *
       * @param p The Packet to read the UserRegionAccess from.
       * @param pos The position in the Packet.
       */
      void readFromPacket( const Packet* p, int& pos );


      /**
       * Get the region's ID.
       *
       * @return The region's ID.
       */
      inline uint32 getRegionID() const;
      inline void setRegionID(uint32 regionID) {
         m_regionID = regionID;
      }
   
      /**
       * Get the time that the access starts.
       *
       * @return The time that the access starts.
       */
      inline uint32 getStartTime() const;


      /**
       * Get the time that the access ends.
       *
       * @return The time that the access end.
       */
      inline uint32 getEndTime() const;


      /**
       * The number of changed fields.
       */
      uint32 getNbrChanged() const;


      /**
       * Prints SQL value of field.
       *
       * @param target String to print into.
       * @param field The field to print.
       * @return The number of chars written to string.
       */
      uint32 printValue( char* target, 
                         UserConstants::UserRegionAccessField field ) const;


      /**
       * Get the id of the UserRegionAccess element.
       * 
       * @return The id of the UserRegionAccess element.
       */
      inline uint32 getID() const;


      /**
       * Field changed.
       */
      bool changed( UserConstants::UserRegionAccessField field ) const;


      /**
       * Returns true if element has changes.
       */ 
      virtual bool isChanged() const;


   private:
      /**
       * Reads changes from a packet.
       */
      bool readDataChanges( const Packet* p, int& pos );


      /**
       * The region id.
       */
      uint32 m_regionID;


      /**
       * The start time.
       */
      uint32 m_startTime;


      /**
       * The end time.
       */
      uint32 m_endTime;


      /**
       * The changed fields.
       */
      bool m_changed[ UserConstants::USER_REGION_ACCESS_NBRFIELDS ];
};


// -----------------------------------------------------------------
//  Implementation of inlined methods for UserRegionAccess
// -----------------------------------------------------------------


uint32
UserRegionAccess::getRegionID() const {
   return m_regionID;
}


uint32
UserRegionAccess::getStartTime() const {
   return m_startTime;
}


uint32
UserRegionAccess::getEndTime() const {
   return m_endTime;
}


uint32 
UserRegionAccess::getID() const {
   return m_id;
}



/**
 * Class for holding information about user's Wayfinder subscription.
 *
 * When packed into or from a packet UserWayfinderSubscription looks 
 * like this.
 * The first TYPE and Total size in bytes and the last 
 * TYPE is read by someone else when read from a packet.
 * \begin{tabular}{lll}
 *    Pos                     & Size     & Destription \\ \hline
 *    pos                     & 4 bytes  & TYPE_WAYFINDER_SUBSCRIPTION \\
 *    +4                      & 2 bytes  & Total size in bytes \\
 *    +6                      & 4 bytes  & TYPE_WAYFINDER_SUBSCRIPTION \\
 *    +10                     & 2 bytes  & Total size in bytes \\
 *    +12                     & 4 bytes  & UserWayfinderSubscription ID \\
 *    +16                     & 1 bytes  & Wayfinder subscription type \\
 * \end{tabular}
 *
 */
class UserWayfinderSubscription : public UserElement {
   public:
      /**
       * Constructs a UserWayfinderSubscription.
       *
       * @param id The id of the UserWayfinderSubscription, use 0 if new 
       *           UserWayfinderSubscription.
       * @param type The WayfinderSubscription type.
       */
      UserWayfinderSubscription( uint32 id, byte type );


      /**
       * Constructs an empty UserWayfinderSubscription, 
       * must call readFromPacket before using the 
       * UserWayfinderSubscription.
       */
      UserWayfinderSubscription( uint32 id );


      /**
       * Sets all variables according to parameters in packet.
       */
      UserWayfinderSubscription( const UserReplyPacket* p, int& pos );

   
      /**
       * Copy constructor copies everything.
       */
      UserWayfinderSubscription( const UserWayfinderSubscription& user );


      /**
       * Destructor.
       */
      virtual ~UserWayfinderSubscription();


      /**
       * The maximum size the UserWayfinderSubscription will take packed
       * into a Packet.
       *
       * @return The size in a Packet.
       */
      uint32 getSize() const;


      /**
       * Pack the UserWayfinderSubscription into a Packet.
       * May resize the Packet if nessesary.
       *
       * @param p The Packet to pack the UserWayfinderSubscription into,
       *          may be resized to make the UserWayfinderSubscription fit.
       * @param pos The position in the Packet.
       */
      virtual void packInto( Packet* p, int& pos );


      /**
       * Adds all changes in the object to the packet.
       *
       */
      virtual void addChanges( Packet* p, int& position );


      /**
       * Reads changes from a packet.
       */
      virtual bool readChanges( const Packet* p, int& pos, 
                                UserConstants::UserAction& action );


      /**
       * Reads a UserWayfinderSubscription from p and sets the values in
       * this.
       * Sets this to be ok if read ok.
       *
       * @param p The Packet to read the UserWayfinderSubscription from.
       * @param pos The position in the Packet.
       */
      void readFromPacket( const Packet* p, int& pos );


      /**
       * Get type.
       *
       * @return The type.
       */
      inline byte getWayfinderType() const;


      /**
       * Set type.
       *
       * @param type The new type.
       */
      inline void setWayfinderType( byte type );


      /**
       * The number of changed fields.
       */
      uint32 getNbrChanged() const;


      /**
       * Prints SQL value of field.
       *
       * @param target String to print into.
       * @param field The field to print.
       * @return The number of chars written to string.
       */
      uint32 printValue( 
         char* target, 
         UserConstants::UserWayfinderSubscriptionField field ) const;


      /**
       * Field changed.
       */
      bool changed( 
         UserConstants::UserWayfinderSubscriptionField field ) const;


      /**
       * Returns true if element has changes.
       */ 
      virtual bool isChanged() const;


   private:
      /**
       * Reads changes from a packet.
       */
      bool readDataChanges( const Packet* p, int& pos );


      /**
       * The type.
       */
      byte m_wayfinderType;


      /**
       * The changed fields.
       */
      bool m_changed[ 
         UserConstants::USER_WAYFINDER_SUBSCRIPTION_NBRFIELDS ];
};


// -----------------------------------------------------------------
//  Implementation of inlined methods for UserWayfinderSubscription
// -----------------------------------------------------------------


byte
UserWayfinderSubscription::getWayfinderType() const {
   return m_wayfinderType;
}


inline void
UserWayfinderSubscription::setWayfinderType( byte type ) {
   m_wayfinderType = type;
   m_changed[ UserConstants::USER_WAYFINDER_SUBSCRIPTION_TYPE ] = true;
}



/**
 * Class for holding information about a token.
 *
 * When packed into or from a packet UserToken looks 
 * like this.
 * The first TYPE and Total size in bytes and the last 
 * TYPE is read by someone else when read from a packet.
 * \begin{tabular}{lll}
 *    Pos                     & Size     & Destription \\ \hline
 *    pos                     & 4 bytes  & TYPE_TOKEN \\
 *    +4                      & 2 bytes  & Total size in bytes \\
 *    +6                      & 4 bytes  & TYPE_TOKEN \\
 *    +10                     & 2 bytes  & Total size in bytes \\
 *    +12                     & 4 bytes  & ID \\
 *    +16                     & 4 bytes  & Creation time. \\
 *    +20                     & 1 bytes  & The number of tokens before 
 *                                         this that has used the AC. \\
 *    +21                     & string   & The token.
 * \end{tabular}
 *
 */
class UserToken : public UserElement {
   public:
      /**
       * Constructs a UserWayfinderSubscription.
       *
       * @param id The id of the UserWayfinderSubscription, use 0 if new 
       *           UserWayfinderSubscription.
       * @param createTime Time when token was created.
       * @param age The number of tokens before this that has used the AC.
       * @param token The token string.
       * @param group The group that this token belongs to.
       */
      UserToken( uint32 id, uint32 createTime, byte age, 
                 const char* token, const MC2String& group );


      /**
       * Constructs an empty UserToken, 
       * must call readFromPacket before using the UserToken.
       */
      UserToken( uint32 id );


      /**
       * Sets all variables according to parameters in packet.
       */
      UserToken( const Packet* p, int& pos );

   
      /**
       * Copy constructor copies everything.
       */
      UserToken( const UserToken& user );


      /**
       * Destructor.
       */
      virtual ~UserToken();


      /**
       * The maximum size the UserToken will take packed into a Packet.
       *
       * @return The size in a Packet.
       */
      uint32 getSize() const;


      /**
       * Pack the UserToken into a Packet.
       * May resize the Packet if nessesary.
       *
       * @param p The Packet to pack the UserToken into,
       *          may be resized to make the UserToken fit.
       * @param pos The position in the Packet.
       */
      virtual void packInto( Packet* p, int& pos );


      /**
       * Adds all changes in the object to the packet.
       *
       * @param p The packet to add changes to.
       * @param position The position in the packet is updated.
       */
      virtual void addChanges( Packet* p, int& position );


      /**
       * Reads changes from a packet.
       */
      virtual bool readChanges( const Packet* p, int& pos, 
                                UserConstants::UserAction& action );


      /**
       * Reads a UserToken from p and sets the values in this.
       * Sets this to be ok if read ok.
       *
       * @param p The Packet to read the UserToken from.
       * @param pos The position in the Packet.
       */
      void readFromPacket( const Packet* p, int& pos );


      /**
       * Get createTime.
       *
       * @return The createTime.
       */
      uint32 getCreateTime() const;


      /**
       * Set createTime.
       *
       * @param createTime The new createTime.
       */
      void setCreateTime( uint32 createTime );


      /**
       * Get age.
       *
       * @return The age.
       */
      byte getAge() const;


      /**
       * Set age.
       *
       * @param age The new age.
       */
      void setAge( byte age );


      /**
       * Get token.
       *
       * @return The token.
       */
      const char* getToken() const;


      /**
       * Set token.
       *
       * @param token The new token.
       */
      void setToken( const char* token );


      /**
       * Get group.
       *
       * @return The group.
       */
      const MC2String& getGroup() const;


      /**
       * Set group.
       *
       * @param group The new group.
       */
      void setGroup( const MC2String& group );


      /**
       * The number of changed fields.
       */
      uint32 getNbrChanged() const;


      /**
       * Prints SQL value of field.
       *
       * @param target String to print into.
       * @param field The field to print.
       * @return The number of chars written to string.
       */
      uint32 printValue( 
         char* target, UserConstants::UserTokenField field ) const;


      /**
       * Field changed.
       */
      bool changed( UserConstants::UserTokenField field ) const;


      /**
       * Returns true if element has changes.
       */ 
      virtual bool isChanged() const;


   private:
      /**
       * Reads changes from a packet.
       */
      bool readDataChanges( const Packet* p, int& pos );


      /**
       * The createTime.
       */
      uint32 m_createTime;


      /**
       * The age.
       */
      byte m_age;


      /**
       * The token.
       */
      MC2String m_token;


      /**
       * The group.
       */
      MC2String m_group;


      /**
       * The changed fields.
       */
      bool m_changed[ 
         UserConstants::USER_TOKEN_NBRFIELDS ];
};


// -----------------------------------------------------------------
//  Implementation of inlined methods for UserToken
// -----------------------------------------------------------------


inline uint32
UserToken::getCreateTime() const {
   return m_createTime;
}


inline void
UserToken::setCreateTime( uint32 createTime ) {
   m_createTime = createTime;
   m_changed[ UserConstants::USER_TOKEN_CREATE_TIME ] = true;
}


inline byte
UserToken::getAge() const {
   return m_age;
}


inline void
UserToken::setAge( byte age ) {
   m_age = age;
   m_changed[ UserConstants::USER_TOKEN_AGE ] = true;
}


inline const char*
UserToken::getToken() const {
   return m_token.c_str();
}


inline void
UserToken::setToken( const char* token ) {
   m_token = token;
   m_changed[ UserConstants::USER_TOKEN_TOKEN ] = true;
}


inline const MC2String&
UserToken::getGroup() const {
   return m_group;
}


inline void
UserToken::setGroup( const MC2String& group ) {
   m_group = group;
   m_changed[ UserConstants::USER_TOKEN_GROUP ] = true;
}


/**
 * Class for holding a PIN.
 *
 * When packed into or from a packet UserPIN looks 
 * like this.
 * The first TYPE and Total size in bytes and the last 
 * TYPE is read by someone else when read from a packet.
 * \begin{tabular}{lll}
 *    Pos                     & Size     & Destription \\ \hline
 *    pos                     & 4 bytes  & TYPE_PIN \\
 *    +4                      & 2 bytes  & Total size in bytes \\
 *    +6                      & 4 bytes  & TYPE_PIN \\
 *    +10                     & 2 bytes  & Total size in bytes \\
 *    +12                     & 4 bytes  & ID \\
 *    +16                     & string   & The PIN.
 *    +X                      & string   & The comment.
 * \end{tabular}
 *
 */
class UserPIN : public UserElement {
   public:
      /**
       * Constructs a UserWayfinderSubscription.
       *
       * @param id The id of the UserWayfinderSubscription, use 0 if new 
       *           UserWayfinderSubscription.
       * @param PIN The PIN string.
       * @param comment The comment string.
       */
      UserPIN( uint32 id, const char* PIN, const char* comment );


      /**
       * Constructs an empty UserPIN, 
       * must call readFromPacket before using the UserPIN.
       */
      UserPIN( uint32 id );


      /**
       * Sets all variables according to parameters in packet.
       */
      UserPIN( const Packet* p, int& pos );

   
      /**
       * Copy constructor copies everything.
       */
      UserPIN( const UserPIN& user );


      /**
       * Destructor.
       */
      virtual ~UserPIN();


      /**
       * The maximum size the UserPIN will take packed into a Packet.
       *
       * @return The size in a Packet.
       */
      uint32 getSize() const;


      /**
       * Pack the UserPIN into a Packet.
       * May resize the Packet if nessesary.
       *
       * @param p The Packet to pack the UserPIN into,
       *          may be resized to make the UserPIN fit.
       * @param pos The position in the Packet.
       */
      virtual void packInto( Packet* p, int& pos );


      /**
       * Adds all changes in the object to the packet.
       *
       * @param p The packet to add changes to.
       * @param position The position in the packet is updated.
       */
      virtual void addChanges( Packet* p, int& position );


      /**
       * Reads changes from a packet.
       */
      virtual bool readChanges( const Packet* p, int& pos, 
                                UserConstants::UserAction& action );


      /**
       * Reads a UserPIN from p and sets the values in this.
       * Sets this to be ok if read ok.
       *
       * @param p The Packet to read the UserPIN from.
       * @param pos The position in the Packet.
       */
      void readFromPacket( const Packet* p, int& pos );


      /**
       * Get PIN.
       *
       * @return The PIN.
       */
      const char* getPIN() const;


      /**
       * Set PIN.
       *
       * @param PIN The new PIN.
       */
      void setPIN( const char* PIN );


      /**
       * Get comment.
       *
       * @return The comment.
       */
      const char* getComment() const;


      /**
       * Set comment.
       *
       * @param comment The new comment.
       */
      void setComment( const char* comment );


      /**
       * The number of changed fields.
       */
      uint32 getNbrChanged() const;


      /**
       * Prints SQL value of field.
       *
       * @param target String to print into.
       * @param field The field to print.
       * @return The number of chars written to string.
       */
      uint32 printValue( 
         char* target, UserConstants::UserPINField field ) const;


      /**
       * Field changed.
       */
      bool changed( UserConstants::UserPINField field ) const;


      /**
       * Returns true if element has changes.
       */ 
      virtual bool isChanged() const;


   private:
      /**
       * Reads changes from a packet.
       */
      bool readDataChanges( const Packet* p, int& pos );


      /**
       * The PIN.
       */
      MC2String m_PIN;


      /**
       * The comment.
       */
      MC2String m_comment;


      /**
       * The changed fields.
       */
      bool m_changed[ 
         UserConstants::USER_PIN_NBRFIELDS ];
};


// -----------------------------------------------------------------
//  Implementation of inlined methods for UserPIN
// -----------------------------------------------------------------


inline const char*
UserPIN::getPIN() const {
   return m_PIN.c_str();
}


inline void
UserPIN::setPIN( const char* PIN ) {
   m_PIN = PIN;
   m_changed[ UserConstants::USER_PIN_PIN ] = true;
}


inline const char*
UserPIN::getComment() const {
   return m_comment.c_str();
}


inline void
UserPIN::setComment( const char* comment ) {
   m_comment = comment;
   m_changed[ UserConstants::USER_PIN_COMMENT ] = true;
}



/**
 * Class for holding a last client type for a user.
 *
 *   When packed into or from a packet UserIDKey looks like this.
 *
 *   \begin{tabular}{lll}
 *      Pos                         & Size     & Destription \\ \hline
 *      pos                         & 4 bytes  & TYPE_LAST_CLIENT \\
 *      +4                          & 2 bytes  & Total size in bytes \\
 *      +6                          & 4 bytes  & TYPE_LAST_CLIENT \\
 *      +10                         & 2 bytes  & Total size in bytes \\
 *      +12                         & 4 bytes  & ID \\
 *      +16                         & string   & client type \\
 *      +x                          & string   & client type options \\
 *      +X                          & string   & version \\
 *      +X                          & string   & extra \\
 *      +X                          & string   & origin \\
 *   \end{tabular}
 *
 */
class UserLastClient : public UserElement {
   public:
      /**
       * Constructs an empty UserLastClient.
       */
      UserLastClient( uint32 id );

      
      /**
       * Constructs a new UserLastClient with all data set.
       */
      UserLastClient( uint32 id, const MC2String& clientType, 
                      const MC2String& clientTypeOptions, 
                      const MC2String& version, const MC2String& extra,
                      const MC2String& origin, bool history = false,
                      uint32 changerUIN = 0, uint32 changeTime = 0 );


      /// Sets all variables according to parameters in packet.
      UserLastClient( const Packet* p, int& pos );

   
      /// Copy constructor copies everything.
      UserLastClient( const UserLastClient& user );
  

      /// Deletes all allocated variables
      virtual ~UserLastClient();


      /// The maximum size the element will take packed into a Packet.
      uint32 getSize() const;


      /// Puts the UserLastClient into p
      virtual void packInto( Packet* p, int& pos );


      /// Adds all changes in the object to the packet
      virtual void addChanges( Packet* p, int& position );


      /// Reads changes from a packet.
      virtual bool readChanges( const Packet* p, int& pos, 
                                UserConstants::UserAction& action );


      /**
       * Reads a element from p and sets the values in this.
       * Sets this to be ok if read ok.
       *
       * @param p The Packet to read the element from.
       * @param pos The position in the Packet.
       */
      void readFromPacket( const Packet* p, int& pos );

      
      /**
       * Returns the client type.
       */
      const MC2String& getClientType() const;

      
      /**
       * Sets the client type.
       */
      void setClientType( const MC2String& clientType );


      /**
       * Returns the client type options.
       */
      const MC2String& getClientTypeOptions() const;

      
      /**
       * Sets the client type options.
       */
      void setClientTypeOptions( const MC2String& clientTypeOptions );

      
      /**
       * Returns the version.
       */
      const MC2String& getVersion() const;

      
      /**
       * Sets the version.
       */
      void setVersion( const MC2String& version );

      
      /**
       * Returns the extra.
       */
      const MC2String& getExtra() const;

      
      /**
       * Sets the extra.
       */
      void setExtra( const MC2String& extra );

      
      /**
       * Returns the origin.
       */
      const MC2String& getOrigin() const;

      
      /**
       * Sets the origin.
       */
      void setOrigin( const MC2String& origin );


      /**
       * Get if UserLastClient is from history.
       */
      bool isHistory() const;


      /**
       * Get the history changer's UIN.
       */
      uint32 getChangerUIN() const;


      /**
       * Get the history change time.
       */
      uint32 getChangeTime() const;


      /**
       * The number of changed fields
       */
      uint32 getNbrChanged() const;


      /**
       * Prints SQL value of field.
       *
       * @param target String to print into.
       * @param field The field to print.
       * @return The number of chars written to string.
       */
      uint32 printValue( char* target, 
                         UserConstants::UserLastClientField field ) const;


      /**
       * Field changed.
       */
      bool changed( UserConstants::UserLastClientField field ) const;


      /**
       * Returns true if element has changes.
       */ 
      virtual bool isChanged() const;


   private:
      /**
       * Reads changes from a packet.
       */
      bool readDataChanges( const Packet* p, int& pos );

   
      /**
       * The client type.
       */
      MC2String m_clientType;


      /**
       * The client type options.
       */
      MC2String m_clientTypeOptions;

   
      /**
       * The version.
       */
      MC2String m_version;

   
      /**
       * The extra.
       */
      MC2String m_extra;

   
      /**
       * The origin.
       */
      MC2String m_origin;


      /**
       * If from history.
       */
      bool m_history;


      /**
       * The history changer.
       */
      uint32 m_changerUIN;


      /**
       * The history time.
       */
      uint32 m_changeTime;


      /**
       * The changed fields.
       */
      bool m_changed[UserConstants::USER_LAST_CLIENT_NBRFIELDS];
};


// -----------------------------------------------------------------
//  Implementation of inlined methods for UserLastClient
// -----------------------------------------------------------------


inline const MC2String&
UserLastClient::getClientType() const {
   return m_clientType;
}


inline const MC2String&
UserLastClient::getClientTypeOptions() const {
   return m_clientTypeOptions;
}


inline const MC2String&
UserLastClient::getVersion() const {
   return m_version;
}


inline const MC2String&
UserLastClient::getExtra() const {
   return m_extra;
}


inline const MC2String&
UserLastClient::getOrigin() const {
   return m_origin;
}


inline bool
UserLastClient::isHistory() const {
   return m_history;
}



inline uint32
UserLastClient::getChangerUIN() const {
   return m_changerUIN;
}


inline uint32
UserLastClient::getChangeTime() const {
   return m_changeTime;
}


#endif //USERDATA_H
