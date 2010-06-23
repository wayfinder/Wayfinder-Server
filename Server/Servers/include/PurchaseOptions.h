/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PURCHASEOPTIONS_H
#define PURCHASEOPTIONS_H

#include "config.h"
#include "MC2String.h"

#include <set>

namespace PurchaseOption {

enum ReasonCode {
   OK = 0,
   /// The connection timed out.
   TIME_OUT = 1,
   /// General error.
   ERROR = 2,
   /// The service, in the region, can not be purchased.
   SERVICE_NOT_FOUND = 32,

   /// If customer needs to buy a third party extension to do the action.
   NEEDS_TO_BUY_APP_STORE_ADDON = 5001,
};

/**
 * Response Package data from authority.
 */
struct Package {
   /// The package id for purchasing the package.
   MC2String m_id;
   /// The name of the package.
   MC2String m_name;
   /// The text to show in the option for the package.
   MC2String m_purchaseText;
};

}

/**
 * A class that contains puchase options.
 */
class PurchaseOptions {
public:

   /**
    * Constructor.
    *
    * @param clientType The client type.
    */
   PurchaseOptions( const MC2String& clientType );

   /// Destructor.
   ~PurchaseOptions();

   /// Sets the reson code.
   void setReasonCode( PurchaseOption::ReasonCode reasonCode );

   /// Returns the reason code.
   PurchaseOption::ReasonCode getReasonCode() const;

   /// Sets the error description.
   void setErrorDescription( const MC2String& errorDescription );

   /**
    * Returns the error description.
    * Is set if the reason code is ERROR.
    */
   const MC2String& getErrorDescription() const;

   /**
    * Returns the url to the purchase page.
    *
    * @return The url.
    */
   const MC2String& getURL() const;

   /**
    * Set the url to something specific.
    */
   void setURL( const MC2String& url );


   /// The set of package ids.
   typedef set< MC2String > packageIDs;

   /**
    * Adds App Store Product id that the user can purchase.
    *
    * @param packageID The package id.
    */
   void addAppPackage( const MC2String& packageID );

   /**
    * Get reference to the App Store package ids.
    *
    * @return A const rference to the App Store package ids.
    */
   const packageIDs& getAppPackages() const;

private:
   /// The client type.
   MC2String m_clientType;
   /// The url to return in the case we got some purchase options.
   MC2String m_url;
   /// The reason code.
   PurchaseOption::ReasonCode m_reasonCode;
   /// The error description, set if reason code is ERROR.
   MC2String m_errorDescription;
   /// A set with the package ids.
   packageIDs m_packageIDs;
};


#endif // PURCHASEOPTIONS_H
