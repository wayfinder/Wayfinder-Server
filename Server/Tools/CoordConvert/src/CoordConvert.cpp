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
#include "CoordinateTransformer.h"
#include "AsciiFile.h"
#include "StringUtility.h"
#include "Utility.h"
#include <fstream>
#include <iomanip>
#include <string>

bool
convertTextFileCoordinates(ifstream& infile, ofstream& outfile, 
            char fileSeparator, uint32 latPos, uint32 lonPos,
            CoordinateTransformer::format_t inFormat,
            int32 inFalseNorthing, int32 inFalseEasting,
            uint32 inUTMzone,
            CoordinateTransformer::format_t outFormat,
            int32 outFalseNorthing, int32 outFalseEasting,
            uint32 outUTMzone)
{
   mc2dbg8 << "------------------------------------------" << endl;
   mc2dbg8 << "inFormat " << int(inFormat) << endl
           << "inFalseNorthing " << inFalseNorthing << endl
           << "inFalseEasting " << inFalseEasting << endl
           << "outFormat " << int(outFormat) << endl
           << "outFalseNorthing " << outFalseNorthing << endl
           << "outFalseEasting " << outFalseEasting << endl;
   
   // Read infile, based on file separator.
   // When lat and lon pos have been read, convert the coordinates
   // and write to output, lat in latPos and lon in lonPos.
   const int maxLineLength =16380;
   char lineBuffer[maxLineLength];
   lineBuffer[0] = '\0';
   char* dest = "\0";

   float64 inLat, inLon;
   uint32 nbrConverted = 0;
   
   uint32 lineNbr = 0;
   infile.getline(lineBuffer, maxLineLength);
   while (!infile.eof() &&  (strlen(lineBuffer) > 0) ) {

      // read one line
      lineNbr++;
      mc2dbg8 << " Line " << lineNbr << " \"" << lineBuffer << "\"" << endl;
      inLat = MAX_FLOAT64;
      inLon = MAX_FLOAT64;
      bool noCoords = false;
      
      // extract the columns separated with fileSeparator
      uint32 column = 0;
      bool rowDone = false;
      char* colBufferPos = lineBuffer;
      while ( !rowDone && (colBufferPos != NULL)) {
         
         const char* colBuffer = colBufferPos;
         colBufferPos = strchr(colBufferPos, fileSeparator)+1;
         
         if ((strlen(colBuffer) > 0) && (colBufferPos != NULL)) {
            char* tmpStr = strchr(colBuffer, fileSeparator);
            if (tmpStr != NULL) {
               tmpStr[0] = '\0';
            } else {
               // The last column in the file has no ending fileSeparator
               // we can not extract another column after this.
               rowDone = true;
            }
            
            // Save lat and lon, or print the buffer ending with fileSeparator
            if (column == latPos) {
               Utility::getFloat64( (const char*) colBuffer, dest, inLat);
               if (strlen(colBuffer) == 0)
                  noCoords = true;
            } else if (column == lonPos) {
               Utility::getFloat64( (const char*) colBuffer, dest, inLon);
               if (strlen(colBuffer) == 0)
                  noCoords = true;
            } else {
               outfile << colBuffer;
               if (!rowDone)
                  outfile << fileSeparator;
            }

            // If latPos or lonPos and both are set - convert the coords
            // and print to outfile
            if ( ((column == latPos) || (column == lonPos)) &&
                 (inLat != MAX_FLOAT64) && (inLon != MAX_FLOAT64) ) {

               // remove any standard addition
               inLat -= inFalseNorthing;
               inLon -= inFalseEasting;

               // transform
               // To handle utm as both inparam and outparam, go via mc2
               int32 mc2lat, mc2lon;
               float64 outLat, outLon, outh;
               CoordinateTransformer::transformToMC2(
                     inFormat, inLat, inLon, 0, mc2lat, mc2lon, inUTMzone);
               if (outFormat == CoordinateTransformer::mc2) {
                  outLat = mc2lat;
                  outLon = mc2lon;
               } else {
                  CoordinateTransformer::transformFromMC2(mc2lat, mc2lon,
                        outFormat, outLat, outLon, outh, outUTMzone);
               }

               // add any standard addition
               outLat += outFalseNorthing;
               outLon += outFalseEasting;

               // Print to outfile
               if (!noCoords) {
                  nbrConverted++;
                  if (latPos < lonPos)
                     outfile << outLat << fileSeparator 
                             << outLon << fileSeparator;
                  else
                     outfile << outLon << fileSeparator 
                             << outLat << fileSeparator;
               } else {
                  outfile << fileSeparator << fileSeparator;
               }
            }
            
            mc2dbg8 << "  " << lineNbr << ":" << column << " \""
                    << colBuffer << "\"" << endl;
            column++;
         }
         // Nothing to read...
         else {
            rowDone = true;
         }
      }
      outfile << endl;
      
      mc2dbg8 << " Read " << column << " columns for line " << lineNbr << endl;

      // Read next row
      infile.getline(lineBuffer, maxLineLength);
   }
   
   mc2dbg << "Read " << lineNbr << " lines, converted " 
          << nbrConverted << " coordinates" << endl;

   return true;
}

bool
transformCoordinates(ifstream& infile, ofstream& outfile,
         CoordinateTransformer::format_t inFormat,
         bool inNormalCoordinateOrder, 
         int32 inFalseNorthing, int32 inFalseEasting,
         uint32 inUTMzone,
         CoordinateTransformer::format_t outFormat,
         bool outNormalCoordinateOrder,
         int32 outFalseNorthing, int32 outFalseEasting,
         uint32 outUTMzone)
{

   mc2dbg8 << "------------------------------------------" << endl;
   mc2dbg8 << "inFormat " << int(inFormat) << endl
           << "inNormalCoordinateOrder " << inNormalCoordinateOrder << endl
           << "inFalseNorthing " << inFalseNorthing << endl
           << "inFalseEasting " << inFalseEasting << endl
           << "outFormat " << int(outFormat) << endl
           << "outNormalCoordinateOrder " << outNormalCoordinateOrder << endl
           << "outFalseNorthing " << outFalseNorthing << endl
           << "outFalseEasting " << outFalseEasting << endl;
   
   const uint32 MAX_LENGTH = 200;
   
   //read and copy the mif header
   bool done = false;
   char row[MAX_LENGTH+1];
   while (!infile.eof() && !done) {
      infile.getline(row, MAX_LENGTH);
      if (StringUtility::strncasecmp(row, "Data", 4) == 0) {
         done = true;
         mc2dbg4 << "Done reading/copying the mif header." << endl;
      }
      outfile << row << endl;
   }
   
   if (!done) {
      mc2log << error << "No \"Data\" tag in mif header - reached eof "
             << " without transforming the coordinates." << endl;
      return false;
   }
  
   //transform the coordinates

   uint32 nbrRegions = 0;
   uint32 nbrPlines = 0;
   uint32 nbrPoints = 0;
   
   char param[MAX_LENGTH];
   uint32 nbrPolygons = 1;
   bool point = false;
   
   infile >> param;

   while (!infile.eof() ) {

      // Find out if it is a "Region", "Pline" or "Point"
      if (strcasecmp(param, "Region") == 0) {
         nbrRegions ++;
         infile >> nbrPolygons;
         outfile << "Region  " << nbrPolygons << endl;
      } else if (strcasecmp(param, "Pline") == 0) {
         nbrPlines ++;
         nbrPolygons = 1;
         outfile << "Pline";
      } else if (strcasecmp(param, "Point") == 0) {
         nbrPoints ++;
         nbrPolygons = 1;
         point = true;
         outfile << "Point ";
      } else {
         // no region, pline or point to read
         mc2log << error << "No \"Region\", \"Pline\" or \"Point\""
                << " to read" << endl;
         mc2dbg << "Read \"" << param << "\" , nbrpoints=" 
                << nbrPoints << ", nbrplines=" << nbrPlines 
                << ", nbrregions=" << nbrRegions << endl;
         return false;
      }

      // Read all the polygons
      for (uint32 p = 0; p < nbrPolygons; p++) {
         // Read the number of coordinates
         uint32 nbrCoordinates;
         if (point) {
            nbrCoordinates = 1;
         } else {
            infile  >> nbrCoordinates;
            outfile << "  " << nbrCoordinates << endl;
         }
         mc2dbg8 << "   nbrCoordinates = " << nbrCoordinates << endl;

         // Transform the coordinates
         for (uint32 i = 0; i < nbrCoordinates; i++) {
            float64 inlat, inlon;

            // read the coords
            if (inNormalCoordinateOrder) {
               infile >> inlat;
               infile >> inlon;
            } else {
               infile >> inlon;
               infile >> inlat;
            }
            // remove any standard addition
            inlat -= inFalseNorthing;
            inlon -= inFalseEasting;

            // transform
            // To handle utm as both inparam and outparam, go via mc2
            int32 mc2lat, mc2lon;
            float64 outlat, outlon, outh;
            CoordinateTransformer::transformToMC2(inFormat, inlat, inlon, 0,
                  mc2lat, mc2lon, inUTMzone);

            if (outFormat == CoordinateTransformer::mc2) {
               outlat = mc2lat;
               outlon = mc2lon;
            } else {
               CoordinateTransformer::transformFromMC2(mc2lat, mc2lon,
                     outFormat, outlat, outlon, outh, outUTMzone);
            }

            // add any standard addition
            outlat += outFalseNorthing;
            outlon += outFalseEasting;

            // write the outfile
            if (outNormalCoordinateOrder) {
               outfile << outlat << " " << outlon << endl;
            } else {
               outfile << outlon << " " << outlat << endl;
            }
            
         }
      }

      infile >> param;
         
   }

   mc2dbg1 << "TransformCoordinates done:" << endl;
   if (nbrRegions > 0)
      mc2dbg1 << "  nbr regions " << nbrRegions << endl;
   if (nbrPlines > 0)
      mc2dbg1 << "  nbr plines  " << nbrPlines << endl;
   if (nbrPoints > 0)
      mc2dbg1 << "  nbr points  " << nbrPoints << endl;
   return true;

}

bool
changeCoordinateOrder(ifstream& infile, ofstream& outfile)
{

   const uint32 MAX_LENGTH = 200;
   
   //read and copy the mif header
   bool done = false;
   char row[MAX_LENGTH+1];
   while (!infile.eof() && !done) {
      infile.getline(row, MAX_LENGTH);
      if (StringUtility::strncasecmp(row, "Data", 4) == 0) {
         done = true;
         mc2dbg2 << "Done reading/copying the mif header." << endl;
      }
      outfile << row << endl;
   }
   
   if (!done) {
      mc2log << error << "No \"Data\" tag in mif header - reached eof "
             << " without changing the coordinate order." << endl;
      return false;
   }
  
   uint32 nbrInsideMifHeaders = 0;
   uint32 nbrRegions = 0;
   uint32 nbrPlines = 0;
   uint32 nbrPoints = 0;
   
   char param[MAX_LENGTH];
   uint32 nbrPolygons = 1;
   bool point = false;
   
   infile >> param;

   while (!infile.eof() ) {

      // Find out if it is a "Region", "Pline" or "Point"
      if (strcasecmp(param, "Region") == 0) {
         nbrRegions ++;
         infile >> nbrPolygons;
         outfile << "Region  " << nbrPolygons << endl;
      } else if (strcasecmp(param, "Pline") == 0) {
         nbrPlines ++;
         nbrPolygons = 1;
         outfile << "Pline";
      } else if (strcasecmp(param, "Point") == 0) {
         nbrPoints ++;
         nbrPolygons = 1;
         point = true;
         outfile << "Point ";
      } else if (strcasecmp(param, "VERSION") == 0) {
         // another mcm map has been printed with mid/mif, read a mif header
         done = false;
         while (!infile.eof() && !done) {
            infile.getline(row, MAX_LENGTH);
            if (StringUtility::strncasecmp(row, "Data", 4) == 0) {
               done = true;
               nbrInsideMifHeaders++;
               mc2dbg2 << "Inside mif header " << nbrInsideMifHeaders
                       << " after " << nbrRegions << ":" << nbrPlines 
                       << ":" << nbrPoints << endl;
            }
         }
         infile >> param;
         // copied
         if (strcasecmp(param, "Region") == 0) {
            nbrRegions ++;
            infile >> nbrPolygons;
            outfile << "Region  " << nbrPolygons << endl;
         } else if (strcasecmp(param, "Pline") == 0) {
            nbrPlines ++;
            nbrPolygons = 1;
            outfile << "Pline";
         } else if (strcasecmp(param, "Point") == 0) {
            nbrPoints ++;
            nbrPolygons = 1;
            point = true;
            outfile << "Point ";
         } else {
            mc2log << error << "No mif feature to read" << endl;
            return false;
         }
         
      } else {
         // no region, pline or point to read
         mc2log << error << "No \"Region\", \"Pline\" or \"Point\""
                << " to read" << endl;
         mc2dbg << "Read \"" << param << "\" , nbrpoints=" 
                << nbrPoints << ", nbrplines=" << nbrPlines 
                << ", nbrregions=" << nbrRegions << endl;
         return false;
      }

      // Read all the polygons
      for (uint32 p = 0; p < nbrPolygons; p++) {
         // Read the number of coordinates
         uint32 nbrCoordinates;
         if (point) {
            nbrCoordinates = 1;
         } else {
            infile  >> nbrCoordinates;
            outfile << "  " << nbrCoordinates << endl;
         }
         mc2dbg8 << "   nbrCoordinates = " << nbrCoordinates << endl;

         // Change the order
         for (uint32 i = 0; i < nbrCoordinates; i++) {
            float64 coord1, coord2;

            infile >> coord1 >> coord2;
            outfile << coord2 << " " << coord1 << endl;

         }
      }

      infile >> param;
         
   }

   mc2dbg1 << "ChangeOrderOfCoordinates: done" << endl
           << "  nbr regions " << nbrRegions << endl
           << "  nbr plines  " << nbrPlines << endl
           << "  nbr points  " << nbrPoints << endl;
   return true;

}

bool
inspectCoordinates(ifstream& infile, uint32 x)
{

   const uint32 MAX_LENGTH = 200;

   if (x == 0) {
      mc2log << error << "0 is an invalid number, "
             << "the numbering starts at 1." << endl;
      return false;
   }
   
   //read the mif header
   bool done = false;
   char row[MAX_LENGTH+1];
   while (!infile.eof() && !done) {
      infile.getline(row, MAX_LENGTH);
      if (StringUtility::strncasecmp(row, "Data", 4) == 0) {
         done = true;
         mc2dbg2 << "Done reading the mif header." << endl;
      }
   }
   
   if (!done) {
      mc2log << error << "No \"Data\" tag in mif header - reached eof "
             << " without searching for coordinates." << endl;
      return false;
   }
  

   // prepare for reading the mif-fetaures
   const int maxLineLength =16380;
   char lineBuffer[maxLineLength];
   lineBuffer[0] = '\0';
   
   char param[MAX_LENGTH];
   uint32 nbrPolygons = 1;
   bool point = false;
   bool line = false;
   
   uint32 nbrGfx = 1;
   bool found = false;
   cout << setprecision(12);
   
   infile >> param;

   while (!infile.eof() && !found) {

      bool cont = true;
      line = false;
      point = false;
      
      // Find out if it is a "Region", "Pline" or "Point"
      if (strcasecmp(param, "Region") == 0) {
         infile >> nbrPolygons;
      } else if (strcasecmp(param, "Pline") == 0) {
         nbrPolygons = 1;
      } else if (strcasecmp(param, "Line") == 0) {
         nbrPolygons = 1;
         line = true;
      } else if (strcasecmp(param, "Point") == 0) {
         nbrPolygons = 1;
         point = true;
      } else {
         // no region, pline or point to read
         cont = false;
         //return false;
      }

      if ( cont ) {
         // OUT print the feature type and number of poygons
         if (nbrGfx == x) {
            found = true;
            mc2log << info << "Found gfx number " << x << endl;
            cout << param;
            if (strcasecmp(param,"Region") == 0)
               cout << " " << nbrPolygons;
            if (!point)
               cout << endl;
            else
               cout << " ";
         }

         // Read all the polygons
         for (uint32 p = 0; p < nbrPolygons; p++) {
            // Read the number of coordinates
            uint32 nbrCoordinates;
            if (point) {
               nbrCoordinates = 1;
            } else if (line) {
               nbrCoordinates = 2;
            } else {
               infile  >> nbrCoordinates;
            }
            mc2dbg8 << "   nbrCoordinates = " << nbrCoordinates << endl;

            // OUT print number of coordinates
            if (nbrGfx == x) {
               if (!point && !line)
                  cout << "  " << nbrCoordinates << endl;
            }

            for (uint32 i = 0; i < nbrCoordinates; i++) {
               float64 coord1, coord2;

               infile >> coord1 >> coord2;

               // OUT print the coordinates
               if (nbrGfx == x) {
                  cout << coord1 << " " << coord2 << endl;
               }

            }
         }

         infile >> param;
         nbrGfx ++;
      
      } else {
         // perhaps read a "Pen" or any other MapInfo tags.
         mc2dbg4 << "No \"Region\", \"Pline\" or \"Point\""
                 << " to read" << endl;
         mc2dbg4 << "Reading shape nbr " << nbrGfx << ", param = \"" 
                 << param << "\"" << endl;
         // read this row, and get next param
         infile.getline(lineBuffer, maxLineLength);
         infile >> param;
      }
         
   }

   if ( infile.eof() && !found ) {
      mc2log << error << "inspectCoordinates: only " << nbrGfx-1 
                      << " gfx's in the file." << endl;
      return false;
   }

   mc2dbg1 << "inspectCoordinates: done" << endl;
   return true;

}

void
convertCoordinate( CommandlineOptionHandler coh,
                     CoordinateTransformer::format_t informat )
{
   if ( coh.getTailLength() != 2 ) {
      mc2log << error << "Needs both latitude and longitude in tail!" << endl;
      exit(1);
   }

   const char* latStr = coh.getTail(0);
   const char* lonStr = coh.getTail(1);
   
   mc2log << info << "Convert coordinate " << latStr << " " << lonStr << endl
          << "Negative coordinates might have round-off errors" << endl;

   // Extract coords from coord string
   float64 inlat = 0, inlon = 0;
   char* foo;
   Utility::getFloat64(latStr, foo, inlat);
   Utility::getFloat64(lonStr, foo, inlon);

   cout << setprecision(9);

   // Find out in what coord sys the coordinates are expressed
   CoordinateTransformer::format_t format = informat;
   if ( informat == CoordinateTransformer::nbrCoordinateTypes ) {
      char* unsignedLatStr = StringUtility::replaceString(latStr, "-", "");
      char* unsignedLonStr = StringUtility::replaceString(lonStr, "-", "");
      if ( unsignedLatStr == NULL )
         unsignedLatStr = StringUtility::newStrDup(latStr);
      if ( unsignedLonStr == NULL )
         unsignedLonStr = StringUtility::newStrDup(lonStr);

      if ( StringUtility::onlyDigitsInString(unsignedLatStr) &&
           StringUtility::onlyDigitsInString(unsignedLonStr) ) {
         format = CoordinateTransformer::mc2;
      }
   }

   // Check if format was detected..
   if ( format == CoordinateTransformer::nbrCoordinateTypes ) {
      mc2log << error << "Unable to detect coord format, please use "
             << "--inFormat" << endl;
      exit(1);
   }

   // First convert to mc2
   int32 mc2lat = 0, mc2lon = 0;
   if ( format == CoordinateTransformer::mc2 ) {
      mc2lat = int32(inlat);
      mc2lon = int32(inlon);
   } else {
      CoordinateTransformer::transformToMC2(format, inlat, inlon, 0,
            mc2lat, mc2lon);
   }
   
   // Then convert and print
   cout << " coord mc2:      " << mc2lat << " " << mc2lon << endl;
   
   float64 outlat = 0, outlon = 0, outh = 0;
   CoordinateTransformer::transformFromMC2(mc2lat, mc2lon,
            CoordinateTransformer::wgs84deg, outlat, outlon, outh);
   cout << " coord wgs84deg: " << outlat << " " << outlon << endl;
   
   CoordinateTransformer::transformFromMC2(mc2lat, mc2lon,
            CoordinateTransformer::wgs84rad, outlat, outlon, outh);
   cout << " coord wgs84rad: " << outlat << " " << outlon << endl;

}


void uint2str(uint32 ui, string& s)
{
   s.erase();
   char tmp[24];
   tmp[23] = 0;
   int pos = 23;
   
   while(ui > 0) {
      tmp[--pos] = ('0' + (char)(ui % 10));
      ui /= 10;
   }

   s = &tmp[pos];
}

int main(int argc, char* argv[])
{

   CommandlineOptionHandler coh(argc, argv, 1);
   coh.setTailHelp("mif or text file");
   coh.setSummary("Converts the coordinates in a mif or text file to "
                  "be expressed in another way, either by "
                  "transformation or by changing the order "
                  "of the coordinates. In case of mif file, the mif header "
                  "will be read and copied to the outfile.\n"
                  "Also includes methods to inspect coordinates.");

   string strmaxuint32;
   uint2str(MAX_UINT32, strmaxuint32);
   string strzero;
   uint2str(0, strzero);

   // --------- transform the coordinates
   bool CL_transformCoords = false;
   coh.addOption("-t", "--transformCoordinates",
                 CommandlineOptionHandler::presentVal,
                 1, &CL_transformCoords, "F",
                 "Transform the coordinates in the mif file in tail to be "
                 "expressd in another coordinate system.\n"
                 "Use togheter with the options for coordinate format, "
                 "coordinate order, any false easting/northing etc, and "
                 "saveAs.");

   // --------- change order of the coordinates
   bool CL_changeOrder = false;
   coh.addOption("-c", "--changeCoordinateOrder",
                 CommandlineOptionHandler::presentVal,
                 1, &CL_changeOrder, "F",
                 "Change the order of the coordinates in the mif file in "
                 "tail, i.e. reverse the axis (latlon->lonlat or the "
                 "opposite). Combine with saveAs.");

   // --------- find and print coordinates for the x:th gfx
   uint32 CL_inspectCoords = MAX_UINT32;
   coh.addOption("-i", "--inspectCoordinates",
                 CommandlineOptionHandler::uint32Val,
                 1, &CL_inspectCoords, strmaxuint32.c_str(),
                 "Find and print the coordinates for the x:th gfx "
                 "in the mif file in tail. Used for inspecting coordinates. "
                 "The numbering of the gfxs is 1,2,3...");

   // --------- outFile
   char* CL_outFile = NULL;
   coh.addOption("-s", "--saveAs",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_outFile, "",
                 "The output converted file will be given this name.");

   // --------- indata coordinate format
   char* CL_inFormat = NULL;
   coh.addOption("-f", "--inFormat",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_inFormat, "",
                 "The indata coordinates are in the specified format\n"
                 "mc2\t\t     -- mc2\n"
                 "rt90_XYH\t  -- rt90 2,5 gon V\n"
                 "utm\t\t     -- utm\n"
                 "wgs84_deg\t -- wgs84, decimal degrees\n"
                 "wgs84_rad\t -- wgs84, radians");

   // --------- outdata coordinate format
   char* CL_outFormat = NULL;
   coh.addOption("-F", "--outFormat",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_outFormat, "",
                 "The outdata coordinates are wanted in the specified format\n"
                 "mc2\t\t     -- mc2 \n"
                 "rt90_XYH\t  -- rt90 2,5 gon V\n"
                 "utm\t\t     -- utm\n"
                 "wgs84_deg\t -- wgs84, decimal degrees\n"
                 "wgs84_rad\t -- wgs84, radians");
   
   // --------- indata coordinateOrder
   char* CL_inCoordOrder = NULL;
   coh.addOption("-o", "--inCoordinateOrder",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_inCoordOrder, "latlon",
                 "The indata coordinates are in the specified order\n"
                 "latlon (default)\n"
                 "lonlat");
   
   // --------- outdata coordinateOrder
   char* CL_outCoordOrder = NULL;
   coh.addOption("-O", "--outCoordinateOrder",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_outCoordOrder, "latlon",
                 "The outdata coordinates are wanted in the specified order\n"
                 "latlon (default)\n"
                 "lonlat");
   
   // --------- indata utmzone
   uint32 CL_inUTMzone = MAX_UINT32;
   coh.addOption("-u", "--inUTMzone",
                 CommandlineOptionHandler::uint32Val,
                 1, &CL_inUTMzone, strmaxuint32.c_str(),
                 "The UTM zone number for the indata if that is "
                 "expressed in UTM coordinates.");
   
   // --------- outdata utmzone
   uint32 CL_outUTMzone = MAX_UINT32;
   coh.addOption("-U", "--outUTMzone",
                 CommandlineOptionHandler::uint32Val,
                 1, &CL_outUTMzone, strmaxuint32.c_str(),
                 "The wanted UTM zone number for the outdata if that "
                 "is to be expressed in UTM coordinates.");

   // --------- indata false Northing
   int CL_inFalseNorthing = 0;
   coh.addOption("-n", "--inFalseNorthing",
                 CommandlineOptionHandler::integerVal,
                 1, &CL_inFalseNorthing, strzero.c_str(),
                 "Any standard addition to the latitude value "
                 "of the indata.");
   
   // --------- outdata false Northing
   int CL_outFalseNorthing = 0;
   coh.addOption("-N", "--outFalseNorthing",
                 CommandlineOptionHandler::integerVal,
                 1, &CL_outFalseNorthing, strzero.c_str(),
                 "Any standard addition to the latitude value "
                 "of the outdata.");
   
   // --------- indata false Easting
   int CL_inFalseEasting = 0;
   coh.addOption("-e", "--inFalseEasting",
                 CommandlineOptionHandler::integerVal,
                 1, &CL_inFalseEasting, strzero.c_str(),
                 "Any standard addition to the longitude value "
                 "of the indata.");
   
   // --------- outdata false Easting
   int CL_outFalseEasting = 0;
   coh.addOption("-E", "--outFalseEasting",
                 CommandlineOptionHandler::integerVal,
                 1, &CL_outFalseEasting, strzero.c_str(),
                 "Any standard addition to the longitude value "
                 "of the outdata.");
   
   // --------- convert coordinates in a text file
   bool CL_convertTextFile = false;
   coh.addOption("", "--convertTextFile",
                 CommandlineOptionHandler::presentVal,
                 1, &CL_convertTextFile, "F",
                 "Convert the coordinates in the text file in tail. "
                 "Use togheter with the options for coordinate format, "
                 "any false easting/northing etc. Specify file separator "
                 "and lat/lon positions. Specify also outfile name.");

   // --------- latitude position for text file conversions
   int CL_latPos = 0;
   coh.addOption("", "--latPos",
                 CommandlineOptionHandler::uint32Val,
                 1, &CL_latPos, strzero.c_str(),
                 "The column of latitude coordinate in text file.\n"
                 "Counted as 0, 1, 2, ...  Column 0 is reserved for an idnbr.");

   // --------- longitude position for text file conversions
   int CL_lonPos = 0;
   coh.addOption("", "--lonPos",
                 CommandlineOptionHandler::uint32Val,
                 1, &CL_lonPos, strzero.c_str(),
                 "The column of longitude coordinate in text file.\n"
                 "Counted as 0, 1, 2, ...  Column 0 is reserved for an idnbr.");

   // --------- text file separator
   char CL_fileSeparator = '\0';
   coh.addOption("", "--fileSep",
                 CommandlineOptionHandler::characterVal,
                 1, &CL_fileSeparator, "\0",
                 "The text file separator.");

   // --------- convert a coordinate
   bool CL_convertCoord = false;
   coh.addOption("", "--convertCoord",
                 CommandlineOptionHandler::presentVal,
                 1, &CL_convertCoord, "F",
                 "Converts one coordinate. Give the coord in tail, latitude "
                 "followed by longitude. The method tries to detects the "
                 "coord sys automatically, but it is also possible to give "
                 "with --inFormat. Converts the coordinate to misc formats "
                 "and print result to stdout.");


   if (!coh.parse()) {
      cerr << argv[0] << ": Error on commandline, (-h for help)" << endl;
      exit(1);
   }
   

   // Check if there is anything to do
   if ( !CL_transformCoords && !CL_changeOrder && 
        (CL_inspectCoords == MAX_UINT32) &&
        !CL_convertTextFile &&
        !CL_convertCoord ) {
      mc2dbg << "Nothing to do." << endl;
      exit(1);
   }
   
   // Outfile must be given for several of the methods
   if ( (CL_outFile == NULL) && 
        (CL_inspectCoords == MAX_UINT32) &&
        !CL_convertCoord ) {
      mc2dbg << "Outfile must be specified" << endl;
      exit(1);
   }

   //Get the in/out-data coordinate formats
   CoordinateTransformer::format_t inCoordFormat = 
      CoordinateTransformer::nbrCoordinateTypes;
   CoordinateTransformer::format_t outCoordFormat = 
      CoordinateTransformer::nbrCoordinateTypes;
   if ( CL_inFormat != NULL ) {
      MC2String inCoord;
      inCoord.append(CL_inFormat);
      inCoordFormat = CoordinateTransformer::getCoordinateType(inCoord);
   }
   if ( CL_outFormat != NULL ) {
      MC2String outCoord;
      outCoord.append(CL_outFormat);
      outCoordFormat = CoordinateTransformer::getCoordinateType(outCoord);
   }
 
   // Coordinate order
   bool inNormalCoordinateOrder = true;
   if (strcasecmp(CL_inCoordOrder,"lonlat") == 0) {
      inNormalCoordinateOrder = false;
   }
   bool outNormalCoordinateOrder = true;
   if (strcasecmp(CL_outCoordOrder,"lonlat") == 0) {
      outNormalCoordinateOrder = false;
   }
   
   
   // Action part 1
   bool exitHere = false;
   
   if ( CL_convertCoord ) {
      convertCoordinate( coh, inCoordFormat );
      exitHere = true;
   }
   
   if (exitHere) {
      exit(0);
   }

   // If no coord type given for inFormat and outFormat, set mc2.
   if ( inCoordFormat == CoordinateTransformer::nbrCoordinateTypes )
      inCoordFormat = CoordinateTransformer::mc2;
   if ( outCoordFormat == CoordinateTransformer::nbrCoordinateTypes )
      outCoordFormat = CoordinateTransformer::mc2;
   
   // Open files for reading and writing
   const char* inFileName = coh.getTail(0);
   const char* outFileName = CL_outFile;

   ifstream infile(inFileName);
   ofstream outfile(outFileName);
   if(!infile) {
      mc2log << error << "No infile exists with such name \"" 
             << inFileName << "\"" << endl;
      infile.close();
      exit (1);
   } else {
      mc2dbg << "Reading from \"" << inFileName << "\"";
      if (CL_inspectCoords == MAX_UINT32) {
         mc2dbg << ", result will be written to \"" 
                << outFileName << "\"";
      }
      mc2dbg << endl;
   }

   if ( (CL_outFormat == NULL) ||
        (strcasecmp(CL_outFormat, "mc2") == 0) )
      outfile << setprecision(10);
   else if ( strcasecmp(CL_outFormat, "rt90_XYH") == 0)
      outfile << setprecision(12);
   else if ( strcasecmp(CL_outFormat, "utm") == 0)
      outfile << setprecision(12);
   else if ( strcasecmp(CL_outFormat, "wgs84_deg") == 0)
      outfile << setprecision(9);
   else if ( strcasecmp(CL_outFormat, "wgs84_rad") == 0)
      outfile << setprecision(9);
   else
      outfile << setprecision(12);
   
   
   // Action part 2
   if (CL_transformCoords && !CL_convertTextFile) {
      mc2dbg << "To transform the coordinates." << endl;
      transformCoordinates(infile, outfile, 
            inCoordFormat, inNormalCoordinateOrder, 
            CL_inFalseNorthing, CL_inFalseEasting,
            CL_inUTMzone,
            outCoordFormat, outNormalCoordinateOrder,
            CL_outFalseNorthing, CL_outFalseEasting,
            CL_outUTMzone);
   }
      
   if (CL_changeOrder) {
      mc2dbg << "To change the coordinate order." << endl;
      changeCoordinateOrder(infile, outfile);
   }

   if (CL_inspectCoords != MAX_UINT32) {
      mc2dbg << "To inspect coordinates of gfx number " 
             << CL_inspectCoords << endl;
      inspectCoordinates(infile, CL_inspectCoords);
   }

   if (CL_convertTextFile) {
      // Check that file separator and columns for lon and lat was given.
      mc2dbg << "To convert coordinates in text file." << endl;
      mc2dbg << "File separator=\"" << CL_fileSeparator << "\" latPos="
             << CL_latPos << " lonPos=" << CL_lonPos << endl;
      mc2dbg << "Infile:  " << inFileName << endl
             << "Outfile: " << outFileName << endl;
      if (CL_fileSeparator == '\0') {
         mc2dbg << "No file separator specified - end" << endl;
         exit (1);
      } else if (CL_latPos == 0) {
         mc2dbg << "No lat position specified - end" << endl;
         exit (1);
      } else if (CL_lonPos == 0) {
         mc2dbg << "No lon position specified - end" << endl;
         exit (1);
      } else if (inCoordFormat == outCoordFormat) {
         mc2dbg << "No inFormat or outFormat for coordinates specified." 
                << endl;
         exit (1);
      }
      
      // All in data ok - read the text file and convert.
      convertTextFileCoordinates(
            infile, outfile, 
            CL_fileSeparator, CL_latPos, CL_lonPos,
            inCoordFormat, CL_inFalseNorthing, 
            CL_inFalseEasting, CL_inUTMzone,
            outCoordFormat, CL_outFalseNorthing, 
            CL_outFalseEasting, CL_outUTMzone);
   }
   
   mc2dbg << "CoordConvert: done!" << endl;
   mc2dbg << "Negative coordinates might have round-off errors" << endl;

   return 0;
}


