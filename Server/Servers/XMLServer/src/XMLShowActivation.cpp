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


#ifdef USE_XML
#include "UserData.h"
#include "XMLTool.h"
#include "XMLAuthData.h"
#include "TimeUtility.h"
#include "ParserActivationHandler.h"
#include "XMLServerElements.h"

using namespace XMLTool;

bool
XMLParserThread::xmlParseShowActivationcode(  DOMNode* cur, 
                                              DOMNode* out,
                                              DOMDocument* reply,
                                              bool indent )
{
   bool ok = true;
   int indentLevel = 1;
   MC2String errorCode;      //errorcode if one should be sent in reply.
   MC2String errorMessage;   //errorMessage if one should be sent in reply

   DOMElement* ac_reply = 
      reply->createElement( X( "show_activationcode_reply" ) );
   out->appendChild( ac_reply );

   MC2String transaction_id;
   getAttrib( transaction_id, "transaction_id", cur );
   ac_reply->setAttribute( X( "transaction_id" ), X( transaction_id ) );

   MC2String activationCode;
   getAttrib( activationCode, "actvationcode", cur );

   if ( m_user->getUser()->getEditUserRights() ) {
      uint32 ownerUIN = 0;
      MC2String serverStr;
      MC2String rightStr;

      int res = getActivationHandler()->getActivationData( 
         activationCode.c_str(),
         rightStr,
         ownerUIN,
         serverStr );
      if ( res == 0 ) {
         mc2dbg4 << "Got activation data " << rightStr << " Owner "
                 << ownerUIN
                 << endl;
         // Add data to reply
         ac_reply->setAttribute( X( "rights" ), X( rightStr ) );
         ac_reply->setAttribute( X( "server" ), X( serverStr ) );
         ac_reply->setAttribute( X( "ownerUIN" ), XUint32( ownerUIN ) );
      } else {
         mc2log << warn << "[XSA] Failed to get activation data "
                << "for " << MC2CITE(activationCode) << endl;
         ok = false;
         errorCode = "-1";
         errorMessage = "Problem with activation code "; 
         if ( res == -2 ) {
            errorCode = "-3";
            errorMessage.append( "Timeout" );
         } else if ( res == -5 ) {
            errorCode = "-302";
            errorMessage.append( "Bad activation code" );
         } // Else -1
      }


   } else {
      errorCode = "-201";
      errorMessage = "Access denied.";
      ok = false;
   }

   if ( !ok ) {
      XMLServerUtility::
         appendStatusNodes( ac_reply, reply, 0, false,
                            errorCode.c_str(), errorMessage.c_str() );
      ok = true; // Error handled
   }
   

   if ( indent ) {
      XMLUtility::indentPiece( *ac_reply, indentLevel );
   }
   return ok;

}


#endif
