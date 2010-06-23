/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AbbreviationTable.h"
#include "StringUtility.h"
#include "MC2String.h"
#include "UTF8Util.h"
#include "ScopedArray.h"
#include <memory>
#include <ctype.h>

#define STR_LIMIT 1
// Do the conversion later.
#define _S(x) (x)

// ------------------------------------------------------- m_abbreviations
// All strings should be in utf-8 now.
const AbbreviationTable::abbreviation_t
AbbreviationTable::m_abbreviations_orig[] = 
{
   
   // Are these case sensitive?
   // Swedish string that will be abbreviated
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("VÄG"),_S("v"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("VÄGEN"), _S("v"), end},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("GATA"), _S("g"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("GATAN"), _S("g"), end},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("GRÄND"), _S("gr"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("GRÄNDEN"), _S("gr"), end},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("TRAFIKPLATS"), _S("Tp"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("NORRA"), _S("N"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("SÖDRA"), _S("S"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("ÖSTRA"), _S("Ö"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("VÄSTRA"), _S("V"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("SANKT"), _S("S:t"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("GAMLA"), _S("Ga"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("STORA"), _S("St"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("LILLA"), _S("L"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("PLATS"), _S("Pl"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("PLATSEN"), _S("pl"), end},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("STOCKHOLM"), _S("Sthlm"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("GÖTEBORG"), _S("Gbg"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("HELSINGBORG"), _S("Hbg"), word},
   // Any part of the string. NB! The order among these are important!!!
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("OMRÅDET"), _S("omr"), anywhere},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("OMRÅDE"), _S("omr"), anywhere},
   
   // Strings in english, StringTable::NBR_COUNTRY_CODES, to be abbreviated.
   // Combined with country if country specific abbreviations.
   // Try not to abbreviate a strig that have a meaning as fullstr
   // (typically last name of a person naming a stret..)
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("WEST"), _S("W"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("EAST"), _S("E"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NORTH"), _S("N"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SOUTH"), _S("S"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SOUTHWEST"), _S("SW"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SOUTHEAST"), _S("SE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NORTHWEST"), _S("NW"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NORTHEAST"), _S("NE"), word},

   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("WESTERN"), _S("W"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("EASTERN"), _S("E"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NORTHERN"), _S("N"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SOUTHERN"), _S("S"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SOUTHWESTERN"), _S("SW"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SOUTHEASTERN"), _S("SE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NORTHWESTERN"), _S("NW"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NORTHEASTERN"), _S("NE"), word},
   
   {LangTypes::english, StringTable::USA_CC, _S("ACRES"), _S("Acrs"), word},
   {LangTypes::english, StringTable::USA_CC, _S("ALLEY"), _S("Aly"), word},
   {LangTypes::english, StringTable::USA_CC, _S("ANNEX"), _S("Anx"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("ARCADE"), _S("Arc"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("AVENUE"), _S("Ave"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BEACH"), _S("Bch"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BOULEVARD"), _S("Blvd"), word},
   {LangTypes::english, StringTable::USA_CC, _S("BRANCH"), _S("Br"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BRIDGE"), _S("Brg"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BROOK"), _S("Brk"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BROOKS"), _S("Brks"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BYPASS"), _S("Byp"), word},
   {LangTypes::english, StringTable::USA_CC, _S("BAYOU"), _S("Byu"), word},
   {LangTypes::english, StringTable::USA_CC, _S("BYWAY"), _S("Bywy"), word},
   {LangTypes::english, StringTable::USA_CC, _S("CALLE"), _S("C"), word},
   {LangTypes::english, StringTable::USA_CC, _S("CHART"), _S("Chrt"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CIRCLE"), _S("Cir"), word},
   {LangTypes::english, StringTable::USA_CC, _S("CIRCUIT"), _S("Cirt"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("COURSE"), _S("Crse"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CAUSEWAY"), _S("Cswy"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("COURT"), _S("Ct"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CANYON"), _S("Cyn"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("DRIVE"), _S("Dr"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("DRIVEWAY"), _S("Drwy"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("ESTATE"), _S("Est"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("ESTATES"), _S("Ests"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("EXPRESSWAY"), _S("Expy"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("EXTENSION"), _S("Ext"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FIELD"), _S("Fld"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FORGE"), _S("Frg"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FORK"), _S("Frk"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FRONT"), _S("Frnt"), word},
//   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FOREST"), _S("Frst"), word},   // don't want to use this abbrev
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FORT"), _S("Ft"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FREEWAY"), _S("Fwy"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("GARDEN"), _S("Gdn"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("GARDENS"), _S("Gdns"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("GATEWAY"), _S("Gtwy"), word},
   {LangTypes::english, StringTable::USA_CC, _S("HARBOR"), _S("Hbr"), word},
   {LangTypes::english, StringTable::USA_CC, _S("HOLLOW"), _S("Holw"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("HEIGHTS"), _S("Hts"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("HIGHWAY"), _S("Hwy"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("INLET"), _S("Inlt"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("JUNCTION"), _S("Jct"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("KNOLL"), _S("Knl"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("KNOLLS"), _S("Knls"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("LANE"), _S("Ln"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("LANDING"), _S("Lndg"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MEADOW"), _S("Mdw"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MEADOWS"), _S("Mdws"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MANOR"), _S("Mnr"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MOUNT"), _S("Mt"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MOUNTAIN"), _S("Mtn"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MOUNTAINS"), _S("Mtns"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PARKWAY"), _S("Pkwy"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PLACE"), _S("Pl"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PLAZA"), _S("Plz"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PROMENADE"), _S("Prom"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PASSAGE"), _S("Psge"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("POINT"), _S("Pt"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PATHWAY"), _S("Ptwy"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("ROAD"), _S("Rd"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("RIDGE"), _S("Rdg"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SHOAL"), _S("Shl"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SHOALS"), _S("Shls"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SHORE"), _S("Shr"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SHORES"), _S("Shrs"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SUMMIT"), _S("Smt"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SPRING"), _S("Spg"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SPRINGS"), _S("Spgs"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SQUARE"), _S("Sq"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SQUARES"), _S("Sqs"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("STREET"), _S("St"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("STATION"), _S("Sta"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SUBDIVISION"), _S("Subd"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TERRACE"), _S("Ter"), word},
   {LangTypes::english, StringTable::USA_CC, _S("TRAFFICWAY"), _S("Trfy"), word},
   {LangTypes::english, StringTable::ENGLAND_CC, _S("TRAFFICWAY"), _S("Tfwy"), word},
   {LangTypes::english, StringTable::USA_CC, _S("THROUGHWAY"), _S("Trwy"), word},
   {LangTypes::english, StringTable::ENGLAND_CC, _S("THROUGHWAY"), _S("Thwy"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TURNPIKE"), _S("Tpke"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TRACK"), _S("Trak"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TRACE"), _S("Trce"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TRAIL"), _S("Trl"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TRAILER"), _S("Trlr"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("VIADUCT"), _S("Via"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("VISTA"), _S("Vis"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("VILLAGE"), _S("Vlg"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("VALLEY"), _S("Vly"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("VIEW"), _S("Vw"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CROSSING"), _S("Xing"), word},
   
   // Dutch
   {LangTypes::dutch, StringTable::NBR_COUNTRY_CODES, _S("DREEF"), _S("Dr"), word},
   {LangTypes::dutch, StringTable::NBR_COUNTRY_CODES, _S("LAAN"), _S("Ln"), word},
   {LangTypes::dutch, StringTable::NBR_COUNTRY_CODES, _S("PLEIN"), _S("Pl"), word},
   {LangTypes::dutch, StringTable::NBR_COUNTRY_CODES, _S("STRAAT"), _S("Str"), word},
   
   // German
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("GASSE"), _S("G"), word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("PLATZ"), _S("Plz"), word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("STRASSE"), _S("Str") ,word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("BRÜCKE"), _S("Br"), word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("MARKT"), _S("Mkt"), word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("PROMENADE"), _S("Prom"), word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("RING"), _S("Rg"), word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("STIEGE"), _S("Stg"), word},
  
   // French
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("ALLÉE"), _S("All"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("AVENUE"), _S("Av"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("BOULEVARD"), _S("Bd"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("CHEMIN"), _S("Ch"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("CROIX"), _S("Crx"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("DESCENTE"), _S("Dsc"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("DOMAINE"), _S("Dom"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("IMPASSE"), _S("Imp"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("PLACE"), _S("Pl"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("PROMENADE"), _S("Prom"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("RÉSIDENCE"), _S("Res"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("ROND-POINT"), _S("Rpt"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("ROUTE"), _S("Rte"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("VILLAGE"), _S("Vge"), word},
   
   // Danish
   // (NT street type abbrevs for Europe 2000)
   {LangTypes::danish, StringTable::NBR_COUNTRY_CODES, _S("VEJ"), _S("v"), end},   //
   {LangTypes::danish, StringTable::NBR_COUNTRY_CODES, _S("VEJEN"), _S("v"), end}, //
   {LangTypes::danish, StringTable::NBR_COUNTRY_CODES, _S("GADE"), _S("g"), end},
   {LangTypes::danish, StringTable::NBR_COUNTRY_CODES, _S("VEJ"), _S("v"), word},
   {LangTypes::danish, StringTable::NBR_COUNTRY_CODES, _S("GADE"), _S("g"), word},
   
   // Norwegian
   //{LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("VEIEN"), _S("v"), end},  //
   //{LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("VEGEN"), _S("v"), end},  //
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("VEIEN"), _S("vn"), end},   //
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("VEGEN"), _S("vn"), end},   //
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("GATA"), _S("g"), end},
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("GATE"), _S("g"), end},
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("GATA"), _S("g"), word},
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("GATE"), _S("g"), word},
};

const uint32
AbbreviationTable::m_nbrAbbreviations = 
   sizeof(m_abbreviations_orig) / sizeof(abbreviation_t);

const vector<AbbreviationTable::abbreviation_tommi_t>
AbbreviationTable::m_abbreviations =
convAbbrev(m_abbreviations_orig,
           sizeof(m_abbreviations_orig) / sizeof(abbreviation_t));


// ------------------------------------------------------- m_expansions
// 
// NB! The char '#' is interpreted as "isalnum() == 0", that is
//     "many non alphanumeric character"
const char AbbreviationTable::m_nonAlnumChar = '#';

const AbbreviationTable::abbreviation_t
AbbreviationTable::m_expansions_orig[] = 
{
   // Swedish string that will be expanded
   // Expand ends before words,
   // to avoid Hammars v -> Hammars väg -> Hammars vägatan
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("VÄGEN"), _S("V"), end},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("GATAN"), _S("G"), end},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("GRÄNDEN"), _S("GR"), end},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("PLATSEN"), _S("PL"), end},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("VÄG"), _S("V"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("GATA"), _S("G"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("GRÄND"), _S("GR"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("TRAFIKPLATS"), _S("TP"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("NORRA"), _S("N"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("SÖDRA"), _S("S"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("ÖSTRA"), _S("Ö"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("VÄSTRA"), _S("V"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("SANKT"), _S("ST"), word},  //
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("SANKT"), _S("S:T"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("GAMLA"), _S("GA"), word},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("STORA"), _S("ST"), word},  //
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("LILLA"), _S("L"), word},
   // Any part of the string. NB! The order among these are important!!!
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("OMRÅDET"), _S("OMR"), anywhere},
   {LangTypes::swedish, StringTable::NBR_COUNTRY_CODES, _S("OMRÅDE"), _S("OMR"), anywhere},
   
   // Strings in english, StringTable::NBR_COUNTRY_CODES, to be expanded
   // Combined with country if country specific abbreviations
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("WEST"), _S("W"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("EAST"), _S("E"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NORTH"), _S("N"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SOUTH"), _S("S"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SOUTHWEST"), _S("SW"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SOUTHEAST"), _S("SE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NORTHWEST"), _S("NW"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NORTHEAST"), _S("NE"), word},

   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("WESTERN"), _S("W"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("EASTERN"), _S("E"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NORTHERN"), _S("N"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SOUTHERN"), _S("S"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SOUTHWESTERN"), _S("SW"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SOUTHEASTERN"), _S("SE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NORTHWESTERN"), _S("NW"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NORTHEASTERN"), _S("NE"), word},
   
   {LangTypes::english, StringTable::USA_CC, _S("ACRES"), _S("ACRS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("ALCOVE"), _S("ALCV"), word},
   {LangTypes::english, StringTable::USA_CC, _S("ALLEY"), _S("ALY"), word},
   {LangTypes::english, StringTable::USA_CC, _S("ANNEX"), _S("ANX"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("ARCADE"), _S("ARC"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("AVENUE"), _S("AVE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BEACH"), _S("BCH"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BURG"), _S("BG"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BURGS"), _S("BGS"), word},
   {LangTypes::english, StringTable::USA_CC, _S("BLUFF"), _S("BLF"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BOULEVARD"), _S("BLVD"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BEND"), _S("BND"), word},
   {LangTypes::english, StringTable::USA_CC, _S("BRANCH"), _S("BR"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BRIDGE"), _S("BRG"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BROOK"), _S("BRK"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BROOKS"), _S("BRKS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("BYPASS"), _S("BYP"), word},
   {LangTypes::english, StringTable::USA_CC, _S("BAYOU"), _S("BYU"), word},
   {LangTypes::english, StringTable::USA_CC, _S("BYWAY"), _S("BYWY"), word},
   {LangTypes::english, StringTable::USA_CC, _S("CHART"), _S("CHRT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CIRCLE"), _S("CIR"), word},
   {LangTypes::english, StringTable::USA_CC, _S("CIRCUIT"), _S("CIRT"), word},
   {LangTypes::english, StringTable::NEW_ZEALAND_CC, _S("CIRCUIT"), _S("CIRT"), word},
   {LangTypes::english, StringTable::AUSTRALIA_CC, _S("CIRCUIT"), _S("CIRT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CLUB"), _S("CLB"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CLIFFS"), _S("CLFS"), word},
   {LangTypes::english, StringTable::USA_CC, _S("CAMPUS"), _S("CMPS"), word},
   {LangTypes::english, StringTable::NEW_ZEALAND_CC, _S("CLOSE"), _S("CLOS"), word},
   {LangTypes::english, StringTable::AUSTRALIA_CC, _S("CLOSE"), _S("CLOS"), word},
   {LangTypes::english, StringTable::USA_CC, _S("CORNERS"), _S("CORS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("COMMON"), _S("COM"), word},
   {LangTypes::english, StringTable::USA_CC, _S("CORNER"), _S("COR"), word},
   {LangTypes::english, StringTable::USA_CC, _S("CAMP"), _S("CP"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CAPE"), _S("CPE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CRESCENT"), _S("CRES"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CROFT"), _S("CRFT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CREEK"), _S("CRK"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("COURSE"), _S("CRSE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CURVE"), _S("CRVE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CAUSEWAY"), _S("CSWY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("COURT"), _S("CT"), word},
   {LangTypes::english, StringTable::USA_CC, _S("CENTER"), _S("CTR"), word},
   {LangTypes::english, StringTable::ENGLAND_CC, _S("CENTRE"), _S("CTR"), word},
   {LangTypes::english, StringTable::IRELAND_CC, _S("CENTRE"), _S("CTR"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("COURTS"), _S("CTS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("COVE"), _S("CV"), word},       // *
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CANYON"), _S("CYN"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("DALE"), _S("DL"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("DAM"), _S("DM"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("DRIVE"), _S("DR"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("DRIVEWAY"), _S("DRWY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("ESTATE"), _S("EST"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("ESTATES"), _S("ESTS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("EXPRESSWAY"), _S("EXPY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("EXTENSION"), _S("EXT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FIELD"), _S("FLD"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FIELDS"), _S("FLDS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FALLS"), _S("FLS"), word},     // *
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FLAT"), _S("FLT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FLATS"), _S("FLTS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FORD"), _S("FRD"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FORGE"), _S("FRG"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FORK"), _S("FRK"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FRONT"), _S("FRNT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FOREST"), _S("FRST"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FORT"), _S("FT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("FREEWAY"), _S("FWY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("GARDEN"), _S("GDN"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("GARDENS"), _S("GDNS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("GLADE"), _S("GLAD"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("GLEN"), _S("GLN"), word},      // *
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("GROUNDS"), _S("GRDS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("GREEN"), _S("GRN"), word},     // *
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("GROVE"), _S("GRV"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("GATEWAY"), _S("GTWY"), word},
   {LangTypes::english, StringTable::USA_CC, _S("HARBOR"), _S("HBR"), word},
   {LangTypes::english, StringTable::ENGLAND_CC, _S("HARBOUR"), _S("HRBR"), word},
   {LangTypes::english, StringTable::IRELAND_CC, _S("HARBOUR"), _S("HRBR"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("HILL"), _S("HL"), word},       // *
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("HILLS"), _S("HLS"), word},     // *
   {LangTypes::english, StringTable::USA_CC, _S("HOLLOW"), _S("HOLW"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("HEIGHTS"), _S("HTS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("HAVEN"), _S("HVN"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("HIGHWAY"), _S("HWY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("INLET"), _S("INLT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("ISLAND"), _S("IS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("ISLANDS"), _S("ISS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("JUNCTION"), _S("JCT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("KNOLL"), _S("KNL"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("KNOLLS"), _S("KNLS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("KEY"), _S("KY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("LOCK"), _S("LCK"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("LOCKS"), _S("LCKS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("LODGE"), _S("LDG"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("LOAF"), _S("LF"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("LIGHT"), _S("LGT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("LAKE"), _S("LK"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("LOOKOUT"), _S("LKOT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("LAKES"), _S("LKS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("LANE"), _S("LN"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("LANDING"), _S("LNDG"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MEADOW"), _S("MDW"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MEADOWS"), _S("MDWS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MILL"), _S("ML"), word},       // *
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MILLS"), _S("MLS"), word},     // *
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MANOR"), _S("MNR"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MISSION"), _S("MSN"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MOUNT"), _S("MT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MOUNTAIN"), _S("MTN"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MOUNTAINS"), _S("MTNS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("MOTORWAY"), _S("MTWY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("NECK"), _S("NCK"), word},      // *
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("ORCHARD"), _S("ORCH"), word},  // *
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("OUTLOOK"), _S("OTLK"), word},
   {LangTypes::english, StringTable::USA_CC, _S("OVERPASS"), _S("OPAS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PARKWAY"), _S("PKWY"), word},
   {LangTypes::english, StringTable::NEW_ZEALAND_CC, _S("PARADE"), _S("PDE"), word},
   {LangTypes::english, StringTable::NEW_ZEALAND_CC, _S("PARKWAY"), _S("PKY"), word},
   {LangTypes::english, StringTable::AUSTRALIA_CC, _S("PARADE"), _S("PDE"), word},
   {LangTypes::english, StringTable::AUSTRALIA_CC, _S("PARKWAY"), _S("PKY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PLACE"), _S("PL"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PLAIN"), _S("PLN"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PLAINS"), _S("PLNS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PLAZA"), _S("PLZ"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PINE"), _S("PNE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PINES"), _S("PNES"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PRAIRIE"), _S("PR"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PROMENADE"), _S("PROM"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PORT"), _S("PRT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PASSAGE"), _S("PSGE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("POINT"), _S("PT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PATHWAY"), _S("PTWY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("PRIVATE"), _S("PVT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("RADIAL"), _S("RADL"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("ROAD"), _S("RD"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("RIDGE"), _S("RDG"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("RANCH"), _S("RNCH"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("RANGE"), _S("RNGE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("RAPID"), _S("RPD"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("RAPIDS"), _S("RPDS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("REST"), _S("RST"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SHOAL"), _S("SHL"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SHOALS"), _S("SHLS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SHORE"), _S("SHR"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SHORES"), _S("SHRS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SKYWAY"), _S("SKWY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SUMMIT"), _S("SMT"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SPRING"), _S("SPG"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SPRINGS"), _S("SPGS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SQUARE"), _S("SQ"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SQUARES"), _S("SQS"), word},
   {LangTypes::english, StringTable::ENGLAND_CC, _S("SAINT"), _S("ST"), word},   // first word in string
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("STREET"), _S("ST"), word}, // last word in string
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("STATION"), _S("STA"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("STREAM"), _S("STRM"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("SUBDIVISION"), _S("SUBD"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TERRACE"), _S("TER"), word},
   {LangTypes::english, StringTable::USA_CC, _S("TRAFFICWAY"), _S("TRFY"), word},
   {LangTypes::english, StringTable::ENGLAND_CC, _S("TRAFFICWAY"), _S("TFWY"), word},
   {LangTypes::english, StringTable::IRELAND_CC, _S("TRAFFICWAY"), _S("TFWY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("THICKET"), _S("THKT"), word},
   {LangTypes::english, StringTable::USA_CC, _S("THROUGHWAY"), _S("TRWY"), word},
   {LangTypes::english, StringTable::ENGLAND_CC, _S("THROUGHWAY"), _S("THWY"), word},
   {LangTypes::english, StringTable::IRELAND_CC, _S("THROUGHWAY"), _S("THWY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TURNPIKE"), _S("TPKE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TRACK"), _S("TRAK"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TRACE"), _S("TRCE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TRAIL"), _S("TRL"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TRAILER"), _S("TRLR"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("TUNNEL"), _S("TUNL"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("UNION"), _S("UN"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("UNIONS"), _S("UNS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("UNDERPASS"), _S("UNPS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("UNDERPASS"), _S("UPAS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("VIADUCT"), _S("VIA"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("VISTA"), _S("VIS"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("VILLE"), _S("VL"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("VILLAGE"), _S("VLG"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("VILLAGE"), _S("VLGE"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("VALLEY"), _S("VLY"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("VIEW"), _S("VW"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("WHARF"), _S("WHRF"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("WELL"), _S("WL"), word},
   {LangTypes::english, StringTable::NBR_COUNTRY_CODES, _S("CROSSING"), _S("XING"), word},
   
   // Dutch
   {LangTypes::dutch, StringTable::NBR_COUNTRY_CODES, _S("DREEF"), _S("DR"), word},
   {LangTypes::dutch, StringTable::NBR_COUNTRY_CODES, _S("LAAN"), _S("LN"), word},
   {LangTypes::dutch, StringTable::NBR_COUNTRY_CODES, _S("PLEIN"), _S("PL"), word},
   {LangTypes::dutch, StringTable::NBR_COUNTRY_CODES, _S("STRAAT"), _S("STR"), word},
   
   // German
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("DOKTOR"), _S("DR"), word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("GASSE"), _S("G"), word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("PLATZ"), _S("PL"), word},  //
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("PLATZ"), _S("PLZ"), word}, //
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("STRASSE"), _S("STR"),word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("BRÜCKE"), _S("BR"), word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("MARKT"), _S("MKT"), word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("PROMENADE"), _S("PROM"), word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("RING"), _S("RG"), word},
   {LangTypes::german, StringTable::NBR_COUNTRY_CODES, _S("STIEGE"), _S("STG"), word},
  
   // French
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("ALLÉE"), _S("ALL"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("AVENUE"), _S("AE"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("AVENUE"), _S("AV"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("BOULEVARD"), _S("BD"), word},   //
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("BOULEVARD"), _S("BLVD"), word}, //
   {LangTypes::french, StringTable::CANADA_CC, _S("BOULEVARD"), _S("BOUL"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("CHEMIN"), _S("CHE"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("CHEMIN"), _S("CH"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("CROIX"), _S("CRX"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("DESCENTE"), _S("DSC"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("DOMAINE"), _S("DOM"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("IMPASSE"), _S("IMP"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("PLACE"), _S("PL"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("PROMENADE"), _S("PROM"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("RÉSIDENCE"), _S("RES"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("ROND-POINT"), _S("RPT"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("ROUTE"), _S("RTE"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("VILLAGE"), _S("VGE"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("SAINT"), _S("ST-"), anywhere},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("SAINTE"), _S("ST-"), anywhere},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("SAINT"), _S("ST"), word},
   {LangTypes::french, StringTable::NBR_COUNTRY_CODES, _S("SAINTE"), _S("ST"), word},


   // Danish
   // (NT street type abbrevs for Europe 2000)
   {LangTypes::danish, StringTable::NBR_COUNTRY_CODES, _S("VEJ"), _S("V"), end},   //
   {LangTypes::danish, StringTable::NBR_COUNTRY_CODES, _S("VEJEN"), _S("V"), end}, //
   {LangTypes::danish, StringTable::NBR_COUNTRY_CODES, _S("GADE"), _S("G"), end},
   {LangTypes::danish, StringTable::NBR_COUNTRY_CODES, _S("VEJ"), _S("V"), word},
   {LangTypes::danish, StringTable::NBR_COUNTRY_CODES, _S("GADE"), _S("G"), word},
   
   // Norwegian
   // Find veien when searching for vegen etc
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("VEIEN"), _S("VEGEN"), end},
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("VEGEN"), _S("VEIEN"), end},
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("VEIEN"), _S("V"), end},    //
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("VEGEN"), _S("V"), end},    //
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("VEIEN"), _S("VN"), end},   //
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("VEGEN"), _S("VN"), end},   //
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("GATA"), _S("G"), end},
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("GATA"), _S("GT"), end},
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("GATE"), _S("G"), word},
   {LangTypes::norwegian, StringTable::NBR_COUNTRY_CODES, _S("GATE"), _S("GT"), word},
   
};


const uint32
AbbreviationTable::m_nbrExpansions = 
   sizeof(m_expansions_orig) / sizeof(abbreviation_t);

const vector<AbbreviationTable::abbreviation_tommi_t>
AbbreviationTable::m_expansions =
convAbbrev(m_expansions_orig,
           sizeof(m_expansions_orig) / sizeof(abbreviation_t));

vector<AbbreviationTable::abbreviation_tommi_t>
AbbreviationTable::convAbbrev(const abbreviation_t* table, int nbrAbb)
{
   // Result vector
   vector<AbbreviationTable::abbreviation_tommi_t> res( nbrAbb );
   
   for( int i = 0; i < nbrAbb; ++i ) {
      res[i] = table[i];
   }
   return res;
}


// ------------------------------------------------------- m_areaExpansions
// 

const AbbreviationTable::abbreviation_t
AbbreviationTable::m_areaExpansions[] = 
{
   {LangTypes::swedish, StringTable::SWEDEN_CC, _S("STOCKHOLM"), _S("STHLM"), word},
   {LangTypes::swedish, StringTable::SWEDEN_CC, _S("GÖTEBORG"), _S("GBG"), word},
   {LangTypes::swedish, StringTable::SWEDEN_CC, _S("HELSINGBORG"), _S("HBG"), word},
   
   {LangTypes::french, StringTable::FRANCE_CC, _S("SAINT"), _S("ST-"), anywhere},
   {LangTypes::french, StringTable::FRANCE_CC, _S("SAINTE"), _S("ST-"), anywhere},
   {LangTypes::french, StringTable::FRANCE_CC, _S("SAINT"), _S("ST"), word},
   {LangTypes::french, StringTable::FRANCE_CC, _S("SAINTE"), _S("ST"), word},
   {LangTypes::german, StringTable::AUSTRIA_CC, _S("SANKT"), _S("ST"), anywhere},
};

const uint32
AbbreviationTable::m_nbrAreaExpansions = 
   sizeof(m_areaExpansions) / sizeof(abbreviation_t);

// --------------------------------------------------------------- methods

bool
AbbreviationTable::abbrevMatches(const char* origStr,
                                 const char* foundPos,
                                 const char* foundAbbrev,
                                 pos_t abbrevPos,
                                 uint32 abbrevNbrInTable,
                                 bool toAbbreviate,
                                 bool singleWordExpand,
                                 bool manageAddress)
{
   int len = strlen(origStr);
   int abbrevLen = strlen(foundAbbrev);
   
   // TODO?: Handle special cases of forbidden expansions, e.g.:
   // swedish: Torg -> Torgatan  (10-90% that the expansion is allowed)
   // english: N <-> North  (expand: N is the name, not a directional)
   //                       (abbrev: North is the name, not a dir..)
   
   // Check where the match is located
   switch ( abbrevPos ) {
      case beginning: {
         const char* prevPos = foundPos - 1;
         if ( (foundPos == origStr) ||    // begin-in-beginning
              (*prevPos == ' ') ) {       // begin-anywhere-in-origStr
            mc2dbg8 << "origStr=\"" << origStr << "\" fPos=\"" << foundPos
                    << "\" fAbbr=\"" << foundAbbrev << "\" - abbrev beginning"
                    << endl;
            // Check that there is no blank after foundAbbrev in origStr
            // or that foundAbbrev is not in the end of origStr
            // (beginning is not word)
            const char* nextPos = foundPos + abbrevLen;
            if ( (*nextPos != ' ') &&
                 ((foundPos + abbrevLen) != (origStr + len)) ) {
                  return true;
            } else {
               mc2dbg8 << "beginning not word (nextPos=\"" << nextPos
                       << " *nP=\"" << *nextPos << "\")" << endl;
            }
         }
         break;
      }
      case end: {
         const char* nextPos = foundPos + abbrevLen;
         if ( ((foundPos + abbrevLen) == (origStr + len)) ||   // end-in-end
              (*nextPos == ' ') ||                             // end-anywhere
              (*nextPos == '.') ) {                // end-anywhere-with-dot (.)
            mc2dbg8 << "origStr=\"" << origStr << "\" fPos=\"" << foundPos
                    << "\" fAbbr=\"" << foundAbbrev << "\" - abbrev end" << endl
                    << " abbrevLen " << abbrevLen << " len " << len 
                    << ", \"" << foundPos + abbrevLen << "\" == \"" 
                    << origStr + len << "\"" << endl;
            // Check that there is no blank before foundPos or that foundPos
            // is not in the begining of origStr (end is not word)
            // In "Södra vägen", "vägen" is not end!
            const char* prevPos = foundPos - 1;
            if ( (*prevPos != ' ') && (foundPos != origStr) ) {
               return true;
            } else {
               mc2dbg8 << "end not word (prevPos=\"" << prevPos 
                       << "\" *pP=\"" << *prevPos << "\")" << endl;
            }
         }
         break;
      }
      case beginningAndEnd:
         return abbrevMatches(origStr, foundPos, foundAbbrev, beginning,
                              abbrevNbrInTable, toAbbreviate,
                              singleWordExpand, manageAddress)
            || abbrevMatches(origStr, foundPos, foundAbbrev, end, 
                             abbrevNbrInTable, toAbbreviate,
                             singleWordExpand, manageAddress);
         break;
      case centre:
         return !abbrevMatches(origStr, foundPos, foundAbbrev, beginningAndEnd,
                               abbrevNbrInTable, toAbbreviate,
                               singleWordExpand, manageAddress);
         break;
      case anywhere:
         mc2dbg4 << " origStr=\"" << origStr << "\" fPos=\"" << foundPos
                 << "\" fAbbr=\"" << foundAbbrev << "\" - abbrev anywhere" 
                 << endl;
         return true;
         break;
      case word:
         
         mc2dbg4 << " origStr=\"" << origStr << "\" fPos=\"" << foundPos
                 << "\" fAbbr=\"" << foundAbbrev << "\" - word"
                 << endl;
         // NB! If foundPos == origStr then prevPos is illegal!
         const char* prevPos = foundPos - 1;
         const char* nextPos = foundPos + abbrevLen;

         // word if:
         // prevPos = ' ' or if foundPos is the beginning of origStr
         //  and
         // nextPos = ' ' or '.' or if foundPos (foundAbbrev) is end of origStr
         if ( ( (foundPos == origStr)  || (*prevPos == ' ') ) && 
              ((*nextPos == ' ') || (*nextPos == '.') ||
               (strlen(foundPos) == strlen(foundAbbrev))) ) {
            mc2dbg4 << " - abbrev word" << endl;

            // Exception: expanding address 
            // long origStr + swedish + word: v ->väg|västra
            if (manageAddress && 
                !singleWordExpand &&
                !toAbbreviate && 
                (m_expansions[abbrevNbrInTable].lang == LangTypes::swedish) && 
                ( strcmp(foundAbbrev, "V") == 0 )) {
               // VÄG is never in the beginning of origStr
               if ( (foundPos == origStr) &&
                    (m_expansions[abbrevNbrInTable].fullString ==
                     _S("VÄG")) ) {
                  return false;
               }
               // VÄSTRA is never in the end of origStr
               if ( (*nextPos == '\0') &&
                    (m_expansions[abbrevNbrInTable].fullString ==
                     _S("VÄSTRA")) ) {
                  return false;
               }
            }
            
            return true;
         }
        
         return false;
         break;
   }
   return false;
}

bool
AbbreviationTable::abbrevAllowed(pos_t inPos, pos_t tablePos,
                                 bool toAbbreviate)
{
   return true;
}

char*
AbbreviationTable::abbreviate(const char* src, char* dest, 
                              LangTypes::language_t lang,
                              AbbreviationTable::pos_t pos,
                              StringTable::countryCode country)
{
   //static const char illegalChar = 'X'; // Good when debugging.
   static const char illegalChar = '\001';
   
   size_t srclen = strlen(src);
   strcpy( dest, src );
   
   if(srclen < STR_LIMIT) {            // String too short to abbreviate
      return dest;
   }

   // True if the input string is only one word
   const bool oneWord =
      strchr( StringUtility::trimStartEnd(dest).c_str(), ' ') == NULL;

   char* srcCpy = StringUtility::newStrDup(src);
   
   MC2String usrc_str = StringUtility::copyUpper( srcCpy );
   const char* usrc   = usrc_str.c_str();

   
   for(uint32 i = 0; i < m_nbrAbbreviations; i++) {

      if ( m_abbreviations[i].pos == word && oneWord ) {
         // Shouldn't shorten one word.
         continue;
      }
      
      if(m_abbreviations[i].lang != lang) {
         continue;
      }

      if ( (country != StringTable::NBR_COUNTRY_CODES) &&
           (m_abbreviations[i].country != StringTable::NBR_COUNTRY_CODES) &&
           (country != m_abbreviations[i].country) ) {
         continue;
      }
      
      const MC2String& abbrev = m_abbreviations[i].abbreviation;
      const MC2String& fullstr = m_abbreviations[i].fullString;
      
      const int fullLen = fullstr.length();
      const int abbrevLen = abbrev.length();
      if ( (strlen(dest) - fullLen + abbrevLen) < STR_LIMIT) {
         continue;
      }
      
      char* posInStr = strstr( const_cast<char*>( usrc ), fullstr.c_str());
      if ( posInStr == NULL ) {
         // No match
         continue;
      }
      
      if ( !abbrevMatches(usrc, posInStr, fullstr.c_str(),
                          m_abbreviations[i].pos, i, true ) ) { //toAbbreviate
                          // noSingleWordExpand, no manageAddress
         continue;
      }
      
      if ( ! abbrevAllowed(pos, m_abbreviations[i].pos, true ) ) {
         continue;
      }

      // All checks passed - exchange shortened chars for ASCII 1
      // String must become shorter for this to work.

      // Start position in original strCpy 
      char* origPtr = srcCpy + (posInStr - usrc);
      mc2dbg4 << " srcCpy=\"" << srcCpy << "\" posInStr=\"" << posInStr
              << "\" usrc=\"" << usrc << "\" ->origPtr=\"" << origPtr << "\""
              << endl;
      for( char* c = posInStr; c < posInStr + fullLen; ++c ) {
         *c = illegalChar;
         *origPtr++ = illegalChar;
      }
      mc2dbg4 << " origPtr1=\"" << origPtr << "\"" << endl;
      // Reset the source pointer.
      origPtr = srcCpy + (posInStr - usrc);
      mc2dbg4 << " srcCpy=\"" << srcCpy << "\" posInStr=\"" << posInStr
              << "\" usrc=\"" << usrc << "\" ->origPtr=\"" << origPtr << "\""
              << endl;
      // Put in the abbrevation in srcCpy
      memcpy(origPtr, abbrev.c_str(), abbrevLen);
      mc2dbg4 << " origPtr2=\"" << origPtr << "\"" << endl;
      mc2dbg4 << " -> usrc=\"" << usrc << "\" posInStr=\"" << posInStr 
           << "\" srcCpy=\"" << srcCpy << "\"" << endl;
   }
   
   mc2dbg8 << "[AT]: srcCpy = " << MC2CITE(srcCpy)
           << " dest = " << MC2CITE(dest) << " usrc = "
           << MC2CITE(usrc) << endl;
   
   int srcCpyLen = strlen(srcCpy);
   int destPos = 0;

   // Copy including ending \0.
   for( int i = 0; i < srcCpyLen + 1; ++i ) {
      if ( srcCpy[i] != illegalChar ) {
         dest[destPos++] = srcCpy[i];
      }
   }
   mc2dbg4 << " --> usrc=\"" << usrc << "\" srcCpy=\"" 
           << srcCpy << "\"" << endl;
   
   delete [] srcCpy;
   
   return dest;
}


char*
AbbreviationTable::expand(const char* src, char* dest, size_t maxLen,
                          LangTypes::language_t lang,
                          AbbreviationTable::pos_t expandPos,
                          StringTable::countryCode country,
                          bool manageAddress)
{
   // Add a singleWordExpansion-param to this function?

   vector<MC2String> expandedStrings;
   dest = helpExpand( src, dest, maxLen, lang, country,
                      expandPos, false, expandedStrings,
                      manageAddress );
   
   return dest;
}

char*
AbbreviationTable::helpExpand(const char* src, char* dest, size_t maxLen,
                              LangTypes::language_t lang,
                              StringTable::countryCode country,
                              AbbreviationTable::pos_t expandPos,
                              bool singleWordExpand,
                              vector<MC2String>& expandedStrings,
                              bool manageAddress)
{
   // for single word
   bool stringWasExpanded = false;

   // Expand address or area
   uint32 nbrExpansionsInTable = m_nbrExpansions;
   if (!manageAddress)
      nbrExpansionsInTable = m_nbrAreaExpansions;
   
   
   strncpy( dest, src, maxLen );
   dest[ maxLen ] = 0;

   vector<char*> deleteTheseLater; // contains char* to be deleted later

   char* srcCpy = StringUtility::newStrDup(src);
   char* usrc   = StringUtility::newStrDup(
      StringUtility::copyUpper(MC2String(srcCpy)).c_str() );
   char udest[maxLen + 1];
   strncpy( udest, usrc, maxLen );
   udest[ maxLen ] = 0;
   for(uint32 i = 0; i < nbrExpansionsInTable; i++) {
      // Assuming that there always are more expansions in the
      // address expansions table compared to the area expansions table
      MC2String fullstr = m_expansions[i].fullString;
      MC2String abbrev = m_expansions[i].abbreviation;
      
      if (!manageAddress) {
         fullstr = m_areaExpansions[i].fullString;
         abbrev = m_areaExpansions[i].abbreviation;
      }
      
      if ( (strlen(dest) - abbrev.length() + fullstr.length()) > maxLen) {
         // String to create too long for the preallocated dest.
         continue;
      }
      
      if( manageAddress && (m_expansions[i].lang != lang) ) { 
         continue;
      }
      if( !manageAddress && 
          (lang != LangTypes::invalidLanguage) && 
          (m_areaExpansions[i].lang != lang) ) {
         continue;
      }
      // This was a temp fix here for expanding areas (bua+municipal), 
      // since there are no native languages in the overviewMap
      
      if ( manageAddress &&
           (country != StringTable::NBR_COUNTRY_CODES) &&
           (m_expansions[i].country != StringTable::NBR_COUNTRY_CODES) &&
           (country != m_expansions[i].country) ) {
         continue;
      }
      if ( !manageAddress &&
           (country != StringTable::NBR_COUNTRY_CODES) &&
           (m_areaExpansions[i].country != StringTable::NBR_COUNTRY_CODES) &&
           (country != m_areaExpansions[i].country) ) {
         continue;
      }
      
      const char* posInStr = strstr(usrc, abbrev.c_str());
      if ( posInStr == NULL ) {
         // No match
         continue;
      }
      
      // Try expanding for all occurances of abbrev in the usrc string
      pos_t tableExpandPosition;
      if (manageAddress)
         tableExpandPosition = m_expansions[i].pos;
      else
         tableExpandPosition = m_areaExpansions[i].pos;
      uint32 loopNbr = 0;
      while ( posInStr != NULL ) {
         DEBUG8(if (singleWordExpand)
               cout << " exp " << i << ":" << loopNbr
                     << " (" << abbrev << "," << fullstr << ")"
                     << " pIS=\"" << posInStr << "\"" << endl;);
         if ( abbrevMatches(usrc, posInStr, abbrev.c_str(), 
                            tableExpandPosition, i, false,   // toExpand
                            singleWordExpand, manageAddress ) ) {
         
            if ( abbrevAllowed(expandPos, tableExpandPosition, false ) ) {

               // All checks passed - expand
               //
               // beginning of origstr(srcCpy) +
               // abbrev replaced with fullstr +
               // end of origstr(srcCpy)

               const int fullLen = fullstr.length();
               const int abbrevLen = abbrev.length();
               int commonBeginnning = strlen(usrc) - strlen(posInStr);
               int posAfterAbbrev = commonBeginnning + abbrevLen;
               
               // copy common beginning
               memcpy(udest, usrc, commonBeginnning);
               // copy the expansion (fullstr)
               int fpos = 0;
               do {
                  udest[commonBeginnning] = fullstr[fpos];
                  commonBeginnning++;
                  fpos++;
               } while ( fpos < fullLen);
               // copy the common end, if any, including ending \0
               const int usrcLen = strlen(usrc);
               while ( posAfterAbbrev < usrcLen + 1) {
                  udest[commonBeginnning] = usrc[posAfterAbbrev];
                  commonBeginnning++;
                  posAfterAbbrev++;
               }
               DEBUG4(cout << "  -> usrc=\"" << usrc << "\" udest=\"" 
                           << udest << "\"" << endl;);
               
               if (singleWordExpand) {
                  // Add this expanded string to the expandedStrings vector
                  // make first in word upper
                  MC2String oneExpand = 
                     StringUtility::makeFirstInWordCapital(MC2String(udest),
                                                           true);
                  expandedStrings.push_back(MC2String(udest));// (oneExpand)
                  stringWasExpanded = true;
               }
               else {
                  deleteTheseLater.push_back( usrc );
                  usrc = StringUtility::newStrDup(udest);
              } 
            }
         }
         
         loopNbr++;
         // Find next occurrance of abbrev in the string
         const char* nextPos = posInStr + abbrev.length();
         posInStr = strstr(nextPos, abbrev.c_str());
      }
   }
   
   if ( singleWordExpand ) {
      if ( !stringWasExpanded ) {
         // no expansion was found for the origStr, insert it.
         expandedStrings.push_back(srcCpy);
      }
   } else {
      int udestLen = strlen(udest);
      int destPos = 0;
      // Copy including ending \0.
      for( int i = 0; i < udestLen + 1; ++i ) {
         dest[destPos++] = usrc[i];
      }
      // Make first in word upper.
      MC2String upperStr =
         StringUtility::makeFirstInWordCapital(MC2String(dest),
                                               true);
      dest = StringUtility::newStrDup(upperStr.c_str());
   }
   
   delete [] usrc;
   // kill all ...   
   for ( uint32 i = 0 ; i < deleteTheseLater.size(); ++i ) {
      delete [] deleteTheseLater[ i ];
   }

   delete [] srcCpy;
   
   return dest;
}
// helpExpand

vector<MC2String>
AbbreviationTable::fullExpand(const char* src, LangTypes::language_t lang,
                              StringTable::countryCode country,
                              bool manageAddress)
{
   vector<MC2String> expandedStrings;
   vector<MC2String> tempStrings;
   vector<MC2String>::iterator it;
   vector<MC2String>::const_iterator cit;

   // 1. Split the src string into words
   // 2. Expand each word individually, result 1-n matches
   // 3. Add all combinations of word-expansions to the expandedStrigns vector
   
   ScopedArray<char> srcCopy( StringUtility::newStrDup(src) );

   vector< char* > strVector;
   StringUtility::tokenListToVector( strVector, srcCopy.get(), ' ');

   uint32 maxLen = 126;
   char dest[maxLen + 1];
   char srcFixedString[ maxLen + 1 ];
   dest[0] = '\0';

   for (uint32 part = 0; part < strVector.size(); part++) {
      char* curStr = strVector[ part ];
      vector<MC2String> wordExpVector;
      strncpy( srcFixedString, curStr, maxLen );
      srcFixedString[ maxLen ] = 0;
      helpExpand( srcFixedString, dest, maxLen, lang, country, anywhere,
                  true, wordExpVector, manageAddress );
      
      // If first part, add the word expansions to the expandedStrings-vector
      if (part == 0) {
         for (cit = wordExpVector.begin(); cit != wordExpVector.end(); cit++) {
            mc2dbg8 << " adding " << *cit << " to expStrings" << endl;
            expandedStrings.push_back( *cit );
         }
      }
      // Otherwise, add the word expansions to the prior strings in the
      // expandedStrings-vector. If more than one returned word expansion, 
      // duplicate via tempStrings to get all combinations.
      else {
         tempStrings.clear();
         uint32 nbrFullExpansions = expandedStrings.size();
         uint32 expNbr = 0;
         it = expandedStrings.begin();
         vector<MC2String>::iterator curIt;
         while ( (it != expandedStrings.end()) &&
                 (expNbr < nbrFullExpansions) ) {
            curIt = it;
            MC2String curExpansion = *curIt;
            it++;
            mc2dbg8 << " expNbr " << expNbr << " " << curExpansion << endl;
            uint32 nbr = 0;
            for ( cit = wordExpVector.begin();
                  cit != wordExpVector.end(); cit++ ) {
               MC2String longerExpansion = curExpansion + " " + *cit;
               mc2dbg8 << " nbr " << nbr << " " << longerExpansion << endl;
               // Add first word expansion to curExpansion
               // ( = replace *curIt with longerExpansion)
               if (nbr == 0) {
                  *curIt = longerExpansion;
               }
               // Otherwise add a new string to the tempStrings vector
               else {
                  tempStrings.push_back( longerExpansion );
               }
               nbr++;
            }

            expNbr++;
         }

         // Add any strings in tempStrings to the expandedStrings vector
         for ( cit = tempStrings.begin(); cit != tempStrings.end(); cit++ ) {
            mc2dbg8 << " to insert (after) '" << *cit << "'" << endl;
            expandedStrings.push_back( *cit );
         }
         tempStrings.clear();
      }
      
      // TODO: Make some checks that the SAME word is not included more than
      //     once in the longerExpansions.. ? "V Hammars v" -> "Väg Hammars Väg"
      
      // debug
      DEBUG4(
         cout << " expandedStrings";
      uint32 j = 0;
      for ( cit = expandedStrings.begin();
            cit != expandedStrings.end(); cit++ ) {
         cout << " " << j << "=\"" << *cit << "\"";
         j++;
      }
      cout << endl;);
   }
   

   // Do not return results that are equal to the source string.
   vector<MC2String> result;
   for (uint32 i = 0; i<expandedStrings.size(); ++i){
      
      if ( expandedStrings[i] != MC2String(src) ){
         result.push_back( expandedStrings[i] );
      }
   }

   return result;
}

char* 
AbbreviationTable::oldExpand(const char* src, char* dest, size_t maxLen,
                          LangTypes::language_t lang,
                          AbbreviationTable::pos_t pos,
                          bool &expanded)
{
   strcpy(dest, src);

   char* usrc = StringUtility::newStrDup(
      StringUtility::copyUpper(MC2String(src)).c_str() );
   expanded = false;
   for(uint32 i = 0; i < m_nbrExpansions; i++) {
      MC2String fullstr = m_expansions[i].fullString;
      MC2String abbrev = m_expansions[i].abbreviation;
      DEBUG8(
         cerr << "usrc=\"" << usrc << "\", dest=\"" << dest 
            << "\", fullstr=\"" << fullstr
            << "\", abbrev=\"" << abbrev << endl;
            cerr << "strlen(usrc)=" << strlen(usrc)
                 << ", strlen(fullstr)=" << strlen(fullstr)
                 << ", strlen(abbrev)=" << strlen(abbrev)
                 << ", STR_LIMIT=" << STR_LIMIT << endl;
         );
      if(m_expansions[i].lang != lang) {
         continue;
         }

      char* srcCopy = StringUtility::newStrDup(usrc);

      vector< char* > strVector;
      StringUtility::tokenListToVector( strVector, srcCopy, ' ');

      switch(pos) {
         case beginning:
         {
            char* firstElement =
               StringUtility::newStrDup(strVector[ 0 ]);
            pos_t position = m_expansions[i].pos;
            if( position == word ) {
               if( strcmp(strVector[ 0 ],
                          abbrev.c_str()) == 0 ) 
               {
                  delete [] firstElement;
                  break;
               }
               else {
                  memcpy(dest, fullstr.c_str(), fullstr.length());
                  size_t rpos = abbrev.length();
                  size_t wpos = fullstr.length();
                  do {
                     dest[wpos] = usrc[rpos];
                     ++wpos; ++rpos;
                  } while(usrc[rpos-1] != '\0');
                  expanded = true;
               }
            }
            else if( (position == beginning) || (position == anywhere) ||
                     (position == beginningAndEnd) )
            {
               char* hit = strstr(firstElement, abbrev.c_str());
               if( (hit != NULL) && ( hit == firstElement) ) {
                  memcpy(dest, fullstr.c_str(), fullstr.length());
                  size_t rpos = abbrev.length();
                  size_t wpos = fullstr.length();
                  do {
                     dest[wpos] = usrc[rpos];
                     ++wpos; ++rpos;
                  } while(usrc[rpos-1] != '\0');
                  expanded = true;
               }
            }

            // else if position == end ?
            
            delete [] firstElement;
            break;
         }
         case end:
         {
            if(usrc[strlen(usrc)-1] != abbrev[abbrev.length()-1]) {
               break;   // optimization
            }
            char* lastElement =
               StringUtility::newStrDup(strVector[ strVector.size() - 1 ]);
            pos_t position = m_expansions[i].pos;
            if( position == word ) {
               if( !strcmp(lastElement, abbrev.c_str()) ) {
                  delete [] lastElement;
                  lastElement = StringUtility::newStrDup(fullstr.c_str());
                  
                  MC2String s;
                  uint32 i = 0;
                  for(i = 0; i < strVector.size(); i++) {
                     if( i != (strVector.size()-1) ) {
                        s += strVector[ i ];
                        s += " ";
                     }
                     else {
                        s += lastElement;
                     }
                  }
                  for(i = 0; i < s.size(); i++)
                     dest[i] = s[i];
                  dest[s.size()] = '\0';
                  expanded = true;
               }
            }
            else if( (position == end) || (position == anywhere) ||
                     (position == beginningAndEnd) )
            {
               char* hit = strstr(lastElement, abbrev.c_str());
               if( hit != NULL ) {
                  size_t eoffs = abbrev.length();
                  char* addr = &dest[strlen(dest)] - eoffs;
                  memcpy(addr, fullstr.c_str(), fullstr.length());
                  addr[fullstr.length()] = '\0';
                  expanded = true;
               }
            }

            // else if position == beginning ?
            
            delete [] lastElement;
         }
         break;
         
         case word:
         {
            // Not used...
            break;
         }
         
         case beginningAndEnd:
         {
            char tempDest[1024];
            expanded = false;
            
            // Same as beginning
            char* firstElement =
               StringUtility::newStrDup(strVector[ 0 ]);
            pos_t position = m_expansions[i].pos;
            if( position == word ) {
               if( !strcmp(strVector[ 0 ],
                           abbrev.c_str()) ) 
               {
                  memcpy(tempDest, fullstr.c_str(), fullstr.length());
                  size_t rpos = abbrev.length();
                  size_t wpos = fullstr.length();
                  do {
                     tempDest[wpos] = usrc[rpos];
                     ++wpos; ++rpos;
                  } while(usrc[rpos-1] != '\0');
                  expanded = true;
               }
            }
            else if( (position == beginning) || (position == anywhere) ||
                     (position == beginningAndEnd) )
            {
               char* hit = strstr(firstElement, abbrev.c_str());
               if( (hit != NULL) && ( hit == firstElement) ) {
                  memcpy(tempDest, fullstr.c_str(), fullstr.length());
                  size_t rpos = abbrev.length();
                  size_t wpos = fullstr.length();
                  do {
                     tempDest[wpos] = usrc[rpos];
                     ++wpos; ++rpos;
                  } while(usrc[rpos-1] != '\0');
                  expanded = true;
               }
            }
            if( expanded ) {
               strcpy(dest, tempDest);
               delete [] srcCopy;
               srcCopy = StringUtility::newStrDup(tempDest);
               StringUtility::tokenListToVector( strVector,
                                                 srcCopy,
                                                 ' ' );
            }
            delete [] firstElement;
            
            // Almost the same as for end
            if(usrc[strlen(usrc)-1] != abbrev[abbrev.length()-1]) {
               break;   // optimization
            }
            
            char* lastElement =
               StringUtility::newStrDup(strVector[ strVector.size() - 1 ]);
            position = m_expansions[i].pos;
            if( position == word ) {
               if( !strcmp(lastElement, abbrev.c_str()) ) {
                  delete [] lastElement;
                  lastElement = StringUtility::newStrDup(fullstr.c_str());
     
                  MC2String s;
                  uint32 i = 0;
                  for(i = 0; i < strVector.size(); i++) {
                     if( i != (strVector.size()-1) ) {
                        s += strVector[ i ];
                        s += " ";
                     }
                     else {
                        s += lastElement;
                     }
                  }
                  for(i = 0; i < s.size(); i++)
                     dest[i] = s[i];
                  dest[s.size()] = '\0';
                  expanded = true;
               }
            }
            else if( (position == end) || (position == anywhere) ||
                     (position == beginningAndEnd) )
            {
               char* hit = strstr(lastElement, abbrev.c_str());
               if( hit != NULL ) {
                  size_t eoffs = abbrev.length();
                  char* addr = &dest[strlen(dest)] - eoffs;
                  memcpy(addr, fullstr.c_str(), fullstr.length());
                  addr[fullstr.length()] = '\0';
                  expanded = true;
               }
            }
            delete [] lastElement;
            break;
         }

         case centre: 
         {
            MC2String s;
            pos_t position = m_expansions[i].pos;
            bool update = false;

            char* destCopy = StringUtility::newStrDup(dest);
            vector< char* > sv;
            StringUtility::tokenListToVector( sv, destCopy, ' ');
            
            for(uint32 i = 0; i < sv.size(); i++) {
               char* currString = sv[ i ];
               if( position == word ) {
                  if( (!strcmp(currString, abbrev.c_str())) &&
                      ( i > 0 ) && ( i < sv.size()-1) )
                  {
                     s += fullstr.c_str();
                     if(i != (sv.size() - 1) )
                        s += ' ';
                     update = true;
                     expanded = true;
                  }
                  else {
                     s += currString;
                     if(i != (sv.size() - 1) )
                        s += ' ';
                  }
               }
               else if( (position == anywhere) ||
                        (position == centre) )
               {
                  char* hit = strstr(currString, abbrev.c_str());
                  if( (hit != NULL) && (i > 0) && (i < sv.size()-1) ) {
                     memcpy(currString, fullstr.c_str(), fullstr.length());
                     size_t rpos = abbrev.length();
                     size_t wpos = fullstr.length();
                     do {
                        currString[wpos] = currString[rpos];
                        ++wpos; ++rpos;
                        s += currString;   }
                     while(currString[rpos-1] != '\0');
                     update = true;
                     expanded = true;
                  }
               }

               // else if position == beginning end ?
            }
            if( update ) {
               for(uint32 i = 0; i < s.size(); i++) {
                  dest[i] = s[i];
               }
               dest[s.size()] = '\0';
            }
            delete [] destCopy;
            break;
         }

         case anywhere:
         {
            MC2String s;
            pos_t position = m_expansions[i].pos;
            bool update = false;
            
            char* destCopy = StringUtility::newStrDup( dest );
            vector< char* > sv;
            StringUtility::tokenListToVector( sv, destCopy, ' ' );
            
            for(uint32 i = 0; i < sv.size(); i++) {
               char* currString = sv[ i ];
               if( position == word ) {
                  if( !strcmp(currString, abbrev.c_str()) ) {
                     s += fullstr.c_str();
                     if(i != (sv.size() - 1) )
                        s += ' ';
                     update = true;
                     expanded = true;
                  }
                  else {
                     s += currString;
                     if(i != (sv.size() - 1) )
                        s += ' ';
                  }
               }
               
               else if( position == anywhere ) {
                  char* hit = strstr(currString, abbrev.c_str());
                  if( hit != NULL ) {
                     memcpy(currString, fullstr.c_str(), fullstr.length());
                     size_t rpos = abbrev.length();
                     size_t wpos = fullstr.length();
                     do {
                        currString[wpos] = currString[rpos];
                        ++wpos; ++rpos;
                        s += currString;   }
                     while(currString[rpos-1] != '\0');
                     update = true;
                     expanded = true;
                  }
               }

               // else if position == beginning end beginningAndEnd ?
            }
            if( update ) {
               for(uint32 i = 0; i < s.size(); i++) {
                  dest[i] = s[i];
               }
               dest[s.size()] = '\0';
            }
            delete [] destCopy;
            break;
         }               
         default:
         {
            mc2log << warn << here << "Expanding in position "
                   << uint32(pos) << " not implemented." << endl;
            break;
         }
      }

      delete [] srcCopy;

      if( expanded ) {
         delete [] usrc;
         usrc = StringUtility::newStrDup(dest);
      }
   }
   delete [] usrc;

   return (dest); 
}


int
AbbreviationTable::getExpandPosition(const char* src, uint32 i,
                                     AbbreviationTable::pos_t pos)
{
   int retVal = -1;
   switch (pos) {
      case (beginning):
      {
         if ( strncmp_alnum(src, m_expansions[i].abbreviation.c_str(), 
                            (m_expansions[i].abbreviation.length())) == 0) {
            // Found a match
            retVal = 0;
         }
         break;
      }

      case (anywhere): 
      {
         const char* pos = NULL;
         if ( (pos = strstr(src,
                            m_expansions[i].abbreviation.c_str())) != NULL) {
            retVal = pos-src;
         }
      break;
      }
   
      case (word): 
      {
         const char* pos = NULL;
         if ((pos = strstr(src,
                           m_expansions[i].abbreviation.c_str())) != NULL) {
            int32 ablen = m_expansions[i].abbreviation.length();
            int32 srclen = strlen(src);
            int32 pindex = pos-src;
            int32 pend   = pindex + ablen;
            if ((pindex == 0) /*beginning of MC2String*/ ||
                ((pindex > 0) &&
                 (isspace(pos[-1])) &&
                 (isspace(pos[pend+1]))) /*word surrounded by isspace*/ ||
                ((pindex > 0) &&
                 (isspace(pos[-1])) &&
                 (pos[pend] == '\0') &&
                 (pend == srclen)))
            {
               retVal = pos-src;
            }
         }
         break;
      }
      
      case (end):
      {
         const char* pos = NULL;
         if ((pos = strstr(src,
                           m_expansions[i].abbreviation.c_str())) != NULL) {
            int32 ablen = m_expansions[i].abbreviation.length();
            int32 srclen = strlen(src);
            if ((pos-src) == (srclen-ablen)) {
               retVal = pos - src;
            }
         }
         break;
      }
      
      default :
         DEBUG1(cerr << "   ::getExpandPosition(), " 
                     << (uint32) m_expansions[i].pos 
                     << " not implemented" << endl);
         break;
   }
   
   // Return the position of the abbreviation number i in src
   return (retVal);
}

int 
AbbreviationTable::strncmp_alnum(const char* s1, const char* s2, size_t n)
{
   // Check the inparameters
   if ((s1 == NULL) || (s2 == NULL)) {
      return (0);
   }

   uint32 pos = 0;
   while ( ( (s1[pos] == s2[pos]) || 
             ( (s1[pos] == m_nonAlnumChar) && 
               (StringUtility::isAlNum(s2[pos]) == 0)) ||
             ( (s2[pos] == m_nonAlnumChar) && 
               (StringUtility::isAlNum(s1[pos]) == 0))
           ) && (pos < n) && (s1[pos] != '\0') && (s2[pos] != '\0') ) {
      pos++;
   }

   if (pos >= n)
      return (0);
   else if (s1[pos] > s2[pos])
      return (-1);
   else 
      return (1);

}

