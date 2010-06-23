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
#include "UserRight.h"
#include "UserData.h"
#include "UserLicenceKey.h"
#include "HttpInterfaceRequest.h"
#include "HttpHeader.h"
#include "ParserTokenHandler.h"
#include "ParserExternalAuth.h"
#include "ParserExternalAuthHttpSettings.h"
#include "ParserActivationHandler.h"
#include "XMLAuthData.h"
#include "ClientSettings.h"
#include "ParserUserHandler.h"
#include "XMLTool.h"
#include "XMLServerElements.h"
#include "UserLicenceKey.h"
#include "ServerTypes.h"

#define XMLA "[XMLA:" << __LINE__ << "]"


bool
XMLParserThread::xmlParseActivateRequest( DOMNode* cur, DOMNode* out,
                                          DOMDocument* reply, 
                                          bool indent,
                                          const HttpHeader& inHeaders )
{
   bool ok = true;
   int indentLevel = 1;
   XStr XindentStr( MC2String(indentLevel*3, ' ') + MC2String("\n") );

   MC2String activationCode; //activationcode if included in request
   uint32 activationRegionID = MAX_INT32; // The selected region for ac
   MC2String phoneNumber;    //content of >phone_number> if included
   MC2String email;          //content of <email> if included in request
   MC2String name;           //content of <name> if included in request
   MC2String newPassword;    //content of <new_password> if included
   uint32 uin = 0;           //uin read from request
   MC2String errorCode;      //errorcode if one should be sent in reply.
   MC2String errorMessage;   //errorMessage if one should be sent in reply
   //if the may_use attribute is present and set to "false", the
   //activation code must have been used. If it has not been used, it
   //must not be used by the server. 
   bool may_use = true;
   //whether a new token should be created for the user. 
   bool create_new_token = true; 
   DOMNode* external_auth = NULL;   // The external_auth node if present
   bool sendStatusReply = false;
   DOMNode* server_auth_bob = NULL; //The server_auth_bob node if present
   DOMNode* handle_me = NULL;       //The handle_me node if present
   MC2String opt_in_name;

   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   for ( uint32 i = 0 ; i < attributes->getLength() && ok ; i++ ) {
      DOMNode* attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "transaction_id" ) ) 
      {
         // Handled above 
      } else if ( XMLString::equals( attribute->getNodeName(),
                              "activation_code" ) ) 
      {
         activationCode = tmpStr;
         if ( activationCode.empty() && 
              m_authData->clientSetting->getWFID() ) {
            // Some java clients sends empty ac with
            // name and email. And that must still work.
            errorCode = "-302";
            errorMessage = "Problem with activation code "
               "Empty activation code"; 
            ok = false;
         }
      } else if ( XMLString::equals( attribute->getNodeName(), "uin" ) ) {
         char* endPtr = NULL;
         uin = strtoul( tmpStr, &endPtr, 10 );
         if ( endPtr == NULL || *endPtr != '\0' || uin == 0 ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Problem parsing uin not a valid number."; 
            mc2log << warn << XMLA << " uin not "
                   << "a valid number. " << MC2CITE( tmpStr ) << endl;
         }
      } else if ( XMLString::equals( attribute->getNodeName(), 
                                     "top_region_id" ) ) {
         char* endPtr = NULL;
         activationRegionID = strtoul( tmpStr, &endPtr, 10 );
         if ( endPtr == NULL || *endPtr != '\0' ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Problem parsing top_region_id not a valid number.";
            mc2log << warn << XMLA << " top_region_id not "
                   << "a valid number. " << MC2CITE( tmpStr ) << endl;
         }
      } else if ( XMLString::equals( attribute->getNodeName(), "may_use" ))
      {
         may_use = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(), 
                                     "create_new_token" ) )
      {
         create_new_token = StringUtility::checkBoolean( tmpStr );
      } else {
         mc2log << warn << XMLA
                << "unknown attribute for activate_request element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   // Get children
   for(DOMNode* child = cur->getFirstChild(); 
       ok && child != NULL; child = child->getNextSibling() ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), "phone_number" ))
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               char* phoneNbr = StringUtility::cleanPhonenumber( 
                  tmpStr );
               phoneNumber = phoneNbr; 
               if ( !StringUtility::validPhonenumber( phoneNbr ) ) {
                  ok = false;
                  errorCode = "-304";
                  errorMessage = "Not valid phone number."; 
                  mc2log << warn << XMLA << " not valid phone "
                         << "number \"" <<  phoneNbr<< "\" from \""
                         << tmpStr << "\"" << endl;
               }
               delete [] phoneNbr;
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "new_password" ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               newPassword = tmpStr;
               delete [] tmpStr;
               if ( newPassword.size() < 6 ) {
                  ok = false;
                  errorCode = "-301";
                  errorMessage = "Too short password."; 
                  mc2log << warn << XMLA << " password too short \""
                         << newPassword << "\"" << endl;
               }
            } else if ( XMLString::equals( child->getNodeName(),
                                           "email"  ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               email = StringUtility::trimStartEnd( tmpStr );
               if ( !StringUtility::validEmailAddress( email.c_str() ) ) {
                  ok = false;
                  errorCode = "-308";
                  errorMessage = "Not valid email address."; 
                  mc2log << warn << XMLA << " not valid email '"
                         << email << "' from '" << tmpStr << "'" << endl;
               }
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "name"  ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               name = StringUtility::trimStartEnd( tmpStr );
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "opt_in"  ) ) 
            {
               opt_in_name = XMLUtility::transcodefrom( 
                  static_cast<DOMElement*>( child )->getAttribute( 
                     X( "name" ) ) );
               if ( !opt_in_name.empty() ) {
                  opt_in_name.insert( 0, "opt_in_" );
               }
            } else if ( XMLString::equals( child->getNodeName(), 
                                           "hardware_id" ) ||
                        XMLString::equals( child->getNodeName(), 
                                           "hardware_key" ) ) 
            {
               m_authData->hwKeys.push_back( UserLicenceKey( MAX_UINT32 ) );
               m_authData->hwKeys.back().setProduct( 
                  m_authData->clientSetting->getProduct() );
               MC2String type;
               XMLTool::getAttribValue( type, "type", child );
               m_authData->hwKeys.back().setKeyType( type );
               m_authData->hwKeys.back().setLicence( 
                  XMLUtility::getChildTextStr( *child ) );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "external_auth" ) ) 
            {
               external_auth = child;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "server_auth_bob" ) ) 
            {
               server_auth_bob = child;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "handle_me" ) ) 
            {
               handle_me = child;
            } else {
               mc2log << warn << XMLA 
                      << "odd Element in activate_request element: " 
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
            mc2log << warn << XMLA
                   << "odd node type in activate_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
   }

   // parsing complete

   // Get Activation data
   MC2String rights;
   MC2String tokenStr = getTokenHandler()->makeTokenStr();
   uint32 ownerUIN = 0;
   MC2String server;
   const uint32 now = TimeUtility::getRealTime();

   if ( ok && !external_auth && !server_auth_bob && !handle_me && 
        ( ! activationCode.empty() || m_authData->hwKeys.empty() ) ) {
      int res = getActivationHandler()->getActivationData( 
         activationCode.c_str(), rights, ownerUIN, server );
      if ( res == 0 ) {
         mc2dbg4 << "Got activation data " << rights << " Owner "
                 << ownerUIN
                 << endl;
      } else {
         mc2log << warn << XMLA << " Failed to get activation data "
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
   }

   if ( ok ) {
      if ( ownerUIN == 0 && !activationCode.empty() ) {
         // Check client_type level matches activation level
         if ( getClientTypeLevel() == UserEnums::UR_GOLD ) {
            if ( rights.find( "GOLD" ) == MC2String::npos &&
                 rights.find( "SILVER" ) != MC2String::npos )
            {
               ok = false;
               errorMessage = "Insufficient credit.";
               errorCode = "-209";
               mc2log << warn << XMLA
                      << "Gold client but not gold activation code, "
                      << "sending Insufficient credit."
                      << endl;
            }
         }
      } // Else check user's level not code against client level
      //  Done below after we gets the user.
   }

   //find out if there is a user account for the hardware id
   UserItem* hardwareUser = NULL;
   const UserLicenceKey* imeiKey = NULL;
   if ( ok && !m_authData->hwKeys.empty() ) {
      // Find and set best
      getUserHandler()->setBestLicenceKey( m_authData->hwKeys, 
                                           m_authData->hwKey );
      UserLicenceKey userKey( m_authData->hwKey );
      ok = getUserFromLicenceKeys( m_authData->hwKeys, hardwareUser, 
                                  "-311", errorCode, errorMessage,
                                  true /*Really uptodate*/ );
      imeiKey = m_authData->getLicenceKeyType( 
         ParserUserHandler::imeiType );
   }

   UserItem* userItem = NULL;
   if ( ok ) {
      if ( ownerUIN != 0 ) { //activation code already used
         // Check user
         if ( uin != 0 ) {
            // Update to owner user
            mc2log << info << XMLA << " Used activation code "
                   << activationCode << " changing from " << uin 
                   << " to " << ownerUIN << endl;
            uin = ownerUIN;
            create_new_token = true;
            may_use = false; // Just for safety
         } // Else nothing to check
         // Get user
         if ( ok && server.empty() ) {
            if ( !getUser( ownerUIN, userItem, 
                           true, true /*Really uptodate*/ ) || 
                 userItem == NULL ) 
            {
               ok = false;
               errorCode = "-1";
               errorMessage = "Failed to get user.";
               mc2log << XMLA << " Failed to get user ("
                      << ownerUIN << endl;
            }
         }

         if ( ok && server.empty() ) {
            if ( getClientTypeLevel() == UserEnums::UR_GOLD ) {
               // Check if user has gold (now or before)
               if ( !checkUserAccess( userItem->getUser(),
                                      "JAVA", false/*checkTime*/,
                                      UserEnums::TG_MASK ) )
               {
                  // But what if user whants to upgrade?? Enter New
                  // code, and have separate users for cities and nav!
                  // Or upgrade in cities and then use nav.
                  ok = false;
                  errorMessage = "Insufficient credit.";
                  errorCode = "-209";
                  mc2log << warn << XMLA
                         << "Gold client but not gold user (used AC), "
                         << "sending Insufficient credit."
                         << endl;
               }
            }
         }
         
         if ( ok && !server.empty() ) {
            ok = false; // Don't do any more
            if ( m_authData->server_list_crcSet ) {
               // Get server list for serverStr
               errorCode = "-211";
               errorMessage = "Redirect.";
               sendStatusReply = true;
            } else {
               mc2log << warn << XMLA << " Client doesn't "
                      << "support redirect, sending bad Activation Code."
                      << endl;
               errorCode = "-302";
               errorMessage = "Failed to activate user: ";
               errorMessage.append( "Bad activation code" );
            }
         }
      } else if ( activationCode.empty() && 
                  !external_auth && !server_auth_bob && !handle_me) {
         if ( hardwareUser != NULL ) { //user IDed by hardware id
            // Update to owner user
            mc2log << info << XMLA << " Used hardware key "
                   << m_authData->hwKey.getKeyType() << ":" 
                   << m_authData->hwKey
                   << " changing from " << uin 
                   << " to " << hardwareUser->getUIN() << endl;
            uin = hardwareUser->getUIN();
            userItem = hardwareUser;
            hardwareUser = NULL; //avoid double "delete"
            create_new_token = true;
            may_use = false; // Just for safety  
         } else if( uin != 0 ){
            // Update to owner user
            mc2log << info << XMLA << " Using user " << uin << endl;
            create_new_token = true;
            may_use = false; // Just for safety
            // Get user
            if ( !getUser( uin, userItem, true, true /*Really uptodate*/ ) || 
                 userItem == NULL ) {
               ok = false;
               errorCode = "-1";
               errorMessage = "Failed to get user.";
               mc2log << XMLA << " Failed to get user ("
                      << uin << ")" << endl;
            }
         } else if ( ! m_authData->hwKeys.empty() ) {
            const ParserExternalAuthHttpSettings* peh = 
               getExternalAuth()->getNavHttpSetting( 
                  m_authData->clientSetting->getClientType() );
            if ( peh != NULL  ) {
               // Do Http auth
               bool authorized = doHttpAuth( userItem, errorMessage, 
                                             errorCode, peh->settingId, 
                                             m_authData->clientLang,
                                             tokenStr/*currentID*/, 
                                             tokenStr/*newID*/,
                                             false/*checkTime*/ );
               if ( authorized ) {
                  mc2log << info << XMLA
                         << "ExternalAuth " << peh->settingId 
                         << " via client_type " 
                         << m_authData->clientSetting->getClientType()
                         << " User accepted. UserName " 
                         << userItem->getUser()->getLogonID() 
                         << "(" << userItem->getUIN() << ")" << endl;
               } else {
                  // Else error in errorMessage, errorCode
                  ok = false;
               }
            } else {
               //create user how?
               // Why no userKey here? (m_authData->hwKey)
               // Possibly set elsewhere... strange things... See licenceTo
               // below...
               mc2log << info << XMLA << " Creating trial user." << endl;
               const uint32 createStartTime = now - 24*60*60;
               const uint32 createEndTime = ParserUserHandler::addTime( 
                  m_authData->clientSetting->getCreateRegionTimeYear(),
                  m_authData->clientSetting->getCreateRegionTimeMonth(),
                  m_authData->clientSetting->getCreateRegionTimeDay(),
                  m_authData->clientSetting->getExplicitCreateRegionTime(),
                  now );

               ParserUserHandler::ExtraTypes extraTypes;
               ParserUserHandler::UserElementVector extraElements;
               ok = createWayfinderUserForXML( 
                  errorCode, errorMessage, newPassword, server,
                  NULL/*userKey*/, userItem, createStartTime, createEndTime, 
                  m_authData->clientSetting->getCreateRegionID(),
                  m_authData->clientLang, 
                  m_authData->clientType.c_str(),
                  NULL/*m_authData->client_type_options.c_str()*/,
                  NULL/*programV*/, activationCode/*empty*/, 
                  activationRegionID/*Not used as empty ac*/, extraTypes, 
                  m_authData->clientSetting->getCreateLevel(), 
                  m_authData->clientSetting->getCreateTransactionDays(),
                  m_authData->clientSetting->getBrand(), 
                  NULL/*fixedUserName*/, NULL/*extraComment*/, 
                  extraElements, NULL/*userNamePrefix*/, email.c_str(),
                  name.c_str(), opt_in_name, "1",
                  m_authData->clientSetting->getExtraRights(),
                  imeiKey /*The key for auto IMEI activation*/ );
               newPassword = ""; //don't set below
               if ( errorCode == "-211" ) {
                  if ( getServerList( server ) == NULL ) {
                     // No server list so can't change, fix configuration
                     mc2log << warn << XMLA
                            << " No server list " << server
                            << " returning error." << endl;
                     errorCode = "-1";
                     errorMessage = "Failed to create user ";
                  }
               }
            } // End else not external client type
         } else {
            //ERROR! no activationCode and no user!
            mc2log << warn << XMLA << " No hardware id and "
                   << "no activation code, sending -310" << endl;
            errorCode = "-310"; 
            errorMessage = "Neither activation_code nor hardware_id";
            ok = false;
         }
      } else { // Activation code present but it is not already used

         // Get user
         if ( external_auth != NULL ) {
            // Do external auth (via special software - internal joke)

            MC2String type = XMLUtility::transcodefrom( 
               static_cast<DOMElement*>( external_auth )->getAttribute( 
                  X( "type" ) ) );
         
            if ( getExternalAuth()->getHttpSetting( type ) != NULL ) {
               // Do Http auth
               bool authorized = doHttpAuth( userItem, errorMessage, 
                                             errorCode, type, 
                                             m_authData->clientLang,
                                             tokenStr/*currentID*/, 
                                             tokenStr/*newID*/,
                                             false/*checkTime*/ );
               if ( authorized ) {
                  mc2log << info << XMLA
                         << "ExternalAuth " << type << " User accepted. "
                         << "UserName " 
                         << userItem->getUser()->getLogonID() 
                         << "(" << userItem->getUIN() << ")" << endl;
               } else {
                  // Else error in errorMessage, errorCode
                  ok = false;
               }
            } else {
               mc2log << warn << XMLA
                      << "unknown external_auth type '" << type << "'" << endl;
               errorMessage = "Unknown external_auth type.";
               errorCode = "-1";
               ok = false;
            }

         } else if ( server_auth_bob != NULL ) {
            // Do something
            // Send error
            ok = false;
            errorMessage = "Not yet supported.";
            errorCode = "-1";
            mc2log << warn << XMLA
                   << "got server_auth_bob not supported."
                   << "sending error."
                   << endl;
         } else if ( hardwareUser != NULL ) {
            //we have a hardware user!
            mc2log << XMLA << " Use hardwareUser " 
                   << hardwareUser->getUIN() << endl;
            uin = hardwareUser->getUIN();
            MC2_ASSERT( userItem == NULL );
            swap( userItem, hardwareUser );
            create_new_token = true;
         } else if ( handle_me != NULL ) {
            // Check client type in client settings for ...
            bool createUser = false;
            uint32 createStartTime = now - 24*60*60;
            uint32 createEndTime = now;
            ParserUserHandler::ExtraTypes extraTypes;
            ParserUserHandler::UserElementVector extraElements;

            if ( m_authData->clientSetting->getCreateLevel() ==
                 WFSubscriptionConstants::IRON ) 
            { 
               // Earth
               // Email and Name
               createUser = true;
               uint32 startTime = now - 24*60*60;
               uint32 endTime = now;
               m_group->getCreateWFTime( m_authData->clientSetting, 
                                         endTime );
               MC2String origin( "AUTO Iron: " );
               origin.append( m_authData->clientSetting->getClientType() );
               extraElements.push_back( 
                  new UserRight(
                     0, now, UserEnums::URType( 
                        UserEnums::UR_GOLD, 
                        UserEnums::UR_MYWAYFINDER ),
                     m_authData->clientSetting->getCreateRegionID(),
                     startTime, endTime, false, origin.c_str() ) );
            } else if ( m_authData->clientSetting->getMatrixOfDoomLevel() 
                        == 1 &&
                        !m_authData->clientSetting->getNotCreateWLUser() )
            {
               // Auto silver (Code less java)
               // Create user and add silver time
               createUser = true;
               // Make the silver rights
               uint32 startTime = now - 24*60*60;
               uint32 endTime = ParserUserHandler::addTime( 
                  m_authData->clientSetting->getSilverTimeYear(),
                  m_authData->clientSetting->getSilverTimeMonth(),
                  m_authData->clientSetting->getSilverTimeDay(),
                  m_authData->clientSetting->getExplicitSilverTime(),
                  now );
               // Add new
               MC2String origin( "AUTO Silver: " );
               origin.append( m_authData->clientSetting->getClientType() );
               UserRight* newRight = NULL;
               // Gets silver wf in createRegionID
               newRight = new UserRight(
                  0, now, UserEnums::URType( UserEnums::UR_GOLD, 
                                             UserEnums::UR_MYWAYFINDER ),
                  m_authData->clientSetting->getSilverRegionID(),
                  startTime, endTime, false, origin.c_str() );
               extraElements.push_back( newRight );
            } else if ( !m_authData->clientSetting->getExtraRights().empty() ) 
            {
               // Code less java 
               createUser = true;
               // Extra rights in extra rights!
            } else {
               // We can't handle you!
               ok = false;
               errorCode = "-213";
               errorMessage = "We can't handle you.";
            }

            if ( ok && createUser ) {
               createEndTime = ParserUserHandler::addTime( 
                  m_authData->clientSetting->getCreateRegionTimeYear(),
                  m_authData->clientSetting->getCreateRegionTimeMonth(),
                  m_authData->clientSetting->getCreateRegionTimeDay(),
                  m_authData->clientSetting->getExplicitCreateRegionTime(),
                  createStartTime );

               ok = createWayfinderUserForXML( 
                  errorCode, errorMessage, newPassword, server,
                  NULL/*userKey*/, userItem, createStartTime, createEndTime, 
                  m_authData->clientSetting->getCreateRegionID(),
                  m_authData->clientLang, 
                  m_authData->clientType.c_str(),
                  NULL/*m_authData->clientTypeOptions.c_str()*/,
                  NULL/*programV*/, ""/*activationCode*/, 
                  MAX_INT32/*activationRegionID*/, extraTypes, 
                  m_authData->clientSetting->getCreateLevel(), 
                  m_authData->clientSetting->getCreateTransactionDays(),
                  m_authData->clientSetting->getBrand(), 
                  NULL/*fixedUserName*/, NULL/*extraComment*/, 
                  extraElements, NULL/*userNamePrefix*/, email.c_str(),
                  name.c_str(), opt_in_name, "1",
                  m_authData->clientSetting->getExtraRights(),
                  imeiKey /*The key for auto IMEI activation*/ );
               newPassword = ""; // Don't set this below
               if ( errorCode == "-211" ) {
                  if ( getServerList( server ) == NULL ) {
                     // No server list so can't change, fix configuration
                     mc2log << warn << XMLA
                            << " No server list " << server
                            << " returning error." << endl;
                     errorCode = "-1";
                     errorMessage = "Failed to create user ";
                  }
               }
            }

         } else if ( uin != 0 ) {
            // Get user
            if ( !getUser( uin, userItem, true, true /*Really uptodate*/ ) || 
                 userItem == NULL ) 
            {
               ok = false;
               errorCode = "-1";
               errorMessage =  "Failed to get user from uin.";
               mc2log << warn << XMLA << " Failed to get user "
                      << "from uin(" << uin << ")" << endl;
            }
         } else if ( may_use ) {
            // Make new user for the activation code
            uint32 createStartTime = now;
            uint32 createEndTime = now;//Same as createStartTime ->no TRIAL
            ParserUserHandler::ExtraTypes extraTypes;
            ParserUserHandler::UserElementVector extraElements;
            ok = createWayfinderUserForXML( 
               errorCode, errorMessage, newPassword, server,
               NULL/*userKey*/, userItem, createStartTime, 
               createEndTime, 
               m_authData->clientSetting->getCreateRegionID(),
               m_authData->clientLang, 
               m_authData->clientType.c_str(),
               NULL/*m_authData->clientTypeOptions.c_str()*/,
               NULL/*programV*/, activationCode, activationRegionID, 
               extraTypes,
               m_authData->clientSetting->getCreateLevel(), 
               m_authData->clientSetting->getCreateTransactionDays(),
               m_authData->clientSetting->getBrand(), 
               NULL/*fixedUserName*/, NULL/*extraComment*/, 
               extraElements, NULL/*userNamePrefix*/, email.c_str(),
               name.c_str(), opt_in_name, "1",
               m_authData->clientSetting->getExtraRights(),
               imeiKey /*The key for auto IMEI activation*/ );
            newPassword = ""; // Don't set this below
            if ( errorCode == "-211" ) {
               if ( getServerList( server ) == NULL ) {
                  // No server list so can't change, fix configuration
                  mc2log << warn << XMLA
                         << " No server list " << server
                         << " returning error." << endl;
                  errorCode = "-1";
                  errorMessage = "Failed to create user ";
               }
            }
         } // Else nothing to do


         mc2log << info << ServerTypes::XML << " userItem " << (void*)userItem << endl;

         // Activate
         // When we get here, userItem should be the user that the
         // activation code should be appilied to
         if ( external_auth || server_auth_bob || 
              handle_me || activationCode.empty() ) {
            // No need to activate
         } else if ( ok && may_use ) {
            MC2String userAgent;
            MC2String userInput;
            const char* USERAGENT = "User-Agent";

            if ( m_irequest->getRequestHeader()->getHeaderValue( 
                    USERAGENT ) != NULL ) 
            {
               userAgent = *m_irequest->getRequestHeader()->getHeaderValue(
                  USERAGENT );
            }
            userInput.append( "Phonenumber: " );
            userInput.append( phoneNumber );

            // Copy to modify
            uint32 ownerUIN = 0;
            auto_ptr<UserUser> cuser( new UserUser( *userItem->getUser() ) );
            int res = getActivationHandler()->activateUser( 
               cuser.get(), m_authData->clientSetting, activationCode.c_str(),
               phoneNumber.c_str(), activationRegionID,
               m_irequest->getPeer().getIP(), 
               userAgent.c_str(), userInput.c_str(), "UNKNOWN", ownerUIN,
               server, false/*allowSpecialCodes*/ );

            if ( res == -8 ) {
               // Redirect to server if client handles that check
               // if server_list_crc in auth first otherwise bad code.
               // with redirect ststus_code and the server list.

               // validate server list
               if ( getServerList( server ) == NULL ) {
                  mc2log << warn << XMLA
                         << " bad serverlist (null) for server = '" 
                         << server << "'." << endl;
                  res = -5;
               } else if ( m_authData->server_list_crcSet ) {
                  // Get server list for server
                  errorCode = "-211";
                  errorMessage = "Redirect.";
                  sendStatusReply = true;
                  ok = false; // Don't do any more
               } else {
                  // No crc then no support for redirect
                  mc2log << warn << XMLA << " Client doesn't "
                         << "support redirect, sending bad code." << endl;
                  res = -5;
               }
               
               
            }

            if ( res != 0 && !sendStatusReply ) {
               ok = false;
               errorCode = "-1";
               errorMessage = "Failed to activate user ";
               mc2log << XMLA << " Failed to activate "
                      << "user ";
               if ( res == -2 ) {
                  errorCode = "-3";
                  mc2log << "Timeout";
                  errorMessage.append( "Timeout" );
               } else if ( res == -3 ) {
                  errorCode = "-304";
                  mc2log << "Wrong phone number";
                  errorMessage.append( "Wrong phone number" );
               } else if ( res == -4 ) {
                  errorCode = "-303";
                  mc2log << "Used activation code. By (" << ownerUIN
                         << ")";
                  errorMessage.append( "Used activation code" );
               } else if ( res == -5 ) {
                  errorCode = "-302";
                  mc2log << "Bad activation code";
                  errorMessage.append( "Bad activation code" );
               } else if ( res == -6 ) {
                  errorCode = "-305";
                  mc2log << "Extension not allowed";
                  errorMessage.append( "Extension not allowed" );
               } else if ( res == -7 ) {
                  errorCode = "-306";
                  mc2log << "Creation not allowed";
                  errorMessage.append( "Creation not allowed" );
               } else if ( res == -10 ) {
                  errorCode = "-302";
                  mc2log << "Must choose region, sending Bad "
                         << "activation code";
                  errorMessage.append( "Bad activation code" );
               } // Else just error
               mc2log << endl;
            } // Else activated ok or has set error already
            
         } else if ( ok ) {
            if ( !may_use ) {
               ok = false;
               errorCode = "-307";
               errorMessage = "May not use activation code";
               mc2log << XMLA << " May not use activation "
                      << "code AC: \"" << activationCode << "\"" << endl;
            }
         }
      } // End else not used activation code

      // Set password
      if ( ok ) {
         if ( !newPassword.empty() ) {
            int ret = ParserThread::changeUserPassword( 
               userItem->getUser(), newPassword.c_str(), "", false,
               NULL/*changer*/ );
            if ( ret != 0 ) {
               errorCode = "-1";
               errorMessage = "Failed to set password.";
               ok = false;
               mc2log << warn << XMLA << " Failed to set password "
                      << "for AC: \"" << activationCode << "\"" << endl;
               if ( ret == -2 ) {
                  errorCode = "-3";
               }
            } // Else ret == 0 and it is ok
         } // Else don't set it
      }

      // Create new token (remove old) update age
      if ( ok && create_new_token ) {
         // Copy to modify
         UserUser* cuser = new UserUser( *userItem->getUser() );
         int res = getTokenHandler()->setToken( cuser, tokenStr, 
                                                m_authData->clientSetting );

         if ( res != 0 ) {
            errorCode = "-1";
            errorMessage = "Failed to set token.";
            ok = false;
            mc2log << warn << XMLA << " Failed to set token "
                   << "for AC: \"" << activationCode << "\"" << endl;
            if ( res == -2 ) {
               errorCode = "-3";
            }
         } // Else ret == 0 and it is ok
         delete cuser;
      } // End if ok and to create new token

      // Check if licence key
      if ( ok && !external_auth ) {
         if ( ! m_authData->hwKeys.empty() ) {
            //either add hwid to this user or switch with the current
            //owner of this hwid (hardwareUser)
            // This could have been added when creating user above...
            // Less confusion it whould be then.
            int res = getUserHandler()->licenceTo( userItem->getUser(), 
                                                   &m_authData->hwKey );
            // Now userItem does not have new hw key
            if ( ! ( ok = (res == 0) ) ) {
               mc2log << warn << XMLA
                      << "Licence move failed with error " << res << endl;
               switch(res){
               case -1:// if error, 
                  errorCode = "-1";
                  errorMessage = "Unable to move licence key.";
                  break;
               case -2:// if timeout and
                  errorCode = "-3";
                  errorMessage = "Timeout while moving licence key.";
                  break;
               case -3: // if may not
                  errorCode = "-309"; 
                  errorMessage = "Moving of licence key prohibited.";
                  break;
               }
            }
         }
         else if ( getUserHandler()->getNbrProductKeys( 
                      userItem->getUser(), 
                      m_authData->clientSetting->getProduct() ) == 1 )
         {
            // If the user has exactly one licence key and no hwid
            // listed in the request, remove his hwid.

            // Yes exactly one. Users with many are made by us
            // and should have them all

            // Remove it
            UserUser cuser( *userItem->getUser() );

            UserLicenceKeyPVect keys( 
               getUserHandler()->getProductKeys(
                  userItem->getUser(), m_authData->clientSetting->getProduct() ) );
            keys[ 0 ]->remove();

            uint32 startTime = TimeUtility::getCurrentTime();
            if ( !changeUser( &cuser, NULL/*changer*/ ) ) {
               ok = false;
               errorMessage = "Failed to update user.";
               errorCode = "-1";
               mc2log << warn << XMLA << " Failed to remove licence"
                      << " key from user.";
               uint32 endTime = TimeUtility::getCurrentTime();
               if ( (endTime-startTime) > 2000 ) {
                  errorCode = "-3";
                  mc2log << " Timeout.";
               }
               mc2log << endl;
            } else {
               // Else changed ok
               mc2log << info << XMLA << " removed licence "
                      << "from user." << endl;
            }
         } // End if have licence key
      } // End if ok
   } // End if ok
   
   using XMLServerUtility::appendElementWithText;
   using XMLServerUtility::appendStatusNodes;

   if ( !sendStatusReply ) {

      // Create activate_reply element
      DOMElement* activate_reply = 
         reply->createElement( X( "activate_reply" ) );
      // Transaction ID
      activate_reply->setAttribute( 
         X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
            X( "transaction_id" ) )->getNodeValue() );
      out->appendChild( activate_reply );
      if ( indent ) {
         // Newline
         out->insertBefore( 
            reply->createTextNode( XindentStr.XMLStr() ), 
            activate_reply );
      }


      if ( ok ) {
         uint32 userUIN = 0;
         // user_id
         if ( userItem != NULL ) {
            userUIN = userItem->getUIN();
            appendElementWithText( activate_reply, reply, "user_id", 
                                   userItem->getUser()->getLogonID(),
                                   indentLevel + 1, indent  );
         }
         
         // auth_token
         if ( create_new_token ) {
            appendElementWithText( activate_reply, reply, "auth_token", 
                                   tokenStr.c_str(),
                                   indentLevel + 1, indent  );
         }
         
         // uin
         if ( userUIN != 0 ) {
            activate_reply->setAttribute( X( "uin" ), XUint32( userUIN ) );
         }

      } else {
         // An error in errorCode, errorMessage
         ok = true;
         appendStatusNodes( activate_reply, reply, 
                            indentLevel + 1, indent, 
                            errorCode.c_str(), errorMessage.c_str() );

         if ( userItem != NULL ) {
            // uin
            activate_reply->setAttribute( X( "uin" ), 
                                          XUint32( userItem->getUIN() ) );
         }
      }

      if ( indent ) {
         // Newline and indent before end tag   
         activate_reply->appendChild( 
            reply->createTextNode( XindentStr.XMLStr() ) );
      }
   } else {
      // Send only status reply, with data.
      // auth-bob? Don't think we ever needs to send that when activating.
      ok = true;
      appendStatusNodes( out, reply, 
                         indentLevel + 1, indent, 
                         errorCode.c_str(), errorMessage.c_str() );
      XMLServerUtility::
         appendServerList( out, reply, indentLevel + 1, indent, 
                           getServerList( server ),
                           server );
   }

   releaseUserItem( userItem );
   releaseUserItem( hardwareUser );

   return ok;
}

#endif // USE_XML

