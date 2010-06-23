/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ConverterSystem34.h"

#include "MC2Coordinate.h"

#include "CoordinateTransformer.h"


namespace {

   /// A coordinate pair. X in first, Y in second.
   typedef pair<double, double> coordPair_t;

   /**
    *   Computes something.
    */
   coordPair_t computeRealImag( const double factors[][2],
                                int nbrFactors,
                                double xm, double ym) {
        double realPol = 0.0;
        double imagPol = 0.0;
        for ( int i = nbrFactors - 1; i >= 0; --i ) {
           double z = realPol*ym - imagPol*xm + factors[i][0];
           imagPol = realPol*xm + imagPol*ym  + factors[i][1];
           realPol = z;
        }
        return coordPair_t( imagPol, realPol );
   }

   coordPair_t UTM32ToSystem34Jylland( const coordPair_t& utm32 ) {
      const double factors[][2] = {
         {+1.9937114647E+05, -2.6538172775E+05},
         {+1.0001365021E+00, +1.9995520454E-02},
         {-1.6486053260E-11, +3.4452862838E-10},
         {-1.8877421278E-17, -9.6740792784E-17},
      };
      double xm = utm32.first - 530000;
      double ym = utm32.second - 6231000;
      return computeRealImag(factors, NBR_ITEMS( factors ),
                             xm, ym );
   }

   coordPair_t UTM32ToSystem34Sjalland ( const coordPair_t& utm32 ) {
      const double factors[][2] = {
         {+9.6432190004E+04, -1.1935322229E+05},
         {+9.9980074884E-01, +1.9555849611E-02},
         {-7.2077820180E-12, +2.1675209626E-09},
         {+6.2510854612E-17, -2.5215002720E-16},
      };
      double xm = utm32.first  - 678000;
      double ym = utm32.second - 6131000;
      return computeRealImag(factors, NBR_ITEMS( factors ),
                             xm, ym );
   }

   coordPair_t UTM33ToSystem45( const coordPair_t& utm33 ) {
      const double factors[][2] = {
         {+4.2579934018E+04, -4.2993547686E+04},
         {+1.0004053319E+00, -1.4643915438E-03},
         {-7.7039294168E-11, -4.7898338026E-10},
         {+4.7460391692E-15, +3.7432077762E-14},
         {+1.8259975589E-18, -4.8899686702E-18},
         {-2.0727959960E-22, +2.0540300545E-22},
         {+7.0257273976E-27, -2.9421011013E-27},
         {-8.0529957628E-32, +1.5387577766E-33},
      };
      double xm = utm33.first  - 500000;
      double ym = utm33.second - 6100000;
      return computeRealImag(factors, NBR_ITEMS( factors ),
                             xm, ym );
   }
   
   coordPair_t system34JyllandToUTM32( const coordPair_t& system34j ) {
      const double factors[][2] = {
         {+6.2329350512E+06, +5.9533558288E+05},
         {+9.9950870832E-01, -1.9984265541E-02},
         {-2.2732469700E-11, -3.3921905837E-10},
         {+2.6264691305E-17, +9.4810550416E-17},
      };
      double xm = -(-system34j.first - 200000.0);
      double ym = system34j.second - 200000.0;
      return computeRealImag(factors, NBR_ITEMS( factors ),
                             xm, ym );
   }
   
   coordPair_t system34SjallandToUTM32( const coordPair_t& system34s ) {
      const double factors[][2] = {
         {+6.2329356122E+06, +5.9533534820E+05},
         {+9.9945508220E-01, -1.9979251874E-02},
         {-7.3897597436E-11, -2.0717946709E-09},
         {-5.2080308192E-17, +2.5703781294E-16},
      };
      double xm = -(-system34s.first - 200000.0);
      double ym = system34s.second - 200000.0;
      return computeRealImag(factors, NBR_ITEMS( factors ),
                             xm, ym );
   }
   
   coordPair_t system45ToUTM33( const coordPair_t& system45 ) {
      const double factors[][2] = {
         {+6.1074273382E+06, +4.9300725696E+05},
         {+9.9959968336E-01, +1.4686762273E-03},
         {+1.2072957024E-10, +5.4905973626E-10},
         {-9.4593449880E-15, +2.1510833240E-15},
         {+6.5994864072E-19, -1.0163477904E-18},
         {+2.4410466467E-23, +4.4726583308E-23},
         {-2.8946629725E-27, -1.1066966998E-27},
         {+8.0282195772E-32, -5.8658108998E-34},
      };
      double xm =  -(-system45.first - 50000.0);
      double ym = system45.second - 50000.0;
      return computeRealImag( factors, NBR_ITEMS( factors ),
                              xm, ym );
   }

   coordPair_t convertSystem34ToUtm( const coordPair_t& inCoord,
                                     int& utmzone ) {
      // Formula according to Eniro:
      // x < -172816             DanishS34-J99, dvs Jylland och Fyn
      // x < -63946              DanishS34-S99, dvs Själland
      // x >= -63946             DanishS45-B99, dvs Bornholm
      //      -68632 is Kastrup
      coordPair_t utm;

      mc2dbg << "[ConverterSystem34::convertSystem34ToUtm]: x = "
             << inCoord.first << ", y = " << inCoord.second << endl;
      
      if ( inCoord.first < -172816 ) {
         utm = system34JyllandToUTM32( inCoord );
         utmzone = 32;
      } else if ( inCoord.first < -63946 ) {
         utm = system34SjallandToUTM32( inCoord );
         utmzone = 32;
      } else {
         utm = system45ToUTM33( inCoord );
         utmzone = 33;
      }

      mc2dbg << "[ConverterSystem34::convertSystem34ToUtm]. outx = "
             << utm.first << ", outy= " << utm.second << endl;
      
      return utm; 
   }
   
}

MC2Coordinate
ConverterSystem34::convertSystem34ToMC2( double x, double y )
{
   int utmzone;
   coordPair_t utm = convertSystem34ToUtm( coordPair_t(x,y),
                                           utmzone );
   
   return CoordinateTransformer::transformToMC2(
      CoordinateTransformer::utm_ed50,
      utm.second,
      utm.first,
      0.0,
      utmzone );
}

void
ConverterSystem34::convertSystem34ToMC2( float64 in1,
                                         float64 in2,
                                         float64 in3,
                                         float64& out1,
                                         float64& out2,
                                         float64& out3)
{
   out3 = 0;
   in3 = 0;
   int utmzone;
   coordPair_t utm = convertSystem34ToUtm( coordPair_t(in1,in2),
                                           utmzone );
   in1 = utm.second;
   in2 = utm.first;
   CoordinateTransformer::transform( CoordinateTransformer::utm_ed50,
                                     in1, in2, in3,
                                     CoordinateTransformer::mc2,
                                     out1, out2, out3, utmzone );
}

void
ConverterSystem34::convertMC2ToSystem34( float64 in1,
                                         float64 in2,
                                         float64 in3,
                                         float64& out1,
                                         float64& out2,
                                         float64& out3)
{
   // This is not implemented (yet).
   MC2_ASSERT( false );
}
