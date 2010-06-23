/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MELogFilePrinter.h"
#include "MC2String.h"
#include "StringUtility.h"
#include "MEMapEditorWindow.h"
#include "OldExtraDataUtility.h"
#include "CharEncoding.h"

#define END_OF_RECORD "EndOfRecord"

void
MELogFilePrinter::print( vector<MC2String> logCommentStrings,
                       const char* edTypeString,
                       MC2String identString,
                       vector<MC2String> valueStrings )
{
   // Do we need to convert any of the strings?
   CharEncoding* charEncoder = NULL;
   bool dieOnError = true;

   // If the server code is compiled utf-8:
   //  - print the extra data log file in utf-8
   // If the server code is compiled latin-1:
   //  - print the extra data log file in latin-1

   // Use the g_utf8Strings to check if the loaded map was saved as utf-8
   
   // Assuming that strings entered manually is in latin-1
   // This might be changed later!
   //bool utf8InInput = false;
   // Running on centos, input is UTF8 already
   // If running on some machine interpretting input as latin-1 change!
   bool utf8InInput = true;

   // This UTF8 define decides the behavior of the mc2 code, use it to 
   // detect what kind of char encoding conversion is needed.
#ifdef MC2_UTF8
   // Map strings are utf-8
   // For latin-1 maps: strings were converted in ItemNames::internalLoad
   // We want extra data file printed in utf-8

   if ( utf8InInput ) {
      // everything is utf-8, no conversion needed
   
   } else {
      // input in MapEditor entries is latin-1, need conversion to utf-8
      mc2dbg8 << "ed log file: convert latin-1 -> utf8" << endl;
      charEncoder = new CharEncoding(
         CharEncodingType::iso8859_1, CharEncodingType::UTF8, dieOnError );
   }
#else
   // Map strings are latin-1
   // For utf-8 maps: strings were converted in ItemNames::internalLoad
   // We want extra data file printed in latin-1

   if ( ! utf8InInput ) {
      // everything is in latin-1, no conversion needed
   
   } else {
      // input in MapEditor entries is utf-8, need conversion to latin-1
      mc2dbg8 << "ed log file: convert utf-8 -> latin-1" << endl;
      charEncoder = new CharEncoding(
         CharEncodingType::UTF8, CharEncodingType::iso8859_1, dieOnError );
   }
#endif

   // Conversion:
   //  - logCommentStrings (origStrVal field has correct char encoding)
   //  - valueStrings
   // No conversion:
   //  - edTypeString (always english)
   //  - identString (created from map strings, which are correct)

   if ( charEncoder != NULL ) {
   
      valueStrings = convertEDStrings( valueStrings, charEncoder, false );

      logCommentStrings =
            convertEDStrings( logCommentStrings, charEncoder, true );

   } else {
      // need to create the log string without conversions
   }


   // Open log file
   char* logFileName = 
            StringUtility::newStrDup( g_mapEditorWindow->getLogFileName() );
   ofstream logfile(logFileName, ios::app);
   delete logFileName;
   logFileName = NULL;

   // Get vector strings as one string
   MC2String logComment =
         OldExtraDataUtility::createEDFileString( logCommentStrings );
   MC2String valueString =
         OldExtraDataUtility::createEDFileString( valueStrings );
   
   // Print to log file
   logfile << logComment << endl;   // ok with new separator
   logfile << edTypeString
           << OldExtraDataUtility::edFieldSep
           << identString
           << valueString
           << "EndOfRecord" 
           << OldExtraDataUtility::edFieldSep
           << endl;
}

vector<MC2String>
MELogFilePrinter::convertEDStrings( 
      vector<MC2String> src, CharEncoding* charEncoder, bool isLogComment )
{
   vector<MC2String> result;

   // Convert all substrings and append to dest
   // If a log comment, don't convert original value (string nbr 4)
   uint32 i = 0;
   for ( vector<MC2String>::iterator it = src.begin();
         it != src.end(); it++ ) {
      
      MC2String t = *it;
      // convert
      if ( !isLogComment || (i != 4) ) {
         charEncoder->convert( t, t);
      }
      // add to result
      result.push_back( t );
      
      i++;
   }
   
   return result;
}


