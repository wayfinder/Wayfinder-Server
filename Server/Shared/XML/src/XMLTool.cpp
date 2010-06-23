/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLTool.h"
#include "StringUtility.h"

namespace XMLTool {

template <>
void addAttrib<uint32>( DOMElement* element, const char* name, 
                        const uint32& value ) {
   element->setAttribute( X( name ), XUint32( value ) );
}

template <>
void addAttrib<uint16>( DOMElement* element, const char* name, 
                        const uint16& value ) {
   element->setAttribute( X( name ), XUint32( value ) );
}

template <>
void addAttrib<unsigned long>( DOMElement* element, const char* name, 
                               const unsigned long& value ) {
   // XUint64 is used so this will work on both 32 and 64 bit architecture
   element->setAttribute( X( name ), XUint64( value ) );
}

template <>
void addAttrib<long>( DOMElement* element, const char* name, 
                      const long& value ) {
   element->setAttribute( X( name ), XInt64( value ) );
}

template <>
void addAttrib<int>( DOMElement* element, const char* name, 
                     const int& value ) {
   element->setAttribute( X( name ), XInt32( value ) );
}

template <>
void addAttrib<MC2String>( DOMElement* element, const char* name, 
                           const MC2String& value ) {
   element->setAttribute( X( name ), X( value ) );
}

template <>
void addAttrib<bool>( DOMElement* element, const char* name,
                      const bool& value ) {
   addAttrib<MC2String>( element, name, value ? "true" : "false" );
}

template <>
void setNodeValue<MC2String>( DOMElement* element, const MC2String& value ) { 
   DOMDocument* doc = element->getOwnerDocument();
   element->appendChild( doc->createTextNode( X( value ) ) );
}

template <>
void setNodeValue<XMLCh>( DOMElement* element, const XMLCh& value ) { 
   DOMDocument* doc = element->getOwnerDocument();
   element->appendChild( doc->createTextNode( &value ) );
}

template <>
void setNodeValue<uint32>( DOMElement* element, const uint32& value ) {
   DOMDocument* doc = element->getOwnerDocument();
   element->appendChild( doc->createTextNode( XInt32( value ) ) );
}

template <>
void setNodeValue<bool>( DOMElement* element, const bool& value ) {
   setNodeValue<MC2String>( element, value ? "true" : "false" );
}

template <>
void setNodeValue<int>( DOMElement* element, const int& value ) {
   char buff[255];
   snprintf( buff, 255, "%d", value );
   setNodeValue<MC2String>( element, buff );
}

const DOMNode* findAttribConst( const DOMNode* root,
                                const char* attribName,
                                bool caseCmp ) {
   if ( root->getAttributes() == NULL ) {
      return NULL;
   }

   if ( caseCmp ) {
      return root->getAttributes()->getNamedItem( X( attribName ) );
   }

   if ( root->getAttributes()->item( 0 ) == NULL ) {
      return NULL;
   }
   XTmpStr str( attribName );
   DOMNamedNodeMap* map = root->getAttributes();
   for ( XMLSize_t i = 0; map->item( i ) != NULL; ++i ) {
      if ( XMLString::compareIString( str, map->item( i )->getNodeName() ) == 0 ) {
         return map->item( i );
      }
   }
   return NULL;
}


DOMNode* findNode( DOMNode* root, const char* nodeName ) {
   for ( DOMNode* child = root->getFirstChild();
         child != NULL;
         child = child->getNextSibling() ) {
      if ( XMLString::equals( child->getNodeName(),
                              nodeName ) ) {
         return child;
      }
   }

   return NULL;
}

const DOMNode* findNodeConst( const DOMNode* root,
                              const char* nodeName ) {
   return findNode( const_cast<DOMNode*>( root ),
                    nodeName );
}

DOMNode* findNodeIgnoreNamespace( DOMNode* root, const char* nodeName ) {
   for ( DOMNode* child = root->getFirstChild(); child != NULL;
         child = child->getNextSibling() ) {
      MC2String name = XMLUtility::transcodefrom( child->getNodeName() );
      size_t colonPos = name.find( ':' );
      if ( colonPos != MC2String::npos ) {
         // Prefixed by namespace
         name.erase( 0, colonPos + 1 );
      }
      if ( strcmp( name.c_str(), nodeName ) == 0 ) {
         return child;
      }
   }

   return NULL;
}

const DOMNode* findNodeIgnoreNamespaceConst( const DOMNode* root, 
                                             const char* nodeName ) {
   return findNodeIgnoreNamespace( const_cast<DOMNode*>( root ), nodeName );
}

MC2String getNodeValue( const char* nodeName, const DOMNode* root ) 
   throw ( Exception ){

   // find first matching element
   const DOMNode* child = findNodeConst( root, nodeName );
   if ( child == NULL ) {
      throw Exception("No such node", nodeName );
   }

   return XMLUtility::getChildTextStr( *child );
}

MC2String getNodeValue( const DOMNode* root, 
                        const MC2String& defaultValue ) 
{
   return XMLUtility::getChildTextStr( *root, defaultValue );
}

/*
bool getNodeValue( MC2String& value, const char* nodeName, 
                   const DOMNode* root )
{
   try {
      value = getNodeValue( nodeName, root );
      return true;
   } catch( ... ) {
      return false;
   }
}
*/

MC2String getTrimmedNodeValue( const DOMNode* root, 
                               const MC2String& defaultValue ) 
{
   MC2String value = getNodeValue(root, defaultValue);
   return StringUtility::trimInPlace(value);
}


}
