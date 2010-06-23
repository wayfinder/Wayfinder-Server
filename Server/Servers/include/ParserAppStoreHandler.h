/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSER_APP_STORE_HANDLER_H
#define PARSER_APP_STORE_HANDLER_H

#include "config.h"
#include "ParserHandler.h"
#include "MC2String.h"
#include "OperationTypes.h"
#include "MC2Coordinate.h"
#include "LangTypes.h"
#include <set>

// Forwards
class PurchaseOptions;
class ClientSetting;
class HttpInterfaceRequest;
class URLFetcher;

namespace MC2JSON {
namespace JPath {
class MultiExpression;
} // JPath
} // MC2JSON

class AppStoreReceipt;


/**
 * Handles App Store related actions, like checking if a user has 
 * access to to things and then to buy it.
 */
class ParserAppStoreHandler : public ParserHandler {
public:
   /**
    * Constructor.
    */
   ParserAppStoreHandler( ParserThread* thread,
                          ParserThreadGroup* group );
   ~ParserAppStoreHandler();

   /**
    * Checks if the client is allowed to consume the service.
    * It will return true if the client is allowed to use the service and
    * and false if the user can not.
    *
    * @param clientSetting The setting to check.
    * @param httpRequest The http request.
    * @param operationType The type of service to consume.
    * @param url An url with possible packages to purchase if the user is
    *            not allowed to consume the service.
    * @param checkedServiceIDs A set with the service ids we have already
    *                          queried.
    * @param topRegionID The top region id for the service requested.
    * @param clientLang The client's language type. Default is set to
    *                   English.
    * @param mayUseCachedCheckResult If the result of a previous check 
    *                                may be used as return for this call.
    * @return True if checked and user has access, false if not.
    */
   bool checkService( const ClientSetting* clientSetting,
                      const HttpInterfaceRequest* httpRequest,
                      OperationType::operationType operationType,
                      PurchaseOptions& purchaseOptions,
                      set< MC2String >& checkedServiceIDs,
                      uint32 topRegionID = MAX_UINT32,
                      LangType::language_t clientLang = LangTypes::english,
                      bool mayUseCachedCheckResult = false );

   /**
    * Verify a App Store receipt.
    * If a purchase was made set the correct right for that particular user.
    *
    * @param receipt The receipt to check.
    * @return True if verification and adding rights succeeded.
    */
   bool verifyReceipt( const MC2String& receipt );

private:

   /**
    * The internal verifyReceipt that doesn't log to debit log.
    * And sets some params.
    * 
    * @param receipt The receipt to check.
    * @param errorReason Set to a string describing the reason for the
    *                    method returning false.
    * @return True if verification and adding rights succeeded.
    */
   bool internalVerifyReceipt( const MC2String& receipt,
                               MC2String& errorReason );

   auto_ptr<URLFetcher> m_urlFetcher;
   /// The App Store receipt.
   auto_ptr< AppStoreReceipt > m_receipt;
   auto_ptr< MC2JSON::JPath::MultiExpression > m_expr;
};


#endif // PARSER_APP_STORE_HANDLER_H

