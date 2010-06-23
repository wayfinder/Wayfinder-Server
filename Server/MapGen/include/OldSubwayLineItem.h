/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDSUBWAYLINEITEM_H
#define OLDSUBWAYLINEITEM_H

#include "OldItem.h"

/**
 *    One subway line on the map.
 *
 */
class OldSubwayLineItem : public OldItem {
   public:
      /**
       *    Default construcor, implemented to be as fast as possible.
       */
      inline OldSubwayLineItem() { };

      /**
       *    Creates an item containing information about a
       *    OldSubwayLineItem.
       *    @param id The localID of this item.
       */
      OldSubwayLineItem(uint32 id);

      /**
       *    Destroys this OldSubwayLineItem.
       */
      virtual ~OldSubwayLineItem();

      /**
       *    Writes the item into the dataBuffer.
       *    @param   dataBuffer Where to print the information.
       *    @return  True if no errors, false otherwise.
       */
      virtual bool save(DataBuffer* dataBuffer) const;

      /**
       *    Writes data about the item into a class variable
       *    and returns a pointer to it. Not thread-safe.
       *    @return The item in string form.
       */
      virtual char* toString();

      /**
       *    Writes the subway line specific header to the mifFile.
       *    @param mifFile  The mif-file to write header to.
       */
      static void writeMifHeader(ofstream& mifFile);

      /**
       *    Writes a subway line item to a mid and mif file.
       *    @param midFile  File to write attributes in.
       *    @param mifFile  File to write gfx-data to.
       *    @param namePointer Pointer to stringtable.
       */
      virtual void printMidMif(ofstream& midFile, ofstream& mifFile,
		               OldItemNames* namePointer);

   protected:
      /**
       *    Fill this item with information from the databuffer.
       */
      virtual bool createFromDataBuffer(DataBuffer* dataBuffer, 
                                        OldGenericMap* theMap);

      /**
       *   Declare OldItem as a friend, to make it possible to
       *   call the createFromDataBuffer-method.
       */
      friend OldItem* OldItem::createNewItem(DataBuffer*, OldGenericMap*);
      friend class GMSItem;
};

#endif

