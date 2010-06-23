/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLSMSCommon.h"
#include "XMLCommonElements.h"
#include "LangUtility.h"
#include "UserData.h"
#include "ParserThread.h"
#include "FixedSizeString.h"
#include "XMLSearchUtility.h"
#include "ParserThreadGroup.h"
#include "XMLServerElements.h"
#include "GfxUtility.h"
#include "STLStringUtility.h"
#include "ItemInfoRequest.h"
#include "Properties.h"

namespace XMLSMSCommon {

StringTable::languageCode getLanguageCode( const char* langStr ) {
   return LangUtility::getLanguageCode( langStr );
}

bool 
xmlParseWayfinderSMS( DOMNode* cur,
                      char*& signature,
                      int32& originLat, int32& originLon,
                      MC2String& originDescription,
                      int32& destLat, int32& destLon,
                      MC2String& destDescription,
                      MC2String& errorCode, 
                      MC2String& errorMessage ) {

   if ( ! ( XMLString::equals( cur->getNodeName(), "wayfinder_destination_sms" ) ||
            XMLString::equals( cur->getNodeName(), "wayfinder_route_sms" ) ) ) {
      errorCode = "-1";
      errorMessage = "wayfinder_destination_sms element is not named "
         "wayfinder_destination_sms.";
      return false;
   }
   
   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      MC2String tmpStr = XMLUtility::transcodefrom( attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "orig_description" ) ) {
         originDescription = tmpStr;
      } else if( XMLString::equals( attribute->getNodeName(),
                                    "dest_description" ) ) {
         destDescription = tmpStr;
      } else if( XMLString::equals( attribute->getNodeName(),
                                    "description" ) ) {
         destDescription = tmpStr;
      } else {
         mc2log << warn << "XMLParserThread::"
            "xmlParseWayfinderSMS "
            "unknown attribute for wayfinder_destination_sms "
            "element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
   }

   // Check children
      

   bool readDest = true;
   bool ok = true;
   for ( DOMNode* child = cur->getFirstChild(); 
         child != NULL && ok;
         child = child->getNextSibling() ) {
      if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         continue;
      }

      // See if the element is a known type
      if( XMLString::equals( child->getNodeName(),
                             "position_item" ) ) {
         uint16 angle = 0;
         if( !readDest ) {
            if ( ! XMLCommonElements::
                 getPositionItemData( child, originLat, originLon, angle,
                                      errorCode, errorMessage ) ) {
               ok = false;
               // errorCode, errorMessage set i n getPositionItemData
               errorMessage.insert( 0, "Problem with dest_position_item "
                                    "in wayfinder_route_sms: " );
            } 
         } else {
            if ( ! XMLCommonElements::getPositionItemData( child, 
                                                           destLat, destLon,
                                                           angle,
                                                           errorCode,
                                                           errorMessage ) ) {
               ok = false;
               // errorCode, errorMessage set i n getPositionItemData
               errorMessage.insert( 0, "Problem with dest_position_item "
                                    "in wayfinder_route_sms: " );
            } else {
               readDest = false;
            }
         }
      } else if ( XMLString::equals( child->getNodeName(),
                                     "signature" ) ) {
         signature = XMLUtility::getChildTextValue( child );
      } else {
         mc2log << warn << "XMLParserThread::"
            "xmlParseWayfinderSMS "
            "odd node type in " << cur->getNodeName()
                << " element: "
                << child->getNodeName() 
                << " type " << child->getNodeType() << endl;
      }
   }
    

   return ok;
}

bool 
xmlParseSendSMSRequestRouteMessageData( DOMNode* cur,
                                        uint32& routeID, 
                                        uint32& routeCreateTime,
                                        StringTable::languageCode& language,
                                        char*& signature,
                                        char*& originString, 
                                        char*& originLocationString,
                                        char*& destinationString, 
                                        char*& destinationLocationString ) {
   
   if ( ! XMLString::equals( cur->getNodeName(), "route_message_data" ) ) {
      return false;
   }

   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
         
      MC2String tmpStr = XMLUtility::transcodefrom( attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(), "route_id" ) ) {
         if ( sscanf( tmpStr.c_str(), "%X_%X",
                      &routeID, &routeCreateTime ) == 2 ) {
            // Ok
         } else {
            routeID = MAX_UINT32;
            routeCreateTime = MAX_UINT32;
            mc2log << warn << "XMLParserThread::"
               "xmlParseSendSMSRequestRouteMessageData "
               "Illegal routeID found " << tmpStr << endl;
         }
      } else {
         mc2log << warn << "XMLParserThread::"
            "xmlParseSendSMSRequestRouteMessageData "
            "unknown attribute for route_message_data element "
                << "Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType()
                << " Value " << tmpStr << endl;
      }
   }

   // Children
   for ( DOMNode* child = cur->getFirstChild();
         child != NULL;
         child = child->getNextSibling() ) {

      if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         continue;
      }

      // See if the element is a known type
      if ( XMLString::equals( child->getNodeName(),
                              "language" ) ) {
         char* tmpStr = XMLUtility::getChildTextValue( child );
         language = getLanguageCode( tmpStr );
         delete [] tmpStr;
      } else if ( XMLString::equals( child->getNodeName(),
                                     "signature" ) ) {
         signature = XMLUtility::getChildTextValue( child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "originString" ) ) {
         originString = XMLUtility::getChildTextValue( child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "originLocationString" ) ) {
         originLocationString = XMLUtility::getChildTextValue( 
                                                              child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "destinationString" ) ) {
         destinationString = XMLUtility::getChildTextValue( child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "destinationLocationString" ) ) {
         destinationLocationString = 
            XMLUtility::getChildTextValue( child );
      } else { // Odd element in route_data element
         mc2log << warn << "XMLParserThread::"
            "xmlParseSendSMSRequestRouteSMSMessage "
            "odd Element in route_message_data"
            " element: "
                << child->getNodeName() << endl;
      }
   }

   return true;
}

bool
xmlParseWayfinderFavouriteSMS( DOMNode* cur,
                               char*& signature,
                               int32& lat, int32& lon,
                               MC2String& name,
                               MC2String& shortName,
                               MC2String& description,
                               MC2String& category,
                               MC2String& mapIconName,
                               MC2String& errorCode,
                               MC2String& errorMessage ) {

   if ( ! XMLString::equals( cur->getNodeName(), "wayfinder_favourite_sms" ) ) {
      errorCode = "-1";
      errorMessage = "wayfinder_favourite_sms element is not named "
         "wayfinder_favourite_sms.";
      return false;
   }

   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      MC2String tmpStr = XMLUtility::transcodefrom( attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "description" ) ) {
         description = tmpStr;
      } else {
         mc2log << warn << "XMLParserThread::"
            "xmlParseWayfinderFavouriteSMS "
            "unknown attribute for wayfinder_favourite_sms "
            "element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
   }

   bool ok = true;

   // Check children
   for ( DOMNode* child = cur->getFirstChild();
         child != NULL && ok;
         child = child->getNextSibling() ) {

      if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         continue;
      }

      // See if the element is a known type
      if ( XMLString::equals( child->getNodeName(),
                              "position_item" ) ) {
         uint16 angle = 0;
         if ( ! XMLCommonElements::getPositionItemData( child,
                                                        lat, lon, angle,
                                                        errorCode,
                                                        errorMessage ) ) {
            ok = false;
            // errorCode, errorMessage set i n getPositionItemData
            errorMessage.insert( 0, "Problem with position_item "
                                 "in wayfinder_favourite_sms: " );
         }
      } else if ( XMLString::equals( child->getNodeName(),
                                     "name" ) ) {
         name = XMLUtility::getChildTextStr( *child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "short_name" ) ) {
         shortName = XMLUtility::getChildTextStr( *child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "category_name" ) ) {
         category = XMLUtility::getChildTextStr( *child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "map_icon_name" ) ) {
         mapIconName = XMLUtility::getChildTextStr( *child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "signature" ) ) {
         signature = XMLUtility::getChildTextValue( child );
      } else { // Odd element in
         mc2log << warn << "XMLParserThread::"
            "xmlParseWayfinderFavouriteSMS "
            "odd Element in wayfinder_favourite_sms"
            " element: "
                << child->getNodeName() << endl;
      }

   }


   return ok;
}

bool
appendSMSList( DOMNode* cur, DOMDocument* reply,
               StringVector* smsVector,
               int indentLevel, bool indent ) {
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   bool ok = true;

   DOMElement* sms_list = reply->createElement( X( "sms_list" ) );

   for ( uint32 i = 0 ; i < smsVector->getSize() ; i++ ) {
      XMLServerUtility::
         appendElementWithText( sms_list, reply, "smsmessage",
                                smsVector->getElementAt( i ),
                                indentLevel + 1, indent );
   }

   // Add sms_list to cur
   if ( indent ) {
      cur->appendChild( reply->createTextNode( X( indentStr.c_str() ) ) );
   }
   if ( smsVector->getSize() > 0 && indent ) {
      // Newline and indent before end sms_list tag
      sms_list->appendChild( reply->createTextNode(
                                                   X( indentStr.c_str() ) ) );
   }
   cur->appendChild( sms_list );

   return ok;
}

void composeVicinityPlaceSMS( DOMNode* cur, DOMDocument* reply,
                              int indentLevel, bool indent,
                              const UserItem* user,
                              const PlaceSMSData& data ) {
   StringVector strings;
   const char* placeFormat = StringTable::
      getString( StringTable::VICINITY_PLACE_SMS, user->getUser()->getLanguage() );

   FixedSizeString placeString( 1024 );

   // TODO: fetch street name for coordinate
   MC2String latStr, lonStr;
   GfxUtility::printLatLon( latStr, lonStr,
                            data.m_coord,
                            user->getUser()->getLanguage() );

   MC2String coordStr = "(" + latStr + ", " + lonStr + ")";
   // compose url
   MC2String url = "http://localhost.localdomain/?id=";
   url += STLStringUtility::uint2str( user->getUser()->getUIN() );
   url += "&moredataetc";

   sprintf( placeString, placeFormat,
            user->getUser()->getLogonID(),
            "Street name", coordStr.c_str(),
            url.c_str() );

   strings.addLast( StringUtility::newStrDup( placeString.c_str() ) );
   appendSMSList( cur, reply, &strings, indentLevel, indent );
}

MC2String getWFGigDownloadURL( ) {
   return Properties::getProperty( "WFGIG_DOWNLOAD_URL",
                                   "http://localhost.localdomain" );
}

void composeInviteSMS( DOMNode* cur, DOMDocument* reply,
                       int indentLevel, bool indent,
                       const UserItem* user,
                       const InviteData& data ) {
   
   StringTable::languageCode lang = user->getUser()->getLanguage();
   // vicinity type is the only type supported right now
   // so ignore data.m_type

   if ( data.m_type == "vicinity" ) {
      const char* formatStr =
         StringTable::getString( StringTable::VICINITY_INVITE_SMS, lang );
      // compose url
      char url[ 256 ];
      snprintf( url, 256, "http://localhost.localdomain?vicinity.php?id=%d", user->getUser()->getUIN() );

      FixedSizeString inviteString( 1024 );
      sprintf( inviteString, formatStr,
               data.m_name.c_str(),
               url,
               user->getUser()->getLogonID() );

      StringVector strings;
      strings.addLast( StringUtility::newStrDup( inviteString.c_str() ) );
      appendSMSList( cur, reply, &strings, indentLevel, indent );
   } else if ( data.m_type == "eventfinder" ) {
      StringVector strings;
      MC2String url = getWFGigDownloadURL();
      char *str = new char[ 256 ];
      const char *formatStr = 
         StringTable::getString( StringTable::DISC_GIGS_SHARE_GIGS_SMS, lang );
      snprintf( str, 256, formatStr, url.c_str() );
      strings.addLast( str );
      appendSMSList( cur, reply, &strings, indentLevel, indent );
   }
}

MC2String composeEventSMS( const ItemInfoRequest& request ) {
   const uint32 FIRST_VALID_ITEM = 0;
   MC2String address;
   MC2String performer;
   MC2String date;
   // venue is the items name
   MC2String venue = request.getReplyName( FIRST_VALID_ITEM );

   // take the first info item as the valid one.
   for ( uint32 i = 0 ;
         i < request.getReplyNbrFields( FIRST_VALID_ITEM );
         ++i ) {

      switch ( request.getReplyItemType( FIRST_VALID_ITEM, i ) ) {
      case ItemInfoEnums::start_time:
         date = request.getReplyItemValue( FIRST_VALID_ITEM, i );
         break;
      case ItemInfoEnums::vis_full_address:
         address = request.getReplyItemValue( FIRST_VALID_ITEM, i );
         break;
      case ItemInfoEnums::performer:
         performer = request.getReplyItemValue( FIRST_VALID_ITEM, i );
         break;
      default:
         break;
      }
   }
   MC2String timestr;

   MC2String::size_type pos = date.find_first_of( " ");
   if ( pos != MC2String::npos ) {
      timestr = date.substr( pos + 1 );
      date = date.substr( 0, pos );
   } else if ( date == "ongoing" ) {
      MC2String url = getWFGigDownloadURL();
      const char* formatStr = 
         StringTable::getString( StringTable::DISC_GIGS_SEND_GIG_ONGOING_SMS, 
                                 request.getLanguage() );
      char buff[256];
      snprintf( buff, 256, formatStr, performer.c_str(), 
                venue.c_str(),
                url.c_str() );
   }

   MC2String url = getWFGigDownloadURL();
   const char* formatStr = 
      StringTable::getString( StringTable::DISC_GIGS_SEND_GIG_SMS, 
                              request.getLanguage() );
   char buff[256];
   snprintf( buff, 256, formatStr, performer.c_str(), 
             venue.c_str(),
             date.c_str(), timestr.c_str(),
             url.c_str() );
   
   // now we should have all the valid fields to compose a SMS
   return buff;
}

void composeGigSMS( ParserThread& thread,
                    DOMNode* cur, DOMDocument* reply,
                    int indentLevel, bool indent,
                    const PlaceSMSData& data ) {
   // valid match?
   if ( data.m_match.get() == NULL ) {
      return;
   }
   // create poi info request
   auto_ptr<ItemInfoRequest>
      req(new ItemInfoRequest( thread.getNextRequestID(),
                               thread.getGroup()->
                               getTopRegionRequest( &thread ) ) );

   if ( ! req->setItem( *data.m_match, data.m_language )  ) {
      XMLServerUtility::
         appendStatusNodes( cur, reply, indentLevel + 1, indent,
                            "-1",
                            "Not id nor coords valid for search_item." );
      return;
   }

   thread.putRequest( req.get() );

   // Date, performer, venue, address
   // compose event message from info fields

   // add strings
   StringVector strings;
   strings.addLast( StringUtility::
                    newStrDup( composeEventSMS( *req ).c_str() ) );
   appendSMSList( cur, reply, &strings, indentLevel, indent );
}

void composePlaceSMS( ParserThread& thread,
                      DOMNode* cur, DOMDocument* reply,
                      int indentLevel, bool indent,
                      const UserItem* user,
                      const PlaceSMSData& data ) {

   if ( data.m_type == "vicinity" ) {
      composeVicinityPlaceSMS( cur, reply, indentLevel, indent, user, data );
   } else if ( data.m_type == "eventfinder" ) {
      composeGigSMS( thread, cur, reply, indentLevel, indent, data );
   }
}

}
