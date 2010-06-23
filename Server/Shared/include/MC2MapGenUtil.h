/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2MAPGENUTIL_H
#define MC2MAPGENUTIL_H

#include "config.h"
#include "MC2String.h"
#include "MapGenEnums.h"

class StringTable;

/**
 *   Class used for keeping map generation related stuff that the MC2 
 *   server needs access to. If possible, use this class instead:
 *   ./Server/MapGen/include/MapGenUtil.h
 *
 *   NB. This class is not thread safe.
 *
 */
class MC2MapGenUtil {
 public:
 
   /**
    * Delimiter to use when writing POIs to log file to use for
    * inserting to WASP database. Also used for map correction records.
    */
   static const MC2String poiWaspFieldSep;
   static const MC2String poiWaspNameSep;
   static const MC2String poiWaspSubNameSep;

 private:

   /**
    * Stores names of all map suppliers to use for copyright.
    */
   static const char* const mapSupCopyrigthStrings[];


   
}; // class MC2MapGenUtil

#endif // MC2MAPGENUTIL_H
