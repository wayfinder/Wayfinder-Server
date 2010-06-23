/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAV_VERIFY_THIRD_PARTY_TRANSACTION_HANDLER_H
#define NAV_VERIFY_THIRD_PARTY_TRANSACTION_HANDLER_H

#include "config.h"
#include "NavHandler.h"

// Forward declaration
class NavRequestData;

/**
 * Handles requests to verify a third party transaction and apply the
 * result appropriately.
 */
class NavVerifyThirdPartyTransactionHandler : public NavHandler {
public:
   /**
    * Constructor.
    */
   NavVerifyThirdPartyTransactionHandler( InterfaceParserThread* thread,
                                          NavParserThreadGroup* group );

   /**
    * Destructor.
    */
   ~NavVerifyThirdPartyTransactionHandler();

   /**
    * Handle a verify third party transaction id request.
    *
    * @param rd The holder of the request and reply data, reply filled in by
    *           this method.
    * @return True if operation was successful, false if not and then 
    *         reply's status is set.
    */
   bool handleVerifyThirdPartyTransaction( NavRequestData& rd );
};

#endif // NAV_VERIFY_THIRD_PARTY_TRANSACTION_HANDLER_H

