/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXDATAMULTIPLEPOINTS_H
#define GFXDATAMULTIPLEPOINTS_H

#include "config.h"
#include "GfxData.h"

/**
 *
 */   
class GfxDataMultiplePoints : public GfxData {
public:
   /**
    *    Create an empty object, inlined to optimize speed.
    *    @todo Remove initialization of m_closedFigure etc.
    */
   GfxDataMultiplePoints() { };

   /**
    *    Create an object that is a copy of an other GfxData.
    */
   GfxDataMultiplePoints(const GfxDataMultiplePoints& );
      
   /**
    *    Create an object that is a copy of an other GfxData.
    */
   GfxDataMultiplePoints(const GfxData& );

   /**
    *    Delete this object and all data that is created in it.
    */
   virtual ~GfxDataMultiplePoints();

   /**
    *    @name Virtual methods.
    *    This is the implementation of the necessary, virtual methods
    *    in the superclass. See documentation in GfxData.h!
    *    @see GfxData
    */
   //@{
   /// Returns the begin iterator for polygon p
   const_iterator polyBegin( uint16 p ) const;
   /// Returns the end iterator for polygon p
   const_iterator polyEnd( uint16 p ) const;
   /// Returns the begin iterator for polygon p
   iterator polyBegin( uint16 p );
   /// Returns the end iterator for polygon p
   iterator polyEnd( uint16 p );
      
   virtual coordinate_type getLat(uint16 p, uint32 i) const;

   virtual coordinate_type getLon(uint16 p, uint32 i) const;

   virtual uint16 getNbrPolygons() const;

   virtual uint32 getNbrCoordinates(uint16 poly) const;

   virtual bool sortPolygons();

   virtual void setClosed(uint16 poly, bool closed);

   virtual bool getClosed(uint16 poly) const;

   virtual coordinate_type getMinLat() const;

   virtual coordinate_type getMaxLat() const;

   virtual coordinate_type getMinLon() const;

   virtual coordinate_type getMaxLon() const;

   virtual float64 getCosLat() const;

   virtual float64 getLength(uint16 p) const;
         
   virtual bool updateLength(uint16 polindex = MAX_UINT16);

   virtual void setLength(uint16 p, float64 length);

   virtual bool updateBBox();

   virtual bool updateBBox(const coordinate_type lat, 
                           const coordinate_type lon);

   //@}
protected:
   void readPolygons( uint16 nbrPolygons,
                      DataBuffer& dataBuffer, 
                      GenericMap* map);

private:
   typedef MC2Coordinate coord_t;
   vector<coord_t> m_coordinates;
};

#endif

