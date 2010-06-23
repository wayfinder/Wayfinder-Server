/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERCONSTANTS_H
#define USERCONSTANTS_H

#include "config.h"
#include "StringTable.h"

/**
  *   This class contains all constants for the user module. There is no
  *   code for this class, and there may not be any instances of it.
  *
  */
class UserConstants
{
   public:

      static const byte NBR_TYPEBITS;

      static const byte NBR_ITEMBITS;

      static const uint32 MASK_TYPEBITS;

      static const uint32 MASK_ITEMBITS;

      enum UserTable {
         TABLE_SUBSCRIPTION = 0,
         TABLE_USER,
         TABLE_VEHICLE,
         TABLE_CELLULAR,
         TABLE_PHONE,
         TABLE_ADDRESS,
         TABLE_EMAIL,

         TABLE_NBR
      };

      enum UserAction {
         ACTION_NEW,
         ACTION_DELETE,
         ACTION_CHANGE,
         ACTION_NOP
      };

      
      /**
       * Types of UserElements. Number of Types are in TYPE_NBR.
       */
      enum UserItemType {
         TYPE_ILLEGAL = 0x0,
         TYPE_SUBSCRIPTION = 0x1,
         TYPE_USER = 0x2,
         TYPE_VEHICLE = 0x4,
         TYPE_ADDRESS = 0x8,
         TYPE_CELLULAR = 0x10,
         TYPE_EMAIL = 0x20,
         TYPE_PHONE = 0x40,
         TYPE_BUDDY = 0x80,
         TYPE_NAVIGATOR = 0x160,
         TYPE_LICENCE_KEY = 0x200,
         TYPE_REGION_ACCESS = 0x400,
         TYPE_WAYFINDER_SUBSCRIPTION = 0x800,
         TYPE_RIGHT = 0x1000,
         TYPE_TOKEN = 0x2000,
         TYPE_PIN   = 0x4000,
         TYPE_ID_KEY = 0x8000,
         TYPE_LAST_CLIENT = 0x10000,
         /* TYPE_NBR Below */
         TYPE_ALL = MAX_INT32
      };
      

      /**
       * Number of UserItemTypes.
       */
      enum UserItemTypeNbr {
        TYPE_NBR = 14
      };


      enum UserSubscriptionField {
         SUBSCRIPTION_SUBSCRIBER,
         SUBSCRIPTION_DEBITADDRESS,
         SUBSCRIPTION_INFOADDRESS,
         SUBSCRIPTION_NBRFIELDS
      };

      enum UserDataField {
         USER_UIN = 0,
         USER_LOGONID,
         USER_FIRSTNAME,
         USER_INITIALS,
         USER_LASTNAME,
         USER_SESSION,
         USER_MEASUREMENT_SYSTEM,
         USER_LANGUAGE,

         USER_LASTDEST_MAPID,
         USER_LASTDEST_ITEMID,
         USER_LASTDEST_OFFSET,
         USER_LASTDEST_TIME,
         USER_LASTDEST_STRING,
       
         USER_LASTORIG_MAPID,
         USER_LASTORIG_ITEMID,
         USER_LASTORIG_OFFSET,
         USER_LASTORIG_TIME,
         USER_LASTORIG_STRING,
         USER_SEARCH_TYPE,
         USER_SEARCH_SUBSTRING,
         USER_SEARCH_SORTING,
         USER_SEARCH_OBJECTS,

         USER_ROUTING_COST_A,
         USER_ROUTING_COST_B,
         USER_ROUTING_COST_C,
         USER_ROUTING_COST_D,
         USER_ROUTING_TYPE,

         USER_EDIT_MAP_RIGHTS,
         USER_EDIT_DELAY_RIGHTS,
         USER_EDIT_USER_RIGHTS,

         USER_WAP_SERVICE,
         USER_HTML_SERVICE,
         USER_OPERATOR_SERVICE,

         USER_NBRMUNICIPAL,
         USER_MUNICIPAL,

         USER_ROUTING_VEHICLE,
         USER_BIRTHDATE,
         USER_ROUTEIMAGETYPE,
         USER_VALIDDATE,
         USER_GENDER,
         USER_SMS_SERVICE,

         USER_DEFAULT_COUNTRY,
         USER_DEFAULT_MUNICIPAL,
         USER_DEFAULT_CITY,

         USER_SEARCH_DBMASK,

         USER_NAV_SERVICE,
         USER_OPERATOR_COMMENT,
         USER_EMAILADDRESS,
         USER_ADDRESS1,
         USER_ADDRESS2,
         USER_ADDRESS3,
         USER_ADDRESS4,
         USER_ADDRESS5,
         USER_ROUTETURNIMAGETYPE,
         USER_EXTERNALXMLSERVICE,
         USER_TRANSACTIONBASED,
         USER_DEVICECHANGES,
         USER_SUPPORTCOMMENT,
         USER_POSTALCITY,
         USER_ZIPCODE,
         USER_COMPANYNAME,
         USER_COMPANYREFERENCE,
         USER_COMPANYVATNBR,
         USER_EMAILBOUNCES,
         USER_ADDRESSBOUNCES,
         USER_CUSTOMERCONTACTINFO,

         USER_NBRFIELDS
      };

      enum UserVehicleField {
         VEHICLE_TYPE,
         VEHICLE_WEIGHT,
         VEHICLE_HEIGHT,
         VEHICLE_LENGTH,
         VEHICLE_WIDTH,
         VEHICLE_SPEEDLIMIT,

         VEHICLE_NBRFIELDS
      };

      enum VehicleType {
         VEHICLETYPE_UNKNOWN,
         VEHICLETYPE_CAR,
         VEHICLETYPE_PICKUP,
         VEHICLETYPE_VAN,
         VEHICLETYPE_TRUCK,
         VEHICLETYPE_MOTORCYCLE,
         VEHICLETYPE_MOPED,
         VEHICLETYPE_BIKE,
         VEHICLETYPE_BUS,
         VEHICLETYPE_TAXI,
         VEHICLETYPE_RV,
         VEHICLETYPE_ELECTRIC_CAR,

         VEHICLETYPE_NBR
      };

      enum UserAddressField {
         ADDRESS_SUBSCRIPTION,
         ADDRESS_ADDRESS,
         ADDRESS_ZIP,
         ADDRESS_STATE,
         ADDRESS_COUNTRY,
         ADDRESS_TYPE,

         ADDRESS_NBRFIELDS
      };

      enum AddressType {
         ADDRESSTYPE_UNKNOWN,
         ADDRESSTYPE_HOME,
         ADDRESSTYPE_WORK,
         ADDRESSTYPE_COTTAGE,

         ADDRESSTYPE_NBR
      };

      enum UserCellularField {
         CELLULAR_ID = 0,
         CELLULAR_USERUIN,
         CELLULAR_PHONENUMBER,
         CELLULAR_MODEL,
         CELLULAR_SMSPARAMS,
         CELLULAR_MAXSEARCHHITSWAP,
         CELLULAR_MAXROUTELINESWAP,
         CELLULAR_SMSSERVICE,
         CELLULAR_EOL_TYPE,
         CELLULAR_CHARS_PER_LINE,
         CELLULAR_POSACTIVE,
         CELLULAR_TYPE_OF_POS,
         CELLULAR_POS_USERNAME,
         CELLULAR_POS_PASSWORD,
         CELLULAR_LASTPOS_LAT,
         CELLULAR_LASTPOS_LONG,
         CELLULAR_LASTPOS_INNERRADIUS,
         CELLULAR_LASTPOS_OUTERRADIUS,
         CELLULAR_LASTPOS_STARTANGLE,
         CELLULAR_LASTPOS_STOPANGLE,
         CELLULAR_LASTPOS_TIME,

         CELLULAR_NBRFIELDS
      };

      enum UserEmailField {
         EMAIL_USER,
         EMAIL_ADDRESS,
         EMAIL_EMAILSERVICE,

         EMAIL_NBRFIELDS
      };

      enum UserPhoneField {
         PHONE_ADDRESS,
         PHONE_TYPE,
         PHONE_PHONENUMBER,

         PHONE_NBRFIELDS
      };

      enum CellularModelField {
         CELLULAR_MODEL_NAME = 0,
         CELLULAR_MODEL_MANUFACTURER,
         CELLULAR_MODEL_CHARS_PER_LINE,
         CELLULAR_MODEL_EOL_TYPE,
         CELLULAR_MODEL_LINES,
         CELLULAR_MODEL_DYNAMIC_WIDTH,
         CELLULAR_MODEL_GRAPHIC_WIDTH,
         CELLULAR_MODEL_GRAPHIC_HEIGHT,
         CELLULAR_MODEL_SMS_CAPABLE,
         CELLULAR_MODEL_SMS_CONCATENATE,
         CELLULAR_MODEL_SMS_GRAPHIC,
         CELLULAR_MODEL_WAP_CAPABLE,
         CELLULAR_MODEL_WAP_VERSION,
         CELLULAR_MODEL_MODEL_YEAR,
         CELLULAR_MODEL_COMMENT,
         
         CELLULAR_MODEL_NBRFIELDS,
      };
      

      enum UserBuddyListField {
         BUDDY_ID = 0,
         BUDDY_USERUIN,
         BUDDY_NAME,
         BUDDY_BUDDIES,

         BUDDY_NBRFIELDS
      };


      enum RouteStorageField {
         ROUTE_STORAGE_ROUTEID = 0,
         ROUTE_STORAGE_CREATETIME,
         ROUTE_STORAGE_USERUIN,
         ROUTE_STORAGE_VALIDUNTIL,
         ROUTE_STORAGE_EXTRAUSERINFO,
         ROUTE_STORAGE_ROUTEPACKETLENGTH,
         ROUTE_STORAGE_ROUTEPACKETNAME,
         ROUTE_STORAGE_ORIGINLAT,
         ROUTE_STORAGE_ORIGINLON,
         ROUTE_STORAGE_ORIGINMAPID,
         ROUTE_STORAGE_ORIGINITEMID,
         ROUTE_STORAGE_ORIGINOFFSET,
         ROUTE_STORAGE_DESTINATIONLAT,
         ROUTE_STORAGE_DESTINATIONLON,
         ROUTE_STORAGE_DESTINATIONMAPID,
         ROUTE_STORAGE_DESTINATIONITEMID,
         ROUTE_STORAGE_DESTINATIONOFFSET,

         ROUTE_STORAGE_NBRFIELDS
      };


      enum UserLicenceKeyField {
         USER_LICENCE_KEY_ID = 0,
         USER_LICENCE_KEY_USERUIN = 1,
         USER_LICENCE_KEY = 2,
         USER_LICENCE_PRODUCT = 3,
         USER_LICENCE_KEY_TYPE = 4,

         USER_LICENCE_KEY_NBRFIELDS
      };


      enum UserRegionAccessField {
         USER_REGION_ACCESS_ID = 0,
         USER_REGION_ACCESS_USERUIN = 1,
         USER_REGION_ACCESS_REGION_ID = 2,
         USER_REGION_ACCESS_START_TIME = 3,
         USER_REGION_ACCESS_END_TIME = 4,

         USER_REGION_ACCESS_NBRFIELDS
      };


      enum UserRightField {
         USER_RIGHT_ID = 0,
         USER_RIGHT_USERUIN = 1,
         USER_RIGHT_ADD_TIME = 2,
         USER_RIGHT_TYPE = 3,
         USER_RIGHT_REGION_ID = 4,
         USER_RIGHT_START_TIME = 5,
         USER_RIGHT_END_TIME = 6,
         USER_RIGHT_DELETED = 7,
         USER_RIGHT_ORIGIN = 8,

         USER_RIGHT_NBRFIELDS
      };


      enum UserWayfinderSubscriptionField {
         USER_WAYFINDER_SUBSCRIPTION_ID = 0,
         USER_WAYFINDER_SUBSCRIPTION_USERUIN = 1,
         USER_WAYFINDER_SUBSCRIPTION_TYPE    = 2,

         USER_WAYFINDER_SUBSCRIPTION_NBRFIELDS
      };


      enum UserTokenField {
         USER_TOKEN_ID             = 0,
         USER_TOKEN_USERUIN        = 1,
         USER_TOKEN_CREATE_TIME    = 2, 
         USER_TOKEN_AGE            = 3,
         USER_TOKEN_TOKEN          = 4,
         USER_TOKEN_GROUP          = 5,

         USER_TOKEN_NBRFIELDS
      };

      enum UserPINField {
         USER_PIN_ID             = 0,
         USER_PIN_USERUIN        = 1,
         USER_PIN_PIN            = 2, 
         USER_PIN_COMMENT        = 3,

         USER_PIN_NBRFIELDS
      };


      enum UserIDKeyField {
         USER_ID_KEY_ID       = 0,
         USER_ID_KEY_USERUIN  = 1,
         USER_ID_KEY_TYPE     = 2,
         USER_ID_KEY_KEY      = 3,

         USER_ID_KEY_NBRFIELDS
      };


      enum UserLastClientField {
         USER_LAST_CLIENT_ID                   = 0,
         USER_LAST_CLIENT_USERUIN              = 1,
         USER_LAST_CLIENT_CLIENT_TYPE          = 2,
         USER_LAST_CLIENT_CLIENT_TYPE_OPTIONS  = 3,
         USER_LAST_CLIENT_VERSION              = 4,
         USER_LAST_CLIENT_EXTRA                = 5,
         USER_LAST_CLIENT_ORIGIN               = 6,

         USER_LAST_CLIENT_NBRFIELDS
      };


      enum PhoneType {
         PHONETYPE_UNKNOWN,
         PHONETYPE_NORMAL,
         PHONETYPE_FAX,
         PHONETYPE_ISDN,

         PHONETYPE_NBR
      };

      enum MeasurementType {
         MEASUREMENTTYPE_IMPERIAL,
         MEASUREMENTTYPE_METRIC,

         MEASUREMENTTYPE_NBR
      };


      enum UserGroup {
         ALL_GROUP,
         ROUTE_GROUP,
         SEARCH_GROUP,
         SEARCH_AND_ROUTE_GROUP,
      };

      
      enum PhoneModel {
         PHONEMODEL_UNKNOWN,
         PHONEMODEL_DEBUG_XTERM,
         PHONEMODEL_OLDNOKIA,
         PHONEMODEL_NEWNOKIA,
         PHONEMODEL_OLDERICSSON,
         PHONEMODEL_NEWERICSSON,

         PHONEMODEL_NBR
      };


      enum SessionField {
         SESSION_SESSIONID = 0,
         SESSION_SESSIONKEY,
         SESSION_UIN,
         SESSION_LAST_ACCESS_TIME,
         SESSION_LOGIN_TIME,
         SESSION_LOGOUT_TIME,

         SESSION_NBR_FIELDS
      };
      
      enum EOLType {
         EOLTYPE_CR = 0,
         EOLTYPE_LF,
         EOLTYPE_CRLF,
            // The Always* EOLTYPE_s are for always using the specified
            // new line character combination instead of spaces.
            // This is to handle phones with variable width font.
         EOLTYPE_AlwaysCR,
         EOLTYPE_AlwaysLF,
         EOLTYPE_AlwaysCRLF,
         EOLTYPE_NBR,
         // Not defined EOL
         EOLTYPE_NOT_DEFINED = 255,         
      };

      enum RouteImageType {
         ROUTEIMAGETYPE_NONE = 0,
         ROUTEIMAGETYPE_JAVA_APPLET,
         ROUTEIMAGETYPE_IMAGE,

         ROUTEIMAGETYPE_NBR 
      };

      enum GenderType {
         GENDERTYPE_MALE = 0,
         GENDERTYPE_FEMALE,

         GENDERTYPE_NBR
      };

      enum posType {
         POSTYPE_NO_POSITIONING = 0,
         
         POSTYPE_NBRFIELDS
      };
         

      enum navigatorType {
         NAVIGATORTYPE_EBOX,
         NAVIGATORTYPE_NAVIGATOR,

         NAVIGATORTYPE_NBR
      };


      enum navigatorMessageType {
         NAVIGATORMESSAGETYPE_PICKMEUP,
         NAVIGATORMESSAGETYPE_HTTP,

         NAVIGATORMESSAGETYPE_NBR
      };

      enum RouteTurnImageType {
         ROUTE_TURN_IMAGE_TYPE_PICTOGRAM = 0,
         ROUTE_TURN_IMAGE_TYPE_MAP_IMAGE = 1,
         ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_1 = 2,
         ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_2 = 3,
         ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_3 = 4,
         ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_4 = 5,
         ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_5 = 6,

         ROUTE_TURN_IMAGE_TYPE_NBR 
      };


      /**
       * The types of transactions a user has.
       */
      enum transactionBased_t {
         /// No transactions
         NO_TRANSACTIONS = 0,
         /// Transactions per request (search,route,map)
         TRANSACTIONS = 1,
         /// Transactions per 24h days
         TRANSACTION_DAYS = 2,

         /// Nbr transactionbased types
         NBR_TRANSACTION_T
      };

      
      static uint8 PhoneSMSLineLength[];
      static EOLType PhoneSMSEOLType[];

      /// UserUser fields names in SQL
      static const char* UserUserFieldName[];


      /// UserCellular fields names in SQL
      static const char* UserCellularFieldName[];


      /// CellularModel fields names in SQL
      static const char* CellularModelFieldName[];


      /// EOLType as stringCodes
      static StringTable::stringCode EOLTypeFieldSC[];


      /// Session fields names in SQL
      static const char* SessionFieldName[];


      /// UserBuddyList fields names in SQL
      static const char* UserBuddyListFieldName[];


      /// Static RouteStorage fields in SQL
      static const char* RouteStorageFieldName[];


      /// UserLicenceKey fields names in SQL
      static const char* UserLicenceKeyFieldName[];


      /// UserRegionAccess fields names in SQL
      static const char* UserRegionAccessFieldName[];


      /// UserRight fields names in SQL
      static const char* UserRightFieldName[];


      /// UserWayfinderSubscription fields names in SQL
      static const char* UserWayfinderSubscriptionFieldName[];

      /// UserToken fields names in SQL
      static const char* UserTokenFieldName[];


      /// UserPIN fields names in SQL
      static const char* UserPINFieldName[];


      /// UserIDKey fields names in SQL
      static const char* UserIDKeyFieldName[];


      /// UserLastClient fields names in SQL
      static const char* UserLastClientFieldName[];


      /// MeasurementType as stringCodes
      static StringTable::stringCode MeasurementTypeSC[];
};

#endif // USERCONSTANTS_H

