/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVCATEGORIESDATA_H
#define NAVCATEGORIESDATA_H

#include "config.h"
#include "LangTypes.h"
#include <map>
#include "MC2String.h"
#include "ISABThread.h"
#include "CategoriesData.h"

/**
 * Class that loads and holds categories.
 *
 */
class NavCategoriesData : public CategoriesData {
public:
   /**
    * Constructor.
    *
    * @param clientUsesLatin1 If client uses utf-8 or iso-8859-1.
    */
   NavCategoriesData( bool clientUsesLatin1 );


   /**
    * Destructor.
    */
   virtual ~NavCategoriesData();

   /**
    * Get the categories.
    */
   const byte* getCategories() const;


   /**
    * Get the length of categories.
    */
   uint32 getCategoriesLength() const;


   /**
    * Get the categories as Nav2prot data, the length of the data, and
    * the number of categories.
    * @param categories Pointer that will be updated to point to the data.
    * @param length     Out-parameter that will hold the length of the data.
    * @param nbr        Out-parameter that will hold the 
    *                   number of categories
    */ 
   void getCategories( const byte*& categories, 
                       uint32& length, uint32& nbr ) const;
   /**
    * Get the categories as Nav2prot data, the length of the data.
    * @param filenames will point to filenames data 
    * @param length will contain the filenames data length
    */
   void getCategoriesFilenames( const byte*& filenames,
                                uint32& length ) const;
   void getCategoryIDs( const byte*& categoryids,
                        uint32& length )  const;

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
   byte* m_categories;


   /**
    * The length of categories.
    */
   uint32 m_categoriesLength;

   /// filenames as bulk data
   byte* m_filenamesData;
   /// length of filenames bulk data
   uint32 m_filenamesDataLength;

   /// categories as a bulk data
   vector<byte> m_categoryIDs;
};

/**
 * Class for holding read NavCategoriesData data.
 *
 */
class NavCategoriesDataHolder : public CategoriesDataHolder {
protected:
   /**
    * Create a new CategoriesData object.
    */
   virtual CategoriesData* createCategoriesData( bool latin1 );
};



#endif // NAVCATEGORIESDATA_H

