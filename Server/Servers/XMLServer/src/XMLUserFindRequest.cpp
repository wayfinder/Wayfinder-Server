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
#include "STLStrComp.h"
#include "XMLServerElements.h"

using XMLServerUtility::appendStatusNodes;

bool 
XMLParserThread::xmlParseUserFindRequest( DOMNode* cur, 
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

   
   UserItem* user = new UserItem( new UserUser( MAX_UINT32 ) );


   // Create user_find_reply element
   DOMElement* user_find_reply = 
      reply->createElement( X( "user_find_reply" ) );
   // Transaction ID
   user_find_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( user_find_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), user_find_reply );
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
         mc2log << warn << "XMLParserThread::xmlParseUserFindRequest "
                   "unknown attribute for user_find_request element "
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
            if ( XMLString::equals( child->getNodeName(), "user" ) ) {
               MC2String newPassword;
               MC2String oldPassword;
               bool haveNewPassword = false;
               bool haveOldPassword = false;
               ok = readUser( child, user->getUser(), 
                              newPassword, oldPassword, 
                              haveNewPassword, haveOldPassword,
                              errorCode, errorMessage );
            } else {
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseUserFindRequestRequest "
                      << "odd with Element in user_find_request "
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
                      "xmlParseUserFindRequestRequest odd "
                      "node type in user_find_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }
   
   // Security check
   if ( !m_user->getUser()->getEditUserRights() ) {
      ok = false;
      errorCode = "-201";
      errorMessage = "Access denied.";
   }

   if ( ok ) {
      // Send find user for user if anything changed in UserUser
      // For all phones send a find user
      map< const char*, uint32, strCompareLess > userMap;
      char tmpStr[ 256 ];
      vector<PacketContainer*> ansConts;
      uint32* uins = NULL;
      const_char_p* logonIDs = NULL;
      PacketContainer* ansCont = NULL;
      uint32 nbrUsers = 0;
      bool changed = false;

      // UserUser
      if ( user->getUser()->isChanged() ) {
         changed = true;
         ok = getUsersFromUserElement( user->getUser(), nbrUsers,
                                       uins, logonIDs, ansCont,
                                       errorCode, errorMessage );
         ansConts.push_back( ansCont );
         for ( uint32 i = 0 ; i < nbrUsers ; i++ ) {
            userMap.insert( make_pair( logonIDs[ i ], uins[ i ] ) );
         }
         delete [] uins;
         delete [] logonIDs;
      }

      // All changed UserElements
      vector<UserElement*> changed_elements;
      user->getUser()->getAllChangedElements( changed_elements );
      for ( vector<UserElement*>::iterator it = changed_elements.begin();
            it != changed_elements.end();
            ++it ) {
            UserElement* el = *it;
            changed = true;
            el->setID( MAX_UINT32 ); // Find id
            ok = getUsersFromUserElement( 
               el, nbrUsers, uins, logonIDs, ansCont,
               errorCode, errorMessage );
            ansConts.push_back( ansCont );
            for ( uint32 i = 0 ; i < nbrUsers ; i++ ) {
               userMap.insert( make_pair( logonIDs[ i ], uins[ i ] ) );
            }
            delete [] uins;
            delete [] logonIDs;    
      }

      if ( changed && ok ) {
         // All ok print user_ids
         for ( map< const char*, uint32, strCompareLess >::const_iterator
                  it = userMap.begin() ; it != userMap.end() ; ++it )
         {
            XMLServerUtility::
               appendElementWithText( user_find_reply, reply, 
                                      "user_id", it->first, 
                                      indentLevel + 1, indent  );
            sprintf( tmpStr, "%u", it->second );
            XMLServerUtility::
               appendElementWithText( user_find_reply, reply, 
                                      "uin", tmpStr, 
                                      indentLevel + 1, indent  );
         }
         mc2log << info << "UserFind: OK Found " << userMap.size()
                << " users" << endl;
      } else if ( !changed )  {
         ok = false;
         errorCode = "-1";
         errorMessage = "Nothing to find user with.";
      }

      // Delete all PacketContainers
      for ( uint32 i = 0 ; i < ansConts.size() ; i++ ) {
         delete ansConts[ i ];
      }
   } // End if ok

   if ( !ok ) {
      // An error in errorCode, errorMessage
      appendStatusNodes( user_find_reply, reply, 
                         indentLevel + 1, indent, 
                         errorCode.c_str(), errorMessage.c_str() );
      // Error handled 
      ok = true;
      mc2log << info << "UserFind: Error " << errorCode << ","
             << errorMessage << endl;
   }

   if ( indent ) {
      // Newline and indent before end tag   
      user_find_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   delete user;

   return ok;
}

   
bool 
XMLParserThread::getUsersFromUserElement( const UserElement* elem, 
                                          uint32& nbrUsers, 
                                          uint32*& uins,
                                          const_char_p*& logonIDs,
                                          PacketContainer*& ansCont,
                                          MC2String& errorCode, 
                                          MC2String& errorMessage )
{
   bool ok = true;
   uins = NULL;
   logonIDs = NULL;
   nbrUsers = 0;
   ansCont = NULL;

   PacketContainer* cont = new PacketContainer( new FindUserRequestPacket(
      0, 0, const_cast< UserElement* >( elem ) ), 0, 0, MODULE_TYPE_USER );
   SinglePacketRequest* req = new SinglePacketRequest( 
      m_group->getNextRequestID(), cont );
   mc2dbg8 << "About to send FindUserRequest" << endl;
   putRequest( req );
   mc2dbg8 << "FindUserRequest returned" << endl;
   
   ansCont = req->getAnswer();
   
   delete req;
   
   if ( ansCont != NULL ) {
      FindUserReplyPacket* findAns = static_cast< FindUserReplyPacket* > (
         ansCont->getPacket() );
      
      if ( findAns->getStatus() == StringTable::OK ) {
         nbrUsers = findAns->getNbrUsers();
         uins = findAns->getUINs();
         logonIDs = findAns->getLogonIDs();
      } else {
         ok = false;
         errorCode = "-1";
         errorMessage = "Problem when searching for users: ";
         errorMessage.append( 
            StringTable::getString( 
               StringTable::stringCode(
                  static_cast<ReplyPacket*>( 
                     ansCont->getPacket() )->getStatus() ), 
               StringTable::ENGLISH ) );
      }
   } else {
     ok = false;
     errorCode = "-3";
     errorMessage = "Timeout from database when searching for users.";
   }

   return ok;   
}

#endif // USE_XML

