/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserThread.h"

#include "SearchParserHandler.h"
#include "XMLTool.h"
#include "XMLUtility.h"
#include "STLStringUtility.h"
#include "MC2CRC32.h"
#include "XMLServerElements.h"
#include "XPathExpression.h"
#include "XMLTool.h"

namespace {

// This is just temporary, most of these stuff
// will be moved to Servers/{include, src}
// so navigator can use them.


/// ad debit types
enum AdDebitTypes { 
   SAVE_FAVORITE = 1, ///< save as favorite
   NAVIGATE_TO,       ///< navigate to
   VIEW_DETAILS,      ///< view details
   CALL,              ///< call
   OPEN_WEB,          ///< open web
   SET_AS_START,      ///< set as start
   SHOW_IN_MAP,       ///< show in map
   SEND_TO_FRIEND     ///< send to a friend
};

MC2String translateAdDebitType( uint32 typeID ) {
#define ADDEBIT_STRING(x) case x: return #x
   switch ( typeID ) {
      ADDEBIT_STRING( SAVE_FAVORITE );
      ADDEBIT_STRING( NAVIGATE_TO );
      ADDEBIT_STRING( VIEW_DETAILS );
      ADDEBIT_STRING( CALL );
      ADDEBIT_STRING( OPEN_WEB );
      ADDEBIT_STRING( SET_AS_START );
      ADDEBIT_STRING( SHOW_IN_MAP );
      ADDEBIT_STRING( SEND_TO_FRIEND );
   default:
      break;
   }
   return "";
#undef ADDEBIT_STRING
}
}

bool
XMLParserThread::xmlParseAdDebitRequest( DOMNode* cur,
                                         DOMNode* out,
                                         DOMDocument* reply,
                                         bool indent )
try { 
   DOMElement* root = XMLUtility::createStandardReply( *reply, *cur,
                                                       "ad_debit_reply" );

   out->appendChild( root );

   uint32 numAds;
   XMLTool::getAttrib( numAds, "count", cur, (uint32)0 );

   // get all ad_debit nodes
   using XMLTool::XPath::Expression;
   Expression adDebitExp("ad_debit_request/ad_debit*");
   Expression::result_type nodes = adDebitExp.evaluate( cur );

   if ( numAds != nodes.size() ) {
      mc2log << warn << "[AdDebitRequest] 'count' attribute value differs"
             << " from actual number of ad_debit nodes. Will use the actual"
             << " number of nodes." << endl;
   }

   // go throu all ad_debit nodes and do logging

   for ( uint32 i = 0; i < nodes.size(); ++i ) try {
      // just do logging for now
      uint32 type;
      XMLTool::getAttrib( type, "type", nodes[ i ] );
      MC2String itemID;
      XMLTool::getNodeValue( itemID, "itemid", nodes[ i ], MC2String() );

      mc2log << "[AdDebitRequest] debit: " 
             << ::translateAdDebitType( type ) << " :  " << itemID << endl;

   } catch ( const XMLTool::Exception& e ) {
      mc2log << warn << "[AdDebitRequest] While parsing ad_debit node: "
             << e.what() << endl;
   }

   return true;

} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[]  " << e.what() << endl;
   XMLServerUtility::
      appendStatusNodes( out->getFirstChild(), reply, 1, false,
                         "-1", e.what() );
   return false;

} 
