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
#include "GetUserTrackRequest.h"
#include "AddUserTrackRequest.h"
#include "UserData.h"
#include "XMLAuthData.h"
#include "ParserPosPush.h"
#include "XMLSearchUtility.h"
#include "XMLServerElements.h"
#include "ClientSettings.h"

using XMLServerUtility::appendStatusNodes;
namespace {


/**
 * Reads a user_track_items from cur.
 *
 * @param cur The user_track_item node.
 * @param errorCode If problem then this is set to error code.
 * @param errorMessage If problem then this is set to error 
 *                     message.
 * @return A new UserTrackElement with the data in the node or
 *         NULL if a problem occured.
 */
UserTrackElement* 
readUserTrackItem( DOMNode* cur, 
                   MC2String& errorCode, MC2String& errorMessage );

/**
 * Appends a number of user_track_items as children to cur.
 *
 * @param cur Where to put the user_track_item nodes.
 * @param reply The document to make the nodes in.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param userTrackElements The list with UserTrackElements.
 * @param positionSystem The position system to use.
 */
void 
appendUserTrackList( DOMNode* cur, DOMDocument* reply,
                     int indentLevel, bool indent,
                     const UserTrackElementsList& userTrackElements,
                     XMLCommonEntities::coordinateType positionSystem );

}

bool 
XMLParserThread::xmlParseUserTrackRequest( DOMNode* cur, 
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

   // User track request data
   MC2String userID;
   uint32 uin = 0;
   MC2String user_session_id;
   MC2String user_session_key;

   bool hasStartTime = false;
   bool hasEndTime = false;
   MC2String startTimeStr;
   MC2String endTimeStr;
   MC2String maxNbrTracksStr;
   uint32 startTime = 0;
   uint32 endTime = MAX_UINT32;
   uint32 maxNbrTracks = 1024;
   XMLCommonEntities::coordinateType positionSystem =
      XMLCommonEntities::MC2;   

   // Create user_track_reply element
   DOMElement* user_track_reply = 
      reply->createElement( X( "user_track_reply" ) );
   // Transaction ID
   user_track_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( user_track_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), user_track_reply );
   }


   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
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
         startTimeStr = tmpStr;
         hasStartTime = true;
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "end_time" ) ) 
      {
         endTimeStr = tmpStr;
         hasEndTime = true;
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "max_nbr_tracks" ) ) 
      {
         maxNbrTracksStr = tmpStr;
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "position_system" ) ) 
      {
         positionSystem = XMLCommonEntities::coordinateFormatFromString(
            tmpStr );
      } else {
         mc2log << warn << "XMLParserThread::xmlParseUserTrackRequest "
                   "unknown attribute for user_track_request element "
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
            } else if ( XMLString::equals( child->getNodeName(), "uin" ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               char* tmpPtr = NULL;
               uin = strtoul( tmpStr, &tmpPtr, 10 );
               if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
                  // Hmm, what about that
                  errorCode = "-1";
                  errorMessage = "uin is not a number.";
                  ok = false;
               } else if ( uin == 0 ) {
                  errorCode = "-1";
                  errorMessage = "uin is not a valid number.";
               }
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
                         "xmlParseUserTrackRequest "
                         "odd Element in user_track_request element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                      "xmlParseUserTrackRequest odd "
                      "node type in user_track_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }
   
   if ( ok && hasStartTime ) {
      // Get start time
      char* endPtr = NULL;
      startTime = strtoul( startTimeStr.c_str(), &endPtr, 10 );
      if ( endPtr == NULL || *endPtr != '\0' ) {
         ok = false;
         errorCode = "-1";
         errorMessage = "Problem parsing start_time not a valid number."; 
      }
   }
   
   if ( ok && hasEndTime ) {
      // Get end time
      char* endPtr = NULL;
      endTime = strtoul( endTimeStr.c_str(), &endPtr, 10 );
      if ( endPtr == NULL || *endPtr != '\0' ) {
         ok = false;
         errorCode = "-1";
         errorMessage = "Problem parsing end_time not a valid number."; 
      }
   }

   if ( ok ) {
      if ( maxNbrTracksStr.compare( "inf" ) != 0 ) {
        char* endPtr = NULL;
        maxNbrTracks = strtoul( maxNbrTracksStr.c_str(), &endPtr, 10 );
        if ( endPtr == NULL || *endPtr != '\0' ) {
           ok = false;
           errorCode = "-1";
           errorMessage = "Problem parsing max_nbr_tracks not a valid number.";
        }
      } // Else maxumum (default)
   }

   if ( ok && 
        ( !userID.empty() || uin != 0 ||
          ( !user_session_id.empty() && !user_session_key.empty() ) ) )
   {
      UserItem* userItem = NULL;
      bool status = true;

      if ( uin != 0 ) {
         status = getUser( uin, userItem, true );
      } else if ( !userID.empty() ) {
         status = getUser( userID.c_str(), userItem, true );
      } else {
         status = getUserBySession( 
            user_session_id.c_str(), user_session_key.c_str(), userItem,
            true );
      }

      if ( status ) {
         if ( userItem != NULL ) {
            if ( userItem->getUIN() == m_user->getUIN() ||
                 m_user->getUser()->getEditUserRights() )
            {
               // Get user track
               GetUserTrackRequest* req = new GetUserTrackRequest( 
                  getNextRequestID(), userItem->getUIN() );
            
               req->setTimeInterval( startTime, endTime );
               req->setMaxNbrHits( maxNbrTracks );
               mc2dbg4 << "About to send GetUserTrackRequest" << endl;
               putRequest( req );
               mc2dbg4 << "GetUserTrackRequest done" << endl;

               if ( req->getStatus() == StringTable::OK ) {
                  // Append user track data
                  ::appendUserTrackList( user_track_reply, reply, 
                                         indentLevel+1, indent,
                                         req->getResult(), positionSystem );
                  mc2log << info << "UserTrack: OK Nbr items "
                         << req->getResult().size() << " start "
                         << startTime << " end " << endTime << " User "
                         << userItem->getUser()->getLogonID() << "("
                         << userItem->getUIN() << ")" << endl;
               } else {
                  mc2log << warn << "XMLParserThread::"
                            "xmlParseUserTrackRequest Faild to process "
                            "request Status: " 
                         << StringTable::getString( req->getStatus(), 
                                                    StringTable::ENGLISH )
                         << endl;
                  ok = false;
                  errorCode = "-1";
                  if ( req->getStatus() == StringTable::TIMEOUT_ERROR ) {
                     errorCode = "-3";
                  }
                  errorMessage = "Failed to process UserTrackRequest: "; 
                  errorMessage.append( StringTable::getString( 
                     req->getStatus(), StringTable::ENGLISH ) );
               }
               delete req;
            } else {
               mc2log << warn << "XMLParserThread::"
                         "xmlParseUserTrackRequest "
                         "no access to user, userID: " << userID 
                      << " uin " << uin
                      << " user_session_id " << user_session_id 
                      << " user_session_key " << user_session_key << endl;
               ok = false;
               errorCode = "-1";
               errorMessage = "Access denied to user.";  
            }
         } else {
            mc2log << warn << "XMLParserThread::"
                      "xmlParseUserTrackRequest "
                      "unknown user, userID: " << userID 
                   << " uin " << uin
                   << " user_session_id " << user_session_id 
                   << " user_session_key " << user_session_key << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "Failed to find user."; 
         }
      } else {
         mc2log << warn << "XMLParserThread::"
                   "xmlParseUserTrackRequest "
                   "Database connection error, "
                   "couldn't get user from userID: " << userID 
                << " uin " << uin
                << " user_session_id " << user_session_id 
                << " user_session_key " << user_session_key << endl;
         ok = false;
         errorCode = "-1";
         errorMessage = "Database error retreiving user information.";
      }

      releaseUserItem( userItem );
   } else {
      if ( ok ) {
         mc2log << warn << "XMLParserThread::"
                   "xmlParseUserTrackRequest"
                   " Not enough input to find user with."
                << endl;
         ok = false;
         errorCode = "-1";
         errorMessage = "Not enough input to find user with.";
      } else {
         mc2log << info << "UserTrack: Error "
                << errorCode << ", " << errorMessage << endl;
      }
   }

   if ( !ok ) {
      // An error in errorCode, errorMessage
      appendStatusNodes( user_track_reply, reply, 
                         indentLevel + 1, indent, 
                         errorCode.c_str(), errorMessage.c_str() );
      // Error handled 
      ok = true;
   }

   if ( indent ) {
      // Newline and indent before end tag   
      user_track_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   return ok;
}

namespace {

void 
appendUserTrackList( DOMNode* cur, DOMDocument* reply,
                     int indentLevel, bool indent,
                     const UserTrackElementsList& userTrackElements,
                     XMLCommonEntities::coordinateType positionSystem ) {

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   char tmpStr[128];

   for ( UserTrackElementsList::const_iterator it = 
            userTrackElements.begin() ;
         it != userTrackElements.end() ; ++it )
   {
      // Create user_track_item element
      DOMElement* user_track_item = 
         reply->createElement( X( "user_track_item" ) );
      // time
      sprintf( tmpStr, "%d", (*it)->getTime() );
      user_track_item->setAttribute( X( "time" ), X( tmpStr ) );
      // dist
      sprintf( tmpStr, "%d", (*it)->getDist() );
      user_track_item->setAttribute( X( "dist" ), X( tmpStr ) );
      // speed
      sprintf( tmpStr, "%hd", (*it)->getSpeed() );
      user_track_item->setAttribute( X( "speed" ), X( tmpStr ) );
      // source
      user_track_item->setAttribute( X( "source" ), 
                                     X( (*it)->getSource() ) );

      // position_item
      XMLSearchUtility::
         appendPositionItem( (*it)->getLat(), (*it)->getLon(), 
                             (*it)->getHeading(), positionSystem,
                             user_track_item, reply, 
                             indentLevel + 1, indent );

      if ( indent ) {
         // Newline
        user_track_item->appendChild( 
           reply->createTextNode( XindentStr.XMLStr() ) );
      }
      cur->appendChild( user_track_item );
      if ( indent ) {
         // Newline
         cur->insertBefore( reply->createTextNode( XindentStr.XMLStr() ), 
                            user_track_item );
      }
   }
}

}

bool 
XMLParserThread::xmlParseUserTrackAddRequest( DOMNode* cur, 
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

   // User track request data
   MC2String userID;
   uint32 uin = 0;
   MC2String user_session_id;
   MC2String user_session_key;

   UserTrackElementsList userTrackElementsList;

   // Create user_track_reply element
   DOMElement* user_track_reply = 
      reply->createElement( X( "user_track_add_reply" ) );
   // Transaction ID
   user_track_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( user_track_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), user_track_reply );
   }


   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "transaction_id" ) ) 
      {
         // Handled above 
      } else {
         mc2log << warn << "XMLParserThread::xmlParseUserTrackAddRequest "
                   "unknown attribute for user_track_request element "
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
            } else if ( XMLString::equals( child->getNodeName(), "uin" ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               char* tmpPtr = NULL;
               uin = strtoul( tmpStr, &tmpPtr, 10 );
               if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
                  // Hmm, what about that
                  errorCode = "-1";
                  errorMessage = "uin is not a number.";
                  ok = false;
               } else if ( uin == 0 ) {
                  errorCode = "-1";
                  errorMessage = "uin is not a valid number.";
               }
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
            } else if ( XMLString::equals( child->getNodeName(),
                                           "user_track_item" ) ) 
            {
               UserTrackElement* elem = 
                  ::readUserTrackItem( child, errorCode, errorMessage );

               if ( elem == NULL ) {
                  ok = false;
               } else {
                  userTrackElementsList.push_back( elem );
               }
            } else {
               mc2log << warn << "XMLParserThread::"
                         "xmlParseUserTrackAddRequest "
                         "odd Element in user_track_request element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                      "xmlParseUserTrackAddRequest odd "
                      "node type in user_track_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }
   

   if ( ok && 
        ( !userID.empty() || uin != 0 ||
          ( !user_session_id.empty() && !user_session_key.empty() ) ) )
   {
      UserItem* userItem = NULL;
      bool status = true;

      if ( uin != 0 ) {
         status = getUser( uin, userItem, true );
      } else if ( !userID.empty() ) {
         status = getUser( userID.c_str(), userItem, true );
      } else {
         status = getUserBySession( 
            user_session_id.c_str(), user_session_key.c_str(), userItem,
            true );
      }

      if ( status ) {
         if ( userItem != NULL ) {
            if ( userItem->getUIN() == m_user->getUIN() ||
                 m_user->getUser()->getEditUserRights()  )
            {
               // Add user tracks
               AddUserTrackRequest* req = new AddUserTrackRequest( 
                  getNextRequestID(), userItem->getUIN() );
               vector< UserTrackElement* > posItems;
            
               for ( UserTrackElementsList::iterator it = 
                     userTrackElementsList.begin() ; 
                     it != userTrackElementsList.end() ; ++it )
               {
                  posItems.push_back( *it );
                  req->addUserTrackElement( *it );
               }

               int pres = 0;
               if ( !posItems.empty() ) {
                  pres = getPosPush().positionsReceived(
                     posItems, userItem, 
                     m_authData->clientSetting );
                  posItems.clear(); // req owns the UserTrackElements
               }

               if ( pres != 1 ) {
                  mc2dbg4 << "About to send AddUserTrackRequest" << endl;
                  putRequest( req );
                  mc2dbg4 << "AddUserTrackRequest done" << endl;
               }

               if ( req->getStatus() == StringTable::OK ||
                    req->getStatus() == StringTable::UNKNOWN/*Not sent*/) {
                  // Append ok status
                  appendStatusNodes( user_track_reply, reply, 
                                     indentLevel + 1, indent,
                                     "0", "OK" );
                  mc2log << info << "UserTrackAdd: OK Nbr items "
                         << userTrackElementsList.size() << " added"
                         << endl;
               } else {
                  mc2log << warn << "XMLParserThread::"
                            "xmlParseUserTrackAddRequest Faild to process "
                            "request Status: " 
                         << StringTable::getString( req->getStatus(), 
                                                    StringTable::ENGLISH )
                         << endl;
                  ok = false;
                  errorCode = "-1";
                  if ( req->getStatus() == StringTable::TIMEOUT_ERROR ) {
                     errorCode = "-3";
                  }
                  errorMessage = "Failed to process UserTrackAddRequest: ";
                  errorMessage.append( StringTable::getString( 
                     req->getStatus(), StringTable::ENGLISH ) );
               }
               delete req;
               userTrackElementsList.clear();
            } else {
               mc2log << warn << "XMLParserThread::"
                         "xmlParseUserTrackAddRequest "
                         "no access to user, userID: " << userID 
                      << " uin " << uin
                      << " user_session_id " << user_session_id 
                      << " user_session_key " << user_session_key << endl;
               ok = false;
               errorCode = "-1";
               errorMessage = "Access denied to user.";  
            }
         } else {
            mc2log << warn << "XMLParserThread::"
                      "xmlParseUserTrackAddRequest "
                      "unknown user, userID: " << userID 
                   << " uin " << uin
                   << " user_session_id " << user_session_id 
                   << " user_session_key " << user_session_key << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "Failed to find user."; 
         }
      } else {
         mc2log << warn << "XMLParserThread::"
                   "xmlParseUserTrackAddRequest "
                   "Database connection error, "
                   "couldn't get user from userID: " << userID 
                << " uin " << uin
                << " user_session_id " << user_session_id 
                << " user_session_key " << user_session_key << endl;
         ok = false;
         errorCode = "-1";
         errorMessage = "Database error while, "
            "retreiving user information.";
      }

      releaseUserItem( userItem );
   } else {
      if ( ok ) {
         mc2log << warn << "XMLParserThread::"
                   "xmlParseUserTrackAddRequest"
                   " Not enough input to find user with."
                << endl;
         ok = false;
         errorCode = "-1";
         errorMessage = "Not enough input to find user with.";
      } else {
         mc2log << info << "UserTrackAdd: Error "
                << errorCode << ", " << errorMessage << endl;
      }
   }

   if ( !ok ) {
      // An error in errorCode, errorMessage
      appendStatusNodes( user_track_reply, reply, 
                         indentLevel + 1, indent, 
                         errorCode.c_str(), errorMessage.c_str() );
      // Error handled 
      ok = true;
   }

   if ( indent ) {
      // Newline and indent before end tag   
      user_track_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   return ok;
}

namespace {
UserTrackElement*
readUserTrackItem( DOMNode* cur, 
                    MC2String& errorCode, 
                    MC2String& errorMessage ) {
   UserTrackElement* res = NULL;

   if ( XMLString::equals( cur->getNodeName(), "user_track_item" ) ) {
      uint32 itemTime = 0;
      MC2String source;
      int32 lat = MAX_INT32;
      int32 lon = MAX_INT32;
      uint32 dist = MAX_UINT32;
      uint16 speed = MAX_UINT16;
      uint16 angle = MAX_UINT16;
      char* endPtr = NULL;
      bool ok = true;

      // Attributes
      DOMNamedNodeMap* attributes = cur->getAttributes();
      DOMNode* attribute;
   
      for ( uint32 i = 0 ; i < attributes->getLength() && ok ; i++ ) {
         attribute = attributes->item( i );
      
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         if ( XMLString::equals( attribute->getNodeName(), "time" ) ) {
            // Time
            itemTime = strtoul( tmpStr, &endPtr, 10 );
            if ( endPtr == NULL || *endPtr != '\0' ) {
               ok = false;
               errorCode = "-1";
               errorMessage = "Problem parsing time attribute in "
                  "user_track_item: not a valid number."; 
            }
            if ( itemTime < 100000000 ) { // Time relative now
               itemTime = TimeUtility::getRealTime() - itemTime;
            }
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "dist" ) ) 
         {
            dist = strtoul( tmpStr, &endPtr, 10 );
            if ( endPtr == NULL || *endPtr != '\0' ) {
               ok = false;
               errorCode = "-1";
               errorMessage = "Problem parsing dist attribute in "
                  "user_track_item: not a valid number."; 
            }
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "speed" ) ) 
         {
            uint32 tmp = strtoul( tmpStr, &endPtr, 10 );
            if ( endPtr == NULL || *endPtr != '\0' || tmp > MAX_UINT16 ) {
               ok = false;
               errorCode = "-1";
               errorMessage = "Problem parsing speed attribute in "
                  "user_track_item: ";
               if ( tmp <= MAX_UINT16 ) {
                  errorMessage.append( "not a valid number." );
               } else {
                  errorMessage.append( "speed is too big." );
               }
            }
            speed = tmp;
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "source" ) ) 
         {
            source = tmpStr;
         } else {
            mc2log << warn << "XMLParserThread::readUserTrackItem "
                      "unknown attribute for user_track_item element "
                   << " Name " << attribute->getNodeName()
                   << " Type " << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }

      // Children
      DOMNode* child = cur->getFirstChild();
   
      while ( child != NULL && ok ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(),
                                       "position_item" ) ) 
               {
                  if ( ! XMLCommonElements::getPositionItemData( 
                          child, lat, lon, angle, 
                          errorCode, errorMessage ) )
                  {
                     ok = false;
                     // errorCode, errorMessage set in getPositionItemData
                  }
               } else {
                  mc2log << warn << "XMLParserThread::"
                            "readUserTrackItem "
                            "odd Element in user_track_item element: " 
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn << "XMLParserThread::"
                         "xmlParseUserTrackRequest odd "
                         "node type in user_track_item element: " 
                      << child->getNodeName() 
                      << " type " << child->getNodeType() << endl;
               break;
         }
         child = child->getNextSibling();
      }
      
      if ( ok ) {
         // Create UserTrackElement
         if ( source.empty() ) {
            source = "Track";
         }
         res = new UserTrackElement( lat, lon, dist, speed, angle, 
                                     itemTime, source.c_str() );
      } // Error set with ok
   } else {
      errorCode = "-1";
      errorMessage = "user_track_item not a user_track_item.";
   }

   return res;
}

}

#endif // USE_XML

