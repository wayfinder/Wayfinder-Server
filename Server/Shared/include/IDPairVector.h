/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IDPAIRVECTOR_H
#define IDPAIRVECTOR_H

#include "config.h"
#include <vector>

class Packet;

/**
 *   Pair containing mapID in first and nodeid in second.
 */
class IDPair_t : public pair<uint32,uint32>
{
public:

   /**
    *   Creates a new, invalid IDPair_t.
    */
   IDPair_t() : pair<uint32,uint32>(MAX_UINT32, MAX_UINT32) {};
   
   /**
    *   Creates a new IDPair_t.
    *   @param first mapID.
    *   @param second itemID.
    */
   IDPair_t(uint32 first,
            uint32 second)
         : pair<uint32,uint32>(first, second) {}
   
   static bool less(const IDPair_t& a, const IDPair_t& b) {
      if ( a.first < b.first )
         return true;
      if ( a.first > b.first )
         return false;
      if ( a.second < b.second )
         return true;
      else
         return false;
   }

   bool operator<(const IDPair_t& other) const {
      return less(*this, other);
   }

   /**
    *    Returns the map id.
    *    @return Map ID.
    */
   inline uint32 getMapID() const;

   /**
    *    Returns the item id.
    *    @return ItemID.
    */
   inline uint32 getItemID() const;

   /**
    *    Returns true if the id is valid, i.e. the item and mapids
    *    are not MAX_UINT32.
    */
   inline bool isValid() const;
   
   /**
    *   Print the IDPair_t on an ostream. Will print a dec to the stream.
    *   @param stream The stream to print on.
    *   @param idpair The IDPair_t to print.
    *   @return The stream.
    */
   inline friend ostream& operator<<( ostream& stream,
                                      const IDPair_t& idpair);

   /**
    * Save this IDPair into packet.
    */
   uint32 save( Packet* p, int& pos ) const;

   /**
    * Load from packet. Reads mapID and itemID.
    */
   void load( const Packet* p, int& pos );
};
//typedef pair<uint32,uint32> IDPair_t;

/**
 *   Vector holding mapID in first and nodeID in second of every element.
 */
typedef vector<IDPair_t> IDPairVector_t;

// ----------- Inlined methods ----------------------

inline uint32
IDPair_t::getMapID() const
{
   return first;
}

inline uint32
IDPair_t::getItemID() const
{
   return second;
}

inline bool
IDPair_t::isValid() const
{
   return (getMapID() != MAX_UINT32) &&
           (getItemID() != MAX_UINT32);
}

inline ostream&
operator<<( ostream& stream,
            const IDPair_t& idpair)
{
   return stream << "0x"
                 << hex << idpair.getMapID()
                 << ",0x" << idpair.getItemID() << dec;
}

#endif
