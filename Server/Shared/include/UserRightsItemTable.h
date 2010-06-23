/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USER_RIGHTS_ITEM_TABLE_H
#define USER_RIGHTS_ITEM_TABLE_H

#include "config.h"

#include "DataBufferObject.h"
#include "MapRights.h"
#include "ItemComboTableFwd.h"

#include <map>
#include <set>

class DataBuffer;

/**
 *    Class to store the user rights of items in GenericMap.
 *    Currently has an array of itemids that it binary searches in.
 *    If many Items have user rights that are not the defaults, then
 *    consider storing the itemids like in the GenericMap ( zoom etc. ).
 */
class UserRightsItemTable: public DataBufferObject {
public:
   /// Type of input for the table. Item id in first and URType in second.
   typedef map<uint32, MapRights> itemMap_t;

   /// copy
   UserRightsItemTable( const UserRightsItemTable& other );

   /**
    *   Creates a new table with the supplied default value.
    *   @param defaultVal The value to return if an item isn't found
    *                     in the table. Items with exactly this type will not
    *                     be added to the table.
    */
   UserRightsItemTable( const MapRights& defaultVal  );

   /**
    *   Creates a new table with the supplied default value.
    *   @param defaultVal The value to return if an item isn't found
    *                     in the table. Items with exactly this type will not
    *                     be added to the table.
    *   @param itemsToAdd The items to add.
    */
   UserRightsItemTable( const MapRights& defaultVal,
                        const itemMap_t& itemsToAdd );

   /**
    *    Creates a new table keeping only the ids allowed (if possible).
    */
   UserRightsItemTable( const UserRightsItemTable& original,
                        const set<uint32>& allowedIDs );

   /**
    *   Swaps the contents of this table with the contents of
    *   the other table. This is a very quick operation.
    */
   void swap( UserRightsItemTable& other );
    

   /// Deallocates the memory previously allocated
   virtual ~UserRightsItemTable();

   /// Get user right for item
   const MapRights& getRights( uint32 itemID ) const;

   /// Sets the table to the items in itemsToAdd. 
   /// @return nbr items added
   int setItems( const itemMap_t& itemsToAdd );
   
   /** Get all items in the table
    *  @param itemsToGet Outparameter Is filled whith all items in the 
    *                    table, I.e. not cleared first.
    *  @return Nbr of items in itemsToGet.
    */
   int getItems( itemMap_t& itemsToGet );

   /// Saves the table in the buffer
   void save( DataBuffer& buf ) const;

   /// Loads the table from the buffer
   void load( DataBuffer& buf );

   /// Returns the size of the table in a databuffer
   uint32 getSizeInDataBuffer() const;

   /// Clears the table
   void clear();

   /// The type of storage. Change to larger int if necessary.
   typedef ItemComboTable<MapRights, uint8> table_t;

private:
   friend class M3Creator;

   /// Not implemented
   UserRightsItemTable& operator=( UserRightsItemTable& other );
   
   /// Returns true if the table is empty.
   bool empty() const;
   

   /// The storage
   table_t* m_table;
   
};

/// Dumps the table to a stream. 
ostream& operator<<(ostream& o, UserRightsItemTable& urim);

#endif
