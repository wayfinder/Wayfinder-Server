/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PurchaseOptions.h"
#include "URLParams.h"

#include "boost/lexical_cast.hpp"


PurchaseOptions::PurchaseOptions( const MC2String& clientType ):
   m_clientType( clientType ),
   m_url(),
   m_reasonCode( PurchaseOption::OK ),
   m_packageIDs() {
}

PurchaseOptions::~PurchaseOptions() {
}

void
PurchaseOptions::setReasonCode( PurchaseOption::ReasonCode reasonCode ) {
   m_reasonCode = reasonCode;
}

PurchaseOption::ReasonCode 
PurchaseOptions::getReasonCode() const {
   return m_reasonCode;
}

void
PurchaseOptions::setErrorDescription( const MC2String& errorDescription ) {
   m_errorDescription = errorDescription;
}

const MC2String&
PurchaseOptions::getErrorDescription() const {
   return m_errorDescription;
}

const MC2String&
PurchaseOptions::getURL() const {
   return m_url;
}

void
PurchaseOptions::setURL( const MC2String& url ) {
   m_url = url;
}

void
PurchaseOptions::addAppPackage( const MC2String& packageID ) {
   m_packageIDs.insert( packageID );
}

const PurchaseOptions::packageIDs&
PurchaseOptions::getAppPackages() const {
   return m_packageIDs;
}
