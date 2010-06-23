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
#include "ParserAppStoreHandler.h"
#include "ParserThread.h"
#include "ParserThreadGroup.h"
#include "ParserActivationHandler.h"
#include "PurchaseOptions.h"
#include "UserData.h"
#include "InterfaceRequestData.h"
#include "ParserDebitHandler.h"
#include "STLStringUtility.h"
#include "ServerRegionIDs.h"
#include "Properties.h"

#include "URLFetcher.h"
#include "URL.h"

#include <json_spirit/json_spirit_reader_template.h>

#include "JPathMultiExpression.h"
#include "JPathAssigner.h"
#include "JPathMatchHandler.h"

struct AppStoreReceipt {

   AppStoreReceipt():
      m_status( MAX_UINT32 ) {}

   /**
    * If the value of the status key is 0, this is a valid receipt.
    * If the value is anything other than 0, this receipt is invalid.
    */
   int m_status;

   /// The number of items purchased.
   MC2String m_quantity;

   ///  The product identifier of the item that was purchased.
   MC2String m_productID;

   /// The transaction identifier of the item that was purchased.
   MC2String m_transactionID;

   /// The date and time this transaction occurred.
   MC2String m_purchaseDate;

   /**
    * For a transaction that restores a previous transaction, this holds the
    * original transaction identifier.
    */
   MC2String m_originalTransactionID;

   /**
    * For a transaction that restores a previous transaction, this holds the
    * original purchase date.
    */
   MC2String m_originalPurchaseDate;

   /**
    * A string that the App Store uses to uniquely identify the iPhone application
    * that created the payment transaction. If your server supports multiple iPhone
    * applications, you can use this value to differentiate between them. Applications
    * that are executing in the sandbox do not yet have an app-item-id assigned
    * to them, so this key is missing from receipts created by the sandbox.
    */
   MC2String m_appItemID;

   /**
    * An arbitrary number that uniquely identifies a revision of your application.
    * This key is missing in receipts created by the sandbox.
    */
   MC2String m_versionExternalIdentifier;

   /// The bundle identifier for the iPhone application.
   MC2String m_bid;

   /// A version number for the iPhone application.
   MC2String m_bvrs;
};

ParserAppStoreHandler::ParserAppStoreHandler( ParserThread* thread,
                                              ParserThreadGroup* group ):
   ParserHandler( thread, group ),
   m_urlFetcher( new URLFetcher() ),
   m_receipt( new AppStoreReceipt() )
{
   using namespace MC2JSON;
   using namespace JPath;
   // setup path expression
   MC2JSON::JPath::MultiExpression::NodeDescription desc[] = {
      { "status", makeAssigner( m_receipt->m_status ) },
      { "receipt/quantity", makeAssigner( m_receipt->m_quantity ) },
      { "receipt/product_id", makeAssigner( m_receipt->m_productID ) },
      { "receipt/transaction_id", makeAssigner( m_receipt->m_transactionID ) },
      { "receipt/purchase_date", makeAssigner( m_receipt->m_purchaseDate ) },
      { "receipt/original_transaction_id",
         makeAssigner( m_receipt->m_originalTransactionID ) },
      { "receipt/original_purchase_date",
         makeAssigner( m_receipt->m_originalPurchaseDate ) },
      { "receipt/app_item_id", makeAssigner( m_receipt->m_appItemID ) },
      { "receipt/version_external_identifier",
         makeAssigner( m_receipt->m_versionExternalIdentifier ) },
      { "receipt/bid", makeAssigner( m_receipt->m_bid ) },
      { "receipt/bvrs", makeAssigner( m_receipt->m_bvrs ) },
   };

   uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );

   m_expr.reset( new MultiExpression( MultiExpression::
                                      Description( desc, desc + descSize ) ) );
}

ParserAppStoreHandler::~ParserAppStoreHandler() {
}

bool
ParserAppStoreHandler::checkService( 
   const ClientSetting* clientSetting,
   const HttpInterfaceRequest* httpRequest,
   OperationType::operationType operationType,
   PurchaseOptions& purchaseOptions,
   set< MC2String >& checkedServiceIDs,
   uint32 topRegionID,
   LangType::language_t clientLang,
   bool mayUseCachedCheckResult )
{
   // Only check rights for route.
   if ( operationType == OperationType::ROUTE_RIGHT ) {
      if ( ! m_thread->checkAccessToService(
              m_thread->getCurrentUser()->getUser(),
              UserEnums::UR_APP_STORE_NAVIGATION,
              UserEnums::UR_NO_LEVEL,
              topRegionID,
              true, true ) ) {
#if 0
         // User has no rights to route in the chosen region. Return options
         // to purchase in the App Store.
         purchaseOptions.setReasonCode( PurchaseOption::
                                        NEEDS_TO_BUY_APP_STORE_ADDON );
         // Add products to buy here
         MC2String appStoreID( "com." ); // FIXME: App Store ID here, without country
         // Add the country iso code to the app store id
         appStoreID.append( "SWE" ); // FIXME: Not hardcoded

         checkedServiceIDs.insert( appStoreID );
         purchaseOptions.addAppPackage( appStoreID );
         purchaseOptions.setErrorDescription( "Needs to buy package: " +
                                              appStoreID );

         return false;
#endif
      }
   }

   return true;
}

MC2String createJSONTextObject( const MC2String& receipt ) {
   MC2String jsonTextObj( "{\n\t\"receipt-data\" : \"" );
   jsonTextObj += StringUtility::base64Encode( receipt ) + "\"\n}";

   return jsonTextObj;
}

bool addRights( ParserThread* thread, const AppStoreReceipt& receipt,
                MC2String& errorReason ) {

   bool ok = true;

   // Get the topregion from the receipt m_productID, made in checkService
   // above.
   uint32 purchadedTopRegionID = MAX_UINT32;
   {
      const MC2String& str( receipt.m_productID );
      size_t lastDot = str.rfind( "." );
      if ( lastDot != MC2String::npos ) {
         purchadedTopRegionID = thread->getGroup()->getRegionIDs()->
            getRegionIdFromIsoName( str.substr( lastDot + 1 ) );
      }
   }

   if ( purchadedTopRegionID == MAX_UINT32 ) {
      mc2log << "[ParserAppStoreHandler]:addRights couldn't get country from "
             << receipt.m_productID << endl;
      errorReason = "Couldn't get country from productID: " + 
         receipt.m_productID;
      return false;
   }

   // "APP_STORE_NAV(12m,2097152)"
   MC2String purchedRights( "APP_STORE_NAV(12m," );
   purchedRights.append( STLStringUtility::uint2str( purchadedTopRegionID ) );
   purchedRights.append( ")" );

   // add rights to user but first we need a local copy to modify
   uint32 currentTime = TimeUtility::getRealTime();
   UserUser userToChange( *thread->getCurrentUser()->getUser() );
   thread->getActivationHandler()->addUserRights( 
      &userToChange,
      "", // phoneNumber
      "", // cellularModel
      MAX_UINT32, // selected topRegionID
      purchedRights,
      receipt.m_transactionID + receipt.m_productID,
      currentTime );

   // Change the user to the updated one with new rigths
   if ( ! thread->changeUser( &userToChange, NULL ) ) {
      mc2log << "[ParserAppStoreHandler]:addRights failed to change user "
             << thread->getCurrentUser()->getUser()->getLogonID()
             << " (" << thread->getCurrentUser()->getUIN() << ")" << endl;
      errorReason = "Failed to change user in db";
      ok = false;
   }

   return ok;
}

bool
ParserAppStoreHandler::verifyReceipt( const MC2String& receipt ) {
   MC2String errorReason;
   bool success = internalVerifyReceipt( receipt, errorReason );

   // Log the result
   m_thread->getDebitHandler()->makeAppStoreVerifyDebit( 
      m_thread->getCurrentUser(), "", OperationType::APP_STORE_VERIFY, success,
      errorReason, m_receipt->m_productID, m_receipt->m_transactionID );

   return success;
}

bool
ParserAppStoreHandler::internalVerifyReceipt( const MC2String& receipt,
                                              MC2String& errorReason ) {

   MC2String url( Properties::
                  getProperty( "APP_STORE_URL",
                               "https://sandbox.itunes.apple.com/verifyReceipt" ) );
                               //"https://buy.itunes.apple.com/verifyReceipt" ) );

   // Clear the receipt before we do an eval, and early so old is not logged
   *m_receipt = AppStoreReceipt();

   MC2String jsonResult;
   int ures;
   ures = m_urlFetcher->post( jsonResult, url,
                              createJSONTextObject( receipt ),
                              5000 ); // timeout in ms

   if ( jsonResult.empty() ) {
      mc2log << warn << "[ParserAppStoreHandler]:verifyReceipt empty reply "
             << " HTTP status code " << MC2CITE( ures ) << endl;
      errorReason = "Empty reply, status " + 
         STLStringUtility::int2str( ures );
      return false;
   }

   mc2dbg << "[ParserAppStoreHandler]:verifyReceipt jsonResult: "
          << jsonResult << endl;

   // Parse the reply data and see if the purchase was ok
   MC2JSON::JPath::NodeType jsonNode;
   if ( ! json_spirit::read_string( jsonResult, jsonNode ) ) {
      mc2log << warn << "[ParserAppStoreHandler]:verifyReceipt"
             << " No result parsed" << endl;
      errorReason = "No result parsed, status " + 
         STLStringUtility::int2str( ures );
      return false;
   }

   // evaluate jpath expression on json document
   m_expr->evaluate( jsonNode );

   if ( m_receipt->m_status != 0 ) {
      mc2log << error << "[ParserAppStoreHandler]:verifyReceipt receipt is "
             << " not valid status is set to " << m_receipt->m_status << endl;
      errorReason = "Error status returned: " + 
         STLStringUtility::int2str( m_receipt->m_status );
      return false;
   }

   if ( ! addRights( m_thread, *m_receipt, errorReason ) ) {
      return false;
   }

   return true;
}
