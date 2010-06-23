/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CoordinateTransformer.h"
#include "GfxUtility.h"
#include "GfxConstants.h"
#include "StringUtility.h"
#include "MC2Coordinate.h"
#include "ConverterSystem34.h"
#include "WGS84Coordinate.h"

#include "Math.h"
#include <math.h>

bool CoordinateTransformer::m_initiated = false;
map<MC2String, CoordinateTransformer::format_t>
CoordinateTransformer::m_coordinateTypes;


void
CoordinateTransformer::init()
{
   if (! m_initiated) {

      // These must be in upper case.
      m_coordinateTypes["WGS84_RAD"] = wgs84rad;

      m_coordinateTypes["WGS84_DEG"] = wgs84deg;

      m_coordinateTypes["SWEREF93_LLA"] = sweref93_LLA;

      m_coordinateTypes["SWEREF93_XYZ"] = sweref93_XYZ;

      m_coordinateTypes["RR92_XYZ"] = 
         m_coordinateTypes["RT92_XYZ"] = rr92_XYZ;

      m_coordinateTypes["RT90_LLA"]
         = m_coordinateTypes["RR92_LLA"] = rt90_LLA;

      m_coordinateTypes["RT90_2_5GONV_RH70_XYH"]
         = m_coordinateTypes["RT90_XYH"]
         = m_coordinateTypes["RT90_XYZ"] = rt90_2_5gonV_rh70_XYH;

      m_coordinateTypes["UTM"] = utm;
      
      m_coordinateTypes["UTM_ED50"] = utm_ed50;
      
      m_coordinateTypes["SYSTEM34"] = system34;

      m_coordinateTypes["MC2"]
         = m_coordinateTypes[""] = mc2;
   }
   m_initiated = true;
}


// Initialization of the derived constants.

//Eccentricity squared
float64 CoordinateTransformer::K_wgs84_e2 = K_wgs84_f*(2.0-K_wgs84_f);

float64 CoordinateTransformer::K_wgs84_e4 = SQUARE(K_wgs84_e2);

float64 CoordinateTransformer::K_wgs84_e6 = K_wgs84_e4*K_wgs84_e2;

float64 CoordinateTransformer::K_wgs84_e8 = SQUARE(K_wgs84_e4);

float64 CoordinateTransformer::wgs84_n = K_wgs84_f/(2-K_wgs84_f);

/*This is a series expansion */
float64 CoordinateTransformer::wgs84_a_caret = 
   K_wgs84_a/(1.0+wgs84_n)*(1.0+0.25*SQUARE(wgs84_n) +
   1.0/64.0*SQUARE(SQUARE(wgs84_n)));

float64 CoordinateTransformer::wgs84_delta_1 = 
   0.5 * wgs84_n - 2.0/3.0*SQUARE(wgs84_n) + 
   37.0/96.0 * wgs84_n*SQUARE(wgs84_n) - 
   1.0/360.0 * SQUARE(SQUARE(wgs84_n));

float64 CoordinateTransformer::wgs84_delta_2 = 
   1.0/48.0 * SQUARE(wgs84_n) + 1.0/15.0*wgs84_n*SQUARE(wgs84_n) - 
   437.0/1440.0*SQUARE(SQUARE(wgs84_n));

float64 CoordinateTransformer::wgs84_delta_3 = 
   17.0/480.0*wgs84_n*SQUARE(wgs84_n) - 37.0/840.0*SQUARE(SQUARE(wgs84_n));

float64 CoordinateTransformer::wgs84_delta_4 = 
   4397.0/161280.0*SQUARE(SQUARE(wgs84_n));

float64 CoordinateTransformer::wgs84_beta_1 = 
   0.5*wgs84_n - 2.0/3.0*SQUARE(wgs84_n) + 5.0/16.0*wgs84_n*SQUARE(wgs84_n) + 
   41.0/180.0*SQUARE(SQUARE(wgs84_n));

float64 CoordinateTransformer::wgs84_beta_2 = 
   13.0/48.0*SQUARE(wgs84_n) - 3.0/5.0*wgs84_n*SQUARE(wgs84_n) + 
   557.0/1440.0*SQUARE(SQUARE(wgs84_n));

float64 CoordinateTransformer::wgs84_beta_3 = 
   61.0/240.0*wgs84_n*SQUARE(wgs84_n) - 103.0/140.0*SQUARE(SQUARE(wgs84_n));

float64 CoordinateTransformer::wgs84_beta_4 = 
   49561.0/161280.0*SQUARE(SQUARE(wgs84_n));


// Rotational matrix from GRS80 (used in wgs84 mfl) in XYZ to RR92 in XYZ
float64 CoordinateTransformer::K_rr92_R[3][3] = 
   { 
      {  cos(K_rr92_omega_Y) * cos(K_rr92_omega_Z), 

         -cos(K_rr92_omega_Y) * sin(K_rr92_omega_Z),

         sin(K_rr92_omega_Y)
      },
      {
         cos(K_rr92_omega_X) * sin(K_rr92_omega_Z) +
         sin(K_rr92_omega_X)*sin(K_rr92_omega_Y) * cos(K_rr92_omega_Z),

         cos(K_rr92_omega_X)*cos(K_rr92_omega_Z) -
         sin(K_rr92_omega_X)*sin(K_rr92_omega_Y) * sin(K_rr92_omega_Z),

         -sin(K_rr92_omega_X)*cos(K_rr92_omega_Y)
      },
      {
         -cos(K_rr92_omega_X)*sin(K_rr92_omega_Y) * cos(K_rr92_omega_Z) +
         sin(K_rr92_omega_X)*sin(K_rr92_omega_Z),

         sin(K_rr92_omega_X)*cos(K_rr92_omega_Z) +
         cos(K_rr92_omega_X)*sin(K_rr92_omega_Y) * sin(K_rr92_omega_Z),

         cos(K_rr92_omega_X)*cos(K_rr92_omega_Y)
      }
   };

//Inverse of the above
float64 CoordinateTransformer::K_rr92_R_inv[3][3] = 
   {
      {
         cos(K_rr92_omega_Z)*cos(K_rr92_omega_Y),

         cos(K_rr92_omega_Z)*sin(K_rr92_omega_Y) * sin(K_rr92_omega_X) +
         sin(K_rr92_omega_Z)*cos(K_rr92_omega_X),

         sin(K_rr92_omega_Z)*sin(K_rr92_omega_X) -
         cos(K_rr92_omega_Z)*sin(K_rr92_omega_Y) * cos(K_rr92_omega_X)
      },
      {
         -sin(K_rr92_omega_Z)*cos(K_rr92_omega_Y),

         cos(K_rr92_omega_Z)*cos(K_rr92_omega_X) -
         sin(K_rr92_omega_Z)*sin(K_rr92_omega_Y) * sin(K_rr92_omega_X),

         sin(K_rr92_omega_Z)*sin(K_rr92_omega_Y) * cos(K_rr92_omega_X) +
         cos(K_rr92_omega_Z)*sin(K_rr92_omega_X)
      },
      {
         sin(K_rr92_omega_Y),

         -cos(K_rr92_omega_Y)*sin(K_rr92_omega_X),

         cos(K_rr92_omega_Y)*cos(K_rr92_omega_X)
      }
   };

float64 CoordinateTransformer::K_bessel_e2 = K_bessel_f*(2.0-K_bessel_f);

float64 CoordinateTransformer::K_bessel_e4 = SQUARE(K_bessel_e2);

float64 CoordinateTransformer::K_bessel_e6 = K_bessel_e4*K_bessel_e2;

float64 CoordinateTransformer::K_bessel_e8 = SQUARE(K_bessel_e4);

float64 CoordinateTransformer::gauss_n = K_bessel_f/(2-K_bessel_f);

/*This is a series expansion */
float64 CoordinateTransformer::gauss_a_caret = 
   K_bessel_a/(1.0+gauss_n)*(1.0+0.25*SQUARE(gauss_n) +
   1.0/64.0*SQUARE(SQUARE(gauss_n)));

float64 CoordinateTransformer::gauss_delta_1 = 
   0.5 * gauss_n - 2.0/3.0*SQUARE(gauss_n) + 
   37.0/96.0 * gauss_n*SQUARE(gauss_n) - 
   1.0/360.0 * SQUARE(SQUARE(gauss_n));

float64 CoordinateTransformer::gauss_delta_2 = 
   1.0/48.0 * SQUARE(gauss_n) + 1.0/15.0*gauss_n*SQUARE(gauss_n) - 
   437.0/1440.0*SQUARE(SQUARE(gauss_n));

float64 CoordinateTransformer::gauss_delta_3 = 
   17.0/480.0*gauss_n*SQUARE(gauss_n) - 37.0/840.0*SQUARE(SQUARE(gauss_n));

float64 CoordinateTransformer::gauss_delta_4 = 
   4397.0/161280.0*SQUARE(SQUARE(gauss_n));

float64 CoordinateTransformer::gauss_beta_1 = 
   0.5*gauss_n - 2.0/3.0*SQUARE(gauss_n) + 5.0/16.0*gauss_n*SQUARE(gauss_n) + 
   41.0/180.0*SQUARE(SQUARE(gauss_n));

float64 CoordinateTransformer::gauss_beta_2 = 
   13.0/48.0*SQUARE(gauss_n) - 3.0/5.0*gauss_n*SQUARE(gauss_n) + 
   557.0/1440.0*SQUARE(SQUARE(gauss_n));

float64 CoordinateTransformer::gauss_beta_3 = 
   61.0/240.0*gauss_n*SQUARE(gauss_n) - 103.0/140.0*SQUARE(SQUARE(gauss_n));

float64 CoordinateTransformer::gauss_beta_4 = 
   49561.0/161280.0*SQUARE(SQUARE(gauss_n));


float64
CoordinateTransformer::geoid_height_rn92(float64 x, float64 y)
{
   const uint32 x0 = 6881500;
   const uint32 y0 = 1535000;

   float64 p;
   float64 q;
   p = (x-x0) / 1000000.0;
   q = (y-y0) / 1000000.0;
   return ( -1.495 + 13.971*p - 35.508*q + 17.798*p*p + 1.161*p*q +
            5.807*q*q - 11.195*p*p*p + 38.700*p*p*q - 7.616*p*q*q +
            2.246*q*q*q);
};

void 
CoordinateTransformer::transform_sweref93_LLA_to_sweref93_XYZ(
  float64 sweref93_lat, float64 sweref93_lon, float64 sweref93_h,
  float64 &sweref93_X, float64 &sweref93_Y, float64 &sweref93_Z)

{
  float64 nPrim;
  mc2dbg2 << "IN: sweref93 LLA: " << sweref93_lat << " " 
          << sweref93_lon << " " << sweref93_h << endl;

   nPrim = 
      K_wgs84_a / sqrt((float64)1.0-K_wgs84_e2 * SQUARE(sin(sweref93_lat)));
   sweref93_X = (nPrim + sweref93_h) * cos(sweref93_lat) * cos(sweref93_lon);
   sweref93_Y = (nPrim + sweref93_h) * cos(sweref93_lat) * sin(sweref93_lon);
   sweref93_Z = (nPrim*((float64) 1.0-K_wgs84_e2) + sweref93_h) *
                sin(sweref93_lat);

  mc2dbg2 << "OUT: sweref93 XYZ: " << sweref93_X << " " << sweref93_Y
          << " " << sweref93_Z << endl;
}

class CoordTransHelper {
public:
   /**
    *   Transform between data using 7-parameter Helmert.
    */
   static void datum_transform_x_y_z( float64& x2,
                                      float64& y2,
                                      float64& z2,
                                      const float64& x1,
                                      const float64& y1,
                                      const float64& z1,
                                      const float64& dX,
                                      const float64& dY,
                                      const float64& dZ,
                                      const float64& rotX_seconds,
                                      const float64& rotY_seconds,
                                      const float64& rotZ_seconds,
                                      const float64& s_ppm,
                                      bool negateParams = false ) {
      if ( negateParams ) {
         datum_transform_x_y_z( x2, y2, z2,
                                x1, y1, z1,
                                -dX, -dY, -dZ,
                                -rotX_seconds,
                                -rotY_seconds,
                                -rotZ_seconds,
                                -s_ppm );
         return;
      }
      const float64 degFact = GfxConstants::degreeToRadianFactor;
      const float64 rotX = ( rotX_seconds / 60 ) * degFact;
      const float64 rotY = ( rotY_seconds / 60 ) * degFact;
      const float64 rotZ = ( rotZ_seconds / 60 ) * degFact;
      const float64 s    = s_ppm / (1e6);
      
      // Formula at http://www.swsurveys.co.uk/extras/parameters.htm
      
      x2 = (        x1 - rotZ * y1 + rotY * z1); /* * (1.0 + s) + dX; */
      y2 = ( rotZ * x1 +        y1 - rotX * z1); /* * (1.0 + s) + dY; */
      z2 = (-rotY * x1 + rotX * y1 +        z1); /* * (1.0 + s) + dZ; */
      
      // ( 1.0 + s ) * x2 etc. Might work if s isn't too small.
      x2 = x2 + s * x2;
      y2 = y2 + s * y2;
      z2 = z2 + s * z2;

      
      // Add delta
      x2 += dX;
      y2 += dY;
      z2 += dZ;
   }

   static void transform_ed50_LLA_to_ed50_XYZ( float64 ed50_lat,
                                               float64 ed50_lon,
                                               float64 ed50_h,
                                               float64& ed50_X,
                                               float64& ed50_Y,
                                               float64& ed50_Z)
      
   {
      const float64 K_ed50_a  = 6378388.0; // International 1924 Hayford
      const float64 K_ed50_f  = 1/297.0;
      const float64 K_ed50_e2 = K_ed50_f*(2.0-K_ed50_f);
      
      float64 nPrim = 
         K_ed50_a / sqrt((float64)1.0-K_ed50_e2 * SQUARE(sin(ed50_lat)));
      ed50_X = (nPrim + ed50_h) * cos(ed50_lat) * cos(ed50_lon);
      ed50_Y = (nPrim + ed50_h) * cos(ed50_lat) * sin(ed50_lon);
      ed50_Z = (nPrim*((float64) 1.0-K_ed50_e2) + ed50_h) *
         sin(ed50_lat);
   }

   static void transform_ed50_XYZ_to_sweref93_XYZ( float64& sweref93_X,
                                                   float64& sweref93_Y,
                                                   float64& sweref93_Z,
                                                   const float64& ed50_X,
                                                   const float64& ed50_Y,
                                                   const float64& ed50_Z )
   {
      // This is what works in Denmark
      const double dx = -87;
      const double dy = -96;
      const double dz = -120;

      // Old values that didn't work out
      
//       const double dx = -84.491;
//       const double dy = -100.559;
//       const double dz = -114.209;
//       const double dx = -89.5;
//       const double dy = -93.8;
//       const double dz = -123.1;

      // This is the real formula. Didn't work with the constants I found.
      
//       datum_transform_x_y_z( sweref93_X, sweref93_Y, sweref93_Z,
//                              ed50_X,     ed50_Y,     ed50_Z,
//                              //      dX          dY          dZ
//                                      dx,         dy,         dz,
//                              //     rotX        rotY       rotZ
//                              0,          0,     -0.156,
//                              +1.2, false );
      
      // Simpler formula which probably isn't entirely correct.
      sweref93_X = ed50_X + dx;
      sweref93_Y = ed50_Y + dy;
      sweref93_Z = ed50_Z + dz;
   }
   
   static void transform_ed50_LLA_to_sweref93_XYZ( float64& sweref93_X,
                                                   float64& sweref93_Y,
                                                   float64& sweref93_Z,
                                                   const float64& ed50_lat,
                                                   const float64& ed50_lon,
                                                   const float64& ed50_h ) {
      float64 ed50_X;
      float64 ed50_Y;
      float64 ed50_Z;

      // First convert it to x, y, z
      transform_ed50_LLA_to_ed50_XYZ( ed50_lat,
                                      ed50_lon,
                                      ed50_h,
                                      ed50_X,
                                      ed50_Y,
                                      ed50_Z );

      
      transform_ed50_XYZ_to_sweref93_XYZ( sweref93_X,
                                          sweref93_Y,
                                          sweref93_Z,
                                          ed50_X,
                                          ed50_Y,
                                          ed50_Z );
   }

   static void transform_ed50_LLA_to_sweref93_LLA( float64& sweref93_lat,
                                                   float64& sweref93_lon,
                                                   float64& sweref93_h,
                                                   const float64& ed50_lat,
                                                   const float64& ed50_lon,
                                                   const float64& ed50_h ) {
      float64 sweref93_X;
      float64 sweref93_Y;
      float64 sweref93_Z;

      // Transform to xyz
      transform_ed50_LLA_to_sweref93_XYZ( sweref93_X,
                                          sweref93_Y,
                                          sweref93_Z,
                                          ed50_lat,
                                          ed50_lon,
                                          ed50_h );

      // And then to lla!
      CoordinateTransformer::transform_sweref93_XYZ_to_sweref93_LLA(
         sweref93_X, sweref93_Y, sweref93_Z,
         sweref93_lat, sweref93_lon, sweref93_h );      
   }

   static void transform_ed50_LLA_to_sweref93_LLA( float64& sweref93_lat,
                                                   float64& sweref93_lon,
                                                   float64& sweref93_h ) {
      // Temporaries
      float64 ed50_lat = sweref93_lat;
      float64 ed50_lon = sweref93_lon;
      float64 ed50_h   = sweref93_h;
      // Trasnsform
      transform_ed50_LLA_to_sweref93_LLA( sweref93_lat,
                                          sweref93_lon,
                                          sweref93_h,
                                          ed50_lat,
                                          ed50_lon,
                                          ed50_h );
   }
      
};


void 
CoordinateTransformer::transform_rr92_XYZ_to_sweref93_XYZ(
      float64 rr92_X, float64 rr92_Y, float64 rr92_Z,
      float64 &sweref93_X, float64 &sweref93_Y, float64 &sweref93_Z)
{
   mc2dbg2 << "IN: RR92 XYZ = " << rr92_X << " " << rr92_Y 
           << " " << rr92_Z << endl;
   //Translation
   float64 tmp_X = rr92_X-K_rr92_delta_X;
   float64 tmp_Y = rr92_Y-K_rr92_delta_Y;
   float64 tmp_Z = rr92_Z-K_rr92_delta_Z;

   //Rotation and stretching
   sweref93_X =  (K_rr92_R_inv[0][0]*tmp_X+
                  K_rr92_R_inv[1][0]*tmp_Y+
                  K_rr92_R_inv[2][0]*tmp_Z) / (1.0+K_rr92_delta);

   sweref93_Y =  (K_rr92_R_inv[0][1]*tmp_X+
                  K_rr92_R_inv[1][1]*tmp_Y+
                  K_rr92_R_inv[2][1]*tmp_Z) / (1.0+K_rr92_delta);

   sweref93_Z =  (K_rr92_R_inv[0][2]*tmp_X+
                  K_rr92_R_inv[1][2]*tmp_Y+
                  K_rr92_R_inv[2][2]*tmp_Z) / (1.0+K_rr92_delta);
   
   mc2dbg2 << "OUT: sweref93 XYZ: " << sweref93_X << " " << sweref93_Y
           << " " << sweref93_Z << endl;
}


void 
CoordinateTransformer::transform_rt90_LLA_to_rr92_XYZ(
   float64 bessel_lat, float64 bessel_lon, float64 bessel_h,
   float64 &rr92_X, float64 &rr92_Y, float64 &rr92_Z)
{
   mc2dbg2 << "IN: RT90 LLA= " << bessel_lat << " " << bessel_lon
           << " " << bessel_h;
   
   float64 bessel_N_prim = 
      K_bessel_a / sqrt((float64)1-K_bessel_e2*SQUARE(sin(bessel_lat)));
   
   //Simple transformation from polar to cartesian (varying distance)
   rr92_X = (bessel_N_prim+bessel_h)*cos(bessel_lat)*cos(bessel_lon);
   rr92_Y = (bessel_N_prim+bessel_h)*cos(bessel_lat)*sin(bessel_lon);
   rr92_Z = (bessel_N_prim*(1.0-K_bessel_e2)+bessel_h)*sin(bessel_lat);
   
   mc2dbg2 << "OUT: RR92 XYZ = " << rr92_X << " " << rr92_Y << " "
           << rr92_Z << endl;
}

void 
CoordinateTransformer::transform_rt90_XYH_to_rt90_LLA(
   float64 gauss_x, float64 gauss_y, float64 gauss_rh70_h,
   float64 &bessel_lat, float64 &bessel_lon, float64 &bessel_h)
{
   mc2dbg2 << "IN: rt90 XYH = " << gauss_x << " "  << gauss_y << " "
           << gauss_rh70_h << endl;
   
   bessel_h = gauss_rh70_h+geoid_height_rn92(gauss_x, gauss_y);
   float64 gauss_xi = (gauss_x-K_rt90_gauss_origo_x)/gauss_a_caret;
   float64 gauss_eta = (gauss_y-K_rt90_gauss_origo_y)/gauss_a_caret;
   float64 gauss_xi_prim  =  gauss_xi -
                  gauss_delta_1*sin(2*gauss_xi)*cosh(2*gauss_eta) -
                  gauss_delta_2*sin(4*gauss_xi)*cosh(4*gauss_eta) -
                  gauss_delta_3*sin(6*gauss_xi)*cosh(6*gauss_eta) -
                  gauss_delta_4*sin(8*gauss_xi)*cosh(8*gauss_eta);
   float64 gauss_eta_prim  =  gauss_eta -
                  gauss_delta_1*cos(2*gauss_xi)*sinh(2*gauss_eta) -
                  gauss_delta_2*cos(4*gauss_xi)*sinh(4*gauss_eta) -
                  gauss_delta_3*cos(6*gauss_xi)*sinh(6*gauss_eta) -
                  gauss_delta_4*cos(8*gauss_xi)*sinh(8*gauss_eta);

   mc2dbg2 << "xi_prim = " << gauss_xi_prim << ", eta_prim = " 
           << gauss_eta_prim << endl;

   float64 gauss_iso_lat = asin(sin(gauss_xi_prim) / cosh(gauss_eta_prim));
   float64 gauss_delta_lon = atan(sinh(gauss_eta_prim) / cos(gauss_xi_prim));

   mc2dbg2 << "iso_lat = " << gauss_iso_lat << ", delta_lon = " 
           << gauss_delta_lon << endl;

   bessel_lat =   gauss_iso_lat+sin(gauss_iso_lat)*cos(gauss_iso_lat) *
                  ((K_bessel_e2+K_bessel_e4+K_bessel_e6+K_bessel_e8) +
                  (-(7*K_bessel_e4+17*K_bessel_e6+30*K_bessel_e8)/6) *
                  SQUARE(sin(gauss_iso_lat)) +
                  ((224*K_bessel_e6+889*K_bessel_e8) / 120) *
                  SQUARE(SQUARE(sin(gauss_iso_lat))) +
                  (-(4279*K_bessel_e8)/1260) * 
                  SQUARE(SQUARE(sin(gauss_iso_lat))) * 
                  SQUARE(sin(gauss_iso_lat)));
   bessel_lon =   gauss_delta_lon + K_rt90_gauss_lon0;
   
   mc2dbg2 << "OUT: rt90 LLA: " << bessel_lat << " " << bessel_lon
           << " " << bessel_h << endl;
}

void
CoordinateTransformer::transform_sweref93_XYZ_to_sweref93_LLA(
   float64 sweref93_X, float64 sweref93_Y, float64 sweref93_Z,
   float64 &sweref93_lat, float64 &sweref93_lon, float64 &sweref93_h)
{
   mc2dbg2 << "IN: sweref93 XYZ = " << sweref93_X << " " 
           << sweref93_Y << " " << sweref93_Z << endl;

   sweref93_lon = atan(sweref93_Y/sweref93_X);
   float64 sweref93_p = sqrt(SQUARE(sweref93_X)+SQUARE(sweref93_Y));
   float64 sweref93_theta = atan(sweref93_Z / 
                            (sweref93_p*sqrt(1-K_wgs84_e2)));
   sweref93_lat = atan(
      (sweref93_Z + (K_wgs84_a * K_wgs84_e2 / sqrt(1-K_wgs84_e2)) *
       sin(sweref93_theta)*sin(sweref93_theta)*sin(sweref93_theta)) /
       (sweref93_p-K_wgs84_a*K_wgs84_e2*
       cos(sweref93_theta)*cos(sweref93_theta)*cos(sweref93_theta)));
   float64 sweref93_N_prim = K_wgs84_a /
      sqrt(1 - K_wgs84_e2*SQUARE(sin(sweref93_lat)));
   sweref93_h = sweref93_p / cos(sweref93_lat)-sweref93_N_prim;
   
   mc2dbg2 << "OUT: sweref93 LLA = " << sweref93_lat << " " 
           << sweref93_lon << " " << sweref93_h << endl;
}

void
CoordinateTransformer::transform_sweref93_XYZ_to_rr92_XYZ(
   float64 sweref93_X, float64 sweref93_Y, float64 sweref93_Z,
   float64 &rr92_X, float64 &rr92_Y, float64 &rr92_Z)
{
   mc2dbg2 << "IN: sweref93 XYZ = " << sweref93_X << " " << sweref93_Y
           << " " << sweref93_Z << endl;

   rr92_X = K_rr92_delta_X + (1.0+K_rr92_delta)*
            (K_rr92_R[0][0]*sweref93_X+
            K_rr92_R[1][0]*sweref93_Y+
            K_rr92_R[2][0]*sweref93_Z);

   rr92_Y = K_rr92_delta_Y + (1.0+K_rr92_delta)*
            (K_rr92_R[0][1]*sweref93_X+
            K_rr92_R[1][1]*sweref93_Y+
            K_rr92_R[2][1]*sweref93_Z);

   rr92_Z = K_rr92_delta_Z + (1.0+K_rr92_delta)*
            (K_rr92_R[0][2]*sweref93_X+
            K_rr92_R[1][2]*sweref93_Y+
            K_rr92_R[2][2]*sweref93_Z);

   mc2dbg2 << "OUT: RR92 XYZ = " << rr92_X << " " << rr92_Y << " "
           << rr92_Z << endl;
}

void 
CoordinateTransformer::transform_rr92_XYZ_to_rt90_LLA(
   float64 rr92_X, float64 rr92_Y, float64 rr92_Z,
   float64 &bessel_lat, float64 &bessel_lon, float64 &bessel_h)

{
   mc2dbg2 << "IN: RR92 XYZ = " << rr92_X << " " << rr92_Y << " " 
           << rr92_Z << endl;

   float64 bessel_p = sqrt(SQUARE(rr92_X)+SQUARE(rr92_Y));
   float64 bessel_theta = atan(rr92_Z / 
                          (bessel_p*sqrt((float64)1.0-K_bessel_e2)));
   bessel_lat = atan(
      (rr92_Z + (K_bessel_a * K_bessel_e2 / sqrt((float64) 1-K_bessel_e2)) *
      sin(bessel_theta)*sin(bessel_theta)*sin(bessel_theta)) /
      (bessel_p-K_bessel_a*K_bessel_e2*
      cos(bessel_theta)*cos(bessel_theta)*cos(bessel_theta)));
   bessel_lon = atan(rr92_Y/rr92_X);
   float64 bessel_N_prim = K_bessel_a / 
               sqrt((float64) 1 - K_bessel_e2*SQUARE(sin(bessel_lat)));
   bessel_h = bessel_p/cos(bessel_lat)-bessel_N_prim;

   mc2dbg2 << "OUT: RT90 LLA: " << bessel_lat << " " << bessel_lon
           << " " << bessel_h;
}

void CoordinateTransformer::transform_rt90_LLA_to_rt90_XYH(
   float64 bessel_lat, float64 bessel_lon, float64 bessel_h,
   float64 &gauss_x, float64 &gauss_y, float64 &gauss_rh70_h)
{
   mc2dbg2 << "IN: RT90 LLA: " << bessel_lat << " " << bessel_lon
           << " " << bessel_h << endl;

   float64 gauss_iso_lat = bessel_lat-sin(bessel_lat)*cos(bessel_lat) *
      (K_bessel_e2 +
      (5.0 * K_bessel_e4 - K_bessel_e6)/6.0 * SQUARE(sin(bessel_lat)) +
      (104.0 * K_bessel_e6 - 45.0 * K_bessel_e8)/
      120.0 * SQUARE(SQUARE(sin(bessel_lat))) +
      (1237.0 * K_bessel_e8) / 
      1260.0 * SQUARE(SQUARE(sin(bessel_lat)))*SQUARE(sin(bessel_lat))
      );
   float64 gauss_delta_lon = bessel_lon-K_rt90_gauss_lon0;
   mc2dbg2 << "iso_lat = " << gauss_iso_lat << ", delta_lon = " 
           << gauss_delta_lon << endl;
   float64 gauss_xi_prim = atan(tan(gauss_iso_lat)/cos(gauss_delta_lon));
   float64 gauss_eta_prim = atanh(cos(gauss_iso_lat)*sin(gauss_delta_lon));

   mc2dbg2 << "xi_prim = " << gauss_xi_prim << ", eta_prim = " 
           << gauss_eta_prim << endl;

   gauss_x = K_rt90_gauss_origo_x +
      gauss_a_caret * ( gauss_xi_prim +
         gauss_beta_1 * sin(2.0*gauss_xi_prim)*cosh(2.0*gauss_eta_prim) +
         gauss_beta_2 * sin(4.0*gauss_xi_prim)*cosh(4.0*gauss_eta_prim) +
         gauss_beta_3 * sin(6.0*gauss_xi_prim)*cosh(6.0*gauss_eta_prim) +
         gauss_beta_4 * sin(8.0*gauss_xi_prim)*cosh(8.0*gauss_eta_prim));
   gauss_y = K_rt90_gauss_origo_y +
      gauss_a_caret*( gauss_eta_prim +
         gauss_beta_1*cos(2.0*gauss_xi_prim)*sinh(2.0*gauss_eta_prim) +
         gauss_beta_2*cos(4.0*gauss_xi_prim)*sinh(4.0*gauss_eta_prim) +
         gauss_beta_3*cos(6.0*gauss_xi_prim)*sinh(6.0*gauss_eta_prim) +
         gauss_beta_4*cos(8.0*gauss_xi_prim)*sinh(8.0*gauss_eta_prim));

   gauss_rh70_h = bessel_h-geoid_height_rn92(gauss_x,gauss_y);
   
   mc2dbg2 << "NOTE: geoid_height_rn92 = " 
           << geoid_height_rn92(gauss_x,gauss_y) << endl;
   mc2dbg2 << "OUT RT90/RH70 XYH: " << gauss_x << " " << gauss_y
           << " " << gauss_rh70_h;
}


void
CoordinateTransformer::transform_UTM_to_sweref93_LLA(
      float64 utmLat, float64 utmLon, float64 utmH, uint32 utmzone,
      float64 &sweref93Lat, float64 &sweref93Lon, float64 &sweref93H)
{
   // Standard false easting.
   utmLon -= 500000;
   mc2dbg2 << "IN: utmLat " << utmLat << " utmLon " << utmLon
           << " utmzone " << utmzone << endl;
   
   if ((utmzone < 1) && (utmzone > 60)) {
      mc2log << fatal << here << "Can't transform utm coordinates"
             << " without knowing the zone number." << endl;
      return;
   }

   float64 wgs84_xi = utmLat / (wgs84_a_caret * utmScalefactor);
   float64 wgs84_eta = utmLon /(wgs84_a_caret * utmScalefactor);
   float64 wgs84_xi_prim  =  
                  wgs84_xi -
                  wgs84_delta_1*sin(2*wgs84_xi)*cosh(2*wgs84_eta) -
                  wgs84_delta_2*sin(4*wgs84_xi)*cosh(4*wgs84_eta) -
                  wgs84_delta_3*sin(6*wgs84_xi)*cosh(6*wgs84_eta) -
                  wgs84_delta_4*sin(8*wgs84_xi)*cosh(8*wgs84_eta);
   float64 wgs84_eta_prim  =  
                  wgs84_eta -
                  wgs84_delta_1*cos(2*wgs84_xi)*sinh(2*wgs84_eta) -
                  wgs84_delta_2*cos(4*wgs84_xi)*sinh(4*wgs84_eta) -
                  wgs84_delta_3*cos(6*wgs84_xi)*sinh(6*wgs84_eta) -
                  wgs84_delta_4*cos(8*wgs84_xi)*sinh(8*wgs84_eta);
   
   float64 sweref93_iso_lat = 
                  asin(sin(wgs84_xi_prim) / cosh(wgs84_eta_prim));
   float64 sweref93_delta_lon = 
                  atan(sinh(wgs84_eta_prim) / cos(wgs84_xi_prim));
   
   sweref93Lat =  
      sweref93_iso_lat +
      sin(sweref93_iso_lat)*cos(sweref93_iso_lat) *
       ( (K_wgs84_e2+K_wgs84_e4+K_wgs84_e6+K_wgs84_e8) +
         (-(7*K_wgs84_e4+17*K_wgs84_e6+30*K_wgs84_e8)/6) *
          SQUARE(sin(sweref93_iso_lat)) +
         ((224*K_wgs84_e6+889*K_wgs84_e8) / 120) *
          SQUARE(SQUARE(sin(sweref93_iso_lat))) +
         (-(4279*K_wgs84_e8)/1260) * 
          SQUARE(SQUARE(sin(sweref93_iso_lat))) * 
          SQUARE(sin(sweref93_iso_lat)) );
   sweref93Lon = sweref93_delta_lon + 
                  getCentralMeridianForUTMZone(utmzone);
   sweref93H = utmH;

}

void
CoordinateTransformer::transform_UTM_to_ed50_LLA(
   float64 utmLat, float64 utmLon, float64 utmH, uint32 utmzone,
   float64 &sweref93Lat, float64 &sweref93Lon, float64 &sweref93H)
{

   // Standard false easting.
   utmLon -= 500000;
   mc2dbg2 << "IN: utmLat " << utmLat << " utmLon " << utmLon
           << " utmzone " << utmzone << endl;
   
   if ((utmzone < 1) && (utmzone > 60)) {
      mc2log << fatal << here << "Can't transform utm coordinates"
             << " without knowing the zone number." << endl;
      return;
   }

   const float64 K_ed50_a  = 6378388.0; // International 1924 Hayford
   const float64 K_ed50_f  = 1/297.0;
   const float64 K_ed50_e2 = K_ed50_f*(2.0-K_ed50_f);

   float64 ed50_n = K_ed50_f/(2-K_ed50_f);
   
float64 ed50_a_caret = 
   K_ed50_a/(1.0+ed50_n)*(1.0+0.25*SQUARE(ed50_n) +
   1.0/64.0*SQUARE(SQUARE(ed50_n)));

float64 ed50_delta_1 = 
   0.5 * ed50_n - 2.0/3.0*SQUARE(ed50_n) + 
   37.0/96.0 * ed50_n*SQUARE(ed50_n) - 
   1.0/360.0 * SQUARE(SQUARE(ed50_n));

float64 ed50_delta_2 = 
   1.0/48.0 * SQUARE(ed50_n) + 1.0/15.0*ed50_n*SQUARE(ed50_n) - 
   437.0/1440.0*SQUARE(SQUARE(ed50_n));

float64 ed50_delta_3 = 
   17.0/480.0*ed50_n*SQUARE(ed50_n) - 37.0/840.0*SQUARE(SQUARE(ed50_n));

float64 ed50_delta_4 = 
   4397.0/161280.0*SQUARE(SQUARE(ed50_n));

float64 K_ed50_e4 = SQUARE(K_ed50_e2);

float64 K_ed50_e6 = K_ed50_e4*K_ed50_e2;

float64 K_ed50_e8 = SQUARE(K_ed50_e4);

   
   float64 ed50_xi = utmLat / (ed50_a_caret * utmScalefactor);
   float64 ed50_eta = utmLon /(ed50_a_caret * utmScalefactor);
   float64 ed50_xi_prim  =  
                  ed50_xi -
                  ed50_delta_1*sin(2*ed50_xi)*cosh(2*ed50_eta) -
                  ed50_delta_2*sin(4*ed50_xi)*cosh(4*ed50_eta) -
                  ed50_delta_3*sin(6*ed50_xi)*cosh(6*ed50_eta) -
                  ed50_delta_4*sin(8*ed50_xi)*cosh(8*ed50_eta);
   float64 ed50_eta_prim  =  
                  ed50_eta -
                  ed50_delta_1*cos(2*ed50_xi)*sinh(2*ed50_eta) -
                  ed50_delta_2*cos(4*ed50_xi)*sinh(4*ed50_eta) -
                  ed50_delta_3*cos(6*ed50_xi)*sinh(6*ed50_eta) -
                  ed50_delta_4*cos(8*ed50_xi)*sinh(8*ed50_eta);
   
   float64 sweref93_iso_lat = 
                  asin(sin(ed50_xi_prim) / cosh(ed50_eta_prim));
   float64 sweref93_delta_lon = 
                  atan(sinh(ed50_eta_prim) / cos(ed50_xi_prim));
   
   sweref93Lat =  
      sweref93_iso_lat +
      sin(sweref93_iso_lat)*cos(sweref93_iso_lat) *
       ( (K_ed50_e2+K_ed50_e4+K_ed50_e6+K_ed50_e8) +
         (-(7*K_ed50_e4+17*K_ed50_e6+30*K_ed50_e8)/6) *
          SQUARE(sin(sweref93_iso_lat)) +
         ((224*K_ed50_e6+889*K_ed50_e8) / 120) *
          SQUARE(SQUARE(sin(sweref93_iso_lat))) +
         (-(4279*K_ed50_e8)/1260) * 
          SQUARE(SQUARE(sin(sweref93_iso_lat))) * 
          SQUARE(sin(sweref93_iso_lat)) );
   sweref93Lon = sweref93_delta_lon + 
                  getCentralMeridianForUTMZone(utmzone);
   sweref93H = utmH;

}


void
CoordinateTransformer::transform_sweref93_LLA_to_UTM(
      float64 sweref93Lat, float64 sweref93Lon, float64 sweref93H,
      float64 &utmLat, float64 &utmLon, float64 &utmH, uint32 utmzone)
{

   mc2dbg2 << "IN: sweref93Lat " << sweref93Lat << " sweref93Lon " 
           << sweref93Lon << " utmzone " << utmzone << endl;

   if ((utmzone > 60) || (utmzone < 1)) {
      // no zone given, calculate from the longitude value
      utmzone = getUTMZoneForLongitude(sweref93Lon);
   }
   
   utmLat = 0;
   utmLon = 0;
   utmH = 0;
   
   float64 sweref93_iso_lat = 
      sweref93Lat - 
      sin(sweref93Lat)*cos(sweref93Lat) *
      (K_wgs84_e2 +
       (5.0 * K_wgs84_e4 - K_wgs84_e6)/6.0 * SQUARE(sin(sweref93Lat)) +
       (104.0 * K_wgs84_e6 - 45.0 * K_wgs84_e8)/120.0 * 
         SQUARE(SQUARE(sin(sweref93Lat))) +
       (1237.0 * K_wgs84_e8) / 1260.0 * 
         SQUARE(SQUARE(sin(sweref93Lat)))*SQUARE(sin(sweref93Lat))
      );
   float64 sweref93_delta_lon = sweref93Lon - 
                                getCentralMeridianForUTMZone(utmzone);

   float64 wgs84_xi_prim = 
      atan(tan(sweref93_iso_lat)/cos(sweref93_delta_lon));
   float64 wgs84_eta_prim = 
      atanh(cos(sweref93_iso_lat)*sin(sweref93_delta_lon));
   
   utmLat =
      utmScalefactor * wgs84_a_caret * 
      ( wgs84_xi_prim +
        wgs84_beta_1 * sin(2.0*wgs84_xi_prim)*cosh(2.0*wgs84_eta_prim) +
        wgs84_beta_2 * sin(4.0*wgs84_xi_prim)*cosh(4.0*wgs84_eta_prim) +
        wgs84_beta_3 * sin(6.0*wgs84_xi_prim)*cosh(6.0*wgs84_eta_prim) +
        wgs84_beta_4 * sin(8.0*wgs84_xi_prim)*cosh(8.0*wgs84_eta_prim));
   utmLon =
      utmScalefactor * wgs84_a_caret *
      ( wgs84_eta_prim +
        wgs84_beta_1*cos(2.0*wgs84_xi_prim)*sinh(2.0*wgs84_eta_prim) +
        wgs84_beta_2*cos(4.0*wgs84_xi_prim)*sinh(4.0*wgs84_eta_prim) +
        wgs84_beta_3*cos(6.0*wgs84_xi_prim)*sinh(6.0*wgs84_eta_prim) +
        wgs84_beta_4*cos(8.0*wgs84_xi_prim)*sinh(8.0*wgs84_eta_prim));
   utmH = sweref93H;

   // Standard false easting.
   utmLon += 500000;
   
   mc2dbg2 << "OUT: utmLat " << utmLat << " utmLon " << utmLon << endl;
   
}

void 
CoordinateTransformer::transform( 
   format_t from_format, float64 in1, float64 in2, float64 in3,
   format_t to_format, float64 &out1, float64 &out2, float64 &out3,
   uint32 utmzone)
{
   mc2dbg2 << "transform IN: " << uint32(from_format) << " " 
           << in1 << " " << in2 << " " << in3 << endl;

   // Check if conversion is needed or not.
   if ( from_format == to_format ) {
      out1 = in1;
      out2 = in2;
      out3 = in3;
      return;
   }
   
   if ( from_format == system34 ) {
      // Can only be converted to MC2.
      double tmpout1;
      double tmpout2;
      double tmpout3;
      // X, Y into the converter.
      ConverterSystem34::convertSystem34ToMC2( in2, in1, in3,
                                               tmpout1, tmpout2, tmpout3 );
      transform( mc2, tmpout1, tmpout2, tmpout3,
                 to_format, out1, out2, out3 );
      return;
   } else if ( to_format == system34 ) {
      // Must convert to MC2 first
      double tmpout1;
      double tmpout2;
      double tmpout3;
      transform( from_format, in1, in2, in3,
                 mc2, tmpout1, tmpout2, tmpout3 );
      ConverterSystem34::convertMC2ToSystem34( tmpout1, tmpout2, tmpout3,
                                               out1, out2, out3 );
      return;
   }
   
   // transform given from_format to sweref93_XYH
   float64 sweref93_X = 0;
   float64 sweref93_Y = 0;
   float64 sweref93_Z = 0;
   float64 tmp1;
   float64 tmp2;
   float64 tmp3;
   
   switch (from_format) {
      case wgs84rad :
      case sweref93_LLA : {
         transform_sweref93_LLA_to_sweref93_XYZ(
             in1, in2, in3,
             sweref93_X, sweref93_Y, sweref93_Z);
         break;
      }
      case sweref93_XYZ : {
         sweref93_X = in1;
         sweref93_Y = in2;
         sweref93_Z = in3;
         break;
      }
      case rr92_XYZ : {
         transform_rr92_XYZ_to_sweref93_XYZ(
            in1, in2, in3,
            sweref93_X, sweref93_Y, sweref93_Z);
         break;
      }
      case rt90_LLA : {
         transform_rt90_LLA_to_rr92_XYZ(in1, in2, in3, tmp1, tmp2, tmp3);
         transform_rr92_XYZ_to_sweref93_XYZ(tmp1, tmp2, tmp3,
                  sweref93_X, sweref93_Y, sweref93_Z);
         break;
      }
      case rt90_2_5gonV_rh70_XYH : {
         transform_rt90_XYH_to_rt90_LLA(in1, in2, in3, tmp1, tmp2, tmp3);
         transform_rt90_LLA_to_rr92_XYZ(tmp1, tmp2, tmp3, tmp1, tmp2, tmp3);
         transform_rr92_XYZ_to_sweref93_XYZ(tmp1, tmp2, tmp3,
                  sweref93_X, sweref93_Y, sweref93_Z);
         break;
      }
      case mc2 : {
         float64 sweref93_lat = in1 * GfxConstants::invRadianFactor;
         float64 sweref93_lon = in2 * GfxConstants::invRadianFactor;
         float64 sweref93_h   = in3 * GfxConstants::MC2SCALE_TO_METER;
         transform_sweref93_LLA_to_sweref93_XYZ(
             sweref93_lat, sweref93_lon, sweref93_h,
             sweref93_X, sweref93_Y, sweref93_Z);
         break;
      }
      case wgs84deg : {
         in1 = in1 * GfxConstants::degreeToRadianFactor;
         in2 = in2 * GfxConstants::degreeToRadianFactor;
         transform_sweref93_LLA_to_sweref93_XYZ(
            in1, in2, in3, sweref93_X, sweref93_Y, sweref93_Z);
         break;
      }
      case utm : {
         MC2_ASSERT(utmzone != MAX_UINT32);
         transform_UTM_to_sweref93_LLA(
               in1, in2, in3, utmzone, tmp1, tmp2, tmp3);
         transform_sweref93_LLA_to_sweref93_XYZ(
               tmp1, tmp2, tmp3, sweref93_X, sweref93_Y, sweref93_Z);
         break;
      }

      case utm_ed50:
         MC2_ASSERT( utmzone != MAX_UINT32 );
         transform_UTM_to_ed50_LLA(
            in1, in2, in3, utmzone, tmp1, tmp2, tmp3);
         float64 ed50_x, ed50_y, ed50_z;
         
         CoordTransHelper::
            transform_ed50_LLA_to_ed50_XYZ(  tmp1, tmp2, tmp3,
                                             ed50_x, ed50_y, ed50_z );
         
         CoordTransHelper::transform_ed50_XYZ_to_sweref93_XYZ( sweref93_X,
                                                               sweref93_Y,
                                                               sweref93_Z,
                                                               ed50_x,
                                                               ed50_y,
                                                               ed50_z );
         
         break;
         
      default : {
         mc2log << warn << "[CoordTrans]: Unknown from_format" << endl;
      }
   }

   // transform from sweref_XYZ to given to_format

   switch (to_format) {
      case wgs84rad :
      case sweref93_LLA: {
         transform_sweref93_XYZ_to_sweref93_LLA(
            sweref93_X, sweref93_Y, sweref93_Z, out1, out2, out3);
         break;
      }
      case sweref93_XYZ : {
         out1 = sweref93_X;
         out2 = sweref93_Y;
         out3 = sweref93_Z;
         break;
      };
      case rr92_XYZ : {
         transform_sweref93_XYZ_to_rr92_XYZ(
            sweref93_X, sweref93_Y, sweref93_Z, out1, out2, out3);
         break;
      }
      case rt90_LLA : {
         transform_sweref93_XYZ_to_rr92_XYZ(
            sweref93_X, sweref93_Y, sweref93_Z, tmp1, tmp2, tmp3);
         transform_rr92_XYZ_to_rt90_LLA(
                  tmp1, tmp2, tmp3, out1, out2, out3);
         break;
      }
      case rt90_2_5gonV_rh70_XYH :{
         transform_sweref93_XYZ_to_rr92_XYZ(
            sweref93_X, sweref93_Y, sweref93_Z, tmp1, tmp2, tmp3);
         transform_rr92_XYZ_to_rt90_LLA(
            tmp1, tmp2, tmp3, tmp1, tmp2, tmp3);
         transform_rt90_LLA_to_rt90_XYH(
                  tmp1, tmp2, tmp3, out1, out2, out3);
         break;
      }
      case mc2 : {
         transform_sweref93_XYZ_to_sweref93_LLA(
            sweref93_X, sweref93_Y, sweref93_Z, out1, out2, out3);
         out1 = out1 * GfxConstants::radianFactor;
         out2 = out2 * GfxConstants::radianFactor;
         out3 = out3 * GfxConstants::METER_TO_MC2SCALE;
         break;         
      }
      case wgs84deg : {
         transform_sweref93_XYZ_to_sweref93_LLA(
            sweref93_X, sweref93_Y, sweref93_Z, out1, out2, out3);
         out1 = out1 * GfxConstants::radianTodegreeFactor;
         out2 = out2 * GfxConstants::radianTodegreeFactor;
         break;
      }
      case utm : {
         transform_sweref93_XYZ_to_sweref93_LLA(
            sweref93_X, sweref93_Y, sweref93_Z, tmp1, tmp2, tmp3);
         transform_sweref93_LLA_to_UTM(
               tmp1, tmp2, tmp3, out1, out2, out3, utmzone);
         break;
      }
      default:
         mc2log << warn << "[CoordTrans]: Unknown to_format" << endl;
   }
   
   mc2dbg2 << "transform OUT: " << uint32(to_format) << " "
           << out1 << " " << out2 << " " << out3 << endl;
}

void 
CoordinateTransformer::transformToMC2( 
   format_t informat, float64 in1, float64 in2, float64 in3,
   int32 &mc2_lat, int32 &mc2_lon, uint32 utmzone)
{
   mc2dbg2 << "transform IN: " << uint32(informat) << " " 
           << in1 << " " << in2 << " " << in3 << endl;

   
   if ( informat == system34 ) {
      // Format x, y in convertMC2ToSystem34
      MC2Coordinate coord =
         ConverterSystem34::convertSystem34ToMC2( in2, in1 );
      mc2_lat = coord.lat;
      mc2_lon = coord.lon;
      return;
   }
   
  float64 sweref93_lat = 0;
  float64 sweref93_lon = 0;
  float64 sweref93_h = 0;
  float64 tmp1;
  float64 tmp2;
  float64 tmp3;
  int32   mc2_h;

   //Transforms the input format to SWEREF LLA (radians)
   switch (informat) {
      case wgs84deg : {
         sweref93_lat = in1 * GfxConstants::degreeToRadianFactor;
         sweref93_lon = in2 * GfxConstants::degreeToRadianFactor;
         sweref93_h = in3;
         break;
      }
      case wgs84rad :
      case sweref93_LLA : {
         sweref93_lat = in1;
         sweref93_lon = in2;
         sweref93_h = in3;
         break;
      }
      case sweref93_XYZ : {
         transform_sweref93_XYZ_to_sweref93_LLA(
            in1, in2, in3, sweref93_lat, sweref93_lon, sweref93_h); 
         break;
      }
      case rr92_XYZ : {
         transform_rr92_XYZ_to_sweref93_XYZ(
            in1, in2, in3, tmp1, tmp2, tmp3);
         transform_sweref93_XYZ_to_sweref93_LLA(
            tmp1, tmp2, tmp3, sweref93_lat, sweref93_lon, sweref93_h); 
         break;
      }
      case rt90_LLA : {
         transform_rt90_LLA_to_rr92_XYZ(
            in1, in2, in3, tmp1, tmp2, tmp3);
         transform_rr92_XYZ_to_sweref93_XYZ(
            tmp1, tmp2, tmp3, tmp1, tmp2, tmp3);
         transform_sweref93_XYZ_to_sweref93_LLA(
            tmp1, tmp2, tmp3, sweref93_lat, sweref93_lon, sweref93_h); 
         break;
      }
      case rt90_2_5gonV_rh70_XYH : {
         transform_rt90_XYH_to_rt90_LLA(
            in1, in2, in3, tmp1, tmp2, tmp3);
         transform_rt90_LLA_to_rr92_XYZ(
            tmp1, tmp2, tmp3, tmp1, tmp2, tmp3);
         transform_rr92_XYZ_to_sweref93_XYZ(
            tmp1, tmp2, tmp3, tmp1, tmp2, tmp3);
         transform_sweref93_XYZ_to_sweref93_LLA(
            tmp1, tmp2, tmp3, sweref93_lat, sweref93_lon, sweref93_h); 
         break;
      }
      case utm : {
         MC2_ASSERT(utmzone != MAX_UINT32);
         transform_UTM_to_sweref93_LLA(
            in1, in2, in3, utmzone, sweref93_lat, sweref93_lon, sweref93_h);
         break;
      }
      case utm_ed50: {
         transform( utm_ed50, in1, in2, in3, mc2, tmp1, tmp2, tmp3, utmzone );
         transformToMC2( mc2, tmp1, tmp2, tmp3, mc2_lat, mc2_lon );
         return;
      }
      case mc2 : {
         mc2_lat = int32( ( (in1 < 0) ? -1: 1) * 0.5 + in1);
         mc2_lon = int32( ( (in2 < 0) ? -1: 1) * 0.5 + in2);
         return;
      }
      default : {
         mc2log << warn << "[CoordinateTransformer]: Unknown informat" << endl;
      }
   }
   float64 addLat = ( (sweref93_lat < 0) ? -1: 1) * 0.5;
   float64 addLon = ( (sweref93_lon < 0) ? -1: 1) * 0.5;

   // Converts the latitude to mc2_lat
   mc2_lat = int32 (sweref93_lat * GfxConstants::radianFactor + addLat);
   
   // Converts the longitude to mc2_lon
   mc2_lon = int32 (sweref93_lon * GfxConstants::radianFactor + addLon);   

   // Converts the altitude to MC2 scale
   mc2_h = int32 (sweref93_h * GfxConstants::METER_TO_MC2SCALE);


   mc2dbg2 << "transform OUT: Latitude and longitude in MC2 " 
           << mc2_lat << " " << mc2_lon << endl;
}

MC2Coordinate
CoordinateTransformer::transformToMC2( format_t informat, float64 in1, 
                                       float64 in2, float64 in3,
                                       uint32 utmzone )
{
   MC2Coordinate retVal;
   transformToMC2( informat, in1, in2, in3, retVal.lat, retVal.lon,
                   utmzone );
   return retVal;
}
   
void
CoordinateTransformer::transformFromMC2(
      int32 mc2_lat, int32 mc2_lon, 
      format_t outformat, float64 &out1, float64 &out2, float64 &out3,
      uint32 utmzone)
{
   mc2dbg2 << "transform IN: latitude and longitude in MC2" 
           << " "<< mc2_lat << " " << mc2_lon << endl;

  float64 sweref93_lat;
  float64 sweref93_lon;
  float64 sweref93_h;
  float64 tmp1;
  float64 tmp2;
  float64 tmp3;
  int32 mc2_h;

  // Height does not exist in mc2 and is therefore set to 0.
  mc2_h = 0;

  // Conversion from mc2 to sweref93_LLA (radians)
  sweref93_lat = float64 (mc2_lat * GfxConstants::invRadianFactor);
  sweref93_lon = float64 (mc2_lon * GfxConstants::invRadianFactor);
  sweref93_h   = float64 (mc2_h * GfxConstants::MC2SCALE_TO_METER);

   switch (outformat) {
      case wgs84deg : {
         out1 = sweref93_lat * GfxConstants::radianTodegreeFactor;
         out2 = sweref93_lon * GfxConstants::radianTodegreeFactor;
         out3 = sweref93_h;
         break;
      }
      case wgs84rad :
      case sweref93_LLA: {
         out1 = sweref93_lat;
         out2 = sweref93_lon;
         out3 = sweref93_h;
         break;
      }
      case sweref93_XYZ : {
	      transform_sweref93_LLA_to_sweref93_XYZ(
	         sweref93_lat, sweref93_lon, sweref93_h, out1, out2, out3); 
         break;
      };
      case rr92_XYZ : {
	      transform_sweref93_LLA_to_sweref93_XYZ(
	         sweref93_lat, sweref93_lon, sweref93_h, tmp1, tmp2, tmp3);
         transform_sweref93_XYZ_to_rr92_XYZ(
            tmp1, tmp2, tmp3, out1, out2, out3);
         break;
      }
      case rt90_LLA : {
	 transform_sweref93_LLA_to_sweref93_XYZ(
	   sweref93_lat, sweref93_lon, sweref93_h, tmp1, tmp2, tmp3);
         transform_sweref93_XYZ_to_rr92_XYZ(
           tmp1, tmp2, tmp3, tmp1, tmp2, tmp3);
         transform_rr92_XYZ_to_rt90_LLA(
           tmp1, tmp2, tmp3, out1, out2, out3);
         break;
      }
      case rt90_2_5gonV_rh70_XYH :{
	      transform_sweref93_LLA_to_sweref93_XYZ(
	         sweref93_lat, sweref93_lon, sweref93_h, tmp1, tmp2, tmp3);
         transform_sweref93_XYZ_to_rr92_XYZ(
            tmp1, tmp2, tmp3, tmp1, tmp2, tmp3);
         transform_rr92_XYZ_to_rt90_LLA(
            tmp1, tmp2, tmp3, tmp1, tmp2, tmp3);
         transform_rt90_LLA_to_rt90_XYH(
            tmp1, tmp2, tmp3, out1, out2, out3);
         break;
      }
      case utm : {
         transform_sweref93_LLA_to_UTM(
            sweref93_lat, sweref93_lon, sweref93_h,
            out1, out2, out3, utmzone);
      }
      case mc2 : {
         out1 = float64(mc2_lat);
         out2 = float64(mc2_lon);
         out3 = 0;
         break;
      }
      default:
         mc2log << warn << "Unknown outformat" << endl;
   }
   
   mc2dbg2 << "transform OUT: " << uint32(outformat) << out1 << " " 
           << out2 << " " << out3 << endl;
}


void 
CoordinateTransformer::test_inverse_transform(float64 gauss_x,
                               float64 gauss_y, float64 gauss_rh70_h)
{ 
   mc2dbg << "BEGIN INVERSE TRANSFORM" << endl;
   float64 bessel_h = gauss_rh70_h+geoid_height_rn92(gauss_x,gauss_y);
   float64 gauss_xi = (gauss_x-K_rt90_gauss_origo_x)/gauss_a_caret;
   float64 gauss_eta = (gauss_y-K_rt90_gauss_origo_y)/gauss_a_caret;
   float64 gauss_xi_prim  =  gauss_xi -
                     gauss_delta_1*sin(2*gauss_xi)*cosh(2*gauss_eta) -
                     gauss_delta_2*sin(4*gauss_xi)*cosh(4*gauss_eta) -
                     gauss_delta_3*sin(6*gauss_xi)*cosh(6*gauss_eta) -
                     gauss_delta_4*sin(8*gauss_xi)*cosh(8*gauss_eta);
   float64 gauss_eta_prim  =  gauss_eta -
                     gauss_delta_1*cos(2*gauss_xi)*sinh(2*gauss_eta) -
                     gauss_delta_2*cos(4*gauss_xi)*sinh(4*gauss_eta) -
                     gauss_delta_3*cos(6*gauss_xi)*sinh(6*gauss_eta) -
                     gauss_delta_4*cos(8*gauss_xi)*sinh(8*gauss_eta);

   mc2dbg << "xi_prim=" << gauss_xi_prim << " eta_prim = " 
          << gauss_eta_prim << endl;

   float64 gauss_iso_lat = asin(sin(gauss_xi_prim)/cosh(gauss_eta_prim));
   float64 gauss_delta_lon = atan(sinh(gauss_eta_prim)/cos(gauss_xi_prim));

   mc2dbg2 << "iso_lat = " << gauss_iso_lat << " delta_lon = "
           << gauss_delta_lon << endl;

   float64 bessel_lat = gauss_iso_lat+sin(gauss_iso_lat)*cos(gauss_iso_lat)*
                 ((K_bessel_e2+K_bessel_e4+K_bessel_e6+K_bessel_e8) +
                  (-(7.0*K_bessel_e4+17.0*K_bessel_e6+30.0*K_bessel_e8)/6.0) *
                  SQUARE(sin(gauss_iso_lat)) +
                  ((224.0*K_bessel_e6+889.0*K_bessel_e8)/120.0) *
                  SQUARE(SQUARE(sin(gauss_iso_lat)))+
                  (-(4279.0*K_bessel_e8)/1260.0) *
                  SQUARE(SQUARE(sin(gauss_iso_lat)))*
                  SQUARE(sin(gauss_iso_lat)));
   float64 bessel_lon = gauss_delta_lon + K_rt90_gauss_lon0;
   
   mc2dbg << "Bessel LLA: " << bessel_lat << " " << bessel_lon
          << " " << bessel_h << endl;
   float64 bessel_N_prim = K_bessel_a/
                           sqrt(1.0-K_bessel_e2*SQUARE(sin(bessel_lat)));

   float64 rr92_X = (bessel_N_prim+bessel_h)*cos(bessel_lat)*cos(bessel_lon);
   float64 rr92_Y = (bessel_N_prim+bessel_h)*cos(bessel_lat)*sin(bessel_lon);
   float64 rr92_Z = (bessel_N_prim*(1-K_bessel_e2)+bessel_h)*sin(bessel_lat);
   mc2dbg << "RR92 XYZ = " << rr92_X << " " << rr92_Y << " " 
          << rr92_Z << endl;

   float64 tmp_X = rr92_X-K_rr92_delta_X;
   float64 tmp_Y = rr92_Y-K_rr92_delta_Y;
   float64 tmp_Z = rr92_Z-K_rr92_delta_Z;

   float64 sweref93_X = (K_rr92_R_inv[0][0]*tmp_X + K_rr92_R_inv[1][0]*tmp_Y +
                        K_rr92_R_inv[2][0]*tmp_Z)/(1.0+K_rr92_delta);

   float64 sweref93_Y = (K_rr92_R_inv[0][1]*tmp_X + K_rr92_R_inv[1][1]*tmp_Y +
                        K_rr92_R_inv[2][1]*tmp_Z)/(1.0+K_rr92_delta);

   float64 sweref93_Z = (K_rr92_R_inv[0][2]*tmp_X + K_rr92_R_inv[1][2]*tmp_Y+
                        K_rr92_R_inv[2][2]*tmp_Z)/(1.0+K_rr92_delta);
  
   mc2dbg << "sweref93 XYZ: " << sweref93_X << " " << sweref93_Y
          << " " << sweref93_Z << endl;

   float64 sweref93_lon = atan(sweref93_Y/sweref93_X);
   float64 sweref93_p = sqrt(SQUARE(sweref93_X)+SQUARE(sweref93_Y));
   float64 sweref93_theta = atan(sweref93_Z / 
                            (sweref93_p*sqrt(1.0-K_wgs84_e2)));
   float64 sweref93_lat = atan(
         (sweref93_Z + (K_wgs84_a * K_wgs84_e2 / sqrt(1.0-K_wgs84_e2)) *
         sin(sweref93_theta)*sin(sweref93_theta)*sin(sweref93_theta)) /
         (sweref93_p-K_wgs84_a*K_wgs84_e2*
         cos(sweref93_theta)*cos(sweref93_theta)*cos(sweref93_theta)));

   float64 sweref93_N_prim = K_wgs84_a /
                             sqrt(1.0 - K_wgs84_e2*SQUARE(sin(sweref93_lat)));
   float64 sweref93_h = sweref93_p/cos(sweref93_lat)-sweref93_N_prim;
   
   mc2dbg << "sweref93 LLA: " << sweref93_lat << " " 
          << sweref93_lon << " " << sweref93_h << endl;
}

void
CoordinateTransformer::test_transform(float64 sweref93_lat, 
                                      float64 sweref93_lon, 
                                      float64 sweref93_h)
{
   mc2dbg << "sweref93 LLA: " << sweref93_lat << " " << sweref93_lon
          << " " << sweref93_h << endl;

   float64 N_prim = K_wgs84_a/sqrt(1.0-K_wgs84_e2*SQUARE(sin(sweref93_lat)));
   float64 sweref93_X = (N_prim + sweref93_h) *
                        cos(sweref93_lat)*cos(sweref93_lon);
   float64 sweref93_Y = (N_prim + sweref93_h) *
                        cos(sweref93_lat)*sin(sweref93_lon);
   float64 sweref93_Z = (N_prim*(1.0-K_wgs84_e2) + 
                        sweref93_h)*sin(sweref93_lat);

   mc2dbg << "sweref93 XYZ: " << sweref93_X << " " << sweref93_Y
          << " " << sweref93_Z << endl;

   float64 rr92_X =  K_rr92_delta_X + (1.0+K_rr92_delta)*
                     (K_rr92_R[0][0]*sweref93_X+
                     K_rr92_R[1][0]*sweref93_Y+
                     K_rr92_R[2][0]*sweref93_Z);

   float64 rr92_Y =  K_rr92_delta_Y + (1.0+K_rr92_delta)*
                     (K_rr92_R[0][1]*sweref93_X+
                     K_rr92_R[1][1]*sweref93_Y+
                     K_rr92_R[2][1]*sweref93_Z);

   float64 rr92_Z =  K_rr92_delta_Z + (1.0+K_rr92_delta)*
                     (K_rr92_R[0][2]*sweref93_X+
                     K_rr92_R[1][2]*sweref93_Y+
                     K_rr92_R[2][2]*sweref93_Z);

   mc2dbg << "RR92 XYZ = " << rr92_X << " " << rr92_Y << " " 
          << rr92_Z << endl;

   float64 bessel_p = sqrt(SQUARE(rr92_X)+SQUARE(rr92_Y));
   float64 bessel_theta = atan(rr92_Z / (bessel_p*sqrt(1.0-K_bessel_e2)));
   float64 bessel_lat = atan(
         (rr92_Z + (K_bessel_a * K_bessel_e2 / sqrt(1.0-K_bessel_e2)) *
         sin(bessel_theta)*sin(bessel_theta)*sin(bessel_theta)) /
         (bessel_p-K_bessel_a*K_bessel_e2*
         cos(bessel_theta)*cos(bessel_theta)*cos(bessel_theta)));
   float64 bessel_lon = atan(rr92_Y/rr92_X);
   float64 bessel_N_prim = K_bessel_a /
                           sqrt(1.0 - K_bessel_e2*SQUARE(sin(bessel_lat)));
   float64 bessel_h = bessel_p / cos(bessel_lat)-bessel_N_prim;

   mc2dbg << "Bessel LLA: " << bessel_lat << " " << bessel_lon
          << " " << bessel_h << endl;

   float64 gauss_iso_lat = bessel_lat-sin(bessel_lat)*cos(bessel_lat) *
         (K_bessel_e2 +
         (5 * K_bessel_e4 - K_bessel_e6)/6 * SQUARE(sin(bessel_lat)) +
         (104 * K_bessel_e6 - 45 * K_bessel_e8)/120 * 
         SQUARE(SQUARE(sin(bessel_lat))) +
         (1237 * K_bessel_e8)/1260 * SQUARE(SQUARE(sin(bessel_lat))) *
         SQUARE(sin(bessel_lat)));
   float64 gauss_delta_lon = bessel_lon-K_rt90_gauss_lon0;

   mc2dbg << "iso_lat = " << gauss_iso_lat << " delta_lon = "
          << gauss_delta_lon << endl;
   float64 gauss_xi_prim = atan(tan(gauss_iso_lat)/cos(gauss_delta_lon));
   float64 gauss_eta_prim = atanh(cos(gauss_iso_lat)*sin(gauss_delta_lon)); 

   mc2dbg << "xi_prim = " << gauss_xi_prim << " eta_prim = "
          << gauss_eta_prim << endl;


   float64 gauss_x = K_rt90_gauss_origo_x + gauss_a_caret*( gauss_xi_prim +
         gauss_beta_1*sin(2.0*gauss_xi_prim)*cosh(2.0*gauss_eta_prim) +
         gauss_beta_2*sin(4.0*gauss_xi_prim)*cosh(4.0*gauss_eta_prim) +
         gauss_beta_3*sin(6.0*gauss_xi_prim)*cosh(6.0*gauss_eta_prim) +
         gauss_beta_4*sin(8.0*gauss_xi_prim)*cosh(8.0*gauss_eta_prim));
   float64 gauss_y = K_rt90_gauss_origo_y + gauss_a_caret*( gauss_eta_prim +
         gauss_beta_1*cos(2.0*gauss_xi_prim)*sinh(2.0*gauss_eta_prim) +
         gauss_beta_2*cos(4.0*gauss_xi_prim)*sinh(4.0*gauss_eta_prim) +
         gauss_beta_3*cos(6.0*gauss_xi_prim)*sinh(6.0*gauss_eta_prim) +
         gauss_beta_4*cos(8.0*gauss_xi_prim)*sinh(8.0*gauss_eta_prim));
   float64 gauss_rh70_h = bessel_h-geoid_height_rn92(gauss_x,gauss_y);

   mc2dbg << "geoid_height_rn92 = " 
          << geoid_height_rn92(gauss_x,gauss_y) << endl;
   mc2dbg << "gauss XYA: " << gauss_x << " " << gauss_y << " " 
          << gauss_rh70_h << endl;

   test_inverse_transform(gauss_x,gauss_y,gauss_rh70_h); 
}

CoordinateTransformer::format_t
CoordinateTransformer::getCoordinateType( const MC2String& ins)
{
   // Initiate the m_coordinateTypes that is used for the lookup.
   init();

   // Convert string to uppercase in order to find it.
   MC2String s = StringUtility::copyUpper( ins );
   
   map<MC2String, format_t>::const_iterator i = m_coordinateTypes.find(s);
   if (i != m_coordinateTypes.end()) {
      return i->second;
   }

   mc2log << warn << "CoordinateTransformer::getCoordinateType, "
         << "unsupported coordinate type (" << s << ")" << endl;

   return nbrCoordinateTypes;
}

uint32
CoordinateTransformer::getUTMZoneForLongitude(
      float64 lon, bool radians )
{
   // value to return
   uint32 retZone = 0;

   if (radians) {
      lon = lon * GfxConstants::radianTodegreeFactor;
   }

   MC2_ASSERT((lon >= -180) && (lon <= 180));

   retZone = uint32((180 + lon)/6) + 1;
   if (retZone == 61){
      retZone = 60;
   }

   return retZone;
}

float64
CoordinateTransformer::getCentralMeridianForUTMZone(
      uint32 utmzone, bool radians)
{
   //value to return
   float64 retLon = 0;

   MC2_ASSERT((utmzone > 0) && (utmzone < 61));
   
   retLon = utmzone*6.0 - 183;
   
   if (radians) {
      retLon = retLon * GfxConstants::degreeToRadianFactor;
   }

   mc2dbg8 << "(" << utmzone << ":" << retLon << ")";

   return retLon;
}

WGS84Coordinate CoordinateTransformer::
transformToWGS84( const MC2Coordinate& coordIn ) {
   WGS84Coordinate coordOut;
   WGS84Coordinate::Type z;
   transformFromMC2( coordIn.lat, coordIn.lon,
                     wgs84deg, coordOut.lat, coordOut.lon, z );
   return coordOut;
}
 
MC2Coordinate CoordinateTransformer::
transformToMC2( const WGS84Coordinate& coordIn ) {
   MC2Coordinate coordOut;
   transformToMC2( wgs84deg, coordIn.lat, coordIn.lon,
                   0, // z, not used
                   coordOut.lat, coordOut.lon );
   return coordOut;
}

const float64 CoordinateTransformer::K_wgs84_a = 6378137;
const float64 CoordinateTransformer::K_wgs84_f = 1/298.257222101;
const float64 CoordinateTransformer::K_rr92_delta_X = -419.375;
const float64 CoordinateTransformer::K_rr92_delta_Y = -99.352;
const float64 CoordinateTransformer::K_rr92_delta_Z = -591.349;
const float64 CoordinateTransformer::K_rr92_omega_X = 4.12313673609e-6;
const float64 CoordinateTransformer::K_rr92_omega_Y = 8.81025237929e-6;
const float64 CoordinateTransformer::K_rr92_omega_Z = -38.1172394024e-6;
const float64 CoordinateTransformer::K_rr92_delta  = 0.99496e-6;
const float64 CoordinateTransformer::K_bessel_a = 6377397.155;
const float64 CoordinateTransformer::K_bessel_f = 1/299.1528128;
const float64 CoordinateTransformer::K_rt90_gauss_lon0 = 0.27590649629;
const float64 CoordinateTransformer::K_rt90_gauss_origo_x = 0.0;
const float64 CoordinateTransformer::K_rt90_gauss_origo_y = 1.5e6;
const float64 CoordinateTransformer::utmScalefactor = 0.9996;
