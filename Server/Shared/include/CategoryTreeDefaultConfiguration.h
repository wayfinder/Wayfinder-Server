/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CATEGORY_TREE_DEFAULT_CONFIGURATION_H
#define CATEGORY_TREE_DEFAULT_CONFIGURATION_H

#include "config.h"

#include "MC2String.h"
#include "CategoryTree.h"

namespace CategoryTreeUtils {

/**
 * Contains a default configuration for the category tree e.g. default icons.
 */
class DefaultConfiguration {

private:
   /// Map for default configuration.
typedef std::map< CategoryID, MC2String > DefaultCategoryConfigMap;

public:
   /**
    * Creates an empty configuration.
    */
   DefaultConfiguration();

   /**
    * Destructor 
    */
   virtual ~DefaultConfiguration();

   /**
    * Adds a default configuration for a category.
    * @param categoryId The category id
    * @param icon       The default icon
    */
   void addCategory( CategoryID categoryId,
                     const MC2String& icon);
   /**
    * Returns the default icon for category
    * @param category Requested category id.
    * @return         The icon name. Empty sring if no icon exist.
    */
   MC2String getCategoryIcon( CategoryID categoryId ) const;

private:
   /**
    * Default configuration for each category.
    */
   DefaultCategoryConfigMap m_defaultCategoryConfig;
};

/**
 * Loads the default configuration for local category tree from file.
 * @param defaulConfigPath Path to the config file
 * @return                 Pointer to an DefaulConfiguration object.
 *                         NULL if the load failed.
 */
DefaultConfiguration*
loadDefaultConfiguration( const MC2String& defaultConfigPath );

} // namespace CategoryTreeUtils

#endif // CATEGORY_TREE_DEFAULT_CONFIGURATION_H
