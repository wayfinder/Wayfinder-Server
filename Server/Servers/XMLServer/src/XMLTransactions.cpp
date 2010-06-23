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
#include "XMLServerElements.h"

using XMLServerUtility::appendStatusNodes;


bool 
XMLParserThread::xmlParseTransactionsRequest( DOMNode* cur, 
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

   // Transactions request data
   MC2String userID;
   MC2String user_session_id;
   MC2String user_session_key;
   MC2String uinStr;
   uint32 uin = 0;
   int32 transactionChange = 0;

   // Create transactions_reply element
   DOMElement* transactions_reply = 
      reply->createElement( X( "transactions_reply" ) );
   // Transaction ID
   transactions_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( transactions_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( reply->createTextNode( XindentStr.XMLStr() ), 
                         transactions_reply );
   }


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
      } else if ( XMLString::equals( attribute->getNodeName(), "uin" ) ) {
         char* endPtr = NULL;
         uin = strtoul( tmpStr, &endPtr, 10 );
         if ( endPtr == NULL || *endPtr != '\0' || uin == 0 ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Problem parsing uin not a valid number."; 
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseTransactionsRequest uin not "
                   << "a valid number. " << MC2CITE( tmpStr ) << endl;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "transaction_change" ) )
      {
         char* endPtr = NULL;
         transactionChange = strtol( tmpStr, &endPtr, 10 );
         if ( endPtr == NULL || *endPtr != '\0' ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Problem parsing transaction_change not a "
               "valid number."; 
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseTransactionsRequest transaction_change not "
                   << "a valid number. " << MC2CITE( tmpStr ) << endl;
         }
      } else {
         mc2log << warn << "XMLParserThread::xmlParseTransactionsRequest "
                   "unknown attribute for transactions_request element "
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
                         "xmlParseTransactionsRequest "
                         "odd Element in transactions_request element: " 
                      << child->getNodeName() << endl;
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
                      "xmlParseTransactionsRequest odd "
                      "node type in transactions_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }
   
   if ( ok && 
        ( uin != 0 || !userID.empty() || 
          ( !user_session_id.empty() && !user_session_key.empty() ) ) )
   {
      UserItem* userItem = NULL;
      bool status = true;

      if ( uin != 0 ) {
         status = getUser( uin, userItem );
      } else if ( !userID.empty() ) {
         status = getUser( userID.c_str(), userItem );
      } else {
         status = getUserBySession( 
            user_session_id.c_str(), user_session_key.c_str(), userItem );
      }

      if ( status ) {
         if ( userItem != NULL ) {
            if ( m_user->getUser()->getEditUserRights()  ) {
               // Do it
               int32 nbrTransactions = 0;
               if ( getAndChangeTransactions( userItem->getUIN(),
                                              transactionChange,
                                              nbrTransactions ) )
               {
                  char tmpStr[ 256 ];
                  sprintf( tmpStr, "%d", nbrTransactions );
                  transactions_reply->setAttribute( 
                     X( "nbr_transactions" ), X( tmpStr ) );
                  errorCode = "0";
                  errorMessage = "OK";
                  mc2log << info << "UserTransactions: OK Changed "
                         << transactionChange << " current "
                         << nbrTransactions << " For " 
                         << userItem->getUser()->getLogonID() << "("
                         << userItem->getUIN() << ")" << endl;
               } else {
                  mc2log << warn << "XMLParserThread::"
                         << "xmlParseTransactionsRequest "
                         << "getAndChangeTransactions failed."
                         << endl;
                  ok = false;
                  errorCode = "-1";
                  errorMessage = "Failed to process transactions request.";
               }
            } else {
               mc2log << warn << "XMLParserThread::"
                         "xmlParseTransactionsRequest "
                      << "no access to user, uin " << uin 
                      << "userID: " << userID 
                      << " user_session_id " << user_session_id 
                      << " user_session_key " << user_session_key << endl;
               ok = false;
               errorCode = "-1";
               errorMessage = "Access denied to user.";  
            }
         } else {
            mc2log << warn << "XMLParserThread::"
                      "xmlParseTransactionsRequest "
                   << "unknown user, uin " << uin << " userID: " << userID 
                   << " user_session_id " << user_session_id 
                   << " user_session_key " << user_session_key << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "Failed to find user."; 
         }
      } else {
         mc2log << warn << "XMLParserThread::"
                   "xmlParseTransactionsRequest "
                   "Database connection error, "
                << "couldn't get user from uin " << uin 
                << "userID: " << userID 
                << " user_session_id " << user_session_id 
                << " user_session_key " << user_session_key << endl;
         ok = false;
         errorCode = "-1";
         errorMessage = "Database error retreiving user information.";
      }

      delete userItem;
   } else {
      if ( ok ) {
         mc2log << warn << "XMLParserThread::"
                   "xmlParseTransactionsRequest"
                   " Not enough input to find user with."
                << endl;
         ok = false;
         errorCode = "-1";
         errorMessage = "Not enough input to find user with.";
      }
   }

   // Always append status
   appendStatusNodes( transactions_reply, reply, 
                      indentLevel + 1, indent, 
                      errorCode.c_str(), errorMessage.c_str() );
   if ( !ok ) {
      mc2log << info << "UserTransactions: Error " << errorCode << ","
             << errorMessage << endl;
   }
   // Error handled 
   ok = true;

   if ( indent ) {
      // Newline and indent before end tag   
      transactions_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   return ok;
}


bool 
XMLParserThread::xmlParseTransactionDaysRequest( DOMNode* cur, 
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

   // Transaction days request data
   MC2String userID;
   MC2String user_session_id;
   MC2String user_session_key;
   MC2String uinStr;
   uint32 uin = 0;
   bool check = false;
   int32 nbrTransactions = 0;

   // Create transaction_days_reply element
   DOMElement* transaction_days_reply = 
      reply->createElement( X( "transaction_days_reply" ) );
   // Transaction ID
   transaction_days_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( transaction_days_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), 
         transaction_days_reply );
   }


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
      } else if ( XMLString::equals( attribute->getNodeName(), "uin" ) ) {
         char* endPtr = NULL;
         uin = strtoul( tmpStr, &endPtr, 10 );
         if ( endPtr == NULL || *endPtr != '\0' || uin == 0 ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Problem parsing uin not a valid number."; 
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseTransactionDaysRequest uin not "
                   << "a valid number. " << MC2CITE( tmpStr ) << endl;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "transaction_change" ) )
      {
         char* endPtr = NULL;
         nbrTransactions = strtol( tmpStr, &endPtr, 10 );
         if ( endPtr == NULL || *endPtr != '\0' ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Problem parsing transaction_change not a "
               "valid number."; 
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseTransactionDaysRequest transaction_change "
                   << "not a valid number. " << MC2CITE( tmpStr ) << endl;
         }
      } else if ( XMLString::equals( attribute->getNodeName(), "check" ) )
      {
         check = StringUtility::checkBoolean( tmpStr );
      } else {
         mc2log << warn << "XMLParserThread::"
                << "xmlParseTransactionDaysRequest "
                << "unknown attribute for transactions_request element "
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
                         "xmlParseTransactionDaysRequest "
                         "odd Element in transactions_request element: " 
                      << child->getNodeName() << endl;
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
                      "xmlParseTransactionDaysRequest odd "
                      "node type in transactions_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }
   
   if ( ok && 
        ( uin != 0 || !userID.empty() || 
          ( !user_session_id.empty() && !user_session_key.empty() ) ) )
   {
      UserItem* userItem = NULL;
      bool status = true;

      if ( uin != 0 ) {
         status = getUser( uin, userItem );
      } else if ( !userID.empty() ) {
         status = getUser( userID.c_str(), userItem );
      } else {
         status = getUserBySession( 
            user_session_id.c_str(), user_session_key.c_str(), userItem );
      }

      if ( status ) {
         if ( userItem != NULL ) {
            if ( m_user->getUser()->getEditUserRights()  ) {
               // Do it
               uint32 curTime = 0;
               uint32 transactionChange = nbrTransactions;
               StringTable::stringCode status = 
                  getAndChangeTransactionDays( 
                     userItem->getUIN(), check, nbrTransactions, curTime );
               char tmpStr[ 256 ];
               sprintf( tmpStr, "%d", nbrTransactions );
               transaction_days_reply->setAttribute( 
                  X( "nbr_transaction_days" ), X( tmpStr ) );
               sprintf( tmpStr, "%u", curTime );
               transaction_days_reply->setAttribute( 
                  X( "current_day" ), X( tmpStr ) );
               if ( status == StringTable::OK ) {
                  errorCode = "0";
                  errorMessage = "OK";
                  mc2log << info << "UserTransactionDays: OK Changed "
                         << transactionChange << " currentDay "
                         << curTime << " nbrLeft "
                         << nbrTransactions << " For " 
                         << userItem->getUser()->getLogonID() << "("
                         << userItem->getUIN() << ")" << endl;
               } else if ( status == StringTable::NOT_ALLOWED ) {
                  mc2log << warn << "XMLParserThread::"
                         << "xmlParseTransactionDaysRequest "
                         << "getAndChangeTransactions no days left."
                         << endl;
                  ok = false;
                  errorCode = "-1";
                  errorMessage = "No days left.";
               } else if ( status == StringTable::TIMEOUT_ERROR ) {
                  mc2log << warn << "XMLParserThread::"
                         << "xmlParseTransactionDaysRequest "
                         << "getAndChangeTransactions timeout."
                         << endl;
                  ok = false;
                  errorCode = "-3";
                  errorMessage = "Timeout on transaction days request.";
               } else {
                  mc2log << warn << "XMLParserThread::"
                         << "xmlParseTransactionDaysRequest "
                         << "getAndChangeTransactions failed."
                         << endl;
                  ok = false;
                  errorCode = "-1";
                  errorMessage = 
                     "Failed to process transaction days request.";
               }
            } else {
               mc2log << warn << "XMLParserThread::"
                         "xmlParseTransactionDaysRequest "
                      << "no access to user, uin " << uin 
                      << "userID: " << userID 
                      << " user_session_id " << user_session_id 
                      << " user_session_key " << user_session_key << endl;
               ok = false;
               errorCode = "-1";
               errorMessage = "Access denied to user.";  
            }
         } else {
            mc2log << warn << "XMLParserThread::"
                      "xmlParseTransactionDaysRequest "
                   << "unknown user, uin " << uin << " userID: " << userID 
                   << " user_session_id " << user_session_id 
                   << " user_session_key " << user_session_key << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "Failed to find user."; 
         }
      } else {
         mc2log << warn << "XMLParserThread::"
                   "xmlParseTransactionDaysRequest "
                   "Database connection error, "
                << "couldn't get user from uin " << uin 
                << "userID: " << userID 
                << " user_session_id " << user_session_id 
                << " user_session_key " << user_session_key << endl;
         ok = false;
         errorCode = "-1";
         errorMessage = "Database error retreiving user information.";
      }

      delete userItem;
   } else {
      if ( ok ) {
         mc2log << warn << "XMLParserThread::"
                   "xmlParseTransactionDaysRequest"
                   " Not enough input to find user with."
                << endl;
         ok = false;
         errorCode = "-1";
         errorMessage = "Not enough input to find user with.";
      }
   }

   // Always append status
   appendStatusNodes( transaction_days_reply, reply, 
                      indentLevel + 1, indent, 
                      errorCode.c_str(), errorMessage.c_str() );
   if ( !ok ) {
      mc2log << info << "UserTransactionDays: Error "
             << errorCode << "," << errorMessage << endl;
   }
   // Error handled 
   ok = true;

   if ( indent ) {
      // Newline and indent before end tag   
      transaction_days_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   return ok;
}


#endif // USE_XML

