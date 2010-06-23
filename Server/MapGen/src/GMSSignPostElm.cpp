/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSSignPostElm.h"
#include "DataBuffer.h"
#include "OldGenericMap.h"

const GMSSignPostElm::ElementClass GMSSignPostElm::INVALID_CLASS = ~0;

GMSSignPostElm::GMSSignPostElm(){
   m_text = MAX_UINT32;
   m_elementClass = INVALID_CLASS;
   m_elementType = invalidElementType;
   m_ambigousInfo = false;
   m_connectionType = unsetConnectionType;
}

bool
GMSSignPostElm::isSet() const{
   return ( (m_text != MAX_UINT32 ) ||
            (m_elementClass != INVALID_CLASS) ||
            (m_elementType != invalidElementType) ||
            (m_ambigousInfo != false) ||
            (m_connectionType != unsetConnectionType) );
}

void
GMSSignPostElm::setTextStringCode(uint32 stringCode)
{
   m_text = stringCode;
} // setTextStringCode


void 
GMSSignPostElm::setType(elementType_t type)
{
   MC2_ASSERT(type < MAX_BYTE);
   m_elementType = type;
} // setElementType

GMSSignPostElm::elementType_t 
GMSSignPostElm::getType() const 
{
   return m_elementType;
} // getType


GMSSignPostElm::ElementClass
GMSSignPostElm::getElementClass() const
{
   return m_elementClass;
}

void
GMSSignPostElm::setClass(ElementClass elementClass)
{
   m_elementClass = elementClass;
} // setClass;

void 
GMSSignPostElm::setConnectionType(GMSSignPostElm::connectionType_t
                                  connectionType)
{
   m_connectionType = connectionType;

} // setConnectionType


void
GMSSignPostElm::setAmbigousInfo(bool ambInfoValue){
   m_ambigousInfo = ambInfoValue;
} // setAmbigousInfo

void
GMSSignPostElm::save(DataBuffer& dataBuffer) const
{
   dataBuffer.writeNextLong(m_text);
   dataBuffer.writeNextLong(m_elementClass);
   dataBuffer.writeNextByte(m_elementType);
   dataBuffer.writeNextBool(m_ambigousInfo);
   dataBuffer.writeNextByte(m_connectionType);
   dataBuffer.writeNextByte( 0 ); // for align
} // save


void
GMSSignPostElm::load(DataBuffer& dataBuffer)
{
   m_text = dataBuffer.readNextLong();
   m_elementClass = dataBuffer.readNextLong();
   m_elementType = static_cast<elementType_t>(dataBuffer.readNextByte());
   m_ambigousInfo = dataBuffer.readNextBool();
   m_connectionType = 
      static_cast<connectionType_t>(dataBuffer.readNextByte());
   dataBuffer.readNextByte(); 
} // load

uint32
GMSSignPostElm::sizeInDataBuffer()
{
   return
      4 + // m_text
      4 + // m_elementClass
      1 + // m_elementType
      1 + // m_ambigousInfo
      1 + // m_connectionType
      1;  // for align
} // sizeInDataBuffer

ostream& operator<< ( ostream& stream, 
                      const GMSSignPostElm& spElm ){
   return stream 
      << "m_text:" << spElm.m_text 
      << ", m_elementClass:" << spElm.m_elementClass 
      << ", m_elementType:" << spElm.m_elementType 
      << ", m_ambigousInfo:" << spElm.m_ambigousInfo 
      << ", m_connectionType:" << spElm.m_connectionType;
} // operator<<

ostream& 
GMSSignPostElm::debugPrint( ostream& stream, 
                     const OldGenericMap* theMap )
{   
   stream 
      << "text: (" << m_text << ") \"" << getTextString(*theMap) << "\""
      << ", m_elementClass:" << m_elementClass 
      << ", m_elementType:" << m_elementType 
      << ", m_ambigousInfo:" << m_ambigousInfo 
      << ", m_connectionType:" << m_connectionType;
   return stream;
} // operator<<


bool 
GMSSignPostElm::operator==( const GMSSignPostElm& other ) const
{
   return ( ( other.m_text == m_text ) &&
            ( other.m_elementClass == m_elementClass ) && 
            ( other.m_elementType == m_elementType ) &&
            ( other.m_ambigousInfo == m_ambigousInfo ) &&
            ( other.m_connectionType == m_connectionType ) );

} // operator==

bool 
GMSSignPostElm::operator!=( const GMSSignPostElm& other ) const
{
   return !(*this == other);
} // operator!=


uint32* 
GMSSignPostElm::getTextStringCodePointer()
{
   return &m_text;
} // getTextStringCodePointer


MC2String 
GMSSignPostElm::getTextString(const OldGenericMap& theMap) const 
{
   const char* name = theMap.getName(m_text);
   if ( name != NULL ){
      return name;
   }
   else {
      return "";
   }
   
} // getText


LangTypes::language_t 
GMSSignPostElm::getTextLang(const OldGenericMap& theMap) const
{
   return static_cast<LangTypes::language_t>(GET_STRING_LANGUAGE(m_text));
} // getTextLang

ItemTypes::name_t 
GMSSignPostElm::getTextNameType(const OldGenericMap& theMap) const
{
   return static_cast<ItemTypes::name_t>(GET_STRING_TYPE(m_text));
} // getTextNameType

