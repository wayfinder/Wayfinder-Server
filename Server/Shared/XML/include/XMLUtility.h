/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLUTILITY_H
#define XMLUTILITY_H

#include "config.h"

#ifdef USE_XML
#include <dom/DOM.hpp>
#include <parsers/XercesDOMParser.hpp>

#include <util/PlatformUtils.hpp>
#include <util/XMLString.hpp>
#include <util/XMLUniDefs.hpp>
#include <framework/MemBufInputSource.hpp> 
#include <framework/XMLFormatter.hpp>
#include "MC2String.h"
#include "NewStrDup.h"
#include "TextIterator.h"
#include <util/XMLString.hpp>
#include "StackOrHeap.h"
#include "Xerces.h"

// Define XMLString::equals
//#define XMLString::equals(a,b) XMLString::compareString(a,b) == 0
class WFXMLStr : public XMLString {
  public:
   static bool equals (const XMLCh *const str1, const XMLCh *const str2) {
      return XMLString::compareString(str1,str2) == 0;
   }
   
   static bool equals (const char *const str1, const char *const str2) {
      return XMLString::compareString(str1,str2) == 0;
   }

   static bool equals (const XMLCh *const str1, const char *const str2) {
#if 0
      // Version without alloc - tested a bit. Activate later.
      // Will it work for > 65535?
      // Will the other one work for > 128?
      const XMLCh* s1 = str1;
      mc2TextIterator s2(str2);
      for ( ;; ) {
         // Difference
         if ( *s1 != *s2 ) {
            return false;
         }
         // No differences so far and we're at the end
         if ( *s2 == 0 ) {
            return true;
         }
         ++s1;
         ++s2;
      }
#else
      // Old version.
      XMLCh* s2 = XMLString::transcode( str2 );
      bool res = XMLString::compareString(str1,s2) == 0;
      delete [] s2;
      return res;
#endif
   }
   
};
// Whoooo!
#define XMLString WFXMLStr


/**
 * Things that can make handling XML documents easier.
 *
 */
class XMLUtility {
   public:
      /**
       *   Create a XML-string from an array of char.  This handles
       *   unicode/iso, if you use XMLString::transcode()
       *   you'll only get 7-bit ASCII.
       *
       *   @param str The array to convert
       *   @return An XML-string based on str
       */
      static XMLCh* transcodetoucs( const char* const str );

      /**
       *   Copies the src string to dest as XMLCh with
       *   conversion. Caller must see to that the
       *   destination string has enough space allocated.
       */
      static void transcodeToXML( XMLCh* dest,
                                  const char* src );

      /**
       *   Create a XML-string from an MC2String.  This handles
       *   unicode/iso, if you use XMLString::transcode()
       *   you'll only get 7-bit ASCII.
       *
       *   @param str The MC2String to convert
       *   @return An XML-string based on str
       */
      static XMLCh* transcodetoucs( const MC2String& str );


      /**
       *   Create a new char array from an XML-string. This handles
       *   unicode/iso, if you use XMLString::transcode()
       *   you'll only get 7-bit ASCII.
       *
       *   @param str The XML-String to convert
       *   @return Pointer to a char array (should be deleted by the caller)
       */
      static char* transcodefromucs( const XMLCh* str );


      /**
       *   Create a new char array from an XML-string. This handles
       *   unicode/iso, if you use XMLString::transcode()
       *   you'll only get 7-bit ASCII.
       *
       *   @param str The XML-String to convert
       *   @return Referece
       */
      static MC2String transcodefrom( const XMLCh* str );

      
      /**
       * Gets the text child of cur.
       * @param cur Where to look for a text-node.
       * @return A new string with the text in the textnode or "" if 
       *         no textnode in cur.
       */
      static inline char* getChildTextValue( const DOMNode* cur );
      
      /**
       * Gets the text child of cur.
       * @param cur Where to look for a text-node.
       * @param defaultValue The value if no text found
       * @return A new string with the text in the textnode or 'defaultValue' if 
       *         no textnode in cur.
       */
      static MC2String getChildTextStr( const DOMNode& cur, 
                                        MC2String defaultValue = MC2String() );

      /**
       *   Intents a piece of a document afterwards. No indentation
       *   should have been applied before.
       *   @param top  The node to start the indentation at.
       *   @param indentLevel Level of indentation
       *   @param nbr_spaces  Spaces per indentation level, max 8
       */
      static void indentPiece( DOMNode& top,
                               int indentLevel,
                               unsigned int nbr_spaces = 3 );

   /**
    * Copies attribute from src to dest.
    *
    * @param dest destination
    * @param src source of attribute
    * @param attribName name of the attribute
    * @return true if attribute is found in src, else false.
    */
   static bool copyAttribute( DOMElement& dest, const DOMNode& src,
                              const char* attribName );

   /** 
    * Creates a standard reply from a request with transaction_id
    * @param doc document to create new nodes in.
    * @param request for which we should reply to.
    * @return root element
    */
   static DOMElement* createStandardReply( DOMDocument& doc,
                                           const DOMNode& request,
                                           const char* replyName );

  private:
      /**
       *   Indents a piece of a document afterwards.
       *   Should be called by the other indentPiece.
       *   @param top  The node to start the indentation at.
       *   @param parent Parent node. Intentation will be inserted before top
       *                 in parent.
       *   @param indentLevel Level of indentation
       *   @param nbr_spaces  Spaces per indentation level
       *   @param indentStr   String containing \n and spaces upto the end.
       *   @param max_indent  max_indent*nbr_spaces+2 == length of indentStr.
       */
      static void indentPiece( DOMNode* top,
                               DOMNode* parent,
                               int indentLevel,
                               unsigned int nbr_spaces,
                               XMLCh* indentStr,
                               int max_indent );
      
};


/**
 * Stream out an XML-string on an ostream.
 * @param target The ostream to put toWrite on.
 * @param toWrite The string to outout.
 * @return Refernce to target.
 */
extern ostream& operator<<(ostream& target, const XMLCh* toWrite);

class XStr {
   public :
      template<class STRING> XStr( const STRING& str ) {
         m_str = XMLUtility::transcodetoucs( str );
      }
      
      ~XStr() {
         delete [] m_str;
      }

      const XMLCh* XMLStr() const {
         return m_str;
      }

   private:
      XMLCh* m_str;
};

/**
 *    Temporary string to use in the X macro.
 *    Will allocate space on the stack and use it
 *    if the string is short enough for it.
 */
class XTmpStr {
public:

   /// Constructor 1 - for const char*
   XTmpStr( const char* str ) : m_str( strlen(str) + 1 ) {
      XMLUtility::transcodeToXML( m_str, str );
   }

   /// Constructor 2 - for MC2String
   XTmpStr( const MC2String& str ) : m_str( str.length() + 1 ) {
      XMLUtility::transcodeToXML( m_str, str.c_str() );
   }
   
   /// Returns the XMLCh*
   operator const XMLCh*() const { return m_str; }
private:
   /// Storage. It we are lucky it will be on the stack.
   StackOrHeap<256, XMLCh> m_str;
};

// This is cheaper than before, but it is still convenient.
#define X(s) XTmpStr(s)

/**
 *   Class for simple conversion from number to
 *   XMLCh*. Uses stack only for allocation (unless newed).
 */
template<class VALTYPE> class XMLNum {
public:
   XMLNum( VALTYPE val, const char* format ) {
      // Print the string in ascii (0-9,a-f)
      char tmp[64];
      sprintf(tmp, format, val );
      // Convert ascii.
      XMLCh* dest = m_str;
      char* src = tmp;
      do {
         *dest++ = *src;
      } while ( *src++ != '\0' );
   }

   operator const XMLCh*() const {
      return m_str;
   }
private:
   /// Stacked string.
   XMLCh m_str[64];   
};

#define XInt32(x)  (XMLNum<int32>(x,"%d"))
#define XHex32(x)  (XMLNum<uint32>(x,"%x"))
#define XUint32(x) (XMLNum<uint32>(x,"%u"))
#define XInt64(x) (XMLNum<int64>(x,"%lld"))
#define XHex64(x) (XMLNum<uint64>(x,"%llx"))
#define XUint64(x) (XMLNum<uint64>(x,"%llu"))

// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -


inline char* 
XMLUtility::getChildTextValue( const DOMNode* cur ) {
   return NewStrDup::newStrDup( getChildTextStr( *cur ) ); 
}

inline XMLCh*
XMLUtility::transcodetoucs( const MC2String& str ) {
   return transcodetoucs( str.c_str() );
}


#endif // USE_XML

#endif // XMLUTILITY_H

