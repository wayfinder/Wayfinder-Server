/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "DataBuffer.h"
#include "UserRightsItemTable.h"
#include "ItemComboTable.h"
#include "Packet.h"

UserRightsItemTable::UserRightsItemTable( const UserRightsItemTable& other ) {
   m_table = new table_t( *other.m_table );
} 

UserRightsItemTable::
UserRightsItemTable( const MapRights& defaultVal  )
{
   m_table = new table_t( defaultVal, itemMap_t() );
}

int
UserRightsItemTable::getItems( itemMap_t& itemsToGet ){

   return m_table->getAllData( itemsToGet );

} // getItems


UserRightsItemTable::UserRightsItemTable( const UserRightsItemTable& original,
                                          const set<uint32>& allowedIDs )
{
   m_table = new table_t( *original.m_table,
                          allowedIDs );
}


UserRightsItemTable::UserRightsItemTable( const MapRights& defaultVal,
                                          const itemMap_t& itemsToAdd )
{
   m_table = new table_t( defaultVal, itemsToAdd );
   for ( itemMap_t::const_iterator it = itemsToAdd.begin();
         it != itemsToAdd.end();
         ++it ) {
      MC2_ASSERT( getRights(it->first) == it->second );
   }
}

UserRightsItemTable::~UserRightsItemTable()
{
   delete m_table;
}

const MapRights&
UserRightsItemTable::getRights( uint32 itemID ) const
{
   return m_table->getData( itemID );
}

void
UserRightsItemTable::clear()
{
   m_table->clear();
}

uint32
UserRightsItemTable::getSizeInDataBuffer() const
{
   return m_table->getSizeInDataBuffer();
}

void
UserRightsItemTable::save( DataBuffer& buf ) const
{
   m_table->save( buf );
}

void
UserRightsItemTable::load( DataBuffer& buf )
{
   m_table->load( buf );
}

void
UserRightsItemTable::swap( UserRightsItemTable& other )
{
   std::swap( other.m_table, m_table );
}

ostream& operator<<(ostream& o, UserRightsItemTable& urim)
{
   UserRightsItemTable::itemMap_t m;
   urim.getItems(m);
   return o << hex << STLUtility::co_dump(m) << dec << endl;
}
