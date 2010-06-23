/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FILEDEBUGCALCROUTE_H
#define FILEDEBUGCALCROUTE_H

//#define FILE_DEBUGGING

#include "config.h"
#include <unistd.h>
#include <stdio.h>
/**
 *   Class for sending debug output from CalcRoute and friends to
 *   a file which can be read later by the MapViewer.
 *   All functions are inlined and should disappear if FILE_DEBUGGING
 *   is undefined with at least -O1.
 */
class FileDebugCalcRoute {

public:
   
   /**
    *   Opens the debugfile.
    */
   inline static void openFile();

   /**
    *   Closes the debugfile.
    */
   inline static void closeFile();

   /** 
    *   Writes an updated cost to the debugfile.
    *   @param what     'C' for ordinary cost 'R' for reset.
    *   @param itemID   The itemID.
    *   @param cost     The new cost for the item.
    *   @param realCost True if the cost is "real cost".
    *   @param freeText A comment to write in the debug file.
    */
   inline static void writeCostToFile(char what,
                                      uint32 itemID,
                                      uint32 oldCost,
                                      uint32 cost,
                                      bool realCost,
                                      bool isDest,
                                      const char* freeText);

   /**
    *  Writes a comment to the debugfile so that I can have a clue.
    *  @param comment The text to write to the file.
    */
   inline static void writeComment(const char* comment);
   
#ifdef FILE_DEBUGGING
   static FILE* c_file;
#endif
};

////////////////////////////////////////////////////////////////////
// Implementation of inlineds.
////////////////////////////////////////////////////////////////////

inline void
FileDebugCalcRoute::openFile()
{
#ifdef FILE_DEBUGGING
   static int curRouteNbr = 0;
   char filename[1024];
   sprintf(filename, "/tmp/route_%u_%d", getpid(), curRouteNbr++);   
   c_file = fopen(filename, "a");
#endif
}


inline void
FileDebugCalcRoute::closeFile()
{
#ifdef FILE_DEBUGGING
   if ( c_file != NULL ) {
      fflush(c_file);
      fclose(c_file);
      c_file = NULL;
   }
#endif
}


inline void
FileDebugCalcRoute::writeCostToFile(char what,
                                    uint32 itemID,
                                    uint32 oldCost,
                                    uint32 cost,
                                    bool realCost,
                                    bool isDest,
                                    const char* freeText)
{
#ifdef FILE_DEBUGGING
   if ( c_file != NULL ) {
      fprintf( c_file, "%c;%u;%u;%u;%u;%d;%s;%s\n",
               what, 0xffffffff, itemID, oldCost, cost, realCost,
               (isDest ? "dest" : "") , freeText);
   }
#endif   
}

inline void
FileDebugCalcRoute::writeComment(const char* comment)
{
#ifdef FILE_DEBUGGING
   if ( c_file != NULL ) {
      fprintf( c_file, "X;%s\n", comment);
   }
#endif   
}

#endif
