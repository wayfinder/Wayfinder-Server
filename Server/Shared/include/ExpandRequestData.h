/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPAND_REQUEST_DATA_H
#define EXPAND_REQUEST_DATA_H

#include "config.h"
#include "IDPairVector.h"
#include "LangTypes.h"
#include "ItemTypes.h"
#include <set>

class ItemTypes;
class Packet;

/**
 *  Class that contains the data for a ExpandRequest.
 */
class ExpandRequestData {

 public:
   /**
    *  Default constructor.
    */
   ExpandRequestData();

   /**
    *  Constructor.
    *  @param id The mapID and the ItemID.
    *  @param reqLang The requested language.
    *  @param types The requested item types.
    */
   ExpandRequestData( const IDPair_t& id,
                      LangTypes::language_t reqLang,
                      set<ItemTypes::itemType> types);

   /**
    *  Copy constructor.
    *  @param data The ExpandRequestData.
    */
   ExpandRequestData( const ExpandRequestData& data );

   /**
    *  Destructor.
    */
   ~ExpandRequestData();

   /**
    *  Saves the data into a packet.
    *  @param p The packet where the data is saved.
    *  @param pos The position.
    */
   void save( Packet* p, int& pos ) const;

   /**
    *  Loads the data from a packet.
    *  @param p Packet from where the data is loaded.
    *  @param pos The position.
    */
   void load( const Packet* p, int& pos );

   /**
    *  Returns the size of the data in the packet.
    *  @return The data size in the packet.
    */
   int getSizeInPacket() const;

   /**
    *  Returns the pair with the mapID and itemID.
    *  @return The IDPair, i.e. the mapID and itemID.
    */
   IDPair_t getIDPair() const;

   /** Returns the requested language.
    *  @return The requested language.
    */
   LangTypes::language_t getLanguage() const;

   /**
    *  Returns the set with the requested item types.
    *  @return The set with requested item types.
    */
   set<ItemTypes::itemType> getItemTypes() const;
   
  private:
   // The pair with mapID and itemID.
   IDPair_t m_id;

   // The requested language
   LangTypes::language_t m_reqLang;

   // The set with requested item types.
   set<ItemTypes::itemType> m_itemTypes;
};

#endif
