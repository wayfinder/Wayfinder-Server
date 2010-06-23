/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NationalAddressFormatter.h"

namespace {
/**
 * Different types of zip codes
 */
enum zipCodeType_t {
   // These zip codes can be merged by removing an arbritary number
   // of numbers from the end and check for equality.
   symmetricNumberZipCodeType,

   // The type of zip codes used in United Kingdom
   ukZipCodeType,

   // Symmetric zip code type with zip area name separated with ", ". 
   //  E.g. "2253, Stripfing".
   numberCommaAreaZipCodeType,

   // Symmetric zip code type with zip area name separated with "-". 
   //  E.g. "00144-ROMA".
   numberDashAreaZipCodeType,

   // Symmetric zip code type with zip area name separated with " ". 
   //  E.g. "28070 MADRID".
   numberSpaceAreaZipCodeType,

   // Zip code type with a name followed by a number, as
   //  used in, for example, Ireland. E.g. "Dublin12".
   numberNameZipCodeType
};

/**
 * Different types of address layout
 */
enum addressType_t {
   // The number first
   // E.g. "3 Stamford Street"
   numberSpaceNameType,
   
   // The name first
   // E.g. "Isbergs gata 3"
   nameSpaceNumberType,

   // The number first followed by comma then name
   // E.g "30, Rue Ahmed Orabi"
   numberCommaNameType
};

addressType_t getAddressType( uint32 topRegion ) {
   
   // Source: http://www.bitboost.com/ref/international-address-formats.html 
   //         http://www.zipcodeworld.com/addressing/egypt.htm
   
   addressType_t addressType;
   
   switch( topRegion ){
      case 65: // Spain (ES)          
      case 70: // Portugal (PT)      
      case 77: // Greece (GR)        
      case 7:  // Netherlands (NL)   
      case 67: // Italy (IT)         
      case 2:  // Germany (DE)       
         addressType = nameSpaceNumberType;
         break;
         
      case 0:   // United Kingdom (GB)
      case 61:  // Ireland (IE)
      case 245: // South Africa (ZAF)
           addressType = numberSpaceNameType;
           break;

      case 138: // Egypt (EGY)
         addressType = numberCommaNameType;
           
      default:
         addressType = nameSpaceNumberType;
         // TODO: Warn about unhandled top region 
         break;
   }
   
   return addressType;
}

zipCodeType_t getZipCodeType( uint32 topRegion ) {
   
   // Source: http://www.bitboost.com/ref/international-address-formats.html 
   //         http://www.zipcodeworld.com/addressing/egypt.htm
   
   zipCodeType_t zipCodeType;
   
   switch( topRegion ){
      case 65: // Spain (ES)          
      case 70: // Portugal (PT)       
      case 77: // Greece (GR)         
      case 7:  // Netherlands (NL)    
      case 2:  // Germany (DE)        
         zipCodeType = numberSpaceAreaZipCodeType;
         break;

      case 67: // Italy (IT)          
         zipCodeType = numberDashAreaZipCodeType;
         break;
         
      case 0:   // United Kingdom (GB)
      case 245: // South Africa (ZAF) 
      case 138: // Egypt (EGY)
         zipCodeType = ukZipCodeType;
         break;
         
      case 61: // Ireland (IE)        
         zipCodeType = numberNameZipCodeType;
         break;
         
      default:
         zipCodeType = numberSpaceAreaZipCodeType;
         // TODO: Warn about unhandled top region
         break;
   }
   
   return zipCodeType;
}

}

namespace NationalAddressFormatter {

MC2String formatAddress( uint32 topRegion, MC2String streetName, 
                         MC2String houseNbr ) {
   
   MC2String formattedAddress;
   switch ( getAddressType( topRegion ) ) {
      case nameSpaceNumberType:
         formattedAddress = streetName + " " + houseNbr;
         break;
      case numberSpaceNameType:
         formattedAddress = houseNbr + " " + streetName;
         break;
      case numberCommaNameType:
         formattedAddress = houseNbr + ", " + streetName;
         break;         
   }

   return formattedAddress;
}

MC2String formatZip( uint32 topRegion, MC2String zipCode, MC2String zipArea ) {

   MC2String formattedZip;

   switch ( getZipCodeType( topRegion ) ) {
      case symmetricNumberZipCodeType:
         formattedZip = zipCode + " " + zipArea; 
         break;
      case ukZipCodeType:
         formattedZip = zipArea + " " + zipCode; 
         break;
      case numberCommaAreaZipCodeType:
         formattedZip = zipCode + ", " + zipArea; 
         break;
      case numberDashAreaZipCodeType:
         formattedZip = zipCode + "-" + zipArea; 
         break;
      case numberSpaceAreaZipCodeType:
         formattedZip = zipCode + " " + zipArea; 
         break;
      case numberNameZipCodeType:
         formattedZip = zipArea + zipCode; 
         break;
   }

   return formattedZip;         
}

}
