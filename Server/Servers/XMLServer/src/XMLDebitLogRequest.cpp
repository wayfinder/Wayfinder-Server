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
#include "UserPacket.h"
#include "UserData.h"
#include "SinglePacketRequest.h"
#include "XMLServerElements.h"
namespace {
/**
 * Appends a debit_log_element to out.
 *
 * @param out Where to put the node.
 * @param reply The document to make the node in.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param el The DebitElement to append.
 */
void appendDebitLogElement( DOMNode* out, 
                            DOMDocument* reply,
                            int indentLevel,
                            bool indent,
                            DebitElement* el );
}

bool 
XMLParserThread::xmlParseUserDebitLogRequest( DOMNode* cur, 
                                              DOMNode* out,
                                              DOMDocument* reply,
                                              bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   MC2String errorCode;
   MC2String errorMessage;

   // User debit log request data
   MC2String userID;
   MC2String user_session_id;
   MC2String user_session_key;

   uint32 startTime = 0;
   uint32 endTime = MAX_UINT32;
   uint32 startIndex = 0;
   uint32 endIndex = 99;
   char* tmpPtr = NULL;
   

   // Create user_debit_log_reply element
   DOMElement* user_debit_log_reply = 
      reply->createElement( X( "user_debit_log_reply" ) );
   // Transaction ID
   user_debit_log_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( user_debit_log_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), 
         user_debit_log_reply );
   }
   // Set attributes to initial values
   user_debit_log_reply->setAttribute( X( "start_index" ), X( "0" ) );
   user_debit_log_reply->setAttribute( X( "end_index" ), X( "0" ) );
   user_debit_log_reply->setAttribute( X( "total_number_elements" ), 
                                       X( "0" ) );


   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() && ok ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "transaction_id" ) ) 
      {
         // Handled above 
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "start_time" ) ) 
      {
         startTime = strtoul( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || tmpPtr[ 0 ] != '\0' ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Problem parsing start_time not a "
               "valid number."; 
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "end_time" ) ) 
      {
         endTime = strtoul( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || tmpPtr[ 0 ] != '\0' ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Problem parsing end_time not a "
               "valid number."; 
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "start_index" ) ) 
      {
         startIndex = strtoul( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || tmpPtr[ 0 ] != '\0' ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Problem parsing start_index not a "
               "valid number."; 
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "end_index" ) ) 
      {
         endIndex = strtoul( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || tmpPtr[ 0 ] != '\0' ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Problem parsing end_index not a "
               "valid number."; 
         }
      } else {
         mc2log << warn << "XMLParserThread::"
                << "xmlParseUserDebitLogRequestRequest "
                << "unknown attribute for user_debit_log_request element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }


   // Get children
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), "user_id" ) ) {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               userID = tmpStr; 
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "user_session_id" ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               user_session_id = tmpStr; 
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "user_session_key" ) )
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               user_session_key = tmpStr; 
               delete [] tmpStr;
            } else {
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseUserDebitLogRequestRequest "
                      << "odd with Element in user_debit_log_request "
                      << "element: " << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                      "xmlParseUserDebitLogRequestRequest odd "
                      "node type in user_debit_log_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }
   
   if ( ok ) {
      if ( !userID.empty() || 
           ( !user_session_id.empty() && !user_session_key.empty() ) )
      {
         UserItem* userItem = NULL;
         bool status = true;

         if ( !userID.empty() ) {
            status = getUser( userID.c_str(), userItem, true );
         } else {
            status = getUserBySession( 
               user_session_id.c_str(), 
               user_session_key.c_str(), userItem, true );
         }

         if ( status ) {
            if ( userItem != NULL ) {
               if ( userItem->getUIN() == m_user->getUIN() ||
                    m_user->getUser()->getEditUserRights()  )
               {
                  // Get debit log
                  SinglePacketRequest* req = new SinglePacketRequest(
                     getNextRequestID(),
                     new PacketContainer( new ListDebitRequestPacket( 
                                             userItem->getUIN(),
                                             startTime, endTime, 
                                             startIndex, endIndex ),
                                          0, 0, MODULE_TYPE_USER ) );
                  mc2dbg4 << "About to send ListDebitRequest" << endl;
                  putRequest( req );
                  mc2dbg4 << "ListDebitRequest done" << endl;
                  PacketContainer* ansCont = req->getAnswer();
                  if ( ansCont != NULL && static_cast<ReplyPacket*>( 
                          ansCont->getPacket() )->getStatus() == 
                       StringTable::OK )
                  {
                     // Append user debit log elements
                     ListDebitReplyPacket* r = 
                        static_cast<ListDebitReplyPacket*>( 
                           ansCont->getPacket() );
                     char tmpStr[256];
                     sprintf( tmpStr, "%u", r->getStartIndex() );
                     user_debit_log_reply->setAttribute( 
                        X( "start_index" ), X( tmpStr ) );
                     sprintf( tmpStr, "%u", r->getEndIndex() );
                     user_debit_log_reply->setAttribute( 
                        X( "end_index" ), X( tmpStr ) );
                     sprintf( tmpStr, "%u", r->getTotalNbrDebits() );
                     user_debit_log_reply->setAttribute( 
                        X( "total_number_elements" ), X( tmpStr ) );
                     
                     int pos = 0;
                     DebitElement* el = r->getFirstElement( pos );
                     while ( el != NULL ) {
                        ::appendDebitLogElement( user_debit_log_reply, 
                                                 reply, indentLevel + 1, 
                                                 indent, el );
                        delete el;
                        el = r->getNextElement( pos );
                     }
                     mc2log << info << "UserDebitLog: OK SI "
                            << r->getStartIndex() << " EI " 
                            << r->getEndIndex() << " Total " 
                            << r->getTotalNbrDebits() << " start "
                            << startTime << " end " << endTime << " For "
                            << userItem->getUser()->getLogonID() << "("
                            << userItem->getUIN() << ")" << endl;
                  } else {
                     mc2log << warn << "XMLParserThread::"
                            << "xmlParseUserDebitLogRequestRequest "
                            << "Faild to process request Status: " ;
                     ok = false;
                     errorCode = "-1";
                     errorMessage = "Failed to process request: "; 
                     if ( ansCont != NULL ) {
                        mc2log << StringTable::getString( 
                           StringTable::stringCode(
                              static_cast<ReplyPacket*>( 
                                 ansCont->getPacket() )->getStatus() ), 
                           StringTable::ENGLISH )
                               << endl;
                        errorMessage.append( 
                           StringTable::getString( 
                              StringTable::stringCode(
                                 static_cast<ReplyPacket*>( 
                                    ansCont->getPacket() )->getStatus() ),
                              StringTable::ENGLISH ) );
                     } else {
                        errorCode = "-3"; 
                        errorMessage.append( 
                           StringTable::getString( 
                              StringTable::TIMEOUT_ERROR, 
                              StringTable::ENGLISH ) );
                     }
                  }
   
                  delete ansCont;
                  delete req;
               } else {
                  mc2log << warn << "XMLParserThread::"
                         << "xmlParseUserDebitLogRequestRequest "
                         << "no access to user, userID: " << userID 
                         << " user_session_id " << user_session_id 
                         << " user_session_key " << user_session_key 
                         << endl;
               ok = false;
               errorCode = "-1";
               errorMessage = "Access denied to user.";
               }
            } else {
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseUserDebitLogRequestRequest "
                      << "unknown user, userID: " << userID 
                      << " user_session_id " << user_session_id 
                      << " user_session_key " << user_session_key << endl;
               ok = false;
               errorCode = "-1";
               errorMessage = "Failed to find user.";
            }
         } else {
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseUserDebitLogRequestRequest "
                   << "Database connection error, "
                   << "couldn't get user from userID: " << userID 
                   << " user_session_id " << user_session_id 
                   << " user_session_key " << user_session_key << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "Database error while retreiving "
               "user information.";
         }

         releaseUserItem( userItem );
      } else {
         ok = false;
         errorCode = "-1";
         errorMessage = "Not enough input to find user with.";
         mc2log << warn << "XMLParserThread::"
                   "xmlParseUserDebitLogRequestRequest"
                   " Not enough input to find user with."
                << endl;
      }
   } // Not ok handled below

   if ( !ok ) {
      // An error in errorCode, errorMessage
      XMLServerUtility::
      appendStatusNodes( user_debit_log_reply, reply, 
                         indentLevel + 1, indent, 
                         errorCode.c_str(), errorMessage.c_str() );
      // Error handled 
      ok = true;
      mc2log << info << "UserDebitLog: Error "
             << errorCode << "," << errorMessage << endl;
   }

   if ( indent ) {
      // Newline and indent before end tag   
      user_debit_log_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   return ok;
}

namespace {
void 
appendDebitLogElement( DOMNode* out, 
                       DOMDocument* reply,
                       int indentLevel,
                       bool indent,
                       DebitElement* el ) {
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   char tmpStr[ 128 ];

   DOMElement* user_debit_log_element = 
      reply->createElement( X( "user_debit_log_element" ) );

   // message_id
   sprintf( tmpStr, "%u", el->getMessageID() );
   user_debit_log_element->setAttribute( X( "message_id" ), X( tmpStr ) );
   
   // debit_info
   sprintf( tmpStr, "%u", el->getDebitInfo() );
   user_debit_log_element->setAttribute( X( "debit_info" ), X( tmpStr ) );

   // time
   sprintf( tmpStr, "%u", el->getTime() );
   user_debit_log_element->setAttribute( X( "time" ), X( tmpStr ) );

   // operationType
   sprintf( tmpStr, "%u", el->getOperationType() );
   user_debit_log_element->setAttribute( X( "operationType" ), 
                                         X( tmpStr ) );

   // sentSize
   sprintf( tmpStr, "%u", el->getSentSize() );
   user_debit_log_element->setAttribute( X( "sentSize" ), X( tmpStr ) );

   // userOrigin
   user_debit_log_element->setAttribute( 
      X( "userOrigin" ), X( el->getUserOrigin() ) );

   // serverID
   user_debit_log_element->setAttribute( 
      X( "serverID" ), X( el->getServerID() ) );

   // description
   user_debit_log_element->setAttribute( 
      X( "description" ), X( el->getDescription() ) );

   // Add user_debit_log_element to out
   if ( indent ) {
      out->appendChild( reply->createTextNode( X( indentStr.c_str() ) ) );
   }
   out->appendChild( user_debit_log_element );
}
}
#endif // USE_XML
