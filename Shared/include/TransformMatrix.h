/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRANSFORMMATRIX_H
#define TRANSFORMMATRIX_H

#include "config.h"

#include "MC2Point.h"
#include "MC2Coordinate.h"

#include <math.h>
#include <stdlib.h>

#define USE_NEW_INTEGERS

/**
 *   Class containing data needed to transform data with rotating,
 *   scaling and translating. Mostly for transforming lat/lon into
 *   screen coordinates.
 *   <br />
 *   Consists of the following transformation: t*R*S*T where
 *   T moves the coordinates to origo, S scales, R rotates and t
 *   moves the coordinates back into the screen.
 * 
 */
class TransformMatrix {
public:

   /**
    *   Type of integer to use for scales.
    *   It may be possible that 32 bits are faster that 16 on thumb.
    */
   typedef int32 scaleInt_t;
   
   /**
    *   Default constructor.
    */
   TransformMatrix();
   
   /**
    *  Sets the matrix to the identity.
    */
   void setToIdentity();
   
   /**
    *   Sets the data in the internal structure. Specialised for
    *   lat/lon->screen. cosLat is set to the cosine of the
    *   center latitude.
    *   @param angle360     Angle in degrees 0-359.999.
    *   @param scale        The scale in pixels per mc2 units.
    *   @param topLeft      The top-left corner of the screen.
    *   @param screenWidth  Width of the screen.
    *   @param screenHeight Height of the screen.
    */
   void updateMatrix(double angle360,
                     double scale,
                     const MC2Coordinate& center,
                     int screenWidth, int screenHeight);

   /**
    *    Sets the internal cosLat and updates constants for calls
    *    to <code>transformPointInternalCosLat</code>. UpdateMatrix
    *    must be called before setInternalCosLat.
    */
   void setInternalCosLat(double cosLat);
   
   /**
    *    Transforms a coordinate into a screen coordinate using the
    *    cosLat set by a prevous call to setInternalCosLat.
    */
   inline void 
     transformPointInternalCosLat( MC2Point::coord_t& outX,
                                   MC2Point::coord_t& outY,
                                   const MC2Coordinate& incoord ) const;

   /**
    *    Transforms a coordinate into a screen coordinate using the
    *    cosLat set by a prevous call to setInternalCosLat.
    */
   inline void 
      transformPointInternalCosLat( MC2Point& outPoint,
                                    const MC2Coordinate& incoord ) const;

   /**
    *    Transforms a coordinate into a screen coordinate.
    */
   inline void transformPointUsingCosLat( MC2Point::coord_t& outX,
                                          MC2Point::coord_t& outY,
                                          const MC2Coordinate& incoord) const;
   
   /**
    *    Transforms a coordinate into a screen coordinate.
    */
   inline void transformPointCosLatSupplied( MC2Point::coord_t& outX,
                                             MC2Point::coord_t& outY,
                                             const MC2Coordinate& incoord,
                                             float cosLat) const;
   
   /**
    *    Transforms a coordinate into a screen coordinate.
    */
   inline void transformPointCosLatSupplied( MC2Point& outPoint,
                                             const MC2Coordinate& incoord,
                                             float cosLat ) const;

   /**
    *    Transforms a screen coordinate into lat/lon.
    */
   inline void inverseTranformUsingCosLat( MC2Coordinate& outCoord,
                                           int32 inX, int32 inY ) const;
   /**
    *    Transforms a screen coordinate into lat/lon.
    */
   inline void inverseTranformUsingCosLat( MC2Coordinate& outCoord,
                                           const MC2Point& inPoint) const;
   
   /**
    *    Transforms a screen coordinate into lat/lon.
    */
   inline void inverseTranformCosLatSupplied( MC2Coordinate& outCoord,
                                              int32 inX, int32 inY,
                                              float cosLat ) const;
   /**
    *    Transforms a screen coordinate into lat/lon.
    */
   inline void inverseTranformCosLatSupplied( MC2Coordinate& outCoord,
                                              const MC2Point& inPoint,
                                              float cosLat ) const {
      inverseTranformCosLatSupplied( outCoord, inPoint.getX(), inPoint.getY(),
                                     cosLat );
   }

   /**
    *    Calculates cosine for the latitude.
    */
   static inline double getCosLat(int32 lat);
   
   /**
    *    Calculates cosine for the mean latitude.
    */
   static inline double getCosLat(int32 lat1, int32 lat2);

 protected:
   /**
    *    Method that is called in updateMatrix so that
    *    subclasses can update their internal state when
    *    that happens. (E.g. for every move,zoom etc)
    */
   virtual void matrixUpdated() {}
   
   /**
    *    Calculates the shift numbers and integer scale for the
    *    supplied inscale.
    *    @param outScale The integer scale.
    *    @param inScale  The real scale.
    *    @param maxShift The maximum allowed number to shift.
    *    @return The shift amount.
    */
   int16 getScaleAndShift(int32& outScale, double inScale, int maxShift);
   
   float m_SxA;   
   float m_SyB;
   float m_SxC;
   float m_SyD;
   float m_SxATx;
   float m_SyBTy;
   float m_SxCTx;
   float m_SyDTy;
   float m_SyCtxSyDtyTy;
   float m_SxAtxBty;

   float m_SxB;
   float m_SyC;
   float m_SxD;

   float m_Sx;
   float m_Sy;
   
   /// Final translation in x-dir.
   float m_tx;
   /// Final translation in y-dir.
   float m_ty;
   /// Starting translation in lon-dir
   float m_Tx;
   float m_Ty;

   float m_A;
   float m_B;
   float m_C;
   float m_D;

   float m_SyBTy_plus_tx;
   float m_SyDTy_plus_ty;

   /// Some more precalculated values depending on cosLat.
   float m_cosLatSxA;
   float m_cosLatSxATx_plus_SyBTy_plus_tx;
   float m_cosLatSxC;
   float m_cosLatSxCTx_plus_m_SyDTy_plus_ty;

   float m_cosLat;
   
   // Some integers   
   /// Shift for minimum scale.
#ifdef USE_INTEGERS
   int m_scale1Shift;
   int m_scale1CosLatShift;
   /// Shift for first constant for X-calc.
   int m_cosLatSxAShift;
   /// Shift amount for second constant X-calc.
   int m_SyBShift;
   /// Shift amount for first constant Y-calc.
   int m_cosLatSxCShift;
   /// Shift amount for second constant Y-calc.
   int m_SyDShift;
#else
#ifdef USE_NEW_INTEGERS
   // For the new version
   /// Shift for first constant for X-calc.
   int m_cosLatSxAShift1;
   /// Shift amount for second constant X-calc.
   int m_SyBShift1;
   /// Shift amount for first constant Y-calc.
   int m_cosLatSxCShift1;
   /// Shift amount for second constant Y-calc.
   int m_SyDShift1;
#endif
#endif

   int32 m_TxInt;
   int32 m_TyInt;
   /// Could be 16
   int32 m_txInt;
   /// Could be 16 
   int32 m_tyInt;
   
   /// Integer scale for second constant in Y-calc.
   scaleInt_t m_SyDInt;
   /// Integer scale for first constant X-calc.
   scaleInt_t m_cosLatSxAInt;
   /// Integer scale for second constant X-calc.
   scaleInt_t m_SyBInt;
   /// Integer scale for first constant Y-calc.
   scaleInt_t m_cosLatSxCInt;
};

inline double
TransformMatrix::getCosLat(int32 lat)
{
   static const double mc2scaletoradians = 1.0 / (11930464.7111*
                                                180.0/3.14159265358979323846);
   return cos(mc2scaletoradians*lat);
}

inline double
TransformMatrix::getCosLat(int32 lat1, int32 lat2)
{
   // Loses a coordinate sometimes, but that is approx one cm.
   return getCosLat( (lat1 >> 1) + (lat2 >> 1) );
}

/// Calculates one term in the expressions below. Approx a multiplication
// Old version without precalculated shift steps
//  #define CALC_ONE_TERM(diff, scale, n, m) 
//                     (int32((scale)*((diff) >> ((n)-(m)+1))) >> (m-1))

#if 0
// Well this does not work when using thumb anyway.
#if defined(__MARM__) && defined(__GCC32__) && defined(__MARM_ARMI__)
static inline int32 MULT16_16(int16 x, int16 y) {
  int res;
  asm ("smulbb  %0,%1,%2;\n"
              : "=&r"(res)
              : "%r"(x),"r"(y));
  return(res);
}
#else
#define MULT16_16(a,b) ( ((int32)(int16(a)))*((int32)(int16(b))) )
#endif
#else
#define MULT16_16(a,b) ((a)*(b))
#endif
// New version where the shift steps are precalculated.
#define CALC_ONE_TERM(diff, scale, n, m) \
                     (int32(MULT16_16((scale), ((diff) >> (n)))) >> (m))

#define CALC_ONE_TERM_MAYBE_BETTER(diff, scale, n, m) \
     (((scale) != 0) ? CALC_ONE_TERM_SUB(diff,scale,n,m) : 0 )

#ifdef USE_NEW_INTEGERS

inline int getShiftDownTo15( int nbr )
{
   int shift = 0;
   // Get the number of shifts needed to get the number down to
   // 15 bits.
   // FIXME: Optimize
   nbr = abs(nbr);
   static const int maxNbr = (0xffff) >> 2;
   while ( nbr > maxNbr ) {
      nbr >>= 1;
      ++shift;
   }
   return shift;
}

static inline int32 calcTerm( int32 diff, int diffShift,
                              int32 scale, int scaleShift )
{
   int32 tmpRes = (diff >> diffShift )*( scale );
   int shiftAmount = scaleShift - diffShift;
   if ( shiftAmount > 0 ) {
      tmpRes >>= shiftAmount;
   } else {
      tmpRes <<= -shiftAmount;
   }
   return tmpRes;
}

#endif

inline void
TransformMatrix::
transformPointInternalCosLat( MC2Point::coord_t& outX,
                              MC2Point::coord_t& outY,
                              const MC2Coordinate& incoord ) const
{
#ifdef USE_NEW_INTEGERS
   const int32 lonDiff = incoord.lon + m_TxInt;
   const int32 latDiff = incoord.lat + m_TyInt;
   const int latShift = getShiftDownTo15(latDiff);
   const int lonShift = getShiftDownTo15(lonDiff);
   
//     mc2log << "[TM]: latShift = " << latShift << ", lonShift = " << lonShift
//            << ", m_cosLatSxAShift = " << m_cosLatSxAShift
//            << ", m_cosLatSxAInt = " << m_cosLatSxAInt
//            << ", latDiff >> latShift = " << (latDiff >> latShift ) << endl;
 
   outX = calcTerm( lonDiff, lonShift,
                    m_cosLatSxAInt,
                    m_cosLatSxAShift1 ) +
      calcTerm( latDiff, latShift, m_SyBInt, m_SyBShift1 ) +
      m_txInt;

   outY = calcTerm( lonDiff, lonShift,
                    m_cosLatSxCInt,
                    m_cosLatSxCShift1 ) +
      calcTerm( latDiff, latShift, m_SyDInt, m_SyDShift1 ) +
      m_tyInt;

#else 
#ifdef USE_INTEGERS
   // Old integer version
   const int32 lonDiff = lon + m_TxInt;   
   const int32 latDiff = lat + m_TyInt;
  
   outX = CALC_ONE_TERM(lonDiff, m_cosLatSxAInt, 
		       m_cosLatSxAShift, m_scale1CosLatShift) +
      CALC_ONE_TERM(latDiff, m_SyBInt,
                    m_SyBShift, m_scale1Shift) + m_txInt;
   
  outY = CALC_ONE_TERM(lonDiff, m_cosLatSxCInt,
                       m_cosLatSxCShift, m_scale1CosLatShift) +
     CALC_ONE_TERM(latDiff, m_SyDInt,
                   m_SyDShift, m_scale1Shift) + m_tyInt;
#else
#if 0
   // These are the expressions which are used to derive the 
   // integer versions.
   outX =
      int32(m_cosLat*m_Sx*m_A*(incoord.lon + m_Tx) +
            m_Sy*m_B*(incoord.lat + m_Ty) + m_tx );
  
   outY = int32(m_cosLat*m_Sx*m_C*(incoord.lon + m_Tx) +
		m_Sy*m_D*(incoord.lat + m_Ty) + m_ty );

#endif

  // These are the shortest floating point expressions for 
  // the calculations
   outX = int32( incoord.lon*(m_cosLatSxA)+
                 incoord.lat*(m_SyB)+(m_cosLatSxATx_plus_SyBTy_plus_tx) );

   outY =
      int32(incoord.lon*(m_cosLatSxC)+
          incoord.lat*(m_SyD)+(m_cosLatSxCTx_plus_m_SyDTy_plus_ty) );
#endif
#endif
   
#define COMPARE_TO_FLOAT
#ifdef COMPARE_TO_FLOAT
#ifdef __unix__
   MC2Point::coord_t compX = int32( incoord.lon*(m_cosLatSxA)+
                                    incoord.lat*(m_SyB)+
                                    (m_cosLatSxATx_plus_SyBTy_plus_tx) );

   MC2Point::coord_t compY =
      int32(incoord.lon*(m_cosLatSxC)+
            incoord.lat*(m_SyD)+(m_cosLatSxCTx_plus_m_SyDTy_plus_ty) );
   if ( ( outX != compX ) || ( outY != compY ) ) {
      if ( ( abs( outX - compX ) > 2 ) || ( abs( outY - compY ) > 2 ) ) {
         mc2log << "[TM]: Diffs are ("
                << (outX-compX) << ","
                << (outY - compY ) << ")" << " for orig coord "
                << incoord << " (latDiff >> latShift) = "
                << (latDiff >> latShift ) 
                << ", (lonDiff >> lonShift) = " << (lonDiff >> lonShift )
                << ", lonDiff = " << lonDiff
                << ", lonShift = " << lonShift
                << ", m_cosLatSxAInt = " << m_cosLatSxAInt
                << ", m_cosLatSxAShift1 = " << m_cosLatSxAShift1
                << ", (m_cosLat * m_Sx * m_A) = " << (m_cosLat * m_Sx * m_A)
                << endl;
      }
   }
#endif
#endif

}

inline void
TransformMatrix::
transformPointInternalCosLat( MC2Point& outPoint,
                              const MC2Coordinate& incoord ) const
{
   transformPointInternalCosLat(outPoint.getX(), outPoint.getY(), incoord);
}


inline void
TransformMatrix::transformPointCosLatSupplied( MC2Point::coord_t& outX,
                                               MC2Point::coord_t& outY,
                                               const MC2Coordinate& incoord,
                                               float cosLat ) const
{   
   outX =
      int32(incoord.lon*(cosLat*m_SxA)+
          incoord.lat*(m_SyB)+(cosLat*m_SxATx + m_SyBTy_plus_tx) );   

#ifdef __unix__
   mc2dbg8 << "[TMX]: Sx = " << m_Sx << " A = " << m_A
          << " SxA " << m_SxA << " lon = " << incoord.lon << " T_x = "
          << m_Tx << " lon + T_X " << (incoord.lon+m_Tx) << endl;
#endif
   
   outY =
      int32(incoord.lon*(cosLat*m_SxC)+
          incoord.lat*(m_SyD)+(cosLat*m_SxCTx + m_SyDTy_plus_ty) );
}

inline void
TransformMatrix::transformPointCosLatSupplied( MC2Point& outPoint,
                                               const MC2Coordinate& incoord,
                                               float cosLat ) const
{
   transformPointCosLatSupplied(outPoint.getX(),
                                outPoint.getY(), incoord, cosLat);
}

inline void
TransformMatrix::transformPointUsingCosLat( MC2Point::coord_t& outX,
                                            MC2Point::coord_t& outY,
                                            const MC2Coordinate& incoord )
   const
{   
   transformPointCosLatSupplied(outX, outY, incoord, getCosLat(incoord.lat));
}



inline void
TransformMatrix::inverseTranformCosLatSupplied( MC2Coordinate& outCoord,
                                              int32 inX, int32 inY,
                                              float cosLat ) const
{
   outCoord.lat = int32( - 1.0/m_Sy * m_C * inX
                         + 1.0/m_Sy * m_D * inY
                         + 1.0/m_Sy * m_C * m_tx 
                         - 1.0/m_Sy * m_D * m_ty
                         - m_Ty );
   
   double S_x = m_Sx * cosLat;
   
   outCoord.lon = int32( + 1.0 / S_x * m_A * inX
                         - 1.0 / S_x * m_B * inY
                         - 1.0 / S_x * m_A * m_tx
                         + 1.0 / S_x * m_B * m_ty
                         - m_Tx);

#ifdef __unix__
   MC2Point backXY(0,0);
   transformPointUsingCosLat(backXY.getX(), backXY.getY(), outCoord);
   
   mc2dbg8 << "[TMX]: Coord is " << outCoord << endl;
   mc2dbg8 << "[TMX]: (inX, inY) = " << MC2Coordinate(inX, inY)
           << " converted back " << MC2Coordinate(backXY.getX(), backXY.getY())
           << endl;
#endif
}

inline void
TransformMatrix::
inverseTranformUsingCosLat( MC2Coordinate& outCoord,
                            int32 inX, int32 inY ) const
{
   outCoord.lat = int32( - 1.0/m_Sy * m_C * inX
                         + 1.0/m_Sy * m_D * inY
                         + 1.0/m_Sy * m_C * m_tx
                         - 1.0/m_Sy * m_D * m_ty
                         - m_Ty );
   
   double S_x = m_Sx * getCosLat(outCoord.lat);
   
   outCoord.lon = int32( + 1.0 / S_x * m_A * inX
                         - 1.0 / S_x * m_B * inY
                         - 1.0 / S_x * m_A * m_tx
                         + 1.0 / S_x * m_B * m_ty
                         - m_Tx);

#ifdef __unix__
   MC2Point backXY(0,0);
   transformPointUsingCosLat(backXY.getX(), backXY.getY(), outCoord);
   
   mc2dbg8 << "[TMX]: Coord is " << outCoord << endl;
   mc2dbg8 << "[TMX]: (inX, inY) = " << MC2Coordinate(inX, inY)
           << " converted back " << MC2Coordinate(backXY.getX(), backXY.getY())
           << endl;
#endif
}

inline void
TransformMatrix::inverseTranformUsingCosLat( MC2Coordinate& outCoord,
                                             const MC2Point& inPoint) const
{
   inverseTranformUsingCosLat(outCoord, inPoint.getX(), inPoint.getY() );
}


#endif





