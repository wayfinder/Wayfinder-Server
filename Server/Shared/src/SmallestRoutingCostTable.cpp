/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include<fstream>
#include "MC2String.h"

#include "SmallestRoutingCostTable.h"

bool
SmallestRoutingCostTable::loadFromTextFile(const char* filename)
{
   // Reads the output from TestServer
   ifstream inFile(filename);

   MC2String firstString;

   if ( inFile ) {
      m_costMap.clear();
   }
   
   while( inFile ) {
      inFile >> firstString;
      if ( firstString == "#XXX" ) {
         // This is a comment to the TestServer but the input for us
         uint32 fromMapID;
         inFile >> fromMapID;
         uint32 toMapID;
         inFile >> toMapID;
         uint32 costAMeters;
         inFile >> costAMeters;
         uint32 costBSeconds;
         inFile >> costBSeconds;
         setCosts(fromMapID, toMapID, RoutingCosts(costAMeters, costBSeconds));
         mc2dbg << "[SRCT]: Added cost " << fromMapID << " "
                << toMapID << " " << costAMeters << " " << costBSeconds
                << endl;
         // Get the rest of the line, I hope
         getline(inFile, firstString);
         mc2dbg8 << "[SRCT]: Got \"" << firstString << '"' << endl;
      } else {
         getline(inFile, firstString);
         mc2dbg8 << "[SRCT]: Do not recognize \"" << firstString
                 << '"' << endl;
      }      
   }
   return true;
}

bool
SmallestRoutingCostTable::loadFromFile(const char* filename)
{
   // Currently only textfile loading is implemented
   return loadFromTextFile(filename);
}
