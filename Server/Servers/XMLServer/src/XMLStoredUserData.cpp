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
#include "XMLServerElements.h"
#include "XMLTool.h"

using namespace XMLTool;

bool
XMLParserThread::xmlParseGetStoredUserData( DOMNode* cur, 
                                            DOMNode* out,
                                            DOMDocument* reply,
                                            bool indent )
try {
   bool ok = true;
   int indentLevel = 1;
   MC2String errorCode;      //errorcode if one should be sent in reply.
   MC2String errorMessage;   //errorMessage if one should be sent in reply

   DOMElement* stud_reply = 
      reply->createElement( X( "get_stored_user_data_reply" ) );
   out->appendChild( stud_reply );

   MC2String transaction_id;
   getAttrib( transaction_id, "transaction_id", cur );
   stud_reply->setAttribute( X( "transaction_id" ), X( transaction_id ) );

   uint32 uin;
   MC2String key;
   getAttribValue( uin, "uin", cur );
   getAttribValue( key, "key", cur );
   
   MC2String value;
   int res = getUserHandler()->getStoredUserData( uin, key, value );
   if ( res == 0 ) {
      DOMElement* stored_user_data = addNode( stud_reply, "stored_user_data" );
      addAttrib( stored_user_data, "key", key );
      addAttrib( stored_user_data, "value", value );
      errorCode = "0";
      errorMessage = "";
   } else if ( res == 1 ) {
      // Error
      errorCode = "-800";
      errorMessage = "GetStoredUserData failed key not in database!";
   } else {
      errorCode = "-1";
      errorMessage = "GetStoredUserData failed!";
   }

   XMLServerUtility::
      appendStatusNodes( stud_reply, reply, 0, false,
                         errorCode.c_str(), errorMessage.c_str() );

   if ( indent ) {
      XMLUtility::indentPiece( *stud_reply, indentLevel );
   }
   return ok;

} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[stud]  " << e.what() << endl;
   XMLServerUtility::
      appendStatusNodes( out->getFirstChild(), reply, 1, false,
                         "-1", e.what() );
   return true;

} 


bool
XMLParserThread::xmlParseSetStoredUserData( DOMNode* cur, 
                                            DOMNode* out,
                                            DOMDocument* reply,
                                            bool indent )
try {
   bool ok = true;
   int indentLevel = 1;
   MC2String errorCode;      //errorcode if one should be sent in reply.
   MC2String errorMessage;   //errorMessage if one should be sent in reply

   DOMElement* stud_reply = 
      reply->createElement( X( "set_stored_user_data_reply" ) );
   out->appendChild( stud_reply );

   MC2String transaction_id;
   getAttrib( transaction_id, "transaction_id", cur );
   stud_reply->setAttribute( X( "transaction_id" ), X( transaction_id ) );

   uint32 uin;
   MC2String key;
   MC2String value;
   getAttribValue( uin, "uin", cur );
   DOMNode* stored_user_data = findNode( cur, "stored_user_data" );
   getAttribValue( key, "key", stored_user_data );
   getAttribValue( value, "value", stored_user_data );
   
   int res = getUserHandler()->setStoredUserData( uin, key, value );
   if ( res == 0 ) {
      errorCode = "0";
      errorMessage = "";
   } else {
      errorCode = "-1";
      errorMessage = "SetStoredUserData failed!";
   }

   XMLServerUtility::
      appendStatusNodes( stud_reply, reply, 0, false,
                         errorCode.c_str(), errorMessage.c_str() );

   if ( indent ) {
      XMLUtility::indentPiece( *stud_reply, indentLevel );
   }
   return ok;

} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[stud]  " << e.what() << endl;
   XMLServerUtility::
      appendStatusNodes( out->getFirstChild(), reply, 1, false,
                         "-1", e.what() );
   return true;

} 


#endif
