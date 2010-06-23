/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSSignPostSet.h"
#include "DataBuffer.h"

void
GMSSignPostSet::save(DataBuffer& dataBuffer) const
{
   dataBuffer.writeNextLong(m_signPostElements.size());
   for ( uint32 i=0; i<m_signPostElements.size(); i++ ){
      m_signPostElements[i].save(dataBuffer);
   }
} // save


void
GMSSignPostSet::load(DataBuffer& dataBuffer)
{
   uint32 nbrElements = dataBuffer.readNextLong();
   for ( uint32 i=0; i<nbrElements; i++){
      GMSSignPostElm spElement;
      spElement.load(dataBuffer);
      m_signPostElements.push_back(spElement);
   }
} // load

uint32
GMSSignPostSet::sizeInDataBuffer() const
{
   return 
      4 + // number of sign post elements
      // the sign post elements
      GMSSignPostElm::sizeInDataBuffer() * m_signPostElements.size(); 
} // sizeInDataBuffer

GMSSignPostElm& 
GMSSignPostSet::getOrAddSignPostElm(uint32 index)
{
   if (index < m_signPostElements.size() ){
      return m_signPostElements[index];
   }
   else {
      m_signPostElements.resize(index+1);
      MC2_ASSERT(m_signPostElements.size() > index);
      return m_signPostElements[index];
   }
} // getOrAddSignPostSet


const vector<GMSSignPostElm>& 
GMSSignPostSet::getSignPostElements() const
{
   return m_signPostElements;
} // getSignPostElements

vector<GMSSignPostElm>& 
GMSSignPostSet::getNonConstSignPostElements()
{
   return m_signPostElements;
} // getSignPostElements


bool 
GMSSignPostSet::operator==( const GMSSignPostSet& other ) const
{
   int32 thisNbrElements = static_cast<int32>(m_signPostElements.size());
   int32 otherNbrElements =static_cast<int32>(other.m_signPostElements.size());
   if ( thisNbrElements != otherNbrElements ){
      // Different number of sign post elements.
      return false;
   }
   for (int32 i=0; i<thisNbrElements; i++){
      if ( m_signPostElements[i] != other.m_signPostElements[i] ){
         return false;
      }
   }
   return true;
} // operator==

bool 
GMSSignPostSet::operator!=( const GMSSignPostSet& other ) const
{
   return !(*this == other);
} // operator!=

ostream& operator<< ( ostream& stream, const GMSSignPostSet& spSet )
{
   for (uint32 i=0; i<spSet.m_signPostElements.size(); i++){
      stream << endl << "      spElm[" << i << "] ";
      if ( spSet.m_signPostElements[i].isSet() ){
         stream << spSet.m_signPostElements[i];
      }
      else {
         stream << "[NOT SET]";
      }
   }
   return stream;

} // operator<<

ostream& 
GMSSignPostSet::debugPrint( ostream& stream, 
                         const OldGenericMap* theMap )
{
   for (uint32 i=0; i<m_signPostElements.size(); i++){
      stream << endl << "   spElm[" << i << "] ";
      if ( m_signPostElements[i].isSet() ){
         m_signPostElements[i].debugPrint(stream, 
                                          theMap );
      }
      else {
         stream << "[NOT SET]";         
      }
   }
   return stream;
}

bool
GMSSignPostSet::removeSpElement(const OldGenericMap& theMap,
                                const MC2String& signPostText)
{
   bool result = false;
   vector<GMSSignPostElm> newSignPostElm;
   for (vector<GMSSignPostElm>::iterator it = m_signPostElements.begin();
        it != m_signPostElements.end(); ++it ){
      MC2String elmText = it->getTextString(theMap);
      if ( elmText != signPostText ){
         newSignPostElm.push_back(*it);
      }
      else {
         result = true;
      }
   }
   m_signPostElements = newSignPostElm;
   return result;
   
} // removeSpElement


bool
GMSSignPostSet::isEmpty() {
   return m_signPostElements.empty();
}


GMSSignPostSet::const_iterator
GMSSignPostSet::begin() const {
   return m_signPostElements.begin();
}


GMSSignPostSet::const_iterator
GMSSignPostSet::end() const {
   return m_signPostElements.end();
}

GMSSignPostSet::iterator
GMSSignPostSet::begin() {
   return m_signPostElements.begin();
}

GMSSignPostSet::iterator
GMSSignPostSet::end() {
   return m_signPostElements.end();
}
