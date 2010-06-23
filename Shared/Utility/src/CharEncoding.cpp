/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CharEncoding.h"

#include "StringUtility.h"
#include <algorithm>
#include "config.h"

//map<CharEncodingType::charEncodingType, MC2String> 
//CharEncoding::m_encTypeToEncString;

const char* 
CharEncoding::m_iconvEnodingTypeString[] = { 
   "invalid charencoding", //   CharEncodingType::invalidEncodingType
   "UTF-8",       //   CharEncodingType::UTF8
   "UTF-16BE",    //   CharEncodingType::UTF16_be
   "UTF-16LE",    //   CharEncodingType::UTF16_le 
   "UCS-2BE",     //   CharEncodingType::UCS2_be
   "UCS-2LE",     //   CharEncodingType::UCS2_le
   "UCS-4BE",     //   CharEncodingType::UCS4_be
   "UCS-4LE",     //   CharEncodingType::UCS4_le
   "ISO-8859-1",  //   CharEncodingType::iso8859_1
   "ISO-8859-2",  //   CharEncodingType::iso8859_2
   "ISO-8859-3",  //   CharEncodingType::iso8859_3
   "ISO-8859-4",  //   CharEncodingType::iso8859_4
   "ISO-8859-5",  //   CharEncodingType::iso8859_5
   "ISO-8859-6",  //   CharEncodingType::iso8859_6
   "ISO-8859-7",  //   CharEncodingType::iso8859_7
   "ISO-8859-8",  //   CharEncodingType::iso8859_8
   "ISO-8859-9",  //   CharEncodingType::iso8859_9
   "ISO-8859-10", //   CharEncodingType::CharEncodingType::iso8859_10
   "ISO-8859-11", //   CharEncodingType::iso8859_11
   "ISO-8859-13", //   CharEncodingType::iso8859_13
   "ISO-8859-14", //   CharEncodingType::iso8859_14
   "ISO-8859-15", //   CharEncodingType::iso8859_15
   "ISO-8859-16", //   CharEncodingType::iso8859_16
   "UTF-8_or_ISO-8859-1", // CharEncodingType::UTF8_or_Iso8859_1
   "WINDOWS-1250", //  CharENcodingType::windows_1250
   "WINDOWS-1251", //  CharENcodingType::windows_1251
   "WINDOWS-1252", //  CharENcodingType::windows_1252
   "WINDOWS-1253", //  CharENcodingType::windows_1253
   "WINDOWS-1254", //  CharENcodingType::windows_1254
   "WINDOWS-1255", //  CharENcodingType::windows_1255
   "WINDOWS-1256", //  CharENcodingType::windows_1256
   "WINDOWS-1257", //  CharENcodingType::windows_1257
   "WINDOWS-1258", //  CharENcodingType::windows_1258

   "",            //   CharEncodingType::nbrEncodingTypes
};

CharEncoding::unicodeToLatin1_entry
CharEncoding::m_unicodeToLatin1Entries[57] = {
   // This table must be sorted on the first entry since
   // binary search is used!
   { 0x102,   0x41   },   //   iso-8859-2:0xc3
   { 0x103,   0x61   },   //   iso-8859-2:0xe3
   { 0x104,   0x41   },   //   iso-8859-2:0xa1
   { 0x105,   0x61   },   //   iso-8859-2:0xb1
   { 0x106,   0x43   },   //   iso-8859-2:0xc6
   { 0x107,   0x63   },   //   iso-8859-2:0xe6
   { 0x10c,   0x43   },   //   iso-8859-2:0xc8
   { 0x10d,   0x63   },   //   iso-8859-2:0xe8
   { 0x10e,   0x44   },   //   iso-8859-2:0xcf
   { 0x10f,   0x64   },   //   iso-8859-2:0xef
   { 0x110,   0x44   },   //   iso-8859-2:0xd0
   { 0x111,   0x64   },   //   iso-8859-2:0xf0
   { 0x118,   0x45   },   //   iso-8859-2:0xca
   { 0x119,   0x65   },   //   iso-8859-2:0xea
   { 0x11a,   0x45   },   //   iso-8859-2:0xcc
   { 0x11b,   0x65   },   //   iso-8859-2:0xec
   { 0x139,   0x4c   },   //   iso-8859-2:0xc5
   { 0x13a,   0x6c   },   //   iso-8859-2:0xe5
   { 0x13d,   0x4c   },   //   iso-8859-2:0xa5
   { 0x13e,   0x6c   },   //   iso-8859-2:0xb5
   { 0x141,   0x4c   },   //   iso-8859-2:0xa3
   { 0x142,   0x6c   },   //   iso-8859-2:0xb3
   { 0x143,   0x4e   },   //   iso-8859-2:0xd1
   { 0x144,   0x6e   },   //   iso-8859-2:0xf1
   { 0x147,   0x4e   },   //   iso-8859-2:0xd2
   { 0x148,   0x6e   },   //   iso-8859-2:0xf2
   { 0x150,   0x4f   },   //   iso-8859-2:0xd5
   { 0x151,   0x6f   },   //   iso-8859-2:0xf5
   { 0x154,   0x52   },   //   iso-8859-2:0xc0
   { 0x155,   0x72   },   //   iso-8859-2:0xe0
   { 0x158,   0x52   },   //   iso-8859-2:0xd8
   { 0x159,   0x72   },   //   iso-8859-2:0xf8
   { 0x15a,   0x53   },   //   iso-8859-2:0xa6
   { 0x15b,   0x73   },   //   iso-8859-2:0xb6
   { 0x15e,   0x53   },   //   iso-8859-2:0xaa
   { 0x15f,   0x73   },   //   iso-8859-2:0xba
   { 0x160,   0x53   },   //   iso-8859-2:0xa9
   { 0x161,   0x73   },   //   iso-8859-2:0xb9
   { 0x162,   0x54   },   //   iso-8859-2:0xde
   { 0x163,   0x74   },   //   iso-8859-2:0xfe
   { 0x164,   0x54   },   //   iso-8859-2:0xab
   { 0x165,   0x74   },   //   iso-8859-2:0xbb
   { 0x16e,   0x55   },   //   iso-8859-2:0xd9
   { 0x16f,   0x75   },   //   iso-8859-2:0xf9
   { 0x170,   0x55   },   //   iso-8859-2:0xdb
   { 0x171,   0x75   },   //   iso-8859-2:0xfb
   { 0x179,   0x5a   },   //   iso-8859-2:0xac
   { 0x17a,   0x7a   },   //   iso-8859-2:0xbc
   { 0x17b,   0x5a   },   //   iso-8859-2:0xaf
   { 0x17c,   0x7a   },   //   iso-8859-2:0xbf
   { 0x17d,   0x5a   },   //   iso-8859-2:0xae
   { 0x17e,   0x7a   },   //   iso-8859-2:0xbe
   { 0x2c7,   0x0    },   //   iso-8859-2:0xb7 caron
   { 0x2d8,   0x0    },   //   iso-8859-2:0xa2 breve
   { 0x2d9,   0x0    },   //   iso-8859-2:0xff dot above
   { 0x2db,   0x0    },   //   iso-8859-2:0xb2 ogonek
   { 0x2dd,   0x0   },   //   iso-8859-2:0xbd double acute
};

CharEncoding::unicodeToLatin1Table
CharEncoding::m_unicodeToLatin1Table = {
   57,
   m_unicodeToLatin1Entries,
};


CharEncodingType::charEncodingType
CharEncoding::encStringToEncType( const MC2String& encodingStr )
{
   CharEncodingType::charEncodingType result = 
      CharEncodingType::invalidEncodingType;

   uint32 i=0;
   bool found = false;
   uint32 nbrEncodingTypes = 
      static_cast<uint32>(CharEncodingType::nbrEncodingTypes);
   while ( (!found) && ( i < nbrEncodingTypes ) ){
      found = ( StringUtility::strcmp( encodingStr.c_str(), 
                                       m_iconvEnodingTypeString[i] ) == 0);
      if ( found ){
         result = static_cast<CharEncodingType::charEncodingType>( i );
      }
      else {
         i++;
      }
   }
   return result;
} // encStringToEncType


MC2String 
CharEncoding::encTypeToEncString( CharEncodingType::charEncodingType
                                        encodingType )
{
   MC2String result;
   if ( encodingType < CharEncodingType::nbrEncodingTypes ){
      uint32 index = static_cast<uint32>(encodingType);
      result = m_iconvEnodingTypeString[index];
   }
   return result;
} // encTypeToEncString


vector<MC2String>
CharEncoding::getAllChEncStrings(){
   vector<MC2String> result;

   uint32 nbrCharEncTypes = 
      static_cast<uint32>(CharEncodingType::nbrEncodingTypes);
   for ( uint32 i=1; i<nbrCharEncTypes; i++){
      result.push_back( MC2String(m_iconvEnodingTypeString[i]) );
   }
   return result;
} // getAllChEncStrings

namespace {
   class UnicodeToLatin1Finder {
   public:
      bool operator()(const struct CharEncoding::unicodeToLatin1_entry& a,
                      const struct CharEncoding::unicodeToLatin1_entry& b) {
         return a.unicode < b.unicode;
      }
      bool operator()(const struct CharEncoding::unicodeToLatin1_entry& a,
                      uint32 code ) {
         return a.unicode < code;
      }
   };
}

uint32
CharEncoding::unicodeToLatin1( uint32 unicode )
{
   const unicodeToLatin1_entry* begin =
      m_unicodeToLatin1Table.entries;

   const unicodeToLatin1_entry* end =
      m_unicodeToLatin1Table.entries + m_unicodeToLatin1Table.nbrEntries;

   const unicodeToLatin1_entry* found =
      std::lower_bound( begin, end,
                        unicode,
                        UnicodeToLatin1Finder() );
      
   if( found->unicode == unicode ) {
      return found->latin1;
   } else {
      return 0;
   }
}


CharEncoding::CharEncoding( CharEncodingType::charEncodingType fromType, 
                            CharEncodingType::charEncodingType toType,
                            bool dieOnError /*false*/ ):
   m_iconvDesc( (iconv_t)(-1) ),
   m_initedOK( true )
{
   if ( ( fromType == CharEncodingType::invalidEncodingType ) || 
        ( fromType >= CharEncodingType::nbrEncodingTypes ) || 
        ( toType == CharEncodingType::invalidEncodingType ) ||
        ( toType >= CharEncodingType::nbrEncodingTypes ) )
   {
      mc2log << error << "CharEncoding::CharEncoding: Invalid encoding"
             << " type supplied in constructor." << endl;
      mc2dbg << "   fromType:" << (int)fromType << endl;
      mc2dbg << "   toType:" << (int)toType << endl;
      if ( dieOnError ){
         exit(1);
      }
   }

   m_fromType = fromType;
   m_toType = toType;
   m_iconvToType = m_toType;
   m_dieOnError = dieOnError;

   bool initIconv = true;

   if (  m_toType == CharEncodingType::iso8859_1 ){
      
      // Not all char encodings can be converted to iso8859-1
      if ( ! (
              ( m_fromType == CharEncodingType::iso8859_1 ) ||
              ( m_fromType == CharEncodingType::iso8859_2 ) ||
              ( m_fromType == CharEncodingType::UTF8 ) ||
              ( m_fromType == CharEncodingType::UTF16_be ) ||
              ( m_fromType == CharEncodingType::UTF16_le ) ||
              ( m_fromType == CharEncodingType::UCS2_be ) ||
              ( m_fromType == CharEncodingType::UCS2_le ) ||
              ( m_fromType == CharEncodingType::UCS4_be ) ||
              ( m_fromType == CharEncodingType::UCS4_le )
              )
           ){
         mc2log << error << "CharEncoding::CharEncoding: Cannot convert "
                << " to CharEncodingType::iso8859_1." << endl;
         mc2dbg << "   fromType:" << (int)fromType << endl;
         mc2dbg << "   toType:" << (int)toType << endl;
         mc2dbg << "   m_fromType:" << (int)m_fromType << endl;
         mc2dbg << "   m_toType:" << (int)m_toType << endl;
         mc2dbg << "   CharEncodingType::iso8859_1:" << 
            (int)CharEncodingType::iso8859_1 << endl;
         mc2dbg << "   CharEncodingType::iso8859_2:" 
                << (int)CharEncodingType::iso8859_2 << endl;


         if ( dieOnError ){
            exit(1);
         } 
      }
           
            
      // When converting to iso8859-1 foldToIso8859_1 takes care of the
      // whole converting. First it uses convert to convert to USC-4,
      // then it folds and converts to iso8859-1 at the same time from 
      // UCS-4.
      //
      // Therefore we don't need a fromType->toType m_iconvDesc, but only
      // a fromType->UCS-4 m_iconvDesc.
      m_iconvToType = CharEncodingType::UCS4_le;
   }
   if ( m_toType == CharEncodingType::UTF8_or_Iso8859_1 ){
      mc2log << error << "CharEncoding::CharEncoding: Cannot convert "
             << " to CharEncodingType::UTF8_or_Iso8859." << endl;
      mc2dbg << "   fromType:" << (int)fromType << endl;
      mc2dbg << "   toType:" << (int)toType << endl;
      if ( dieOnError ){
         exit(1);
      }
   }
   if ( m_fromType == CharEncodingType::UTF8_or_Iso8859_1 ){
      if ( m_toType != CharEncodingType::UTF8 ){
         mc2log << error << "CharEncoding::CharEncoding: Only possible " 
                << " to convert to UTF8 when converting from "
                << "CharEncodingType::UTF8_or_Iso8859." << endl;
         mc2dbg << "   fromType:" << (int)fromType << endl;
         mc2dbg << "   toType:" << (int)toType << endl;
         if ( dieOnError ){
            exit(1);
         }
      }
      initIconv = false;
   }


   if ( initIconv ){
      MC2String fromCode = encTypeToEncString(m_fromType);
      MC2String toCode = encTypeToEncString(m_iconvToType);
      
      
      m_iconvDesc = iconv_open( toCode.c_str(), fromCode.c_str() );
      
      // Error handling.
      if ( m_iconvDesc == (iconv_t)(-1) ){
         int errorNbr = errno;
         mc2log << error << "CharEncoding::CharEncoding: "
                << "Error in opening iconv, error code:" << errorNbr;
         if ( errorNbr == EINVAL ){
            mc2log << error << "CharEncoding::CharEncoding: Conversion"
                   << " from " << fromCode << " to " << toCode 
                   << " not supported." << endl;
         }
         if ( m_dieOnError ){
            exit(1);
         }
         m_initedOK = false;
      }
   }
    
} // CharEncoding

CharEncoding::~CharEncoding()
{
   if ( m_initedOK ) {
      if ( m_iconvDesc != (iconv_t)(-1) &&
           iconv_close( m_iconvDesc ) < 0 ) {
         mc2log << warn << "[CharEnc]: iconv_close failed ! "
                << strerror(errno) << endl;
      }
   }
}

bool 
CharEncoding::convert( const MC2String& src, MC2String& dst ) const
{
   bool result = true;


   // When converting to iso-8859-1 we want to fold the chars
   // when converting.
   if (  m_toType == CharEncodingType::iso8859_1 ){
      // Both folding and conversion is taken care of here.
      result = foldToIso8859_1( src, dst );
   }
   else if ( ( m_fromType == CharEncodingType::UTF8_or_Iso8859_1 ) && 
             ( m_toType == CharEncodingType::UTF8 ) ){
      dst = UTF8Util::cleanUtf8( src );
   }
   else {

      // Using iconv conversion only in the normal case.
      result = iconvConvert( src, dst );
   }
   return result;

} // convert
      
bool
CharEncoding::iconvConvert( const MC2String& src, MC2String& dst ) const
{
   bool result = true;

   // Not converting to iso-8859-1. Do it the normal way.
   size_t srcSize = src.size();
   char* srcBuffer = const_cast<char*> ( src.c_str() );

   size_t dstSize = srcSize*4;  // This one should always be enough.
   size_t dstBufferBytesLeft = dstSize;

   char* dstBuffer = new char[dstSize];
   char* originalDstBuffer = dstBuffer;

   /* Increments srcBuffer and dstBuffer by the number of bytes read
    * and written. decrements srcSize and dstBufferBytesLefr by number
    * of bytes read and written.
    */
   size_t nbrNonReversible = iconv(m_iconvDesc,
                                   &srcBuffer, &srcSize,
                                   &dstBuffer, &dstBufferBytesLeft);
   // Error handling.
   if ( ( nbrNonReversible == size_t(-1) ) || (srcSize > 0) ){
      int errorNbr = errno;
      bool dumpSrcBuf = false;
      
      if ( errorNbr == EILSEQ ){
         mc2log << error << "CharEncoding::iconConvert. Invalid multibyte "
                << "sequence." << endl;
         dumpSrcBuf = true;
      }
      else if ( errorNbr == EINVAL ){
         mc2log << error << "CharEncoding::iconConvert. Incomplete"
                << " multibyte sequence." << endl;
         dumpSrcBuf = true;
      }
      else if ( errorNbr == E2BIG ){
         mc2log << error << "CharEncoding::iconConvert. Destination buffer"
                << "to small." << endl;
      } 
      else {
         mc2log << error << "CharEncoding::iconConvert. Unknown error from"
                << " iconv, error code:" << errorNbr << endl;
      }
      
      if ( dumpSrcBuf ){
         mc2dbg << "Dumping rest of srcBuffer:" << endl;
         for ( uint32 i=0; i<srcSize; i++){
            mc2dbg << "0x" << hex << (int)(unsigned char)srcBuffer[i]
                   << dec << ", ";
         }
         mc2dbg << endl;
      }
      if ( m_dieOnError ){
         exit(1);
      }
      result = false;
   }


  
   // Set the destination string.
   uint32 newDstSize = dstSize - dstBufferBytesLeft;
   dst.clear();
   dst.append( originalDstBuffer, newDstSize );

   delete[] originalDstBuffer;

   return result;
} // iconvConvert


bool
CharEncoding::foldToIso8859_1( const MC2String& src, MC2String& dst ) const
{
   bool result = true;

   if ( m_toType != CharEncodingType::iso8859_1 ){
      // Like the code is written right now (20041021) it is an error 
      // to call this method with another m_toType than iso-8859-1.
      // However it might change some time later.

      mc2log << warn << "CharEncoding::foldToIso8859: "
             << "Tries folding to iso-8859-1 with toType "
             << encTypeToEncString( m_toType )
             << endl;
   }
      

   
   if ( m_iconvToType != CharEncodingType::UCS4_le ){
      mc2log << error << "CharEncoding::foldToIso8859: "
             << "Tries folding with iconv not inited to convert to UCS-4"
             << endl;
      if (m_dieOnError){
         exit(1);
      }
      result = false;
      return result;
   }
   MC2String ucs4Str;
   iconvConvert( src, ucs4Str );

   uint32 nbrChars = static_cast<uint32>(ucs4Str.size() / 4);
   if ( (ucs4Str.size() % 4) != 0 ){
      mc2log << error << "CharEncoding::foldToIso8859: "
             << "[A] Something wrong with UCS-4 string." << endl;
      if ( m_dieOnError ) {
         exit(1);
      }
      result = false;
   }
   else{
      // Put unicode character codes in a vector.
      vector<uint32> charUnicodes;
      for ( uint32 i=0; i<nbrChars*4; i++){
         uint32 charCode = 0;
         charCode |= ( (unsigned char)ucs4Str[i] );
         i++;
         charCode |= ( (unsigned char)ucs4Str[i] << 8 );
         i++;
         charCode |= ( (unsigned char)ucs4Str[i] << 16 );
         i++;
         charCode |= ( (unsigned char)ucs4Str[i] << 24 );
         charUnicodes.push_back( charCode );
      }
      if ( charUnicodes.size() != nbrChars ){
         mc2log << error << "CharEncoding::foldToIso8859: "
                << "[B] Something wrong with UCS-4 string." 
                << " nbrChars:" << nbrChars << " charUnicodes.size():" 
                << charUnicodes.size() << endl;
         if ( m_dieOnError ) {
            exit(1);
         }
         result = false;
      } 
      else {
         
         // Convert unicode character codes to iso-8859-1 codes.

         dst.clear();
         for ( uint32 i=0; i<charUnicodes.size(); i++ ){
            uint32 charCode = charUnicodes[i];

            if ( charCode <= 0xff ){
               // Latin-1 and unicode are the same the 0xff first chars.
               dst.append( 1, static_cast<char>(charCode) );
            }
            else {
               // Consult the folding table.
               uint32 latin1CharCode = unicodeToLatin1(charCode);
               if( latin1CharCode > 0 ) {
                  if ( latin1CharCode > 0xff ){
                     mc2log << error << "m_mapUnicodeToIso8859_1 gave a"
                            << " non latin-1 character:" << latin1CharCode
                            << endl;
                     if ( m_dieOnError ){
                        exit(1);
                     }
                     result = false;
                  }
                  else {
                     dst.append(1, static_cast<char>(latin1CharCode) );
                  }
               }
               else {
                  // Character not present in the map.
               }
            }
         }
      }
   }
   return result;
} // foldToIso8859_1

MC2String 
CharEncoding::getFromTypeStr() const
{
   return encTypeToEncString( m_fromType );

} // getFromTypeStr

MC2String 
CharEncoding::getToTypeStr() const
{
   return encTypeToEncString( m_toType );

} // getToTypeStr


CharEncodingType::charEncodingType 
CharEncoding::getFromType() const
{
   return m_fromType;
} // getFromType

CharEncodingType::charEncodingType
CharEncoding::getToType() const
{
   return m_toType;
} // getToType


CharEncodingType::charEncodingType 
CharEncoding::getMC2CharEncoding()
{

#ifdef MC2_UTF8
   return CharEncodingType::UTF8;
#else
   return CharEncodingType::iso8859_1;
#endif

} // getMC2CharEncoding
