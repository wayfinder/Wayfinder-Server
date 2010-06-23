/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Inludes
#include "SMSStringUtility.h"
#include <ctype.h>

uint32
SMSStringUtility::printSMSString( char* const dest, const char* const src,
                                  StringTable::languageCode language,
                                  UserConstants::EOLType eol,
                                  uint32 phoneWidth,
                                  uint32 destLinePos,
                                  uint32 maxNbrLines,
                                  bool newLineAfterString )
{
   // To make sure that we print at least one line
   if (maxNbrLines < 1)
      maxNbrLines = 1;

   // The position in the destination buffer
   uint32 curDestPos = 0;
      
   if ((dest != NULL) && (src != NULL)) {
      // The number of characters to insert into the destination
      uint32 len = MIN(phoneWidth*maxNbrLines, strlen(src));
      char *srccpy = new char[len+1];
      strncpy(srccpy, src, len);
      srccpy[len]='\0';
      uint32 nameLength = strlen(srccpy);

      // The number of lines written in dest
      uint32 destNbrLines = 0;
      
      // Loop over all characters in srccpy
      uint32 curSrccpyPos = 0;
      bool breakLine = false;
      while ( (curSrccpyPos < nameLength) && 
              (destNbrLines < maxNbrLines) )
      {
         // Copy the current character into the destination string
         // if it is not whitespace to be copied to
         // the beginning of a line.
         if ( true || !((destLinePos == 0) && // TODO: There is no linebeaking!
                (isspace(srccpy[curSrccpyPos]))) )
         {
            dest[curDestPos] = srccpy[curSrccpyPos];
            curDestPos++;
            destLinePos++;
         }
         curSrccpyPos++;

         breakLine = false;
         // Now, check the need for a new line / line breaking.
         if ( (destLinePos >= phoneWidth-1/*2*/) && // why 2? what is 2?
              (curSrccpyPos < nameLength/*-1 2*/) )
         {
            // Almost end of line (leq one/*two*/ character/*s*/ left)
            // /* why 2?*/ CRLF perhaps?

            // Check if line should be broken
            if ( ( isspace(srccpy[curSrccpyPos  ])) &&
                 (!isspace(srccpy[curSrccpyPos+1])) )
            {
               // E.g " C"
               breakLine = true;
            }
            else if ( (!isspace(srccpy[curSrccpyPos  ])) &&
                      (!isspace(srccpy[curSrccpyPos+1])) &&
                      (destNbrLines < maxNbrLines-1))
            {
               // E.g "CCC"
//               dest[curDestPos] = '-';
//               curDestPos++;
               breakLine = true;
            } else if (destLinePos >= phoneWidth-1) {
               // next line
               breakLine = true;
            } else if ( (destNbrLines == maxNbrLines-1) && // last line
                        (curSrccpyPos < nameLength) ) // characters left
            {
               // Add one "extra" character to the end of the destination
//               dest[curDestPos] = srccpy[curSrccpyPos];
//               curDestPos++;
//               curSrccpyPos++;
               breakLine = true;
            }

            if (breakLine) {
//               curDestPos += writeEol(dest, curDestPos, eol, 
//                                      phoneWidth-destLinePos);
               destLinePos = 0;
               destNbrLines++;
            }
         }
      } // while

      // Add an eol if the (optional) newLineAfterString parameter
      // is set and the line was
      // not just broken
      // Yes! now we always want to break the lines.
      breakLine = false; // pretend the line was not just broken,
      // even if it was..
      if ((newLineAfterString) && (!breakLine)) {
         curDestPos += writeEol(dest, curDestPos, eol, 
                                phoneWidth-destLinePos);
      }
      
      // Set the end of the string!
      dest[curDestPos] = '\0';

      delete [] srccpy;
   } else if (dest != NULL) {
      // The string to write to is valid, but the other string is invalid.
      const char* noNameString = StringTable::getString( 
         StringTable::UNKNOWN, //"No name";
         language );
      curDestPos = sprintf(dest, "%s", noNameString);
      curDestPos += writeEol(dest, curDestPos, eol, 
                             phoneWidth-strlen(noNameString)); 
   } else {
      // The strings are not valid
      DEBUG1( cerr << "Error: Indata invalid in printSMSString!" << endl );
   }

   return (curDestPos);

}


uint32
SMSStringUtility::writeEol( char* dest,
                            uint32 pos,
                            UserConstants::EOLType eol,
                            uint32 leftOnLine )
{
   uint32 nbrCharsWritten = 0; // Keep the compiler happy (-O2)
   switch (eol) {
      case (UserConstants::EOLTYPE_CR) :
      case (UserConstants::EOLTYPE_AlwaysCR) :
         dest[pos] = 13;
         nbrCharsWritten = 1;
         break;
      case (UserConstants::EOLTYPE_LF) :
      case (UserConstants::EOLTYPE_AlwaysLF) :
         dest[pos] = 10;
         nbrCharsWritten = 1;
         break;
      case (UserConstants::EOLTYPE_CRLF) :
         if (leftOnLine < 2) {
            // Write one space
            dest[pos] = 32;
            nbrCharsWritten = 1;
         } else {
            dest[pos] = 13;
            dest[pos+1] = 10;
            nbrCharsWritten = 2;
         }
         break;
      case (UserConstants::EOLTYPE_NBR) :
         DEBUG1( cerr
                 << "Error: Incorrect EOLTYPE. Reverting to default."
                 << endl );
      case (UserConstants::EOLTYPE_AlwaysCRLF) :
      case (UserConstants::EOLTYPE_NOT_DEFINED) :
         dest[pos] = 13;
         dest[pos+1] = 10;
         nbrCharsWritten = 2;
         break;
   } // switch
   return (nbrCharsWritten);
}

