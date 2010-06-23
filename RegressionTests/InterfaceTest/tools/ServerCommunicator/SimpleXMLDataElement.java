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

import java.util.StringTokenizer;

/**
 *   SimpleXMLElement containing nothing but a string of data.
 */
class SimpleXMLDataElement extends SimpleXMLElement {

   /**
    *   Creates a new <code>SimpleXMLDataElement</code>.
    *   @param data The data of the element.
    */
   SimpleXMLDataElement(String data) {
      super("", null, DATA);
      m_data = data;
   }

   private String m_data;

   /**
    *   Converts the element to a string.
    *   @return The data of this element.
    */
   public final String toString() {
      return expandEntities(m_data);
   }

   /**
    *   Converts the data to a string for printing from the parent.
    *   @return The element as a string indented with level.
    */
   public final String toString(String level) {
      return toString();
   }

   /**
    *   Returns the data of this element.
    *   @return The data of this element.
    */
   public final String getData() {
      return m_data;
   }

   /**
    *   Converts "&" into "&amp;" etc
    */
   private String expandEntities(String s) {
      Utility.debugPrint("--- expandEntities BEGIN ---", 3);
      StringBuffer sBuf = new StringBuffer();
      StringTokenizer st = new StringTokenizer(s, "&", true);
      while (st.hasMoreTokens()) {
         String subString = st.nextToken();

         // All delimiters have length == 1
         if (subString.length() == 1) {
            if (subString.equals("&")) {
               sBuf.append("&amp;");
            }
//              else if (subString.equals("å")) {
//                 sBuf.append("&aring;");
//              }
//              else if (subString.equals("ä")) {
//                 sBuf.append("&auml;");
//              }
//              else if (subString.equals("ö")) {
//                 sBuf.append("&ouml;");
//              }
//              else if (subString.equals("Å")) {
//                 sBuf.append("&Aring;");
//              }
//              else if (subString.equals("Ä")) {
//                 sBuf.append("&Auml;");
//              }
//              else if (subString.equals("Ö")) {
//                 sBuf.append("&Ouml;");
//              }
            else {
               // We should not get here unless we have an unimplemented
               // delimiter in the tokenizer.
               sBuf.append(subString);
            }
         }
         // All tokens with length != 1 should be conserved
         else {
            sBuf.append(subString);
         }
         Utility.debugPrint("--> " + sBuf.toString(), 3);
      }
      Utility.debugPrint("--- expandEntities END ---", 3);      
      return sBuf.toString();
   } // expandEntities
   
}

/**
 *   Class representing the <!DOCTYPE> tag. Will not be parsed.
 */
class SimpleXMLDocumentData extends SimpleXMLDataElement {

   /**
    *   Creates a new <code>SimpleXMLDocumentData</code>.
    *   @param data The document data.
    */
   SimpleXMLDocumentData(String data) {
      super(data);
   }
   
}
