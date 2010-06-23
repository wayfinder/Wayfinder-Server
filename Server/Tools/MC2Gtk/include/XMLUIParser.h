/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2GTK_XMLUI_UIPARSER_H
#define MC2GTK_XMLUI_UIPARSER_H

#include "config.h"
#include "MC2String.h"
#include "NotCopyable.h"
#include "Exception.h"

#include <dom/DOM.hpp>

#if (XERCES_VERSION_MAJOR == 2 && XERCES_VERSION_MINOR >= 4) || (XERCES_VERSION_MAJOR > 2)
using namespace xercesc;
#endif

#include <gtk/gtk.h>
#include <vector>
#include <map>

/// contains MC2 Gtk classes
namespace MC2Gtk {

/// contains XML user interface classes
namespace XMLUI {

/// exception thrown by XMLUI elements
class Exception: public MC2Gtk::Exception {
public:
   /// @param what description of the exception
   Exception( const MC2String& what ) throw():
      MC2Gtk::Exception( what ) { }
   ~Exception() throw() { }
};

class Parser;

/// parses xml file and elements and creates a Gtk view
class UIParser: private NotCopyable {
public:
   
   typedef std::vector<GtkWidget*> WidgetVector;

   /// @return UIParser instance
   static UIParser& instance();

   /**
    * @param parser the parser to register
    * @param tagName the tag that the parser handles
    */
   static void registerParser( Parser* parser, const char* tagName );

   /**
    * Finds parser that has name and calls the parser.
    * @param name the name of the element
    * @param element the dom element
    */
   static GtkWidget* parseElement( const MC2String& name,
                                   const DOMElement* element );

   /**
    * Parses a file and returns vector of widgets
    * @param filename the file to load
    * @return vector of widgets that was created
    */
   static WidgetVector parseFile( const MC2String& filename );
   /**
    * Parses data and returns vector of widgets
    * @param data contains xml document in text format
    * @return vector of widgets that was created
    */
   static WidgetVector parseFromData( const MC2String& data );

   /// @return widget that has id, will throw exception on error
   static GtkWidget* getWidget( const MC2String& id ) throw (Exception) ;

   /// @return widget that has id else NULL
   static GtkWidget* findWidget( const MC2String& id ) throw();

   /// @return vector of all id strings
   static std::vector<MC2String> getWidgetIDs();

private:
   UIParser();
   /// implemented in XMLAutoRegister.cpp
   void autoRegister();

   static void registerID( GtkWidget* widget, const MC2String& name );

   typedef std::map<MC2String, Parser*> Parsers;
   typedef std::map<MC2String, GtkWidget*> WidgetIDs;
   
   Parsers m_parsers; ///< all parsers
   WidgetIDs m_widgets; ///< widgets that has "id" attribute
   static UIParser s_instance;
};

/**
 * Will go through the entire tree and parse all children.
 * @param firstChild the first child to parse
 * @return vector of widgets that the children parsed.
 */
UIParser::WidgetVector parseChildren( const DOMElement* firstChild );

}

}

#endif
