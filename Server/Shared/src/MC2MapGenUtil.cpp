/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2MapGenUtil.h"

// These constants must be latin-1, 
// they consist of a special char within less-than and greater-than signs.
// They are used in POI files and map correction records, for POIs they are 
// re-defined in the POIObject.pm
// 
// The inverted exclamation mark (hex char code: a1)
const MC2String MC2MapGenUtil::poiWaspFieldSep = "<¡>";
// The inverted question mark (hex char code: bf)
const MC2String MC2MapGenUtil::poiWaspNameSep = "<¿>";
// The plus/minus mark (hex char code: b1)
const MC2String MC2MapGenUtil::poiWaspSubNameSep = "<±>";

const char* const MC2MapGenUtil::mapSupCopyrigthStrings[] = { 
   "",           // unknownSupplier
   "NAVTEQ",     // NavTech
   "Tele Atlas", // TeleAtlas
   "AND",        // AND
   "TopMap",     // TopMap
   "GDT",        // GDT
   "Infotech",   // Infotech
   "Monolit",    // Monolit,
   "Spinfo",     // Spinfo,
   "CE Info Systems",     // CEInfoSystems,
   "MAPA",     // GISrael,
   "dmapas",     // DMapas,
   "NAVTURK",   // NavTurk
   "Carmenta",   // Carmenta
   "OpenStreetMap",   // OpenStreetMap

   // Don't add value for nbrMapSuppliers

   // Add also to map_supplier_names.xml for map generation
};

