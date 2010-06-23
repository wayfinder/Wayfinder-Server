/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CATEGORIESDATA_H
#define CATEGORIESDATA_H

#include "config.h"
#include "LangTypes.h"
#include "MC2String.h"
#include "ISABThread.h"
#include "CategoryTree.h"
#include "ImageTable.h"
#include "CategoryRegionID.h"

#include <map>

class CategoriesData;

/**
 * Class for holding read CategoriesData data.
 *
 */
class CategoriesDataHolder {
public:
   /**
    * CategoriesDataHolder.
    */
   CategoriesDataHolder();


   /**
    * Destructor.
    */
   virtual ~CategoriesDataHolder();


   /**
    * Make the category.
    * Mutex protected method.
    */
   CategoriesData* makeCategory( const MC2String& categorySet, 
                                 ImageTable::ImageSet imageSet,
                                 const char* clientType,
                                 LangTypes::language_t language, 
                                 bool latin1,
                                 CategoryRegionID catRegionID );
   
   struct CatData {
      CatData( const vector< MC2String >& orgName,
               const vector< MC2String >& translatedName,
               const vector< MC2String >& imageFilename,
               const vector< CategoryTreeUtils::CategoryID >& ids ):
         stringIDs( orgName ),
         names( translatedName ),
         filenames( imageFilename ),
         categoryIDs( ids ) {
      }

               
      vector< MC2String > stringIDs;
      vector< MC2String > names;
      vector< MC2String > filenames;
      vector< CategoryTreeUtils::CategoryID > categoryIDs;
   };


   typedef uint32 POIType;
   /**
    * Get image for search list from poi type, if we don't have an icon
    * for the poi or if the image set doesn't have special icons for the
    * search list "" is returned and the caller should fall back on
    * for instance regular poi icons.
    *
    * @return image name without suffix or "".
    */
   const char* getSearchListImageName( ImageTable::ImageSet imageSet,
                                       POIType poiType ) const;

protected:
   /**
    * Create a new CategoriesData object.
    */
   virtual CategoriesData* createCategoriesData( bool latin1 ) = 0;

private:
   /**
    * Gets the category list in a localized format which CategoriesData
    * subclasses can use to populate their lists.
    *
    * The function first gets the appropriate list of categories
    * (a vector of numbers identifying the categories) and then converts
    * this into translated names, icon filenames etc.
    *
    * @return NULL if there was no appropriate category list.
    */
   CatData* getCatData( const MC2String& categorySet,
                        ImageTable::ImageSet imageSet,
                        const char* clientType,
                        LangTypes::language_t language,
                        bool latin1,
                        CategoryRegionID regionID );

   /**
    * Gets the list of categories for a given category set and region ID.
    * If there is no specific list for the region, the category list for
    * CategoryRegionID::NO_REGION will be returned. If the category set
    * doesn't exist or there is no default list defined this function
    * may return an empty list.
    *
    * @param categorySet The category set.
    * @param regionID The region identifier.
    * @param categories The categories are returned in this vector.
    */
   void getCategories( const MC2String& categorySet,
                       CategoryRegionID regionID,
                       vector<CategoryTreeUtils::CategoryID>& categories );

   /**
    * Reads a category list definition file and inserts the
    * lists into m_categoryLists.
    *
    * @param categorySet The category set to read definition for from file.
    */
   void readCategoryListsFromFile( const MC2String& categorySet );

   /**
    * Checks if it's time to flush the cached category lists.
    *
    * @return Whether it's time to flush.
    */
   bool timeToFlushCategoryLists() const;

   /**
    * Flushes the category list cache so category lists
    * will be read from file again. The reason for flushing is that we
    * should be able to change category lists without restarting the
    * servers.
    */
   void flushCategoryLists();

   /// point of interest type to image name map
   typedef std::map< POIType, const char* > POIImageTable;

   typedef MC2String CategoryListName;
   /// category list name to point of interest table map
   typedef std::map< CategoryListName, POIImageTable > POIImageCategoryTable;
   /// Special POI image list for some clients ( i.e iphone )
   /// in the search result.
   POIImageCategoryTable m_poiImages;

   /**
    * The mutex.
    */
   ISABMutex m_mtx;

   typedef map<MC2String, 
               vector<CategoryTreeUtils::CategoryID> > RegionToCategoryListMap;
   typedef map<MC2String, RegionToCategoryListMap> CategoryListMap;

   /**
    * Cached category lists as read from file, for various
    * category sets this map contains category lists for specific
    * regions.
    */
   CategoryListMap m_categoryLists;

   /**
    * When was m_categoryLists last flushed?
    */
   uint32 m_flushTime;
};

/**
 * Class that loads and holds categories.
 *
 */
class CategoriesData {
   public:
      /**
       * Constructor.
       *
       * @param clientUsesLatin1 If client uses utf-8 or iso-8859-1.
       */
      CategoriesData( bool clientUsesLatin1 );


      /**
       * Destructor.
       */
      virtual ~CategoriesData();


      /**
       * Make categories from ready lists.
       */
      void makeList( const char* clientType, 
                     LangTypes::language_t language,
                     const CategoriesDataHolder::CatData& data );


      /**
       * Get the number of categories.
       */
      uint32 getNbrCategories() const;


      /**
       * Get the crc of the data.
       */
      uint32 getCRC() const;

protected:
      /**
       * The innerMakeList where subclasses do their stuff.
       */
      virtual void innerMakeList( const char* clientType,
                                  LangTypes::language_t language,
                                  const CategoriesDataHolder::CatData& data ) = 0;

      /**
       * The crc of the categories.
       */
      uint32 m_categoriesCRC;


      /**
       * The number of categories.
       */
      uint32 m_nbrCategories;


      /**
       * If client uses latin1.
       */
      bool m_clientUsesLatin1;

};

#endif // NAVCATEGORIESDATA_H

