/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CHARENCODINGTYPE_H
#define CHARENCODINGTYPE_H
#include "config.h"

//#include "MC2SimpleString.h"
//#include "MC2String.h"

class CharEncodingType {
public:

   
      /**
       *   
       */
      enum charEncodingType {
         invalidEncodingType = 0,

         /**
          * @name Unicode multibyte encoding systems.
          */
         //@{
         /** UTF-8 encoding
          *  - Used for unicode by Linix (NFC).
          */
         UTF8,  //UTF-8,

         /// UTF-16 encoding, big endian byte order
         UTF16_be, //UTF-16BE
         
         /** UTF-16 encoding, little endian byte order.
          *  Little endian is more common than big endian.
          *  - Used for unicode by Windows (NFC?).
          */
         UTF16_le, //UTF-16LE

         /// UCS-2 encoding, big endian byte order.
         UCS2_be, //UCS-2BE//

         /// UCS-2 encoding, little endian byte order.
         UCS2_le, //UCS-2LE//

         /// UCS-4 encoding, big endian byte order.
         UCS4_be, //UCS-4BE//

         /// UCS-4 encoding, little endian byte order.
         UCS4_le, //UCS-4LE//
         //@}

         
         /**
          * @name ISO standard single byte encoding systems.
          */
         //@{
         /// Latin-1: ASCII + Western European languagers.
         iso8859_1,

         /// Latin-2: ASCII + Central European languages.
         iso8859_2,

         /** Latin-3: ASCII + South European languages, Maltese and 
          *  Esperanto. 
          */
         iso8859_3,

         /// Latin-4: ASCII + Baltic languages
         iso8859_4,

         /// ASCII + Slavic languages
         iso8859_5,

         /// ASCII + Arabic characters
         iso8859_6,

         /// ASCII + Greek characters 
         iso8859_7,

         /// ASCII + Hebrew characters
         iso8859_8,

         /// Latin-5: As Latin-1 but favours Turkish over Icelandic
         iso8859_9,

         /** Latin-6: ASCII + Nordic languages, i.e. Latvian & Inuit & 
          *  non-Skolt Sami.
          */
         iso8859_10,

         /// ASCII + Thai support.
         iso8859_11,
         
         /// Latin-7: ASCII + Celtic support.
         iso8859_13,

         /// Latin-8: ASCII + Baltic Rim chars.
         iso8859_14,

         /// Latin-9: ASCII + Sami support (Finnish)
         iso8859_15,

         /** Latin-10: As Latin-1 but with euro-sign and accented Finnish
          *  & French chars.
          */
         iso8859_16,
         //@}


         /**
          * @name Special encoding systems. E.g. combinations of encoding
          *       systems.
          *      
          */
         //@{

         /** Latin-1 or UTF-8 encoding. Use this when encoding can be 
          *  eithter of UTF-8 and Latin-1.
          */
         UTF8_or_Iso8859_1,

         //@}

         /**
          * @name Windows encoding systems, see
          *       http://www.microsoft.com/globaldev/reference/WinCP.mspx
          */ 
         //@{
         /// Single byte, central Europe
         windows_1250,

         /// Single byte, cyrillic
         windows_1251,

         /// Single byte, latin 1
         windows_1252,

         /// Single byte, greek
         windows_1253,

         /// Single byte, turkish
         windows_1254,

         /// Single byte, hebrew
         windows_1255,

         /// Single byte, arabic
         windows_1256,

         /// Single byte, baltic
         windows_1257,

         /// Single byte, vietnam
         windows_1258,
         //@}

         /**
          * @name Other encoding systems
          */ 
         //@{
         
         

         //@}

         /// End marker, keep last.
         nbrEncodingTypes
      };            


};

#endif //CHARENCODINGTYPE_H
