/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CommandlineOptionHandler.h"
#include "CharEncoding.h"
#include "STLStringUtility.h"
#include "UTF8Util.h"


#include "AsciiFile.h"
#include "StringUtility.h"
#include <fstream>
#include <iomanip>
#include <string>


int main(int argc, char* argv[])
{
   int minTailLength = 0;
   CommandlineOptionHandler coh(argc, argv, minTailLength );
   coh.setTailHelp("INPUT_FILE");
   coh.setSummary("Converts between character encoding systems using the "
                  "routines of mc2. E.g possible to fold between "
                  "ISO-8859-2 and ISO-8859-1. Prints converted data on "
                  "standard out.");


   // --------- character encoding system to convert from.
   const char* CL_fromChEnc = NULL;
   coh.addOption("-f", "--fromCharEncodingSystem",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_fromChEnc, "",
                 "The character encoding system to convert from. "
                 "Requires -t and INPUT_FILE");

   // --------- character encoding system to convert to.
   const char* CL_toChEnc = NULL;
   coh.addOption("-t", "--toCharEncodingSystem",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_toChEnc, "",
                 "The character encoding system to convert to. "
                 "Requires -f and INPUT_FILE");

   // --------- change order of the coordinates
   bool CL_listChEnc = false;
   coh.addOption("-l", "--listCharEncSystems",
                 CommandlineOptionHandler::presentVal,
                 1, &CL_listChEnc, "F",
                 "List character encoding systems possible to set as "
                 "for to and from char encoding system. However not all"
                 "of these systems are possible to convert between.");

   // --------- change order of the coordinates
   const char* CL_utf8CharCodes = NULL;
   coh.addOption("-u", "--utf8ToUnicode",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_utf8CharCodes, "",
                 "Give an UTF-8 character by character codes, like "
                 "\"0xc2 0x34\", and the unicode character code is "
                 "printed to standard out. If the character code is "
                 "below 0xff the ISO-8859-1 character is printed as "
                 "well.");

      

   if (!coh.parse()) {
      cerr << argv[0] << ": Error on commandline, (-h for help)" << endl;
      exit(1);
   }
   
   // List character encoding systems.
   if ( CL_listChEnc ){
      cout << "Listing supported character encoding systems. Folding "
           << "currently only supported for conversion from latin-2 to "
           << "latin-1." << endl << endl;
      

      vector<MC2String> charEncodings = 
         CharEncoding::getAllChEncStrings();
      for (uint32 i=0; i<charEncodings.size(); i++){
         cout << charEncodings[i] << endl;
      }
      exit(0);
   }

   // Convert between encoding systems.
   if ( ( CL_fromChEnc != NULL ) || ( CL_toChEnc != NULL ) ){
      if ( CL_toChEnc == NULL ){
         cerr << "Need to set the -t option whith the character encoding "
              << "system to convert to" << endl;
         exit(1);
      }
      if ( CL_fromChEnc == NULL ){
         cerr << "Need to set the -f option whith the character encoding "
              << "system to convert from" << endl;
         exit(1);
      }

      MC2String tmpStr = CL_fromChEnc;
      CharEncodingType::charEncodingType fromChEnc =
         CharEncoding::encStringToEncType( tmpStr );
      if ( fromChEnc == CharEncodingType::invalidEncodingType ){
         cerr << "Invalid from character encoding system. Got \""
              << CL_fromChEnc << "\"" << endl;
         exit(1);
      }
      tmpStr = CL_toChEnc;
      CharEncodingType::charEncodingType toChEnc =
         CharEncoding::encStringToEncType( tmpStr );
      if ( toChEnc == CharEncodingType::invalidEncodingType ){
         cerr << "Invalid to character encoding system. Got \""
              << CL_toChEnc << "\"" << endl;
         exit(1);
      }

      bool dieOnError = true;
      CharEncoding chEnc( fromChEnc, toChEnc, dieOnError );


      // Get the infile.
      if ( coh.getTailLength() != 1 ){
         cerr << "Wrong number of arguments in tail. Got " 
              << coh.getTailLength() << " number of arguments. "
              << " Should be one "
              << "argument giving INPUT_FILE." << endl;
         exit(1);
      }

      const char* inFileName = coh.getTail(0);
      ifstream infile(inFileName);
      if(!infile) {
         mc2log << error << "Invalid infile. Got \"" 
                << inFileName << "\"" << endl;
         infile.close();
         exit (1);
      }

      
      uint32 nbrLineFeeds=0;
      while (!infile.eof()) {

         uint32 i=0;

         MC2String tmpResult;
         uint32 tmpSize = 65535;
         MC2String tmpInput;
         char tmpCh='\0';
         while ( (!infile.eof()) && (i<tmpSize) && (tmpCh != '\n')){
            
            infile.get(tmpCh);
            if ( !infile.eof()){
               // Not converting eof character.

               tmpInput.append(1, tmpCh);
               i++;
            }
         }
         if ( i == tmpSize ){
            // This is OK as long as iconv does not complain about broken
            // multi byte sequences.
            cerr << "CharEncConvert: Ran into max tmpSize because of to "
                 << "long row." << endl;
         }
         else if (tmpCh == '\n'){
            nbrLineFeeds++;
         }

         chEnc.convert( tmpInput, tmpResult );
         //         cout << tmpResult;
         cout.write(tmpResult.c_str(), tmpResult.size());
         cout.flush();
         cerr << "Processed line feeds: " << nbrLineFeeds << "\15";
         cerr.flush();
      }

      infile.close();

      cerr << endl;
      exit(0);
   }

   if ( CL_utf8CharCodes != NULL ){
      MC2String charCodesStr = CL_utf8CharCodes;
      uint32 pos = 0;
      uint32 oldPos = pos;
      
      MC2String utf8Str;
      mc2dbg8 << "Indata:" << endl;
      while (pos != MC2String::npos ){
         pos = charCodesStr.find(MC2String(" "), pos);
         MC2String charCodeStr = charCodesStr.substr(oldPos, pos-oldPos);
         if ( charCodeStr == "" ){
            cerr << "Something wrong with indata when converting UTF-8 to "
                 << "unicode character codes." << endl;
            cerr << "Got: " << CL_utf8CharCodes 
                 << endl;
            cerr << "Error at position " << oldPos << " to " << pos 
                 << " Trailing spaces?" << endl;
            exit(1);
         }
         uint32 charCode = STLStringUtility::strtol(charCodeStr);

         if ( ( oldPos != 0 ) && 
              ( (charCode < 0x80) || (charCode > 0xbf) ) )
         {
            mc2log << error << "Invalid UTF-8 sequence at pos: " 
                   << oldPos << endl;
            exit(1);
         }

         utf8Str.append(1, static_cast<char>(charCode));

         

         mc2dbg8 << hex << "0x" << charCode << " " << dec;
         if ( charCode < 0x100 ){
            mc2dbg8 << static_cast<char>(charCode);
         }
         mc2dbg8 << endl;

         // Read past any space.
         while ( (  pos != MC2String::npos ) && 
                 (  charCodesStr.substr(pos, 1) == " ") )
         {
            pos++;
         }
         oldPos = pos;
      }
      mc2dbg8 << endl;

      mc2dbg8 << utf8Str << endl;


      uint32 unicode =  UTF8Util::utf8ToUcs(utf8Str.c_str());
      if ( ( unicode < 0x80 ) && ( utf8Str.size() > 1 ) ){
         mc2dbg << "unicode:" << unicode << endl;
         mc2dbg << "utf8Str.size():" << utf8Str.size() << endl;
         mc2log << error << "Invalid UTF-8 sequence" << endl;
         exit(1);
      }
   
      cout << hex << "0x" << unicode << " " << dec;
      if ( unicode < 0x100 ){
         cout << static_cast<char>(unicode);
      }
      cout << endl;


   } // CL_utf8CharCodes
}


