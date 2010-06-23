/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
  compile with -fno-default-inline if possible
  
 */

#include "VectorElement.h"

#define MAX_SAFE_VECTORSIZE 1024
/**
  *   This class template is a collection of functions from
  *   Vector, ObjVector and VoidVector.
  *   (PointerVector is not included since that is never used.)
  *
  */
class TemplateVector
{
public:
      /** Forward iterator */
      typedef ElementType* iterator;
   
      /** Constant forward iterator */
      typedef const ElementType* const_iterator;
      typedef ElementType value_type;
      /**
       *   Creates an empty TemplateVector.
       */
      inline TemplateVector();
         
      /**
       *   Creates an vector with specifications according to the
       *   parameters. If the vector needs to be reallocated it the
       *   size of the vector is doubled.
       *
       *   @param   nbrItems The size of the vector that is allocated.
       */
      inline TemplateVector(uint32 nbrItems);
         
      /**
       *   Creates an vector with specifications according to the
       *   parameters.
       *   @param   nbrItems     The size of the vector that is allocated.
       *   @param   increaseSize When the size of the internal vector 
       *            is increased it is done with this number of items.
       *            If set to 0, the size of the vector is doubled each
       *            time it is resized.
       */
      inline TemplateVector(uint32 nbrItems, uint32 increaseSize);

      /**
       *    Copy constructor.
       *    @param   v  The object that should be used as template.
       */
      inline TemplateVector( const TemplateVector& v );
         
      /**
       *   Destructor for the TemplateVector-objects. Observe that the
       *   elements in the vector {\bf not} are deleted (since they
       *   not are created  in this object).
       */
      inline virtual ~TemplateVector();

      /**
       *   Reallocates the vector, so that the nbrItems is newNbrItems.
       *   If the already allocatedsize is bigger than newNbrItems
       *   nothing is done.
       *   @param newNbrItems The new size of this Vector.
       */
      inline void setAllocSize(uint32 newNbrItems);
         
      /**
       *    Trims the capacity of this vector to be the vector's current 
       *    size.
       *    @return The number of removed elements.
       */
      inline int trimToSize();
         
      /**
       *    Adds one element last in the array. If theres no room left
       *    in the allocated array a new one is created.
       *
       *    @return  The index of the added item, MAX_UINT32 upon failure.
       */
      inline uint32 addLast(ElementType elm);

      /**
       *    Adds one element last in the array. If theres no room left
       *    in the allocated array a new one is created.
       */
      inline void push_back(ElementType elm);
   
      /**
       *    Adds one element last in the array if the element is not
       *    already in the TemplateVector. If theres no room left
       *    in the allocated array a new one is created.
       *
       *    @param   elm   The element to insert into this array.
       *    @param   print If false, do not print "element already present".
       *    @return  The index of the added item, MAX_UINT32 if the new
       *             element was not added.
       */
      inline uint32 addLastIfUnique(ElementType elm, bool print = false);

      /**
       *    Removes the element elm from the vector.
       *    @param   elm   The element to remove.
       *    @return  True if the element was removed, false otherwise.
       */
      inline bool remove(ElementType elm);

      /**
       *    Removes all elements that are 'InvalidElement'.
       *    @return The number of elements removed.
       */
      inline uint32 removeInvalidElements();

      /**
       *    Remove all duplicate elements in this array. The result is 
       *    a sorted array with unique elements.
       */
      inline void removeDoubles();

      /**
       *   Returns a pointer to the internal buffer. Valid as long as
       *   the Vector isn't changed.
       *   @return  Pointer to the internal buffer. {\it NB! This must
       *            not be changed!}
       */
      inline const ElementType* getBuffer() const;
     
      /**
       *    Reset the vector. No elements are removed from the Vector, but
       *    is is considered empty.
       */
      inline void reset();

      /**
       *    Reset the vector. No elements are removed from the Vector, but
       *    is is considered empty.
       */
      inline void clear();

      /**
       *    Get the size of this Vector.
       *    @return  The number of elements in the vector (lastUsed).
       */
      inline uint32 getSize() const;

      /**
       *    Returns the size of the Vector. The same as in stl.
       *    @return The number of elements in the vector.
       */
      inline uint32 size() const;

      /**
       *    Returns true if the Vector is empty.
       *    @return True if the vector is empty.
       */
      inline bool empty() const;
   
      /**
       *    Get the number of allocated positions in this array.
       *    @return The allocated size of this vector.
       */
      inline uint32 getAllocSize() const;

      /**
       *    Get the element at position pos. {\it {\bf NB!} No check is done 
       *    against writing outside the buffer!}
       *    @param   pos   The position of the element to return.
       */
      inline ElementType operator[](uint32 pos) const;
         
      /**
       *    Get the element at position pos. {\it {\bf NB!} No check is done 
       *    against writing outside the buffer!}
       *    @param   pos   The position of the element to return.
       */
      inline const TemplateVector& operator=( const TemplateVector& v );

      /**
       *    Set or get the element at position pos. {\it {\bf NB!} No check 
       *    is done against writing outside the buffer!}
       *    @param   pos   The position of the element to return.
       */
      inline ElementType &operator[](uint32 pos);

      /**
       *    Get the element at position pos which has previously been used and thus is valid and initialized. {\it {\bf NB!} No check 
       *    is done in release mode against reading outside the buffer!}
       *    @param pos   The position of the element to return.
       *    @return The element at position pos.  
	   */
      inline ElementType& getUsedElementAt(uint32 pos) const;
        
      /**
       *    Get the element at position pos. {\it {\bf NB!} No check 
       *    is done against reading outside the buffer!}
       *    @param pos   The position of the element to return.
       *    @return The element at position pos.  
       */
      inline ElementType getElementAt( uint32 pos ) const;

      /**
       *    Get the element at position pos. If pos is outside the buffer
       *    the InvalidElement is returned.
       *    @param pos The position of the element to return.
       *    @return The element at position pos. The InvalidElement is
       *            returned if pos is outside the buffer.
       */
      inline ElementType getSafeElementAt(uint32 pos) const;

      /**
       *    Get the last element in this Vector.
       *    @return  The last element in this Vector. InvalidElement is 
       *             returned upon error.
       */
      inline ElementType getLast() const;

      /**
       *    Set the element at a given position to a new value. Elements
       *    in the Vector with higher index are not affected.
       *    {\it {\bf NB!} No check is done against writing outside the 
       *    buffer!}
       *
       *    @see  insertElementAt
       *    @param   pos   The position of the new value.
       *    @param   elm   The element to insert into this Vector.
       */
      inline void setElementAt( uint32 pos, ElementType elm);

      /**
       *    Set the element at a given position to a new value. Elements
       *    in the Vector with higher index are not affected.
       *    {\it {\bf NB!} No check is done against writing outside the 
       *    buffer in release mode!}
       *
       *    @see  insertElementAt
       *    @param   pos   The position of the new value.
       *    @param   elm   The element to insert into this Vector.
       */
	  inline void setUsedElementAt( uint32 pos, ElementType elm);
   
      /**
       *    Set the element at a given position to a new value. Elements
       *    in the Vector with higher index are not affected.
       *    If the position is outside the buffer the vector will be 
       *    reallocated to include default elements up to pos.
       *    @param pos The position of the new value.
       *    @param elm The element to insert into this Vector.
       */
      inline bool setSafeElementAt(uint32 pos, ElementType elm);

   /**
    * Inserts an element in a sorted array at it's correct position.
    * Not optimal yet (no need yet).
    * @param value The value to insert.
    */
   inline void insertElementSorted(ElementType value);
   
      /**
       *   Insert an element at index. The elements that are located at
       *   higher index are moved up, so no element will disapear.
       *
       *   @see   setElementAt
       *   @param value   The value to insert.
       *   @param index   The position to put it at.
       */
      inline void insertElementAt(uint32 idx, ElementType value);

      /**
       *   Removes element at idx. The elements that are located at
       *   higher index are moved down, so there will be no "hole"
       *   in the array after the removal.
       *
       *   @param   idx    The position of the element to delete.
       *   @return  True if the element is removed, false otherwise.
       */
      inline bool removeElementAt(uint32 idx);

      /**
       *    Removes the element at it.
       *
       */
      inline iterator erase(iterator it);
   
      /**
       *   Performs a linearsearch for an element with the same key
       *   as the parameter.
       *   
       *   @return  Idx whose key
       *            is equal equal to the parameters.
       */
      inline uint32 linearSearch(const ElementType keyElm) const;

   /**
    *  Performs a linearsearch for an element with the same key
    *  as the parameter.
    *
    *  @param keyElm The element to find.
    *  @return Pointer to the element equal to keyElm.
    */
   inline ElementType linearGet(const ElementType keyElm) const;

//   #ifndef UINT32
   /**
    *  @see binarySearch.
    *  @return The element at the position returned by the other binarys.
    */
   inline ElementType binaryGet(const ElementType KEY,
                                uint32 start = 0,
                                uint32 stop  = MAX_UINT32) const;
//   #endif
   
      /**
       *   Performs a binarysearch for an element with the same key
       *   as the parameter.
       *   The vector has to be sorted when using this function.
       *   If it's not sorted -- use the linearSerach()-method.
       *
       *   @param   keyElm   The value to search for.
       *   @param   start    Optional parameter, indicating the index
       *                     of the first element to search among.
       *                     Default value is 0.
       *   @param   stop     Optional parameter, indicating the index
       *                     of the last element to search among.
       *                     Default value is indexToUse-1.
       *   @return  Offset in array to an element with key == key.
       *            InvalidElement is returned if no such element in array.
       */
      inline uint32 binarySearch(const ElementType keyElm, 
                                 uint32 start = 0,
                                 uint32 stop  = MAX_UINT32) const;

      /**
       *   Performes a binarysearch and also returns the index
       *   of the found element
       *   @param   index    Output paramater containing the index 
       *                     of the found element.
       *   @param   keyElm   The element to search for.
       *   @param   start    Optional parameter, indicating the index
       *                     of the first element to search among.
       *                     Default value is 0.
       *                     of the last element to search among.
       *                     Default value is indexToUse-1.
       *   @param   stop     Optional parameter, indicating the index
       *   @return  Pointer to the found ElementType
       */
/*      inline ElementType binarySearch(uint32& index, 
                                      ElementType keyElm,
                                      uint32 start = 0,
                                      uint32 stop = MAX_UINT32);
*/

      /**
       *    Sorts the vector. Calls the {\bf private} quickSort-method
       *    with 0 and indexToUse-1 as parameters.
       *    @return  True if the search has been done, false otherwise.
       *             A returning false indicates that the virtual
       *             operators hasn't been implemented.
       */
      inline bool sort();

      /**
       *    Select and sort the i lowest elements of the vector.
       *    @param   start    Lower interval limit.
       *    @param   stop     Higher interval limit (normally getSize()).
       *    @param   count    The number of elements to sort.
       */
      inline void sortSmallest( int32 start, int32 stop, uint32 count );

      /**
       *    Reverses the elements of the vector.
       */
      inline void reverse();

      /**
       *    Prints the contents of the vector using toString() of the 
       *    ElementType.
       */
      inline void debugPrintVector();

      /**
       *    Deletes all ElementTypes and resets the vector.
       *    (Deletes objects in reverse order).
       *    WARNING: The destructor will probably not be called for
       *    objects put into a VoidVector since it doesn't know if
       *    it points to an object or not.
       */
      inline void deleteAllObjs();

      /**
       *    Get the size of the memory used by this object.
       *    @return The memoryusage in bytes for this object.
       */
      inline virtual uint32 getMemoryUsage() const;

      /**
       *   Prints the contents in the vector to standard out. This 
       *   method is not  implemented under win32.
       *   
       *   @param   singleLine  If this optional parameter is set to true
       *                        all items in this Vector are printed on a
       *                        single line, otherwise one item per line.
       *                        Default value is false.
       */
      inline void dump(bool singleLine = false) const;

     /**
      *    Where forward iterators start.
      */
     inline iterator begin() const;

     /**
      *    Where forward iterators should not be.
      */
     inline iterator end() const;
   
   protected:
      /**
       *    The size of the allocated array.
       */
      uint32 m_allocSize;

      /**
       *    The number of items used in the array
       */
      uint32 m_indexToUse;

      /**
       *    The number of items the size of the array is increased with
       *    when full. If set to 0, the size of the array is doubled when
       *    full.
       */
      uint32 m_increaseSize;

      /**
       *    The actual array.
       */
      ElementType *m_buf;

   private:
      /**
       *    Reallocates the array to a bigger one.
       */
      inline void reallocate();
         
      /**
       *    Performes the search in the vector. Uses recursive quickSort.
       *    This method is deprecated and should not be used!
       *    @param start  The index of the elemnent where to start sort.
       *    @param stop   The index of the elemnent where to stop sort.
       */
      inline void quickSort(uint32 start, uint32 stop);

      /**
       *    Performes a search in the vector. Uses non recursive 
       *    quickSort, (O(nlog n)).
       *    This method is called from the public sort-method
       */
      inline void nonRecursiveQuickSort();

      /**
       *    Partitions the elements between start and stop.
       *    @param start lower interval limit
       *    @param stop  upper interval limit
       *    @return The index of the element separating the two partitions.
       */
      inline uint32 partition( uint32 start, uint32 stop );

      /**
       *    Selects a random element in an interval of
       *    the vector as pivot element and partitions the array.
       *    @param   start Lower interval limit.
       *    @param   stop  Higher interval limit.
       *    @return  The index of the element separating the
       *             two partitions.
       */
      inline uint32 randomizedPartition( uint32 start, uint32 stop );

      /**
       *    Selects the count:th smallest element from the array
       *    and returns its vector index.
       *    @param   start Lower interval limit.
       *    @param   stop  Higher interval limit.
       *    @param   rank  The rank of the element to find.
       *    @return  The index of the rank:th smallest element.
       */
      inline uint32 randomizedSelect( uint32 start, uint32 stop, uint32 rank );
            
      /**
       *    @name Methods used by the ".src-files".
       *    Theses methods are used by the binarySearch() and sort()
       *    methods.
       */
      //@{
         /**
          *    Compare two elements at two indices.
          *    @param   a  Index of the first element in comparison.
          *    @param   b  Index of the secon element in comparison.
          *    @return  -1 if buf[a] < buf[b]; 0 if buf[a] == buf[b]
          *            and 1 if buf[a] > buf[b].
          */
         inline int COMP(uint32 a, uint32 b) const;

         /**
          *    Compare one element with another that is located at a
          *    given index.
          *    @param   key   Value to compare with.
          *    @param   pos   Index of the other element in comparison.
          *    @return  -1 if key < buf[pos]; 0 if key == buf[pos]
          *             and 1 if key > buf[pos].
          */
         inline int SEARCH_COMP(const ElementType key, uint32 pos) const;

         /**
          *    Method that changes places of element at position
          *    a and b.
          *    @param   a  Index of the one element.
          *    @param   b  Index of the other element.
          */
         inline void SWAP(int a, int b);

      //@}
};


/*
 *         \\ \\
 *       - // //
 *     -- //_//__     INLINES
 *   --- /____)__)          
 * ......00000000.............
 */


TemplateVector::TemplateVector()
{
   m_buf = NULL;
   m_allocSize = 0;
   m_indexToUse = 0;
   m_increaseSize = 0; // double for each resize
}

TemplateVector::TemplateVector(uint32 nbrItems)
{
   if (nbrItems == 0) {
	   m_buf = NULL;
	} else {
      m_buf = new ElementType[nbrItems];
	}
   m_allocSize = nbrItems;
   m_indexToUse = 0;
   m_increaseSize = 0; // double for each resize
}

TemplateVector::TemplateVector(uint32 nbrItems, uint32 increase)
{
   if (nbrItems == 0) {
	   m_buf = NULL;
	} else {
      m_buf = new ElementType[nbrItems];
	}
   m_allocSize = nbrItems;
   m_indexToUse = 0;
   this->m_increaseSize = increase;
}

TemplateVector::~TemplateVector()
{
/*
   // check for inefficient uses of vector
   if ((m_indexToUse > 1) && (m_allocSize > (m_indexToUse*10))) {
      // might have allocated too much
      almostAssert(false);
   }*/
   delete [] (ElementType*)m_buf;
}

TemplateVector::TemplateVector( const TemplateVector& v )
{
   m_allocSize = v.m_allocSize;
   m_indexToUse = v.m_indexToUse;
   m_increaseSize = v.m_increaseSize;
   if (v.m_allocSize > 0) {
      m_buf = new ElementType[v.m_allocSize];
      for (uint32 i = 0; i < m_indexToUse; i++) {
         m_buf[i] = v.m_buf[i];
      }
   }
   else {
      m_buf = NULL;
   }
}

const TemplateVector& TemplateVector::operator=( const TemplateVector& v )
{
	if( &v != this ) {
		delete [] m_buf;

		m_allocSize = v.m_allocSize;
		m_indexToUse = v.m_indexToUse;
		m_increaseSize = v.m_increaseSize;
		if (v.m_allocSize > 0) {
			m_buf = new ElementType[v.m_allocSize];
			for (uint32 i = 0; i < m_indexToUse; i++) {
            m_buf[i] = v.m_buf[i];
			}
		} else {
         m_buf = NULL;
		}
	}
   return *this;
}

const ElementType* 
TemplateVector::getBuffer() const 
{
   return const_cast<const ElementType*>(m_buf);
}
     
void 
TemplateVector::reset() 
{ 
   m_indexToUse = 0; 
}

void
TemplateVector::clear()
{
   reset();
}

uint32 
TemplateVector::getSize() const {
   return m_indexToUse;
}

uint32
TemplateVector::size() const
{
   return getSize();
}

bool
TemplateVector::empty() const
{
   return getSize() == 0;
}

uint32 
TemplateVector::getAllocSize() const 
{
   return m_allocSize;
}

         
ElementType&
TemplateVector::operator[](uint32 pos) 
{
   DEBUG1( if (pos >= m_allocSize) 
   {
      MC2ERROR( "operator[] panic" );      
      PANIC("Panic in operator[]: "
            "Fatal error in " << debugString << "-vector operator[]", 
            " pos:" << pos << " >= allocSize:" << m_allocSize << " " );
   } );
	if (pos >= m_indexToUse) {
      m_indexToUse = MIN(m_allocSize, pos+1);
   }
   return m_buf[pos];
}

ElementType&
TemplateVector::getUsedElementAt(uint32 pos) const
{
   MC2_ASSERT(pos < m_indexToUse);
   return m_buf[pos];
}
         
ElementType 
TemplateVector::getElementAt( uint32 pos ) const 
{
   DEBUG1(
      if (pos >= m_allocSize) {
         MC2ERROR2( "",
            cerr << "pos=" << pos << ", allocSize=" << m_allocSize 
            << endl;
            cerr << "Error in " << debugString
            << "[] " << pos << " >= " << m_allocSize << endl
            << "ElementType operator[](uint32 pos) const "
            << endl; );
         MC2_ASSERT(false);
      } );
   if (pos >= m_indexToUse) {
      MC2WARNING2( "", {
         cerr << debugString << "-vector::getElementAt, "
              << "warning: pos >= indexToUse, pos = " << pos
              << ", indexToUse = " << m_indexToUse << endl
              << "Warning: Trying to access uninitialized element." << endl;
      });
      return InvalidElement;
   } else {
      return (m_buf[pos]);
   }
}

ElementType 
TemplateVector::operator[](uint32 pos) const 
{
   return getElementAt(pos);
}


uint32
TemplateVector::addLast( ElementType elm)
{
   if (m_indexToUse >= m_allocSize) {
      reallocate();
   }
   m_buf[m_indexToUse] = elm;
   m_indexToUse++;
   return (m_indexToUse - 1);
}

void
TemplateVector::push_back( ElementType elm )
{
   addLast(elm);
}


uint32
TemplateVector::addLastIfUnique(ElementType elm, bool print)
{
   if (linearSearch(elm) == MAX_UINT32) {
      return (addLast(elm));
   } else {
      DEBUG2({ if (print) {
         MC2INFO2( "", {
            cerr << debugString
                 << "-Vector: addLastIfUnique :: "
                 << "Not added, element already present" 
                 << endl; 
         });
      }});
      return (MAX_UINT32);
   }
}

void
TemplateVector::insertElementSorted(ElementType value)
{
   addLast(value); // O(1)
   sort();         // O(n log n)

   // can be changed to O(n + log n) if needed
}

void
TemplateVector::insertElementAt( uint32 idx, ElementType value)
{
   // XXX: inefficient to first realloc the _entire_ array and then move
	//      (on average) half of the data to insert the new element.
   if ( (idx >= m_allocSize) || (getSize() >= m_allocSize) ){
      reallocate();
   }
   if( idx < getSize() ){
      // move the elements with higher index
      for(uint32 i = getSize(); i > idx; i-- )
         m_buf[i] = m_buf[i-1];
      m_indexToUse++;
   }

   m_buf[idx] = value;
   m_indexToUse = MAX(idx + 1, m_indexToUse);
}

bool
TemplateVector::removeElementAt(uint32 idx)
{
   if ( idx < m_indexToUse-1 ) {
      memmove(&m_buf[idx], &m_buf[idx+1],
              (m_indexToUse - 1 - idx)*sizeof(ElementType));
      m_indexToUse--;
      return (true);
   } else if (idx == m_indexToUse-1) { // Last element
      m_indexToUse--;
      return (true);
   } else {
      return (false);
   }
}

TemplateVector::iterator
TemplateVector::erase(TemplateVector::iterator it)
{
   uint32 idx = (it - begin());
   removeElementAt(idx);
   return it;
}

void
TemplateVector::deleteAllObjs()
{
/*
   // check for inefficient uses of vector
   if ((m_indexToUse > 0) && (m_allocSize > (m_indexToUse*10))) {
      // might have allocated too much
      almostAssert(false);
   }
   */
#ifndef UINT32 // Can't delete UINT32:s
   // in reverse order
   for (int32 i = getSize()-1; i >= 0; i--) {
#ifndef VectorReallyIsVoidVector
#ifdef DELETE_ARRAY
      delete [] ((*this)[i]);
#else 
      delete ((*this)[i]);
#endif // TemplateVector == StringVectorClass
#else
      // VoidVector cannot delete void*, this may
      // cause trouble.
      delete (byte*)((*this)[i]);
#endif
   }
#endif
   reset();
}


void
TemplateVector::setAllocSize(uint32 newNbrItems)
{
   if (newNbrItems > m_allocSize) {
      ElementType* tmpBuf = new ElementType [newNbrItems];
		if (m_buf != NULL) {
         memcpy(tmpBuf, m_buf, m_indexToUse * sizeof(ElementType) );
         delete [] m_buf;
		}
      m_allocSize = newNbrItems;
      m_buf = tmpBuf;
   }
}


int
TemplateVector::trimToSize()
{
   int before = m_allocSize;
   if ( getSize() < m_allocSize && getSize() != 0 ) { // Do nothing if equal
      ElementType* tmpBuf = new ElementType[getSize()];
      if ( m_buf != NULL ) {
         memcpy( tmpBuf, m_buf, getSize() * sizeof(ElementType) );
         delete [] m_buf;
      }
      m_buf = tmpBuf;
      m_allocSize = getSize();
   } else if ( getSize() == 0 ) {
      // Remove the buffer. No copy needed.
      delete [] m_buf;
      m_buf = NULL;
      m_allocSize = 0;
   }
   return before - m_allocSize;
}

void
TemplateVector::reallocate()
{
   uint32 newSize;

   // check for inefficient uses of vector
   if ( ((m_increaseSize > 0) && (m_allocSize > (m_increaseSize*30))) /*|| (m_allocSize > 1024)*/ ) {
      almostAssert(false); // inefficient if this happens often.
   }

   if ( m_increaseSize > 0 ) {
      newSize = m_allocSize + m_increaseSize;
   } else {
      newSize = MAX(1, m_allocSize * 2);
   }
   ElementType* tmpBuf = new ElementType[ newSize ];
   if (m_buf != NULL) {
	   memcpy( tmpBuf, m_buf, m_indexToUse * sizeof(ElementType) );
      delete [] m_buf;
	}
   m_allocSize = newSize;
   m_buf = tmpBuf;
}

uint32
TemplateVector::removeInvalidElements()
{
   uint32 nbrRemoved = 0;
   // reallocate, copy valid elements to the new vector, delete the old one.
   if (m_allocSize > 0) {
      ElementType* newBuf = new ElementType[m_allocSize];
      uint32 newIndexToUse = 0;
      for (uint32 i = 0; i < m_indexToUse; i++) {
         ElementType curr = m_buf[i];
         if (curr != InvalidElement) {
            // copy it
            newBuf[newIndexToUse++] = curr;
         } else {
            nbrRemoved++;
         }
      }
      delete [] m_buf;
      m_buf = newBuf;
      m_indexToUse = newIndexToUse;
   }
   return nbrRemoved;
}

void
TemplateVector::removeDoubles()
{
   // Make sure that the array constains at least two elements
   if (m_indexToUse < 2)
      return;

   // Sort the array
   sort();

   // Loop through all the elements
   uint32 writePos = 0;
   uint32 checkPos = 1;
   while (checkPos < m_indexToUse) {
      if (Deref(m_buf[writePos]) != Deref(m_buf[checkPos])) { // ZZZ char*??
         writePos++;
         m_buf[writePos] = m_buf[checkPos];
      } else {
#ifndef UINT32
#ifndef VectorReallyIsVoidVector
         delete m_buf[checkPos];
#else
         delete (byte*)m_buf[checkPos];
#endif
#endif
      }
      checkPos++;
   }
	m_indexToUse = writePos + 1;
}

void
TemplateVector::dump(bool singleLine ) const
{
   #ifndef _WIN32
      if ((singleLine) && (getSize() > 0)) {
         for (uint32 i=0; i<getSize()-1; i++)
            cout << m_buf[i] << ", ";
         cout << m_buf[getSize()-1] << endl;
      } else if (getSize() > 0) {
         for (uint32 i=0; i<getSize(); i++)
            cout << "   [" << i << "] = " << m_buf[i] << endl;
      } else {
         cout << "   " << debugString << " empty" << endl;
      }
   #endif
}

bool 
TemplateVector::remove(ElementType elm) {
   // Find elm in the vector
   if ((elm == InvalidElement) || (Deref(elm) != Deref(elm))) { // ZZZ char*?
      if (elm==InvalidElement) {
         DEBUG1( MC2WARNING2( "",
            cerr << debugString
            << "-Vector::remove(ElementType) Can't remove, "
            << "elm is NULL." << endl; ); );
      }
      else if (Deref(elm) != Deref(elm)) { // ZZZ char*?
         DEBUG1( MC2ERROR2( "",
            cerr << debugString << "-Vector::remove(ElementType) "
            << "!= returns true for"
            << " same object. Can't remove, use removeElementAt"
            << "(uint32 index)." 
            << endl; ); );
         MC2_ASSERT(false);
      }
      return false;
   }
	uint32 i = linearSearch(elm);
	if (i == MAX_UINT32) {
	   return false;
	} else { 
	   return removeElementAt(i);
	}
}

ElementType
TemplateVector::linearGet(const ElementType keyElm) const
{
   uint32 idx = linearSearch(keyElm);
   if (idx == MAX_UINT32) {
      return InvalidElement;
   }
   else {
      return getElementAt(idx);
   }
}

uint32
TemplateVector::linearSearch(const ElementType keyElm) const
{
/*   if (Deref(keyElm) != Deref(keyElm)) { // ZZZ char*?
      DEBUG1( MC2ERROR2( "",
         cerr << debugString
         << "-Vector::linearSearch  != returns true even if same object. "
         << "Performs NO SEARCH!" << endl;
         ); );
      return MAX_UINT32;
   } else {
*/
   uint32 i = 0;
   while ( (i < m_indexToUse) &&
           (ElementTypeLINEAR_SEARCH_COMP(keyElm, i)) ) { // xxx slow for objects. implement ElementTypeLINEAR_SEARCH_COMP using just operator !=.
//              ( Deref(buf[i]) != Deref(keyElm))) { // ZZZ char*?
      i++;
   }
   if (i == m_indexToUse) {
      return MAX_UINT32;
   } else {
      return i;
   }
//}
}

//#ifndef UINT32
ElementType
TemplateVector::binaryGet(const ElementType KEY,
                          uint32 start, uint32 stop) const
{
   uint32 idx = binarySearch(KEY, start, stop);
   if (idx == MAX_UINT32) {
      return InvalidElement;
   } else {
      return getElementAt(idx);
   }
}
//#endif

uint32
TemplateVector::binarySearch(const ElementType KEY,
                             uint32 start, uint32 stop) const
{
   // Initiate the lower limit of the intervall to search
   int SEARCH_START_INDEX;
   if (start < m_indexToUse)
      SEARCH_START_INDEX = start;
   else {
      if (start > 0) {
            // Start index greater than the number of elements, error
         DEBUG1( MC2WARNING2( "",
            cerr << debugString
            << "-Vector::binarySearch: start >= indexToUse (" 
            << start << " >= " << m_indexToUse
            << ". No search performed." << endl; ); );
      }
      return MAX_UINT32;
   }

   // Initiate the higher limit of the intervall to search
   int SEARCH_STOP_INDEX;
   if (stop < m_indexToUse)
      SEARCH_STOP_INDEX = stop;
   else {
      // Stop index greater/equal than the number of elements,
      // using default value, less 1!!!!
      SEARCH_STOP_INDEX = m_indexToUse-1;
   }

   #include "binarysearch.src"

   if (RESULT_INDEX < 0)
      return MAX_UINT32;
   else
      return (RESULT_INDEX);
}

bool
TemplateVector::sort()
{
   if (m_indexToUse < 2) {            // Just one element or empty vector
      return (true);
   } else {
      if ( Deref(m_buf[0]) != Deref(m_buf[0])) {
         // ZZZ char*?
         // The != is not implemented
         DEBUG1( MC2ERROR2( "",
            cerr << debugString << "-Vector::sort returning false"
            << " because operator!= returns true for same element."
            << endl; ); );
         return (false);
      } else {
         //quickSort(0, indexToUse-1);
         DEBUG8(cerr << "Using nonRecursiveQuickSort()" << endl);
         nonRecursiveQuickSort();
         return (true);
      }
   }
}


// This is the _private_ method that performes the actual search.
void 
TemplateVector::quickSort(uint32 start, uint32 stop)
{
   // Might want to use some other search method when 
   // (stop-start) < "small number"...
   if(start < stop) {
      ElementType x = m_buf[start];
      uint32 i = start - 1;
      uint32 j = stop + 1;

      bool done = false;
      while(!done) {
         while( (j > start) && (Deref(m_buf[--j]) > Deref(x))) { // ZZZ char*?
         }
         while( Deref(m_buf[++i]) < Deref(x)) { // ZZZ char*?
         }
         if(i < j) {
            ElementType temp = m_buf[i];
            m_buf[i] = m_buf[j];
            m_buf[j] = temp;
         } else {
            done = true;
         }
      }
      quickSort(start, j);
      quickSort(j + 1, stop);
   }
}


void
TemplateVector::nonRecursiveQuickSort()
{
   const int SORTING_END = getSize();

   #include "nonrecursivequicksort.src"
}

uint32
TemplateVector::partition( uint32 start, uint32 stop )
{
   ElementType base = m_buf[start];
   uint32 i = start-1;
   uint32 j = stop+1;
   while (true) {
      do {
         j--;
      } while (Deref(m_buf[j]) > Deref(base)); // ZZZ char*?
      do {
         i++;
      } while (Deref(m_buf[i]) < Deref(base)); // ZZZ char*?
      if (i < j) {
         SWAP( i, j );
      } else {
         return j;
      }
   }
}

uint32
TemplateVector::randomizedPartition( uint32 start, uint32 stop )
{
   uint32 newPivot =
      uint32 ( double (stop - start) * rand()/(RAND_MAX+1.0) + start );
   DEBUG8( cerr << " random element : " << newPivot
           << " from interval [" << start << ", " << stop << "]." << endl);
   SWAP( start, newPivot );
   return partition( start, stop );
}

uint32
TemplateVector::randomizedSelect( uint32 start, uint32 stop, uint32 rank )
{
   if ( start == stop ) {
      return start;
   } else {
      uint32 inside = randomizedPartition( start, stop );
      uint32 offset = inside - start + 1;
      if (rank <= offset) {
         return randomizedSelect( start, inside, rank );
      } else {
         return randomizedSelect( inside+1, stop, rank - offset );
      }
   }
}

void
TemplateVector::sortSmallest( int32 start, int32 stop, uint32 count )
{
   if (stop > start) {
      const int SORTING_END = randomizedSelect( start, stop, count );
      DEBUG8( cerr << "sorting end: " << SORTING_END <<
              " count: " << count << endl );
      #include "nonrecursivequicksort.src"
   }
}

void
TemplateVector::reverse() 
{
   uint32 middlePos = getSize() / 2;
   uint32 lastPos = getSize() - 1;
   for (uint32 i = 0; i < middlePos; i++)
      SWAP(i, lastPos - i);
}

void
TemplateVector::debugPrintVector()
{
   #ifdef DEFAULT_PRINT
   DEBUG1(
      for (uint32 i = 0; i < getSize(); i++ ) {
         cerr << i << " " << (*this)[i] << endl;
      } );
   #else
   #ifdef VectorInclVOID
   DEBUG1(
      for (uint32 i = 0; i < getSize(); i++) {
         cerr << i << " (void element)" << endl;
      } );
   #else
   DEBUG1(
      for (uint32 i = 0; i < getSize(); i++ ) {
         ElementType tmpe = (*this)[i];
         if (tmpe != NULL) {
            const char *tmp = (*this)[i]->toString();
            cerr << i << tmp << endl;
            delete [] tmp;
         } else {
            cerr << i << "<element is NULL>" << endl;
         }
      } );
   #endif
   #endif
}



ElementType 
TemplateVector::getSafeElementAt( uint32 pos ) const
{
   if(pos >= m_allocSize)    // Not necessary if indexToUse is correct.
      return InvalidElement;
   if (pos >= m_indexToUse)  // To avoid the mc2error of getElementAt()
      return InvalidElement;
   else
      return getElementAt(pos);
}


ElementType 
TemplateVector::getLast() const
{
   if (m_indexToUse > 0) {
      // OK, the array is not empty
      return (m_buf[m_indexToUse-1]);
   } else {
      // The Vector is empty
      return (InvalidElement);
   }
}

void 
TemplateVector::setUsedElementAt( uint32 pos, ElementType elm)
{
   MC2_ASSERT(pos < m_allocSize);
   (*this).m_buf[pos] = elm;
}
      
void 
TemplateVector::setElementAt( uint32 pos, ElementType elm)
{
   // now calls operator[].
   (*this)[pos] = elm;
}

bool 
TemplateVector::setSafeElementAt(uint32 pos, ElementType elm)
{
   if(pos < MAX_SAFE_VECTORSIZE)
   {
      if(pos >= m_allocSize)
      {
         setAllocSize(pos + 1);
      }
      if(pos >= m_indexToUse)
      {
         for(uint32 i = m_indexToUse; i < pos ; i++)
            (*this)[i] = InvalidElement; // all unused elements defaulted.
         m_indexToUse = pos+1;
      }
      (*this)[pos] = elm;
      return true;
   }
   return false;
}

uint32 
TemplateVector::getMemoryUsage() const 
{
   return sizeof(TemplateVector) + m_allocSize * sizeof(ElementType);
}

int 
TemplateVector::COMP(uint32 a, uint32 b) const 
{
   return ElementTypeCOMP(a, b);
/*   if (Deref(buf[a]) < Deref(buf[b]))
      return (-1);
   else if (Deref(buf[a]) > Deref(buf[b]))
      return (1);
   else
   return (0);*/
}

int 
TemplateVector::SEARCH_COMP(const ElementType key, uint32 pos) const 
{
   return ElementTypeSEARCH_COMP(key, pos);
/*   if (Deref(key) < Deref(buf[pos]))
      return (-1);
   else if (Deref(key) > Deref(buf[pos]))
      return (1);
   else
   return (0);*/
}

void 
TemplateVector::SWAP(int a, int b) 
{
   ElementType foo = m_buf[a]; 
   m_buf[a] = m_buf[b]; 
   m_buf[b] = foo;
}

TemplateVector::iterator
TemplateVector::begin() const
{
   return m_buf;
}

TemplateVector::iterator
TemplateVector::end() const
{
   if ( m_buf == NULL ) {
      return NULL;
   } else {
      return &m_buf[m_indexToUse];
   }
}
