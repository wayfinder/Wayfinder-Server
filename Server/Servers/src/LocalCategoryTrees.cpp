/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "LocalCategoryTrees.h"

#include "CategoryTree.h"
#include "STLUtility.h"
#include "MC2Exception.h"
#include "CategoryTreeRegionConfiguration.h"

namespace CategoryTreeUtils {

/// The hidden implementation
struct LocalCategoryTrees::Implementation {

   Implementation( const MC2String& fullTreePath,
                   const MC2String& defaultConfigPath,
                   const MC2String& regionConfigPath )
         : m_fullTreePath( fullTreePath ),
           m_defaultConfigPath ( defaultConfigPath ),
           m_regionConfigPath( regionConfigPath ) {
      load();
   }

   void load() {
      m_fullTree.reset( new CategoryTree );
      m_localTrees.clear();

      if ( !m_fullTree->load( m_fullTreePath, m_defaultConfigPath ) ) {
         throw MC2Exception( "Failed to load full category tree" );
      }

      // load the region configurations
      std::auto_ptr<RegionConfigurations> configs(
         loadRegionConfigurations( m_regionConfigPath ) );

      if ( configs.get() == NULL ) {
         throw MC2Exception( 
            "Failed to read category tree region configuration file" );
      }

      // create the local trees
      for ( RegionConfigurations::iterator itr = configs->begin();
            itr != configs->end(); ++itr ) {
         RegionConfigSharedPtr config = (*itr).second;

         CategoryTreePtr localTree(
            applyRegionConfiguration( m_fullTree.get(),*config ) );

         m_localTrees[ (*itr).first ] = localTree;
      }

      if ( !STLUtility::has( m_localTrees, CategoryRegionID::NO_REGION ) ) {
         throw MC2Exception(
            "No default category tree region configuration" );
      }
   }

   /// The full category tree.
   CategoryTreePtr m_fullTree;

   /// The region specific trees.
   std::map< CategoryRegionID, CategoryTreePtr > m_localTrees;

   MC2String m_fullTreePath;     ///< Path to full tree
   MC2String m_defaultConfigPath;///< Path to the default config file
   MC2String m_regionConfigPath; ///< Path to region config
};

LocalCategoryTrees::LocalCategoryTrees( 
   const MC2String& fullTreePath,
   const MC2String& defaultConfigPath,
   const MC2String& regionConfigPath )
      : m_pimpl( new Implementation( fullTreePath,
                                     defaultConfigPath,
                                     regionConfigPath ) ) {
}

CategoryTreePtr
LocalCategoryTrees::getTree( CategoryRegionID catRegionID ) const {
   if ( STLUtility::has( m_pimpl->m_localTrees, catRegionID ) ) {
      return m_pimpl->m_localTrees[ catRegionID ];
   } else {
      return m_pimpl->m_localTrees[ CategoryRegionID::NO_REGION ];
   }
}

CategoryTreePtr
LocalCategoryTrees::getFullTree() const {
   return m_pimpl->m_fullTree;
}

}
