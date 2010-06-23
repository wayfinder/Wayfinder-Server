/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UserConstants.h"

const uint32 UserConstants::MASK_TYPEBITS = 0xf0000000;

const uint32 UserConstants::MASK_ITEMBITS = 0x0fffffff;

const byte UserConstants::NBR_TYPEBITS = 4;

const byte UserConstants::NBR_ITEMBITS = 28;


uint8
UserConstants::PhoneSMSLineLength[UserConstants::PHONEMODEL_NBR] = {
   10,
   12, // xterm
   12, // oldnokia
   12, // newnokia
   10, // oldericsson
   12  // newericsson
} ;

UserConstants::EOLType
UserConstants::PhoneSMSEOLType[UserConstants::PHONEMODEL_NBR] = {
   EOLTYPE_AlwaysCRLF, // unknown phone
   EOLTYPE_AlwaysLF, // xterm
   EOLTYPE_CR,  // oldnokia
   EOLTYPE_AlwaysCR, // newnokia
   EOLTYPE_CR,  // oldericsson
   EOLTYPE_AlwaysCR  // newericsson
};

const char*
UserConstants::UserUserFieldName[UserConstants::USER_NBRFIELDS] = {
   "UIN", 
   "logonID",
   "firstname",
   "initials",
   "lastname",
   "sessionID",
   "measurementSystem",
   "language",
   "lastdestmapID",
   "lastdestitemID",
   "lastdestOffset",
   "lastdestTime",
   "lastdestString",
   "lastorigmapID", 
   "lastorigitemID", 
   "lastorigOffset", 
   "lastorigTime", 
   "lastorigString", 
   "searchType", 
   "searchSubstring", 
   "searchSorting", 
   "searchObject", 
   "routeCostA", 
   "routeCostB", 
   "routeCostC", 
   "routeCostD", 
   "routeType", 
   "editMapRights", 
   "editDelayRights", 
   "editUserRights", 
   "wapService",
   "htmlService",
   "operatorService",
   "nbrMunicipals", 
   "municipals",
   "vehicleType",
   "birthDate",
   "routeImageType",
   "validDate",
   "gender",
   "smsService",
   "defaultCountry",
   "defaultMunicipal",
   "defaultCity",
   "searchDbMask",
   "navService",
   "operatorComment",
   "emailAddress",
   "address1",
   "address2",
   "address3",
   "address4",
   "address5",
   "routeTurnImageType",
   "externalXMLService",
   "transactionBased",
   "deviceChanges",
   "supportComment",
   "postalCity",
   "zipCode",
   "companyName",
   "companyReference",
   "companyVATNbr",
   "emailBounces",
   "addressBounces",
   "customerContactInfo",
};


const char*
UserConstants::UserCellularFieldName[UserConstants::CELLULAR_NBRFIELDS] = {
   "id",
   "userUIN",
   "phoneNumber",
   "model",
   "SMSParams",
   "maxSearchHitsWap",
   "smsService",
   "maxRouteLinesWap",
   "EOLType",
   "CharsPerLine",
   "posActive",
   "typeOfPos",
   "posUserName",
   "posPassword",
   "lastposLat",
   "lastposLong",
   "lastposInnerRadius",
   "lastposOuterRadius",
   "lastposStartAngle",
   "lastposStopAngle",
   "lastposTime", 
};


const char*
UserConstants::
CellularModelFieldName[UserConstants::CELLULAR_MODEL_NBRFIELDS] = {
   "Name",
   "Manufacturer",
   "CharsPerLine",
   "EOLType",
   "DisplayableLines",
   "DynamicWidth",
   "GraphicDisplayWidth",
   "GraphicDisplayHeight",
   "SMSCapable",
   "SMSContatenated",
   "SMSGraphic",
   "WAPCapable",
   "WAPVersion",
   "ModelYear",
   "CommentString"
};


StringTable::stringCode 
UserConstants::EOLTypeFieldSC[ UserConstants::EOLTYPE_NBR + 1 ] = {
   StringTable::EOLTYPE_CR,
   StringTable::EOLTYPE_LF,
   StringTable::EOLTYPE_CRLF,
   StringTable::EOLTYPE_AlwaysCR,
   StringTable::EOLTYPE_AlwaysLF,
   StringTable::EOLTYPE_AlwaysCRLF,
   StringTable::EOLTYPE_NOT_DEFINED,
};


const char* 
UserConstants::SessionFieldName[ UserConstants::SESSION_NBR_FIELDS ] = {
   "sessionID",
   "sessionKey",
   "sessionUIN",
   "lastAccessTime",
   "loginTime",
   "logoutTime",
};


const char* 
UserConstants::UserBuddyListFieldName[ UserConstants::BUDDY_NBRFIELDS ] = {
   "id",
   "userUIN",
   "name",
   "b%u",
};


const char* 
UserConstants::RouteStorageFieldName[ 
   UserConstants::ROUTE_STORAGE_NBRFIELDS ] = 
{
   "routeID",
   "createTime",
   "userUIN",
   "validUntil",
   "extraUserinfo",
   "routePacketLength",
   "routePacketName",
   "originLat",
   "originLon",
   "originMapID",
   "originItemID",
   "originOffset",
   "destinationLat",
   "destinationLon",
   "destinationMapID",
   "destinationItemID",
   "destinationOffset",
};


const char* 
UserConstants::UserLicenceKeyFieldName[ 
   UserConstants::USER_LICENCE_KEY_NBRFIELDS ] = 
{
   "id",
   "userUIN",
   "licenceKey",
   "product",
   "keyType",
};



const char* 
UserConstants::UserRegionAccessFieldName[ 
   UserConstants::USER_REGION_ACCESS_NBRFIELDS ] = 
{
   "id",
   "userUIN",
   "regionID",
   "startTime",
   "endTime",
};


const char* 
UserConstants::UserRightFieldName[
   UserConstants::USER_RIGHT_NBRFIELDS ] = 
{
   "id",
   "userUIN",
   "addTime",
   "type",
   "regionID",
   "startTime",
   "endTime",
   "deleted",
   "origin",
};


const char* 
UserConstants::UserWayfinderSubscriptionFieldName[ 
   UserConstants::USER_WAYFINDER_SUBSCRIPTION_NBRFIELDS ] = 
{
   "id",
   "userUIN",
   "type",
};


const char* 
UserConstants::UserTokenFieldName[ 
   UserConstants::USER_TOKEN_NBRFIELDS ] = 
{
   "id",
   "userUIN",
   "createTime",
   "age",
   "token",
   "group_name",
};


const char* 
UserConstants::UserPINFieldName[ 
   UserConstants::USER_PIN_NBRFIELDS ] = 
{
   "id",
   "userUIN",
   "PIN",
   "comment",
};


const char* 
UserConstants::UserIDKeyFieldName[ 
   UserConstants::USER_ID_KEY_NBRFIELDS ] = 
{
   "id",
   "userUIN",
   "type",
   "idkey",
};


const char* 
UserConstants::UserLastClientFieldName[ 
   UserConstants::USER_LAST_CLIENT_NBRFIELDS ] = 
{
   "id",
   "userUIN",
   "client_type",
   "client_type_options",
   "version",
   "extra",
   "origin",
};


StringTable::stringCode 
UserConstants::MeasurementTypeSC[ UserConstants::MEASUREMENTTYPE_NBR ] = {
   StringTable::IMPERIAL,
   StringTable::METRIC,
};


//  StringTable::stringCode 
//  UserConstants::posTypeFieldSC[ UserConstants::POSTYPE_NBRFIELDS] = {
//     StringTable::POSTYPE_NO_POSITIONING,
//  };

