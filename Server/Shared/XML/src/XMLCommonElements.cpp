/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLCommonElements.h"
#include <util/XMLString.hpp>
#include "StringConvert.h"

#ifdef USE_XML



bool 
XMLCommonElements::
getPositionItemData( const DOMNode* positionItem,
                     int32& lat, int32& lon,
                     uint16& angle,
                     MC2String& errorCode, MC2String& errorMessage,
                     XMLCommonEntities::
                     coordinateType* coordType )
{
   bool ok = true;
   angle = MAX_UINT16; // Init to not set

   if ( XMLString::equals( positionItem->getNodeName(), "position_item" ) )
   {
      XMLCommonEntities::coordinateType coordinateSystem = 
         XMLCommonEntities::WGS84;

      // Get attributes
      const DOMNamedNodeMap* attributes = positionItem->getAttributes();
      const DOMNode* attribute;

      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
         
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         if ( XMLString::equals( attribute->getNodeName(), 
                                 "position_system" ) ) 
         {
            coordinateSystem = 
               XMLCommonEntities::coordinateFormatFromString( 
                  tmpStr, XMLCommonEntities::WGS84 );
         } else {
            mc2log << warn << "XMLCommonElements::getPositionItemData "
                      "unknown attribute Name " << attribute->getNodeName()
                   << " Type " << attribute->getNodeType() << " Value "
                   << tmpStr << endl;
         }
         delete [] tmpStr;
      }

      if ( coordType != NULL ) {
         *coordType = coordinateSystem;
      }

      // Get children
      const DOMNode* child = positionItem->getFirstChild();
   
      while ( child != NULL && ok ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(), "lat" ) ) {
                  char* tmpStr = XMLUtility::getChildTextValue( child );
                  if ( !XMLCommonEntities::coordinateFromString( 
                          tmpStr, lat, coordinateSystem ) )
                  {
                     ok = false;
                     errorCode = "-1";
                     errorMessage = "Problem with lat-coordinate of"
                        " position_item.";
                     mc2log << warn << "XMLCommonElements::"
                               "getPositionItemData coordinateFromString "
                               " failed, problem with lat-coordinate. "
                            << tmpStr << endl;
                  }
                  delete [] tmpStr;
               } else  if ( XMLString::equals( child->getNodeName(), 
                                               "lon" ) )
               {
                  char* tmpStr = XMLUtility::getChildTextValue( child );
                  if ( !XMLCommonEntities::coordinateFromString( 
                          tmpStr, lon, coordinateSystem ) )
                  {
                     ok = false;
                     errorCode = "-1";
                     errorMessage = "Problem with lon-coordinate of"
                        " position_item.";
                     mc2log << warn << "XMLCommonElements::"
                               "getPositionItemData coordinateFromString "
                               " failed, problem with lon-coordinate. "
                            << tmpStr << endl;
                  }
                  delete [] tmpStr;
               } else  if ( XMLString::equals( child->getNodeName(), 
                                               "angle" ) ) {
                 

                  try { 
                     MC2String tmpStr( XMLUtility::getChildTextStr( *child ) );
                     StringConvert::assign( angle, tmpStr );
                     if ( angle > 360 ) {
                        ok = false;
                        errorCode = "-1";
                        errorMessage = "Angle of position item is out of"
                                       " range.";
                     }
                  } catch ( const StringConvert::ConvertException& e ) {
                     ok = false;
                     errorCode = "-1";
                     errorMessage = e.what();
                  }
                 
               } else {
                  mc2log << warn << "XMLCommonElements::getPositionItemData "
                            "odd Element in position_item element: " 
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
               mc2log << warn 
                      << "XMLCommonElements::getPositionItemData odd"
                         " node type in position_item element: " 
                      << child->getNodeName() 
                      << " type " << child->getNodeType()<< endl;
               break;
         }
         child = child->getNextSibling();
      }
      
   } else {
      ok = false;
   }

   return ok;
}



bool 
XMLCommonElements::readBoundingbox( const DOMNode* cur, MC2BoundingBox& bbox,
                                    MC2String& errorCode, 
                                    MC2String& errorMessage )
{
   if ( XMLString::equals( cur->getNodeName(), "boundingbox" ) ) {
      bool ok = true;
      // Attributes
      const DOMNamedNodeMap* attributes = cur->getAttributes();
      const DOMNode* attribute;

      // Find position_sytem
      const DOMNode* position_sytem = attributes->getNamedItem( 
         X("position_sytem") );
      char* formatStr = XMLUtility::transcodefromucs( 
         position_sytem->getNodeValue() );
      XMLCommonEntities::coordinateType coordinateFormat =
         XMLCommonEntities::coordinateFormatFromString( formatStr );
      delete [] formatStr;

      for ( uint32 i = 0 ; i < attributes->getLength() && ok ; i++ ) {
         attribute = attributes->item( i );
      
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         int32 latLon = 0;
         if ( XMLString::equals( attribute->getNodeName(), "north_lat" ) )
         {
            if ( XMLCommonEntities::coordinateFromString( 
                    tmpStr, latLon, coordinateFormat ) ) 
            {
               bbox.setMaxLat( latLon );               
            } else {
               mc2log << warn << "XMLCommonElements::readBoundingbox "
                      << "north_lat not a valid coordinate." << endl;
               ok = false;  
               errorCode = "-1";
               errorMessage = "north_lat not a valid coordinate in"
                  " boundingbox.";
            }
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "west_lon" ) )
         {
            if ( XMLCommonEntities::coordinateFromString( 
                    tmpStr, latLon, coordinateFormat ) )
            {
               bbox.setMinLon( latLon );               
            } else {
               mc2log << warn << "XMLCommonElements::readBoundingbox "
                      << "west_lon not a valid coordinate." << endl;
               ok = false;  
               errorCode = "-1";
               errorMessage = "west_lon not a valid coordinate in"
                  " boundingbox.";
            }
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "south_lat" ) )
         {
            if ( XMLCommonEntities::coordinateFromString( 
                    tmpStr, latLon, coordinateFormat ) )
            {
               bbox.setMinLat( latLon );               
            } else {
               mc2log << warn << "XMLCommonElements::readBoundingbox "
                      << "south_lat not a valid coordinate." << endl;
               errorCode = "-1";
               errorMessage = "south_lat not a valid coordinate in"
                  " boundingbox.";
               ok = false;  
            }
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "east_lon" ) ) 
         {
            if ( XMLCommonEntities::coordinateFromString( 
                    tmpStr, latLon, coordinateFormat ) )
            {
               bbox.setMaxLon( latLon );               
            } else {
               mc2log << warn << "XMLCommonElements::readBoundingbox "
                      << "east_lon not a valid coordinate." << endl;
               errorCode = "-1";
               errorMessage = "east_lon not a valid coordinate in"
                  " boundingbox.";
               ok = false;  
            }
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "position_sytem" ) )
         {
            // Already handled
         } else {
            mc2log << warn << "XMLCommonElements::readBoundingbox "
                   << "unknown attribute for boundingbox element "
                   << "Name " << attribute->getNodeName()
                   << "Type " << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }
      
      return ok;
   } else {
      errorCode = "-1";
      errorMessage = "Not a boundingbox!";
      return false;
   }
}


#endif // USE_XML
