/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDWATERITEM_H
#define OLDWATERITEM_H

#include "config.h"
#include "OldItem.h"

/**
  *   Describes one piece of water on the map.
  *
  */
class OldWaterItem : public OldItem {
   public:
      /**
        *   Writes the waterItem specific header to the mifFile.
        *   @param mifFile   The mif-file to write header to.
        */
      static void writeMifHeader(ofstream& mifFile);
   
      /**
       *    Default construcor, implemented to be as fast as possible.
       */
      inline OldWaterItem() { };

      /**
        *   Creates an item containing information about a
        *   OldWaterItem.
        *
        *   @param   id The localID of this item
        */
      OldWaterItem(uint32 id);

      /**
        *   Destroys the OldStreetSegmentItem.
        */
      virtual ~OldWaterItem();

      /**
        *   Writes the item into the dataBuffer.
        *   @param   dataBuffer Where to print the information.
        *   @return  True if no errors occurred, false otherwise.
        */
      virtual bool save(DataBuffer *dataBuffer) const;

      /**
       *   Writes data about the item into a class variable
       *   and returns a pointer to it. Not thread-safe
       *   @return The item in string form
       */
      virtual char *toString();

      /**
        *  Writes a water item to a mid and mif file.
        *  @param  mifFile File to write gfx-data to.
        *  @param  midFile File to write attributes in.
        *  @param  namePointer Pointer to stringtable.
        */
      virtual void printMidMif(ofstream& midFile, ofstream& mifFile, OldItemNames* namePointer); 

      /**
        *   @name Methods to get and set the water type attribute.
        */
      //@{
         /**
           *   Set the water type.
           *   @param   type  The water type.
           */
         inline void setWaterType(byte type);

         /**
           *   Get the water type.
           *   @return The water type.
           */
         inline byte getWaterType() const;
      //@}

      /**
       *    Virtual method that updates all attributes of item and,
       *    if any, its nodes and connections to the values of the 
       *    other item. This includes e.g. names, groups, entry 
       *    restrictions, sign posts, speed limit and gfxdata.
       *    For more documentation see OldItem.h.
       *
       *    @param otherItem  The item from which to get update values.
       *    @param sameMap    Default true meaning that the items are
       *                      one and the same (same map), set to false if
       *                      the items originates from different maps (e.g.
       *                      underview and overview map)
       *    @return  True if some attribute was updated for the item,
       *             false if not.
       */
      virtual bool updateAttributesFromItem(OldItem* otherItem,
                                            bool sameMap = true);
   
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
       *    Water type.
       */
      byte m_waterType;

      /**
       *    Declaire OldItem as a friend, to make it possible to
       *    call the createFromDataBuffer-methd.
       */
      friend OldItem* OldItem::createNewItem(DataBuffer*, OldGenericMap*);
      friend class GMSItem;

};

// ========================================================================
//                                      Implementation of inlined methods =

inline void 
OldWaterItem::setWaterType(byte type) 
{
   MC2_ASSERT(type < MAX_BYTE);
   m_waterType = type;
}

inline byte 
OldWaterItem::getWaterType() const 
{
   return (m_waterType);
}

#endif

