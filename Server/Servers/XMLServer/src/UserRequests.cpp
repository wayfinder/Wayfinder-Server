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
#include "ClientRights.h"

#ifdef USE_XML
#include "SinglePacketRequest.h"
#include "UserRight.h"
#include "UserData.h"
#include "UserLicenceKey.h"
#include "UserPacket.h"
#include "STLStringUtility.h"
#include "StringConversion.h"
#include <algorithm>
#include "ParserExternalAuth.h"
#include "XMLAuthData.h"
#include "XMLSearchUtility.h"
#include "XMLServerUtility.h"
#include "XMLServerElements.h"
#include "XMLTool.h"
#include <memory> 
#include "RouteTypes.h"


using XMLServerUtility::appendStatusNodes;
using namespace XMLTool;

namespace {
DOMElement* createPINElement( DOMDocument& root,
                              const UserPIN& pin, bool addPIN, bool addComment ) {
   // pin
   DOMElement* pinEl = root.createElement( X( "pin" ) );
   
   // id
   pinEl->setAttribute( X( "id" ), XInt32( pin.getID() ) );
   
   if ( addPIN ) {
      pinEl->setAttribute( X( "PIN" ), X( pin.getPIN() ) );
   }
   if ( addComment ) {
      pinEl->setAttribute( X( "comment" ), X( pin.getComment() ) );
   }

   return pinEl;
}

}

bool 
XMLParserThread::xmlParseUserRequest( DOMNode* cur, 
                                      DOMNode* out,
                                      DOMDocument* reply,
                                      bool indent )
{
   bool ok = true;
   int indentLevel = 1;
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   bool hasNewUser = false;
   bool newUser = false;

   // Create user_reply
   DOMElement* user_reply = XMLUtility::createStandardReply( *reply, *cur,
                                                             "user_reply" );
   out->appendChild( user_reply );

   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), user_reply );
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
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "new_user" ) ) 
      {
         hasNewUser = true;
         newUser = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "uin" ) ) 
      {
         // UIN ignored here but used in xmlParseUserRequestUser
      } else {
         mc2log << warn << "XMLParserThread::xmlParseUserRequest "
                << "unknown attribute for user_request element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   // Go throu the elements and handle them.   

   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), "user" ) ) {
               ok = xmlParseUserRequestUser( child, hasNewUser,
                                             newUser, user_reply, 
                                             reply, 
                                             indentLevel + 1, indent );
            } else {
               ok = false;
               mc2log << warn << "XMLParserThread::xmlParseUserList "
                      << "odd Element in user_request element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            ok = false;
            mc2log << warn << "XMLParserThread::xmlParseUserList odd "
                   << "node type in user_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }


   if ( indent ) {
      // Newline and indent before end user_reply tag
      user_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   return ok;
}


bool 
XMLParserThread::xmlParseUserRequestUser( DOMNode* cur, bool hasNewUser,
                                          bool newUser,
                                          DOMNode* out,
                                          DOMDocument* reply,
                                          int indentLevel,
                                          bool indent )
{
   bool ok = true;
   MC2String statusCodeString;
   MC2String statusMessageString;

   // Get user

   // Find user_id
   DOMElement* xmlUser = static_cast< DOMElement* >( cur );
   DOMNodeList* user_ids = xmlUser->getElementsByTagName( X( "user_id" ) );

   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* uinAttribute = attributes->getNamedItem( X( "uin" ) );

   if ( user_ids->getLength() == 1 || uinAttribute != NULL ) {
      UserItem* userItem = NULL;
      uint32 uin = 0;
      char* logonIDString = NULL;
      bool status = true;
      if ( uinAttribute != NULL ) {
         if ( hasNewUser && newUser ) {
            // What?! UIN for a new user you must be kidding me.
            statusCodeString = "-1";
            statusMessageString = "uin for new user is not acceptable.";
            ok = false;
         } else {
            char* tmpStr = XMLUtility::transcodefromucs( 
               uinAttribute->getNodeValue() );
            char* tmpPtr = NULL;
            uin = strtoul( tmpStr, &tmpPtr, 10 );
            if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
               statusCodeString = "-1";
               statusMessageString = "uin is not a number.";
               ok = false;
            } else {
               status = getUser( uin, userItem );
            }
            delete [] tmpStr;
         }
      } else { // logonid
         // Extract logonID
         DOMNode* logonIDNode = user_ids->item( 0 );
         logonIDString = XMLUtility::getChildTextValue( logonIDNode );
         status = getUser( logonIDString, userItem );
      }

      if ( ok && status ) {
         UserUser* user = NULL;
         bool isNewUser = false;
         MC2String newPassword;
         MC2String oldPassword;
         bool haveNewPassword = false;
         bool haveOldPassword = false;
         if ( hasNewUser ) {
            if ( userItem == NULL && newUser ) {
               // Ok, new unique user
               user = new UserUser( 0 );
               userItem = new UserItem( user );
               isNewUser = true;
            } else if ( userItem != NULL && !newUser ) {
               // Ok, edit of existing user 
               user = userItem->getUser();
            } else {
               ok = false;
               if ( userItem != NULL ) {
                  statusCodeString = "-107";
                  statusMessageString = "Not unique userID.";
               } else { // No such user
                  statusCodeString = "-108";
                  statusMessageString = "No such user.";
               }
            }
         } else if ( userItem != NULL ) {
            user = userItem->getUser();
         } else {
            if ( uin == 0 ) {
               user = new UserUser( 0 );
               userItem = new UserItem( user );
               isNewUser = true;
            } else { // No new user from uin must be user_id
               statusCodeString = "-108";
               statusMessageString = "No such user.";
               ok = false;  
            }
         }

         
         // Check access
         if ( ok && ( m_user->getUser()->getEditUserRights() ||
                      m_user->getUIN() == user->getUIN() ) )
         {

            ok = readUser( cur, user, newPassword, oldPassword, 
                           haveNewPassword, haveOldPassword, 
                           statusCodeString, statusMessageString );

            if ( ok ) {
               if ( isNewUser ) {
                  // Add new user
                  if ( !newPassword.empty() ) {
                     // Create new user
                     uint32 resUIN = createUser( user, 
                                                 newPassword.c_str(),
                                                 m_user->getUser() );
                     if ( resUIN == 0 ) {
                        statusCodeString = "-107";
                        statusMessageString = "Failed to add new user, "
                           "not unique user_id.";
                     } else if ( resUIN == MAX_UINT32 ) {
                        statusCodeString = "-3";
                        statusMessageString = "Failed to add new user.";
                     } else if ( resUIN == MAX_UINT32-1 ) {
                        statusCodeString = "-1";
                        statusMessageString = "Failed to add new user."; 
                     } else if ( resUIN == MAX_UINT32-2 ) {
                        statusCodeString = "-1";
                        statusMessageString = "Failed to add new user, "
                           "id_key of type account not unique."; 
                     } else {
                        statusCodeString = "0";
                        statusMessageString = "OK";
                     }
                  } else {
                     statusCodeString = "-1";
                     statusMessageString = 
                        "Must supply password for new user.";
                  }
                  
               } else {
                  // Save changes
                  bool changed = user->isChanged() ||
                                 user->hasChangedElement();

                  if ( changed ) {
                     if ( changeUser( user, m_user->getUser() ) ) {
                        statusCodeString = "0";
                        statusMessageString = "OK";
                        mc2log << "UserRequest: Changed OK "
                               << user->getLogonID()
                               << "(" << user->getUIN() << ")" << endl;
                     } else {
                        ok = false;
                        statusCodeString = "-1";
                        statusMessageString = "Failed to change user.";  
                     }
                  }
                     
                  if ( ok ) {
                     if ( haveNewPassword ) {
                        // Check the old one and then update to new passwd
                        bool checkOldPassword = haveOldPassword;

                        ok = changeUserPassword( 
                           user, 
                           newPassword.c_str(), oldPassword.c_str(),
                           checkOldPassword,
                           statusCodeString, statusMessageString );
                        if ( ok ) {
                           statusCodeString = "0";
                           statusMessageString = "OK";
                           mc2log << " Password Changed OK ";
                        } else {
                           ok = false;
                           mc2log << " Password Change failed: "
                                  << statusCodeString << ","
                                  << statusMessageString << " ";
                        }
                     } // End if new password
                  } // End if ok
               } // End else changed user
            } else {
               // Error in statusCodeString and statusMessageString
               
            }
         } else {
            if ( ok ) {
               // Not allowed
               mc2log << warn 
                      << "XMLParserThread::xmlParseUserRequestUser "
                      << "Access denied to logonIDString " << logonIDString
                      << " uin " << uin
                      << " for " << m_user->getUser()->getLogonID()
                      << endl;
               statusCodeString = "-1";
               statusMessageString = "Access denied.";
            } // else set with ok
         }
      } else {
         // Database error
         mc2dbg4 << " Database connection error" << endl;
         statusCodeString = "-1";
         statusMessageString = "Database connection error, "
            "please try again.";
      }
      delete [] logonIDString;
      delete userItem;
   } else {
      mc2log << warn << "xmlParseUserRequestUser not one user_id in "
             << "user tag" << endl;
      statusCodeString = "-1";
      statusMessageString = "No user_id element in content user";
   }

   if ( !ok ) {
      mc2log << "UserRequest failed: "
             << statusCodeString << ", "
             << statusMessageString << endl;
   }
   // Add resultstatus to reply
   XMLServerUtility::
      appendStatusNodes( out, reply, indentLevel, indent, 
                         statusCodeString.c_str(), 
                         statusMessageString.c_str() );
   // Possible error appended
   ok = true;

   return ok;
}


bool 
XMLParserThread::xmlParseUserPhone( DOMNode* cur, UserUser* user ) const {
   bool ok = true;
   MC2String phoneNumber;

   // phone_number
   DOMElement* phone = static_cast< DOMElement* >( cur );
   DOMNodeList* phoneNumbers = phone->getElementsByTagName( 
      X( "phone_number" ) );
   if ( phoneNumbers->getLength() == 1 ) {
      DOMNode* phoneNumberNode = phoneNumbers->item( 0 );
      phoneNumber = XMLUtility::getChildTextStr( *phoneNumberNode );
   } else {
      phoneNumber = "-1";
   }

   // See if phone already exists.
   UserCellular* cellular = getUsersCellular( user, phoneNumber.c_str() );
   auto_ptr<UserCellular> deleteCellular;
   if ( user->getUIN() == MAX_UINT32 /*find user*/ ) {
      // Find with what was entered including wildchars
      phoneNumber = StringUtility::trimStartEnd( phoneNumber );
   } else {
      // Clean phonenumber.
      char* tmp = StringUtility::cleanPhonenumber( phoneNumber.c_str() );
      phoneNumber = tmp;
      delete [] tmp;
   }

   // See if phone exists after clean
   if ( cellular == NULL ) {
      cellular = getUsersCellular( user, phoneNumber.c_str() );
   }

   if ( cellular != NULL && cellular->removed() ) {
      // Removed already, perhaps new
      cellular = NULL;
   }

   char* phoneManufacturer = NULL;
   char* phoneModel = NULL;
   bool deletePhone = false;
   bool newCellular = false;

                     
   if ( cellular == NULL && !phoneNumber.empty() &&
        (StringUtility::validPhonenumber( phoneNumber.c_str() ) ||
         user->getUIN() == MAX_UINT32 /*find user*/) )
   {
      cellular = new UserCellular( 0 );
      cellular->setPhoneNumber( phoneNumber.c_str() );
      newCellular = true;
      deleteCellular.reset( cellular );
   }

   if ( cellular != NULL ) {
      // Go thou children and make changes
      DOMNode* phoneChild = cur->getFirstChild();
      
      while ( phoneChild != NULL ) {
         switch ( phoneChild->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( phoneChild->getNodeName(),
                                       "phone_manufacturer" ) )
               {
                  phoneManufacturer = 
                     XMLUtility::getChildTextValue( phoneChild );
               } else if ( XMLString::equals( phoneChild->getNodeName(),
                                              "phone_model" ) ) 
               {
                  phoneModel = XMLUtility::getChildTextValue( phoneChild );
               } else if ( XMLString::equals( phoneChild->getNodeName(),
                                              "phone_delete" ) ) 
               {
                  deletePhone = true; 
               } else if ( XMLString::equals( phoneChild->getNodeName(),
                                              "phone_number" ) ) 
               {
                  // Ignore as we read it above
               } else {
                  mc2log << warn << "XMLParserThread::"
                         << "xmlParseUserPhone "
                         << "odd Element in phone element: "
                         << phoneChild->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseUserPhone "
                      << "odd node type in user element: "
                      << phoneChild->getNodeName() 
                      << " type " << phoneChild->getNodeType() << endl;
               break;
         }
         phoneChild = phoneChild->getNextSibling();
      }
      
      if ( phoneManufacturer != NULL &&
           phoneModel != NULL && 
           user->getUIN() != MAX_UINT32 ) /* Not when searching */
      {
         CellularPhoneModel* model = 
            new CellularPhoneModel();
         
         model->setManufacturer( phoneManufacturer );
         model->setName( phoneModel );

         cellular->setModel( model );
      }
      if ( !newCellular || m_user->getUser()->getEditUserRights() ) {
         if ( deletePhone ) {
            cellular->remove();
         }
         if ( newCellular && ! deletePhone ) {
            user->addElement( cellular );
            deleteCellular.release();
         }
      }
   } else {
      mc2log << warn << "XMLParserThread::"
             << "xmlParseUserPhone "
             << "invalid phonenumber: " << phoneNumber << endl;
      ok = false;
   }

   delete [] phoneManufacturer;
   delete [] phoneModel;
   
   return ok;
}


bool 
XMLParserThread::changeUserPassword( const UserUser* user, 
                                     const char* newPassword, 
                                     const char* oldPassword, 
                                     bool checkPassword,
                                     MC2String& errorCode, 
                                     MC2String& errorMessage )
{
   bool ok = true;


   int32 status = ParserThread::changeUserPassword( 
      user, newPassword, oldPassword, checkPassword, 
      m_user->getUser() );

   if ( status == 0 ) {
      // OK!
   } else if ( status == -2 ) {
      ok = false;
      errorCode = "-3";
      errorMessage = "Database connection error, please try again.";
   } else if ( status == -3 ) {
      ok = false;
      errorCode = "-105";
      errorMessage = "Old password not valid.";
   } else {
      ok = false;
      errorCode = "-1";
      errorMessage = "Failed to change user's password.";
   }

   return ok;
}


bool 
XMLParserThread::xmlParseUserLicence( DOMNode* cur, UserUser* user,
                                      MC2String& statusCodeString,
                                      MC2String& statusMessageString ) const
{
   bool ok = true;

   
   uint32 id = 0;
   byte* key = NULL;
   uint32 keyLength = 0;
   bool remove = false;

   // Go throu attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(), "id" ) ) {
         char* tmpPtr = NULL;
         id = strtoul( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = "binary_key's id is not a number.";
            ok = false;
         }
      } else {
         mc2log << warn << "XMLParserThread::xmlParseUserLicence "
                << "unknown attribute for binary_key element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   // Go throu users childrens and take apropriate actions
   DOMNode* child = cur->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), "key_data" ) ) {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               uint32 strLen = strlen( tmpStr );
               key = new byte[ strLen ]; // More than enough
               int res = StringUtility::base64Decode( tmpStr, key );
               if ( res != -1 ) {
                  keyLength = res;
               } else {
                  delete [] key;
                  key = NULL;
                  statusCodeString = "-1";
                  statusMessageString = "key_data not valid.";
                  ok = false;
               }
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "key_delete" ) ) 
            {
               remove = true;
            } else { // Odd element in binary_key element
               mc2log << warn << "XMLParserThread::xmlParseUserLicence"
                      << " odd Element in binary_key element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParseUserLicence "
                   << "odd node type in binary_key element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( ok && m_user->getUser()->getEditUserRights() ) {
      // Find 
      UserLicenceKey* el = NULL;
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_LICENCE_KEY ) ; 
            ++i )
      {
         if ( static_cast< UserLicenceKey* > ( user->getElementOfType( 
                 i, UserConstants::TYPE_LICENCE_KEY ) )->getID() == id &&
              id != 0 )
         {
            el = static_cast< UserLicenceKey* > ( 
               user->getElementOfType( 
                  i, UserConstants::TYPE_LICENCE_KEY ) );
            break;
         }
         
      }

      // Find ':' and make type and key (if no type then set type as imei)
      const byte* keyStart = key;
      byte* colonPos = reinterpret_cast<byte*>( 
         memchr( key, ':', keyLength ) );
      MC2String keyType( ParserUserHandler::imeiType );
      if ( colonPos != NULL ) {
         keyStart = colonPos + 1;
         keyLength -= colonPos - key + 1;
         keyType = MC2String( key, colonPos );
      }
      
      
      if ( el != NULL ) {
         if ( remove ) {
            el->remove();
         } else {
            // New key?
            const byte* oldKey = el->getLicenceKey();
            uint32 oldKeyLength = el->getLicenceLength();
            if ( oldKeyLength != keyLength ||
                 memcmp( oldKey, keyStart, oldKeyLength/*Same length*/ ) != 0 )
            {
               // New key!
               el->setLicence( keyStart, keyLength );
            }
            if ( keyType != el->getKeyType() ) {
               el->setKeyType( keyType );
            }
         }
      } else {
         // New?
         if ( !remove ) {
            UserLicenceKey* newEl = new UserLicenceKey( 0 );
            newEl->setLicence( keyStart, keyLength );
            newEl->setKeyType( keyType );
            user->addElement( newEl );
         } else { // else done, does not exist so it is removed!
            statusCodeString = "-1";
            statusMessageString = "Removing nonexistant binary_key.";
            ok = false;  
         }
      }

   }

   delete [] key;

   return ok;
}



bool 
XMLParserThread::xmlParseUserLicenceKey( DOMNode* cur, UserUser* user,
                                         MC2String& statusCodeString,
                                         MC2String& statusMessageString ) const
{
   bool ok = true;
   
   uint32 id = 0;
   MC2String licenceKey;
   MC2String keyType;
   MC2String product;
   MC2String deleteStr;
   bool remove = false;

   getAttribValue( id, "id", cur );
   getAttribValue( licenceKey, "key", cur );
   bool hasKeyType = getAttribValue( keyType, "key_type", cur );
   bool hasProduct = getAttribValue( product, "product", cur );
   getAttribValue( deleteStr, "delete", cur );
   remove = StringUtility::checkBoolean( deleteStr.c_str() );

   if ( ok && m_user->getUser()->getEditUserRights() ) {
      // Find 
      UserLicenceKey* el = NULL;
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_LICENCE_KEY ) ; ++i )
      {
         UserLicenceKey* licence = static_cast< UserLicenceKey* > ( 
            user->getElementOfType( i, UserConstants::TYPE_LICENCE_KEY ) );
         if ( licence->getID() == id && id != 0 ) {
            el = licence;
            break;
         }
      }
      
      if ( el != NULL ) {
         if ( remove ) {
            el->remove();
         } else {
            // Changed?
            if ( (licenceKey.size() != el->getLicenceLength() ||
                  memcmp( licenceKey.c_str(), el->getLicenceKey(), 
                          licenceKey.size() ) != 0) )
            {
               el->setLicence( licenceKey );
               
            }
            if ( hasKeyType && keyType != el->getKeyType() ) {
               el->setKeyType( keyType );
            }
            if ( hasProduct && product != el->getProduct() ) {
               el->setProduct( product );
            }
         }
      } else {
         // New?
         if ( !remove ) {
            UserLicenceKey* newEl = new UserLicenceKey( 0 );
            newEl->setLicence( licenceKey );
            if ( hasKeyType ) {
               newEl->setKeyType( keyType );
            }
            if ( hasProduct ) {
               newEl->setProduct( product );
            }
            user->addElement( newEl );
         } else { // else done, does not exist so it is removed!
            statusCodeString = "-1";
            statusMessageString = "Removing nonexistant user_licence_key.";
            ok = false;  
         }
      }
   }

   return ok;
}


bool 
XMLParserThread::xmlParseUserRegionAccess( 
   DOMNode* cur, UserUser* user,
   MC2String& statusCodeString, MC2String& statusMessageString ) const
{
   bool ok = true;

   uint32 id = 0;
   uint32 regionID = 0;
   uint32 startTime = 0;
   uint32 endTime = 0;
   bool remove = false;

   // Go throu attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(), "id" ) ) {
         char* tmpPtr = NULL;
         id = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = "region_access's id is not a number.";
            ok = false;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "top_region_id" ) ) 
      {
         char* tmpPtr = NULL;
         regionID = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = 
               "region_access's top_region_id is not a number.";
            ok = false;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "start_time" ) ) 
      {
         char* tmpPtr = NULL;
         startTime = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = 
               "region_access's start_time is not a number.";
            ok = false;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "end_time" ) ) 
      {
         char* tmpPtr = NULL;
         endTime = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = 
               "region_access's end_time is not a number.";
            ok = false;
         }
 
      } else {
         mc2log << warn << "XMLParserThread::xmlParseUserRegionAccess "
                << "unknown attribute for region_access element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   // Go throu users childrens and take apropriate actions
   DOMNode* child = cur->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "region_access_delete" ) ) 
            {
               remove = true;
            } else { // Odd element in region_access element
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseUserRegionAccess"
                      << " odd Element in region_access element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParseUserRegionAccess "
                   << "odd node type in region_access element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( ok && m_user->getUser()->getEditUserRights() ) {
      // Find 
      UserRegionAccess* el = NULL;
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_REGION_ACCESS ) ; 
            ++i )
      {
         if ( static_cast< UserRegionAccess* > ( user->getElementOfType( 
                 i, UserConstants::TYPE_REGION_ACCESS ) )->getID() == id &&
              id != 0 )
         {
            el = static_cast< UserRegionAccess* > ( 
               user->getElementOfType( 
                  i, UserConstants::TYPE_REGION_ACCESS ) );
            break;
         }
         
      }
      
      if ( el != NULL ) {
         if ( remove ) {
            mc2log << "Removing region id " << el->getID() << endl;
            el->remove();
         } else {
            // New time?
            // WARNING: NOT SUPPORTED
         }
      } else {
         // New?
         if ( !remove ) {
            UserRegionAccess* newEl = new UserRegionAccess( 0,
                                                            regionID,
                                                            startTime,
                                                            endTime );
            user->addElement( newEl );
         } else { // else done, does not exist so it is removed!
            statusCodeString = "-1";
            statusMessageString = "Removing nonexistant region_access.";
            ok = false;  
         }
      }

   } else {
      mc2log << info << "XMLParserThread::xmlParseUserRegionAccess "
             << "User " << m_user->getUser()->getLogonID() 
             << " may not editusers. Changed UserRegionAccess disscarded."
             << endl;
   }

   return ok;
}


bool 
XMLParserThread::xmlParseUserRight( 
   DOMNode* cur, UserUser* user,
   MC2String& statusCodeString, MC2String& statusMessageString ) const
{
   bool ok = true;

   bool idSet = false;
   uint32 id = 0;
   bool addTimeSet = false;
   uint32 addTime = TimeUtility::getRealTime();
   bool typeSet = false;
   uint64 type = 0;
   bool regionIDSet = false;
   uint32 regionID = 0;
   bool startTimeSet = false;
   uint32 startTime = 0;
   bool endTimeSet = false;
   uint32 endTime = 0;
   bool deletedSet = false;
   bool deleted = false;
   bool originSet = false;
   MC2String origin;

   // Go throu attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(), "id" ) ) {
         char* tmpPtr = NULL;
         id = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = "right's id is not a number.";
            ok = false;
         } else {
            idSet = true;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "add_time" ) ) 
      {
         char* tmpPtr = NULL;
         addTime = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = 
               "right's add_time is not a number.";
            ok = false;
         } else {
            addTimeSet = true;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "type" ) ) 
      {
         char* tmpPtr = NULL;
         type = strtoull( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = 
               "right's type is not a number.";
            ok = false;
         } else {
            typeSet = true;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "top_region_id" ) ) 
      {
         char* tmpPtr = NULL;
         regionID = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = 
               "right's top_region_id is not a number.";
            ok = false;
         } else {
            regionIDSet = true;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "start_time" ) ) 
      {
         char* tmpPtr = NULL;
         startTime = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = 
               "right's start_time is not a number.";
            ok = false;
         } else {
            startTimeSet = true;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "end_time" ) ) 
      {
         char* tmpPtr = NULL;
         endTime = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = 
               "right's end_time is not a number.";
            ok = false;
         } else {
            endTimeSet = true;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "deleted" ) ) 
      {
         deleted = StringUtility::checkBoolean( tmpStr );
         deletedSet = true;
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "origin" ) ) 
      {
         origin = tmpStr;
         originSet = true;
      } else {
         mc2log << warn << "XMLParserThread::xmlParseUserRight "
                << "unknown attribute for right element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   // Go throu users childrens and take apropriate actions
   DOMNode* child = cur->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // Odd element in right element
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseUserRight"
                   << " odd Element in right element: "
                   << child->getNodeName() << endl;
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParseUserRight "
                   << "odd node type in right element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( ok && m_user->getUser()->getEditUserRights() ) {
      // Find 
      UserRight* el = NULL;
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_RIGHT ) ; ++i )
      {
         if ( static_cast< UserRight* > ( user->getElementOfType( 
                 i, UserConstants::TYPE_RIGHT ) )->getID() == id &&
              id != 0 )
         {
            el = static_cast< UserRight* > ( 
               user->getElementOfType( 
                  i, UserConstants::TYPE_RIGHT ) );
            break;
         }
      }
      
      if ( el != NULL ) {
         // Changed?
         if ( addTimeSet && addTime != el->getAddTime() ) {
            el->setAddTime( addTime );
         }
         if ( typeSet && type != el->getUserRightType().getAsInt() ) {
            el->setUserRightType( UserEnums::URType( type ) );
         }
         if ( regionIDSet && regionID != el->getRegionID() ) {
            el->setRegionID( regionID );
         }
         if ( startTimeSet && startTime != el->getStartTime() ) {
            el->setStartTime( startTime );
         }
         if ( endTimeSet && endTime != el->getEndTime() ) {
            el->setEndTime( endTime );
         }
         if ( deletedSet && deleted != el->isDeleted() && deleted ) {
            // May only set deleted not unset
            el->setDeleted( deleted );
         }
         if ( originSet && origin.compare( el->getOrigin() ) != 0 ) {
            el->setOrigin( origin.c_str() );
         }
      } else {
         // New?
         if ( /*StringUtility::*/strncmp( origin.c_str(), "SUP: ", 5 )
              != 0 && !checkAccessToService( 
                 m_user->getUser(), UserEnums::UR_ADD_NON_SUP_RIGHTS,
                 UserEnums::UR_NO_LEVEL, MAX_UINT32, true, true ) )
         {
            statusCodeString = "-1";
            statusMessageString = 
               "right's origin not started by \"SUP: \"";
            ok = false;
         } else if ( typeSet && regionIDSet && startTimeSet && endTimeSet
                     && originSet )
         {
            UserRight* newEl = new UserRight(
               0, addTime, UserEnums::URType( type ), regionID, startTime,
               endTime, deleted, origin.c_str() );
            user->addElement( newEl );
         } else {
            ok = false;
            statusCodeString = "-1";
            statusMessageString = 
               "Not all needed attributes set for new right.";
            ok = false;
         }
      }

   } else {
      if ( ok ) {
         mc2log << info << "XMLParserThread::xmlParseUserRight "
                << "User " << m_user->getUser()->getLogonID() 
                << " may not editusers. Changed UserRight disscarded."
                << endl;
      } // Else error printed already
   }

   return ok;
}


bool 
XMLParserThread::xmlParseUserToken( 
   DOMNode* cur, UserUser* user,
   MC2String& statusCodeString, MC2String& statusMessageString ) const
{
   bool ok = true;

   bool idSet = false;
   uint32 id = 0;
   bool tokenSet = false;
   MC2String token;
   bool groupSet = false;
   MC2String group;
   bool ageSet = false;
   uint32 age = 0;
   bool createTimeSet = false;
   uint32 createTime = TimeUtility::getRealTime();
   bool deletedSet = false;
   bool deleted = false;


   // Go throu attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(), "id" ) ) {
         char* tmpPtr = NULL;
         id = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = "token's id is not a number.";
            ok = false;
         } else {
            idSet = true;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "token" ) ) 
      {
         token = tmpStr;
         tokenSet = true;
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "group" ) ) 
      {
         group = tmpStr;
         groupSet = true;
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "age" ) ) 
      {
         char* tmpPtr = NULL;
         age = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = "token's age is not a number.";
            ok = false;
         } else {
            ageSet = true;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "create_time" ) ) 
      {
         char* tmpPtr = NULL;
         createTime = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = "token's create_time is not a number.";
            ok = false;
         } else {
            createTimeSet = true;
         }
      } else {
         mc2log << warn << "XMLParserThread::xmlParseUserToken "
                << "unknown attribute for token element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   // Go throu users childrens and take apropriate actions
   DOMNode* child = cur->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            if ( XMLString::equals( child->getNodeName(),
                                    "delete" ) ) 
            {
               deletedSet = true;
               deleted = true;
            } else {
               // Odd element in token element
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseUserToken"
                      << " odd Element in token element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParseUserToken "
                   << "odd node type in token element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( ok  ) {
      // Find 
      UserToken* el = NULL;
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_TOKEN ) ; ++i )
      {
         if ( static_cast< UserToken* > ( user->getElementOfType( 
                 i, UserConstants::TYPE_TOKEN ) )->getID() == id &&
              id != 0 )
         {
            el = static_cast< UserToken* > ( 
               user->getElementOfType( 
                  i, UserConstants::TYPE_TOKEN ) );
            break;
         }
      }
      
      if ( el != NULL ) {
         // Changed?
         if ( tokenSet && token != el->getToken() ) {
            el->setToken( token.c_str() );
         }
         if ( ageSet && age != el->getAge() ) {
            el->setAge( age );
         }
         if ( createTimeSet && createTime != el->getCreateTime() ) {
            el->setCreateTime( createTime );
         }
         if ( deletedSet && deleted ) {
            el->remove();
         }
      } else {
         // New?
         if ( tokenSet ) {
            UserToken* newEl = new UserToken(
               0, createTime, age, token.c_str(), group );
            user->addElement( newEl );
         } else {
            ok = false;
            statusCodeString = "-1";
            statusMessageString = 
               "Not all needed attributes set for new token.";
            ok = false;
         }
      }

   }

   return ok;
}


bool 
XMLParserThread::xmlParseUserPIN( 
   DOMNode* cur, UserUser* user,
   MC2String& statusCodeString, MC2String& statusMessageString ) const
{
   bool ok = true;

   bool idSet = false;
   uint32 id = 0;
   bool pinSet = false;
   MC2String pin;
   bool commentSet = false;
   MC2String comment;
   bool deletedSet = false;
   bool deleted = false;


   // Go throu attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(), "id" ) ) {
         char* tmpPtr = NULL;
         id = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = "pin's id is not a number.";
            ok = false;
         } else {
            idSet = true;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "PIN" ) ) 
      {
         pin = tmpStr;
         pinSet = true;
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "comment" ) ) 
      {
         comment = tmpStr;
         commentSet = true;
      } else {
         mc2log << warn << "XMLParserThread::xmlParseUserPIN "
                << "unknown attribute for PIN element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   // Go throu users childrens and take apropriate actions
   DOMNode* child = cur->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            if ( XMLString::equals( child->getNodeName(),
                                    "delete" ) ) 
            {
               deletedSet = true;
               deleted = true;
            } else {
               // Odd element in PIN element
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseUserPIN"
                      << " odd Element in PIN element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParseUserPIN "
                   << "odd node type in PIN element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( ok  ) {
      // Find 
      UserPIN* el = NULL;
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_PIN ) ; ++i )
      {
         if ( static_cast< UserPIN* > ( user->getElementOfType( 
                 i, UserConstants::TYPE_PIN ) )->getID() == id &&
              id != 0 )
         {
            el = static_cast< UserPIN* > ( 
               user->getElementOfType( 
                  i, UserConstants::TYPE_PIN ) );
            break;
         }
      }
      
      if ( el != NULL ) {
         // Changed?
         if ( pinSet && pin != el->getPIN() ) {
            el->setPIN( pin.c_str() );
         }
         if ( commentSet && comment != el->getComment() ) {
            el->setComment( comment.c_str() );
         }
         if ( deletedSet && deleted ) {
            el->remove();
         }
      } else {
         // New?
         if ( pinSet && commentSet )
         {
            UserPIN* newEl = new UserPIN(
               0, pin.c_str(), comment.c_str() );
            user->addElement( newEl );
         } else {
            ok = false;
            statusCodeString = "-1";
            statusMessageString = 
               "Not all needed attributes set for new PIN.";
            ok = false;
         }
      }

   }

   return ok;
}


bool 
XMLParserThread::xmlParseUserWayfinderSubscription( 
   DOMNode* cur, UserUser* user,
   MC2String& statusCodeString, 
   MC2String& statusMessageString ) const
{
   bool ok = true;

   uint32 id = 0;
   byte type = 0;
   bool remove = false;

   // Go throu attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(), "id" ) ) {
         char* tmpPtr = NULL;
         id = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = 
               "wayfinder_subscription's id is not a number.";
            ok = false;
         }
      } else if ( XMLString::equals( attribute->getNodeName(), "type" ) ) {
         char* tmpPtr = NULL;
         type = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = 
               "wayfinder_subscription's type is not a number.";
            ok = false;
         }
      } else {
         mc2log << warn << "XMLParserThread::"
                << "xmlParseUserWayfinderSubscription "
                << "unknown attribute for wayfinder_subscription element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   // Go throu users childrens and take apropriate actions
   DOMNode* child = cur->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "wayfinder_subscription_delete" ) ) 
            {
               remove = true;
            } else { // Odd element in wayfinder_subscription element
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseUserWayfinderSubscription"
                      << " odd Element in wayfinder_subscription element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseUserWayfinderSubscription "
                   << "odd node type in wayfinder_subscription element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( ok && m_user->getUser()->getEditUserRights() ) {
      // Find 
      UserWayfinderSubscription* el = NULL;
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( 
               UserConstants:: TYPE_WAYFINDER_SUBSCRIPTION) ; 
            ++i )
      {
         if ( static_cast< UserWayfinderSubscription* > ( 
                 user->getElementOfType( 
                    i, UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) )
              ->getID() == id )
         {
            el = static_cast< UserWayfinderSubscription* > ( 
               user->getElementOfType( 
                  i, UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) );
            break;
         }
         
      }
      
      if ( el != NULL ) {
         if ( remove ) {
            mc2log << "Removing wayfinder subscription id " 
                   << el->getID() << endl;
            el->remove();
         } else {
            // New type?
            if ( type != el->getType() ) {
               el->setWayfinderType( type );
            }
         }
      } else {
         // New?
         if ( !remove ) {
            UserWayfinderSubscription* newEl = 
               new UserWayfinderSubscription( 0 );
            newEl->setWayfinderType( type );
            user->addElement( newEl );
         } else { // else done, does not exist so it is removed!
            statusCodeString = "-1";
            statusMessageString = 
               "Removing nonexistant wayfinder_subscription.";
            ok = false;  
         }
      }

   } else {
      mc2log << info << "XMLParserThread::"
             << "xmlParseUserWayfinderSubscription "
             << "User " << m_user->getUser()->getLogonID() 
             << " may not editusers. Changed UserWayfinderSubscription "
             << "disscarded." << endl;
   }

   return ok;   
}


bool 
XMLParserThread::xmlParseUserIDKey( 
   DOMNode* cur, UserUser* user,
   MC2String& statusCodeString, MC2String& statusMessageString ) const
{
   bool ok = true;

   bool idSet = false;
   uint32 id = 0;
   bool typeSet = false;
   UserIDKey::idKey_t type = UserIDKey::account_id_key;
   bool idKeySet = false;
   MC2String idKey;
   bool deletedSet = false;
   bool deleted = false;


   // Go throu attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(), "id" ) ) {
         char* tmpPtr = NULL;
         id = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = "IDKey's id is not a number.";
            ok = false;
         } else {
            idSet = true;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "type" ) ) 
      {
         
         type = XMLServerUtility::idKeyTypeFromString( tmpStr );
         typeSet = true;
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "key" ) ) 
      {
         idKey = tmpStr;
         idKeySet = true;
      } else {
         mc2log << warn << "XMLParserThread::xmlParseUserIDKey "
                << "unknown attribute for id_key element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   // Go throu users childrens and take apropriate actions
   DOMNode* child = cur->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            if ( XMLString::equals( child->getNodeName(),
                                    "delete" ) ) 
            {
               deletedSet = true;
               deleted = true;
            } else {
               // Odd element in id_key element
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseUserIDKey"
                      << " odd Element in id_key element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParseUserIDKey "
                   << "odd node type in id_key element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( ok  ) {
      // Find 
      UserIDKey* el = NULL;
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_ID_KEY ) ; ++i )
      {
         if ( static_cast< UserIDKey* > ( user->getElementOfType( 
                 i, UserConstants::TYPE_ID_KEY ) )->getID() == id &&
              id != 0 )
         {
            el = static_cast< UserIDKey* > ( 
               user->getElementOfType( 
                  i, UserConstants::TYPE_ID_KEY ) );
            break;
         }
      }
      
      if ( el != NULL ) {
         // Changed?
         if ( idKeySet && idKey != el->getIDKey() ) {
            el->setIDKey( idKey.c_str() );
         }
         if ( typeSet && type != el->getIDType() ) {
            el->setIDType( type );
         }
         if ( deletedSet && deleted ) {
            el->remove();
         }
      } else {
         // New?
         if ( idKeySet ) {
            UserIDKey* newEl = new UserIDKey( 0 );
            newEl->setIDKey( idKey );
            if ( typeSet ) {
               newEl->setIDType( type );
            }
            user->addElement( newEl );
         } else {
            ok = false;
            statusCodeString = "-1";
            statusMessageString = 
               "Not all needed attributes set for new id_key.";
            ok = false;
         }
      }

   }

   return ok;
}


bool 
XMLParserThread::xmlParseUserLoginRequest( DOMNode* cur, 
                                           DOMNode* out,
                                           DOMDocument* reply,
                                           bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );

   char* userLogin = NULL;
   char* userPassword = NULL;
   char* serviceType = NULL;
   bool createSession = false;

   // Create user_login_reply element
   DOMElement* user_login_reply = 
      reply->createElement( X( "user_login_reply" ) );
   // Transaction ID
   user_login_reply->setAttribute( 
      X( "transaction_id" ),
      cur->getAttributes()->getNamedItem( X( "transaction_id" ) )
      ->getNodeValue() );
   out->appendChild( user_login_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( reply->createTextNode( X( indentStr.c_str() ) ), 
                         user_login_reply );
   }


   // Read attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      
      if ( XMLString::equals( attribute->getNodeName(),
                              "user_create_session" ) ) 
      {
         createSession = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "client_type" ) ) 
      {
         m_authData->clientType = tmpStr;
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "transaction_id" ) ) 
      {
         // Already used above
      } else {
         mc2log << warn << "XMLParserThread::"
                << "xmlParseUserLoginRequest "
                << "Preferences unknown attribute "
                << "Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }

      delete [] tmpStr;
   }
         

   // Go throu the elements and handle them.
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), "user_name" ) ) {
               userLogin = XMLUtility::getChildTextValue( child );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "user_password" ) ) 
            {
               userPassword = XMLUtility::getChildTextValue( child );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "user_service" ) ) 
            {
               serviceType = XMLUtility::getChildTextValue( child );
            } else {
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseUserLoginRequest "
                      << "odd Element in user_login_request "
                      << "element: " << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParseUserLoginRequest "
                   << "odd node type in user_login_request element: "
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   m_authData->clientSetting = m_group->getSetting( 
      m_authData->clientType, "" );

   UserEnums::userRightLevel levelmask = 
      getUrLevel( m_authData->clientSetting  );


   if ( userLogin != NULL && userPassword != NULL ) {
      mc2dbg8 << "userLogin " << userLogin<< endl;
      UserRequestPacket* p = NULL;
      if ( createSession ) {
         p = new CheckUserPasswordRequestPacket( 0, 0, 
                                                 userLogin, userPassword,
                                                 true );
      } else {
         p = new AuthUserRequestPacket( 0, 0, userLogin, userPassword, 
                                        true );
      }

      SinglePacketRequest* req = new SinglePacketRequest( 
         m_group->getNextRequestID(), 
         new PacketContainer( p, 0, 0, MODULE_TYPE_USER ) ); 
      ThreadRequestContainer* reqCont = new ThreadRequestContainer( req );
      mc2dbg8 << "About to send UserRequest" << endl;
      putRequest( reqCont );
      mc2dbg8 << "UserRequest returned" << endl;
      PacketContainer* ansCont = req->getAnswer();
      if ( ansCont != NULL &&
           static_cast< ReplyPacket* > ( 
              ansCont->getPacket() )->getStatus() == StringTable::OK )
      {
         uint32 UIN = 0;
         if ( createSession ) {
            UIN = static_cast< CheckUserPasswordReplyPacket* > ( 
               ansCont->getPacket() )->getUIN();
         } else {
            UIN = static_cast< AuthUserReplyPacket* > ( 
               ansCont->getPacket() )->getUIN();
         }

         if ( UIN != 0 && UIN != (MAX_UINT32 -1) ) {
            UserItem* userItem = NULL;
            if ( getUser( UIN, userItem, true ) ) {
               bool serviceOk = true;

               if ( serviceType != NULL ) {
                  serviceOk = checkUserAccess( userItem->getUser(),
                                               serviceType );
               }
               
               if ( serviceOk ) {
                  char ctmp[20];
                  sprintf( ctmp, "%u", static_cast< UserReplyPacket* > ( 
                     ansCont->getPacket() )->getUIN() );
                  appendStatusNodes( user_login_reply, reply, 
                                     indentLevel + 1,
                                     indent,
                                     ctmp, "Login ok." );
                  mc2log << "XMLParserThread::"
                         << "xmlParseUserLoginRequest User login ok."
                         << " UserName " 
                         << userItem->getUser()->getLogonID() 
                         << "(" << userItem->getUIN() << ")"<< endl;
                  if ( createSession ) {
                     CheckUserPasswordReplyPacket* r = static_cast<
                        CheckUserPasswordReplyPacket* > ( 
                           ansCont->getPacket() );
                     MC2String sessionIndentStr( (indentLevel + 1)*3, ' ' );
                     sessionIndentStr.insert( 0, "\n" );

                     // Session ID
                     DOMElement* user_session_id = 
                        reply->createElement( X( "user_session_id" ) );
                     user_session_id->appendChild( 
                        reply->createTextNode( X( r->getSessionID() ) ) );
                     if ( indent ) {
                        user_login_reply->appendChild( 
                           reply->createTextNode( 
                              X( sessionIndentStr.c_str() ) ) );
                     }
                     user_login_reply->appendChild( user_session_id );
               
                     // Session key
                     DOMElement* user_session_key = 
                        reply->createElement( X( "user_session_key" ) );
                     user_session_key->appendChild( 
                        reply->createTextNode( X( r->getSessionKey() ) ) );
                     if ( indent ) {
                        user_login_reply->appendChild( 
                           reply->createTextNode( 
                              X( sessionIndentStr.c_str() ) ) );
                     }
                     user_login_reply->appendChild( user_session_key );   
                  }
               } else {
                  if ( checkUserAccess( userItem->getUser(),
                                        serviceType, false, levelmask ) ) 
                  {
                     mc2log << info 
                            << "XMLParserThread::xmlParseUserLoginRequest "
                            << "User expired, userName " << userLogin 
                            << endl;
                     appendStatusNodes( user_login_reply, reply, 
                                        indentLevel + 1, indent,
                                        "-206", "Expired user." );
                  } else {
                     mc2log << warn << "XMLParserThread::"
                            << "xmlParseUserLoginRequest User has no "
                            << "access to service! UserName " 
                            << userItem->getUser()->getLogonID() 
                            << "(" << userItem->getUIN() << ")" << endl;
                     appendStatusNodes( user_login_reply, reply, 
                                        indentLevel + 1, indent,
                                        "-201",
                                        "Access denied." );
                  }
               }
            } else {
               mc2log << warn << "XMLParserThread::"
                         "xmlParseUserLoginRequest "
                         "failed to get user from database, UserName " 
                      << userLogin << "(" << UIN << ")" << endl;
               appendStatusNodes( 
                  user_login_reply, reply, indentLevel + 1, indent,
                  "-3", "Database connection error, no connection." );
            }
            releaseUserItem( userItem );
         } else {
            if ( UIN == 0 ) {
               mc2log << info 
                      << "XMLParserThread::xmlParseUserLoginRequest "
                      << "Login failed, userName " << userLogin << endl;
               appendStatusNodes( user_login_reply, reply, indentLevel + 1,
                                  indent,
                                  "-203",
                                  "Invalid login." );
            } else if ( UIN == (MAX_UINT32 -1) ) {
               mc2log << info 
                      << "XMLParserThread::xmlParseUserLoginRequest "
                      << "User expired, userName " << userLogin << endl;
               appendStatusNodes( user_login_reply, reply, indentLevel + 1,
                                  indent,
                                  "-206",
                                  "Expired user." );
            }
         }
      } else {
         mc2log << warn << "XMLParserThread::xmlParseUserLoginRequest "
                << "user request failed. Status ";
         if ( ansCont != NULL ) {
            mc2log << " UIN " << 
               static_cast< UserReplyPacket* > ( 
                  ansCont->getPacket() )->getUIN() << endl;
            mc2log << " statuscode " << 
               StringTable::getString( 
                  StringTable::stringCode( 
                     static_cast< ReplyPacket* > (
                        ansCont->getPacket() )->getStatus() ),
                  StringTable::ENGLISH ) 
                   << endl;
         } else {
            mc2log << " NULL reply->" << endl;
         }
         appendStatusNodes( user_login_reply, reply, indentLevel + 1, 
                            indent,
                            "-3",
                            "Database connection failed." );
      }

      delete ansCont;
      delete reqCont;
      delete req;
   } else {
      appendStatusNodes( user_login_reply, reply, indentLevel + 1, indent,
                         "-1",
                         "Error in inparameters." );
      mc2log << warn << "XMLParserThread::xmlParseUserLoginRequest "
             << "not username and password in inparameters!" << endl;
   }

   if ( indent ) {
      // Newline and indent before end user_login_reply tag   
      user_login_reply->appendChild( 
         reply->createTextNode( X( indentStr.c_str() ) ) );
   }

   delete [] userLogin;
   delete [] userPassword;
   delete [] serviceType;

   return ok;
}


bool
XMLParserThread::xmlParseUserVerifyRequest( DOMNode* cur, 
                                            DOMNode* out,
                                            DOMDocument* reply,
                                            bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   char* userSessionID = NULL;
   char* userSessionKey = NULL;

   // Create user_verify_reply element
   DOMElement* user_verify_reply = 
      reply->createElement( X( "user_verify_reply" ) );
   // Transaction ID
   user_verify_reply->setAttribute( 
      X( "transaction_id" ),
      cur->getAttributes()->getNamedItem( X( "transaction_id" ) )
      ->getNodeValue() );
   out->appendChild( user_verify_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), user_verify_reply );
   }
   
   
   // Go throu the elements and handle them.
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "user_session_id" ) ) 
            {
               userSessionID = XMLUtility::getChildTextValue( child );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "user_session_key" ) )
            {
               userSessionKey = XMLUtility::getChildTextValue( child );
            } else {
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseUserVerifyRequest odd Element in "
                      << "user_verify_request element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParseUserVerifyRequest "
                   << "odd node type in user_verify_request element: "
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( userSessionID != NULL && userSessionKey != NULL ) {
      uint32 UIN = authenticateUserSession( userSessionID, userSessionKey, 
                                            true );
      mc2dbg2 << "authenticateUserSession returned UIN " << UIN
              << endl;

      if ( UIN != 0 && UIN != MAX_UINT32 && UIN != (MAX_UINT32 -1) &&
           UIN != (MAX_UINT32 -2) ) 
      {
         mc2log << info << "XMLParserThread::xmlParseUserVerifyRequest "
                << "verify session OK. UIN " << UIN << endl;
         appendStatusNodes( user_verify_reply, reply, indentLevel + 1, 
                            indent,
                            "0", "Verify succeeded." );
      } else if ( UIN == 0 ) {
         mc2log << info << "XMLParserThread::xmlParseUserVerifyRequest "
                << "verify session not accepted sessionID "
                << userSessionID << endl;
         appendStatusNodes( user_verify_reply, reply, indentLevel + 1, 
                            indent,
                            "-204", "Invalid session." );
      } else if ( UIN == MAX_UINT32 ) {
         mc2log << info << "XMLParserThread::xmlParseUserVerifyRequest "
                << "verify session expired sessionID "
                << userSessionID << endl;
         appendStatusNodes( user_verify_reply, reply, indentLevel + 1, 
                            indent,
                            "-205",
                            "Session has expired, login again." );
      } else if ( UIN == (MAX_UINT32 -1) ) {
         mc2log << info << "XMLParserThread::xmlParseUserVerifyRequest "
                << "verify session expired user "
                << userSessionID << endl;
         appendStatusNodes( user_verify_reply, reply, indentLevel + 1, 
                            indent,
                            "-206", "Expired user." );
      } else if ( UIN == (MAX_UINT32 -2) ) {
         mc2log << info << "XMLParserThread::xmlParseUserVerifyRequest "
                << "verify session database connection problem, sessionID "
                << userSessionID << endl;
         appendStatusNodes( user_verify_reply, reply, indentLevel + 1, 
                            indent,
                            "-3", "Database connection error." );
      }

   } else {
      appendStatusNodes( user_verify_reply, reply, indentLevel + 1, indent,
                         "-1",
                         "Error in inparameters." );
   }

   if ( indent ) {
      // Newline and indent before end user_verify_reply tag   
      user_verify_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   delete [] userSessionID;
   delete [] userSessionKey;
   
   return ok;  
}


bool 
XMLParserThread::xmlParseUserLogoutRequest( DOMNode* cur, 
                                            DOMNode* out,
                                            DOMDocument* reply,
                                            bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   char* userSessionID = NULL;
   char* userSessionKey = NULL;

   // Create user_logout_reply element
   DOMElement* user_logout_reply = 
      reply->createElement( X( "user_logout_reply" ) );
   // Transaction ID
   user_logout_reply->setAttribute( 
      X( "transaction_id" ),
      cur->getAttributes()->getNamedItem( X( "transaction_id" ) )
      ->getNodeValue() );
   out->appendChild( user_logout_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), user_logout_reply );
   }
   
   
   // Go throu the elements and handle them.
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "user_session_id" ) ) 
            {
               userSessionID = XMLUtility::getChildTextValue( child );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "user_session_key" ) )
            {
               userSessionKey = XMLUtility::getChildTextValue( child );
            } else {
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseUserLogoutRequest odd Element in "
                      << "user_logout_request element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParseUserLogoutRequest "
                   << "odd node type in user_logout_request element: "
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( userSessionID != NULL && userSessionKey != NULL ) {
      LogoutUserRequestPacket* p = new LogoutUserRequestPacket(
         0, 0, userSessionID, userSessionKey );
      SinglePacketRequest* req = new SinglePacketRequest( 
         m_group->getNextRequestID(), 
         new PacketContainer( p, 0, 0, MODULE_TYPE_USER ) ); 
      ThreadRequestContainer* reqCont = new ThreadRequestContainer( req );
      mc2dbg8 << "About to send UserLogoutRequest" << endl;
      putRequest( reqCont );
      mc2dbg8 << "UserLogoutRequest returned" << endl;
      PacketContainer* ansCont = req->getAnswer();
      if ( ansCont != NULL &&
           static_cast< ReplyPacket* > ( 
              ansCont->getPacket() )->getStatus() == StringTable::OK )
      {
         if ( static_cast< LogoutUserReplyPacket* > ( 
            ansCont->getPacket() )->getUIN() != 0 )
         {
            appendStatusNodes( user_logout_reply, reply, indentLevel + 1, 
                               indent,
                               "0",
                               "Logout succedded." );
            mc2log << info << "UserLogout OK" << endl;
         } else {
            appendStatusNodes( user_logout_reply, reply, indentLevel + 1, 
                               indent,
                               "-1",
                               "Logout failed." );
            mc2log << info << "UserLogout Error Logout failed." << endl;
         }
      } else {
         mc2log << warn <<  "XMLParserThread::xmlParseUserLogoutRequest "
                << "logout user request failed.";
         if ( ansCont != NULL ) {
            mc2log << " UIN " << 
               static_cast< UserReplyPacket* > ( 
                  ansCont->getPacket() )->getUIN() << endl;
            mc2log << "   status " << 
               StringTable::getString( 
                  StringTable::stringCode( 
                     static_cast< ReplyPacket* > (
                        ansCont->getPacket() )->getStatus() ),
                  StringTable::ENGLISH ) << endl;
         } else {
            mc2log << " NULL reply->" << endl;
         }
         appendStatusNodes( user_logout_reply, reply, indentLevel + 1, 
                            indent,
                            "-3",
                            "Database connection failed." );
      }

      delete ansCont;
      delete reqCont;
      delete req;
   } else {
      appendStatusNodes( user_logout_reply, reply, indentLevel + 1, 
                         indent,
                         "-1",
                         "Error in inparameters." );
      mc2log << warn << "UserLogout: Error Error in inparameters" << endl;
   }

   if ( indent ) {
      // Newline and indent before end user_logout_reply tag   
      user_logout_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   delete [] userSessionID;
   delete [] userSessionKey;
   
   return ok;
}

bool
XMLParserThread::xmlParseUserCapRequest( DOMNode* cur, DOMNode* outPtr,
                                         DOMDocument* replyPtr, bool indent ) 
{
   MC2_ASSERT( m_user );
   MC2_ASSERT( m_user->getUser() );

   // Before we compose anything, make sure we have a valid request
   
   DOMNamedNodeMap* attributes = cur->getAttributes();
   MC2_ASSERT( attributes );
   const uint32 nbrAttributes = attributes->getLength();
   for ( uint32 i = 0 ; i < nbrAttributes; ++i ) {
      const XMLCh* nodeName = attributes->item( i )->getNodeName();
      if ( XMLString::equals( nodeName, "transaction_id" ) ) {
         // valid attrib, used later
      } else {
         mc2log << warn << "XMLParserThread::"
                << "xmlParseUserCapRequest "
                << "user_cap_request unknown attribute "
                << "Name " << nodeName
                << " Type " << attributes->item( i )->getNodeType() << endl;
      }
   }


   DOMNode &out = *outPtr;
   DOMDocument &reply = *replyPtr;


   // setup head
   DOMElement* user_cap_reply = reply.createElement( X( "user_cap_reply" ) );
   user_cap_reply->setAttribute( X( "transaction_id" ),
                                 cur->getAttributes()->
                                 getNamedItem( X( "transaction_id" ) )->getNodeValue() );
   out.appendChild( user_cap_reply );
   
   UserUser &userUser = *m_user->getUser();

   using XMLServerUtility::appendElementWithText;
   appendElementWithText( user_cap_reply, replyPtr, 
                          "user_id", userUser.getLogonID(),
                          0, indent );

   struct RightStrings {
      const char* m_rightStr;
      ClientRights::Rights m_clientRight;
   } rightStrings[] = {
      { "locator", ClientRights::POSITIONING },
      { "fleet", ClientRights::FLEET },
      { "gps",  ClientRights::CONNECT_GPS },
      { "route", ClientRights::ROUTE },
      { "traffic", ClientRights::TRAFFIC },
   };

   // get "rights" a.k.a "capabilities" a.k.a "cap".
   ClientRights::ClientRights possibleRights = 
      ClientRights::getClientRights( userUser );

   // 20080228 All Earth clients should have USE_GPS
   if ( checkIfIronClient( m_authData->clientSetting ) ) {
      possibleRights.insert( ClientRights::CONNECT_GPS );
   }

   // 20080421 All, precisely all, should have locator.
   possibleRights.insert( ClientRights::POSITIONING );

   // now we got a uniq set of "capabilities"  a.k.a client rights strings to 
   // compare with the clientrights <-> string and add cap tag(s)
   for ( uint32 i = 0; 
         i < sizeof( rightStrings ) / sizeof( rightStrings[ 0 ] );
         ++i ) {
      // find right strings
      ClientRights::ClientRights::iterator rightIt = 
         possibleRights.find( rightStrings[ i ].m_clientRight );
      if ( rightIt == possibleRights.end() ) {
         continue;
      }

      DOMElement* capElement = reply.createElement( X( "cap" ) );
      capElement->setAttribute( X( "name" ), 
                                X( rightStrings[ i ].m_rightStr ) );

      user_cap_reply->appendChild( capElement );
   }

   // add pin tag(s)
   // do we want them or not?

   UserUser::constUserElRange_t 
      itRange = userUser.getElementRange( UserConstants::TYPE_PIN );

   UserUser::constUserElRange_t::first_type rightIt = itRange.first;
   for (; rightIt != itRange.second; ++rightIt ) {
      MC2_ASSERT( *rightIt );
      UserPIN& pin = static_cast<UserPIN&>( **rightIt );
      // add pin with id, comment and PIN
      user_cap_reply->appendChild( ::createPINElement( reply, 
                                                       pin, true, true ) );
   }

   // Popup
   MC2String popOnce;
   MC2String popMessage;
   MC2String popURL;
   PopupEnums::popup_url_t popURLType = PopupEnums::popup_no_type;

   if ( getExternalAuth()->getPopup( &userUser, *m_authData, popOnce,
                                     popMessage, popURL, popURLType ) ) {
      // Add popup
      DOMElement* popElement = reply.createElement( X( "popup" ) );
      // Message
      appendElementWithText( popElement, replyPtr, "popup_message",
                             popMessage.c_str(), 0, false );
      // Once
      if ( !popOnce.empty() ) {
         appendElementWithText( popElement, replyPtr, "popup_once",
                                popOnce.c_str(), 0, false );
      }
      // URL
      if ( !popURL.empty() ) {
         XMLServerUtility::attrVect attr; 
         if ( popURLType != PopupEnums::popup_no_type ) {
            // Move type to string conversion to own method when needed.
            attr.push_back ( 
               XMLServerUtility::stringStringStruct( 
                  "url_type", popURLType == PopupEnums::popup_yes_no ?
                  "yes_no" : "goto_or_exit" ) );
         }
         appendElementWithText( popElement, replyPtr, "popup_url",
                                popURL.c_str(), 0, false, &attr );
      }
      user_cap_reply->appendChild( popElement );
   }

   // Now indent if necessary.
   if ( indent ) {
      XMLUtility::indentPiece( *user_cap_reply, 1 );
   }
   return true;

}

bool
XMLParserThread::xmlParseUserShowRequest( DOMNode* cur, 
                                          DOMNode* out,
                                          DOMDocument* reply,
                                          bool indent )
{
   bool ok = true;
   int indentLevel = 1;
   MC2String statusCodeString;
   MC2String statusMessageString;

   uint32 uin = 0;
   bool wipeFromCache = false;
   bool showAll = true;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   // Create user_show_reply element
   DOMElement* user_show_reply = 
      reply->createElement( X( "user_show_reply" ) );
   // Transaction ID
   user_show_reply->setAttribute( 
      X( "transaction_id" ),
      cur->getAttributes()->getNamedItem( X( "transaction_id" ) )
      ->getNodeValue() );
   out->appendChild( user_show_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), user_show_reply );
   }
   
   // Go throu the attributes and handle them.
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() && ok ; i++ ) {
      attribute = attributes->item( i );
      ScopedArray<char> 
         tmpStrPtr( XMLUtility::
                    transcodefromucs( attribute->getNodeValue() ));
      char* tmpStr = tmpStrPtr.get();
      
      if ( XMLString::equals( attribute->getNodeName(), "uin" ) ) {
         char* tmpPtr = NULL;
         uin = strtoul( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            statusCodeString = "-1";
            statusMessageString = "uin is not a number.";
            ok = false;
         }
      } else if ( XMLString::equals( attribute->getNodeName(), 
                                     "cached_data" ) ) {
         wipeFromCache = !StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(), 
                                     "show" ) ) {
         if ( StringUtility::strcmp( tmpStr, "active" ) == 0 ) {
            showAll = false;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "transaction_id" ) ) {
         // Already used above
      } else {
         mc2log << warn << "XMLParserThread::"
                << "xmlParseUserShowRequest "
                << "user_show_request unknown attribute "
                << "Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
   }


   typedef ScopedArray<char> CArray;
   XMLUtility util;

   MC2String userID;
   MC2String user_session_id;
   MC2String user_session_key;

   // Go throu the elements and handle them.
   DOMNode* child = cur->getFirstChild();
   struct {
      const char* nodeName;
      MC2String* value;
   } elements[] = {
      { "user_id", &userID },
      { "user_session_id", &user_session_id },
      { "user_session_key", &user_session_key }
   };

   const uint32 nbrElements = sizeof ( elements ) / sizeof ( elements[ 0 ] );

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
      case DOMNode::ELEMENT_NODE: {
         uint32 i = 0;
         for ( ; i < nbrElements; ++i ) {
            if ( XMLString::equals( child->getNodeName(),
                                    elements[ i ].nodeName ) ) {
               *elements[ i ].value = 
                  CArray(util.getChildTextValue( child ) ).get();
               break;
            }
         }
         if ( i == nbrElements ) {
            mc2log << warn << "XMLParserThread::xmlParseUserShowRequest"
                   << " odd Element in user_show_request element: "
                   << child->getNodeName() << endl;
         }
      } break;
      case DOMNode::COMMENT_NODE :
         // Ignore comments
         break;
      default:
         mc2log << warn << "XMLParserThread::xmlParseUserShowRequest "
                << "odd node type in user_show_request element: "
                << child->getNodeName() 
                << " type " << child->getNodeType() << endl;
         break;
      }
      child = child->getNextSibling();
   }

   
   if ( ok && ( !userID.empty() || 
                ( !user_session_id.empty() && !user_session_key.empty() )||
                uin != 0 ) )
   {
      UserItem* userItem = NULL;
      bool status = true;

      if ( uin != 0 ) {
         status = getUser( uin, userItem, true, wipeFromCache );
      } else if ( !userID.empty() ) {
         status = getUser( userID.c_str(), userItem, true, wipeFromCache );
      } else {
         status = getUserBySession( 
            user_session_id.c_str(), user_session_key.c_str(), userItem,
            true, wipeFromCache );
      }

      if ( status ) {
         if ( userItem != NULL ) {
            if ( m_user->getUser()->getEditUserRights() ||
                 m_user->getUIN() == userItem->getUser()->getUIN() )
            {
               appendUser( userItem, user_show_reply, reply, 
                           indentLevel + 1, indent, showAll );
               mc2log << info << "UserShow OK user " 
                      << userItem->getUser()->getLogonID() << "("
                      << userItem->getUser()->getUIN() << ")" << endl;
            } else {
               // Not allowed
               mc2log << warn 
                      << "XMLParserThread::xmlParseUserShowRequest "
                      << "Access denied to userID " << userID 
                      << " user_session_id " << user_session_id
                      << " user_session_key " << user_session_key 
                      << " uin " << uin
                      << " for " << m_user->getUser()->getLogonID()
                      << endl;
               appendStatusNodes( user_show_reply, reply, indentLevel + 1, 
                                  indent,
                                  "-201", "Access denied." );
            }
         } else {
            mc2log << warn << "XMLParserThread::xmlParseUserShowRequest "
                      "unknown user, "
                      " userID: " << userID 
                   << " user_session_id " << user_session_id
                   << " user_session_key " << user_session_key 
                   << " uin " << uin << endl;
            appendStatusNodes( user_show_reply, reply, indentLevel + 1, 
                               indent,
                               "-202", "Unknown user, "
                               "check your spelling and try again." );  
         }
         
      } else {
         mc2log << warn << "XMLParserThread::xmlParseUserShowRequest "
                   "Database connection error, "
                   "couldn't get user from userID: " << userID 
                << " user_session_id " << user_session_id
                << " user_session_key " << user_session_key 
                << " uin " << uin << endl;
         appendStatusNodes( user_show_reply, reply, indentLevel + 1, 
                            indent,
                            "-3", "Database connection error, "
                            "please try again." );
      }

      releaseUserItem( userItem );
   } else {
      if ( ok ) {
         appendStatusNodes( user_show_reply, reply, indentLevel + 1, 
                            indent,
                            "-1", "No user specified." );
         mc2log << info << "UserShow Error: No user specified" << endl;
      } else {
         appendStatusNodes( user_show_reply, reply, indentLevel + 1, 
                            indent, statusCodeString.c_str(), 
                            statusMessageString.c_str() );
         mc2log << info << "UserShow Error: " << statusCodeString << ","
                << statusMessageString << endl;
      }
   }


   if ( indent ) {
      // Newline and indent before end user_show_reply tag   
      user_show_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   return ok;
}


void 
XMLParserThread::appendUser( UserItem* userItem, 
                             DOMNode* cur, DOMDocument* reply,
                             int indentLevel, bool indent,
                             bool showAll )
{
   UserUser* userUser = userItem->getUser();
   int childIndentLevel = indentLevel + 1;
   MC2String indentStr( indentLevel*3, ' ' );
   MC2String childIndentStr( childIndentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   childIndentStr.insert( 0, "\n" );
   XStr XchildIndentStr( childIndentStr.c_str() );
   uint32 now = TimeUtility::getRealTime();

   DOMElement* user = reply->createElement( X( "user" ) );

   // Attributes
   // uin
   user->setAttribute( X( "uin" ), XUint32( userUser->getUIN() ) );
   // birth_date
   user->setAttribute( X( "birth_date" ), 
                       X( userUser->getBirthDate() ) );
   // route_cost
   RouteTypes::routeCostType routeCost = RouteTypes::costToRouteCostType(
      userUser->getRouting_costA(), userUser->getRouting_costB(),
      userUser->getRouting_costC(), userUser->getRouting_costC() );
   user->setAttribute( X( "route_cost" ), 
                       X( RouteTypes::routeCostTypeToString( routeCost )));
   // route_vehicle
   user->setAttribute( X( "route_vehicle" ), 
                       X( StringConversion::vehicleTypeToString(
                             userUser->getRouting_vehicle() ) ) );
   // search_match_type
   user->setAttribute( 
      X( "search_match_type" ), 
      X( StringConversion::searchStringMatchingTypeToString(
            SearchTypes::StringMatching(
               userUser->getSearch_type() ) ) ) );
   // search_word_match_type
   user->setAttribute( X( "search_word_match_type" ),
                       X( StringConversion::searchStringPartTypeToString(
                             SearchTypes::StringPart( 
                                userUser->getSearch_substring() ) ) ) );
   // search_sort_type
   user->setAttribute( X( "search_sort_type" ),
                       X( StringConversion::searchSortingTypeToString(
                             SearchTypes::SearchSorting( 
                                userUser->getSearch_sorting() ) ) ) );
   // valid_date
   user->setAttribute( X( "valid_date" ), 
                       XUint32( userUser->getValidDate() ) );
   // edit_user_right
   user->setAttribute( X( "edit_user_right" ), 
                       X( StringUtility::booleanAsString(
                             userUser->getEditUserRights() ) ) );

   // address1
   user->setAttribute( X( "address1" ), 
                       X( userUser->getAddress1() ) );
   // address2
   user->setAttribute( X( "address2" ), 
                       X( userUser->getAddress2() ) );
   // address3
   user->setAttribute( X( "address3" ), 
                       X( userUser->getAddress3() ) );
   // address4
   user->setAttribute( X( "address4" ), 
                       X( userUser->getAddress4() ) );
   // address5
   user->setAttribute( X( "address5" ), 
                       X( userUser->getAddress5() ) );

   using namespace XMLServerUtility;
   // route_turn_image
   user->setAttribute( X( "route_turn_image" ), 
                       X( routeTurnImageTypeToString( 
                             userUser->getRouteTurnImageType() ) ) );

   // overview_image_type
   user->setAttribute( X( "overview_image_type" ), 
                       X( overviewImageTypeToString( 
                             userUser->getRouteImageType() ) ) );

   // transactionBased
   user->setAttribute( X( "transactionBased" ), 
                       X( transactionBasedTypeToString(
                             userUser->getTransactionBased() ) ) );

   // deviceChanges
   MC2String nbr;
   STLStringUtility::int2str( userUser->getDeviceChanges(), nbr );
   user->setAttribute( X( "deviceChanges" ), X( nbr.c_str() ) );

   // supportComment
   if ( m_user->getUser()->getEditUserRights() ) {
      user->setAttribute( X( "supportComment" ), 
                          X( userUser->getSupportComment() ) );
   }

   // postalCity
   user->setAttribute( X( "postalCity" ), 
                       X( userUser->getPostalCity() ) );

   // zipCode
   user->setAttribute( X( "zipCode" ), 
                       X( userUser->getZipCode() ) );

   // companyName
   user->setAttribute( X( "companyName" ), 
                       X( userUser->getCompanyName() ) );

   // companyReference
   user->setAttribute( X( "companyReference" ), 
                       X( userUser->getCompanyReference() ) );

   // companyVATNbr
   user->setAttribute( X( "companyVATNbr" ), 
                       X( userUser->getCompanyVATNbr() ) );

   // emailBounces
   nbr.clear();
   STLStringUtility::int2str( userUser->getEmailBounces(), nbr );
   user->setAttribute( X( "emailBounces" ), X( nbr.c_str() ) );


   // addressBounces
   nbr.clear();
   STLStringUtility::int2str( userUser->getAddressBounces(), nbr );
   user->setAttribute( X( "addressBounces" ), X( nbr.c_str() ) );

   // customerContactInfo
   user->setAttribute( X( "customerContactInfo" ), 
                       X( userUser->getCustomerContactInfo() ) );

   // brand_origin
   user->setAttribute( X( "brand_origin" ),
                       X( userUser->getBrandOrigin() ) );

   // brand
   user->setAttribute( X( "brand" ),
                       X( userUser->getBrand() ) );

   using XMLServerUtility::appendElementWithText;

   // user_id
   appendElementWithText( user, reply, "user_id", userUser->getLogonID(), 
                          childIndentLevel, indent  );

   // first_name
   appendElementWithText( user, reply, "first_name", 
                          userUser->getFirstname(), 
                          childIndentLevel, indent  );
   // last_name
   appendElementWithText( user, reply, "last_name", 
                          userUser->getLastname(), 
                          childIndentLevel, indent  );

   // initials
   appendElementWithText( user, reply, "initials", 
                          userUser->getInitials(), 
                          childIndentLevel, indent  );

   // language
   appendElementWithText( user, reply, "language", 
                          StringTable::getString(
                             StringTable::getLanguageAsStringCode( 
                                userUser->getLanguage() ),
                             StringTable::ENGLISH ),
                          childIndentLevel, indent  );
   
   // measurement_system
   appendElementWithText( user, reply, "measurement_system", 
                          userUser->getMeasurementSystem() == 
                          UserConstants::MEASUREMENTTYPE_IMPERIAL ?
                          "imperial" : "metric",
                          childIndentLevel, indent  );

   // email_address
   appendElementWithText( user, reply, "email_address", 
                          userUser->getEmailAddress(),
                          childIndentLevel, indent  );

   // operator_comment
   if ( m_user->getUser()->getEditUserRights() ) {
      appendElementWithText( user, reply, "operator_comment", 
                             userUser->getOperatorComment(),
                             childIndentLevel, indent  );
   }

   // search_for_municipal
   if ( userUser->getSearchForLocationTypes() & SEARCH_MUNICIPALS ) {
      appendElement( user, reply, "search_for_municipal", 
                     childIndentLevel, indent  );
   }

   // search_for_city
   if ( userUser->getSearchForLocationTypes() & SEARCH_BUILT_UP_AREAS ) {
      appendElement( user, reply, "search_for_city", 
                     childIndentLevel, indent  );
   }

   // search_for_citypart
   if ( userUser->getSearchForLocationTypes() & SEARCH_CITY_PARTS ) {
      appendElement( user, reply, "search_for_citypart", 
                     childIndentLevel, indent  );
   }

   // search_for_zipcode
   if ( userUser->getSearchForLocationTypes() & SEARCH_ZIP_CODES ) {
      appendElement( user, reply, "search_for_zipcode", 
                     childIndentLevel, indent  );
   }

   // search_for_ziparea
   if ( userUser->getSearchForLocationTypes() & SEARCH_ZIP_AREAS ) {
      appendElement( user, reply, "search_for_ziparea", 
                     childIndentLevel, indent  );
   }

   // search_for_street
   if ( userUser->getSearchForTypes() & SEARCH_STREETS ) {
      appendElement( user, reply, "search_for_street", 
                     childIndentLevel, indent  );
   }

   // search_for_company
   if ( userUser->getSearchForTypes() & SEARCH_COMPANIES ) {
      appendElement( user, reply, "search_for_company", 
                     childIndentLevel, indent  );
   }

   // search_for_category
   if ( userUser->getSearchForTypes() & SEARCH_CATEGORIES ) {
      appendElement( user, reply, "search_for_category", 
                     childIndentLevel, indent  );
   }

   // search_for_misc
   if ( userUser->getSearchForTypes() & SEARCH_MISC ) {
      appendElement( user, reply, "search_for_misc", 
                     childIndentLevel, indent  );
   }

   // new_password Should not be sent to user
   // old_password Should not be sent to user


   // service*
   // WAP
   if ( userUser->getWAPService() ) {
      DOMElement* service = reply->createElement( X( "service" ) );
 
      appendElementWithText( service, reply, "service_type", 
                             "ROUTE",
                             childIndentLevel + 1, indent  );
      appendElementWithText( service, reply, "service_method", 
                             "WAP",
                             childIndentLevel + 1, indent  );

      if ( indent ) {
         service->appendChild( reply->createTextNode( 
                                  XchildIndentStr.XMLStr() ) );
      }
      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( service );
   }

   // SMS
   if ( userUser->getSMSService() ) {
      DOMElement* service = reply->createElement( X( "service" ) );
 
      appendElementWithText( service, reply, "service_type", 
                             "ROUTE",
                             childIndentLevel + 1, indent  );
      appendElementWithText( service, reply, "service_method" , 
                             "SMS",
                             childIndentLevel + 1, indent  );

      if ( indent ) {
         service->appendChild( reply->createTextNode( 
                                  XchildIndentStr.XMLStr() ) );
      }
      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( service );
   }
   
   // HTML
   if ( userUser->getHTMLService() ) {
      DOMElement* service = 
         reply->createElement( X( "service" ) );
 
      appendElementWithText( service, reply, "service_type", 
                             "ROUTE",
                             childIndentLevel + 1, indent  );
      appendElementWithText( service, reply, "service_method", 
                             "HTML",
                             childIndentLevel + 1, indent  );

      if ( indent ) {
         service->appendChild( reply->createTextNode( 
                                  XchildIndentStr.XMLStr() ) );
      }
      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( service );
   }

   // NAV
   if ( userUser->getNavService() ) {
      DOMElement* service = 
         reply->createElement( X( "service" ) );
 
      appendElementWithText( service, reply, "service_type", 
                             "ROUTE",
                             childIndentLevel + 1, indent  );
      appendElementWithText( service, reply, "service_method", 
                             "NAV",
                             childIndentLevel + 1, indent  );

      if ( indent ) {
         service->appendChild( reply->createTextNode( 
                                  XchildIndentStr.XMLStr() ) );
      }
      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( service );
   }

   // XML
   if ( userUser->getExternalXmlService() ) {
      DOMElement* service = 
         reply->createElement( X( "service" ) );
 
      appendElementWithText( service, reply, "service_type", 
                             "ROUTE",
                             childIndentLevel + 1, indent  );
      appendElementWithText( service, reply, "service_method", 
                             "XML",
                             childIndentLevel + 1, indent  );
      if ( indent ) {
         service->appendChild( reply->createTextNode( 
                                  XchildIndentStr.XMLStr() ) );
      }
      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( service );
   }

   // OPERATOR
   if ( userUser->getOperatorService() ) {
      DOMElement* service = 
         reply->createElement( X( "service" ) );
 
      appendElementWithText( service, reply, "service_type", 
                             "ROUTE",
                             childIndentLevel + 1, indent  );
      appendElementWithText( service, reply, "service_method", 
                             "OPERATOR",
                             childIndentLevel + 1, indent  );
      if ( indent ) {
         service->appendChild( reply->createTextNode( 
                                  XchildIndentStr.XMLStr() ) );
      }
      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( service );
   }

   
   // phone*
   for ( uint32 i = 0 ; 
         i < userUser->getNbrOfType( UserConstants::TYPE_CELLULAR ) ;
         i++ )
   {
      UserCellular* cellular = static_cast< UserCellular* >( 
         userUser->getElementOfType( 
            i, UserConstants::TYPE_CELLULAR ) );
      
      // phone
      DOMElement* phone = 
         reply->createElement( X( "phone" ) );
      
      // phone_number
      appendElementWithText( phone, reply, "phone_number", 
                             cellular->getPhoneNumber(),
                             childIndentLevel + 1, indent  );
      
      // phone_manufacturer
      appendElementWithText( phone, reply, "phone_manufacturer", 
                             cellular->getModel()->getManufacturer(),
                             childIndentLevel + 1, indent  );

      // phone_model
      appendElementWithText( phone, reply, "phone_model", 
                             cellular->getModel()->getName(),
                             childIndentLevel + 1, indent  );
      
      if ( indent ) {
         phone->appendChild( reply->createTextNode( 
                                XchildIndentStr.XMLStr() ) );
      }
      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( phone );
   }

   // user_licence_key*
   for ( uint32 i = 0 ;
         i < userUser->getNbrOfType( UserConstants::TYPE_LICENCE_KEY ) ;
         i++ )
   {
      UserLicenceKey* licence = static_cast< UserLicenceKey* >( 
         userUser->getElementOfType( i, UserConstants::TYPE_LICENCE_KEY ) );

      // user_licence_key
      DOMElement* licence_key = 
         reply->createElement( X( "user_licence_key" ) );

      // id
      licence_key->setAttribute( X( "id" ), XInt32( licence->getID() ) );
      // key
      licence_key->setAttribute( X( "key" ), 
                                 X( licence->getLicenceKeyStr() ) );
      // key_type
      licence_key->setAttribute( X( "key_type" ), X( licence->getKeyType() ) );
      // product
      licence_key->setAttribute( X( "product" ), X( licence->getProduct() ) );

      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( licence_key );
   }


   // binary_key*  This is keept to make old webhacks work (airtel_admin.php)
   //              and old websites too.
   for ( uint32 i = 0 ;
         i < userUser->getNbrOfType( UserConstants::TYPE_LICENCE_KEY ) ;
         i++ )
   {
      UserLicenceKey* licence = static_cast< UserLicenceKey* >( 
         userUser->getElementOfType( 
            i, UserConstants::TYPE_LICENCE_KEY ) );

      // binary_key
      DOMElement* binary_key = reply->createElement( X( "binary_key" ) );

      // id
      binary_key->setAttribute( X( "id" ), XInt32( licence->getID() ) );

      // key_data
      MC2String licenceKey;
      if ( licence->getKeyType() != "imei" ) {
         licenceKey.append( licence->getKeyType() );
         licenceKey.append( ":" );
      }
      licenceKey.append( licence->getLicenceKeyStr() );
      MC2String base64Key = StringUtility::base64Encode( licenceKey );
      appendElementWithText( binary_key, reply, "key_data", 
                             base64Key.c_str(),
                             childIndentLevel + 1, indent );

      if ( indent ) {
         binary_key->appendChild( reply->createTextNode( 
                                     XchildIndentStr.XMLStr() ) );
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( binary_key );
   }


   // region_access*
   for ( uint32 i = 0 ;
         i < userUser->getNbrOfType( 
            UserConstants::TYPE_REGION_ACCESS ) ; i++ )
   {
      UserRegionAccess* access = static_cast< UserRegionAccess* >( 
         userUser->getElementOfType( 
            i, UserConstants::TYPE_REGION_ACCESS ) );

      // region_access
      DOMElement* region_access = reply->createElement( 
         X( "region_access" ) );

      // id
      region_access->setAttribute( X( "id" ), XInt32( access->getID() ) );

      // top_region_id
      region_access->setAttribute( X( "top_region_id" ), 
                                   XInt32( access->getRegionID() ) );

      // start_time
      region_access->setAttribute( X( "start_time" ), 
                                   XInt32( access->getStartTime() ) );

      // end_time
      region_access->setAttribute( X( "end_time" ), 
                                   XInt32( access->getEndTime() ) );

      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( region_access );
   }

   // wayfinder_subscription?
   if ( userUser->getNbrOfType( 
           UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) > 0 )
   {
      UserWayfinderSubscription* subscr = 
         static_cast< UserWayfinderSubscription* >( 
            userUser->getElementOfType( 
               0, UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) );

      // wayfinder_subscription
      DOMElement* wayfinder_subscription = reply->createElement( 
         X( "wayfinder_subscription" ) );

      // id
      wayfinder_subscription->setAttribute( X( "id" ), 
                                            XInt32( subscr->getID() ) );

      // type
      wayfinder_subscription->setAttribute( 
         X( "type" ), XInt32( subscr->getWayfinderType() ) );

      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( wayfinder_subscription );
   }

   // right*
   typedef pair<uint32,uint64> alreadyShown_t;
   set<alreadyShown_t> alreadyShown;
   for ( uint32 i = 0 ;
         i < userUser->getNbrOfType( UserConstants::TYPE_RIGHT ) ; i++ )
   {
      UserRight* right = static_cast< UserRight* >( 
         userUser->getElementOfType( i, UserConstants::TYPE_RIGHT ) );

      // Compact form for active rights only.
      if ( !showAll ) {
         if ( right->isDeleted() || !right->checkAccessAt( now ) ) {
            continue;
         }
         // To compare to already shown items
         alreadyShown_t key =
            alreadyShown_t(right->getRegionID(),
                           right->getUserRightType().getAsInt() );
         if ( alreadyShown.find( key ) != alreadyShown.end() ) {
            // Has already shown a similar thing.
            continue;
         }
         // Do not show next time
         alreadyShown.insert( key );
      }

      
      // right
      DOMElement* rightEl = reply->createElement( X( "right" ) );

      // id
      rightEl->setAttribute( X( "id" ), XInt32( right->getID() ) );

      // add_time
      rightEl->setAttribute( X( "add_time" ), 
                             XUint32( right->getAddTime() ) );

      // type
      rightEl->setAttribute( X( "type" ), XUint64( 
                                right->getUserRightType().getAsInt() ) );
      

      // top_region_id
      rightEl->setAttribute( X( "top_region_id" ), 
                             XInt32( right->getRegionID() ) );

      // start_time
      rightEl->setAttribute( X( "start_time" ), 
                             XUint32( right->getStartTime() ) );

      // end_time
      rightEl->setAttribute( X( "end_time" ), 
                             XUint32( right->getEndTime() ) );

      // deleted
      rightEl->setAttribute( X( "deleted" ), 
                             X( StringUtility::booleanAsString( 
                                   right->isDeleted() ) ) );

      // origin
      rightEl->setAttribute( X( "origin" ), X( right->getOrigin() ) );

      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( rightEl );
   }

   // token*
   for ( uint32 i = 0 ;
         i < userUser->getNbrOfType( UserConstants::TYPE_TOKEN ) ; i++ )
   {
      UserToken* t = static_cast< UserToken* >( 
         userUser->getElementOfType( i, UserConstants::TYPE_TOKEN ) );

      // token
      mc2dbg4 << "Token: ID " << t->getID() << " CreateTime " 
              << t->getCreateTime() << " Age " << int(t->getAge())
              << " Token " << t->getToken() << endl;

      // token
      DOMElement* tokenEl = reply->createElement( X( "token" ) );

      // id
      tokenEl->setAttribute( X( "id" ), XInt32( t->getID() ) );

      // Token
      tokenEl->setAttribute( X( "token" ), X( t->getToken() ) );

      // Group
      tokenEl->setAttribute( X( "group" ), X( t->getGroup() ) );

      // create_time
      tokenEl->setAttribute( X( "create_time" ), 
                             XUint32( t->getCreateTime() ) );

      // age
      tokenEl->setAttribute( X( "age" ), XInt32(t->getAge()  ) );

      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( tokenEl );
   }

   // pin*
   for ( uint32 i = 0 ;
         i < userUser->getNbrOfType( UserConstants::TYPE_PIN ) ; i++ )
   {
      UserPIN* t = static_cast< UserPIN* >( 
         userUser->getElementOfType( i, UserConstants::TYPE_PIN ) );

      // PIN
      mc2dbg4 << "PIN: ID " << t->getID() << " PIN " << t->getPIN() 
              << " Comment " << t->getComment()  << endl;
      if ( indent ) {
         user->appendChild( reply->createTextNode( XchildIndentStr.XMLStr() ) );
      }

      user->appendChild( ::createPINElement( *reply, *t, true, true ) );

   }

   // id_key*
   for ( uint32 i = 0 ;
         i < userUser->getNbrOfType( UserConstants::TYPE_ID_KEY ) ; i++ )
   {
      UserIDKey* t = static_cast< UserIDKey* >( 
         userUser->getElementOfType( i, UserConstants::TYPE_ID_KEY ) );

      // id_key
      mc2dbg4 << "id_key: ID " << t->getID() << " key " << t->getIDKey() 
              << " type " << idKeyTypeToString( t->getIDType() ) << endl;

      // idKey
      DOMElement* idKeyEl = reply->createElement( X( "id_key" ) );

      // id
      idKeyEl->setAttribute( X( "id" ), XInt32( t->getID() ) );

      // key
      idKeyEl->setAttribute( X( "key" ), X( t->getIDKey() ) );

      // type
      idKeyEl->setAttribute( X( "type" ), 
                             X( idKeyTypeToString( t->getIDType() ) ) );

      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( idKeyEl );
   }

   // last_client*
   for ( uint32 i = 0 ;
         i < userUser->getNbrOfType( UserConstants::TYPE_LAST_CLIENT ) ; 
         i++ )
   {
      UserLastClient* t = static_cast< UserLastClient* >( 
         userUser->getElementOfType( i, UserConstants::TYPE_LAST_CLIENT ));
      
      if ( !showAll ) {
         if ( t->isHistory() ) {
            // No need to show old stuff.
            continue;
         }
      }
      
      // last_client
      mc2dbg4 << "last_client: ID " << t->getID() << " client_type " 
              << t->getClientType() << " client_type_options " 
              << t->getClientTypeOptions() << " version " 
              << t->getVersion() << " extra " << t->getExtra()
              << " origin " << t->getOrigin()
              << " history " << BP(t->isHistory()) << " changer_uin "
              << t->getChangerUIN() << " change_time " 
              << t->getChangeTime() << endl;


      // lastClient
      DOMElement* lastClientEl = reply->createElement( 
         X( "last_client" ) );

      // id
      lastClientEl->setAttribute( X( "id" ), XInt32( t->getID() ) );
      // client_type
      lastClientEl->setAttribute( X( "client_type" ), 
                                  X( t->getClientType() ) );
      // client_type_options
      lastClientEl->setAttribute( X( "client_type_options" ), 
                                  X( t->getClientTypeOptions() ) );
      // version
      lastClientEl->setAttribute( X( "version" ), X( t->getVersion() ) );
      // extra
      lastClientEl->setAttribute( X( "extra" ), X( t->getExtra() ) );
      // origin 
      lastClientEl->setAttribute( X( "origin" ), X( t->getOrigin() ) );
      // history
      lastClientEl->setAttribute( 
         X( "history" ), 
         X( StringUtility::booleanAsString( t->isHistory() ) ) );
      if ( t->isHistory() ) {
         // changer_uin
         lastClientEl->setAttribute( X( "changer_uin" ), 
                                     XUint32( t->getChangerUIN() ) );
         // change_time
         lastClientEl->setAttribute( X( "change_time" ), 
                                     XUint32( t->getChangeTime() ) );
      }

      if ( indent ) {
         user->appendChild( reply->createTextNode( 
                               XchildIndentStr.XMLStr() ) );
      }
      user->appendChild( lastClientEl );
   }


   if ( indent ) {
      user->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   if ( indent ) {
      cur->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   cur->appendChild( user );
   
}


bool 
XMLParserThread::readUser( DOMNode* userNode, UserUser* user, 
                           MC2String& newPassword, MC2String& oldPassword,
                           bool& haveNewPassword, bool& haveOldPassword,
                           MC2String& errorCode, MC2String& errorMessage ) const
{
   bool ok = true;


   bool hasSearchObjects = false;
   uint32 searchObject = 0;
   bool hasSearchLocationObjects = false;
   uint32 searchLocationObject = 0;

   // Go throu attributes
   DOMNamedNodeMap* attributes = userNode->getAttributes();
   DOMNode* attribute;
   bool hasUIN = false;

   using namespace XMLServerUtility;
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(), "birth_date" ) ) {
         user->setBirthDate( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_cost" ) ) 
      {
         RouteTypes::routeCostType routeCost = 
            RouteTypes::stringToRouteCostType( tmpStr );
         byte costA = 0;
         byte costB = 0;
         byte costC = 0;
         byte costD = 0;
                  
         RouteTypes::routeCostTypeToCost( 
            routeCost, costA, costB, costC, costD);
         user->setRouting_costA( costA );
         user->setRouting_costB( costB );
         user->setRouting_costC( costC );
         user->setRouting_costD( costD );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_vehicle" ) ) 
      {
         user->setRouting_vehicle( 
            ItemTypes::getVehicleFromString( tmpStr ) );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "search_match_type" ) ) 
      {
         user->setSearch_type( 
            StringConversion::searchStringMatchingTypeFromString( 
               tmpStr ) );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "search_word_match_type" ) ) 
      {
         user->setSearch_substring(
            StringConversion::searchStringPartTypeFromString( tmpStr ) );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "search_sort_type" ) ) 
      {
         user->setSearch_sorting( 
            StringConversion::searchSortingTypeFromString( tmpStr ) );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "valid_date" ) ) 
      {
         if ( m_user->getUser()->getEditUserRights() ) {
            // May set valid_date
            char* endPtr = NULL;
            uint32 validTime = strtoul( tmpStr, &endPtr, 10 );
            if ( endPtr == NULL || *endPtr != '\0' ) {
               ok = false;
               errorCode = "-1";
               errorMessage = "valid_date not a valid number.";
               mc2log << warn << "XMLParserThread::readUser"
                         "valid_date not a valid number." << endl;
            } else {
               user->setValidDate( validTime );
            }
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "edit_user_right" ) ) 
      {
         if ( m_user->getUser()->getEditUserRights() ) {
            // May set EditUserRights
            user->setEditUserRights( StringUtility::checkBoolean( 
                                        tmpStr ) );
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "address1" ) ) 
      {
         user->setAddress1( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "address2" ) ) 
      {
         user->setAddress2( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "address3" ) ) 
      {
         user->setAddress3( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "address4" ) ) 
      {
         user->setAddress4( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "address5" ) ) 
      {
         user->setAddress5( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_turn_image" ) ) 
      {
         user->setRouteTurnImageType( 
            routeTurnImageTypeFromString( tmpStr ) );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "overview_image_type" ) )
      {
         UserConstants::RouteImageType imageType = 
            overviewImageTypeFromString( tmpStr );
         if ( imageType != UserConstants::ROUTEIMAGETYPE_NBR ) {
            user->setRouteImageType( imageType );
         } else {
            ok = false;
            errorCode = "-1";
            errorMessage = "Unknown overview image type.";
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "transactionBased" ) ) 
      {
         UserConstants::transactionBased_t transType = 
            transactionBasedTypeFromString( tmpStr );
         if ( transType != UserConstants::NBR_TRANSACTION_T ) {
            // May set
            if ( m_user->getUser()->getEditUserRights() ) {
               user->setTransactionBased( transType );
            }
         } else {
            ok = false;
            errorCode = "-1";
            errorMessage = "Unknown transactionBased type.";
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "deviceChanges" ) ) 
      {
         char* tmpPtr = NULL;
         int32 d = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            errorCode = "-1";
            errorMessage = "deviceChanges is not a number.";
            ok = false;
         } else {
            // May set
            if ( m_user->getUser()->getEditUserRights() ) {
               user->setDeviceChanges( d );
            }
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "supportComment" ) ) 
      {
         // May set
         if ( m_user->getUser()->getEditUserRights() ) {
            user->setSupportComment( tmpStr );
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "postalCity" ) ) 
      {
         // May set
         if ( m_user->getUser()->getEditUserRights() ) {
            user->setPostalCity( tmpStr );
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "zipCode" ) ) 
      {
         // May set
         if ( m_user->getUser()->getEditUserRights() ) {
            user->setZipCode( tmpStr );
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "companyName" ) ) 
      {
         // May set
         if ( m_user->getUser()->getEditUserRights() ) {
            user->setCompanyName( tmpStr );
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "companyReference" ) ) 
      {
         // May set
         if ( m_user->getUser()->getEditUserRights() ) {
            user->setCompanyReference( tmpStr );
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "companyVATNbr" ) ) 
      {
         // May set
         if ( m_user->getUser()->getEditUserRights() ) {
            user->setCompanyVATNbr( tmpStr );
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "emailBounces" ) ) 
      {
         char* tmpPtr = NULL;
         int32 d = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            errorCode = "-1";
            errorMessage = "emailBounces is not a number.";
            ok = false;
         } else {
            // May set
            if ( m_user->getUser()->getEditUserRights() ) {
               user->setEmailBounces( d );
            }
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "addressBounces" ) ) 
      {
         char* tmpPtr = NULL;
         int32 d = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            errorCode = "-1";
            errorMessage = "addressBounces is not a number.";
            ok = false;
         } else {
            // May set
            if ( m_user->getUser()->getEditUserRights() ) {
               user->setAddressBounces( d );
            }
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "customerContactInfo" ) ) 
      {
         // May set
         if ( m_user->getUser()->getEditUserRights() ) {
            user->setCustomerContactInfo( tmpStr );
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "brand_origin" ) )
      {
         // May set
         if ( m_user->getUser()->getEditUserRights() ) {
            user->setBrandOrigin( tmpStr );
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "brand" ) )
      {
         // May set
         if ( m_user->getUser()->getEditUserRights() ) {
            user->setBrand( tmpStr );
         }
      } else if ( XMLString::equals( attribute->getNodeName(), "uin" ) ) {
         // Then can safely change logonID
         char* tmpPtr = NULL;
         strtoul( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
            // Hmm, what about that
            errorCode = "-1";
            errorMessage = "uin is not a number.";
            ok = false;
         } else {
            hasUIN = true;
         }
      } else {
         mc2log << warn << "XMLParserThread::readUser "
                   "unknown attribute for user element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }


   // Go throu users childrens and take apropriate actions
   DOMNode* child = userNode->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), "user_id" ) ) {
               char* tmpStr = NULL;
               tmpStr = XMLUtility::getChildTextValue( child );
               if ( user->getUIN() == 0 ||          // New user
                    user->getUIN() == MAX_UINT32 || // Search user
                    hasUIN ) // Has uin so user_id not used to identify
               { 
                  user->setLogonID( tmpStr );
               }
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "first_name" ) ) 
            {
               char* tmpStr = NULL;
               tmpStr = XMLUtility::getChildTextValue( child );
               user->setFirstname( tmpStr );
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "last_name" ) ) 
            {
               char* tmpStr = NULL;
               tmpStr = XMLUtility::getChildTextValue( child );
               user->setLastname( tmpStr );
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "initials" ) ) 
            {
               char* tmpStr = NULL;
               tmpStr = XMLUtility::getChildTextValue( child );
               user->setInitials( tmpStr );
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "language" ) ) 
            {
               char* tmpStr = NULL;
               tmpStr = XMLUtility::getChildTextValue( child );
               user->setLanguage( getLanguageCode( tmpStr ) );
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "measurement_system" ) )
            {
               char* tmpStr = NULL;
               tmpStr = XMLUtility::getChildTextValue( child );
               if ( StringUtility::strcasecmp( "imperial",
                                               tmpStr ) == 0 )
               {
                  user->setMeasurementSystem( 
                     UserConstants::MEASUREMENTTYPE_IMPERIAL ); 
               } else {
                  user->setMeasurementSystem( 
                     UserConstants::MEASUREMENTTYPE_METRIC ); 
               }
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "service" ) ) 
            {
               // Service type
               char* typeStr = NULL;
               typeStr = XMLUtility::getChildTextValue( child );
               // Service Method
               char* methodStr = NULL;
               if ( child->getFirstChild()->getNextSibling()
                    ->getFirstChild() != NULL )
               {
                  methodStr = XMLUtility::transcodefromucs( 
                     child->getFirstChild()->getNextSibling()
                     ->getFirstChild()->getNodeValue() );
               } else {
                  methodStr = StringUtility::newStrDup( "" ); 
               }
               bool deleteService = false;
               if ( XMLString::equals( 
                       child->getLastChild()->getNodeName(),
                       "service_delete" ) )
               {
                  deleteService = true;   
               }
                        
               if ( m_user->getUser()->getEditUserRights() ) {
                  if ( StringUtility::strcasecmp( methodStr, "WAP" ) == 0 )
                  {
                     user->setWAPService( !deleteService );
                  } else if ( StringUtility::strcasecmp( 
                                 methodStr, "SMS" ) == 0 )
                  {
                     user->setSMSService( !deleteService );
                  } else if ( StringUtility::strcasecmp( 
                                 methodStr, "HTML" ) == 0 )
                  {
                     user->setHTMLService( !deleteService );
                  } else if ( StringUtility::strcasecmp( 
                                 methodStr, "NAV" ) == 0 )
                  {
                     user->setNavService( !deleteService );
                  } else if ( StringUtility::strcasecmp( 
                                 methodStr, "XML" ) == 0 )
                  {
                     user->setExternalXmlService( !deleteService );
                  } else if ( StringUtility::strcasecmp( 
                                 methodStr, "OPERATOR" ) == 0 )
                  {
                     user->setOperatorService( !deleteService );
                  } else {
                     mc2log << warn << "XMLParserThread::readUser "
                            << "unknown service method #" << methodStr 
                            << "#" << endl;
                     ok = false;
                     errorCode = "-1";
                     errorMessage = "Unknown service method.";
                  }
               }
               delete [] typeStr;
               delete [] methodStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "phone" ) ) 
            {
               ok = xmlParseUserPhone( child, user );
               if ( !ok ) {
                  errorCode = "-1";
                  errorMessage = "Invalid phone number";
               }
            } else if ( XMLString::equals( child->getNodeName(),
                                           "binary_key" ) ) 
            {
               ok = xmlParseUserLicence( child, user, 
                                         errorCode,
                                         errorMessage );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "user_licence_key" ) ) 
            {
               ok = xmlParseUserLicenceKey( child, user, 
                                            errorCode,
                                            errorMessage );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "region_access" ) ) 
            {
               ok = xmlParseUserRegionAccess( child, user, 
                                              errorCode,
                                              errorMessage );
            } else if ( XMLString::equals( child->getNodeName(), 
                                           "right" ) )
            {
               ok = xmlParseUserRight( child, user, errorCode,
                                       errorMessage );
            } else if ( XMLString::equals( child->getNodeName(), 
                                           "token" ) )
            {
               ok = xmlParseUserToken( child, user, errorCode,
                                       errorMessage );
            } else if ( XMLString::equals( child->getNodeName(), 
                                           "pin" ) )
            {
               ok = xmlParseUserPIN( child, user, errorCode,
                                     errorMessage );
            } else if ( XMLString::equals( child->getNodeName(), 
                                           "id_key" ) )
            {
               ok = xmlParseUserIDKey( child, user, errorCode,
                                       errorMessage );
            } else if ( XMLString::equals( child->getNodeName(), 
                                           "last_client" ) )
            {
               // Known, nothing supported for last_client
            } else if ( XMLString::equals( child->getNodeName(),
                                           "email_address" ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               if ( StringUtility::validEmailAddress( tmpStr ) ||
                    tmpStr[ 0 ] == '\0' )
               {
                  user->setEmailAddress( tmpStr );
               } else {
                  ok = false;
                  errorCode = "-1";
                  errorMessage = "Invalid email address.";
               }
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "operator_comment" ) )
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               // May set
               if ( m_user->getUser()->getEditUserRights() ) {
                  user->setOperatorComment( tmpStr );
               }
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "wayfinder_subscription" ) )
            {
               ok = xmlParseUserWayfinderSubscription( child, user, 
                                                       errorCode,
                                                       errorMessage );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_municipal" ) )
            {
               hasSearchLocationObjects = true;
               searchLocationObject |= SEARCH_MUNICIPALS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_city" ) ) 
            {
               hasSearchLocationObjects = true;
               searchLocationObject |= SEARCH_BUILT_UP_AREAS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_citypart" ) )
            {
               hasSearchLocationObjects = true;
               searchLocationObject |= SEARCH_CITY_PARTS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_zipcode" ) )
            {
               hasSearchLocationObjects = true;
               searchLocationObject |= SEARCH_ZIP_CODES;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_ziparea" ) )
            {
               hasSearchLocationObjects = true;
               searchLocationObject |= SEARCH_ZIP_AREAS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_street" ) )
            {
               hasSearchObjects = true;
               searchObject |= SEARCH_STREETS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_company" ) )
            {
               hasSearchObjects = true;
               searchObject |= SEARCH_COMPANIES;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_category" ) )
            {
               hasSearchObjects = true;
               searchObject |= SEARCH_CATEGORIES;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_misc" ) )
            {
               hasSearchObjects = true;
               searchObject |= SEARCH_MISC;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "new_password" ) )
            {
               haveNewPassword = true;
               char* tmpStr = XMLUtility::getChildTextValue( child );
               newPassword = tmpStr;
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "old_password" ) ) 
            {
               haveOldPassword = true;
               char* tmpStr = XMLUtility::getChildTextValue( child );
               oldPassword = tmpStr;
               delete [] tmpStr;
            } else { // Odd element in user element
               mc2log << warn << "XMLParserThread::readUser"
                      << " odd Element in user element: "
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
            mc2log << "XMLParserThread::readUser"
                      "odd node type in user element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

         
   // Check search object
   if ( hasSearchObjects ) {
      user->setSearchForTypes( searchObject );
   }
   if ( hasSearchLocationObjects) {
      user->setSearchForLocationTypes( searchLocationObject );
   }

   return ok;
}

#endif // USE_XML

