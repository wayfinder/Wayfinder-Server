/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COORDINATETRANSFORMER_H
#define COORDINATETRANSFORMER_H

#include "config.h"
#include "MC2String.h"

#include <map>


class MC2Coordinate;
class CoordTransHelper;
class WGS84Coordinate;

#ifdef _MSC_VER
#  ifdef _DEBUG
#     pragma warning( disable : 4786 )
#  endif
#endif

/**
 *    Class with static methods to be used for converting between
 *    different coordinate and reference systems.
 *
 *    Position can be given in different reference systems. A reference 
 *    system is based on a reference ellipsoid and defines coordinates 
 *    in relation to some points with known coordinates in that 
 *    reference system.
 *
 *    This class uses 2 different reference systems.
 * 
 *    - <B>SWEREF93;</B> (equals wgs84) This is a global reference system 
 *       for positioning in 3 dimensions. It is the system used for GPS 
 *       measuring in Sweden and is based on a global reference ellipsoid 
 *       called GRS80. This ellipsoid fits well with the entire earth and 
 *       differs at most 100 meters above or under the mean sea level on 
 *       earth.
 *
 *    - <B>rr92 "Rikets nät";</B> This is the referece system used on 
 *       national basis in Sweden. This system is based on a local 
 *       reference ellipsoid (Bessel's ellipsoid) which fits with the earth 
 *       in Sweden. It differs at most 10 meters above or under the mean 
 *       sea level in Sweden. This difference is denoted geoid height. RR92 
 *       consists of one 2-dimensional system called rt90 which gives 
 *       coordinates in gauss' map-projection, one system for heights 
 *       called rh70 which gives height above the mean sea level, and one 
 *       system called rn92 which gives the geoid height.
 *
 *    The reference system for positioning with GPS is called WGS84. It 
 *    is based on an global reference ellipsoid with the same name. This 
 *    wgs84 ellipsoid is based on GRS80. The difference between these 
 *    two ellipsoids is very small so constants for the wgs84 ellipsoid 
 *    can be used when giving positions in SWEREF93.
 *
 *    Position can be given in different coordinate systems. There are 
 *    3 posibilities:
 *
 *    - <B>Cartesian:</B> This system gives the position in an orthogonal 
 *       coordinate system @f$XYZ@f$ with origo in the center of the reference 
 *       ellipsoid (close the the centre of earth),the Z-axis parallell 
 *       to the rotational axis, the X-axis pointing towards the prime 
 *       meridian (Greenwich, lat=0 and lon=0) and the Y-axis orthogonal  
 *       to both Z and X. It is important to note that different systems 
 *       (like rr92 XYZ and SWEREF93 XYZ) have different centres.
 *
 *    - <B>Geodetic:</B> Positions in this system are mostly denoted 
 *       @f$\phi,\lambda,h@f$ where @f$\phi@f$ and @f$\lambda@f$ are 
 *       geodetic latitude and geodetic longitude and @f$h@f$ is altitude 
 *       given relative to the reference ellipsoid. For use in this class 
 *       @f$\phi@f$ and @f$\lambda@f$ must be given in radians. Exception is 
 *       the enum type wgs84deg, where of course degrees are used.
 *
 *    - <B>Planar map projection:</B> In the third types of coordinate system, 
 *       the coordinates are given in a plane, i.e. in a map projection. 
 *       Positions are expressed by @f$x,y@f$, where x usually gives the 
 *       northing (most often counted from the equator) and y gives the 
 *       easting from the central meridian of the map projection. In some 
 *       projections y gives northing and x easting. To avoid dealing with 
 *       negative cordinate values often there is a standard addition to 
 *       one or both axis.
 *       
 *    In this class currently two map projections are used:
 *
 *    - <B>rt90 2,5 gon Väst</B> This projection is used for official 
 *       mapping in Sweden and constitutes the plane part of the Swedish 
 *       reference system rr92. Positions are given according to @f$x,y,h@f$ 
 *       where the x-axis points towards north starting at the equator and 
 *       the y-axis points towards east starting at the central meridian 
 *       for the projection. This is located at 17.56 gon east of Greenwich, 
 *       corresponding to 15 48' 29''.8 degrees. To avoid negative values 
 *       of @f$y@f$ west of the central meridian there is a standard 
 *       addition of 1 500 000 meters. @f$h@f$ gives the height above the 
 *       mean sea level.
 *       
 *    - <B>UTM</B> Universal Transverse Mercator is a system of map
 *       projections normally connected to the wgs84 refrence system 
 *       (equal to sweref93). In UTM the earth is divided into 60 zones 
 *       of 6 degrees width. Each zone is projected with a central 
 *       meridian in the center of the zone. E.g. 3 degrees  east of 
 *       Greenwich is the central meridian of zone number 31. The 
 *       x-coordinate gives the northing counted from the equator, and y 
 *       gives the easting counted from the central meridian. To avoid 
 *       distortions in the outer areas of the zone a scale factor is applied
 *       to the coordinates. Most often there are various additions and/or 
 *       subtractions to either of the axis to avoid negative or too large
 *       coordinate values. This class manage UTM coordinates without any
 *       additions, while they must be removed before transforming the 
 *       coordinates.
 *
 *
 *    Most of the theory can be found in
 *    - <I>GPS-Beräkning för stomnät</I> by <I>Lotti Jivall</I> at 1991-11-27.
 *    - <I>SWEREF 93 - ett nytt svenskt referenssystem</I> by 
 *      <I>Bo-Gunnar Reit</I>.
 *
 *    Both of these can be queried at
 *    @verbatim
         Lantmäteriverket
         Blankettförrådet
         801 82  GÄVLE
      @endverbatim
 *
 *    All formulas used in this class can be found in Handbok till  
 *    mätningskungörelsen, HMK-Ge:GPS published by Lantmäteriverket.
 *
 *    But the main issue for this code is: <I>"don't touch if it is still 
 *    working"</I>.
 * 
 */
class CoordinateTransformer {
   public:
      /**
        *   Enumeration with the supported formats.
        */
      enum format_t {
         sweref93_LLA,
         sweref93_XYZ,
         rr92_XYZ,
         rt90_LLA,              // This should be called rr92_LLA
         rt90_2_5gonV_rh70_XYH, // This should be called rt90_2_5gonV_rh70_xyh
         mc2,
         wgs84rad,
         wgs84deg,
         utm,
         /// Danish System34J, System34S or System45
         system34,
         /// UTM in the ED50 system
         utm_ed50,
         /// The number of coordinate types.
         nbrCoordinateTypes
      };

      /**
        *   Method used to transform between the different coordinate-
        *   and reference systems. The outdata is given by reference-
        *   parameters, and given in the format described by the 
        *   parameter.
        *
        *   @param   informat  The format of the given indata.
        *   @param   in1       Inparameter with the value of 
        *                      latitude, X or x.
        *   @param   in2       Inparameter with the value of 
        *                      longitude, Y or y.
        *   @param   in3       Inparameter with the value of 
        *                      h, Z or h.
        *   @param   outformat The desired format of the outdata.
        *   @param   out1      Outparameter that is set to 
        *                      latitude, X or x.
        *   @param   out2      Outparameter that is set to 
        *                      longitude, Y or y.
        *   @param   out3      Outparameter that is set to 
        *                      h, Z or h.
        *   @param   utmzone   If transforming from UTM-coordinates the
        *                      UTM zone number MUST be given. Preferrable
        *                      also when transforming to UTM, but otherwise
        *                      the zone number is calculated from the
        *                      longitude value.
        */
      static void transform(  format_t informat, float64 in1, 
                              float64 in2, float64 in3, 
                              format_t outformat, float64 &out1, 
                              float64 &out2, float64 &out3,
                              uint32 utmzone = MAX_UINT32);

      /**
        *   Method used to transform from any coordinate system to
        *   the one used by MC2.
        *
        *   @param   informat The format of the given indata.
        *   @param   in1      Inparameter with the value of 
        *                     latitude, X or x.
        *   @param   in2      Inparameter with the value of 
        *                     longitude, Y or y.
        *   @param   in3      Inparameter with the value of 
        *                     h, Z or h.
        *   @param   mc2_lat  Outparameter that is set to the vertical 
        *                     part of outdata in mc2-latitudes.
        *   @param   mc2_lon  Outparameter that is set to the horizontal 
        *                     part of outdata in mc2-longitudes.
        *   @param   utmzone  If transforming from UTM coordinates, the 
        *                     zone number must be given.
        */
      static void transformToMC2(format_t informat, float64 in1, 
                                 float64 in2, float64 in3, 
                                 int32 &mc2_lat, int32 &mc2_lon,
                                 uint32 utmzone = MAX_UINT32);


   static WGS84Coordinate transformToWGS84( const MC2Coordinate& coordIn );
   static MC2Coordinate transformToMC2( const WGS84Coordinate& coordIn );
      /**
        *   Method used to transform from any coordinate system to
        *   the one used by MC2.
        *
        *   @param   informat The format of the given indata.
        *   @param   in1      Inparameter with the value of 
        *                     latitude, Y or y.
        *   @param   in2      Inparameter with the value of 
        *                     longitude, X or x.
        *   @param   in3      Inparameter with the value of 
        *                     h, Z or h.
        *   @param   utmzone  If transforming from UTM coordinates, the 
        *                     zone number must be given.
        *   @return Transformed MC2Coordinate.
        */
      
      static MC2Coordinate transformToMC2( format_t informat,
                                           float64 in1, 
                                           float64 in2,
                                           float64 in3,
                                           uint32 utmzone = MAX_UINT32 );

      /**
        *   Method used to transform from the coordinate system used
        *   by MC2 to any coordinate system.
        *
        *   @param   mc2_lat   Inparameter that is the vertical 
        *                      part of indata in mc2-latitudes.
        *   @param   mc2_lon   Inparameter that is the horizontal 
        *                      part of indata in mc2-longitudes.
        *   @param   outformat The format of the desired outdata.
        *   @param   out1      Outparameter with the value of 
        *                      latitude, X or x.
        *   @param   out2      Outparameter with the value of 
        *                      longitude, Y or y.
        *   @param   out3      Outparameter with the value of 
        *                      h, Z or h.
        *   @param   utmzone   If transforming to UTM coordinates, the 
        *                      zone number most preferrable should be given.
        *                      If not, it is calculated from the longitude
        *                      value.
        *
        *   Since height does not exist in MC2 it is set to 0 to 
        *   enable calculations.
        */
      static void transformFromMC2(int32 mc2_lat, int32 mc2_lon,
                                   format_t outformat, float64 &out1, 
                                   float64 &out2, float64 &out3,
                                   uint32 utmzone = MAX_UINT32);

      /**
       *    Get the coordinate reference system from a string.
       *    @param s A string with the name of the coordinate reference 
       *             system.
       *    @return  The format_t described by <tt>s</tt>. 
       *             <tt>nbrCoordinateTypes</tt> will be returned if no 
       *             type matches <tt>s</tt>.
       */
      static format_t getCoordinateType( const MC2String& s);

      /**
        *   Get the UTM zone number for a longitude value in wgs84.
        *   @param   lon      The longitude value.
        *   @param   radians  True if the longitude is expressed in 
        *                     radians (default), false is expressed 
        *                     in degrees.
        *   @return  The zone for the given longitude.
        */
      static uint32 getUTMZoneForLongitude(float64 lon, bool radians = true);

      /**
        *   Get the central meridian of a UTM zone (longitude value).
        *   @param   zone     The zone for which to get the central meridian.
        *   @param   radians  True if the meridian should be expressed
        *                     in radians (default), false if it should be
        *                     expressed in degrees.
        *   @return  The central meridian of a utm zone.
        */
      static float64 getCentralMeridianForUTMZone(
                           uint32 utmzone, bool radians = true);
      
   private:
      /**
        *   @name Converting methods
        *   Internal converting methods. These are called from the 
        *   public transform-methods.
        */
      //@{
         /** 
          *    "Rikets nät" From xyh (normal notation) to LatLonAlt.
          */
         static void transform_rt90_XYH_to_rt90_LLA( 
                                  float64 gauss_x, float64 gauss_y, 
                                  float64 gauss_rh70_h, float64 &rr92_lat, 
                                  float64 &rr92_lon, float64 &rr92_h);

         /** 
          *    "Rikets nät" From LatLonAlt to XYZ.
          */
         static void transform_rt90_LLA_to_rr92_XYZ(
                                  float64 rr92_lat, float64 rr92_lon, 
                                  float64 rr92_h, float64 &rr92_X, 
                                  float64 &rr92_Y, float64 &rr92_Z);


         /**
          *    Transforms from UTM ed50 to LLA, also ed50.
          */         
         static void transform_UTM_to_ed50_LLA(
            float64 utmLat, float64 utmLon, float64 utmH, uint32 utmzone,
            float64 &ed50Lat, float64 &ed50Lon, float64 &ed50H);
         
         /**   
          *    From "Rikets nät" XYZ to "SWEREF93" XYZ,  
          *    this is translation, rotation and scaling.
          */
         static void transform_rr92_XYZ_to_sweref93_XYZ(
                                  float64 rr92_X, float64 rr92_Y, 
                                  float64 rr92_Z, float64 &sweref93_X, 
                                  float64 &sweref93_Y, float64 &sweref93_Z);

         /** 
          *    "SWEREF93" From XYZ to LatLonAlt.
          */
         static void transform_sweref93_XYZ_to_sweref93_LLA(
                                  float64 sweref93_X, float64 sweref93_Y, 
                                  float64 sweref93_Z, float64 &sweref93_lat, 
                                  float64 &sweref93_lon, float64 &sweref93_h);

         /** 
          *    "SWEREF93" From LatLonAlt to XYZ.
          */
         static void transform_sweref93_LLA_to_sweref93_XYZ(
                                  float64 sweref93_lat, float64 sweref93_lon, 
                                  float64 sweref93_h, float64 &sweref93_X, 
                                  float64 &sweref93_Y, float64 &sweref93_Z);

         /** 
          *    From "SWEREF93" XYZ to "Rikets nät" XYZ,
          *    this is translation rotation and scaling.
          */
         static void transform_sweref93_XYZ_to_rr92_XYZ(
                                  float64 sweref93_X, float64 sweref93_Y, 
                                  float64 sweref93_Z, float64 &rr92_X, 
                                  float64 &rr92_Y, float64 &rr92_Z);

         /**
          *    "Rikets nät" From XYZ to LatLonAlt.
          */
         static void transform_rr92_XYZ_to_rt90_LLA(
                                  float64 rr92_X, float64 rr92_Y, 
                                  float64 rr92_Z, float64 &rr92_lat, 
                                  float64 &rr92_lon, float64 &rr92_h);

         /**
          *    "Rikets nät" From LatLongAlt to xyh (normal notation).
          */
         static void transform_rt90_LLA_to_rt90_XYH(
                                  float64 rr92_lat, float64 rr92_lon, 
                                  float64 rr92_h, float64 &gauss_x, 
                                  float64 &gauss_y, float64 &gauss_rh70_h);

         /** 
          *    From UTM (wgs84-based) planar coordinates 
          *    to SWEREF93 LatLonAlt.
          *    Currently the standard false easting 500000 m is subtracted
          *    from the utm_lon value.
          *    The utm zone number must be given, when transforming from 
          *    a utm coordinate system.
          */
         static void transform_UTM_to_sweref93_LLA( 
                        float64 utm_lat, float64 utm_lon, float64 utm_h,
                        uint32 utmzone,
                        float64 &sweref93_lat, float64 &sweref93_lon, 
                        float64 &sweref93_h);
         
         /**
          *    From SWEREF93 LatLonAlt to UTM (wgs84-based) 
          *    planar coordinates.
          *    Currently the standard false easting 500000 m is added to the
          *    resulting utm_lon.
          *    The utm zone number should preferrable be given when 
          *    transforming to a utm coordinate system. If not, the zone 
          *    is calculated from the longitude value of the in_coordinate.
          */
         static void transform_sweref93_LLA_to_UTM(
                        float64 sweref93_lat, float64 sweref93_lon, 
                        float64 sweref93_h, 
                        float64 &utm_lat, float64 &utm_lon, float64 &utm_h,
                        uint32 utmzone = MAX_UINT32);

      //@}

      /** 
       *    This function gives the geoid height, the difference in 
       *    altitude between Bessel's ellisoid and the geoid = the mean  
       *    sea level in Sweden.
       *    @param   x Latitude parameter. Given in the gaussian coordinate 
       *               system but without any standard additions.
       *    @param   y Longitude parameter. Given in the gaussian coordinate 
       *               system but without any standard additions.
       *    @return    The height of the geoid above Bessel's ellipsoid 
       *               in meters.
       */
      static float64 geoid_height_rn92(float64 gauss_x, float64 gauss_y);

      
      /**
        *   @name Constants
        *   Constants used in the transformations. These can be found 
        *   in report from "Lantmäteriverket" or in HMK-Ge:GPS.
        */
      //@{
         /** 
          *    Constant to wgs84 ellipsoid, semi-major axis.
          */
         static const float64 K_wgs84_a;

         /** 
          *    Constant to wgs84 ellipsoid, flattening.
          */
         static const float64 K_wgs84_f;

         /**
          *    Position of centre of Bessel's ellipsoid compared to wgs84.
          */
         static const float64 K_rr92_delta_X;

         /** 
          *    Position of centre of Bessel's ellipsoid compared to wgs84.
          */
         static const float64 K_rr92_delta_Y;

         /** 
          *    Position of centre of Bessel's ellipsoid compared to wgs84.
          */
         static const float64 K_rr92_delta_Z;

         /**  
          *    Parameter of rotation of XYZ from wgs84 to Bessel's
          *    ellipsoid (radians).
          */
         static const float64 K_rr92_omega_X;

         /**  
          *    Parameter of rotation of XYZ from wgs84 to Bessel's
          *    ellipsoid (radians).
          */
         static const float64 K_rr92_omega_Y;

         /**  
          *    Parameter of rotation of XYZ from wgs84 to Bessel's
          *    ellipsoid (radians).
          */
         static const float64 K_rr92_omega_Z;

         /**
          *    Parameter of scaling of XYZ from wgs84 to Bessel's
          *    ellipsoid.
          */
         static const float64 K_rr92_delta;

         /**
          *    Constant to Bessel's ellipsoid, semi-major axis.
          */
         static const float64 K_bessel_a;

         /** 
          *    Constant to Bessel's ellipsoid, flattening.
          */
         static const float64 K_bessel_f;

         /** 
          *    Central meridian for rt90 2,5 gon V. This is 17.56 gon 
          *    east of Greenwich, responding to 15 48' 29''.8 degrees.
          *    The constant is given in radians.
          */
         static const float64 K_rt90_gauss_lon0;

         /** 
          *    This number says that latitude(x) is calculated from 
          *    equator = no standard addition, no false northing.
          */
         static const float64 K_rt90_gauss_origo_x;

         /**
          *    Longitudinal values (y) in rt90 will be modified by a 
          *    standard addition of 1 500 000 metrs to avoid negative
          *    numbers west of the central meridian. This is the 
          *    false easting.
          */
         static const float64 K_rt90_gauss_origo_y;

         /**
           *   Coordinates (planar) in UTM are multiplied with a
           *   scalefactor of 0.9996 to avoid large distortions 
           *   in the areas that are located far away from the cetral
           *   meridian of the utm zone.
           */
         static const float64 utmScalefactor;
      //@}

      /**
        *   @name Derived constants.
        *   Some constants that are derived from the "true ones". These 
        *   constants are precalculated by using the constants above.
        */
      //@{
         /**
          *    Eccentricity squared of the wgs84 ellipsoid, e2=f*(2.0-f).
          */
         static float64 K_wgs84_e2;

         /** 
          *    e^4 square of the above.
          */
         static float64 K_wgs84_e4;

         /** 
          *    e^6
          */
         static float64 K_wgs84_e6;

         /** 
          *    $e^8
          */
         static float64 K_wgs84_e8;

         /** 
          *    Constant derived from the wgs84 ellipsoid, n=f/(2.0-f).
          */
         static float64 wgs84_n;

         /** 
          *    Constant derived from the wgs84 ellipsoid, 
          *    a^ = a/(1+n) * (1 + n^2/4 + n^4/64 + ... )
          *    $a^ =\frac{1}{1+n}(1+\frac{n^2}{4}+\frac{n^4}{64} + 
          *    \ldots$
          */
         static float64 wgs84_a_caret;

         /** 
          *    Constant used in transformation from planar to geodetic 
          *    coordinates in systems base on the wgs84 ellipsoid, utm.
          *    delta_1 = n/2 - 2n^2/3 + 37n^3/96 - n^4/360 + ...
          */
         static float64 wgs84_delta_1;

         /**
          *    Constant used in transformation from planar to geodetic 
          *    coordinates in systems base on the wgs84 ellipsoid, utm.
          *    delta_2 = n^2/48 + n^3/15 - 437n^4/1440 + ...
          */
         static float64 wgs84_delta_2;

         /**
          *    Constant used in transformation from planar to geodetic 
          *    coordinates in systems base on the wgs84 ellipsoid, utm.
          *    delta_3 = 17n^3/480 - 37n^4/840 + ...
          */
         static float64 wgs84_delta_3;

         /**
          *    Constant used in transformation from planar to geodetic 
          *    coordinates in systems base on the wgs84 ellipsoid, utm.
          *    delta_4 = 4397n^4/161280 + ...
          */
         static float64 wgs84_delta_4;

         /**
          *    Constant used in transformation from geodetic to planar
          *    coordinates in systems base on the wgs84 ellipsoid, utm.
          *    beta_1 = n/2 - 2n^2/3 + 5n^3/16 + 41n^4/180 + ...
          */
         static float64 wgs84_beta_1;

         /**
          *    Constant used in transformation from geodetic to planar
          *    coordinates in systems base on the wgs84 ellipsoid, utm.
          *    beta_2 = 13n^2/48 - 3n^3/5 + 557n^4/1440 + ...
          */
         static float64 wgs84_beta_2;

         /**
          *    Constant used in transformation from geodetic to planar
          *    coordinates in systems base on the wgs84 ellipsoid, utm.
          *    beta_3 = 61n^3/240 - 103n^4/140 + ...
          */
         static float64 wgs84_beta_3;

         /**
          *    Constant used in transformation from geodetic to planar
          *    coordinates in systems base on the wgs84 ellipsoid, utm.
          *    beta_4 = 49561n^4/161280 + ...
          */
         static float64 wgs84_beta_4;
         
         /** 
          *    Rotational matrix from SWEREF93 to rr92.
          */
         static float64 K_rr92_R[3][3];

         /** 
          *    Inverse of the above.
          */
         static float64 K_rr92_R_inv[3][3];

         /** 
          *    Eccentricity squared of Bessel's ellipsoid, e2=f*(2.0-f).
          */
         static float64 K_bessel_e2;

         /** 
          *    e^4 square of the above.
          */
         static float64 K_bessel_e4;

         /** 
          *    e^6
          */
         static float64 K_bessel_e6;

         /** 
          *    e^8
          */
         static float64 K_bessel_e8;

         /** 
          *    Constant derived from Bessel's ellipsoid, n=f/(2.0-f).
          */
         static float64 gauss_n;

         /** 
          *    Constant derived from Bessel's ellipsoid.
          *    $a^ =\frac{1}{1+n}(1+\frac{n^2}{4}+\frac{n^4}{64} + 
          *    \ldots$
          */
         static float64 gauss_a_caret;

         /** 
          *    Constant used in transformation from planar to geodetic 
          *    coordinates in systems base on Bessel's ellipsoid, rt90.
          *    $\delta_1=\frac{n}{2}-\frac{2n^2}{3}+\frac{37n^3}{96} -
          *    \frac{n^4}{360}+\ldots$.
          */
         static float64 gauss_delta_1;

         /**
          *    Constant used in transformation from planar to geodetic 
          *    coordinates in systems base on Bessel's ellipsoid, rt90.
          *    $\delta_2=\frac{n^2}{48}+\frac{n^3}{15}-
          *    \frac{437n^4}{1440}+ \ldots$.
          */
         static float64 gauss_delta_2;

         /**
          *    Constant used in transformation from planar to geodetic 
          *    coordinates in systems base on Bessel's ellipsoid, rt90.
          *    $\delta_3=\frac{17n^3}{480}-\frac{37n^4}{840}+\ldots$.
          */
         static float64 gauss_delta_3;

         /**
          *    Constant used in transformation from planar to geodetic 
          *    coordinates in systems base on Bessel's ellipsoid, rt90.
          *    $\delta_4=\frac{4397n^4}{161280}+\ldots$.
          */
         static float64 gauss_delta_4;

         /**
          *    Constant used in transformation from geodetic to planar
          *    coordinates in systems base on Bessel's ellipsoid, rt90.
          *    $\beta_1=\frac{n}{2}-\frac{2n^2}{3}+\frac{5n^3}{16} +
          *    \frac{41n^4}{180}\ldots$.
          */
         static float64 gauss_beta_1;

         /**
          *    Constant used in transformation from geodetic to planar
          *    coordinates in systems base on Bessel's ellipsoid, rt90.
          *    $\beta_2=\frac{13n^2}{48}-\frac{3n^3}{5} +
          *    \frac{557n^4}{1440}+\ldots$.
          */
         static float64 gauss_beta_2;

         /**
          *    Constant used in transformation from geodetic to planar
          *    coordinates in systems base on Bessel's ellipsoid, rt90.
          *    $\beta_3=\frac{61n^3}-\frac{103n^4}{140}+\ldots$.
          */
         static float64 gauss_beta_3;

         /**
          *    Constant used in transformation from geodetic to planar
          *    coordinates in systems base on Bessel's ellipsoid, rt90.
          *    $\beta_4=\frac{49561n^4}{161280}+\ldots$.
          */
         static float64 gauss_beta_4;
      //@}

      /**
        *   @name Testing methods.
        *   These methods test wether a coordinate is in SWEREF93 
        *   LatLonAlt or not.
        */
      //@{
         /**
          *    This function transforms the "Rikets nät" xyh to 
          *    SWEREF93 LatLonAlt.
          */
         static void test_inverse_transform(
               float64 gauss_x, float64 gauss_y, float64 gauss_rh70_h);

         /**
          *    This function transforms the SWEREF93 LatLonAlt to
          *    "Rikets nät" xyh and then calls the inverse function
          *    with xyh as inparameters.
          */
         static void test_transform(float64 sweref93_lat, 
                                    float64 sweref93_lon, 
                                    float64 sweref93_h);
      //@}

      /**
       *    @name Get type from string.
       */
      //@{
         /**
          *    Dictionary used to look-up coordinate types from strings.
          */
         static map<MC2String, format_t> m_coordinateTypes;

         /**
          *    Initiate m_coordinateTypes. Checks m_initiated before 
          *    adding any data to m_coordinateTypes.
          */
         static void init();

         /**
          *    Boolean to make sure that init() only initiate 
          *    m_coordinateTypes once.
          */
         static bool m_initiated;
      //@}

         /**
          *  Extra class to avoid exporting internal functions into
          *  the .h-file which is included indirectly by lots of files.
          */
         friend class CoordTransHelper;
         
};

#endif

