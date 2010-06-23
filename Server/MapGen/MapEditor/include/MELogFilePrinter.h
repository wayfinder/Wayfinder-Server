/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MELOGFILEPRINTER_H
#define MELOGFILEPRINTER_H

#include <vector>
#include "MC2String.h"

class CharEncoding;

/**
 *    Class that handles all printing to log file (extra data file) 
 *    when correcting map errors
 */
class MELogFilePrinter {
   public:

      /**
       *    Print to extra data file. If the mc2 code is utf8 compiled the
       *    extra data file will be written in utf-8, else it is written
       *    in latin-1.
       *
       *    @param   logCommentStrings  The extra data record comment created
       *                         by each editDialog. Includes EndOfRecord-tag.
       *    @param   edTypeString   String for which extra data record type
       *                         we are writing, e.g. "removeNameFromItem",
       *                         created by each editDialog. Exclude separator.
       *    @param   identString The item/node/conn identification string,
       *                         created by each editDialog.
       *    @param   valueString The correction we are making, created by 
       *                         each editDialog. Does not include the ending
       *                         "EndOfRecord;"-tag
       */
      static void print( vector<MC2String> logCommentStrings,
                         const char* edTypeString,
                         MC2String identString,
                         vector<MC2String> valueStrings );

   private:
      
      /**
       *    Converts a vector with extra data record sub strings using a 
       *    given char encoding object. Each sub string is converted and
       *    added to the output vector. If the sub strings vector is
       *    the extra data log comment string, sub string nr 4 is not
       *    converted (original value) since it already has the correct
       *    char encoding.
       *
       *    @param   src   The extra data strings vector to convert.
       *    @param   charEncoder The char encoding object to use.
       *    @param   isLogComment  Tells if the sub strings vector to convert
       *                   is the extra data log comment string (true)
       *                   or one part of the extra data record (false).
       */
      static vector<MC2String> convertEDStrings(
                  vector<MC2String> src, CharEncoding* charEncoder,
                  bool isLogComment);
      
};

#endif

