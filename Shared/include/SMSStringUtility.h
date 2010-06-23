/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SMSSTRINGUTILITY_H
#define SMSSTRINGUTILITY_H

// Includes
#include "config.h"
#include "UserConstants.h"
#include "StringTable.h"


/**
 * Some useful SMS related string functions.
 *
 */
class SMSStringUtility {
   public:
      
      /**
       *   Print one string (NULL-terminated) into another, formatted
       *   according to the given parameters. This method might be used 
       *   by the other formatting methods in this
       *   class as well as by other classes.
       *
       *   @param   dest        String where the result is written.
       *   @param   src         String containing the information that
       *                        should be written correctly formatted to 
       *                        dest.
       *   @param   language    The language of the string text if error.
       *   @param   eol         The type of end of line for the receivers
       *                        telephone.
       *   @param   phoneWidth  The number of characters of the receivers
       *                        deisplay.
       *   @param   destLinePos The offet to the first line.
       *                        Default value is 0.
       *   @param   maxNbrLines The maximum number of lines (if the 
       *                        result does not fit it is truncated). If 
       *                        less than one then the needed number of 
       *                        lines are used. Default value is 
       *                        MAX_UINT32.
       *   @param   newLineAfterString
       *                        True if the string shouls end with eol,
       *                        false otherwise. Dafault value is true.
       *   @return  The number of characters written to the dest string
       *            dest.
       */
      static uint32 printSMSString(char* const dest,
                                   const char* const src,
                                   StringTable::languageCode language,
                                   UserConstants::EOLType eol, 
                                   uint32 phoneWidth,
                                   uint32 destLinePos = 0,
                                   uint32 maxNbrLines = MAX_UINT32, 
                                   bool newLineAfterString = true);

      /**
       *   Write end of line to a string. The format and the number
       *   of characters required depends on the phonetype (the EOLType
       *   parameter given to this method).
       *
       *   @param   dest  The destination databuffer.
       *   @param   pos   The position of the first character to write
       *                  to dest (e.g the first character is written
       *                  to dest[pos]).
       *   @param   eol   What end of line type the receivers telephone
       *                  have.
       *   @param   charsLeftOnLine
       *                  The number of characters left on the current
       *                  line. Needed if the phone does not recognize
       *                  any new line character and must be padded
       *                  with spaces.
       *   @return  The number of characters written to dest.
       */
      static uint32 writeEol(char* dest, uint32 pos, 
                             UserConstants::EOLType eol,
                             uint32 charsLeftOnLine);


   private:
      /** 
       * Private constructor to avoid objects of this class.
       */
      SMSStringUtility();
};


#endif // SMSSTRINGUTILITY_H
