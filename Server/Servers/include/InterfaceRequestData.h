/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INTERFACEREQUESTDATA_H
#define INTERFACEREQUESTDATA_H


#include "config.h"
#include "LangTypes.h"
#include "StringTable.h"
#include "IPnPort.h"
#include "UserEnums.h"
#include "UserLicenceKey.h"

class ClientSetting;

/**
 * Class for holding data for request handling.
 *
 */
class InterfaceRequestData  {
public:
   /**
    * Constructor.
    */
   InterfaceRequestData() { reset(); }

   /**
    * Destructor.
    */
   virtual ~InterfaceRequestData() {}

   /**
    * Reset the data.
    */
   virtual void reset();

   /**
    * The client type.
    */
   MC2String clientType;

   /**
    * If client type is set.
    */
   bool clientTypeSet;

   /**
    * The client type options.
    */
   MC2String clientTypeOptions;

   /**
    * If client type options is set.
    */
   bool clientTypeOptionsSet;

   /**
    * The client's language type.
    */
   LangTypes::language_t clientLang;

   /**
    * The client's language code.
    */
   StringTable::languageCode stringtClientLang;

   /**
    * The ClientSetting.
    */
   const ClientSetting* clientSetting;

   /**
    * If the user is a backup user.
    */
   bool backupUser;

   /**
    * @name Client version.
    * @memo Client version.
    * @doc  Client version.
    */
   //@{
   /**
    * Function for setting all client version variables correctly.
    * @param major Major version.
    * @param minor Minor version.
    * @param mini  Mini version.
    */
   void setClientVersion( uint32 major, uint32 minor, uint32 mini );

   /**
    * Get the client version as "Major.Minor.Mini".
    */
   MC2String getClientVersionAsString() const;
   
   /**
    * Major.
    */
   uint32 clientMajorV;

   /**
    * Minor.
    */
   uint32 clientMinorV;

   /**
    * Mini.
    */
   uint32 clientMiniV;

   /**
    * Client version as array.
    */
   uint32 clientV[3];

   /**
    * Set or not.
    */
   bool clientVSet;
   //@}

   /**
    * The Wayfinder level.
    */
   byte wayfinderType;


   /**
    * If the Wayfinder level is set.
    */
   bool wayfinderTypeSet;

   /**
    * The UR mask for the user.
    */
   UserEnums::URType urmask;

   /**
    * The client's IPnPort.
    */
   IPnPort ipnport;

   /**
    * If external auth was used this is set to the name of external auth.
    */
   MC2String externalAuth;

   /**
    * The licence/hardware key(s).
    */
   UserLicenceKeyVect hwKeys;

   /**
    * Get the licence/hardware key of a specific type, the first if
    * many, NULL if none.
    */
   const UserLicenceKey* getLicenceKeyType( const MC2String& type ) const;

   /**
    * The licence/hardware key that is selected among the hwKeys to represent
    * the client.
    */
   UserLicenceKey hwKey;

   /**
    * Get the licence/hardware key of a specific type, the first if
    * many, NULL if none for a UserLicenceKeyVect.
    */
   static const UserLicenceKey* getLicenceKeyType( const UserLicenceKeyVect& v,
                                                   const MC2String& type );
};


inline void 
InterfaceRequestData::reset() {
   clientType = "";
   clientTypeSet = false;
   clientTypeOptions = "";
   clientTypeOptionsSet = false;
   clientLang = LangTypes::english;
   stringtClientLang = StringTable::ENGLISH;
   clientSetting = NULL;
   backupUser = false;
   setClientVersion( 0, 0, 0 );
   clientVSet = false;
   wayfinderType = MAX_BYTE;
   wayfinderTypeSet = false;
   urmask = UserEnums::URType( UserEnums::TSG_MASK, UserEnums::UR_WF );
   ipnport = IPnPort( 0, 0 );
   externalAuth = "";
   hwKeys.clear();
   hwKey.setLicence( "" );
}

inline 
void InterfaceRequestData::setClientVersion(
   uint32 major, uint32 minor, uint32 mini )
{
   clientMajorV = major;
   clientMinorV = minor;
   clientMiniV  = mini;
   clientV[ 0 ] = clientMajorV;
   clientV[ 1 ] = clientMinorV;
   clientV[ 2 ] = clientMiniV;
   clientVSet = true;
}


#endif // INTERFACEREQUESTDATA_H

