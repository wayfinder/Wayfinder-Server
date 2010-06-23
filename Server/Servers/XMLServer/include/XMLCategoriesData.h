/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLCATEGORIESDATA_H
#define XMLCATEGORIESDATA_H

#include "config.h"
#include "LangTypes.h"
#include <map>
#include "MC2String.h"
#include "ISABThread.h"
#include "CategoriesData.h"
#include "SearchParserHandler.h"
#include "StackOrHeap.h"
#include <util/PlatformUtils.hpp>

/**
 * Class for a single category in a specific language.
 *
 */
class XMLCategory {
public:
   /**
    * Constructor.
    */
   XMLCategory( const MC2String& name, const MC2String& image, 
                SearchParserHandler::Category::CategoryID id );

   /**
    * Copy constructor.
    */
   XMLCategory( const XMLCategory& other );

   /**
    * Get name in xml format.
    */
   const XMLCh* getName() const;

   /**
    * Get the id.
    */
   SearchParserHandler::Category::CategoryID getID() const;

   /**
    * Get the image name in xml format.
    */
   const XMLCh* getImage() const;

private:
   /// The name of the category in the language.
   StackOrHeap< 256, XMLCh > m_name;

   /// The image name
   StackOrHeap< 256, XMLCh > m_image;

   /// The category tree id.
   SearchParserHandler::Category::CategoryID m_id;
};

/**
 * Class that loads and holds categories for XMLServer.
 *
 */
class XMLCategoriesData : public CategoriesData {
public:
   /**
    * Constructor.
    *
    * @param clientUsesLatin1 If client uses utf-8 or iso-8859-1, not used
    *                         in xml.
    */
   XMLCategoriesData( bool clientUsesLatin1 );

   /**
    * Destructor.
    */
   virtual ~XMLCategoriesData();

   typedef vector< XMLCategory >::const_iterator const_iterator;

   /// Get the first category
   const_iterator begin() const;

   /// Get the end of categories.
   const_iterator end() const;

protected:
   /**
    * Make categories from ready lists.
    */
   virtual void innerMakeList( const char* clientType, 
                               LangTypes::language_t language,
                               const CategoriesDataHolder::CatData& data );

private:
   /**
    * The categories.
    */
   vector< XMLCategory > m_categories;
};


/**
 * Class for holding read XMLCategoriesData data.
 *
 */
class XMLCategoriesDataHolder : public CategoriesDataHolder {
protected:
   /**
    * Create a new CategoriesData object.
    */
   virtual CategoriesData* createCategoriesData( bool latin1 );
};


// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -

inline const XMLCh*
XMLCategory::getName() const {
   return m_name;
}

inline SearchParserHandler::Category::CategoryID
XMLCategory::getID() const {
   return m_id;
}

inline const XMLCh*
XMLCategory::getImage() const {
   return m_image;
}

inline XMLCategoriesData::const_iterator
XMLCategoriesData::begin() const {
   return m_categories.begin();
}

inline XMLCategoriesData::const_iterator
XMLCategoriesData::end() const {
   return m_categories.end();
}

#endif // XMLCATEGORIESDATA_H

