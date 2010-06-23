/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMMAP_H
#define ITEMMAP_H

#include "config.h"
#include <map>
#include <set>
#include "DataBuffer.h"
#include "MC2String.h"
#include "GMSLane.h"
/**
 * Use this class to store item attributes as tables in the Map. 
 *
 * To get the data of a specific item, use find with item ID as the key. An 
 * ItemMap<Data>::iterator or ItemMap<Data>::const_iterator is returned. 
 * Check that it differs from ItemMap::end().
 * 
 * When inserting data for an item, call ItemMap::insert with a pair storing
 * item ID in first and the data in second.
 * 
 * This class is currently implemented for data of the types:
 * - uint32
 * - MC2String
 * - set<uint16>
 * - vector<GMSLane>
 * - bool
 * 
 * Implement the methods loadOneItem saveOneItem and oneItemDBufSize when you 
 * need to support a new type of Data.
 *
 */
template<class Data>
class ItemMap: public map<uint32, Data> {

 public:

   /**
    * Loads the item map from a data buffer.
    * @param dBuf The data buffer to load from.
    */
   void load(DataBuffer& dBuf){
      uint32 nbrItems = dBuf.readNextLong();
      for ( uint32 i=0; i< nbrItems; i++){
         pair<uint32, Data> itemPair;
         loadOneItem(dBuf, itemPair.first, itemPair.second);
         insert(itemPair);
      }
      if ( this->size() != nbrItems ){
         mc2log << error << "ItemMap::load size() != nbrItems." << endl;
         MC2_ASSERT(false);
      }
   };

   /**
    * Saves the item map to a data buffer.
    * @param dBuf The data buffer to save to.
    */
   void save(DataBuffer& dBuf){
      dBuf.writeNextLong( this->size() );
      typename ItemMap<Data>::const_iterator it = this->begin();
      while ( it != this->end() ){
         saveOneItem(dBuf, it->first, it->second);
         ++it;
      }
   };

   /**
    * Tells the size needed in a data buffer for saving this item map.
    */
   uint32 getSizeInDataBuffer(){
      uint32 result = 0;
      result += 4; // nbrItems;
      typename ItemMap<Data>::const_iterator it = this->begin();
      while ( it != this->end() ){
         result += 4; // item ID
         result += itemDataDBufSize(it->second);
         ++it;
      }
      return result;
   }


   /**
    * @name Implementations of methods needed for template type 
    *       Data == uint32
    */
   //@{
   void loadOneItem( DataBuffer& dBuf, uint32& itemID, uint32& itemData ){
      itemID = dBuf.readNextLong();
      itemData = dBuf.readNextLong();
   };

   void saveOneItem(DataBuffer& dBuf, uint32 itemID, uint32 itemData) const {
      dBuf.writeNextLong( itemID );
      dBuf.writeNextLong( itemData );
   };

   uint32 itemDataDBufSize( uint32 itemData ) const {
      return 4; 
   };
   //@}   


   /**
    * @name Implementations of methods needed for template type 
    *       Data == bool
    */
   //@{
   void loadOneItem( DataBuffer& dBuf, uint32& itemID, bool& itemData ){
      itemID = dBuf.readNextLong();
      itemData = dBuf.readNextBool();
   };

   void saveOneItem(DataBuffer& dBuf, uint32 itemID, bool itemData) const {
      dBuf.writeNextLong( itemID );
      dBuf.writeNextBool( itemData );
   };

   uint32 itemDataDBufSize( bool itemData ) const {
      return 1 + 3; // bool + worst possilbe alignment padding.
   };

   //@}   

   /**
    * @name Implementations of methods needed for template type 
    *       Data == MC2String
    */
   //@{
   void loadOneItem( DataBuffer& dBuf, uint32& itemID, MC2String& itemData ){
      itemID = dBuf.readNextLong();
      itemData = dBuf.readNextString();
   };

   void saveOneItem(DataBuffer& dBuf, 
                    uint32 itemID, const MC2String& itemData) const {
      dBuf.writeNextLong( itemID );
      dBuf.writeNextString( itemData.c_str() );
   };

   uint32 itemDataDBufSize( MC2String itemData ) const {
      return itemData.size() + 1 + 3; // string + null byte + 
      // worst possible alignment padding.
   };
   //@}   


   /**
    * @name Implementations of methods needed for template type 
    *       Data == set<uint16>
    */
   //@{
   void loadOneItem( DataBuffer& dBuf, uint32& itemID, set<uint16>& itemData ){
      itemID = dBuf.readNextLong();

      uint32 nbrInSet = dBuf.readNextLong();
      for (uint32 i = 0; i< nbrInSet; i++ ){
         uint16 theShort = dBuf.readNextShort();
         itemData.insert(theShort);
      }
   };

   void saveOneItem(DataBuffer& dBuf, 
                    uint32 itemID, const set<uint16>& itemData) const {
      dBuf.writeNextLong( itemID );
      
      dBuf.writeNextLong( itemData.size() );
      for (set<uint16>::const_iterator it = itemData.begin(); 
           it != itemData.end(); ++it ){
         dBuf.writeNextShort( *it );
      }
   };

   uint32 itemDataDBufSize( const set<uint16>& itemData ) const {
      return 
         4 +                 // nbr in set
         itemData.size()*2 + // short set data
         2;                  // worst possible alignment padding.
   };
   //@}   




   /**
    * @name Implementations of methods needed for template type 
    *       Key == Node ID (uint32)
    *       Data == vector<GMSLane>
    */
   //@{
   void loadOneItem( DataBuffer& dBuf, uint32& nodeID, 
                     vector<GMSLane>& nodeData ){
      nodeID = dBuf.readNextLong();

      uint32 nbrInVector = dBuf.readNextLong();
      for (uint32 i = 0; i< nbrInVector; i++ ){
         GMSLane gmsLane;
         gmsLane.load(dBuf);
         nodeData.push_back(gmsLane);
      }
   };

   void saveOneItem(DataBuffer& dBuf, 
                    uint32 nodeID, const vector<GMSLane>& nodeData) const {
      dBuf.writeNextLong( nodeID );
      
      dBuf.writeNextLong( nodeData.size() );
      for (uint32 i=0; i<nodeData.size(); i++){
         nodeData[i].save(dBuf);
      }
   };

   uint32 itemDataDBufSize( const vector<GMSLane>& nodeData ) const {
      
      return
         4 +                // nbr in vector
         nodeData.size() * GMSLane::sizeInDataBuffer()  // vector content
         ;
   };
   //@}   


}; // class ItemMap

#endif //ITEMMAP_H

