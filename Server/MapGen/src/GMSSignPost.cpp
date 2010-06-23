/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSSignPost.h"
#include "DataBuffer.h"

void
GMSSignPost::save(DataBuffer& dataBuffer) const
{
   dataBuffer.writeNextLong(m_signPostSets.size());
   for ( uint32 i=0; i<m_signPostSets.size(); i++ ){
      m_signPostSets[i].save(dataBuffer);
   }

} // save


void
GMSSignPost::load(DataBuffer& dataBuffer)
{
   uint32 nbrSets = dataBuffer.readNextLong();
   for ( uint32 i=0; i<nbrSets; i++){
      GMSSignPostSet spSet;
      spSet.load(dataBuffer);
      m_signPostSets.push_back(spSet);
   }
} // load

uint32
GMSSignPost::sizeInDataBuffer() const
{
   uint32 size = 4; // number of sign post sets
   // The sign post sets;
   for ( uint32 i=0; i<m_signPostSets.size(); i++){
      size += m_signPostSets[i].sizeInDataBuffer();
   }
   return size;
} // sizeInDataBuffer


GMSSignPostSet& 
GMSSignPost::getOrAddSignPostSet(uint32 index)
{
   if (index < m_signPostSets.size() ){
      return (m_signPostSets[index]);
   }
   else {
      m_signPostSets.resize(index+1);
      MC2_ASSERT(m_signPostSets.size() > index);
      return (m_signPostSets[index]);
   }
} // getOrAddSignPostSet

const vector<GMSSignPostSet>& 
GMSSignPost::getSignPostSets() const
{
   return m_signPostSets;
}


vector<GMSSignPostSet>& 
GMSSignPost::getNonConstSignPostSets()
{
   return m_signPostSets;
}

bool 
GMSSignPost::operator==( const GMSSignPost& other ) const
{
   int32 thisNbrSets = static_cast<int32>(m_signPostSets.size());
   int32 otherNbrSets = static_cast<int32>(other.m_signPostSets.size());
   if ( thisNbrSets != otherNbrSets ){
      // Different number of sing post sets.
      return false;
   }
   
   for (int32 i=0; i<thisNbrSets; i++){
      if ( m_signPostSets[i] != other.m_signPostSets[i] ){
         return false;
      }
   }
   return true;
} // operator==

bool 
GMSSignPost::operator!=( const GMSSignPost& other ) const
{
   return !(*this == other);
} // operator!=


ostream& operator<< ( ostream& stream, const GMSSignPost& signPost )
{
   for (uint32 i=0; i<signPost.m_signPostSets.size(); i++){
      stream << endl << "   spSet[" << i << "] ";
      stream << signPost.m_signPostSets[i];
   }
   return stream;
} // operator<<


ostream& 
GMSSignPost::debugPrint( ostream& stream, 
                         const OldGenericMap* theMap )
{
   for (uint32 i=0; i<m_signPostSets.size(); i++){
      stream << endl << "   spSet[" << i << "] ";
      m_signPostSets[i].debugPrint(stream, 
                                   theMap );
   }
   return stream;
}


bool
GMSSignPost::removeSpElement(const OldGenericMap& theMap,
                             const MC2String& signPostText)
{
   bool result = false;
   for (uint32 i=0; i<m_signPostSets.size(); i++){
      if (m_signPostSets[i].removeSpElement(theMap, signPostText)){
         result = true;
      }
   }
   return result;

} // removeSpElement

bool
GMSSignPost::isEmpty()
{
   for (uint32 i=0; i<m_signPostSets.size(); i++){
      if ( ! m_signPostSets[i].isEmpty() ){
         return false;
      }
   }
   return true;

} // isEmpty


GMSSignPost::const_iterator
GMSSignPost::begin() const {
   return m_signPostSets.begin();
}


GMSSignPost::const_iterator
GMSSignPost::end() const {
   return m_signPostSets.end();
}
