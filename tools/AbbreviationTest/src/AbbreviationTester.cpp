/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Types.h"
#include "config.h"
#include "AbbreviationTable.h"
#include "CommandlineOptionHandler.h"
#include "StringUtility.h"

#ifndef __WIN32
#  define TRACE printf
#endif

// Also test the CommandlineOptionHandler

class AbbOptionHandler : public CommandlineOptionHandler {
   public:
      AbbOptionHandler(int argc, char** argv)
            : CommandlineOptionHandler(argc, argv) {

         addOption("-e", "--expand", presentVal, 1, &m_expand, "false",
                   "Test the expand-method");

         addOption("-c", "--case", presentVal, 1, &m_case, "false",
                   "Test the makeFirstInWordCapital-method");

         if (!parse()) {
            printHelp(cout);
            exit(2);
         }
      };

      bool toAbbreviate() { 
         return (!m_expand && !m_case) ;
      };

      bool toChangeCase() { 
         return (m_case);
      };

   private:
      bool m_expand;
      bool m_case;

};

int main(int argc, char** argv)
{
   // Create the CommandlineOptionHandler
   AbbOptionHandler* args = new AbbOptionHandler(argc, argv);

   char* indata = new char[1024];
   char* result = new char[1024];
   bool exitTest = false;
   do {
      cout << "Enter string" << endl;
      fgets(indata, 1023, stdin);
      if ( (indata != NULL) && (strlen(indata) > 0)) {
         StringUtility::trimEnd(indata);
         
         if (args->toAbbreviate()) {
            AbbreviationTable::abbreviate(indata, result,
                                          LangTypes::swedish,
                                          AbbreviationTable::anywhere/*Might be too generous*/);
            cout << "\"" << indata << "\" abreviated as \"" 
                 << result << "\"" << endl; 
         } else if (args->toChangeCase()) {
            cout << "\"" << indata << "\" is changed to \"";
            StringUtility::makeFirstInWordCapital(indata, true);
            cout << indata << "\"" << endl;
         } else {
            AbbreviationTable::expand(indata, result, 1024,
                                      LangTypes::swedish,
                                      AbbreviationTable::anywhere/*Might be too generous*/);
            cout << "\"" << indata << "\" expanded to \"" 
                 << result << "\"" << endl; 
         }
      } else {
         exitTest = true;
      }
   } while (!exitTest);
   cout << "Exiting" << endl;
}

