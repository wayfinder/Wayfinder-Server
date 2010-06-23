/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPAND_ITEM_ID
#define EXPAND_ITEM_ID

#include "Types.h"
#include "Vector.h"

/**
  *   The route in gfx-form is inserted into a list of 
  *   ExpandItemIDs that is returned to the caller
  *   of the getItemIDs()-method.
  *
  */
class ExpandItemID : public VectorElement {
   public:
      /**
        *   Constructor that creates a new 
        */
      ExpandItemID();

      /**
        */
      void addItem( uint32 mapID,
                    uint32 itemID );

      /**
       */
      void addItem( uint32 mapID,
                    uint32 groupID,
                    uint32 itemID,
                    int32 lat,
                    int32 lon );

      /**
       */
      void addItem( uint32 mapID,
                    uint32 groupID,
                    uint32 itemID );
      
      /**
       */
      void addItem ( uint32 mapID,
                     uint32 itemID,
                     int32 lat,
                     int32 lon);
      
      /**
       *    Add speedlimit for item.
       *    @param   speedLimit  The speedlimit.
       */
      void addSpeedLimit(byte speedLimit);
      
      /**
       *    Add additional attributes (bitfield) for item.
       *    @see ExpandRouteReplyPacket::createAttributes 
       *    @param   attributes  Additional attributes.
       */
      void addAttributes(byte attributes);
      
      /**
       *   Add coordiante for item. Requires that any of the addItem()
       *   methods has been called first in order to specify which 
       *   item the coordiante should be added to.
       *   @param lat   Lat coordinate.
       *   @param lon   Lon coordinate.
       */
      void addCoordinate(int32 lat, int32 lon);
      
      /**
        */
      uint32 getNbrItems() {
         return m_itemID.getSize();
      }
     
      /**
        */
      Vector& getItemID() {
         return m_itemID;
      }

      /**
        */
      Vector& getMapID() {
         return m_mapID;
      }

       /**
        */
      Vector& getGroupID() {
         return m_groupID;
      }
      
      /**
        *   Get raw vector of latitude coordinates. Use with the
        *   coordinate offset vector in order to know which coordinate
        *   corresponds to which item (unless all items only have one
        *   coordinate).
        *   @return The latitude coordinate vector.
        */
      IntVector& getLat() {
         return m_lat;
      }
      
      /**
        *   Get raw vector of longitude coordinates. Use with the
        *   coordinate offset vector in order to know which coordinate
        *   corresponds to which item (unless all items only have one
        *   coordinate).
        *   @return The longitude coordiante vector.
        */
      IntVector& getLon() {
         return m_lon;
      }

      /**
       *    Get the vector of coordinate offsets.
       *    Index $i$ in this vector has the value of at which position
       *    the first coordinate in item $i$ can be found in the lat and 
       *    and lon vector. This means that when there only is one 
       *    coordinate per item the vector would look like 
       *    [0, 1, 2, 3, ...] and
       *    if all items have 3 coordinates it would look like
       *    [0, 3, 6, ...].
       *    
       */
      Vector& getCoordinateOffset() {
         return (m_coordinateOffset);
      }

      /**
       *    Get the vector of speedlimits.
       *    @return The vector of speedlimits.
       */
      Vector& getSpeedLimit() {
         return (m_speedLimit);
      }
      
      /**
       *    Get the vector of additional attributes (bitfield).
       *    @see ExpandRouteReplyPacket::extractAttributes.
       *    @return The vector of additional attributes.
       */
      Vector& getAttributes() {
         return (m_attributes);
      }
      
      /**
       *    Get the number of coordinates for a item.
       *    Somewhat costly to do this, especially if the intention
       *    is to go through all coordinates of all items.
       *    
       *    @param   itemNbr  The item nbr, ie. a number between 0 and
       *                      getNbrItems() - 1.
       *    @return  The number of coordinates for this item.
       */
      inline uint32 getNbrCoordinates(uint32 itemNbr); 
      
      /**
       *    Get the latitude coordinate # coordinateNbr in item # itemNbr.
       *    Somewhat costly to do this, especially if the intention
       *    is to go through all coordinates of all items.
       *    
       *    @param   itemNbr        The ordinal number of the item.
       *    @param   coordinateNbr  The ordinal number of the coordinate
       *                            in the specified item.
       *    @return  The latitude value.
       */
      inline int32 getLatValue(uint32 itemNbr, uint32 coordinateNbr);       
      
      /**
       *    Get the longitude coordinate # coordinateNbr in item # itemNbr.
       *    Somewhat costly to do this, especially if the intention
       *    is to go through all coordinates of all items.
       *    
       *    @param   itemNbr        The ordinal number of the item.
       *    @param   coordinateNbr  The ordinal number of the coordinate
       *                            in the specified item.
       *    @return  The longitude value.
       */
      inline int32 getLonValue(uint32 itemNbr, uint32 coordinateNbr); 
      
      /**
       *    Get the position in the coordinate vectors 
       *    for the first coordinate of the next item.
       *    @param   itemNbr  The current itemNbr.
       *    @return  The position in the coordinate vectors for the
       *             first coordinate of item # itemNbr+1.
       */
      inline uint32 nextCoordStart(uint32 itemNbr);
    
   private:
      /**
        */
      Vector m_itemID;

      /**
        */
      Vector m_mapID;

      /**
       */
      Vector m_groupID;

      /**
        *   Raw vector of latitude coordinates. Use with m_coordinateOffset
        *   in order to know which item the coordinates belongs to.
        */
      IntVector m_lat;

      /**
        *   Raw vector of latitude coordinates. Use with m_coordinateOffset
        *   in order to know which item the coordinates belongs to.
        */
      IntVector m_lon;

      /**
        */
      uint32  position;
      
      /**
       *    Vector with offsets to the coordinate vectors m_lat and m_lon.
       *    Index $i$ in this vector has the value of at which position
       *    the first coordinate in item $i$ can be found in m_lat 
       *    and m_lon. This means that when there only is one coordinate
       *    per item the vector would look like [0, 1, 2, 3, ...] and
       *    if all items have 3 coordinates it would look like
       *    [0, 3, 6, ...], you'll get the idea.
       */
      Vector m_coordinateOffset;

      /**
       *    Vector of speedlimits.
       */
      Vector m_speedLimit;

      /**
       *    Vector of additional attributes (bitfield).
       *    @see ExpandRouteReplyPacket::extractAttributes
       */
      Vector m_attributes;

};

// =======================================================================
//                                     Implementation of inlined methods =

inline uint32 
ExpandItemID::getNbrCoordinates(uint32 itemNbr) 
{
   if (itemNbr < (m_coordinateOffset.getSize() - 1)) {
      return (m_coordinateOffset.getElementAt(itemNbr + 1) - 
            m_coordinateOffset.getElementAt(itemNbr));
   } else if (itemNbr == (m_coordinateOffset.getSize() - 1)) {
      return (m_lon.getSize() - m_coordinateOffset.getElementAt(itemNbr));
   } else {
      // Error
      mc2log << error << "ExpandItemID::getNbrCoordinates() Idx out of range"
             << endl;
      return (MAX_UINT32);
   }
}

inline int32 
ExpandItemID::getLatValue(uint32 itemNbr, uint32 coordinateNbr) 
{
   uint32 coordOffset = m_coordinateOffset.getElementAt(itemNbr);
   return (int32(m_lat.getElementAt(coordOffset + coordinateNbr)));
}

inline int32 
ExpandItemID::getLonValue(uint32 itemNbr, uint32 coordinateNbr) 
{
   uint32 coordOffset = m_coordinateOffset.getElementAt(itemNbr);
   return (int32(m_lon.getElementAt(coordOffset + coordinateNbr)));
}

inline uint32 
ExpandItemID::nextCoordStart(uint32 itemNbr)
{
   itemNbr++;
   if (itemNbr >= m_coordinateOffset.getSize()) {
      return (m_lat.getSize());
   } else {
      return (m_coordinateOffset.getElementAt(itemNbr));
   }
}

#endif
