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
#include "MC2CRC32.h"

#include "UserFavorites.h"
#include "UserFavoritesRequest.h"
#include "UserData.h"
#include "XMLAuthData.h"
#include "DataBuffer.h"
#include "ParserUserHandler.h"
#include "XMLSearchUtility.h"
#include "XMLServerElements.h"
#include "InfoTypeConverter.h"
#include "WFSubscriptionConstants.h"
#include "ClientSettings.h"

/**
 * @return true if user is a vicinity user and unregistred 
 */
bool isUnregistredVicinityUser( const UserUser* user,
                                const ClientSetting* setting ) {
   // not a vicinity user?
   if ( setting->getCreateLevel() != WFSubscriptionConstants::WOLFRAM ) {
      return false;
   }

   mc2dbg << "[UserFavoritesRequest] email: " << user->getEmailAddress() << endl;
   // have valid email?
   if ( user->getEmailAddress() == 0 ||
        ( user->getEmailAddress() != 0 &&
          strlen( user->getEmailAddress() ) == 0 ) ) {
      mc2dbg << "[USerFavoritesRequest] not valid email!" << endl;
      // vicinity but not valid email
      return true;
   }

   // is vicinity but email is valid
   return false;
}

bool 
XMLParserThread::xmlParseUserFavoritesRequest( DOMNode* cur, 
                                               DOMNode* out,
                                               DOMDocument* reply,
                                               bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   // UserFavoriteRequest data
   UserItem* userItem = NULL;
   bool syncFavorites = true;
   vector< uint32 > favoriteIDList;
   vector< uint32 > deleteFavoriteIDList;
   vector< UserFavorite* > addFavoriteList;
   MC2String userID;
   MC2String userSessionID;
   MC2String userSessionKey;
   uint32 uin = 0;
   XMLCommonEntities::coordinateType positionSystem =
      XMLCommonEntities::MC2;
   bool fav_info_in_desc = true;
   MC2String errorCode;
   MC2String errorMessage;

   // Create user_favorites_reply element
   DOMElement* user_favorites_reply = XMLUtility::
      createStandardReply( *reply, *cur, "user_favorites_reply" );

   out->appendChild( user_favorites_reply );

   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), 
         user_favorites_reply );
   }


   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      MC2String tmpStr = 
         XMLUtility::transcodefrom( attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "fetch_auto_dest" ) ) 
      {
         // Not supported anymore 20051011
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "sync_favorites" ) ) 
      {
         syncFavorites = StringUtility::checkBoolean( tmpStr.c_str() );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "position_system" ) ) 
      {
         positionSystem = 
            XMLCommonEntities::coordinateFormatFromString( tmpStr.c_str() );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "fav_info_in_desc" ) ) 
      {
         fav_info_in_desc = StringUtility::checkBoolean( tmpStr.c_str() );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "transaction_id" ) ) 
      {
         // Handled above 
      } else {
         mc2log << warn << "XMLParserThread::xmlParseUserFavoritesRequest "
                   "unknown attribute for user_favorites_request "
                    "element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
   }


   // Get children

   
   for ( DOMNode* child = cur->getFirstChild(); 
         child != NULL && ok; 
         child = child->getNextSibling() ) {

      if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         continue;
      }

      // See if the element is a known type
      if ( XMLString::equals( child->getNodeName(), "user_id" ) ) {
         userID = XMLUtility::getChildTextStr( *child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "user_session_id" ) ) {
         userSessionID = XMLUtility::getChildTextStr( *child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "user_session_key" ) ) {
         userSessionKey = XMLUtility::getChildTextStr( *child );
      } else if ( XMLString::equals( child->getNodeName(), "uin" ) ){
         MC2String tmpStr = XMLUtility::getChildTextStr( *child );
         char* endPtr = NULL;
         uin = strtoul( tmpStr.c_str(), &endPtr, 10 );
         if ( endPtr == NULL || *endPtr != '\0' || uin == 0 ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Problem parsing uin not a valid number.";
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseUserFavoritesRequest uin not "
                   << "a valid number. " << MC2CITE( tmpStr )
                   << endl;
         }
      } else if ( XMLString::equals( child->getNodeName(),
                                     "favorite_id_list" ) ) {
         readFavoriteIDList( child, favoriteIDList );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "delete_favorite_id_list" ) ) {
         readFavoriteIDList( child, deleteFavoriteIDList );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "add_favorite_list" ) ) {
         ok = readFavoriteList( child, addFavoriteList, errorCode,
                                errorMessage );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "auto_dest_favorite" ) ) {
         // Not supported anymore 20051011
      } else {
         mc2log << warn << "XMLParserThread::"
            "xmlParseUserFavoritesRequest "
            "odd Element in user_favorites_request element: " 
                << child->getNodeName() << endl;
      }

   }

   if ( ok && m_authData->backupUser ) {
      ok = false;
      errorCode = "-212";
      errorMessage = "Not on backup server.";
      mc2log << warn << "XMLParserThread::"
             << "xmlParseUserFavoritesRequest backup user may not sync "
             << "favorites" << endl;
   }
   
   if ( !ok ) {
      // Nothing to do
   } else if ( !userID.empty() ) {
      // Get user by id
      ok = getUser( userID.c_str(), userItem, true );
   } else if ( !userSessionID.empty() ) {
      // Get user by session
      ok = getUserBySession( userSessionID.c_str(), 
                             userSessionKey.c_str(), userItem, true );
   } else if ( uin != 0 ) {
      ok = getUser( uin, userItem, true );
   } else {
      // Get auth user
      ok = getUser( m_user->getUIN(), userItem, true );
   }

   using XMLServerUtility::appendStatusNodes;

   if ( ok ) {
      if ( userItem != NULL ) {
         UserUser* user = userItem->getUser();

         bool badVicinityUser = 
            isUnregistredVicinityUser( user,
                                       m_group->
                                       getSetting( m_authData->clientType,
                                                   "" ) );
         // Check access
         if ( ( m_user->getUser()->getEditUserRights() ||
                m_user->getUIN() == user->getUIN() ) &&
              ! badVicinityUser ) {
            
            UserFavoritesRequest* req = new UserFavoritesRequest( 
               getNextRequestID(), user->getUIN() );
         
            if ( !syncFavorites ) {
               req->setNoSync();
            }
         
            // addFavSync
            for ( uint32 i = 0 ; i < favoriteIDList.size() ; i++ ) {
               req->addFavSync( favoriteIDList[ i ] );
            }
            // addFavDelete
            for ( uint32 i = 0 ; i < deleteFavoriteIDList.size() ; i++ ) {
               req->addFavDelete( deleteFavoriteIDList[ i ] );
            }
            // addFavNew
            for ( uint32 i = 0 ; i < addFavoriteList.size() ; i++ ) {
               // Make sure adding using id 0
               addFavoriteList[ i ]->setID( 0 );
               req->addFavNew( addFavoriteList[ i ] );
            }
         
            ThreadRequestContainer* reqCont = 
               new ThreadRequestContainer( req );
            mc2dbg8 << "About to send UserFavoritesRequest" << endl;
            putRequest( reqCont );
            mc2dbg8 << "UserFavoritesRequest returned" << endl;

            if ( StringTable::stringCode( req->getStatus() ) == 
                 StringTable::OK ) {
               const UserFavorite* favorite = NULL;
               vector< uint32 > deleteFavoriteIDList;
               vector< const UserFavorite* > addFavoriteList;

               // AddFav
               favorite = req->getAddFav();
               while ( favorite != NULL ) {
                  addFavoriteList.push_back( favorite );
                  favorite = req->getAddFav();
               }

               // DelFav
               favorite = req->getDelFav();
               while ( favorite != NULL ) {
                  deleteFavoriteIDList.push_back( favorite->getID() );
                  favorite = req->getDelFav();
               }

               // delete_favorite_id_list
               if ( deleteFavoriteIDList.size() > 0 ) {
                  appendFavoriteIDList( user_favorites_reply, reply,
                                        indentLevel + 1, indent,
                                        "delete_favorite_id_list",
                                        deleteFavoriteIDList );
               }
                
               if ( addFavoriteList.size() > 0 ) {
                  // add_favorite_list
                  appendFavoriteList( user_favorites_reply, reply,
                                      indentLevel + 1, indent,
                                      "add_favorite_list",
                                      positionSystem, fav_info_in_desc,
                                      addFavoriteList );
               }

               mc2log << info << "UserFavorites OK nbrAdd "
                      << addFavoriteList.size() << " nbrDel "
                      << deleteFavoriteIDList.size() << endl;

               // set crc attribute
               user_favorites_reply->
                  setAttribute( X( "crc" ),
                                XHex32( getUserHandler()->getUserFavCRC(
                                           user->getUIN() ) ) );
            } else {
               // Error
               appendStatusNodes( 
                  user_favorites_reply, reply, 
                  indentLevel + 1, indent,
                  "-1", StringTable::getString( StringTable::stringCode( 
                     req->getStatus() ), StringTable::ENGLISH ) );
               mc2log << info << "UserFavorites Error: "
                      << StringTable::getString( 
                         StringTable::stringCode( 
                            req->getStatus() ), StringTable::ENGLISH )
                      << endl;
            }

            
            delete reqCont;
            delete req;
         } else {
            if ( badVicinityUser ) {
               appendStatusNodes( user_favorites_reply, reply,
                                  indentLevel + 1, indent,
                                  "-1",
                                  StringTable::
                                  getString( StringTable::
                                             VICINITY_FAVORITES_LOCKED,
                                             m_user->getUser()->getLanguage() ) );
            } else {

               appendStatusNodes( user_favorites_reply, reply, 
                                  indentLevel + 1, indent,
                                  "-1",
                                  "Access denied." );
            }
            mc2log << info << "UserFavorites Error: Access denied to user."
                   << endl;
         }
      } else {
         // No such user
         appendStatusNodes( user_favorites_reply, reply, 
                            indentLevel + 1, indent,
                            "-1", "Unknown user." );
         mc2log << info << "UserFavorites Error: Unknown user."
                << endl;
      }
   } else {
      if ( errorCode.empty() ) {
         // Database communication error
         appendStatusNodes( 
            user_favorites_reply, reply, indentLevel + 1, indent,
            "-1", "Database connection error, please try again." );
         mc2log << warn << "XMLParserThread::xmlParseUserFavoritesRequest "
                << "Database connection error, falied to get user data."
                << endl;
      } else {
         appendStatusNodes( 
            user_favorites_reply, reply, indentLevel + 1, indent,
            errorCode.c_str(), errorMessage.c_str() );
      }
      ok = true; // Error handled
   }

   
   if ( indent ) {
      // Newline and indent before end user_favorites_reply tag   
      user_favorites_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }
   
   // Delete all UserFavorites in lists
   // addFavoriteList favorites sent to UserFavoritesRequest
   addFavoriteList.clear();

   releaseUserItem( userItem );

   return ok;
}


void 
XMLParserThread::readFavoriteIDList( DOMNode* cur, 
                                     vector< uint32 >& favIDs ) 
{
   // Get children
   
   
   for ( DOMNode* child = cur->getFirstChild();
         child != NULL; 
         child = child->getNextSibling() ) {
      
      if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         continue;
      }

      // See if the element is a known type
      if ( XMLString::equals( child->getNodeName(),
                              "favorite_id" ) ) {
         MC2String tmpStr = XMLUtility::getChildTextStr( *child );
         char* tmp = NULL;
         uint32 favID = strtoul( tmpStr.c_str(), &tmp, 10 );
         if ( tmp != tmpStr ) {
            favIDs.push_back( favID );
         } else {
            mc2log << warn << "XMLParserThread::"
               "readFavoriteIDList "
               "bad favorite id " 
                   << tmpStr << endl;
         }
      } else {
         mc2log << warn << "XMLParserThread::"
            "readFavoriteIDList "
            "odd Element in favorite_id_list element: " 
                << child->getNodeName() << endl;
      }

   }

}


void 
XMLParserThread::appendFavoriteIDList( DOMNode* cur, DOMDocument* reply,
                                       int indentLevel, bool indent,
                                       const char* favoriteIDListName,
                                       const vector< uint32 >& favIDs )
{
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   int childIndentLevel = indentLevel + 1;
   
   DOMElement* favoriteIDList = reply->createElement( 
      X( favoriteIDListName ) );
   char tmp[20];
   
   for ( uint32 i = 0 ; i < favIDs.size() ; i++ ) {
      sprintf( tmp, "%u", favIDs[ i ] );
      XMLServerUtility::
         appendElementWithText( favoriteIDList, reply,
                             "favorite_id", tmp,
                             childIndentLevel, indent );
   }
   
   // Append favoriteIDList
   if ( indent ) {
      cur->appendChild( reply->createTextNode( X( indentStr.c_str() ) ) );
   }
   cur->appendChild( favoriteIDList );
   if ( indent && favIDs.size() > 0 ) {
      favoriteIDList->appendChild( reply->createTextNode( 
                                      X( indentStr.c_str() ) ) );
   }
}


bool
XMLParserThread::readFavInfo( DOMNode* cur, InfoVect& v, 
                              MC2String& errorCode, 
                              MC2String& errorMessage )
{
   bool ok = true;

   MC2String key;
   MC2String val;
   ItemInfoEnums::InfoType type = ItemInfoEnums::dont_show;

    // Get children
   DOMNode* child = cur->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            mc2log << warn << "XMLParserThread::readFavInfo "
                   << "odd Element in fav_info element: " 
                   << child->getNodeName() << endl;
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::readFavInfo "
                   << "odd node type in fav_info element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      child = child->getNextSibling();
   }

   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "type" ) ) 
      {
         type = getInfoTypeConverter().strToInfoType( tmpStr );
         if ( type == ItemInfoEnums::more ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Unknown poi_info_t type in fav_info.";
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "key" ) ) 
      {
         key = tmpStr;
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "value" ) ) 
      {
         val = tmpStr;
      } else {
         mc2log << warn << "XMLParserThread::readFavInfo "
                << "unknown attribute for fav_info element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   
   if ( ok ) {
      v.push_back( ItemInfoEntry( key, val, type ) );
   }

   return ok;
}


bool
XMLParserThread::readFavoriteList( DOMNode* cur, 
                                   vector< UserFavorite* >& favorites,
                                   MC2String& errorCode,
                                   MC2String& errorMessage )
{
   // Get children
   DOMNode* child = cur->getFirstChild();
   bool ok = true;

   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), "favorite" ) ) {
               uint32 favID = 0;
               char* name = NULL;
               char* shortName = NULL;
               char* description = NULL;
               char* category = NULL;
               char* mapIconName = NULL;
               int32 lat = MAX_INT32;
               bool latSet = false;
               int32 lon = MAX_INT32;
               bool lonSet = false;
               uint16 angle = MAX_UINT16;
               UserFavorite::InfoVect v;

               // Attributes
               DOMNamedNodeMap* attributes = child->getAttributes();
               DOMNode* attribute;
   
               for ( uint32 i = 0 ; i < attributes->getLength() && ok ; 
                     i++ ) 
               {
                  attribute = attributes->item( i );
      
                  char* tmpStr = XMLUtility::transcodefromucs( 
                     attribute->getNodeValue() );
                  char* tmp = NULL;
                  if ( XMLString::equals( attribute->getNodeName(), 
                                          "id" ) ) 
                  {
                     uint32 tmpFavID = strtoul( tmpStr, &tmp, 10 );
                     if ( tmp != tmpStr ) {
                        favID = tmpFavID;
                     } else {
                        mc2log << warn << "XMLParserThread::"
                                  "readFavoriteList "
                                  "bad favorite id in favorite " 
                               << tmpStr << endl;
                        ok = false;
                        errorCode = "-1";
                        errorMessage = "Bad favorite id in favorite.";
                     }
                     delete [] tmpStr;
                  } else if ( XMLString::equals( attribute->getNodeName(),
                                                 "name" ) ) 
                  {
                     name = tmpStr;
                  } else if ( XMLString::equals( attribute->getNodeName(),
                                                 "short_name" ) )
                  {
                     shortName = tmpStr;
                  } else if ( XMLString::equals( attribute->getNodeName(),
                                                 "description" ) )
                  {
                     description = tmpStr;
                  } else if ( XMLString::equals( attribute->getNodeName(),
                                                 "category" ) )
                  {
                     category = tmpStr;
                  } else if ( XMLString::equals( attribute->getNodeName(),
                                                 "map_icon_name" ) )
                  {
                     mapIconName = tmpStr;
                  } else {
                     mc2log << warn << "XMLParserThread::readFavoriteList "
                               "unknown attribute for favorite "
                               "element "
                            << " Name " << attribute->getNodeName()
                            << " Type " << attribute->getNodeType() 
                            << endl;
                     delete [] tmpStr;
                  }
               }

               // Children
               DOMNode* favChild = child->getFirstChild();

               while ( favChild != NULL && ok ) {
                  switch ( favChild->getNodeType() ) {
                     case DOMNode::ELEMENT_NODE :
                        // See if the element is a known type
                        if ( XMLString::equals( favChild->getNodeName(),
                                                "position_item" ) )
                        {
                           if ( !XMLCommonElements::getPositionItemData( 
                                   favChild, lat, lon, angle,
                                   errorCode, errorMessage ) )
                           {
                              mc2log << warn << "XMLParserThread::"
                                       "readFavoriteList "
                                       "couldn't parse position_item in "
                                       "favorite " << endl;
                              ok = false;
                              // errorCode and errorMessage set
                           } else {
                              latSet = true;
                              lonSet = true;
                           }
                        } else if ( XMLString::equals( 
                                       favChild->getNodeName(),
                                       "fav_info" ) )
                        {
                           ok = readFavInfo( favChild, v, errorCode,
                                             errorMessage );
                        } else {
                           mc2log << warn << "XMLParserThread::"
                              "readFavoriteList "
                              "odd Element in favorite element: " 
                                  << favChild->getNodeName() << endl;
                        }
                        break;
                     case DOMNode::COMMENT_NODE :
                        // Ignore comments
                        break;
                     default:
                        mc2log << warn << "XMLParserThread::"
                           "readFavoriteList odd "
                           "node type in favorite element: " 
                               << favChild->getNodeName() 
                               << " type " << favChild->getNodeType() 
                               << endl;
                        break;
                  }
                  favChild = favChild->getNextSibling();
               }

               if ( name != NULL && shortName != NULL && 
                    description != NULL && category != NULL &&
                    mapIconName != NULL && latSet && lonSet )
               {
                  favorites.push_back( new UserFavorite(
                     favID, lat, lon, name, shortName, description,
                     category, mapIconName, v ) );
               }

               delete [] name;
               delete [] shortName;
               delete [] description;
               delete [] category;
               delete [] mapIconName;
            } else {
               mc2log << warn << "XMLParserThread::"
                         "readFavoriteList "
                         "odd Element in favorite_list element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                      "readFavoriteList odd "
                      "node type in favorite_list element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      child = child->getNextSibling();
   }

   return ok;
}


void 
XMLParserThread::appendFavoriteList( 
   DOMNode* cur, DOMDocument* reply,
   int indentLevel, bool indent,
   const char* favoriteListName, 
   XMLCommonEntities::coordinateType positionSystem,
   bool fav_info_in_desc,
   const vector< const UserFavorite* >& favorites )
{ 
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   int childIndentLevel = indentLevel + 1;
   MC2String childIndentStr( childIndentLevel*3, ' ' );
   childIndentStr.insert( 0, "\n" );
   
   DOMElement* favoriteList = reply->createElement( 
      X( favoriteListName ) );
   for ( uint32 i = 0 ; i < favorites.size() ; i++ ) {
      const UserFavorite* favorite = favorites[ i ];
      
      DOMElement* favoriteElement = makeUserFavoriteElement( 
         favorite, positionSystem, fav_info_in_desc, reply, 
         childIndentLevel, indent );

      // Append favorite
      if ( indent ) {
         favoriteList->appendChild( reply->createTextNode( 
                                       X( childIndentStr.c_str() ) ) );
      }
      favoriteList->appendChild( favoriteElement );
   }
   
   // Append favoriteList
   if ( indent ) {
      cur->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   cur->appendChild( favoriteList );
   if ( indent && favorites.size() > 0 ) {
      favoriteList->appendChild( reply->createTextNode( 
                                   XindentStr.XMLStr() ) );
   }  

}

DOMElement* 
XMLParserThread::
makeUserFavoriteElement( const UserFavorite* favorite,
                         XMLCommonEntities::coordinateType positionSystem,
                         bool fav_info_in_desc,
                         DOMDocument* reply,
                         int indentLevel, bool indent ) {
   char tmp[20];
   int favChildIndentLevel = indentLevel + 1;
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   DOMElement* favoriteElement = reply->createElement( X( "favorite" ) );

   sprintf( tmp, "%u", favorite->getID() );
   
   favoriteElement->setAttribute( X( "id" ), X( tmp ) );
   favoriteElement->setAttribute( X( "name" ), 
                                  X( favorite->getName() ) );
   favoriteElement->setAttribute( X( "short_name" ), 
                                  X( favorite->getShortName() ) );
   MC2String desc( favorite->getDescription() );
   if ( fav_info_in_desc ) {
      for ( uint16 i = 0 ; i < favorite->getInfos().size() ; ++i ) {
         if ( favorite->getInfos()[ i ].getInfoType() == 
              ItemInfoEnums::dont_show ) {
            continue;
         }
         MC2String addStr;
         if ( !desc.empty() ) {
            addStr.append( ", " );
         }
         addStr.append( favorite->getInfos()[ i ].getKey() );
         addStr.append( ": " );
         addStr.append( favorite->getInfos()[ i ].getVal() );

         if ( desc.size() + addStr.size() < 256 ) {
            desc.append( addStr );
         }
      }
   }
   favoriteElement->setAttribute( X( "description" ), X( desc ) );
   favoriteElement->setAttribute( X( "category" ), 
                                  X( favorite->getCategory() ) );
   favoriteElement->setAttribute( X( "map_icon_name" ), 
                                  X( favorite->getMapIconName() ) );
   XMLSearchUtility::
      appendPositionItem( favorite->getLat(), favorite->getLon(), MAX_UINT16,
                          positionSystem, favoriteElement, reply,
                          favChildIndentLevel, indent );
   const UserFavorite::InfoVect& v = favorite->getInfos();
   if ( !fav_info_in_desc ) {
      for ( UserFavorite::InfoVect::const_iterator it = v.begin() ; 
            it != v.end() ; ++it )
      {
         using namespace XMLServerUtility;
         attrVect attr;
         attr.push_back( stringStringStruct( "key", (*it).getKey() ) );
         attr.push_back( stringStringStruct( "value", (*it).getVal() ) );
         attr.push_back( stringStringStruct( "type", getInfoTypeConverter().
                                             infoTypeToStr( (*it).getInfoType() ) ) );
         appendElementWithText( favoriteElement, reply, "fav_info", NULL,
                                favChildIndentLevel, indent, &attr );
      }
   }
   if ( indent ) {
      favoriteElement->appendChild( reply->createTextNode( 
                                       X( indentStr.c_str() ) ) );
   }
   return favoriteElement;
}

bool 
XMLParserThread::xmlParseUserFavoritesCRCRequest( DOMNode* cur, 
                                                  DOMNode* out,
                                                  DOMDocument* reply,
                                                  bool indent )  {
   DOMElement* root = XMLUtility::createStandardReply( *reply, *cur,
                                                       "user_favorites_crc_reply" );

   out->appendChild( root );

   const XMLCh* requestCRC = 
      cur->getAttributes()->getNamedItem( X( "crc" ) )->getNodeValue();

   uint32 userCRC = getUserHandler()->getUserFavCRC( m_user->getUIN() );

   bool crcMatch = XMLString::equals( XHex32( userCRC ), requestCRC );

   if ( ! crcMatch ) {
      mc2dbg4 << "crc: " << XHex32( userCRC ) << " req: " << requestCRC << endl;
   }

   root->setAttribute( X( "crc_match" ),
                       crcMatch ? X( "true" ) : X( "false" ) );

   if ( indent ) {
      XMLUtility::indentPiece( *root, 1 );
   }

   return true;
}


