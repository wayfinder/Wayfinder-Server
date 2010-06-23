/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ALTEREDGFXLIST_H
#define ALTEREDGFXLIST_H

#include "config.h"
#include "CCSet.h"

class GfxData;

/**
 *  Class repesenting one routable item from a route and indicating
 *  which GfxData coordinates that should be disregarded or replaced when
 *  expanding the route to a navigator. 
 *
 */
class AlteredGfxLink : public Link
{
  public:
   /**
    *  Constructor.
    *  @param id The Id of the routable item the link belongs to.
    */
   AlteredGfxLink(uint32 id);

   /**
    * Accesses the Id of the item.
    * @return The Id of the item.
    */
   uint32 getLinkID() const
      { return m_id; }
   
   /**
    *  Mark the first coordinate of the items GfxData to be disregarded.
    */
   void removeFirstCoord()
      { m_removeFirst = true;}
   
   /**
    *  Mark the last coordinate of the items GfxData to be disregarded.
    */
   void removeLastCoord()
      { m_removeLast = true;}
   
   /**
    *  Add a new first coordinate to the item.
    *  @param lat The lat of the new coordinate.
    *  @param lon The lon of the new coordinate.
    */
   void addFirstCoord(int32 lat, int32 lon);

   /**
    *  Add a new last coordinate to the item.
    *  @param lat The lat of the new coordinate.
    *  @param lon The lon of the new coordinate.
    */
   void addLastCoord(int32 lat, int32 lon);

   /**
    *  Checks if the first coordinate of the items GfxData should  be
    *  disregarded.
    */
   bool getRemoveFirst()
      { return m_removeFirst; }
   
   /**
    *  Checks if the last coordinate of the items GfxData should  be
    *  disregarded.
    */
   bool getRemoveLast() 
      { return m_removeLast; }
   
   /**
    *  Return the new first coordinate if there is one.
    *  @param lat The lat of the new coordinate.
    *  @param lon The lon of the new coordinate.
    *  @return True if the new coordinate was set.
    */
   bool getFirstCoord(int32 &lat, int32 &lon);
   
   /**
    *  Return the new last coordinate if there is one.
    *  @param lat The lat of the new coordinate.
    *  @param lon The lon of the new coordinate.
    *  @return True if the new coordinate was set.
    */
   bool getLastCoord(int32 &lat, int32 &lon);

  private:
   /// Default constructor. Private to avoid use.
   AlteredGfxLink(){}
   
   /// The Id of the RoutableItem of this link.
   uint32 m_id;
   
   /**
    *  Flag indicating that the first coordinate of the GfxData should
    *  be disregarded.
    */
   bool m_removeFirst;
   
   /**
    *  Flag indicating that the last coordinate of the GfxData should
    *  be disregarded.
    */
   bool m_removeLast;

   /**
    *  New first coordinate lat.
    */
   int32 m_newFirstLat;
   
   /**
    *  New first coordinate lon.
    */
   int32 m_newFirstLon;
   
   /**
    *  New last coordinate lat.
    */   
   int32 m_newLastLat;
   
   /**
    *  New last coordinate lon.
    */   
   int32 m_newLastLon;
   
};

/**
 *  Class holding a list of modifications to the GfxData coordinates of the
 *  RoutableItems of a route to send to a navigator.
 */
class AlteredGfxList : public Head 
{
  public:
   /**
    *  Constructor. Creates a empty AlteredGfxList.
    */
   AlteredGfxList(){}
   

   /**
    *  Search the list for a link belonging to the itemId.
    *  @param itemId The Id of the searche item.
    *  @return Pointer to the link of the item.
    */
   AlteredGfxLink* getItemInList(uint32 itemId);

   /**
    *  Finds the Item in the list or creates a new link for the item.
    *  Marks it to remove the first cordinate calculates new first coord
    *  if needed.
    *  @param nodeId The Id of the first node of the item being passed.
    *  @param gfx The GfxData of the item.
    */
   void setRemoveLastCoordinate(uint32 nodeId, GfxData* gfx); 

   /**
    *  Finds the Item in the list or creates a new link for the item.
    *  Marks it to remove the last cordinate calculates new last coord
    *  if needed.
    *  @param nodeId The Id of the first node of the item being passed.
    *  @param gfx The GfxData of the item.
    */
   void setRemoveFirstCoordinate(uint32 nodeId, GfxData* gfx);

   /**
    *  If this method returns true, the first coordinate {\em of the GfxData}
    *  should not be sent to the navigator. If the outparameters newLat and
    *  newlon are set( not MAX_INT32). This new coordinate should be sent
    *  instead
    *  @param nodeId The Id of the first node of the item being passed.
    *  @param newLat The lat of the new coordinate to add first in the route.
    *  @param newlon The lon of the new coordinate to add first in the route.
    *  @return True if first coordinate should be removed.
    */
   bool removeFirstCoord(uint32 nodeId, int32 &newLat, int32 &newlon);

   /**
    *  If this method returns true, the first coordinate {\em of the GfxData}
    *  should not be sent to the navigator. If the outparameters newLat and
    *  newlon are set( not MAX_INT32). This new coordinate should be sent
    *  instead
    *  @param nodeId The Id of the first node of the item being passed.
    *  @param newLat The lat of the new coordinate to add last in the route.
    *  @param newlon The lon of the new coordinate to add last in the route.
    *  @return True if first coordinate should be removed.
    */
   bool removeLastCoord(uint32 nodeId, int32 &newLat, int32 &newlon);

  private:
   /**
    * Check the GfxData if the second coordinate from the end or begining
    * is more than 5 meters in. If so return the coordinates for the point
    * between the end/begining and the second pont, 5 meters from the
    * end/begining.
    * @param gfx The GfxData to check.
    * @param begining True if checking the beginning. False if the end.
    * @param newLat The output lat. MAX_INT32 if not set.
    * @param newLon The output lon. MAX_INT32 if not set.
    * @return True if a new coordinate was set.
    */
   bool checkNewCoord(GfxData* gfx, bool begining,
                      int32 &newLat, int32 &newLon);

};


#endif //ALTEREDGFXLIST_H
