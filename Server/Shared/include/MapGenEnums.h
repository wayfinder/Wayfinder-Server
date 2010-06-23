/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPGENENUMS_H
#define MAPGENENUMS_H


/**
 *   Class used for storing map generation related enums.
 *
 */
class MapGenEnums {
 public:


   /** Enum used for identifying a map release version. 
    *
    *  Versions from all map suppliers are stored in the same enum. 
    *  It is important that mapversions from a single map supplier are
    *  located in the enum so more recent versions always comes after
    *  less recent versions. 
    *
    *  NB! The values in this enum corresponds to the values of the table
    *      EDVersion in the POI database. Make sure to keep it up to date.
    *
    */
   enum mapVersion {
      invMapVersion = 0,
      TA_2002_2,
      TA_2003_1,
      TA_2003_2,
      TA_2003_3,
      TA_2004_1,
      TM_2004_08,
      TM_2004_09,
      TA_2004_2,
      TM_2004_12,
      TA_2005_1,
      GDT_2005_05,
      TA_2005_2,
      TM_2005_08,
      TA_200508_CityMaps,              // test
      AND_2005_H1,
      Demo_Jiwire_London_200509,       // test
      MapInfo_2005_09,                 // MapInfo supplier not defined yet.
      TA_2005_3                  = 20, // (give room for inter TA releases)
      AND_2005_H2,
      Infotech_200512,
      AND_2005_H2_lev6,                // to separate level2 from level6
      TA_2006_01                 = 30,
      AND_2006_H1,
      TM_2006_2, 
      Infotech_200602,
      AND_2006_H1_lev2,
      Monolit_200603,
      TA_2006_02,
      TA_2006_04                 = 40,
      Monolit_200605,
      Infotech_200606,
      NT_200607_smpl,
      TA_2006_07                 = 50,
      TM_2006_3,
      Spinfo_200608,
      AND_2006_H2_lev2,
      TA_2006_09,
      TA_2006_10                 = 60,
      TA_2006_11,
      AND_2006_H2,
      TA_2007_01                 = 70,
      Monolit_2007_02,
      TA_2007_02,
      CEInfoSystems_200702,
      TM_2006_4,
      CEInfoSystems_200704,
      TA_2007_04                 = 80,
      CEInfoSystems_200705,
      CEInfoSystems_200705_2,
      GISrael_200705,
      Monolit_200706,
      TM_2007_1,
      TA_2007_05,
      TA_2007_07                 = 90,
      GISrael_200706,
      CEInfoSystems_200707,
      DMapas_200708,
      DMapas_200709,
      TA_2007_09,
      TA_2007_10                 = 100,
      DMapas_200710,
      TM_2007_2,
      DMapas_200711,
      NT_200712,
      CEInfoSystems_200710,
      CEInfoSystems_200712,
      TA_2007_12,
      NT_2007_Q2,
      TA_2008_01                 = 110,
      Monolit_200801,
      GISrael_200801,
      TM_2008_1,
      TA_2008_v1_31,    // southern africa
      DMapas_200802,
      TA_2008_02,
      DMapas_200803,
      TA_2008_v8_0,  // brazil
      NT_2007_Q4,
      TA_2008_03,
      TA_2008_04                 = 130,
      NavTurk_200805,
      DMapas_200805,
      CEInfoSystems_200805,
      TA_2008_06,
      TA_2008_05,  // argentina connect
      TA_2008_07                 = 140,
      Carmenta_200809,  // nice world maps
      TA_2008_09,
      CEInfoSystems_200811,
      TA_2008_10                 = 150,
      NT_2008_Q3                 = 151,  // Australia New Zealand
      TA_2009_01                 = 152,
      TA_2009_02                 = 160,
      TA_2009_03                 = 161,
      CEInfoSystems_200905       = 162,
      TA_2009_06                 = 170,
      TA_2009_07                 = 171,
      TA_2009_08                 = 172,
      TA_2009_09                 = 180,
      TA_2009_10                 = 181,
      TA_2009_11                 = 182,
      TA_2009_12                 = 190,
      OSM_201005                 = 191,
      TA_2010_06                 = 192,



      // Don't forget to update:
      // 1) EDVersion table in WASP database
      // 2) MapGenUtil::getMapVerFromString
   };

   /*
    * Map supplier IDs.
    * Note: Do not change the order of these or the number for these. The copyright
    * image handling depends on these numbers.
    */
   enum mapSupplier {
      unknownSupplier = 0,
      NavTech = 1,   ///< TA competitor.
      TeleAtlas = 2, ///< Main Wayfinder supplier (2005)
      AND = 3,       ///< Other Tele Atlas and NAVTEQ competitor with e.g. world data
      TopMap = 4,    ///< Hungarian maps.
      GDT = 5,       ///< Brazil maps.
      Infotech = 6,  ///< Turkey maps.
      Monolit = 7,   ///< Slovenia
      Spinfo = 8,    ///< India pedestrian maps
      CEInfoSystems = 9,    ///< India maps
      GISrael = 10,   ///< Israeli maps
      DMapas = 11,    ///< Chile, Argentina maps
      NavTurk = 12,   ///< Turkey maps
      Carmenta = 13,    ///< Non-navigable world data
      OpenStreetMap = 14,    ///< Open Street Map

      // When adding, don't forget to update 
      // MC2MapGenUtil::mapSupCopyrigthStrings
      // MapGenUtil::getMapSupFromString
      // map_supplier_names.xml

      nbrMapSuppliers  // keep last
   }; 
   
};

#endif // MAPGENENUMS_H
