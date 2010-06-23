/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CategoryTreeRegionConfiguration.h"
#include "Properties.h"
#include "XMLTool.h"
#include "File.h"

namespace CategoryTreeUtils {

//#define EXTRA_DBG

RegionConfigurations* 
loadRegionConfigurations( const MC2String& regionConfigPath ) {
   std::auto_ptr< RegionConfigurations > regionConfs( 
      new RegionConfigurations() );

   // Load the xml configuration into a DOM tree
   XercesDOMParser parser;
   parser.setValidationScheme( XercesDOMParser::Val_Auto );
   parser.setIncludeIgnorableWhitespace( false );

   parser.parse( regionConfigPath.c_str() );
   DOMDocument* root = parser.getDocument();
   if ( root == NULL ) {
      mc2log << "[loadRegionConfigurations] Failed to load region "
         "configuration for categories" << endl;
      return NULL;
   }

   // Loop all regions. Create a RegionConfiguration object for each
   DOMNodeList *nodeRegionList = ( root )->getElementsByTagName( X( "region" ) );
   CategoryRegionID region_id;
   for ( unsigned int i=0 ; i<nodeRegionList->getLength() ; ++i) {
      DOMNode* node = nodeRegionList->item( i );
      uint32 countryCode;
      bool ok = XMLTool::getAttribValue( countryCode, "region_id", node );
      if ( !ok ) { // Use default configuration
         region_id = CategoryRegionID::NO_REGION;
         mc2dbg4 << "[loadRegionConfigurations] Adding default region "
            "configuration for the category tree" << endl; 
      } else {
         region_id = CategoryRegionID( 
            static_cast<StringTable::countryCode>( countryCode ) );
         mc2dbg4 << "[loadRegionConfigurations] Adding region configuration "
            "(region_id:" << region_id.toString() << ") for the category tree" 
                << endl;
      }
      
      // Get the category elements
      DOMNodeList *nodeCatList = dynamic_cast< DOMElement* >
         ( node )->getElementsByTagName( X( "category" ) );
      if ( nodeCatList->getLength() > 0 ) {
         CategoryID categoryId;
         bool isVisible;
         MC2String icon;
         bool ok;

         RegionConfigSharedPtr regionConf( 
            new RegionConfiguration() );
         for ( unsigned int j=0 ; j<nodeCatList->getLength() ; ++j) {
            DOMNode* catNode = nodeCatList->item( j );
            ok = XMLTool::getAttribValue( categoryId, "id", catNode );
            if ( ok ) {
               ok = XMLTool::getAttribValue( isVisible, "isVisible", catNode );
               isVisible = ok ? isVisible : false;

               ok = XMLTool::getAttribValue( icon, "icon", catNode );
               icon = ok ? icon : "";

               // Add the category configuration
               regionConf->addCategory( categoryId, isVisible, icon );

            } else {
               mc2log << error << "[loadRegionConfigurations] No id for category"
                      << endl;
            }
         }
         // Push the regional configuration
         regionConfs->push_back( make_pair( region_id, regionConf ) );
      }
   }
   return regionConfs.release();
}

// Implementation of class RegionConfiguration
RegionConfiguration::RegionConfiguration() :
      m_categoryConfig() {
}

RegionConfiguration::~RegionConfiguration() {
}

void 
RegionConfiguration::addCategory( CategoryID categoryId,
                                  bool isVisible, 
                                  const MC2String& icon) {
#ifdef EXTRA_DBG
   mc2dbg/*4*/ << "[RegionConfiguration] Adding category id:" << categoryId 
               << " isVisible:" << (isVisible ? "true" : "false")
               << " icon:" << icon << endl;
#endif
   m_categoryConfig.insert( make_pair( categoryId, 
                                       make_pair( isVisible, icon ) ) );
}

bool
RegionConfiguration::isCategoryVisible( CategoryID categoryId ) const {
   CategoryConfigMap::const_iterator it = m_categoryConfig.find( categoryId );
   if ( it != m_categoryConfig.end() ) {
      return (*it).second.first;
   }
   
   return false;
}

void
RegionConfiguration::getCategoryIcons( IconInfo& info ) const {
   CategoryConfigMap::const_iterator it;
   for ( it = m_categoryConfig.begin( ) ; it != m_categoryConfig.end() ; ++it) {
      MC2String icon( ( *it ).second.second );
      if ( !icon.empty() ) {
         info.push_back( make_pair( ( *it ).first, icon ) );
      }
   }
}

/**
 * Used to decide which categories to keep when calling 
 * CategoryTree::removeCategories. Basically just a wrapper around
 * RegionConfiguration.
 */
class RegionConfigPredicate : public CategoryTree::InclusionPredicate {
public:
   RegionConfigPredicate( const RegionConfiguration& config ) : 
         m_config( config ) {}

   bool operator()( CategoryID id ) const {
      return m_config.isCategoryVisible( id );
   }

private:
   const RegionConfiguration& m_config;
};

CategoryTree* applyRegionConfiguration( const CategoryTree* tree,
                                        const RegionConfiguration& config ) {
   std::auto_ptr<CategoryTree> result( new CategoryTree( *tree ) );
   result->removeCategories( RegionConfigPredicate( config ) );

   // Set the region specific icons
   RegionConfiguration::IconInfo info;
   RegionConfiguration::IconInfo::const_iterator it;

   config.getCategoryIcons( info );

   // check that all icons are available on disc
   MC2String imagePath = Properties::getProperty( "IMAGES_PATH",
                                                  "./" );

   for ( it = info.begin(); it != info.end(); ++it ) {
      MC2String fullPath = imagePath + "/" + it->second + ".svg";
      if ( !File::fileExist( fullPath ) ) {
         throw MC2Exception( "Missing icon " + it->second + 
                             " for region configuration" );
      }
   }
   
   for ( it = info.begin() ; it != info.end() ; ++it ) {
      result->setIcon( it->first, it->second );
   }

   return result.release();
}

} // namespace CategoryTreeUtil
