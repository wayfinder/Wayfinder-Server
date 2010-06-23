/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDCATEGORYITEM_H
#define OLDCATEGORYITEM_H

#include "config.h"
#include "OldGroupItem.h"


/**
  *   OldCategoryItem {\it Documentation!!!}.
  *
  */
class OldCategoryItem : public OldGroupItem {
   public:
      /**
       *    Default construcor, implemented to be as fast as possible.
       */
      inline OldCategoryItem() { };

      /**
        *   Creates an item containing information about a
        *   category. This constructor extracts (parses)
        *   the information from the dataBuffer.
        *
        *   @param   id The localID of this item
        */
      OldCategoryItem(uint32 id);

      /**
        *   Destroys the OldCategoryItem.
        */
      virtual ~OldCategoryItem();

      /**
        *   Writes the item into the dataBuffer.
        *   @param   dataBuffer Where to print the information.
        *   @return  True if no errors occurred, false otherwise.
        */
      virtual bool save(DataBuffer* dataBuffer) const;

      /**
       *   Writes data about the item into a class variable
       *   and returns a pointer to it. Not thread-safe
       *   @return The item in string form
       */
      virtual char* toString();

   protected:
      /**
       *    Fill this item with information from the databuffer.
       *    @param   dataBuffer  The buffer with the data.
       *    @return  True if the data of the item is set, false
       *             otherwise.
       */
      virtual bool createFromDataBuffer(DataBuffer* dataBuffer, 
                                        OldGenericMap* theMap);

      /**
       *    Declaire OldItem as a friend, to make it possible to
       *    call the createFromDataBuffer-methd.
       */
      friend OldItem* OldItem::createNewItem(DataBuffer*, OldGenericMap*);
      friend class GMSItem;

};

// ========================================================================
//                                      Implementation of inlined methods =

#endif

