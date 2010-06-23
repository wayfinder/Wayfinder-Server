/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSGFXDATA_H
#define GMSGFXDATA_H

#include "config.h"
#include "DataBuffer.h"
#include "GfxDataFull.h"

#include <set>
#include <math.h>

#define MAX_MUNICIPAL_SQUARE_DISTANCE 25

class OldGenericMap;

/**
  *   GenerateMapServer GfxData. A GfxData with some extra features 
  *   needed when the map is created.
  *
  */
class GMSGfxData : public GfxDataFull {
   
   /// Make GMSPolyUtility friend to access fillCoordMap
   friend class GMSPolyUtility;
   
   public:
      /**
       *    Create a new GfxData and fill it with the outline of the
       *    GfxData's given as parameter. The array with GfxDatas will
       *    not be destroyed, and these GfxDatas will not be deleted.
       *    The GfxData returned by this method <i>must be deleted
       *    by the caller!</i>
       *    @param   gfxs     An array with pointers to the GfxData's
       *                      that should be included in the new GfxData.
       *    @param   nbrGfxs  The number of GfxData's in the gfxs-array.
       *    @return  A new GfxData, created in this method, that is set
       *             to the outline of the ones given as parameter.
       *
       */
      static GMSGfxData* createGfxData(GMSGfxData** gfxs, 
                                       int nbrGfxs);

      static GMSGfxData* createNewGfxData(DataBuffer* dataBuffer, 
                                          OldGenericMap* theMap);
      
      static GMSGfxData* createNewGfxData(OldGenericMap* theMap, 
                                          bool createNewPolygon=false);
         
      static GMSGfxData* createNewGfxData(OldGenericMap* theMap, 
                                          const MC2BoundingBox* bbox);

      static GMSGfxData* createNewGfxData(OldGenericMap* theMap, 
                                          const coordinate_type* firstLat,
                                          const coordinate_type* firstLon, 
                                          uint16 nbrCoords, 
                                          bool closed);
      
      static GMSGfxData* createNewGfxData(OldGenericMap* theMap, const GfxData* src);

      
      GMSGfxData() : GfxDataFull() {};


      /**
        *   Concatinates two polygones into one. They have to share at least
        *   one point. Constructed to make it possible to build the gfxData
        *   for the intire map (that consist of a number of municipals).
        *   NB! Not a trust-worthy method!!!
        *
        *   @param   externGfx      Pointer to "the other" gfxData.
        *   @param   externPolygon  Polygon of the externGfx to 
        *                           concatenate. Default set to MAX_UINT16
        *                           which means that all polygones of
        *                           externGfx are to be concatenated.
        */
      bool addOutline( GMSGfxData* externGfx ); 


      /**
        *   Overrides the protected-declaration in GfxData.
        */
      bool updateLength( uint16 polyIdx = MAX_UINT16 );

      /**
        *   Creates a new GMSGfxData and fills it with the outline
        *   of ALL polygons of the complexGfx given as parameter. 
        *   Island polygons (polygons that do not fit with the others)
        *   are added the the returnGfx as new polygons.
        *   The method is quite slow, so it should not be used on 
        *   polygons with too many coordinates.
        *   Holes are removed.
        *
        *   @param complexGfx The gfxData that is to be simplified.
        *   @return  A new GfxData, created in this method, that is filled
        *            with the outline of the one given as parameter.
        */
      static GMSGfxData* removeInsideCoordinates(GfxDataFull* complexGfx);
      
      /**
       *    Merge the polygons of one complex gfxData.
       *    @return  The merged gfxData, or NULL if no merging was done
       *             e.g. if the complex only has 1 polygon.
       */
      static GMSGfxData* mergePolygons(GfxData* complexGfx);

   /**
    *    Write this GfxData to a mif-file.
    *    @param   outfile  The file to save this GfxData into.
    *    @return  True if this GfxData is saved into the file,
    *             false otherwise.
    */
   bool printMif(ofstream& outfile) const;

   /**
    *   Read the header of a mif-file to find out the coordinate system 
    *   for the coordinates in the file. The file is read until "Data" 
    *   is found or until eof(). By default the coordinate system is set 
    *   to "mc2" and the normalOrderOfCoordinates to "true". Then if an 
    *   optional coordinate system (the "Coordsys" tag) is found in the 
    *   mif header, the parameters are updated to comply with that value.
    *   @param  infile    The mif file.
    *   @param  coordsys  Outparam, the coordinate system used.
    *   @param  normalOrderOfCoordinates Outparam, "normal" order is
    *                                    lat,lon or x,y.
    *   @param  utmzone   Outparam, the zone number for utm-coordinates.
    *   @param  falseNorthing Outparam, any addition to the latitude.
    *   @param  falseEasting  Outparam, any addition to the longitude.
    *   
    *   @return True if "Data" tag was found in the mif file, false if 
    *           eof() was reached. Returning true means that, if the 
    *           header contains an optional coordinate system the 
    *           outparameters have been set according to that coordinate 
    *           system, else default values are returned.
    */
   static bool readMifHeader( ifstream& infile,
                       CoordinateTransformer::format_t& coordsys,
                       bool& normalOrderOfCoordinates,
                       uint32& utmzone,
                       int32& falseNorthing, int32& falseEasting );

   /**
    *    Create this gfxData from a mif-file. The method reads only one
    *    pline/region from the file.
    *    @param  infile        The mif file.
    *    @param  coordsys      The coordinate system used in the mif file.
    *    @param  normalOrderOfCoordinates "Normal" order is 
    *                                     lat,lon or x,y.
    *    @param  utmzone       The zone number for utm-coordinates.
    *    @param  falseNorthing Any addition to the latitude coordinate.
    *    @param  falseEasting  Any addition to the longitude coordinate.
    *    @return True if ok.
    */
   bool createGfxFromMif( ifstream& infile,
                          CoordinateTransformer::format_t coordsys,
                          bool normalOrderOfCoordinates,
                          uint32 utmzone,
                          int32 falseNorthing, int32 falseEasting );

   /**
    *    Create this gfxData from a mif-file. Use this method
    *    when there is only ONE gfxdata in the mif-file, e.g. country
    *    polygons for country overview maps. Method reads the mif header 
    *    and then the file, using the "readMifHeader" and 
    *    "createGfxFromMif" methods. Init this gfxData without adding
    *    first polygon.
    *    @param   infile   The file that contains the mif-data.
    *    @return  True if this GfxData is created alright, false
    *             otherwise.
    */
   bool createFromMif( ifstream& infile );


private:

      /**
       *    Merge another gfx data into this one. The polygons are merged
       *    if they are closed, have at least 3 coords, bbox overlaps,
       *    have the same orientation. The result is "falukorv" polygons.
       *    @param   otherGfx    The gfx data to merge into this one.
       *    @return  The result of the merge, or NULL if the two gfx datas 
       *             were not merged.
       */
      GMSGfxData* mergeTwoPolygons( GMSGfxData* otherGfx,
                                    GMSGfxData* polygonGfxs[],
                                    uint16 nbrPolygons,
                                    uint16 p, uint16 mergep,
                                    bool mergePolygonsWithMultipleTouches,
                                    bool& foundMultipleTouches );
      
   /**
    *    Help method to createGfxFromMif.
    *    Read from mif file until next valid mif feature is found.
    *    Valid features are: Region, Pline, Line, Point.
    */
   bool findNextMifFeature( ifstream& infile,
                            bool& region,
                            bool& line,
                            bool& point,
                            uint32& nbrPolygons );

   /**
    * Help function to mergeTwoPolys. Checks if an intersection point between
    * two polys is surrounded by other polys (other than the two considered). 
    * When two polys intersect in one coord only, we only want to merge them 
    * if this intersection point is surrounded by other polys, and thus will 
    * disappear in forthcoming merges with next polygons.
    */
   bool surroundedByOtherPolys( GMSGfxData* otherGfx,
                                GMSGfxData* polygonGfxs[],
                                uint16 nbrPolygons,
                                uint32 p, uint32 mergep,
                                uint32 mySharedEnd,
                                uint32 myNbrCoords,
                                uint32 otherSharedEnd,
                                uint32 otherNbrCoords );

   /**
    * Help function to surroundedByOtherPolys. Checks if a coordinate pair 
    * exists as an adjacent coordinate pair in a polygon.
    */

   bool
   coordPairInPoly( GMSGfxData* gfxData,
                    coordinate_type coord1Lat,
                    coordinate_type coord1Lon,
                    coordinate_type coord2Lat, 
                    coordinate_type coord2Lon );

                                 
};

#endif

