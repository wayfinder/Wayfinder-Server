/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXROADPOLYGON_H
#define GFXROADPOLYGON_H

#include "config.h"
#include "GfxPolygon.h"
#include "ItemTypes.h"


/**
 *    A polygon of relative coordinates describing roads. Contains 
 *    attributes that are valid per polygon in the GfxRoadFeature.
 *    Some of the attributes are only valid in one of the directions,
 *    e.g. the speed limit attribute.
 *
 */
class GfxRoadPolygon : public GfxPolygon{
   public:
      /** 
       *    Constructor, that initialize all the attrbutes.
       *    @param coordinate16Bits True if 16 bits coordinates are used,
       *             false otherwise.
       *    @param startSize The number of coordinates in this polygon.
       *             To be able to pre-allocate the vectors where the 
       *             coordinates are stored. Optional.  
       *    @param posSpeedLimit The speed limit in positive direction.
       *    @param negSpeedLimit The speed limit in negative direction.
       *    @param multidigitialized True if this road is multidigitalized,
       *             false otherwise.
       *    @param ramp True if this road is a ramp, false otherwise.
       *    @param roundabout True if this road is part of a roundabout, 
       *             false otherwise.
       *    @param level0 The level, relative ground, of the first coordinate.
       *    @param level1 The level, relative ground, of the last coordinate.
       *    @param entryrestr0 The entry restrictions when entering the road
       *             at node 0.
       *    @param entryrestr1 The entry restrictions when entering the road
       *             at node 1.
       */
      GfxRoadPolygon( bool coordinates16Bits, uint32 startSize,
                      byte posSpeedLimit = 0, byte negSpeedLimit = 0, 
                      bool multidigitialized = false,
                      bool ramp = false, bool roundabout = false,
                      int8 level0 = 0, int8 level1 = 0,
                      ItemTypes::entryrestriction_t entryrestr0 
                        = ItemTypes::noRestrictions,
                      ItemTypes::entryrestriction_t entryrestr1
                        = ItemTypes::noRestrictions);

      /** 
       *    Constructor, used when reading from databuffer. None of the
       *    attributes are cleared!
       */
      GfxRoadPolygon();

      /**
       *    Destructor.
       */
      virtual ~GfxRoadPolygon();
      
      /**
       *    @name speedLimitGroup Get the speedlimits
       *    Methods to get the speedlimits of the road, in positive resp.
       *    negative direction.
       *    @{
       */
         /**
          *    Get the positive speedlimit (first -> last coordinate).
          *    @return The speedlimit in positive direction (km/h).
          */
         inline byte getPosSpeedlimit() const;

         /**
          *    Get the negative speedlimit (last -> first coordinate).
          *    @return The speedlimit in negative direction (km/h).
          */
         inline byte getNegSpeedlimit() const;
      /**   @} End of speedLimitGroup */

      /**
       *    Find out if this road is represented by more than one line or 
       *    not.
       *    @return True if this road is multidigitalized, false otherwise.
       */
      inline bool isMultidigitialized() const;

      /**
       *    Find out if this road represent a ramp or not.
       *    @return True if this road is a ramp, false otherwise.
       */
      inline bool isRamp() const;

      /**
       *    Find out if this road represent a roundabout or not.
       *    @return True if this road is a roundabout, false otherwise.
       */
      inline bool isRoundabout() const;

      /**
       *    @name levelgroup Get level relative groupd
       *    Get the level of this road-polygon. The level is relative the
       *    ground, e.g. tunnel has level -1 and bridge level 1.
       *    @{
       */
         /**
          *    Get the level of the first coordinate (node0).
          *    @return The level of the first coordinate in this polygon.
          */
         inline int8 getLevel0() const;

         /**
          *    Get the level of the last oordinate (node1).
          *    @return The level of the last coordinate in this polygon.
          */
         inline int8 getLevel1() const;
      /**   @} end of levelgroup */

      /**
       *    Get the entry regulations when driving into this road-polygon. 
       *    @param node The node to get the regulations for. Valid values
       *                are 0 or 1.
       *    @return The entry regulations at node number <tt>node</tt>.
       */
      inline ItemTypes::entryrestriction_t getEntryRestrictions(int node) const;

      /**
       *    The size of the GfxRoadPolygon when put into a DataBuffer, 
       *    in bytes.
       */
      virtual uint32 getSize() const;

      /**
       *    Dump information about the polygon to stdout.
       */
      virtual void dump(int verboseLevel = 1) const;

      /**
       *    Sets the parameters of this road polygon.
       *    @param posSpeedLimit The speed limit when traversing this
       *           road in positive direction (from first to last coordinate).
       *    @param negSpeedLimit The speed limit when traversing this
       *           road in negative direction (from last to first coordinate).
       *    @param multidigitialized True if this road is digitalized with
       *           more than one line, false otherwise.
       *    @param ramp True if this road is a ramp, false otherwise.
       *    @param roundabout True if this road is a roundabout, false 
       *           otherwise.
       *    @param level0 The level, relative ground, of the first coordinate.
       *    @param level1 The level, relative ground, of the last coordinate.
       *    @param entryrestr0 The entry restrictions when entering the road
       *           at node 0.
       *    @param entryrestr1 The entry restrictions when entering the road
       *           at node 1.
       */
      inline void setParams(byte posSpeedLimit, byte negSpeedLimit,
                            bool multidigitialized, bool ramp, 
                            bool roundabout, int8 level0, int8 level1,
                            ItemTypes::entryrestriction_t entryrestr0, 
                            ItemTypes::entryrestriction_t entryrestr1);

      /**
       *    Set the level for the first coordinate to a new value.
       *    @param level The new level for the first coordinate (node0).
       */
      inline void setLevel0(int level);

      /**
       *    Set the level for the last coordinate to a new value.
       *    @param level The new level for the last coordinate (node1).
       */
      inline void setLevel1(int level);

   protected:
      /**
       *    Read the header (all attributes in this class) from a 
       *    databuffer.
       */
      virtual void readHeader( DataBuffer* data );
      
      /**
       *    Write the header (all attributes in this class) into a 
       *    databuffer.
       */
      virtual void writeHeader( DataBuffer* data ) const;

   private:
      /**
       *    The speed limit of the road, when traversing it in positive 
       *    direction (from the first coordinate to the last).
       */
      byte m_posSpeedLimit;

      /**
       *    The speed limit of the road, when traversing it in negative
       *    direction (from the last coordinate to the first).
       */
      byte m_negSpeedLimit;

      /**
       *    @name level Level for the nodes.
       *    The level of the first and last coordinates in this polygon, 
       *    relative ground. Stored in 4 bits for <b>internal use when
       *    rendering the map only</b>. Only 2 bit when sending on
       *    network.
       *    @{
       */
         /**
          *    The level of the last coordinate, relative ground.
          */
         int8 m_level0 : 4;

         /**
          *    The level of the last coordinate, relative ground.
          */
         int8 m_level1 : 4;
      /***  @} */

      /**
       *    The regulations that are valid when entering this road polygon
       *    at the first coordinate (node 0).
       */
      ItemTypes::entryrestriction_t m_entryrestrictions0 : 2;

      /**
       *    The regulations that are valid when entering this road polygon
       *    at the last coordinate (node 1).
       */
      ItemTypes::entryrestriction_t m_entryrestrictions1 : 2;

      /**
       *    True if the road is multidigitilized, false otherwise.
       */
      bool m_multidigitialized : 1;
      
      /**
       *    True if this road is ramp, false otherwise.
       */
      bool m_ramp : 1;

      /**
       *    True if this road represents a roundabout, false otherwise.
       */
      bool m_roundabout : 1;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline void 
GfxRoadPolygon::setParams(byte posSpeedLimit, byte negSpeedLimit,
                           bool multidigitialized, bool ramp, 
                           bool roundabout, int8 level0, int8 level1,
                            ItemTypes::entryrestriction_t entryrestr0, 
                            ItemTypes::entryrestriction_t entryrestr1)
{
   mc2dbg8 << "GfxRoadPolygon::setParams(" << uint32(negSpeedLimit) << ", " 
           << uint32(posSpeedLimit) << "," << multidigitialized << ", " 
           << ramp << ", " << roundabout << "," << level0 << "," << level1 
           << ")" << endl;
   m_posSpeedLimit = posSpeedLimit;
   m_negSpeedLimit = negSpeedLimit;
   m_multidigitialized = multidigitialized;
   m_ramp = ramp;
   m_roundabout = roundabout;
   m_level0 = level0;
   m_level1 = level1;
   m_entryrestrictions0 = entryrestr0;
   m_entryrestrictions1 = entryrestr1;
}

inline void 
GfxRoadPolygon::setLevel0(int level)
{
   m_level0 = level;
}

inline void 
GfxRoadPolygon::setLevel1(int level)
{
   m_level1 = level;
}


inline byte
GfxRoadPolygon::getPosSpeedlimit() const 
{
   return m_posSpeedLimit;
}

inline byte
GfxRoadPolygon::getNegSpeedlimit() const 
{
   return m_negSpeedLimit;
}

inline int8
GfxRoadPolygon::getLevel0() const 
{
   return int8(m_level0);
}

inline int8
GfxRoadPolygon::getLevel1() const 
{
   return int8(m_level1);
}

inline ItemTypes::entryrestriction_t 
GfxRoadPolygon::getEntryRestrictions(int node) const
{
   MC2_ASSERT( (node == 0) || (node == 1));
   if (node == 0)
      return m_entryrestrictions0;
   return m_entryrestrictions1;
}


inline bool
GfxRoadPolygon::isMultidigitialized() const 
{
   return m_multidigitialized;
}

inline bool
GfxRoadPolygon::isRamp() const 
{
   return m_ramp;
}

inline bool
GfxRoadPolygon::isRoundabout() const 
{
   return m_roundabout;
}

#endif // GfxRoadPolygon_H

