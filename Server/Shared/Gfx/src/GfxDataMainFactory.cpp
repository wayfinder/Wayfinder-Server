/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxDataMainFactory.h"

#include "AllocatorTemplate.h"

#include "GfxDataSingleSmallPoly.h"
#include "GfxDataSinglePoint.h"
#include "GfxDataSingleLine.h"
#include "GfxDataMultiplePoints.h"

GfxDataMainFactory::
GfxDataMainFactory( uint32 nbrGfxDatas, 
                    uint32 nbrGfxDataSingleSmallPoly,
                    uint32 nbrGfxDataSingleLine, 
                    uint32 nbrGfxDataSinglePoint,
                    uint32 nbrGfxDataMultiplePoints ):
   GfxDataFullFactory( nbrGfxDatas ),
   m_gfxDataSingleSmallPolyAllocator(
     new MC2Allocator<GfxDataSingleSmallPoly>( nbrGfxDataSingleSmallPoly ) ),
   m_gfxDataSingleLineAllocator( 
     new MC2Allocator<GfxDataSingleLine>( nbrGfxDataSingleLine ) ),
   m_gfxDataSinglePointAllocator( 
     new MC2Allocator<GfxDataSinglePoint>( nbrGfxDataSinglePoint ) ),
   m_gfxDataMultiplePointsAllocator( 
     new MC2Allocator<GfxDataMultiplePoints>( nbrGfxDataMultiplePoints ) )
{

}

GfxDataMainFactory::~GfxDataMainFactory() {
}

GfxData* GfxDataMainFactory::
create( GfxData::gfxdata_t type ) 
{
   GfxData* gfx = NULL;

   switch (type) {
   case GfxData::gfxDataFull :
   case GfxData::gfxDataMultiplePoints :
      gfx = create(); // create GfxDataFull
      break;
   case GfxData::gfxDataSingleSmallPoly :
      gfx = m_gfxDataSingleSmallPolyAllocator->getNextObject();
      break;
   case GfxData::gfxDataSingleLine :
      gfx = m_gfxDataSingleLineAllocator->getNextObject();
      break;
   case GfxData::gfxDataSinglePoint :
      gfx = m_gfxDataSinglePointAllocator->getNextObject();
      break;
      /*case gfxDataMultiplePoints :
           gfx = static_cast<MC2Allocator<GfxDataMultiplePoints> *>
           (theMap->m_gfxDataMultiplePointsAllocator)->getNextObject();
           break;*/
   }

   return gfx;
}

