/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVREQUESTDATA_H
#define NAVREQUESTDATA_H


#include "config.h"
#include "InterfaceRequestData.h"
#include "LangTypes.h"
#include "StringTable.h"
#include "isabBoxInterfaceRequest.h"
#include "IPnPort.h"
#include "NParamBlock.h"
#include "UserEnums.h"
#include "NavPacket.h"


/**
 * Class for holding data for request handlers in ns.
 *
 */
class NavRequestData : public InterfaceRequestData {
public:
   /**
    * Constructor.
    */
   NavRequestData( NParamBlock& p, NParamBlock& r,
                   NavRequestPacket* pp, NavReplyPacket* rp,
                   IsabBoxInterfaceRequest* iq ) 
      : session( iq->getSession() ), params( p ), req( pp ), 
      rparams( r ), reply( rp ), ireq( iq ), reqVer( pp->getReqVer() )
   { reset(); }

   /**
    * Reset the data.
    */
   virtual void reset();

   /**
    * The session.
    */
   isabBoxSession* session;

   /**
    * The request params.
    */
   NParamBlock& params;

   /**
    * The request packet.
    */
   NavRequestPacket* req;

   /**
    * The reply params
    */
   NParamBlock& rparams;

   /**
    * The reply packet.
    */
   NavReplyPacket* reply;

   /**
    * The interface request.
    */
   IsabBoxInterfaceRequest* ireq;

   /**
    * The request version.
    */
   byte reqVer;

   /**
    * The old licence/hardware key(s).
    */
   UserLicenceKeyVect oldHwKeys;

   /**
    * If external auth failed the status code is in this.
    */
   byte externalAuthStatusCode;
};

// ========================================================================
//                                      Implementation of inlined methods =

inline void 
NavRequestData::reset() {
   InterfaceRequestData::reset();
   oldHwKeys.clear();
   externalAuthStatusCode = NavReplyPacket::NAV_STATUS_OK;
}


#endif // NAVREQUESTDATA_H

