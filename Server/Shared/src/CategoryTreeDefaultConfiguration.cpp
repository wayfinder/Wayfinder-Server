/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CategoryTreeDefaultConfiguration.h"

#include "Properties.h"
#include "File.h"
#include "XMLTool.h"

namespace CategoryTreeUtils {

//#define EXTRA_DBG

DefaultConfiguration* 
loadDefaultConfiguration( const MC2String& defaultConfigPath ) {
   std::auto_ptr< DefaultConfiguration > defaultConf;
   
   // Return NULL if no path is given.
   if ( defaultConfigPath.empty() ) {
      mc2log << warn << "[loadDefaultConfigurations] Path to default " 
         "configuration file for catgory tree is empty." << endl;
      return NULL;
   }

   // Load the xml configuration into a DOM tree
   XercesDOMParser parser;
   parser.setValidationScheme( XercesDOMParser::Val_Auto );
   parser.setIncludeIgnorableWhitespace( false );
   parser.parse( defaultConfigPath.c_str() );
   DOMDocument* root = parser.getDocument();
   if ( root == NULL ) {
      throw MC2Exception( "[loadDefaultConfigurations] Failed to load default "
                  "configuration for categories. File: " + defaultConfigPath );
   }

   // Get the category elements
   MC2String imagePath = Properties::getProperty( "IMAGES_PATH", "./" );
   DOMNodeList *nodeCatList = root->getElementsByTagName( X( "category" ) );
   if ( nodeCatList->getLength() > 0 ) {
      CategoryID categoryId;
      MC2String icon;
      bool ok;

      defaultConf.reset( new DefaultConfiguration() );
      for ( unsigned int j=0 ; j<nodeCatList->getLength() ; ++j) {
         DOMNode* catNode = nodeCatList->item( j );
         ok = XMLTool::getAttribValue( categoryId, "id", catNode );
         if ( ok ) {
            ok = XMLTool::getAttribValue( icon, "icon", catNode );
            icon = ok ? icon : "";

            if ( ok && !icon.empty() ) {
               // Make sure all icons are available on disc
               MC2String fullPath = imagePath + "/" + icon + ".svg";
               if ( !File::fileExist( fullPath ) ) {
                  throw MC2Exception( "[loadDefaultConfigurations] Missing "
                                      "icon " + icon + " for category tree" );
               }

               // Add the category configuration
               defaultConf->addCategory( categoryId, icon );
            }
            
         } else {
            mc2log << error << "[loadDefaultConfigurations] No id for category" 
                   << endl;
         }
      }   
   }
   
   return defaultConf.release();
}


// Implementation of class RegionConfiguration
DefaultConfiguration::DefaultConfiguration() :
      m_defaultCategoryConfig() {
}

DefaultConfiguration::~DefaultConfiguration() {
}

void 
DefaultConfiguration::addCategory( CategoryID categoryId,
                                   const MC2String& icon) {
#ifdef EXTRA_DBG
   mc2dbg/*4*/ << "[DefaultConfiguration] Adding category id:" << categoryId 
               << " icon:" << icon << endl;
#endif
   m_defaultCategoryConfig.insert( make_pair( categoryId, icon  ) );
}

MC2String 
DefaultConfiguration::getCategoryIcon( CategoryID categoryId ) const {
   DefaultCategoryConfigMap::const_iterator it;

   it = m_defaultCategoryConfig.find( categoryId );
   if ( it != m_defaultCategoryConfig.end() ) {
      return (*it).second;
   }

   return "";
}

} // namespace CategoryTreeUtils
