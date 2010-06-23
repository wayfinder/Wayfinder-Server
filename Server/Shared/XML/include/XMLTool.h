/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLTOOL_H
#define XMLTOOL_H

#include "XMLUtility.h"
#include "StringConvert.h"
#include <stdexcept>

/**
 * Contains usefull templated tools for reading and
 *  manipulating XML documents.
 */
namespace XMLTool {

/// exception thrown by XMLTool functions
class Exception:public std::exception {
public:

   /**
    * @param what exception message
    * @param nodeName the name of the node that caused exception
    */
   Exception( const MC2String& what, 
              const MC2String& nodeName ) throw():
      m_what( MC2String( "\"" ) + nodeName + "\" : " + what ),
      m_node( nodeName ) { 
   }
   ~Exception() throw() {
   }
   /// @return name of the node that caused exception
   const MC2String& getNodeName() const { return m_node; }
   /// @return exception message
   const char* what() const throw() { return m_what.c_str(); }

private:
   MC2String m_what; //< exception description
   MC2String m_node; //< node name
}; 

/**
 * adds a attrib with a generic type, the type is converted in
 * the specialized template.
 * Basic types like int32 and MC2String are declared in the .cpp file
 * @param node the element for which the new attrib should be inserted
 * @param name the name of the attribute
 * @param value the value of the attribute
 */
template <typename T>
void addAttrib( DOMElement* node, const char *name, const T& value );

/**
 * Sets a nodes child value to "value"
 * @param node the node to set elemenet value
 * @param value the value to assign the node
 */
template <typename T>
void setNodeValue( DOMElement* node, const T& value );

/**
 * Determines the document owner of \c node.
 * @param node The node to find the owner for.
 * @return Document owner for the \c node.
 */
inline DOMDocument* getOwner( DOMNode* node ) {
   if ( node->getNodeType() == DOMNode::DOCUMENT_NODE ||
        node->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE ) {
      return static_cast<DOMDocument*>( node );
   }
   return node->getOwnerDocument();
}

/**
 * Adds child to root and returns child pointer
 * @param root the root of the element tree
 * @param name the name of the new element
 * @return newly created element 
 */
inline DOMElement* addNode( DOMNode* root, const char* name ) {
   DOMElement* element = root->getOwnerDocument()->createElement( X( name ) );
   root->appendChild( element );
   return element;
}

/**
 * Searches for first matching attribute in root.
 * @param root in which we should find the attribute
 * @param attribName the name of the attribute
 * @param caseCmp whether it should ignore case or not
 * @return pointer to attribute node
 */
const DOMNode* findAttribConst( const DOMNode* root, const char* attribName,
                                bool caseCmp = true );

/**
 * Adds a new child to root with a value
 * @param root the root to which the new child should be child of.
 * @param name the name of the new child
 * @param value value of the child element
 * @return newly created child
 */
template <typename T>
DOMElement* addNode( DOMNode* root,
                     const char* name, const T& value ) {
   
   DOMElement* nameElement = addNode( root, name );

   setNodeValue( nameElement, value );

   return nameElement;
}

/**
 * Sets attribValue to string value and returns true on success, 
 * else false if attribName does not exists.
 * @param attribValue the string value of the attribute
 * @param attribName name of the attribute in the node
 * @param node the node that should contain the attrib 
 * @return true if attribName was found else false. If return is false, the
 *              attribValue is unassigned.
 */
template <typename T>
bool getAttribValue( T& attribValue, const char* attribName,
                     const DOMNode* node ) throw()
try {
   const DOMNode* attribNode = findAttribConst( node, attribName );

   if ( attribNode == NULL ) {
      return false;
   }

   StringConvert::assign( attribValue, 
                          XMLUtility::transcodefrom( attribNode->
                                                     getNodeValue() ) );

   return true;

} catch ( const StringConvert::ConvertException& e ) {
   return false;
}

/**
 * gets attribute string value and assigns "dest" to it.
 *
 * @param dest destination variable
 * @param attribName the name of the attribute in node
 * @param node the node that should contain the attribName
 */
template <typename T>
void getAttrib( T& dest, const char* attribName, const DOMNode* node ) 
   throw ( Exception ) {
   MC2_ASSERT( attribName );

   const DOMNode* attribNode = findAttribConst( node, attribName );
   if ( attribNode == NULL ) {
      throw Exception("No such attribute", attribName );
   }

   try {
      StringConvert::assign<T>( dest, XMLUtility::
                                transcodefrom( attribNode->getNodeValue() ) );
   } catch ( const StringConvert::ConvertException& e ) {
      throw Exception( e.what(), attribName );
   }
}

/**
 * Tries to set dest from attrib value, if it fails it will set 
 * the default value. This function does not throw.
 *
 * @param dest destination variable
 * @param attribName the name of the attribute
 * @param node the node to get the value from
 * @param defaultValue the default value to set dest if attribName was not found
 */
template <typename T>
void getAttrib( T& dest, const char* attribName, const DOMNode* node,
                const T& defaultValue ) throw() {
   try {
      getAttrib<T>( dest, attribName, node );
   } catch ( const Exception& e ) {
      dest = defaultValue;
   }
}


/**
 * Searches for first matching node.
 * @param root in which we should find the node
 * @param nodeName the name of the node to find
 * @return pointer to node
 */
DOMNode* findNode( DOMNode* root, const char* nodeName );
const DOMNode* findNodeConst( const DOMNode* root, const char* nodeName );

/**
 * Searches for first matching node, comparing without namespaces.
 *
 * @param root In which we should find the node.
 * @param nodeName The name of the node to find.
 * @return Pointer to matching node.
 */
DOMNode* findNodeIgnoreNamespace( DOMNode* root, const char* nodeName );
const DOMNode* findNodeIgnoreNamespaceConst( const DOMNode* root, 
                                             const char* nodeName );


/** 
 * Returns string value of named node or throws an exception if
 * the node does not exist.
 * @param nodeName the name of the node
 * @param root the root from which we should search for a child that matches
 *        nodeName
 * @return string value of node
 */
MC2String getNodeValue( const char* nodeName, const DOMNode* root ) 
   throw ( Exception );

/**
 * Sets dest to value of node.
 * This function uses StringConvert::assign to convert value from string.
 *
 * @param dest type specific value to be set
 * @param nodeName the name of the node
 * @param root the root node that should contain the named node.
 */
template <typename T>
void getNodeValue( T& dest, const char* nodeName, const DOMNode* root ) 
   throw ( Exception ) {
   try {
      StringConvert::assign<T>( dest, getNodeValue( nodeName, root ) );
   } catch ( StringConvert::ConvertException& e ) {
      throw Exception(e.what(), nodeName );
   }
}

/** 
 * Get the text contained in a named node.
 *
 * @param value Set the to text if ok.
 * @param nodeName the name of the node
 * @param root the root from which we should search for a child that matches
 *        nodeName
 * @return True if node exists and has text child.
 */
template <typename T>
bool getElementValue( T& value, const char* nodeName, 
                      const DOMNode* root )
{
   try {
      getNodeValue( value, nodeName, root );
      return true;
   } catch( ... ) {
      return false;
   }
}

/**
 * Searches for the first TEXT_NODE and get its text value.
 * @param root the root to start searching for a TEXT_NODE type element.
 * @param defaultValue returns this value if no node was found.
 * @return value of the first TEXT_NODE
 */
MC2String getTrimmedNodeValue( const DOMNode* root, 
                               const MC2String& defaultValue = MC2String() );


/**
 * Sets dest to value of node, if node is not found or conversion failed the
 * defaultValue will be used.
 *
 * This function uses StringConvert::assign to convert value from string.
 *
 * @param dest type specific value to be set
 * @param nodeName the name of the node
 * @param root the root node that should contain the named node.
 * @param defaultValue the value to set if normal getNodeValue failed.
 */
template <typename T>
void getNodeValue( T& dest, const char* nodeName, const DOMNode* root, 
                   const T& defaultValue ) throw() {
   try {
      getNodeValue<T>( dest, nodeName, root );
   } catch ( const Exception& e ) {
      dest = defaultValue;
   }
}

/**
 * Returns the text value of a node containing text. The text is
 * converted to the type specified by the template argument.  If
 * the node has no child node , if the child node is anything
 * except a TEXT_NODE, or if there is problem in the conversion to
 * the template type the default value will be returned. 
 *
 * @param root The node that should contain text.
 * @param defaultValue The backup return value.
 * @return The converted value of the node content.
 */
template<typename T>
T getNodeValue( const DOMNode* root, 
                const T& defaultValue = T() ) 
{
   using namespace StringConvert;
   try {
      DOMNode* child = root->getFirstChild();
      if ( child != NULL && child->getNodeType() == DOMNode::TEXT_NODE ) {
         return StringConvert::convert<T>( getTrimmedNodeValue( root ));
      }
   } catch ( const ConvertException& e ) {
   }
   return defaultValue;
}


/**
 * Return the content of the child node of <code>root</code>. If
 * there is no child node or if the child node is not a TEXT_NODE
 * the <code>defaultValue</code> will be returned instead.
 * @param root The node.
 * @param defaultValue The default value.
 * @return The node content. 
 */
MC2String getNodeValue( const DOMNode* root, 
                        const MC2String& defaultValue = MC2String() ) ;


}
#endif
