/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "CommandlineOptionHandler.h"

#include "GMSGfxData.h"
#include "StringUtility.h"
#include "OldItem.h"
#include "TimeUtility.h"



// Method used in createMidMifAreaFiles
void printMid( MC2String fileName, MC2String buffer,
               bool appendComma, bool lastPartOfLine)
{
   ofstream file( (fileName+".mid").c_str(), ios::app);
   file << buffer;
   if ( appendComma ) {
      file << ",";
   }
   if ( lastPartOfLine ) {
      file << endl;
   }
}

// Method used in createMidMifAreaFiles
void printMif( MC2String fileName, GMSGfxData* gfx,
               CoordinateTransformer::format_t coordsys,
               bool normalCoordinateOrder)
{
   ofstream file( (fileName+".mif").c_str(), ios::app);
   gfx->printMif( file );
}

// Method used in createMidMifAreaFiles
GMSGfxData* removeInsidePolys( GMSGfxData* largeGfx )
{
   // Create a new tmp gfx and add only polys that are inside each other.
   // Always adding the first one, so make sure that the other gfxData
   // is sorted.
   mc2dbg1 << "removeInsidePolys action" << endl;
   largeGfx->sortPolygons();
   uint32 nbrPolysInLarge = largeGfx->getNbrPolygons();
   mc2dbg1 << "removeInsidePolys start with " << nbrPolysInLarge
           << " nbr polys" << endl;
   GMSGfxData* tmpGfx = GMSGfxData::createNewGfxData(NULL, true);
   bool firstAdded = false;
   int inside = 0;
   for (uint32 p = 0; p < nbrPolysInLarge; p++) {
      int clockwise = largeGfx->clockWise(p);
      if (firstAdded) {
         inside = tmpGfx->insidePolygon(largeGfx->getLat(p, 0),
                                        largeGfx->getLon(p, 0));
         // if inside, don't add
         if ( (inside == 0) || (inside == 1) ) {
         
            bool backwards = false;
            if (clockwise == 0) {
               backwards = true;
            }
            if (tmpGfx->addPolygon(largeGfx, backwards, p)) {
               cout << "  added poly " << p 
                    << ":" << largeGfx->getNbrCoordinates(p)
                    << "  (" << largeGfx->getLat(p,0) << "," 
                    << largeGfx->getLon(p,0) << ")" << endl;
            }
         } else {
            cout << "  not added poly " << p
                 << ":" << largeGfx->getNbrCoordinates(p) 
                 << "  (" << largeGfx->getLat(p,0) << "," 
                 << largeGfx->getLon(p,0) << ")" << endl;
         }
         cout << "    cw=" << clockwise << " ins=" << inside << endl;
         if (((clockwise == 1) && (inside == 2)) ||
             ((clockwise == 0) && (inside == 0)) ||
             ((clockwise == 0) && (inside == 1)))
            cout << "NOTE: cw=1 ins=2 (" << largeGfx->getLat(p, 0)
                 << "," << largeGfx->getLon(p, 0) << ")" << endl;
      } else {
         if (tmpGfx->add(largeGfx, false, p))
            cout << "  added first poly " << p << endl;
         firstAdded = true;
         tmpGfx->setClosed(0, true);
      }
   }
   largeGfx = tmpGfx;
   nbrPolysInLarge = largeGfx->getNbrPolygons();
   mc2dbg1 << "removeInsidePolys ends with " << nbrPolysInLarge
           << " nbr polys" << endl;
   return largeGfx;
} // removeInsidePolys

bool
createMidMifAreaFiles( CommandlineOptionHandler coh )
{
   // Check the files
   bool filesOK = true;
   const char* midName = coh.getTail(0);
   if ( strstr(midName, ".mid") == NULL ) {
      mc2dbg1 << "File is no mid file " << midName << endl;
      filesOK = false;
   }
   const char* mifName = coh.getTail(1);
   if ( strstr(mifName, ".mif") == NULL ) {
      mc2dbg1 << "File is no mif file " << mifName << endl;
      filesOK = false;
   }
   const char* txtName = coh.getTail(2);
   if ( strstr(txtName, ".txt") == NULL ) {
      mc2dbg1 << "File is no txt file " << txtName << endl;
      filesOK = false;
   }
   if ( ! filesOK ) {
      mc2dbg1 << "File extensions not ok - exit" << endl;
      return false;
   }
   
   // Try to open the files
   ifstream miffile( mifName );
   ifstream midfile( midName );
   ifstream txtfile( txtName );
   if ( !miffile ) {
      mc2dbg1 << "Problem with mif file " << mifName << endl;
      MC2_ASSERT(false);
   }
   if ( !midfile ) {
      mc2dbg1 << "Problem with mid file " << midName << endl;
      MC2_ASSERT(false);
   }
   if ( !txtfile ) {
      mc2dbg1 << "Problem with txt file " << txtName << endl;
      MC2_ASSERT(false);
   }


   bool retVal = false;

   // Read and store info from the map division txt file
   // The municipal id is normally an integer, but to make it more general
   // use string.
   // example of text row: in_delhi_municipalItems 0701
   map<MC2String, MC2String> mapDivision; // munId - mapName
   map<MC2String, MC2String>::const_iterator mapDivIter;
   map<MC2String, uint32> nbrMunPerMap; // mapName - nbrMunicipals
   map<MC2String, uint32>::const_iterator nbrMunCIter;
   char* mapName = new char[128];
   mapName[0] = '\0';
   char* munId = new char[128];
   munId[0] = '\0';
   while ( !txtfile.eof() ) {
      txtfile >> mapName;
      if ( strlen(mapName) > 0 ) {
         txtfile >> munId;
         if ( strlen(munId) > 0 ) {
            mc2dbg8 << "mapName " << mapName << " munId "
                    << munId << endl;
            mapDivision.insert( make_pair(munId, mapName) );
            map<MC2String, uint32>::iterator it =
               nbrMunPerMap.find( mapName );
            if ( it == nbrMunPerMap.end() ) {
               nbrMunPerMap.insert( make_pair(mapName, 1) );
            } else {
               it->second++;
            }
         }
      }
   }
   mc2dbg1 << "Stored " << mapDivision.size() << " items in mapDivision"
           << " in " << nbrMunPerMap.size() << " maps" << endl;
   for ( nbrMunCIter = nbrMunPerMap.begin();
         nbrMunCIter != nbrMunPerMap.end(); nbrMunCIter++ ) {
      mc2dbg1 << "  " << nbrMunCIter->second << " municipals in map "
              << nbrMunCIter->first << endl;
   }

   // Read mif header
   CoordinateTransformer::format_t coordsys;
   bool normalCoordinateOrder;
   uint32 utmzone;
   int32 falseNorthing, falseEasting;
   if (!GMSGfxData::readMifHeader(miffile, 
                               coordsys, normalCoordinateOrder,
                               utmzone, falseNorthing, falseEasting)) {
      mc2log << fatal << "Could not read mif header" << endl;
      return false;
   }

   // Write mif header in all municipal mif areafiles
   // Open and empty all mid files (for later ios:app)
   for ( nbrMunCIter = nbrMunPerMap.begin();
         nbrMunCIter != nbrMunPerMap.end(); nbrMunCIter++ ) {
      ofstream file( (nbrMunCIter->first + ".mif").c_str() );
      OldItem::writeGenericMifHeader( 0, file );
      file << "DATA" << endl;
      file.close();
      ofstream tmp( (nbrMunCIter->first + ".mid").c_str() );
      tmp.close();
   }

   // Read mid and mif file until it ends
   // Write the file info to the correct mun midmif files while looping
   // Also store the gfxDatas for each mapName, so we can merge and create
   // the municipal map mif file(s) when done looping
   uint32 maxLineLength = 65530;
   char inbuffer[maxLineLength];
   inbuffer[0] = '\0';
   uint32 nbrItemsInFile = 0;
   set<MC2String> municipalsInMidFile;
   set<MC2String> municipalsOnlyInMidFile;
   multimap<MC2String, GMSGfxData*> mapGfxDatas; // mapName, munGfx
   map<MC2String, GMSGfxData*> munGfxDatas; // midIdStr, munGfx
   map<MC2String, GMSGfxData*>::const_iterator munGfxIter;
   while ( !midfile.eof() && !miffile.eof() ) {
      
      // read the id-column from the mid-file, check that something
      // is read to go on.
      midfile.getline(inbuffer, maxLineLength, ',');
      if (strlen(inbuffer) > 0) {
         nbrItemsInFile++;
         
         MC2String midIdStr = inbuffer;
         mc2dbg8 <<  "Read mid " << midIdStr << endl;
         municipalsInMidFile.insert( midIdStr );
         
         // Read the rest of the mid line and
         // create the gfx data
         midfile.getline(inbuffer, maxLineLength);
         GMSGfxData* curGfx = GMSGfxData::createNewGfxData(NULL, false);
         mc2dbg8 << "Creating gfxData for item "<< nbrItemsInFile
                 << "." << endl;
         bool gfxOK = curGfx->createGfxFromMif(miffile,
                                  coordsys, normalCoordinateOrder,
                                  utmzone, falseNorthing, falseEasting);
 
         if ( ! gfxOK ) {
            mc2log << warn << "Problems with gfx for midIdStr "
                   << midIdStr << endl;
         }
         else {
            // Store
            munGfxDatas.insert( make_pair(midIdStr, curGfx) );
            
            // Find fileName for midIdStr in the mapDivision
            mapDivIter = mapDivision.find(midIdStr);
            if ( mapDivIter != mapDivision.end() ) {
               // Print mid and mif to the correct municipal areafiles
               printMid( mapDivIter->second, midIdStr, true, false); //comma
               printMid( mapDivIter->second, inbuffer, false, true); // endl
               printMif( mapDivIter->second, curGfx,
                         coordsys, normalCoordinateOrder );
               // Store the gfx data based on mapName
               mapGfxDatas.insert( make_pair(mapDivIter->second, curGfx) );
            } else {
               mc2log << error << "municipal " << midIdStr
                      << " not present in map division file" << endl;
               municipalsOnlyInMidFile.insert( midIdStr );
            }
         }
      }
   }
   mc2dbg1 << "Read " << nbrItemsInFile
           << " items from midmif file" << endl;
   if ( municipalsOnlyInMidFile.size() > 0 ) {
      mc2log << error << "New municipals to include in mapDivision file" << endl;
      MC2_ASSERT(false);
   }

   // Merge the gfxdatas in the mapGfxDatas multimap
   mc2dbg1 << "Merge the gfxdatas in the mapGfxDatas multimap, size="
           << mapGfxDatas.size() << endl;
   multimap<MC2String, GMSGfxData*>::const_iterator gfxIter;
   for ( nbrMunCIter = nbrMunPerMap.begin();
         nbrMunCIter != nbrMunPerMap.end(); nbrMunCIter++ ) {
      MC2String mapName = nbrMunCIter->first;
      mc2dbg1 << "Create large gfx for " << nbrMunCIter->first << ", "
              << nbrMunCIter->second << " municipals in the map" << endl;
      
      // Create one large gfx data with all polygons of all gfx datas
      // that belong to this mapName
      GMSGfxData* largeGfx = GMSGfxData::createNewGfxData(NULL, true);
      bool firstAddedToLarge = false;
      uint32 gfxNbr = 0;
      for ( gfxIter = mapGfxDatas.lower_bound(mapName);
            gfxIter != mapGfxDatas.upper_bound(mapName); gfxIter++ ) {
         GMSGfxData* gfx = gfxIter->second;
         // add all polys to the largeGfx
         for (uint32 p = 0; p < gfx->getNbrPolygons(); p++) {
            if (firstAddedToLarge) {
               if(largeGfx->addPolygon(gfx, false, p))
                  mc2dbg2 << " added poly " << gfxNbr << ":" << p 
                          << ":" << gfx->getNbrCoordinates(p) << endl;
            } else {
               if ( largeGfx->add(gfx, false, p) ) {
                  mc2dbg2 << " added first poly " << gfxNbr << ":" << p
                          << ":" << gfx->getNbrCoordinates(p) << endl;
                  firstAddedToLarge = true;
                  largeGfx->setClosed(0, gfx->getClosed(0));
               }
            }
         }
         gfxNbr++;
      }
      largeGfx->sortPolygons();
      uint32 nbrPolysInLarge = largeGfx->getNbrPolygons();
      mc2dbg1 << "Created largeGfx for " << nbrMunCIter->first << ": "
              << nbrPolysInLarge << " polygons, " 
              << largeGfx->getTotalNbrCoordinates() << " coordinates" << endl;

      // Merge the polygons in this large gfx data,
      // and print to map mif file
      uint32 startTime = TimeUtility::getCurrentTime();
      GMSGfxData* mapGfxData = GMSGfxData::mergePolygons(largeGfx);
      bool printToMapMif = false;
      if ( mapGfxData != NULL ) {
         mc2dbg1 << " Merged large gfx for " << mapName << ", time="
                 << (TimeUtility::getCurrentTime() - startTime)
                 << " ms" << endl;
         printToMapMif = true;
      } else {
         // Failed to merge the polygons in the largeGfx.
         // This means that there was only one polygon in the gfxdata,
         // so we will use the original.
         mc2log << error << "Mo merging of large gfx for " << mapName
                << " - use original" << endl;
         mapGfxData = largeGfx;
         printToMapMif = true;
      }
      if ( printToMapMif ) {
         // remove inside polygons 
         GMSGfxData* mapGfxData2 = removeInsidePolys( mapGfxData );
         ofstream file( (mapName + "map.mif").c_str() );
         OldItem::writeGenericMifHeader( 0, file );
         file << "DATA" << endl;
         mapGfxData2->printMif(file);
      }
   }

   // Compare the content of municipal ids in mapDivision map and
   // municipalsInMidFile set to see if all municipals in the mapDivision
   // was present in the municipal mid file.
   set<MC2String> municipalsOnlyInDivisionFile;
   for ( mapDivIter = mapDivision.begin(); 
         mapDivIter != mapDivision.end(); mapDivIter++ ) {
      MC2String midIdStr = mapDivIter->first;
      set<MC2String>::const_iterator it = municipalsInMidFile.find(midIdStr);
      if ( it == municipalsInMidFile.end() ) {
         municipalsOnlyInDivisionFile.insert( midIdStr );
      }
   }

   // Print the result of the areafiles creation,
   // and info from the mapDivision - municipalsInMidFile comparisons
   // DO NOT REMOVE THE "RESULT:" tags, they are needed!!
   mc2dbg1 << "============================================" << endl;
   mc2dbg1 << "RESULT:" << endl;
   mc2dbg1 << "RESULT: Created " << nbrMunPerMap.size() << " areafiles"
           << endl;
   for ( nbrMunCIter = nbrMunPerMap.begin();
         nbrMunCIter != nbrMunPerMap.end(); nbrMunCIter++ ) {
      mc2dbg1 << "RESULT: " << nbrMunCIter->first << " with "
              << nbrMunCIter->second << " municipals" << endl;
   }
   mc2dbg1 << "RESULT: Number municipals only in mapDivision file = "
           << municipalsOnlyInDivisionFile.size() << endl;
   for ( set<MC2String>::const_iterator it = 
            municipalsOnlyInDivisionFile.begin();
         it != municipalsOnlyInDivisionFile.end(); it++ ) {
      mc2dbg1 << "RESULT: " << *it << endl;
   }
   mc2dbg1 << "RESULT: Number municipals only in municipal mid file = "
           << municipalsOnlyInMidFile.size() << endl;
   for ( set<MC2String>::const_iterator it = 
            municipalsOnlyInMidFile.begin();
         it != municipalsOnlyInMidFile.end(); it++ ) {
      int32 lat = 0; int32 lon = 0;
      munGfxIter = munGfxDatas.find( *it );
      if ( munGfxIter != munGfxDatas.end() ) {
         //lat = munGfxIter->second->getLat(0,0);
         //lon = munGfxIter->second->getLon(0,0);
         munGfxIter->second->getPolygonCentroid(0, lat, lon);
      }
      mc2dbg1 << "RESULT: " << *it 
              << " coord0 = (" << lat << " " << lon << ")" << endl;
      
   }
   retVal = ( (municipalsOnlyInDivisionFile.size() == 0) &&
              (municipalsOnlyInMidFile.size() == 0) );
   
   return retVal;
} // createMidMifAreaFiles


// Method used in modifyMifFiles
void
modifyPrintPolyInfo(GMSGfxData* largeGfx, uint32 maxNbrPolys = 15)
{
   uint32 nbrPolysInLarge = largeGfx->getNbrPolygons();
   for (uint32 p = 0; (p < nbrPolysInLarge) && (p < maxNbrPolys ); p++) {
      cout << " -- largePoly " << p << ":" << largeGfx->getNbrCoordinates(p)
           << " clockwise=" << largeGfx->clockWise(p)
           << " length=" << largeGfx->getLength(p)
           << " area=" << largeGfx->polygonArea(p)
           << " (" << largeGfx->getLat(p, 0) 
           << " " << largeGfx->getLon(p, 0) << ")"
           << endl;
   }
}

bool
modifyMifFiles( CommandlineOptionHandler coh, const char* outFileName,
                bool sortPolygons,
                bool removeClockwise,
                bool removeCounterClockwise,
                bool removeSmallPolys,
                bool mergeAdjacentPolys,
                bool splitToSepGfxs,
                bool printPolyInfo,
                bool removeInsidePolys)
{


   uint32 nbrPolysToPrint = 20;

   uint32 nbrFiles = coh.getTailLength();
   mc2dbg1 << "modifyMifFiles, " << nbrFiles << " files in tail, print to '"
           << outFileName << "'" << endl;

   // Loop all files in tail and create one large gfx holding all info
   GMSGfxData* largeGfx = GMSGfxData::createNewGfxData(NULL, true);
   bool firstAddedToLarge = false;
   
   for ( uint32 i = 0; i < nbrFiles; i++ ) {
      const char* mifName = coh.getTail(i);
      mc2dbg1 << "Processing file " << i << ": " << mifName << endl;
      if ( strstr(mifName, ".mif") == NULL ) {
         mc2dbg1 << "File is no mif file" << endl;
         continue;
      }

      ifstream miffile( mifName );
      if ( !miffile ) {
         mc2dbg1 << "Couldn't find mif file." << endl;
         continue;
      }

      // Read mif header
      CoordinateTransformer::format_t coordsys;
      bool normalCoordinateOrder;
      uint32 utmzone;
      int32 falseNorthing, falseEasting;
      if (!GMSGfxData::readMifHeader(miffile, 
                                  coordsys, normalCoordinateOrder,
                                  utmzone, falseNorthing, falseEasting)) {
         mc2log << fatal << "Could not read mif header" << endl;
         return false;
      }
      
      // Read this mif file
      uint32 mifNbr = 0;
      while ( !miffile.eof() ) {
      
         GMSGfxData* gfx = GMSGfxData::createNewGfxData(NULL, false);
         if (gfx->createGfxFromMif(miffile,
                                   coordsys,normalCoordinateOrder,
                                   utmzone, falseNorthing, falseEasting)) {
            mifNbr++;
            uint32 nbrPoly = gfx->getNbrPolygons();
            
            // add all polys to the largeGfx
            for (uint32 p = 0; p < nbrPoly; p++) {
               if (firstAddedToLarge) {
                  if(largeGfx->addPolygon(gfx, false, p))
                     cout << " added poly " << mifNbr << ":" << p 
                          << ":" << gfx->getNbrCoordinates(p) << endl;
               } else {
                  if ( largeGfx->add(gfx, false, p) ) {
                     cout << " added first poly " << mifNbr << ":" << p
                          << ":" << gfx->getNbrCoordinates(p) << endl;
                     firstAddedToLarge = true;
                     largeGfx->setClosed(0, gfx->getClosed(0));
                  }
               }
            }

         } 
         else {
            mc2dbg << "Could not create gfx nbr " << mifNbr+1 
                   << " - eof() = ";
            if (miffile.eof())
               mc2dbg << "YES" << endl;
            else
               mc2dbg << "NO" << endl;
         }

      } // mif file
      
      mc2dbg1 << "Read " << mifNbr << " mif features from file " << i << endl;
      
   } // all files in tail

   uint32 nbrPolysInLarge = largeGfx->getNbrPolygons();
   mc2dbg1 << "Created largeGfx: " << nbrPolysInLarge << " polygons, " 
           << largeGfx->getTotalNbrCoordinates() << " coordinates" << endl;

   // print info before modifying anything
   if ( printPolyInfo ) {
      modifyPrintPolyInfo( largeGfx );
   }
   
   // Do some processing: sort, remove small islands, remove holes, 
   // merge polygons with shared border ("item polygons"),
   // etc...

   if ( sortPolygons ) {
      mc2dbg1 << "Sort polys" << endl;
      largeGfx->sortPolygons();
      if ( printPolyInfo ) {
         modifyPrintPolyInfo( largeGfx );
      }
   }

   // ---------------------------------------------------------
   // -------- Remove some polys not wanted -------------------
   //
   
   // Remove polygons oriented clockwise (= keep holes).
   if ( removeClockwise ) {
      // Create a new tmp gfx and add all holes backwards (create islands).
      GMSGfxData* tmpGfx = GMSGfxData::createNewGfxData(NULL, true);
      bool firstAdded = false;
      for (uint32 p = 0; p < nbrPolysInLarge; p++) {
         int clockwise = largeGfx->clockWise(p);
         if (firstAdded) {

            if (!clockwise) {
               if(tmpGfx->addPolygon(largeGfx, true, p)) //backwards
                  cout << " removeC added poly " << p << ":" 
                       << largeGfx->getNbrCoordinates(p)
                       << "  (" << largeGfx->getLat(p,0) << "," 
                       << largeGfx->getLon(p,0) << ")" << endl;
            } else {
               cout << " removeC not added poly " << p << ":" 
                    << largeGfx->getNbrCoordinates(p) 
                    << "  (" << largeGfx->getLat(p,0) << "," 
                    << largeGfx->getLon(p,0) << ")" << endl;
            }

         } else {
            if (!clockwise) {
               if ( tmpGfx->add(largeGfx, true, p) ) {
                  cout << " removeC added first poly " << ":" << p << endl;
                  firstAdded = true;
                  tmpGfx->setClosed( 0, largeGfx->getClosed(0) );
               }
            }
         }
      }
      largeGfx = tmpGfx;
      nbrPolysInLarge = largeGfx->getNbrPolygons();
      mc2dbg1 << "Removed clockwise -> " << nbrPolysInLarge << " polys" << endl;
   }
         
   // Remove polygons oriented counter-clockwise (= remove holes).
   if ( removeCounterClockwise ) {
      // Create a new tmp gfx and add all polys.
      GMSGfxData* tmpGfx = GMSGfxData::createNewGfxData(NULL, true);
      bool firstAdded = false;
      for (uint32 p = 0; p < nbrPolysInLarge; p++) {
         int clockwise = largeGfx->clockWise(p);
         if (firstAdded) {

            if ( clockwise ) {
               if(tmpGfx->addPolygon(largeGfx, false, p))
                  cout << " removeCC added poly " << p << ":"
                       << largeGfx->getNbrCoordinates(p)
                       << "  (" << largeGfx->getLat(p,0) << "," 
                       << largeGfx->getLon(p,0) << ")" << endl;
            } else {
               cout << " removeCC not added poly " << p << ":" 
                    << largeGfx->getNbrCoordinates(p) 
                    << "  (" << largeGfx->getLat(p,0) << "," 
                    << largeGfx->getLon(p,0) << ")" << endl;
            }

         } else {
            if ( clockwise ) {
               if ( tmpGfx->add(largeGfx, false, p) ) {
                  cout << " removeCC added first poly " << p << endl;
                  firstAdded = true;
                  tmpGfx->setClosed( 0, largeGfx->getClosed(0) );
               }
            }
         }
      }
      largeGfx = tmpGfx;
      nbrPolysInLarge = largeGfx->getNbrPolygons();
      mc2dbg1 << "Removed counter-clockwise -> " << nbrPolysInLarge 
              << " polys" << endl;
   }
   
   if ( removeSmallPolys ) {
      // Create a new tmp gfx and add only large polys.
      GMSGfxData* tmpGfx = GMSGfxData::createNewGfxData(NULL, true);
      bool firstAdded = false;
      uint32 minLength = 100000; // 100 km 
      for (uint32 p = 0; p < nbrPolysInLarge; p++) {
         float64 length = largeGfx->getLength(p);
         if ( (length < minLength) || (largeGfx->getNbrCoordinates(p) < 3) ) {
            DEBUG8(
            cout << " removeSmall not added small poly " << p << ":" 
                 << largeGfx->getNbrCoordinates(p) << " length=" << length
                 << "  (" << largeGfx->getLat(p,0) << "," 
                 << largeGfx->getLon(p,0) << ")" << endl;);
         } else {
            if (firstAdded) {
               if ( tmpGfx->addPolygon(largeGfx, false, p) ) {
                  DEBUG8(
                  cout << " removeSmall added poly " << p 
                       << ":" << largeGfx->getNbrCoordinates(p)
                       << "  (" << largeGfx->getLat(p,0) << "," 
                       << largeGfx->getLon(p,0) << ")" << endl;);
               }
            } else {
               if ( tmpGfx->add(largeGfx, false, p) ) {
                  cout << " removeSmall added first poly " << p << endl;
                  firstAdded = true;
                  tmpGfx->setClosed(0, true);
               }
            }
         }
      }
      largeGfx = tmpGfx;
      nbrPolysInLarge = largeGfx->getNbrPolygons();
      mc2dbg1 << "Removed small -> " << nbrPolysInLarge << " polys" << endl;
      if ( printPolyInfo ) {
         modifyPrintPolyInfo( largeGfx, nbrPolysToPrint );
      }
   }

   if ( removeInsidePolys ) {
      // Create a new tmp gfx and add only polys that are inside each other.
      // Always adding the first one, so make sure that the other gfxData
      // is sorted.
      mc2dbg1 << " modifyMifFiles:removeInsidePolys sort polys" << endl;
      largeGfx->sortPolygons();
      if ( printPolyInfo ) {
         modifyPrintPolyInfo( largeGfx );
      }
      mc2dbg1 << " modifyMifFiles:removeInsidePolys action" << endl;
      GMSGfxData* tmpGfx = GMSGfxData::createNewGfxData(NULL, true);
      bool firstAdded = false;
      int inside = 0;
      for (uint32 p = 0; p < nbrPolysInLarge; p++) {
         int clockwise = largeGfx->clockWise(p);
         if (firstAdded) {
            inside = tmpGfx->insidePolygon(largeGfx->getLat(p, 0),
                                           largeGfx->getLon(p, 0));
            // if inside, don't add
            if ( (inside == 0) || (inside == 1) ) {
            
               bool backwards = false;
               if (clockwise == 0)
                  backwards = true;
               if(tmpGfx->addPolygon(largeGfx, backwards, p))
                  cout << "  added poly " << p 
                       << ":" << largeGfx->getNbrCoordinates(p)
                       << "  (" << largeGfx->getLat(p,0) << "," 
                       << largeGfx->getLon(p,0) << ")" << endl;
            } else {
               cout << "  not added poly " << p
                    << ":" << largeGfx->getNbrCoordinates(p) 
                    << "  (" << largeGfx->getLat(p,0) << "," 
                    << largeGfx->getLon(p,0) << ")" << endl;
            }
            cout << "    cw=" << clockwise << " ins=" << inside << endl;
            if (((clockwise == 1) && (inside == 2)) ||
                ((clockwise == 0) && (inside == 0)) ||
                ((clockwise == 0) && (inside == 1)))
               cout << "NOTE: cw=1 ins=2 (" << largeGfx->getLat(p, 0)
                    << "," << largeGfx->getLon(p, 0) << ")" << endl;
         } else {
            if (tmpGfx->add(largeGfx, false, p))
               cout << "  added first poly " << p << endl;
            firstAdded = true;
            tmpGfx->setClosed(0, true);
         }
      }
      largeGfx = tmpGfx;
      nbrPolysInLarge = largeGfx->getNbrPolygons();
      mc2dbg1 << "Removed inside -> " << nbrPolysInLarge << " polys" << endl;
      if ( printPolyInfo ) {
         modifyPrintPolyInfo( largeGfx, nbrPolysToPrint );
      }
   }
   
   // ---------------------------------------------------------
   // -------- Merge remaining polys --------------------------
   //
   if ( mergeAdjacentPolys ) {
      mc2dbg1 << " modifyMifFiles:mergeAdjacentPolys action" << endl;
      uint32 startTime = TimeUtility::getCurrentTime();
      GMSGfxData* mergedGfx = GMSGfxData::mergePolygons(largeGfx);
      if ( mergedGfx != NULL ) {
         mc2dbg1 << " Merged large gfx, time="
                 << (TimeUtility::getCurrentTime() - startTime)
                 << " ms" << endl;
         largeGfx = mergedGfx;
      } else {
         // Failed to merge the polygons in the largeGfx.
         // This could mean that there was only one polygon in the gfxdata,
         // so we will use the original.
         mc2dbg1 << "Mo merging of large gfx - use original" << endl;
      }
      nbrPolysInLarge = largeGfx->getNbrPolygons();
      mc2dbg1 << "Merged adjacent polys -> " 
              << nbrPolysInLarge << " polys" << endl;
      if ( printPolyInfo ) {
         modifyPrintPolyInfo( largeGfx, nbrPolysToPrint );
      }
   }

   
   // ---------------------------------------------------------
   // --------------------- Result ----------------------------
   //
   
   // Print to outfile
   ofstream outfile( outFileName );
   OldItem::writeGenericMifHeader( 0, outfile );
   outfile << "DATA" << endl;

   if ( splitToSepGfxs ) {
      // print to outfile with each polygon as separate gfxs
      for (uint32 p = 0; p < nbrPolysInLarge; p++) {
         GMSGfxData* newGfx = GMSGfxData::createNewGfxData(NULL, true);
         if (newGfx->add(largeGfx, false, p)) {
            newGfx->setClosed(0, largeGfx->getClosed(p));
            newGfx->updateLength();
            newGfx->printMif(outfile);
         }
      }
      mc2dbg << "Printed gfx as " << nbrPolysInLarge 
             << " sep gfxs to '" << outFileName << "'" << endl;
   }
   else {
      // print to outfile one Region
      largeGfx->printMif(outfile);
      mc2dbg << "Printed gfx to '" << outFileName << "'" << endl;
   }

   mc2dbg1 << "Include appropriate mif header!" << endl;
   return true;
}

void
findBreakPoints( CommandlineOptionHandler coh )
{
   mc2log << info << "findBreakPoints start" << endl;
   // 1. Create gfxs from mif files in tail
   // 2. Collect all coordinates into a map<MC2Coordinate, set< gfxid > >
   // 3. Identify break point candidates in the gfxs (when something happens)
   //    and collect into a map<MC2Coordinate, set< const char*> >  (gfxnames)
   // 4. Identify those break point candidates that are shared between
   //    more than one mifGfx -> true break point

   // 1.
   typedef pair<GMSGfxData*, const char*> gfxDatas_t;
   set< gfxDatas_t > gfxDatas;
   int nbrFiles = coh.getTailLength();
   for ( int i = 0; i < nbrFiles; i++ ) {
      const char* fileName = coh.getTail(i);
      ifstream file( fileName );
      GMSGfxData* gfx = GMSGfxData::createNewGfxData( NULL, false );
      if ( gfx->createFromMif( file ) ) {
         // make a short version of the file name (remove all directory names)
         char* shortFileName = rindex( fileName, '/');
         if ( shortFileName == NULL ) {
            shortFileName = StringUtility::newStrDup(fileName);
         } else {
            shortFileName = shortFileName+1;
         }
         gfxDatas.insert( make_pair( gfx, shortFileName ) );
         mc2log << info << "Created " << shortFileName << " with "
                <<  gfx->getTotalNbrCoordinates() << " coords"
                << endl;
      }
   }
   mc2log << info << "Created " << gfxDatas.size() << " mif gfx datas" << endl;

   // 2.
   typedef map< MC2Coordinate, set<uint32> > coord_t;
   coord_t gfxCoordinates;
   uint32 gfxNbr = 0;
   for ( set<gfxDatas_t>::const_iterator git = gfxDatas.begin();
         git != gfxDatas.end(); git++ ) {
      GMSGfxData* gfx = git->first;
      for ( uint32 p = 0; p < gfx->getNbrPolygons(); ++p ) {
         GfxData::const_iterator end = gfx->polyEnd( p );
         for ( GfxData::const_iterator coordIt = gfx->polyBegin( p );
               coordIt != end; coordIt++ ) {
            
            MC2Coordinate coord( coordIt->lat, coordIt->lon );
            coord_t::iterator it = gfxCoordinates.find( coord );

            if ( it == gfxCoordinates.end() ) {
               set<uint32> gfxNbrSet;
               gfxNbrSet.insert( gfxNbr );
               gfxCoordinates.insert( make_pair( coord, gfxNbrSet ) );
            }
            else {
               set<uint32>& gfxNbrSet = (*it).second;
               gfxNbrSet.insert( gfxNbr );
            }
            
         }
      }
      gfxNbr++;
   }
   mc2log << info << "Nbr coordinates in gfxCoordinates = "
          << gfxCoordinates.size() << endl;

   // 3.
   mc2log << info << "Look for break points" << endl;
   typedef map< MC2Coordinate, set< const char*> > candidates_t;
   candidates_t breakPointCands;
   for ( set<gfxDatas_t>::const_iterator git = gfxDatas.begin();
         git != gfxDatas.end(); git++ ) {
      GMSGfxData* gfx = git->first;
      for ( uint32 p = 0; p < gfx->getNbrPolygons(); ++p ) {
         
         GfxData::const_iterator polyBegin = gfx->polyBegin( p );
         MC2Coordinate firstCoord( polyBegin->lat, polyBegin->lon );
         coord_t::iterator beginIt = gfxCoordinates.find( firstCoord );
         
         GfxData::const_iterator polyEnd = gfx->polyEnd( p );
         uint32 coordIdx = 0;
         uint32 prevCoordIdx = 0;

         for ( GfxData::const_iterator coordIt = gfx->polyBegin( p );
               coordIt != polyEnd; coordIt++ ) {
            
            MC2Coordinate coord( coordIt->lat, coordIt->lon );
            coord_t::iterator nextIt = gfxCoordinates.find( coord );
            if (  beginIt->second != nextIt->second ) {
               
               cout << "Candidate in " << git->second << " poly=" << p
                    << " from " << prevCoordIdx << " " << beginIt->first
                    << " to " << coordIdx << " " << coord << endl;

               // which is the break point?
               bool storePrev = false;
               bool storeThis = false;
               if ( beginIt->second.size() > nextIt->second.size() ) {
                  storePrev = true;
               } else if ( beginIt->second.size() == nextIt->second.size() ) {
                  storePrev = true;
                  storeThis = true;
               } else {
                  storeThis = true;
               }

               // Store prevCoord
               if ( storePrev ) {
                  candidates_t::iterator it = 
                        breakPointCands.find( beginIt->first );
                  if ( it == breakPointCands.end() ) {
                     set<const char*> gfxNameSet;
                     gfxNameSet.insert( git->second );
                     breakPointCands.insert(
                        make_pair(beginIt->first, gfxNameSet) );
                  } else {
                     set<const char*>& gfxNameSet = it->second;
                     gfxNameSet.insert( git->second );
                  }
               }
               // Store coord
               if ( storeThis ) {
                  candidates_t::iterator it = breakPointCands.find( coord );
                  if ( it == breakPointCands.end() ) {
                     set<const char*> gfxNameSet;
                     gfxNameSet.insert( git->second );
                     breakPointCands.insert( make_pair( coord, gfxNameSet) );
                  } else {
                     set<const char*>& gfxNameSet = it->second;
                     gfxNameSet.insert( git->second );
                  }
               }
            
            }
            
            beginIt = nextIt;
            prevCoordIdx = coordIdx;
            coordIdx++;
         }
      }
   }
   mc2log << info << "Collected " << breakPointCands.size()
          << " break point candidates among the " << gfxDatas.size()
          << " mif gfx datas" << endl;

   // 4.
   uint32 nbrBreakPoints = 0;
   for ( candidates_t::iterator it = breakPointCands.begin();
         it != breakPointCands.end(); it++ ) {
      MC2Coordinate coord = it->first;
      set<const char*> gfxNameSet = it->second;
      if ( gfxNameSet.size() > 1 ) {
         nbrBreakPoints++;
         cout << " break point " << coord.lat << " " << coord.lon << " ";
         for ( set<const char*>::iterator sit = gfxNameSet.begin();
               sit != gfxNameSet.end(); sit++ ) {
            cout << " " << *sit;
         }
         cout << endl;
      }
   }
   mc2log << info << "Sorted out " << nbrBreakPoints
          << " \"true\" break points" << endl;
   
   mc2log << info << "findBreakPoints Done!" << endl;
} // findBreakPoints

void
loadMifFile ( const char* fileName )
{
   mc2log << info << "Load mif file '" << fileName << "'" << endl;
   ifstream miffile(fileName);
   if (!miffile) {
      cout << "Couldn't find mif file." << endl;
      MC2_ASSERT(false);
   }
   
   // read header
   CoordinateTransformer::format_t coordsys;
   bool normalCoordinateOrder;
   uint32 utmzone;
   int32 falseNorthing, falseEasting;
   mc2dbg1 << "Reading mif-header of the file." << endl;
   if (!GMSGfxData::readMifHeader(miffile, 
                               coordsys, normalCoordinateOrder,
                               utmzone, falseNorthing, falseEasting)) {
      mc2log << fatal << "Could not read mif header" << endl;
      MC2_ASSERT(false);
   }
   
   cout << "Reading mif features" << endl;
   uint32 mifNbr = 0;
   while (!miffile.eof()) {
   
      GMSGfxData* gfx = GMSGfxData::createNewGfxData(NULL, false);
      if (gfx->createGfxFromMif(miffile,
                                coordsys,normalCoordinateOrder,
                                utmzone, falseNorthing, falseEasting)) {
         mifNbr++;
         cout << "Mif nbr " << mifNbr << " ok (last coord " 
              << gfx->getLastLat(0) << " " << gfx->getLastLon(0) 
              << ")" << endl;

      } else {
         mc2dbg << "Could not create gfx nbr " << mifNbr+1 
                << " - eof() = ";
         if (miffile.eof())
            mc2dbg << "YES" << endl;
         else
            mc2dbg << "NO" << endl;
      }
   }
   mc2log << info << "Read " << mifNbr 
          << " mif features from mif file" << endl;
}

void
removeInsideMifHeaders( const char* fileName, const char* outFileName )
{
   mc2log << info << "Remove inside mif headers from file '"
          << fileName << "'" << endl;
   ifstream miffile(fileName);
   if ( ! miffile ) {
      mc2log << error << "No such file" << endl;
      exit(1);
   }

   ofstream outfile( outFileName );

   const int maxLineLength = 200;
   char lineBuffer[maxLineLength];
   lineBuffer[0] = '\0';
   
   // read the first line from the mif file
   miffile.getline( lineBuffer, maxLineLength );
   
   uint32 nbrInsideHeaders = 0;
   bool firstHeaderRead = false;
   while ( !miffile.eof() ) {

      if ( strstr(lineBuffer, "VERSION") != NULL ) {
         if ( ! firstHeaderRead ) {
            firstHeaderRead = true;
         } else {
            // We have an inside header, read until this header is passed
            nbrInsideHeaders++;
            bool headerRead = false;
            while ( !headerRead ) {
               headerRead = ( (strlen(lineBuffer) == 4) && 
                              ( (strstr(lineBuffer, "DATA") != NULL) ||
                                (strstr(lineBuffer, "Data") != NULL) ) );
               miffile.getline( lineBuffer, maxLineLength );
            }
         }
      }
      
      outfile << lineBuffer << endl;

      // read next line from the mif file
      miffile.getline( lineBuffer, maxLineLength );
   }

   mc2log << info << "Removed " << nbrInsideHeaders << " inside mif headers "
          << "wrote result in file called 'noInsideHeader.mif'" << endl;
}

#include "MapFilterUtil.h"
void
convertCountryBordersToMidmif( const char* fileName )
{
   // Code copied from OldMapFilter::oldBorderFiltering

   // Border file structure
   //
   //    BORDERPART germany_1
   //    (startBreakPointLat,startBreakPointLon)
   //    (endBreakPointLat,endBreakPointLon)
   //    (pointOnTheWayLat,pointOnTheWayLon) // 50% offset from start->end
   //    nbrCoords
   //    mc2lat mc2lon
   //    mc2lat mc2lon
   //    ...

   // Open border midmif file for writing, write mif header
   ofstream mif( "borders.mif" );
   ofstream mid( "borders.mid" );
   mif << "VERSION 300" << endl
       << "Charset \"WindowsLatin1\"" << endl
	    << "DELIMITER \",\"" << endl
       << "COORDSYS mc2_latlon" << endl
       << "COLUMNS 2" << endl
       << "  Id Decimal (8,0)" << endl
       << "  name char(200)" << endl
       << "Data" << endl;

   // Open border file for reading
   ifstream borderFile( fileName );
   const int maxLineLength = 200;
   char lineBuffer[maxLineLength];
   lineBuffer[0] = '\0';
   
   // read the first line from the file with borders
   borderFile.getline( lineBuffer, maxLineLength );
   
   uint32 nbrBordersInFile = 0;
   while ( !borderFile.eof() && (strlen(lineBuffer) > 0) ) {

      if ( strstr(lineBuffer, "BORDERPART") != NULL ) {
         nbrBordersInFile++;
         
         // Read co map name for which this borderpart was written
         const char* borderCountryMapName = 
               StringUtility::newStrDup(lineBuffer + strlen( "BORDERPART "));
         
         // The three coordinates
         MC2Coordinate firstCoord, lastCoord, middleCoord;
         borderFile.getline( lineBuffer, maxLineLength );
         MapFilterUtil::coordFromString( lineBuffer, firstCoord );
         borderFile.getline( lineBuffer, maxLineLength );
         MapFilterUtil::coordFromString( lineBuffer, lastCoord );
         borderFile.getline( lineBuffer, maxLineLength );
         MapFilterUtil::coordFromString( lineBuffer, middleCoord );

         // read nbr coordinates and add to a filteredPart
         borderFile.getline( lineBuffer, maxLineLength );
         uint32 nbrCoordsInPart = strtoul(lineBuffer, NULL, 10);
         GMSGfxData* filteredPart = 
            GMSGfxData::createNewGfxData( NULL, true );
         for ( uint32 c = 0; c < nbrCoordsInPart; c++ ) {
            int32 lat, lon;
            borderFile >> lineBuffer;
            lat = strtol(lineBuffer, NULL, 10);
            borderFile >> lineBuffer;
            lon = strtol(lineBuffer, NULL, 10);
            filteredPart->addCoordinate( lat, lon );
         }
         filteredPart->updateLength();
         float64 length = filteredPart->getLength(0);
         mc2dbg << "Border part for " << borderCountryMapName 
                << " " << nbrCoordsInPart << " coords, " 
                << length << " meters" << endl;

         // Write this border part to midmif
         filteredPart->printMif( mif );
         mid << nbrBordersInFile << "," << borderCountryMapName << endl;

      }
      
      // read next line from the file with borders
      // end-of-line of last coordinate
      borderFile.getline( lineBuffer, maxLineLength );
         //cout << "Read next '" << lineBuffer << "'" << endl;
      // next line = new BORDERPART or eof
      borderFile.getline( lineBuffer, maxLineLength );
         //cout << "Read next '" << lineBuffer << "'" << endl;
   }
   cout << " borderfile eof " << borderFile.eof() << endl;

   mc2log << info << "Read " << nbrBordersInFile 
          << " borders from file" << endl;
   
}

int
main( int argc, char* argv[] )
{
   CommandlineOptionHandler coh( argc, argv, 0 );
   
   // ---------------------------- Main function: createMidMifAreaFiles
   bool o_createMidMifAreaFiles = false;
   coh.addOption("", "--createMidMifAreaFiles",
                 CommandlineOptionHandler::presentVal,
                 0, &o_createMidMifAreaFiles, "F",
                 "Create municipal mid mif and map mif files (areafiles) "
                 "for generating mcm maps from midmif. "
                 "Start with a Wayfinder municipal midmif file containing "
                 "all municipal items. It will be split in different area "
                 "files using a mapDivision.txt file pairing municipal "
                 "file (areafile) base name and municipal mid ids, e.g.\n"
                 "se_skane_municipalItems 701\n"
                 "se_skane_municipalItems 702\n"
                 "se_stockholm_municipalItems 618\n"
                 "se_stockholm_municipalItems 619\n"
                 "Give in tail: municipal mid+mif + mapDivision.txt");

   // ----------------------------------------------- Main function: load
   bool o_load = false;
   coh.addOption("", "--load",
                 CommandlineOptionHandler::presentVal,
                 0, &o_load, "F",
                 "Load one mif file, to check that mif features are valid");

   // ----------------------------------------------- Main function
   bool o_breakPoints = false;
   coh.addOption("-b", "--breakPoints",
                 CommandlineOptionHandler::presentVal,
                 0, &o_breakPoints, "F",
                 "Find break points for shared polygon parts (common lines) "
                 "given the mif files (country polygons) in tail.\n"
                 "bug: If a polygon has a breakpoint as coord 0+n this "
                 "breakpoint can not be detected by MifTool.");

   // ----------------------------------------------- Main function
   bool o_removeInsideMifHeaders = false;
   coh.addOption("", "--removeInsideMifHeaders",
                 CommandlineOptionHandler::presentVal,
                 0, &o_removeInsideMifHeaders, "F",
                 "Read the mif file given in tail, and remove any inside "
                 "mif headers. Outfile name from --outFileName option.");

   // ----------------------------------------------- Main function
   bool o_countryBorders = false;
   coh.addOption("", "--countryBorders",
                 CommandlineOptionHandler::presentVal,
                 0, &o_countryBorders, "F",
                 "Read the countryBorders.txt file given in tail, and "
                 "convert it to midmif, to visually inspect the result.\n"
                 "NB! Shows all border parts (not only border items).");

   // ----------------------------------------------- Main function: modify
   bool o_modify = false;
   coh.addOption("", "--modify",
                 CommandlineOptionHandler::presentVal,
                 0, &o_modify, "F",
                 "Modify mif files (e.g. country polygons). Read the mif "
                 "files in tail and merge into one large mif feature. "
                 "Combine with other options to decide misc modifiers:\n"
                 "\t--modifySort\n"
                 "\t--modifyRemoveC\n"
                 "\t--modifyRemoveCC\n"
                 "\t--modifyRemoveSmallPolys\n"
                 "\t--modifyRemoveInsidePolys\n"
                 "\t--modifyMergePolys\n"
                 "\t--modifySplit\n"
                 "\t--modifyPrintPolyInfo\n"
                 "\t--outFileName");
   
   // -------------------------------------------------- misc specifiers
   char* o_outFileName = NULL;
   coh.addOption("", "--outFileName",
                 CommandlineOptionHandler::stringVal,
                 1, &o_outFileName, "outFile.mif",
                 "Filename of output file, default 'outFile.mif'.");

   bool o_sortPolygons = false;
   coh.addOption("", "--modifySort",
                 CommandlineOptionHandler::presentVal,
                 0, &o_sortPolygons, "F",
                 "Combined with --modify: sort polygons according to size "
                 "      (number of coordinates).");
   
   bool o_removeClockwise = false;
   coh.addOption("", "--modifyRemoveC",
                 CommandlineOptionHandler::presentVal,
                 0, &o_removeClockwise, "F",
                 "Combined with --modify: remove all polygons oriented "
                 "      clockwise. Holes are kept and reversed to "
                 "      positive orientation.");

   bool o_removeCounterClockwise = false;
   coh.addOption("", "--modifyRemoveCC",
                 CommandlineOptionHandler::presentVal,
                 0, &o_removeCounterClockwise, "F",
                 "Combined with --modify: remove all polygons oriented "
                 "      counter-clockwise (remove holes).");
   
   bool o_removeSmallPolys = false;
   coh.addOption("", "--modifyRemoveSmallPolys",
                 CommandlineOptionHandler::presentVal,
                 0, &o_removeSmallPolys, "F",
                 "Combined with --modify: remove all small polygons. The"
                 "criteria of \"small\" is currently hardcoded." );
   
   bool o_removeInsidePolys = false;
   coh.addOption("", "--modifyRemoveInsidePolys",
                 CommandlineOptionHandler::presentVal,
                 0, &o_removeInsidePolys, "F",
                 "Combined with --modify: remove all polygons that are "
                 "inside any of the others. The first one after sorting is "
                 "always kept." );
   
   bool o_mergeAdjacentPolys = false;
   coh.addOption("", "--modifyMergePolys",
                 CommandlineOptionHandler::presentVal,
                 0, &o_mergeAdjacentPolys, "F",
                 "Combined with --modify: adjacent polygons are merged." );
   
   bool o_splitToSepGfxs = false;
   coh.addOption("", "--modifySplit",
                 CommandlineOptionHandler::presentVal,
                 0, &o_splitToSepGfxs, "F",
                 "Combined with --modify: split the large mif feature into "
                 "      separate gfxs in outfile (many Region 1).");
   
   bool o_printPolyInfo = false;
   coh.addOption("", "--modifyPrintPolyInfo",
                 CommandlineOptionHandler::presentVal,
                 0, &o_printPolyInfo, "F",
                 "Combined with --modify: print info for 15 first polys "
                 "      between each modification step.");



   // Parse command-line
   if(!coh.parse()) {
      cerr << argv[0] << ": Error on commandline! (-h for help)" << endl;
      exit(1);
   }

   if ( coh.getTailLength() < 1 ) {
      cout << " no file in tail - exit" << endl;
      exit(0);
   }

   if ( o_createMidMifAreaFiles ) {
      mc2dbg1 << "createMidMifAreaFiles" << endl;
      if ( coh.getTailLength() != 3 ) {
         mc2log << error << "Must give the municipal mid + mif file "
                << "and the mapDivision.txt file in tail - in this order"
                << endl;
      } else {
         createMidMifAreaFiles( coh );
      }
   }

   if ( o_modify ) {
      modifyMifFiles( coh, o_outFileName,
                      o_sortPolygons, o_removeClockwise,
                      o_removeCounterClockwise, o_removeSmallPolys,
                      o_mergeAdjacentPolys, o_splitToSepGfxs,
                      o_printPolyInfo, o_removeInsidePolys );
   }

   if ( o_load ) {
      if ( coh.getTailLength() > 0 ) {
         const char* fileName = coh.getTail(0);
         loadMifFile( fileName );
      }
   }

   if ( o_breakPoints ) {
      mc2log << info << "Find break points between mif files" << endl;
      if ( coh.getTailLength() > 1 ) {
         findBreakPoints( coh );
      } else {
         mc2log << error << "Must give at least 2 mif files in tail to find " 
                << "break points" << endl;
      }
   }

   if ( o_removeInsideMifHeaders ) {
      mc2log << info << "Remove any inside mif headers from mif file in tail"
             << endl;
      if ( coh.getTailLength() > 0 ) {
         const char* fileName = coh.getTail(0);
         removeInsideMifHeaders( fileName, o_outFileName );
      } else {
         mc2log << error << "Must give mif file in tail in order to remove "
                << "inside mif header" << endl;
         exit(1);
      }
   }

   if ( o_countryBorders ) {
      mc2log << info << "Convert country borders file to midmif" << endl;
      if ( coh.getTailLength() > 0 ) {
         const char* fileName = coh.getTail(0);
         convertCountryBordersToMidmif( fileName );
      }
   }

   exit(0);

}

