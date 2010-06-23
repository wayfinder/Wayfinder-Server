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

import java.util.Vector;
import java.util.Hashtable;
import java.util.Enumeration;

/**
 *   Class describing an element in an XML document.
 */
public class SimpleXMLElement {

   public static final int DOCUMENT    = 1;
   public static final int ELEMENT     = 2;
   public static final int DATA        = 3;
   public static final int DECLARATION = 4;

   /**
    *   Creates a new <code>SimpleXMLElement</code>.
    *   Type will be set to element.
    *   @param name       The name of the element.
    */
   SimpleXMLElement( String name ) {
      init(name,
           (Hashtable)null,
           ELEMENT,
           "/");
   }
   
   /**
    *   Creates a new <code>SimpleXMLElement</code>.
    *   Type will be set to element.
    *   @param name       The name of the element.
    *   @param attributes The attributes of the element.
    */
   SimpleXMLElement(String name,
                    Hashtable attributes) {
      init(name,
           attributes,
           ELEMENT,
           "/");
   }

   /**
    *   Creates a new <code>SimpleXMLElement</code>.
    *   Type will be set to element.
    *   @param name       The name of the element.
    *   @param attributes The attributes of the element.
    *   @param children   Children of the element.
    */
   SimpleXMLElement(String name,
                    String [][] attributes,
                    SimpleXMLElement [] children) {
      init(name,
           attributes,
           ELEMENT,
           "/");
      if ( children != null ) {
         m_children = new Vector(children.length);
         for(int i=0; i < children.length; ++i ) {
            m_children.addElement(children[i]);
         }
      }
   }

   /**
    *   Creates a new <code>SimpleXMLElement</code>.
    *   Type will be set to element.
    *   @param name       The name of the element.
    *   @param attributes The attributes of the element.
    *   @param children   Children of the element.
    *   @param dummy      Makes this contructor unambiguous.
    */
   SimpleXMLElement(String name,
                    Hashtable attributes,
                    SimpleXMLElement [] children,
                    int dummy) {
      init(name,
           attributes,
           ELEMENT,
           "/");
      if ( children != null ) {
         m_children = new Vector(children.length);
         for(int i=0; i < children.length; ++i ) {
            m_children.addElement(children[i]);
         }
      }
   }
   
   /**
    *   Creates a new <code>SimpleXMLElement</code>. Useful for adding
    *   simple children with just names.
    *   Type will be set to element.
    *   @param name       The name of the element.
    *   @param attributes The attributes of the element.
    *   @param childrenNames The names of children tags to be added.    
    */
   SimpleXMLElement(String name,
                    String [][] attributes,
                    String [] childrenNames) {
      init(name,
           attributes,
           ELEMENT,
           "/");      
      if ( childrenNames != null ) {
         int nbr = childrenNames.length;
         m_children = new Vector(nbr);
         for(int i=0; i < nbr; ++i ) {
            m_children.addElement(new SimpleXMLElement(childrenNames[i]));
         }
      }
   }

   
   /**
    *   Creates a new <code>SimpleXMLElement</code>.
    *   Type will be set to element.
    *   @param name       The name of the element.
    *   @param attributes The attributes of the element. Key plus value.
    *   @param data       The data of the element (may be null).
    *   @param dummy      Dummy parameter so this constructor will not
    *                     clash with the one with children.
    */
   SimpleXMLElement(String name,
                    String [][] attributes,
                    String data,
                    int dummy) {   
      init(name,
           attributes,
           ELEMENT,
           "/");
      if ( data != null && data.length() != 0 ) {
         m_data = new SimpleXMLDataElement(data);
         m_children = new Vector(1);
         m_children.addElement(m_data);
      }
   }
   
   /**
    *   Creates a new element. This constructor is used by the subclasses.
    *   @param name       The "name" of the element.
    *   @param attributes The attributes of the element.
    *   @param type       The type of element.
    */
   SimpleXMLElement(String name,
                    Hashtable attributes,
                    int type) {
      init(name,
           attributes,
           type,
           "/");
   }

   /**  
    *   Creates a new <code>SimpleXMLElement</code>.
    *   @param name       The name of the tag.
    *   @param attributes The attributes of the tag.
    *   @param type       The type of the tag.
    *   @param endChar    The character for the end tag.
    */
   SimpleXMLElement(String name,
                    Hashtable attributes,
                    int type,
                    String endChar) {
      init(name,
           attributes,
           type,
           endChar);
   }

   /**
    *   Helper function for the constructors.
    *   @param name       The name of the tag.
    *   @param attributes The attributes of the tag.
    *   @param type       The type of the tag.
    *   @param endChar    The character for the end tag.
    */
   private void init(String name,
                     Hashtable attributes,
                     int type,
                     String endChar) {
      m_name     = name;
      m_attribs  = attributes;
      m_type     = type;
      m_children = null;
      m_data     = null;
      m_endChar  = endChar;      
   }

   /**
    *   Helper function for the constructors.
    *   @param name       The name of the tag.
    *   @param attributes The attributes of the tag.
    *   @param type       The type of the tag.
    *   @param endChar    The character for the end tag.
    */
   private void init(String name,
                     String [][] attributes,
                     int type,
                     String endChar) {
      Hashtable attribHash = null;
      if (attributes != null) {
         attribHash = new Hashtable();
         try {
            for(int i=0; i < attributes.length; i++ ) {
               if (attributes[i][1] != null) {
                  attribHash.put(attributes[i][0], attributes[i][1]);
               }
            }
         } catch ( Exception e ) {
            Utility.debugPrint("Exception in SimpleXMLElement:init: " +
                               e, 2);
            attribHash = null;
         }
      }
      init ( name, attribHash, type, endChar);
   }
   

   /**
    *   Adds a child containing only a name to the parent.
    *   A new SimpleXMLElement will be created inside.
    *   @param name The name of the child to add.
    */
   public void addChild(String name) throws SimpleXMLException {
      addChild( new SimpleXMLElement(name) );
   }
   
   /**
    *   Adds a child to this element. 
    *   If data is set a SimpleXMLException will be thrown.
    *   @param child The child to add.
    */
   public void addChild(SimpleXMLElement child) throws SimpleXMLException {
      if ( m_data != null ) {
         throw new SimpleXMLException("Node <"
                                      + child.getName()
                                      + "> added when data \""
                                      + m_data + "\" exists in <"
                                      + m_name + ">");
      }
      if ( m_children == null )
         m_children = new Vector();      
      m_children.addElement(child);
   }

   /**
    *   Adds the array of children to this element.
    *   @param children Array of children to add.
    */
   public void addChildren(SimpleXMLElement [] children )
      throws SimpleXMLException{
      for(int i=0; i < children.length; ++i )
         addChild(children[i]);
   }
   
   /**
    *   Sets the data of this element.
    *   If nodes exist a SimpleXMLException will be thrown.
    */
   public void setData(SimpleXMLDataElement data) throws SimpleXMLException {
      if ( m_children != null && m_children.size() != 0 )
         throw new SimpleXMLException("Data \""
                                      + data 
                                      + "\" added when node(s) "
                                      + "exist in <"
                                      + m_name + ">");
      m_data = data;
      m_children = new Vector();
      m_children.addElement(data);
   }

   /**
    *   Sets the data for this element.
    *   If nodes exist a SimpleXMLException will be thrown.
    */
   public void setData(String data) throws SimpleXMLException {
      setData( new SimpleXMLDataElement(data) );
   }

   /**
    *   @return The data if there is any. Else <code>null</code>.
    */
   public String getData() {

        if (m_data != null) {
         return m_data.getData();
          } else {
             return null;
          }
   }

   /**
    *   @return The name of this element(tag).
    */
   public final String getName() {
      return m_name;
   }

   /**
    *   @return The value of the attribute or
    *           <code>null</code> if not found.
    */
   public final String getAttrib(String attName) {
      if ( m_attribs != null ) {
         return (String)(m_attribs.get(attName));
      } else {
         return null;
      }
   }

   /**
    *    Returns the number of children of this element.
    *    @return The number of children of this element.
    */
   public int getNbrChildren() {
      if ( m_children == null )
         return 0;
      else
         return m_children.size();
   }

   /**
    *    Returns the child at position i.
    *    @param i The position of the child.
    */
   public SimpleXMLElement getChildAt(int i) {
      return (SimpleXMLElement)(m_children.elementAt(i));
   }

   /**
    *   @return A SimpleXMLElement with the tag matching <code>name</code>
    *           if exactly one matching tag exists, null otherwise.
    *           
    */
   public SimpleXMLElement getSingleTag(String name) {
      Vector v = new Vector();
      getTags(name, v);
      SimpleXMLElement e = null;
      if (v.size() == 1) {
         e = (SimpleXMLElement)(v.elementAt(0));
      } else {
         // Not exactly one element, so return null
      }
      return e;
   }
   
   /**
    *   @return A SimpleXMLElement with the tag matching <code>name</code>
    *           if exactly one matching child tag exists, null otherwise.
    *
    *   Nice method. Nonrecursive.           
    */
   public SimpleXMLElement getSingleChildTag(String name) {
      Vector v = new Vector();
      getChildTags(name, v);
      SimpleXMLElement e = null;
      if (v.size() == 1) {
         e = (SimpleXMLElement)(v.elementAt(0));
      } else {
         // Not exactly one element, so return null
      }
      return e;
   }
   
   /**
    *   @return A vector with the tags matching <code>name</code> from
    *           this tag downward in the tree. NB! Is probably slow.
    */
   public Vector getTags(String name) {
      Vector v = new Vector();
      getTags(name, v);
      return v;
   }

   /**
    *   Adds all the tags from this node downward matching name.
    *   @param name The name to match.
    *   @param v    The vector to add it to.
    */
   protected void getTags(String name, Vector v) {
      if ( m_name.equals(name) ) {
         v.addElement(this);
      }
      if ( m_children != null ) {
         for( int i=0; i < m_children.size(); i++ ) {
            SimpleXMLElement child =
               (SimpleXMLElement)(m_children.elementAt(i));
            child.getTags(name, v);
         }
      }
   }

   /**
    *   @return A vector with the tags matching <code>name</code> one step
    *           down from this tag in the tree. NB! Is probably slow.
    *
    *   Nice method. Nonrecursive.
    */
   public Vector getChildTags(String name) {
      Vector v = new Vector();
      getChildTags(name, v);
      return v;
   }

   /**
    *   Adds all the tags from the children of this node matching name.
    *   If name is null, all children will be matched
    *
    *   @param name The name to match. Null, when getting all children.
    *   @param v    The vector to add it to.
    *
    *   Nice method. Nonrecursive.
    */
   protected void getChildTags(String name, Vector v) {
      if ( m_children != null ) {
         for( int i=0; i < m_children.size(); i++ ) {
            SimpleXMLElement child =
               (SimpleXMLElement)(m_children.elementAt(i));
            // Add child only if name matches, or name == null
            if (child == null) {
            }
            else if ((child.getName()).equals(name) ||
                     (name == null)) {
               v.addElement(child);
            }
         }
      }
   }

   /**
    *   @return This element and its contents as a string.
    */
   public String toString() {
      return toString("");
   }
   
   /**
    *   @return This element and its contents as a string.
    */
   public String toStringDbg() {
      return toStringDbg("");
   }
   
   /**
    *   @param  level Indentation string. Should be spaces.
    *   @return This element and its contents as a string.    
    */
   public String toString(String level) {      
      StringBuffer s = new StringBuffer();
      s.append(level);
      s.append("<");
      s.append(m_name);
      if ( m_attribs != null && ! m_attribs.isEmpty() ) {
         Enumeration keys = m_attribs.keys();
         Enumeration values = m_attribs.elements();
         while ( keys.hasMoreElements() ) {
            s.append(" ");
            s.append(keys.nextElement());
            s.append("=\"");
            s.append(values.nextElement());
            s.append('"');
         }
      }
      if ( m_children != null ) {
         s.append(">");
         if ( m_data == null )
            // Data on the same line
            s.append("\n");
         String nextLevel = level + "  ";
         for(int i=0; i < m_children.size(); ++i ) {
            SimpleXMLElement
               child = (SimpleXMLElement)(m_children.elementAt(i));
            s.append(child.toString(nextLevel));
         }
         if ( m_data == null )
            s.append(level);
         s.append("<");
         s.append(m_endChar);
         s.append(m_name);
         s.append(">\n");
      } else {
         s.append(m_endChar);
         s.append(">\n");
      }
      return s.toString();
   }

      /**
    *   @param  level Indentation string. Should be spaces.
    *   @return This element and its contents as a string.    
    */
   public String toStringDbg(String level) {      
      StringBuffer s = new StringBuffer();
      s.append(level);
      s.append("<");
      s.append(m_name);
      if ( m_attribs != null && ! m_attribs.isEmpty() ) {
         Enumeration keys = m_attribs.keys();
         Enumeration values = m_attribs.elements();
         while ( keys.hasMoreElements() ) {
            s.append("\n");
            s.append(keys.nextElement());
            s.append(" == ");
            s.append(values.nextElement());
         }
         s.append("\n");
      }
      if ( m_children != null ) {
         s.append(">");
         if ( m_data == null )
            // Data on the same line
            s.append("\n");
         String nextLevel = level + "  ";
         for(int i=0; i < m_children.size(); ++i ) {
            SimpleXMLElement
               child = (SimpleXMLElement)(m_children.elementAt(i));
            s.append(child.toStringDbg(nextLevel));
         }
         if ( m_data == null )
            s.append(level);
         s.append("<");
         s.append(m_endChar);
         s.append(m_name);
         s.append(">\n");
      } else {
         s.append(m_endChar);
         s.append(">\n");
      }
      return s.toString();
   }
   
   protected Vector               m_children;
   private   SimpleXMLDataElement m_data;
   private   String               m_name;
   private   Hashtable            m_attribs;
   private   int                  m_type;
   /** The end-tag character. Usually slash, but sometimes questionmark */
   private   String               m_endChar;
}

/**
 *   Element that holds a declaration.
 */
class SimpleXMLDeclaration extends SimpleXMLElement {

   /**
    *   Creates a new declaration with the supplied parameters.
    *   @name       The name of the tag. Probably xml.
    *   @attributes The attributes of the tag.
    */
   SimpleXMLDeclaration(String name,
                        Hashtable attributes) {
      super(name, attributes, DECLARATION, "?");
   }
   
   /**
    *   @return This element and its contents as a string.    
    */
   public String toString() {      
      StringBuffer s = new StringBuffer();

      s.append("<?xml");
      
      // get version
      String version = getAttrib("version");
      if (version != null) {
         s.append(" version=\"");
         s.append(version);
         s.append('"');
      }
      // get encoding
      String encoding = getAttrib("encoding");
      if (encoding != null) {
         s.append(" encoding=\"");
         s.append(encoding);
         s.append('"');
      }

      s.append("?>");
         
      return s.toString();
   }
}




