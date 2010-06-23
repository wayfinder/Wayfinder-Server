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

public class SimpleXMLDocument extends SimpleXMLElement {

   /**
    *   Creates a new <code>SimpleXMLDocument</code> with default
    *   declaration and documentype.
    */
   public SimpleXMLDocument() {
      super("document", null, DOCUMENT);
      // Init the default doctype and declaration.
      Hashtable attribs = new Hashtable();
      attribs.put("version", "1.0");
      attribs.put("encoding", "UTF-8");
      setDeclaration(new SimpleXMLDeclaration("?xml", attribs));
      setDocData(
         new SimpleXMLDocumentData(
            "<!DOCTYPE isab-mc2 SYSTEM \"isab-mc2.dtd\">"));
   }

   /**
    *   Creates a new <code>SimpleXMLDocument</code> with default
    *   declaration and documentype.
    *
    *   @param encoding is the encoding to use, such as
    *                   UTF-8 or iso-8859-1
    */
   public SimpleXMLDocument(String encoding) {
      super("document", null, DOCUMENT);
      // Init the default doctype and declaration.
      Hashtable attribs = new Hashtable();
      attribs.put("version", "1.0");
      attribs.put("encoding", encoding);
      setDeclaration(new SimpleXMLDeclaration("?xml", attribs));
      setDocData(
         new SimpleXMLDocumentData(
            "<!DOCTYPE isab-mc2 SYSTEM \"isab-mc2.dtd\">"));
   }

   /**
    *   @return The document as a <code>String</code>.
    */
   public String toString() {
      StringBuffer s = new StringBuffer();
      s.append(m_declaration);
      s.append('\n');
      s.append(m_docData);
      s.append("\n\n");
      if ( m_children != null ) {         
         for(int i=0; i < m_children.size(); ++i ) {
            s.append((SimpleXMLElement)(m_children.elementAt(i)));
         }
      }
      return s.toString();
   }
   
   /**
    *   Sets the declaration tag of the document.
    */
   public void setDeclaration(SimpleXMLDeclaration decl) {
      m_declaration = decl;
   }

   /**
    *   Sets the document data of the document.
    */
   public void setDocData(SimpleXMLDocumentData data) {
      m_docData = data;
   }

   /** The declaration tag for the document */
   private SimpleXMLDeclaration m_declaration;
   /** The doctype tag */
   private SimpleXMLDocumentData m_docData;
}
