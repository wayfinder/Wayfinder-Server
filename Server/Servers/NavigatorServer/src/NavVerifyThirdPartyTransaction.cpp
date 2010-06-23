/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavVerifyThirdPartyTransaction.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "NavRequestData.h"
#include "ParserAppStoreHandler.h"
#include "STLStringUtility.h"

NavVerifyThirdPartyTransactionHandler::NavVerifyThirdPartyTransactionHandler( 
   InterfaceParserThread* thread,
   NavParserThreadGroup* group )
      : NavHandler( thread, group )
{
   m_expectations.push_back( ParamExpectation( 6400, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6401, NParam::String ) );
}


NavVerifyThirdPartyTransactionHandler::~NavVerifyThirdPartyTransactionHandler()
{
}

bool
NavVerifyThirdPartyTransactionHandler::handleVerifyThirdPartyTransaction( 
   NavRequestData& rd )
{
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }

   bool ok = true;

   // Start parameter printing
   mc2log << info << "handleVerifyThirdPartyTransaction:";

   stringstream inputData;
   // Verification String
   MC2String verifyString;
   if ( getParam( 6400, verifyString, rd.params ) ) {
      inputData << "  " << verifyString;
   }
   // Error Data Selection String
   MC2String selectionString;
   if ( getParam( 6401, selectionString, rd.params ) ) {
      inputData << "  " << selectionString;
   }

   mc2log << inputData.str();

   // End parameter printing
   mc2log << endl;

   // Split selectionString on ;
   vector<MC2String> selectionArray = STLStringUtility::explode( 
      ";", selectionString );
   
   if ( selectionArray.size() == 2 ) {
      if ( selectionArray[ 0 ] == "iPhoneAppStore" ) {
         // verifyString to AppStoreHandler
         bool verified = m_thread->getAppStore().verifyReceipt( verifyString );
         if ( ! verified ) {
            rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         }
         mc2log << info << "handleVerifyThirdPartyTransaction: Verify "
                << (verified ? "Successful" : "Failed" ) << endl;
      } else {
         mc2log << "handleVerifyThirdPartyTransaction: unknown error data "
                << "type : " << MC2CITE( selectionArray[ 0 ] )  << endl;
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
      }
   } else {
      mc2log << "handleVerifyThirdPartyTransaction: bad format on "
             << "Selection String: " << MC2CITE( selectionString )  << endl;
      rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
   }

   return ok;
}

