/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDINDIVIDUALBUILDINGITEM_H
#define OLDINDIVIDUALBUILDINGITEM_H

#include "OldItem.h"
#include "ItemSubTypes.h"

/**
 *    One individual building on the map. That is an item where the 
 *    GfxData contains the actual outline of the building.
 *
 */
class OldIndividualBuildingItem : public OldItem {
   public:
      /**
       *    Default construcor, implemented to be as fast as possible.
       */
      inline OldIndividualBuildingItem() { };

      /**
       *    Creates an item containing information about a
       *    OldIndividualBuildingItem.
       *    @param id The localID of this item.
       */
      OldIndividualBuildingItem(uint32 id);

      /**
       *    Destroys this IndividualBuildningItem.
       */
      virtual ~OldIndividualBuildingItem();

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
       *    Writes the individual building specific header to the mifFile.
       *    @param mifFile  The mif-file to write header to.
       */
      static void writeMifHeader(ofstream& mifFile);

      /**
       *    Writes a individual building item to a mid and mif file.
       *    @param midFile  File to write attributes in.
       *    @param mifFile  File to write gfx-data to.
       *    @param namePointer Pointer to stringtable.
       */
      virtual void printMidMif(ofstream& midFile, ofstream& mifFile,
		               OldItemNames* namePointer);

      /**
        *   @name Methods to get and set the building type attribute.
        */
      //@{
         /**
           *   Set the building type.
           *   @param   type  The building type.
           */
         inline void setBuildingType(
               ItemSubTypes::individualBuildingType_t type);
  
         /**
           *   Get the building type. 
           *   @return The building type.
           */
         inline ItemSubTypes::individualBuildingType_t getBuildingType() const;
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
      
      static MC2String buildingTypeToString(
            ItemSubTypes::individualBuildingType_t buildingType);

      static ItemSubTypes::individualBuildingType_t
            stringToBuildingType(MC2String typeString);


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

   private:
      /**
        *   The type of this individual building.
        */
      ItemSubTypes::individualBuildingType_t m_buildingType;
};

// =======================================================================
//                                     Implementation of inlined methods =

inline void
OldIndividualBuildingItem::setBuildingType(
            ItemSubTypes::individualBuildingType_t type)
{
      m_buildingType = type;
}

inline ItemSubTypes::individualBuildingType_t
OldIndividualBuildingItem::getBuildingType() const
{
      return (m_buildingType);
}

#endif

