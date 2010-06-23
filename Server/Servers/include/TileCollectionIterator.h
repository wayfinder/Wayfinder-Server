/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILECOLLECTIONITERATOR_H
#define TILECOLLECTIONITERATOR_H


#include "TileMapParams.h"
#include "LangTypes.h"

#include <iosfwd>

class TileCollectionNotice;

/**
 * Iterates through TileCollectionNotice and
 * generates TileMapParams with operator *
 */
class TileCollectionIterator: 
   public std::iterator< std::input_iterator_tag,  TileMapParams > {

public:
   /// copies the other and sets firstImportanceOnly
   TileCollectionIterator( const TileCollectionIterator& other, 
                           bool firstImportanceOnly );
   /**
    * Assumes the notices are not empty.
    * @param notices the collection to iterate through
    * @param serverPrefix prefix to use for TileMapParams
    * @param gzip passed to TileMapParams
    * @param lang language to use for tileMapStrings
    */
   TileCollectionIterator( const TileCollectionNotice& notices,
                           uint32 serverPrefix,
                           bool gzip, 
                           LangTypes::language_t lang,
                           bool firstImportanceOnly = false );

   /**
    *  make one step
    * @return reference to this
    */
   TileCollectionIterator& operator ++();
   /// creates a TileMapParams from current point
   TileMapParams operator*() const;
   /// set the iterator to start position
   void goToStart();
   /// set the iterator to end position
   void goToEnd();
   /// compares two iterators
   bool operator == ( const TileCollectionIterator& other ) const;
   /// compares two iterators
   bool operator != ( const TileCollectionIterator& other ) const { 
      return ! ( *this == other ); 
   }

   /// @return language
   LangTypes::language_t getLanguage() const { return m_lang; }
   /// @return server prefix
   uint32 getServerPrefix() const { return m_serverPrefix; }
   /// @return gzip option
   bool getUseGzip() const { return m_gzip; }

   friend std::ostream& operator << ( std::ostream& out, const TileCollectionIterator& it ); 
private:
   /// @return true if the iterator reached end
   bool end() const;

   TileCollectionIterator();

   /// the collection to iterate through
   const TileCollectionNotice* m_notices; 

   
   int m_latIdx; //< lat index
   int m_lonIdx; //< lon index
   int m_layerIdx; //< layer index
   uint32 m_detailIdx; //< detail index
   uint32 m_tileIdx; //< tile index

   int m_endLatIdx; //< end of lat index
   int m_endLonIdx; //< end of lon index

   int m_importanceNbr; //< current importance number

   uint32 m_serverPrefix;//< server prefix
   bool m_gzip; //< if the param is gziped
   LangTypes::language_t m_lang; //< language

   bool m_data; //< data/string type iterator
   /// if the iterator should iterate through first importance only
   bool m_firstImportanceOnly;
};

#endif 
