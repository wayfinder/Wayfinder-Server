/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LOCATIONHASHTABLE_H
#define LOCATIONHASHTABLE_H

#include "config.h"
#include <set>

#include "Types.h"
#include "Vector.h"
#include "MC2BoundingBox.h"

/**
 *    Abstract class that represent a cell in the LocationHashTable. The 
 *    cell has functionality for finding an element at a specific location.
 *    Things to do in the subclasses:
 *    
 *       - The elementdata MUST be supplied by the subclass.
 *       - Support a method to add elements to the elementdata.  
 *         This method can be called from a subclass to LocationHashtable.
 *       - Update nbrElements and the min/max members when neccessary.
 *       - Implement the three methods that calculates the distance 
 *         by using the data at some index.
 *
*/
class LocationHashCell {
   public:
      /**
       *    Constructor. Doesn't do much for now.
       */
      LocationHashCell();

      /**
       *    Destructor.
       */
      virtual ~LocationHashCell();

      /**
       *    Sets the boundingbox-members.
       *
       *    @param hmin Smallest horizontal value of boundingbox.
       *    @param hmax Largest horizontal value of boundingbox.
       *    @param vmin Smallest vertical value of boundingbox.
       *    @param vmax Largest vertical value of boundingbox.
       */
      void setMC2BoundingBox(int32 hmin, int32 hmax, int32 vmin, int32 vmax);

      /**
       *    Get the maximum horizontal coordinate of the boundingbox.
       *
       *    @return The maximum horizontal.
       */
      inline int32 getMaxHorizontal() const;

      /**
       * Get the minimum horizontal coordinate of the boundingbox. Inlined.
       * 
       * @return the minimum horizontal
       */
      inline int32 getMinHorizontal() const;

      /**
       * Get the maximum vertical coordinate of the boundingbox. Inlined.
       * 
       * @return the maximum vertical
       */
      inline int32 getMaxVertical() const;

      /**
       * Get the minimum vertical coordinate of the boundingbox. Inlined.
       *
       * @return the minimum vertical
       */
      inline int32 getMinVertical() const;

      /**
       * Get the number of elements in this cell. Inlined.
       *
       * @return the number of elements.
       */
      inline uint32 getNbrElements() const;

      /**
       *    Find out if this cell contains any valid elements or not.
       *    Usefull if any subclass whants to implement functionallity
       *    to get the closest e.g. street.
       *    @return  True if this cell contains any valid element, false
       *             otherwise.
       */
      virtual bool containsValidElement() const;

      /**
       *    Find the element closest to (hpos, vpos).
       *
       *    @param hpos Horizontal position to match.
       *    @param vpos Vertical position to match.
       *    @param dist The squaredistance from (hpos, vpos) to the 
       *                closest element.
       *    @return The offset in the elementstructure.
       */
      uint32 getClosest(int32 hpos, int32 vpos, bool dummy, uint64 &dist, 
                        bool areaItem = false );

      /**
       *    Find the elements within the circle at (hpos, vpos) with 
       *    a given radius.
       *
       *    @param hpos     The horizontal position of the centre of
       *                    the circle.
       *    @param vpos     The vertical position of the centre of the 
       *                    circle.
       *    @param sqRadius The squared circle radius.
       *    @param proximityResult
       *                    The resulting itemID:s are added to this
       *                    vector. The vector is not emptied.
       */
      void getWithinRadius( int32 hpos, int32 vpos, uint64 sqRadius,
                            set<uint32>& proximityResult);

      /**
       *    Find the elements within the boundingbox.
       *    @param   bbox  The bounding box.
       *    @param   proximityResult The resulting elements are added
       *                             to the end of this vector.
       */
      void getWithinMC2BoundingBox(const MC2BoundingBox* bbox,
                                   Vector* proximityResult);
      
      /**
       *    Find the elements within the boundingbox.
       *    @param   bbox  The bounding box.
       *    @param   proximityResult The resulting elements are added
       *                             to the end of this vector.
       */
      void getWithinMC2BoundingBox(const MC2BoundingBox* bbox,
                                   set<uint32>& proximityResult);

      /**
       *    Get all items in this cell.
       *    @param proximityResult The resulting elements are added
       *                           to this vector.
       */
      void getAllItems(Vector* proximityResult);
      
      /**
       *    Get all items in this cell.
       *    @param proximityResult The resulting elements are added
       *                           to this vector.
       */
      void getAllItems(set<uint32>& proximityResult);
      
      /**
       *    Abstract method, used to get an identifier for an element 
       *    in the elementarray.
       *
       *    @param index The index in the elementarray.
       *    @return An identifier identifier, specified in the subclass.
       */
      virtual uint32 getItemID( uint32 index ) const = 0;

      /**
        *   Abstract method that checks if the identifier exists 
        *   in this cell and that it is ok to use it as a proximity
        *   result according to the settings of the cell and hashtable.
        * 
        *   @param   itemID   The identifier.
        *   @return  True if the identifier was allowed, false otherwise.
        */
      virtual bool isItemAllowed(uint32 itemID) = 0;

      /**
       *    Get the memoryusage for this object.
       *    @return The total memory usage for this object.
       */
      virtual uint32 getMemoryUsage() const;
    
   
   protected:
      /**
        *   Calculate the minimum distance from the boundingbox to 
        *   (hpos, vpos). Abstract method.
        *
        *   @param   index The array-offset for a hashelement in this cell.
        *   @param   hpos  The horizontal coordinate.
        *   @param   vpos  The vertical coordinate.
        *   @param   maxdist The maximum distance for which to calculate the dist.
        *   @return  The minimum distance to (hpos, vpos). NB! The unit
        *            of the distance can be chosen in the implemenatation 
        *            of subclasses, but must be the same in all distance 
        *            methods. Or MAX_UINT64 if not less than maxdist.
        */
      virtual uint64 minSquaredistToBoundingbox( uint32 index,
                                                 const int32 hpos,
                                                 const int32 vpos,
                                                 uint64 maxdist = MAX_UINT64) = 0;

      /**
        *   Calculate the maximum distance from the boundingbox to 
        *   (hpos, vpos). Abstract method.
        *
        *   @param   index The array-offset for a hashelement in this cell.
        *   @param   hpos  The horizontal coordinate.
        *   @param   vpos  The vertical coordinate.
        *   @return  The maximum distance to (hpos, vpos). NB! The unit
        *            of the distance can be chosen in the subclasses, but
        *            must be the same in all distance methods.
        */
      virtual uint64 maxSquaredistToBoundingbox( uint32 index,
                                                 const int32 hpos,
                                                 const int32 vpos ) = 0;

      /**
        *   Calculate the closest distance from the element to 
        *   (hpos, vpos). Abstract method.
        *
        *   @param   index The array-offset for a hashelement in this cell.
        *   @param   hpos  The horizontal coordinate.
        *   @param   vpos  The vertical coordinate.
        *   @return  The minimum distance to (hpos, vpos). NB! The unit
        *            of the distance can be chosen in the subclasses, but
        *            must be the same in all distance methods.
        */
      virtual uint64 minSquaredist( uint32 index,
                                    const int32 hpos,
                                    const int32 vpos ) = 0;
   
      /**
       *    Checks if an item is inside (part of or the whole item) 
       *    the specified boundingbox. Abstract method.
       *    @param   index The index of the item to check.
       *    @param   bbox  The bounding box.
       *    @return  True if part or the whole item was inside the bounding
       *             box.
       */
      virtual bool insideMC2BoundingBox( uint32 index,
                                         const MC2BoundingBox* bbox) = 0;
      
      /**
       *    @param index   
       *    @param hpos    
       *    @param vpos    
       *
       */
      virtual bool isInsidePolygon( uint32 index,
                                    const int32 hpos,
                                    const int32 vpos );
      
      /**
       *    The number of elements in this cell.
       */
      uint32 m_nbrElements;

      /**
       *    The maximum vertical coordinate of the boundingbox.
       */
      int32 m_topVertical;

      /**
       *    The minimum vertical coordinate of the boundingbox.
       */
      int32 m_bottomVertical;

      /**
       *    The maximum horizontal coordinate of the boundingbox.
       */
      int32 m_rightHorizontal;

      /**
       *    The minimum horizontal coordinate of the boundingbox.
       */
      int32 m_leftHorizontal;
};


/**
 *    Class that represent a hashtable for elements with a location. 
 *    The class is intended to speed up the time to find an element at 
 *    a specific location. Things to do in the subclasses:
 *    
 *    - Support a method that adds an element to the hashcells.
 *    - Support a method that builds the hashtable from scratch including 
 *          allocating memory for m_hashCell since the LocationHashCell is 
 *          abstract.
 *    - It's convenient to have a method that returns a pointer to an 
 *          element when calling method getClosest() instead of the hashcell 
 *          and an offset.
 *
*/
class LocationHashTable {
public:
   LocationHashTable();

      /**
       *    Constructor, calculates the parameters of the 
       *    hashingfunction by using the parameters.
       *
       *    @param hmin Minimum horizontal coordinate of the boundingbox.
       *    @param hmax Maximum horizontal coordinate of the boundingbox.
       *    @param vmin Minimum vertical coordinate of the boundingbox.
       *    @param vmax Maximum vertical coordinate of the boundingbox.
       *    @param nbrVerticalZells The preferred number of vertical cells.
       *    @param nbrHorizontalZells Preferred number of horizontal cells.
       */
      LocationHashTable(int32 hmin, int32 hmax, int32 vmin, int32 vmax,
         uint32 nbrVerticalZells, uint32 nbrHorizontalZells);
      
      /**
        *   Destructor, deletes allocated memory.
        */
      virtual ~LocationHashTable();

      /**
        *   Finds an element closest to (hpos, vpos).
        *
        *   @param hpos    Horizontal position to match.
        *   @param vpos    Vertical position to match.
        *   @param offset  Where in the datastructure the element can be 
        *                  found.
        *   @param dist    Distance to the closest item. The unit of this
        *                  distance depends on the unit returned by the
        *                  virtual distance methods.
        *   @return  The cell where the element can be found. if no element 
        *            is found NULL is returned, this is however extremely 
        *            rare.
        */
      LocationHashCell* getClosest( int32 hpos, int32 vpos, uint32 &offset,
                                    uint64 &dist, bool areaItem = false );

      /**
        *   Finds all elements inside the circle defined by (hpos, vpos) 
        *   and radius.
        *   @param   hpos     The horizontal part of the coordinate for
        *                     the center of the circle.
        *   @param   vpos     The vertical part of the coordinate for the
        *                     center of the circle.
        *   @param   radius   The maximum distance from (hpos, vpos) to the
        *                     items returned by this method. The uint on the
        *                     radius depends on the unit returned by the
        *                     virtual distance methods, implemented in the
        *                     subclasses.
        *   @param   shouldKill  Is set to true if the Vector* returned
        *                     by this methos should be deleted by the 
        *                     caller. Set to false if the caller @b must 
        *                     @b not delete the Vector.
        *   @return  Pointer to a Vector containing IDs of the items within
        *            a distance < radius from (hpos, vpos). NB! This 
        *            @b must be deleted if shouldKill-parameter is true, 
        *            and @b must @b not be deleted if that parameter is 
        *            false.
        */  
      Vector* getAllWithinRadius(int32 hpos, int32 vpos, 
                                 int32 radius, bool &shouldKill);

      /**
       *   Finds all elements inside the circle defined by (hpos, vpos) 
       *   and radius. This version does not copy as much as the one above.
       *   @param   ids      The id:s of the found items are put here.
       *   @param   hpos     The horizontal part of the coordinate for
       *                     the center of the circle.
       *   @param   vpos     The vertical part of the coordinate for the
       *                     center of the circle.
       *   @param   radius   The maximum distance from (hpos, vpos) to the
       *                     items returned by this method. The uint on the
       *                     radius depends on the unit returned by the
       *                     virtual distance methods, implemented in the
       *                     subclasses.
       */
      void getAllWithinRadius(set<uint32>& ids,
                              int32 hpos, int32 vpos, 
                              int32 radius);
                              

      /**
        *   Finds all elements inside the boundingbox. 
        *   @param   bbox        The boundingbox.
        *   @param   shouldKill  Is set to true if the Vector* returned
        *                        by this method should be deleted by the 
        *                        caller. Set to false if the caller 
        *                        @b must @b not delete the Vector.
        *   @return  Pointer to a Vector containing IDs of the items within
        *            the bounding box. 
        *   @remark  The returned vector must be deleted if 
        *            shouldKill-parameter is true, and must not be deleted 
        *            if that parameter is false.
        */  
      Vector* getAllWithinMC2BoundingBox(const MC2BoundingBox* bbox,
                                         bool& shouldKill);

      /**
       *    Finds all elements inside the bounding box and puts
       *    them into the set.
       *    @param ids  Set to add the item ids to.
       *    @param bbox Bounding box to look inside.
       */
      void getAllWithinMC2BoundingBox(set<uint32>& ids,
                                      const MC2BoundingBox& bbox);

      /**
       *    Find out the memory usage for this object.
       *    @return The number of bytes used by this object.
       */
      virtual uint32 getMemoryUsage() const;
      
   protected:

      /**
       *    This method does some of the dirty work in getClosest(). It's 
       *    used to get neighbour-cells that's close to (vpos, hpos). 
       *    If a neighbourcell is closer than the closest element in 
       *    cell[v][h] the neibours cell-index and squaredistance will 
       *    be stored.
       *
       *    @param elmDist Distance to closest element in cell[v][h].
       *    @param vpos    The vertical position to find elements at.
       *    @param hpos    The horizontal position to find elements at.
       *    @param v       The vertical cell-index where vpos lies.
       *    @param h       The horizontal cell-index where hpos lies.
       *    @param dv      Vertical offset to neighbour.
       *    @param dh      Horizontal offset to neighbour.
       *    @param vIndex  Vertical cell-index of close neighbours.
       *    @param hIndex  Horizontal cell-index of close neighbours.
       *    @param sqDist  Squaredistance to neighbours.
       *    @param nbr     The number of neighbours with a distance to 
       *                   (vpos, hpos)less than elmDist.
       */
      void addToNeighbours(uint64 elmDist, int32 vpos, int32 hpos,
                           int32 v, int32 h, int32 dv, int32 dh,
                           uint32 vIndex[], uint32 hIndex[], 
                           uint64 sqDist[], int32 &nbr);

      /**
       *    Converts a coordinate to hashindexes.
       *
       *    @param hpos    Horizontal position to convert.
       *    @param vpos    Vertical position to convert.
       *    @param hHash   Outparameter, set to the calculated 
       *                   horizontal hashindex.
       *    @param vHash   Outparameter, set to the calculated 
       *                   vertical hashindex.
       */
      void getHashIndex(int32 hpos, int32 vpos,
                        uint32 &hHash, uint32 &vHash) const;
      /**
       * @see getHashIndex.
       * Doesn't check for errors in release build.
       */
      inline void quickGetHashIndex(int32 hpos, int32 vpos,
                                    uint32 &hHash, uint32 &vHash) const;
      
      /**
       *    Converts hashindexes to a coordinate.
       *
       *    @param hpos    Outparameter, set to the calculated 
       *                   horizontal position.
       *    @param vpos    Outparameter, set to the calculated 
       *                   vertical position.
       *    @param hHash   The horizontal hashindex to convert.
       *    @param vHash   The vertical hashindex to convert.
      */
      void invHashIndex(int32 &hpos, int32 &vpos,
                        uint32 hHash, uint32 vHash);

      /**
       *    Checks if a cell is overlapped by a circle. This is useful 
       *    for applications such as proximitysearch etc.
       *
       *    @param cell 
       *    @param hpos 
       *    @param vpos 
       *    @param sqR  
       *    @return True if the circle overlaps, false otherwise.
      */
      bool isCellWithinRadius(LocationHashCell* cell, int32 hpos, 
                              int32 vpos, uint64 sqR );

      /**
       *    Merges the vectors from the proximitysearch to ensure that 
       *    the result has no doubles. The <code>proximityResult</code>
       *    cannot be used after this function has been called.
       *    @param proximityResult The result to remove dups from.
       *    @return  A Vector with the result from the merge. To be
       *             deleted by the user.
       */
      Vector* removeDups(Vector* proximityResult);

      /**
       *    Some coordinate-systems are scaled horizontally.
       *
       *    @return The horizontalFactor, will return 1.0 if not 
       *            implemeted in subclass.
       */
      virtual double getHorizontalFactor();
      
      /**
       *    The number of cells vertically.
       */
      uint32 m_nbrVerticalCells;

      /**
       *    The number of cells horizontally.
       */
      uint32 m_nbrHorizontalCells;

      /**
       *    The cells as a two-dimensional array.
       *    @remark A hashcell is retrieved with 
       *            @c m_hashCell[vertical][horizontal].
       */
      LocationHashCell*** m_hashCell;

      /**
       *    Variable for the vertical hashfunction.
       */
      uint8 m_verticalShift;

      /**
       *    Variable for the horizontal hashfunction.
       */
      uint8 m_horizontalShift;

      /**
       *    @name Part of the boundingbox
       */
      //@{
         /// Latitude, y 
         int32 m_topVertical;
         
         /// Latitude, y
         int32 m_bottomVertical;  
         
         /// Longitude, x
         int32 m_rightHorizontal; 
         
         /// Longitude, x
         int32 m_leftHorizontal;  
      //@}
};

// =======================================================================
//                                     Implementation of inlined methods =

inline int32 
LocationHashCell::getMaxHorizontal() const
{ 
   return m_rightHorizontal; 
}

inline int32 
LocationHashCell::getMinHorizontal() const
{ 
   return m_leftHorizontal; 
}

inline int32 
LocationHashCell::getMaxVertical() const
{ 
   return m_topVertical; 
}

inline int32 
LocationHashCell::getMinVertical() const
{ 
   return m_bottomVertical; 
}

inline uint32 
LocationHashCell::getNbrElements() const
{ 
   return m_nbrElements; 
}

inline void 
LocationHashTable::quickGetHashIndex(int32 hpos, int32 vpos,
                                     uint32 &hHash, uint32 &vHash) const
{
   vHash = ( vpos - m_bottomVertical ) >> m_verticalShift;
   hHash = ( hpos - m_leftHorizontal ) >> m_horizontalShift;
#ifdef _DEBUG
   uint32 v,h;
   getHashIndex(hpos, vpos, h, v);
   almostAssert(vHash == v);
   almostAssert(hHash == h);
#endif
}

#endif

