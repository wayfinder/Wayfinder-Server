/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INFOTYPECONVERTER_H
#define INFOTYPECONVERTER_H

#include "config.h"
#include "ItemInfoEnums.h"
#include "MC2String.h"

#include <map>

/**
 * Converts between info type, string and additional info type.
 */
class InfoTypeConverter {
public:
   InfoTypeConverter();
   /**
    * Converts string to InfoType.
    *
    * @param type The InfoType to convert.
    * @return The InfoType for the string or ItemInfoEnums::more if
    *         not known string.
    */
   ItemInfoEnums::InfoType strToInfoType( const char* str ) const;
   /**
    * Converts InfoType to string to use in XML document.
    *
    * @param type The InfoType to convert.
    * @return The string representation of type.
    */
   const char* infoTypeToStr( ItemInfoEnums::InfoType type ) const;
   
   /**
    * Converts from MC2 to Nav2.
    *
    * @param type The MC2 type.
    * @return The Nav2 type.
    */
   uint16 infoTypeToAdditionalInfoType( ItemInfoEnums::InfoType type );


   /**
    * Converts from Nav2 to MC2.
    *
    * @param type The Nav2 type.
    * @return The MC2 type.
    */
   ItemInfoEnums::InfoType additionalInfoTypeToInfoType( uint16 type );   

private:

   /**
    * Struct holding a ItemInfoEnums::InfoType, the corresponding
    * string and AdditionalInfoType.
    */
   struct infoTypeString_t {
      ItemInfoEnums::InfoType type;
      MC2String str;
      uint16 addType;
   };
      

   /// The ItemInfoEnums::InfoType and their strings.
   static const struct infoTypeString_t m_infoTypes[];


   typedef map< ItemInfoEnums::InfoType, size_t > infoTypeMap;
   /// The m_infoTypes by InfoType.
   infoTypeMap m_infoTypeMap;


   typedef map< MC2String, size_t > infoStringMap;
   /// The m_infoTypes by string.
   infoStringMap m_infoStringMap;
   
   typedef map< uint16, size_t > infoAddTypeMap;
   ///The m_infoTypes by additional info type
   infoAddTypeMap m_infoAddTypeMap;
};

#endif // INFOTYPECONVERTER_H
