/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SECTIONPAIR_H
#define SECTIONPAIR_H

#include "config.h"
#include<utility>

/**
 *   Pair containing dataset in first and section in second.
 */
class SectionPair : public pair<uint32,uint32>
{
public:

   SectionPair(uint32 first, uint32 second)
         : pair<uint32,uint32>(first, second) {}

   SectionPair()
         : pair<uint32,uint32>() {}

   static bool less(const SectionPair& a, const SectionPair& b) {
      if ( a.first < b.first )
         return true;
      if ( a.first > b.first )
         return false;
      if ( a.second < b.second )
         return true;
      else
         return false;
   }

   bool operator<(const SectionPair& other) const {
      return less(*this, other);
   }


   bool operator!=(const SectionPair& other) const {
      return (first != other.first ) || (second != other.second);
   }

   /**
    *    Returns the dataset.
    *    @return Dataset.
    */
   inline uint32 getDataset() const;

   /**
    *    Returns the section.
    *    @return Section.
    */
   inline uint32 getSection() const;

   /**
    *   Print the SectionPair on an ostream. Will print a dec to the stream.
    *   @param stream The stream to print on.
    *   @param idpair The SectionPair to print.
    *   @return The stream.
    */
   inline friend ostream& operator<<( ostream& stream,
                                      const SectionPair& sectionPair );
};


// ----------- Inlined methods ----------------------

inline uint32
SectionPair::getDataset() const
{
   return first;
}

inline uint32
SectionPair::getSection() const
{
   return second;
}

inline ostream&
operator<<( ostream& stream,
            const SectionPair& sectionPair ) 
{
   return stream << sectionPair.getDataset() 
                 << ", " << sectionPair.getSection();
}

#endif
