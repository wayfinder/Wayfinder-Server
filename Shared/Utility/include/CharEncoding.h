/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CHARENCODING_H
#define CHARENCODING_H
#include "config.h"

#include <iconv.h>
#include <map>

//#include "MC2SimpleString.h"
#include "MC2String.h"
#include "CharEncodingType.h"
/**
 *   Class used for converting strings between different character 
 *   encodings.
 *
 */
class CharEncoding {
public:

   struct unicodeToLatin1_entry {
      uint32 unicode;
      uint32 latin1;
   };

   struct unicodeToLatin1Table {
      uint32 nbrEntries;
      const unicodeToLatin1_entry* const entries;
   };
   
   /**
    *   Constructor. Inits iconv, which is used internally by this class.
    *
    *   @param fromType The character encoding this CharEncoding object
    *                   will be used for converting from.
    *   @param toType   The characer encoding this CharEncoding object
    *                   will be used for converting to.
    *   @param dieOnError When setting this one to true, the program is 
    *                   aborted if an error is encountered.
    */
   CharEncoding( CharEncodingType::charEncodingType fromType, 
                 CharEncodingType::charEncodingType toType,
                 bool dieOnError=false );

   /**
    *   Closes the handle to the iconv stuff.
    */
   ~CharEncoding();

   /**
    *   Converts the string src from the fromType encoding type of this 
    *   encoding object to the to type of this encoding object. The result
    *   is stored in dst.
    *
    *   @param src The string to convert. Must be in the encoding system
    *              fromType of this encoding object.
    *
    *   @param dst The converted result is stored here in the encoding of 
    *              the toType of this encoding object.
    *
    *   @result Returns true if the conversion could be carried out. 
    *           Otherwise false.
    */
   bool convert( const MC2String& src, MC2String& dst ) const;

   /**
    *   Tells whether this conversion object can be used for converting
    *   character. Returns false otherwise.
    *
    *   @return Returns false if this conversion object not can be used
    *           for converting characters, otherwise true.
    */
   inline bool initedOK() const;

   /**
    *   Gives the character encoding type used in mc2 for a string used
    *   with iconv.
    *
    *   @param encodingStr 
    *                String that can be interpreted as a character
    *                encoding by iconv. Make sure that it is one of the 
    *                strings stored in CharEncoding::m_encTypeToEncString.
    *
    *   @return      The character encoding type used in mc2 corresponding
    *                to the iconv character encoding string identifier.
    *                Returns CharEncodingType::invalidEncodingType if no
    *                match was found.
    */
   static CharEncodingType::charEncodingType
      encStringToEncType( const MC2String& encodingStr );


   /**
    *   Gives the string used with iconv for a character encoding type,
    *   used in mc2.
    *
    *   @param encodingType An encoding type used in mc2.
    *
    *   @ return The string used by iconv to identify the  character
    *            encoding corresponding to encodingType. Returns an empty
    *            MC2String if no match was found.
    */
   static MC2String encTypeToEncString( CharEncodingType::charEncodingType
                                        encodingType );

   /**
    *   Returns strings representing all character encoding types
    *   supported.
    *
    *   @return Returns strings representing all character encoding types
    *           supported.
    */
   static vector<MC2String> getAllChEncStrings();


   /**
    *   Maps a unicode character to a iso-8859-1 character.
    *   @param unicode The unicode character.
    *   @return The iso-8859-1 character.
    */
   static uint32 unicodeToLatin1( uint32 unicode );

   /**
    *   Use for printing debug info about this character encoding object.
    */
   MC2String getFromTypeStr() const;

   /**
    *   Use for printing debug info about this character encoding object.
    */
   MC2String getToTypeStr() const;

   /**
    *   @return Returns the char encoding type converted from by this 
    *           char encoder.
    */
   CharEncodingType::charEncodingType getFromType() const;

   /**
    *   @return Returns the char encoding type converted to by this 
    *           char encoder.
    */
   CharEncodingType::charEncodingType getToType() const;

   /**
    *   Returns the char encoding used internally in the server. This 
    *   depends on how the server was compiled.
    *
    *   $return The characer encodign used internally in the server.
    */
   static CharEncodingType::charEncodingType getMC2CharEncoding();
   
  private:

   //====================================================================
   // Private methods:

   /**
    *   The main internal method for converting strings.
    *
    *   Converts the string src from m_fromType to m_iconvToType. The 
    *   result is stored in dst.
    *
    *   @param src The string to convert. Must be in the encoding system
    *              m_fromType of this encoding object.
    *
    *   @param dst The converted result is stored here in the encoding of 
    *              the m_iconvToType of this encoding object.
    *
    *   @result Returns true if the conversion went well. Otherwise false.
    */
   bool iconvConvert( const MC2String& src, MC2String& dst ) const;
   
   /** 
    * Takes any characters not beeing in the ISO-8859-1 set and replace
    * them with one of the characters in ISO-8859-1 or removes it if there
    * is no good replacement character in ISO-8859-1.
    *
    *   @param src The string to fold. Must be in the encoding system
    *              fromType of this encoding object.
    *
    *   @param dst The folded result is stored here. It may include less
    *              letters than the src string. The result is always 
    *              encoded as ISO-8859-1, i.e. if the in data is in 
    *              another charecter encoding it is both converted and
    *              folded to iso-8859-1.
    *
    *   @return Returns true if everything went well, otherwise false.
    */
   bool foldToIso8859_1( const MC2String& src, MC2String& dst ) const;

   //====================================================================
   // Private members:

   /**
    *   Mapping table for converting the enciding type used by mc2 to
    *   the enocding type identifier strings used by iconv. This table
    *   shall as far as it is possible store only unambigous encoding 
    *   type identifier strings. E.g. UCS-2BE and UCS-2LE instead of only
    *   UCS-2.
    */
   static const char* m_iconvEnodingTypeString[];

   /**
    * Maps unicode character codes to iso-8859-1 character codes.
    * Has to be sorted.
    */
   static unicodeToLatin1Table m_unicodeToLatin1Table;

   /**
    * Maps unicode character codes to iso-8859-1 character codes.
    */
   static unicodeToLatin1_entry m_unicodeToLatin1Entries[57];
   
   /// Character encoding type this instace is used for converting from.
   CharEncodingType::charEncodingType m_fromType;

   /// Character encoding type this instace is used for converting to.
   CharEncodingType::charEncodingType m_toType;
   
   /**
    * @name Members used with iconv.
    */
   //@{

   /** Character encoding type iconv is inited with when setting
    *   m_iconvDesc. Only different from m_toType if m_toType is iso8859_1.
    */
   CharEncodingType::charEncodingType m_iconvToType;   

   /**
    *   The descriptor to use with icon in this instance. 
    */
   iconv_t m_iconvDesc;

   //@}



   /**
    * This one is true if the character encoding object was inited OK
    * and by that can be used for converting characters.
    */
   bool m_initedOK;

   /**
    *   If an error is encountered and this one is set to true, the 
    *   program is aborted with an exitcode.
    */
   bool m_dieOnError;

};

//=====================================================================
//==== Inlines ========================================================

inline bool
CharEncoding::initedOK() const
{
   return m_initedOK;
}



#endif //CHARENCODING_H
