/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDEDROUTEMATCH_H
#define EXPANDEDROUTEMATCH_H

#include "config.h"
#include "ItemTypes.h"
#include "Name.h"

class Packet;

/**
 *    Describes a route origin or destination.
 *
 */
class ExpandedRouteMatch {
   public:
      /**
       * Creates a new ExpandedRouteMatch.
       *
       * @param name The name of the match, is copied.
       * @param mapID The mapID of the match.
       * @param itemID The itemID of the match.
       * @param type The type of the match.
       * @param lat The latitude of the match.
       * @param lon The longitude of the match.
       * @param houseNbrDir The direction of the route at match in terms
       *                    of increasing or decreasing housenumbers.
       * @param houseNbrOdd The direction of the route at match in terms
       *                    of odd housenumbers on the left or right side.
       * @param houseNbr The house number at match, 0 means unknown.
       * @param angle The angle of the route at match, MAX_UINT16 means
       *              unknown.
       */
      ExpandedRouteMatch( const Name* name, 
                          uint32 mapID, uint32 itemID,
                          ItemTypes::itemType type,
                          int32 lat, int32 lon,
                          ItemTypes::routedir_nbr_t houseNbrDir,
                          ItemTypes::routedir_oddeven_t houseNbrOdd,
                          uint32 houseNbr, uint16 angle );


      /**
       * Destructor.
       */
      virtual ~ExpandedRouteMatch();


      /**
       * Prints content to out stream.
       *
       * @param out the ostream to print to.
       */
      void dump( ostream& out ) const;


      /**
       * Get the name of the match.
       *
       * @return The name of the match.
       */
      const Name* getName() const;


      /**
       * Get the mapID of the match.
       *
       * @return The mapID of the match.
       */
      uint32 getMapID() const;


      /**
       * Get the itemID of the Match.
       *
       * @return The itemID of the Match.
       */
      uint32 getItemID() const;


      /**
       * Get the type of item.
       * 
       * @return The type of item.
       */
      ItemTypes::itemType getType() const;


      /**
       * Set the type of item.
       * 
       * @param type The type of item.
       */
      void setType( ItemTypes::itemType type );


      /**
       * Get the latitude.
       *
       * @return The latitude.
       */
      int32 getLat() const;


      /**
       * Set the latitude.
       *
       * @param lat The latitude.
       */
      void setLat( int32 lat );


      /**
       * Get the longitude.
       *
       * @return The longitude.
       */
      int32 getLon() const;


      /**
       * Set the longitude.
       *
       * @param lat The longitude.
       */
      void setLon( int32 lon );


      /**
       * Get the direction counting house numbers.
       *
       * @return The direction counting house numbers.
       */
      ItemTypes::routedir_nbr_t getDirectionHousenumber() const;


      /**
       * Get the direction with odd or even house numbers.
       *
       * @return The direction with odd or even house numbers.
       */
      ItemTypes::routedir_oddeven_t getDirectionOddEven() const;


      /**
       * Get the house number at match.
       *
       * @return The house number at match, 0 means unknown.
       */
      uint32 getHousenumber() const;


      /**
       * Get the angle at match. The angle is in degrees and
       * calculated clockwise from the north-direction.
       *
       * @return The angle at match, MAX_UINT16 means unknown.
       */
      uint16 getAngle() const;
                          

      /**
       * Set the name of the match.
       *
       * @param name The new name of the match.
       */
      void setName( const Name* name );


      /**
       * Set the mapID of the match.
       *
       * @param mapID The new mapID of the match.
       */
      void setMapID( uint32 mapID );


      /**
       * Set the itemID of the Match.
       *
       * @param itemID The new itemID of the Match.
       */
      void setItemID( uint32 itemID );


      /**
       * Set the direction counting house numbers.
       *
       * @param houseNbrDir The new direction counting house numbers.
       */
      void setDirectionHousenumber( 
         ItemTypes::routedir_nbr_t houseNbrDir );


      /**
       * Set the direction with odd or even house numbers.
       *
       * @param houseNbrOdd The new direction with odd or even house 
       *                    numbers.
       */
      void setDirectionOddEven( 
         ItemTypes::routedir_oddeven_t houseNbrOdd );


      /**
       * Set the house number at match.
       *
       * @param houseNbr The new house number at match, 0 means unknown.
       */
      void setHousenumber( uint32 houseNbr );


      /**
       * Set the angle at match. The angle is in degrees and
       * calculated clockwise from the north-direction.
       *
       * @param angle The angle at match, MAX_UINT16 means unknown.
       */
      void setAngle( uint16 angle );

      /**
       * Write this ExpandedRoute  as a byte buffer.
       *
       * @param p    The packet where the ExpandedRoute will be saved.
       * @param pos  Position in p, will be updated when calling this 
       *             method.
       */
      uint32 save( Packet* p, int& pos ) const;

      /**
       * Get the size of this when stored in a byte buffer.
       *
       * @return  The number of bytes this will use when stored 
       *          in a byte buffer.
       */
      uint32 getSizeAsBytes() const;

      /**
       * Set all members with data from the buffer.
       *
       * @param p    The packet where the ExpandedRoute will be loaded 
       *             from.
       * @param pos  Position in p, will be updated when calling this 
       *             method.
       */
      void load( Packet* p, int& pos );

   private:
      /**
       * The name object.
       */
      Name m_name;


      /**
       * The mapID.
       */
      uint32 m_mapID;


      /**
       * The itemID.
       */
      uint32 m_itemID;


      /**
       * The type.
       */
      ItemTypes::itemType m_type;


      /**
       * The latitude.
       */
      int32 m_lat;


      /**
       * The longitude.
       */
      int32 m_lon;


      /**
       * The house number dir.
       */
      ItemTypes::routedir_nbr_t m_houseNbrDir;


      /**
       * The oddeven dir.
       */
      ItemTypes::routedir_oddeven_t m_houseNbrOdd;


      /**
       * The house number.
       */
      uint32 m_houseNbr;


      /**
       * The angle.
       */
      uint16 m_angle;
};


#endif // EXPANDEDROUTEMATCH_H

