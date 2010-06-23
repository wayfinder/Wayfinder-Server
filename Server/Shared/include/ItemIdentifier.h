/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMIDENTIFIER_H
#define ITEMIDENTIFIER_H

#include "config.h"

/**
 *   Class describing an item without specifying the item id.
 *
 *   There are two ways of using the ItemIdentifier. The name and the item 
 *   type must always be specified. Then either the name of another item 
 *   can be specified, or the coordinates of a point close to the item. 
 *
 *   This point must be less than maxDistToOpenPolygonInMeters meters away
 *   from the item in case of being an open polygon. If closed polygon, the
 *   coordinate must be inside the polygon.
 *
 */
class ItemIdentifier
{
   public:

      /**
       *    A coordinate describing an item must be within this 
       *    distance to the item, in case the item is an open polygon.
       *    If the item is a closed polygon, then the coordinate must
       *    be inside the polygon.
       */
      static const uint32 maxDistToOpenPolygonInMeters;
      
      /**
       *    Empty constructor.
       *    Use setParameters methods to set the parameters.
       */
      ItemIdentifier();
      
      
      /**
       *    Constructor.
       *    @param   name  The name of the item.
       *    @param   type  The item type of the item.
       *    @param   lat   Latitude close or inside the item.
       *    @param   lon   Longitude close or inside the item.
       */
      ItemIdentifier( const char* name, 
                      ItemTypes::itemType type,
                      int32 lat, 
                      int32 lon );
      
      /**
       *    Constructor.
       *    @param   name        The name of the item.
       *    @param   type        The item type of the item.
       *    @param   insideItem  Name of an item that this item is inside,
       *                         either logically or graphically.
       */
      ItemIdentifier( const char* name, 
                      ItemTypes::itemType type,
                      const char* insideItem );

      /**
       *    Destructor.
       */
      virtual ~ItemIdentifier();

      
      /**
       *    @name Set-methods. To use if the empty constructor is used.
       *    Use either of the two set-methods.
       */
      //@{
         /**
          *    Set the parameters.
          *    @param   name  The name of the item.
          *    @param   type  The item type of the item.
          *    @param   lat   Latitude close or inside the item.
          *    @param   lon   Longitude close or inside the item.
          */
         void setParameters( const char* name,
                             ItemTypes::itemType type,
                             int32 lat,
                             int32 lon );

         
         /**
          *    @param   name        The name of the item.
          *    @param   type        The item type of the item.
          *    @param   insideItem  Name of an item that this item is 
          *                         inside, either logically or 
          *                         graphically.
          */
         void setParameters( const char* name,
                             ItemTypes::itemType type,
                             const char* insideItem );
      //@}
      
      /**
       *    @name Get-methods.
       */
      //@{
         /**
          *    @return  The name.
          */
         inline const char* getName() const;
         
         
         /**
          *    @return  The item type.
          */
         inline ItemTypes::itemType getItemType() const;
      
         
         /**
          *    The latitude of the item.
          *    Only valid if it is not equal to GfxConstants::IMPOSSIBLE.
          *    If not valid, the item will be identified by the name of
          *    an item that this item is inside.
          *    The coordinate must be inside (if open polygon) or
          *    within maxDistToOpenPolygonInMeters (if closed polygon).
          *
          *    @return  The latitude of the item.
          */
         inline int32 getLat() const;
         
         
         /**
          *    The longitude of the item.
          *    Only valid if getLat() is not equal to 
          *    GfxConstants::IMPOSSIBLE.
          *    If not valid, the item will be identified by the name of
          *    an item that this item is inside.
          *    The coordinate must be inside (if open polygon) or
          *    within maxDistToOpenPolygonInMeters (if closed polygon).
          *
          *    @return  The longitude of the item.
          */
         inline int32 getLon() const;
      
         
         /**
          *    Get the name of the item that this item is inside.
          *    The name may be NULL, and in that case the item will
          *    be identified by its coordinates instead.
          *    
          *    @return  The name or NULL.
          */
         inline const char* getInsideName() const;
      //@}
      
      
   private:
      
      /**
       *    The name.
       */
      char* m_name;

      /**
       *    The name.
       */
      ItemTypes::itemType m_itemType;

      /**
       *    Latitude of item. GfxConstants::IMPOSSIBLE if not valid.
       */
      int32 m_lat;

      /**
       *    Longitude of item. GfxConstants::IMPOSSIBLE if not valid.
       */
      int32 m_lon;

      /**
       *    Name of an item that the item is inside. NULL if not valid.
       */
      char* m_insideItem;

};

// ========================================================================
//                                      Implementation of inlined methods =

inline const char* 
ItemIdentifier::getName() const
{
   return m_name;
}

   
inline ItemTypes::itemType 
ItemIdentifier::getItemType() const
{
   return m_itemType;
}
      
         
inline int32 
ItemIdentifier::getLat() const
{
   return m_lat;
}
         

inline int32 
ItemIdentifier::getLon() const
{
   return m_lon;
}


inline const char* 
ItemIdentifier::getInsideName() const
{
   return m_insideItem;
}


#endif

