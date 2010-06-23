/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
package com.itinerary.ServerCommunicator;

import java.util.Hashtable;
import java.util.StringTokenizer;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Stack;
import java.util.EmptyStackException;

/**
 *   Parses an XML Document.
 *   Puts the parsed elements on a stack. The stack is used to find
 *   the parent to an object and to compare the start and end tags.
 *   The elements are first created as a start TempXMLElement and
 *   and end TempXMLElement (or both start and end ). These are combined
 *   into SimpleXMLElements when the start and end tags match.
 *   <br>
 *   BUGS:
 *   <br>
 *   Entities are not handled.
 *   <br>
 *   Could probably be optimized more.
 */
public class SimpleXMLParser {

   /**
    *   Creates a SimpleXMLDocument from an inputstream.
    *   @param is The <code>InputStream</code> to read from.
    *   @return A <code>SimpleXMLDocument</code> if succesful.
    */
   public SimpleXMLDocument parse(InputStream is)
      throws IOException, SimpleXMLException {

      // I guess we will have to read the entire file into a string.
      final int chunk_size = 65536;
      byte [] buffer = new byte[chunk_size];
      ByteArrayOutputStream bao = new ByteArrayOutputStream();
      int res = 0;
      do {
         res = is.read(buffer, 0, chunk_size);
         if ( res > 0 ) 
            bao.write(buffer, 0, res);
      } while ( res > 0 );      
      buffer = bao.toByteArray();    
      String xmlText = new String(buffer);
      
      return parse(xmlText);
   }

   
   /**
    *    Parses a <code>String</code> and returns a SimpleXMLDocument.
    *    @param xmlText The text to parse.
    *    @return A <code>SimpleXMLDocument</code> if succesful.
    */
   SimpleXMLDocument parse(String xmlText) throws SimpleXMLException {
      // Create the document.
      SimpleXMLDocument doc = new SimpleXMLDocument();
      
      // Stack for pushing the previous node onto
      Stack stack = new Stack();
      stack.push(doc);

      // The tokenizer. Find all the end tags
      StringTokenizer tok = new StringTokenizer(xmlText,">");
      while (tok.hasMoreTokens() ) {
         // Find the start tags.
         String [] elementAndData = handleGT(tok.nextToken());
         // Now we should have the element in the string[0]
         // and the data in [1].
         handleElement(doc, stack, elementAndData[0], elementAndData[1]);
      }
      return doc;
   }
   
   /**
    *   The <code>TempXMLElements</code> can be starttags, endttags
    *   or both. They are later combined into real elements. There
    *   is also a flag <code>isComment()</code> which is true if the
    *   tag is a comment. 
    */
   private class TempXMLElement {

      /**
       *   Creates a TempXMLElement from a string containing the
       *   text between the &gt;&lt;.
       *   @param elementString The text between &lt; and &gt; .
       */
      public TempXMLElement(String elementString) throws SimpleXMLException {
         // Remove the last character if the string ends with ?
         if ( elementString.endsWith("?") ) {
            elementString =
               elementString.substring(0, elementString.length() - 1);
         }
         StringTokenizer tok = new StringTokenizer(elementString, " ");

         boolean hasParameters;
         if ( tok.hasMoreTokens() ) {
            // There are parameters
            m_name = tok.nextToken();
            hasParameters = tok.hasMoreTokens();
         } else {
            // No parameters. The tag is the name only.
            m_name = elementString;
            hasParameters = false;
         }

         // Should we use th entire elementString in handleParameters()?

//           if (hasParameters){
//              Utility.debugPrint("----------------");
//              Utility.debugPrint(elementString);
//              Utility.debugPrint(m_name);
//           }
         
         // Check for comment, doctype and xml declaration.
         boolean isComment = m_name.startsWith("!--");
         m_isDeclaration = m_name.startsWith("?");
         m_isDocumentType = m_name.startsWith("!DOC");
         m_isCDATAType = m_name.startsWith("![CDATA");
         if ( m_isDocumentType || m_isCDATAType )
            m_documentData = elementString;
         
         if ( m_name.startsWith("/") ) {
            m_name = m_name.substring(1);
            m_isEndTag = true;
            m_isStartTag = false;
         } else if ( elementString.endsWith("/") || isComment ||
                     m_isDeclaration || m_isDocumentType ||
                     m_isCDATAType ) {
            if ( !hasParameters && ! isComment ) {
               // Remove the last character from the name
               m_name = m_name.substring(0, m_name.length() - 1);
            }
            m_isStartTag = true;
            m_isEndTag   = true;
         } else {
            m_isStartTag = true;
            m_isEndTag   = false;
         }
         // Handle the parameters
         if ( ! isComment ) {
            // Start the parameter parsing at the first space
            int firstSpace = elementString.indexOf(' ');
            if ( firstSpace > 0 ) {
               String paramString = elementString.substring(firstSpace);
               m_params = handleParameters(paramString);
            }
         }
         m_isComment = isComment;

//           Utility.debugPrint("m_isComment == " +
//                              String.valueOf(m_isComment));
//           Utility.debugPrint("m_isDeclaration == " +
//                              String.valueOf(m_isDeclaration));
//           Utility.debugPrint("m_isDocumentType == " +
//                              String.valueOf(m_isDocumentType));
//           Utility.debugPrint("m_isCDATAType == " +
//                              String.valueOf(m_isCDATAType));
//           Utility.debugPrint("m_isStartTag == " +
//                              String.valueOf(m_isStartTag));
//           Utility.debugPrint("m_isEndTag == " +
//                              String.valueOf(m_isEndTag));
      }

      /**
       *   Handles the attributes of the tag.
       *   @param params The string containing the parameters.
       *   @return A hashtable containing the attributes.
       */
      Hashtable handleParameters(String params) throws SimpleXMLException {
         params = params.trim();
         if ( params.endsWith("/") ) {
            params = params.substring(0, params.length() - 1);
         }
         Hashtable table = new Hashtable();
         StringTokenizer tok = new StringTokenizer(params, "=");
         try {
            boolean keepRunning = true;
            String key ="";
            if ( tok.hasMoreTokens() ) {
               // Find the key by searching for the =
               key = tok.nextToken("=");
               key = key.trim();
               if (key.startsWith("\" ") && key.length() >= 2) {
                  key = key.substring(2, key.length());
               }
               // Keep parsing if there is a "=" in the string.
               // Otherwise there cannot be any value
               keepRunning = params.indexOf("=") != -1;
            } else {
               keepRunning = false;
            }
            while ( keepRunning ) {
               // Skip until the first qoute
               String dummy = tok.nextToken("\"");
               // The value ends at the next quote.
               String value = tok.nextToken("\"");
               table.put(key, value);
               // Check if there are more "\"":s
               if ( tok.hasMoreTokens() ) {
                  key = tok.nextToken("=");
                  key = key.trim();
                  if (key.startsWith("\" ") && key.length() >= 2) {
                     key = key.substring(2, key.length());
                  }
               } else {
                  keepRunning = false;
               }
            }
         } catch ( Exception e ) {
            throw new SimpleXMLException("Error when parsing parameters " +
                                         params + e);
         }
         return table;
      }
      
      /**
       *   Takes a string which begins and ends with quotes and removes
       *   them.
       */
      public final String removeQuotes(String with) {
         if ( with.startsWith("\"") && with.endsWith("\"") ) {
            return with.substring(1, with.length() - 1);
         } else {
            return with;
         }
      }
      
      /**
       *   @return The name of this tag.
       */
      public final String getName() {
         return m_name;
      }

      /**
       *   @return True if the tag is an endtag.
       */
      public final boolean isEndTag() {
         return m_isEndTag;
      }

      /**
       *   @return True if the tag is a startTag,
       */
      public final boolean isStartTag() {
         return m_isStartTag;
      }

      /**
       *   @return True if the tag is a comment.
       */      
      public final boolean isComment() {
         return m_isComment;
      }

      /**
       *   @return True if the tag is a declaration.
       */
      public final boolean isDeclaration() {
         return m_isDeclaration;
      }

      /**
       *   @return True if the tag is the document type.
       */
      public final boolean isDocumentType() {
         return m_isDocumentType;
      }

      /**
       *   @return True if the tag contains CDATA.
       */
      public final boolean isCDATAType() {
         return m_isCDATAType;
      }
      
      /**
       *   Creates a SimpleXMLElement from this TempXMLElement.
       */
      public SimpleXMLElement getSimpleXMLElement() {
         Hashtable params = null;
         if ( m_params != null && ! m_params.isEmpty() )
            params = m_params;
         if ( m_isDeclaration ) {
            return new SimpleXMLDeclaration(m_name, m_params);
            // MJ: --- document or cdata
         } else if ( m_isDocumentType || m_isCDATAType ) {
            return new SimpleXMLDocumentData("<" + m_documentData + ">");
         } else {
            return new SimpleXMLElement(m_name, m_params);
         }
      }
      
      private String m_name;
      private boolean m_isStartTag;
      private boolean m_isEndTag;
      private Hashtable m_params;
      private boolean m_isComment;
      private boolean m_isDocumentType;
      private boolean m_isCDATAType;
      private boolean m_isDeclaration;
      private String  m_documentData;
   }
   
   /**
    *   Handles that we find a greater than in the text.
    *   @param uptoGT The string upto greater than.
    *   @return An array with two strings. The first is the
    *           tag including attributes and the second is data
    *           if any.
    */
   private String [] handleGT(String uptoGT) {
      StringTokenizer tok = new StringTokenizer(uptoGT, "<");
      int i=0;
      String element = "";
      String data = "";
      while ( tok.hasMoreTokens() ) {
         data = element;
         element = tok.nextToken();
         i++;
      }
      data = data.trim();
      element = element.trim();
      return new String[] { element, data };

   }

   /**
    *   Handles an element string from the pre-parsing.
    *   @param doc   The document to add the things to.
    *   @param stack The stack to put the finished elements on.
    *   @param elString The string containing the element.
    *   @param data     The data for the start tag of the element.
    */
   private void handleElement(SimpleXMLDocument doc,
                              Stack stack,
                              String elString,
                              String data) throws SimpleXMLException {
      elString = elString.trim();
      TempXMLElement el = null;
      if ( elString.length() > 0 ) {
         // Create the parsing TempXMLElement
         el = new TempXMLElement(elString);
      }
      if ( data.length() > 0 ) {
         // There is data. Add it to the parent.
         SimpleXMLElement stackElement = (SimpleXMLElement)stack.peek();
         
         SimpleXMLDataElement dataEl = new
            SimpleXMLDataElement(contractEntities(data));

         stackElement.setData(dataEl);
      }
      // Check if there is an element and if it should be added to the
      // parent
      if ( el != null ) {
         try {
            if ( el.isEndTag() && ! el.isStartTag() ) {
               // Is an end tag
               // Get the start tag from the stack.
               SimpleXMLElement popElement = (SimpleXMLElement)stack.pop();
               if ( ! popElement.getName().equals(el.getName())) {
                  throw new SimpleXMLException("End tag </" + el.getName()
                                               + "> does not match start tag <"
                                               + popElement.getName() +">");
               } else {
                  // Get the parent
                  SimpleXMLElement stackElement =
                     (SimpleXMLElement)stack.peek();
                  // Add the child
                  stackElement.addChild(popElement);
               }
            } else if ( el.isEndTag() && el.isStartTag() ) {
               if ( el.isDeclaration() ) {
                  // The document declaration. Put it in the document
                  try {
                     SimpleXMLDocument d = (SimpleXMLDocument)(stack.peek());
                     d.setDeclaration(
                        (SimpleXMLDeclaration)el.getSimpleXMLElement());
                  } catch ( Exception e ) {
                     throw new SimpleXMLException("Declaration must"
                                                  +" be in document " + e);
                  }
               } else if ( el.isDocumentType() ) {
                  try {
                     SimpleXMLDocument d = (SimpleXMLDocument)(stack.peek());
                     d.setDocData(
                        (SimpleXMLDocumentData)el.getSimpleXMLElement());
                  } catch ( Exception e ) {
                     throw new SimpleXMLException("Doctype must"
                                                  +" be in document " + e);
                  }
               } else if ( el.isCDATAType() ) {
                  try {
                     // WARNING! --- This is a hack! ---
                     //
                     // Since CDATA is within <>, it is considered
                     // an element by the parser. To circumvent this
                     // we use the CDATA element - elString - as data.
                     //
                     // See SimpleXMLDataElement for an idea on how
                     // to solve the problem without cheating.
                     SimpleXMLElement elem = (SimpleXMLElement)(stack.peek());
                     elem.setData(trimCData(elString));
                  } catch ( Exception e ) {
                     throw new SimpleXMLException("Error extracting"
                                                  +" CDATA " + e);
                  }                  
               } else if ( ! el.isComment() ) {
                  // Both end and start tag
                  // Get the parent
                  SimpleXMLElement stackElement =
                     (SimpleXMLElement)stack.peek();
                  // Add the child
                  SimpleXMLElement addElement = el.getSimpleXMLElement();
                  stackElement.addChild(addElement);
               }
            } else {
               // Start tag
               SimpleXMLElement stackElement =
                  (SimpleXMLElement)stack.peek();
               stack.push(el.getSimpleXMLElement() );
            }
         } catch ( EmptyStackException e ) {
            throw new SimpleXMLException("Too many end tags");
         }
      }
   } // handleElement

   /**
    *   Converts "&amp;" into "&" etc
    */
   private String contractEntities(String s) {
      Utility.debugPrint("--- contractEntities BEGIN ---", 3);
      StringBuffer sBuf = new StringBuffer();
      StringTokenizer from = new StringTokenizer(s, "&");
      // First token never contracts, because first "&" occurs only
      // after first token
      if (from.hasMoreTokens()) {
         sBuf.append(from.nextToken());
      }
      Utility.debugPrint("--> " + sBuf.toString(), 3);
      // Walk through remaining tokens
      while (from.hasMoreTokens()) {
         // Found "&" or eos
         String subString = from.nextToken();
         Utility.debugPrint("--> subString == " + subString, 3);
         StringTokenizer to = new StringTokenizer(subString, ";");
         // For every "&" there can be only one matching ";"
         if (to.hasMoreTokens()) {
            // Found ";" or eos
            String subSubString = to.nextToken();
            Utility.debugPrint("--> subSubString == " + subSubString, 3);
            if (subSubString.equals("amp")) {
               sBuf.append("&").
                  append(subString.substring(subString.indexOf(";") + 1,
                                             subString.length()));
            }
//              else if (subSubString.equals("aring")) {
//                 sBuf.append("å").
//                    append(subString.substring(subString.indexOf(";") + 1,
//                                               subString.length()));
//              }
//              else if (subSubString.equals("auml")) {
//                 sBuf.append("ä").
//                    append(subString.substring(subString.indexOf(";") + 1,
//                                               subString.length()));
//              }
//              else if (subSubString.equals("ouml")) {
//                 sBuf.append("ö").
//                    append(subString.substring(subString.indexOf(";") + 1,
//                                               subString.length()));
//              }
//              else if (subSubString.equals("Aring")) {
//                 sBuf.append("Å").
//                    append(subString.substring(subString.indexOf(";") + 1,
//                                               subString.length()));
//              }
//              else if (subSubString.equals("Auml")) {
//                 sBuf.append("Ä").
//                    append(subString.substring(subString.indexOf(";") + 1,
//                                               subString.length()));
//              }
//              else if (subSubString.equals("Ouml")) {
//                 sBuf.append("Ö").
//                    append(subString.substring(subString.indexOf(";") + 1,
//                                               subString.length()));
//              }
            else {
               Utility.debugPrint("--> subString == " + subString, 3);
               sBuf.append("&").append(subString);
            }
         }
         else {
            Utility.debugPrint("--> subString == " + subString, 3);
            sBuf.append("&").append(subString);
         }
         Utility.debugPrint("--> " + sBuf.toString(), 3);
      }
      Utility.debugPrint("--- contractEntities END ---", 3);
      return sBuf.toString();
   } // contractEntities

   private String trimCData(String cdata) {
      String lTrim = "";
      String rTrim = "";
      StringTokenizer lBracket = new StringTokenizer(cdata, "[");
      while (lBracket.hasMoreTokens()) {
         lTrim = lBracket.nextToken();
      }
      StringTokenizer rBracket = new StringTokenizer(lTrim, "]");
      if (rBracket.hasMoreTokens()) {
         rTrim = rBracket.nextToken();
      }
      return rTrim;
   }
}





