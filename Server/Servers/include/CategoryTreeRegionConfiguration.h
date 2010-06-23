/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CATEGORY_TREE_REGION_CONFIGURATION_H
#define CATEGORY_TREE_REGION_CONFIGURATION_H

#include "config.h"

#include "MC2String.h"
#include "CategoryTree.h"
#include "CategoryRegionID.h"

#include <boost/shared_ptr.hpp>

#include <vector>
#include <map>

namespace CategoryTreeUtils {

/**
 * Contains a region specific configuration for a category tree.
 * A RegionConfiguration object is used together with a 
 * CategoryTree object containing the full tree in order to create a new
 * CategoryTree object for a specific region.
 */
class RegionConfiguration {
private:
   /// Container for category information.
   typedef std::pair< bool, MC2String > CategoryPair;
   /// Map for CategoryID and information about the category.
   typedef std::map< CategoryID, CategoryPair > CategoryConfigMap;
public:
   /// Vector with icon information for the categories.
   typedef std::vector< std::pair <CategoryID, MC2String> > IconInfo;

public:
   /**
    * Creates an empty configuration.
    */
   RegionConfiguration();
   
   /**
    * Destructor 
    */
   virtual ~RegionConfiguration();
   
   /**
    * Adds a category that should be included in the tree.
    */
   void addCategory( CategoryID categoryId, 
                     bool isVisible, 
                     const MC2String& icon);
   
   /**
    * @param category   The category to check for inclusion.
    * @return           Whether this region includes the category.
    */
   bool isCategoryVisible( CategoryID categoryId ) const;

   /**
    * @param category   The category to get the icon for.
    * @return The icon to use for the category in this region.
    *         Empty string if no region configured exist.
    */
   void getCategoryIcons( IconInfo& info ) const;

private:
   /**
    * Map for holding category information
    */
   CategoryConfigMap m_categoryConfig;
   
}; // class RegionConfiguration

// Typedefs for RegionConfigurations 
typedef boost::shared_ptr<RegionConfiguration> RegionConfigSharedPtr;
typedef std::pair< CategoryRegionID, 
                   RegionConfigSharedPtr > CategoryRegionConfiguration;
typedef std::vector< CategoryRegionConfiguration > RegionConfigurations;
 
/**
 * Loads all the region specific settings from one file into different
 * RegionConfiguration objects.
 * @return  A pointer to the configuration for all the regions.
 *          NULL if configuration invalid or missing.
 */
RegionConfigurations* 
loadRegionConfigurations( const MC2String& regionConfigPath );

/**
 * Applies region specific settings to a full category tree, producing 
 * a new tree conforming to the settings.
 *
 * @param tree    The full tree.
 * @param config  Configuration for the region.
 * @throw MC2Exception if some icons aren't available on disc.
 */
CategoryTree* applyRegionConfiguration( const CategoryTree* tree,
                                        const RegionConfiguration& config );
} // namespace CategoryTreeUtils
#endif
