/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TransformMatrix.h"

#include <math.h>

// Move this back to .h file when working. (transformPointInternalCosLat)
// My home computer
// compiles really slow.

// Some macros for fix-point calcs. Seem to exist ARM-optimized versions
#define FPP		16
#define FPONE		65536
#define FPMUL(x,y)	((((x)>>6)*((y)>>6))>>4)
#define FPDIV(x,y)	((((x)<<6)/((y)>>6))>>4)



void TransformMatrix::setToIdentity()
{
   updateMatrix(0.0, 1.0, MC2Coordinate(0,0), 0, 0);
}

TransformMatrix::TransformMatrix()
{
   setToIdentity();
}

int16
TransformMatrix::getScaleAndShift(scaleInt_t& outScale, double inScale, 
                                  int maxShift)
{
  // I guess this function could be optimized.
   double testNumber = inScale;
   int shift = 0;
   while ( fabs(testNumber) < (32767/2) && ( shift <= maxShift ) ) {
      testNumber *= 2;
      shift += 1;
   }
   outScale = int16(testNumber/2.0);
   return shift-1;
}

void
TransformMatrix::setInternalCosLat(double cosLat)
{
   m_cosLatSxA = cosLat * m_SxA;
   m_cosLatSxATx_plus_SyBTy_plus_tx = cosLat*m_SxATx + m_SyBTy_plus_tx;
   m_cosLatSxC = cosLat*m_SxC;
   m_cosLatSxCTx_plus_m_SyDTy_plus_ty = cosLat*m_SxCTx + m_SyDTy_plus_ty;
   m_cosLat = cosLat;

   // Calculate almost-constants.
#ifdef USE_INTEGERS
   // We must start by setting the m_scale1Shift
   scaleInt_t notUsed;
   m_scale1Shift    = getScaleAndShift(notUsed, 0.009, 32);
   m_scale1CosLatShift = getScaleAndShift(notUsed, 0.009*m_cosLat, 32);
   
   const int maxShift = 32;
   m_cosLatSxAShift = getScaleAndShift(m_cosLatSxAInt, m_cosLat * m_Sx * m_A,
                                       maxShift);
   m_SyBShift       = getScaleAndShift(m_SyBInt, m_Sy * m_B,
                                       maxShift);
   m_cosLatSxCShift = getScaleAndShift(m_cosLatSxCInt, m_cosLat * m_Sx * m_C,
                                       maxShift);
   m_SyDShift       = getScaleAndShift(m_SyDInt, m_Sy * m_D,
                                       maxShift);
   // Fixup the constants to avoid later additions and subtractions.
   m_cosLatSxAShift = m_cosLatSxAShift - m_scale1CosLatShift + 1;
   m_SyBShift = m_SyBShift - m_scale1Shift + 1;
   m_cosLatSxCShift = m_cosLatSxCShift - m_scale1CosLatShift + 1;
   m_SyDShift = m_SyDShift - m_scale1Shift + 1;

   m_scale1Shift--;
   m_scale1CosLatShift--;

   // Calculate the new scales
#else
#ifdef USE_NEW_INTEGERS
   const int maxShift = 31;
   // Some constants for the new version
   m_cosLatSxAShift1 = getScaleAndShift(m_cosLatSxAInt, m_cosLat * m_Sx * m_A,
                                        maxShift);   
   m_SyBShift1       = getScaleAndShift(m_SyBInt, m_Sy * m_B,
                                        maxShift);
   m_cosLatSxCShift1 = getScaleAndShift(m_cosLatSxCInt, m_cosLat * m_Sx * m_C,
                                        maxShift);
   m_SyDShift1       = getScaleAndShift(m_SyDInt, m_Sy * m_D,
                                        maxShift);
#endif
#endif
      
   m_TxInt = int32(m_Tx);
   m_TyInt = int32(m_Ty);
   m_txInt = int32(m_tx);
   m_tyInt = int32(m_ty);

#if 0
   // Warning to be able to see it in MapLib.
   mc2log << warn << "m_scale1Shift = " << m_scale1Shift << endl;   
   mc2log << warn << "m_cosLatSxAShift = " << m_cosLatSxAShift
	  << ", m_cosLatSxAInt = " << m_cosLatSxAInt << endl;
   mc2log << warn << "m_SyBShift = " << m_SyBShift << ", m_SyBInt = "
	  << m_SyBInt << endl;
   mc2log << warn << "m_cosLatSxCShift = " << m_cosLatSxCShift
	  << ", m_cosLatSxCInt = " << m_cosLatSxCInt << endl;
   mc2log << warn << "m_SyDShift = " << m_SyDShift << ", m_SyDInt = "
	  << m_SyDInt << endl;
#endif
}

void
TransformMatrix::updateMatrix(double angle360,
                              double mc2scale,
                              const MC2Coordinate& center,
                              int screenWidth, int screenHeight)
{
   float Tx = -center.lon;
   float Ty = -center.lat;

   // The scaling - note that lat is mirrored.
   float origSx = m_Sx =  mc2scale;
   float Sy =     m_Sy = -mc2scale;

   float cosangle = ::cos(angle360 / 180.0 * M_PI );
   float sinangle = ::sin(angle360 / 180.0 * M_PI );

   // Rotation constants 
   float A = m_A = cosangle;
   float B = m_B = sinangle;
   float C = m_C = -sinangle;
   float D = m_D = cosangle;
  
   // Final translation back into the screen.
   float tx = screenWidth  >> 1;
   float ty = screenHeight >> 1;

   // Stuff that we want to use when translating
   float Sx = origSx;
   m_SxA = Sx*A;
   m_SxB = Sx*B;
   m_SyB = Sy*B;
   m_SxC = Sx*C;
   m_SyC = Sy*C;
   m_SxD = Sx*D;
   m_SyD = Sy*D;   
   m_SxATx = Sx*A*Tx;
   m_SyBTy = Sy*B*Ty;
   m_SxCTx = Sx*C*Tx;
   m_SyDTy = Sy*D*Ty;
   m_Tx = Tx;
   m_Ty = Ty;
   m_tx = tx;
   m_ty = ty;
   m_SxAtxBty = Sx*A*tx + Sx*B*ty;
   m_SyCtxSyDtyTy = Sy*C*tx + Sy*D*ty + Ty;

   m_SyBTy_plus_tx = m_SyBTy+m_tx;
   m_SyDTy_plus_ty = m_SyDTy+m_ty;

   setInternalCosLat(getCosLat(center.lat));   

   // FIXME: Check if there really was a difference.
   matrixUpdated();
}




