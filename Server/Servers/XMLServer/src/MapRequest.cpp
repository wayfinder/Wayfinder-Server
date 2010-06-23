/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "XMLParserThread.h"
#include "HttpUtility.h"
#include "GfxFeatureMap.h"
#include "RouteID.h"

#ifdef USE_XML

#include "XMLServerElements.h"


bool 
XMLParserThread::xmlParseMapRequest( DOMNode* cur, 
                                     DOMNode* out,
                                     DOMDocument* reply,
                                     bool indent )
{
   bool ok = true;
   int indentLevel = 1;
   MC2BoundingBox bbox;
   uint16 width = 400;
   uint16 height = 400;
   ImageDrawConfig::imageFormat imageType = ImageDrawConfig::PNG;
   const char* routeID = NULL;
   uint32 beforeTurn = MAX_UINT32;
   uint32 afterTurn = MAX_UINT32;
   MapSettingsTypes::defaultMapSetting mapSetting = 
      MapSettingsTypes::MAP_SETTING_STD;
   bool showMap = true;
   bool showTopographMap = true;
   bool showPOI = true;
   bool showRoute = true;
   bool showScale = true;
   bool showTraffic = false;
   MC2String errorCode;
   MC2String errorMessage;

   struct MapSettingsTypes::ImageSettings imageSettings;
   GfxFeatureMap* mapSymbolMap = new GfxFeatureMap();
   
   imageSettings.reset();
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );

   // Create map_reply element
   DOMElement* map_reply = 
      reply->createElement( X( "map_reply" ) );
   // Transaction ID
   map_reply->setAttribute( X( "transaction_id" ), 
                            cur->getAttributes()->getNamedItem( 
                               X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( map_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( X( indentStr.c_str() ) ), map_reply );
   }

   // Go throu the elements and handle them.
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "map_request_header" ) )
            {
               ok = xmlParseMapRequestHeader( child,
                                              bbox,
                                              width,
                                              height,
                                              imageType,
                                              routeID,
                                              beforeTurn,
                                              afterTurn,
                                              mapSetting,
                                              showMap,
                                              showTopographMap,
                                              showPOI,
                                              showRoute,
                                              showScale,
                                              showTraffic,
                                              imageSettings,
                                              errorCode, errorMessage );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "map_symbol_list" ) ) 
            {
               ok = readMapSymbolList( child, mapSymbolMap );
               if ( !ok ) {
                  errorCode = "-1";
                  errorMessage = "map_symbol_list not ok.";
               }
            } else {
               mc2log << warn << "XMLParserThread::xmlParseMapRequest "
                      << "odd Element in map_request element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParseMapRequest "
                   << "odd node type in map_request element: "
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      child = child->getNextSibling();
   }
   

   if ( ok ) {
      int childIndentLevel = indentLevel + 1;
      MC2String childIndentStr( childIndentLevel*3, ' ' );
      childIndentStr.insert( 0, "\n" );
      char uri[ 4096 ];

      // Make routeIDnbr and routeCreateTime if routeID != NULL
      uint32 routeIDnbr = MAX_UINT32;
      uint32 routeCreateTime = MAX_UINT32;

      // FIXME: rID should be sent to the makeMapURL also.
      if ( routeID != NULL ) {
         RouteID rID( routeID );
         if ( rID.isValid() ) {
            routeIDnbr      = rID.getRouteIDNbr();
            routeCreateTime = rID.getCreationTime();
         } else {
            // This is really the same as above when the routeid
            // is not valid
            routeIDnbr = MAX_UINT32;
            routeCreateTime = MAX_UINT32;
         }
      }

      HttpUtility::makeMapURL( uri, NULL, NULL, 
                               m_inHead, 
                               routeIDnbr, routeCreateTime,
                               beforeTurn, afterTurn, MAX_UINT32,
                               width, height,
                               bbox.getMinLat(), bbox.getMinLon(),
                               bbox.getMaxLat(), bbox.getMaxLon(),
                               imageType, mapSetting, 
                               showMap, showTopographMap, 
                               showPOI, showRoute,
                               showScale, showTraffic,
                               &imageSettings, mapSymbolMap );
      
      // href element
      if ( indent ) {
         // Indent
         map_reply->appendChild( reply->createTextNode( 
                                    X( childIndentStr.c_str() ) ) );
      }
      DOMElement* href = reply->createElement( X( "href" ) );
      href->appendChild( reply->createCDATASection( X( uri ) ) );
      map_reply->appendChild( href );
      mc2log << info << "MapRequest: OK " << uri << endl;
   } else {
      mc2log << warn << "XMLParserThread::xmlParseMapRequest "
             << "failed to understand input. \"" << errorCode.c_str() 
             << "\", \"" << errorMessage.c_str() << " \"" << endl;
      ok = true;
      XMLServerUtility::
         appendStatusNodes( map_reply, reply, indentLevel + 1, indent,
                            errorCode.c_str(), errorMessage.c_str() );
   }

   if ( indent ) {
      // Newline and indent before end map_reply tag
      map_reply->appendChild( 
         reply->createTextNode( X( indentStr.c_str() ) ) );
   }
   
   delete [] routeID;
   delete mapSymbolMap;

   return ok;
}


bool 
XMLParserThread::xmlParseMapRequestHeader( 
   DOMNode* cur, 
   MC2BoundingBox& bbox,
   uint16& width,
   uint16& height,
   ImageDrawConfig::imageFormat& imageType,
   const char*& routeIDstr,
   uint32& beforeTurn,
   uint32& afterTurn,
   MapSettingsTypes::defaultMapSetting& displayType,
   bool& showMap,
   bool& showTopographMap,
   bool& showPOI,
   bool& showRoute,
   bool& showScale,
   bool& showTraffic,
   struct MapSettingsTypes::ImageSettings& imageSettings,
   MC2String& errorCode, MC2String& errorMessage )
{
   mc2dbg4 << "XMLParserThread::xmlParseMapRequestHeader" << endl;
   bool ok = true;

   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      char* endPtr = NULL;
      if ( XMLString::equals( attribute->getNodeName(),
                              "image_width" ) ) 
      {
         width = strtol( tmpStr, &endPtr, 10 );
         if ( endPtr == tmpStr ) {
            mc2log << warn << "XMLParserThread::xmlParseMapRequestHeader "
                   << "image_width not a number using default width." 
                   << endl;
            width = 400; 
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "image_height" ) ) 
      {
         height = strtol( tmpStr, &endPtr, 10 );
         if ( endPtr == tmpStr ) {
            mc2log << warn << "XMLParserThread::xmlParseMapRequestHeader "
                   << "image_height not a number using default height." 
                   << endl;
            height = 400; 
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "image_default_format" ) )
      {
         imageType = ImageDrawConfig::imageFormatFromString( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "image_display_type" ) )
      {
         displayType = MapSettings::defaultMapSettingFromString( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(), 
                                     "showMap" ) )
      {
         showMap = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "showTopographMap" ) ) 
      {
         showTopographMap = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "showPOI" ) ) 
      {
         showPOI = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "showRoute" ) ) 
      {
         showRoute = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "showScale" ) ) 
      {
         showScale = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "showTraffic" ) ) 
      {
         showTraffic = StringUtility::checkBoolean( tmpStr );
      } else {
         mc2log << warn <<  "XMLParserThread::xmlParseMapRequestHeader "
                << "unknown attribute for map_request_header element "
                << "Name " << attribute->getNodeName()
                << "Type " << attribute->getNodeType() << endl;
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
                                    "boundingbox" ) ) 
            {
               // Get bbox
               if ( !XMLCommonElements::readBoundingbox( 
                  child, bbox, errorCode, errorMessage ) ) 
               {
                  ok = false;
                  mc2log << warn << "XMLParserThread::"
                     "xmlParseMapRequestHeader"
                         << " bad boundingbox!" << endl;
               }
            } else if ( XMLString::equals( child->getNodeName(),
                                           "route_data" ) ) 
            {
               // Read route_id and route_turn
               ok = xmlParseMapRequestHeaderRouteData(
                  child, routeIDstr, beforeTurn, afterTurn,
                  errorCode, errorMessage );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "image_settings" ) ) 
            {
               if ( !readImageSettings( child, imageSettings ) ) {
                  mc2log << warn << "xmlParseMapRequestHeader "
                     "readImageSettings failed" << endl;
               }
            } else if ( XMLString::equals( child->getNodeName(),
                                           "phone_position" ) ) 
            {
               mc2log << warn << "XMLParserThread::"
                  "xmlParseMapRequestHeader "
                      << "ignoring phone_position" << endl;
            } else { // Odd element in map_request_header element
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseMapRequestHeader "
                      << "odd Element in map_request_header"
                      << " element: " << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParseMapRequestHeader "
                   << "odd node type in map_request_header element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   return ok;   
}


bool 
XMLParserThread::xmlParseMapRequestHeaderRouteData( 
   DOMNode* cur, 
   const char*& routeIDstr,
   uint32& beforeTurn,
   uint32& afterTurn,
   MC2String& errorCode, MC2String& errorMessage )
{
   if ( XMLString::equals( cur->getNodeName(), "route_data" ) ) {
      bool ok = true;
      DOMNode* child = cur->getFirstChild();

      while ( child != NULL && ok ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(),
                                       "route_id" ) ) 
               {
                  if ( child->getFirstChild() != NULL ) {
                     routeIDstr = XMLUtility::transcodefromucs( 
                        child->getFirstChild()->getNodeValue() );
                  } else {
                     routeIDstr = NULL;
                  }
               } else if ( XMLString::equals( child->getNodeName(),
                                              "route_turn" ) ) 
               {
                  char* tmpStr = XMLUtility::getChildTextValue( child );
                  if ( sscanf( tmpStr, "%X_%X", 
                               &beforeTurn, &afterTurn ) != 2 ) 
                  {
                     mc2log << warn << "XMLParserThread::"
                        "xmlParseMapRequestHeaderRouteData " 
                        "bad route_turn." << endl;
                     ok = false;
                     errorCode = "-1";
                     errorMessage = "Bad route_turn content.";
                  }
                  delete [] tmpStr;
               } else { // Odd element in route_data element
                  mc2log << warn << "XMLParserThread::"
                         << "xmlParseMapRequestHeaderRouteData "
                         << "odd Element in route_data"
                         << " element: " << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseMapRequestHeaderRouteData "
                      << "odd node type in route_data element: " 
                      << child->getNodeName() 
                      << " type " << child->getNodeType() << endl;
               break;
         }
         child = child->getNextSibling();
      }


      return ok;
   } else {
      return false;
   }
}


#endif // USE_XML

