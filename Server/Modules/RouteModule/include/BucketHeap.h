/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BUCKETHEAP_H
#define BUCKETHEAP_H
#include "config.h"

#if defined (__GNUC__) && __GNUC__ > 2
#include <ext/slist>
using namespace __gnu_cxx;
#else
#include <slist>
#endif
#include<deque>
#include<list>
#include "RedBlackTree.h"

#include "RoutingNode.h"
#include "OverflowNode.h"

class RoutingMap;

// If UNORDERED_BUCKET_HEAP is defined, only the buckets containing a
// destination will be dequeued in order. I don't know how much faster
// the program will run because of that.

#define UNORDERED_BUCKET_HEAP
#undef OLD_BUCKET_HEAP

#ifndef OLD_BUCKET_HEAP

// Choose buckettype here
//#define Bucket SlistBucket
//#define Bucket DequeBucket
#define Bucket ArrayBucket

//---------- SlistBucket -----------------

#if ( Bucket == SlistBucket )

/**
 *   Defines one bucket in the bucketheap.
 *   Dequeues the bucket in order if there are destinations
 *   and in no order if there are no destinations.
 */
class SlistBucket {
public:

   /**
    *    Creates a new Bucket with room for size elements.
    */
   inline SlistBucket(RoutingMap* theMap, int size);
   
   /**
    *    Enqueues the node in the bucket.   
    */
   inline void enqueue(RoutingNode* node);

   /**
    *    Dequeues one node from the bucket.
    */
   inline RoutingNode* dequeue();

   /**
    *    Dequeues one node from the bucket.
    */
   inline RoutingNode* dequeueUnordered();

   /**
    *    Returns true if the bucket is empty.
    */
   inline bool isEmpty() const;

   /**
    *    Empties the Bucket.
    */
   inline void reset();

private:

   /**
    *    The routing map will contain the costs
    */
   RoutingMap* m_map;
   
   /**
    *    Dequeues the cheapest node in the list.
    */
   RoutingNode* dequeueCheapest();

   /**
    *    Keeps track of the number of destinations in the bucket.
    */
   int m_nbrDestsInBucket;
   
   /**
    *    List containing the nodes of the Bucket.
    */
   slist<RoutingNode*> m_contents;
   
};

inline void
SlistBucket::reset()
{
   m_contents.clear();
   m_nbrDestsInBucket = 0;
}

inline
SlistBucket::SlistBucket(RoutingMap* theMap, int size)
{
   m_map = theMap;
   reset();
}

inline void
SlistBucket::enqueue(RoutingNode* node)
{
   m_contents.push_front(node);
   m_nbrDestsInBucket += ( node->isDest(m_map) != 0 );
}

inline bool
SlistBucket::isEmpty() const
{
   return m_contents.empty();
}

inline RoutingNode*
SlistBucket::dequeueCheapest()
{
   uint32 cheapestSoFar = MAX_UINT32;
   slist<RoutingNode*>::iterator cheapestNode = m_contents.begin();
   for(slist<RoutingNode*>::iterator it = m_contents.begin();
       it != m_contents.end();
       ++it ) {
      if ( (*it)->getEstCost(m_map) < cheapestSoFar ) {
         cheapestNode = it;
         cheapestSoFar = (*it)->getEstCost(m_map);
      }
   }
   RoutingNode* retVal = *cheapestNode;
   m_contents.erase(cheapestNode); // XXX: Slow?
   return retVal;
}

inline RoutingNode*
SlistBucket::dequeue()
{
   RoutingNode* retVal;
   if ( m_nbrDestsInBucket ) {
      // FIXME: Extra queue for destinations
      retVal = dequeueCheapest();
   } else {
      retVal = m_contents.front();
      m_contents.pop_front();
   }
   m_nbrDestsInBucket -= ( retVal->isDest(m_map) != 0 );
   return retVal;
}

inline RoutingNode*
SlistBucket::dequeueUnordered()
{
   if ( !m_contents.empty() ) {
      // Implement correctly
      RoutingNode* retVal = m_contents.front();
      m_contents.pop_front();
      m_nbrDestsInBucket -= ( retVal->isDest(m_map) != 0 );
      return retVal;
   } else {
      if ( m_nbrDestsInBucket != 0 ) {
         mc2log << error << "[BH] - bucket is empty but nbrDests is "
                << m_nbrDestsInBucket << endl;
      }
      return NULL;
   }  
}

#endif // Bucket == SlistBucket

#if ( Bucket == DequeBucket ) 

// -------------- DequeBucket

/**
 *   DequeBucket should really use a deque but it did not compile
 *   so I'm using a list instead.
 *   The method is to put the destinations so that they will be
 *   dequeued after all the other stuff in a bucket. No counter.
 */
class DequeBucket : private list<RoutingNode*> {
public:

   /**
    *    Creates a new DequeBucket with the supplied (initial) size
    */
   inline DequeBucket(const RoutingMap* theMap, int size);
   
   /**
    *    Enqueues the node in the bucket.   
    */
   inline void enqueue(RoutingNode* node);

   /**
    *    Dequeues one node from the bucket.
    */
   inline RoutingNode* dequeue();

   /**
    *    Dequeues one node from the bucket.
    */
   inline RoutingNode* dequeueUnordered();

   /**
    *    Returns true if the bucket is empty.
    */
   inline bool isEmpty() const;

   /**
    *    Empties the Bucket.
    */
   inline void reset();
   
private:

   /**
    *    The RoutingMap.
    */
   const RoutingMap* m_map;
   
};

inline
DequeBucket::DequeBucket(const RoutingMap* theMap,
                         int size) : list<RoutingNode*>()
{
   m_map = theMap;
}

inline void
DequeBucket::enqueue(RoutingNode* node)
{
   // Put the destinations in the front so that they will
   // be dequeued last.
   if ( MC2_UNLIKELY(node->isDest(m_map)) ) {
      push_front(node);
   } else {
      push_back(node);
   }
}

inline RoutingNode*
DequeBucket::dequeue()
{
   RoutingNode* retVal = back();
   pop_back();
   return retVal;
}

inline bool
DequeBucket::isEmpty() const
{
   return empty();
}

inline RoutingNode*
DequeBucket::dequeueUnordered()
{
   if ( isEmpty() ) {
      return NULL;
   } else {
      return dequeue();
   }
}

inline void
DequeBucket::reset()
{
   clear();
}

#endif // Bucket == DequeBucket

#if ( Bucket == ArrayBucket ) 

// -------------- ArrayBucket

/**
 *   Bucket that uses array instead of stl-stuff for storing
 *   nodes. Uses slist for overflow and RedBlackTree for destinations.
 */
class ArrayBucket {
public:

   /**
    *    Creates a new DequeBucket with the supplied (initial) size
    */
   inline ArrayBucket(RoutingMap* theMap, int size);

   /**
    *    Deletes the arraybucket.
    */
   inline ~ArrayBucket();
   
   /**
    *    Enqueues the node in the bucket.   
    */
   inline void enqueue(RoutingNode* node);

   /**
    *    Dequeues one node from the bucket.
    */
   inline RoutingNode* dequeue();

   /**
    *    Dequeues one node from the bucket.
    */
   inline RoutingNode* dequeueUnordered();

   /**
    *    Returns true if the bucket is empty.
    */
   inline bool isEmpty() const;

   /**
    *    Empties the Bucket.
    */
   inline void reset();
   
private:

   /**
    *   Array of RoutingNodes to use first.
    */
   RoutingNode** m_array;

   /**
    *   Size of array.
    */
   const int m_arraySize;

   /**
    *   Number of nodes used in array.
    */
   int m_nbrUsedInArray;
   
   /**
    *   Destinations are put here.
    */
   RedBlackTree m_dests;

   /**
    *   If the array is full, nodes are put here.
    */
   list<RoutingNode*> m_overflow;

   /** 
    *   The costs of the nodes will be stored here.
    */
   RoutingMap* m_map;
   
};

void
ArrayBucket::reset()
{
   m_nbrUsedInArray = 0;
   m_dests.reset();
   m_overflow.clear();
}

inline
ArrayBucket::ArrayBucket(RoutingMap* theMap, int size)
      : m_arraySize(size),
        m_dests(theMap)
{
   m_map = theMap;
   m_array = new RoutingNode*[size];
   reset();
}

inline
ArrayBucket::~ArrayBucket()
{
   delete [] m_array;
}

void
ArrayBucket::enqueue(RoutingNode* node)
{
   if ( node->isDest(m_map) == false ) {
      if ( m_nbrUsedInArray < m_arraySize ) {
         m_array[m_nbrUsedInArray++] = node;
      } else {
         m_overflow.push_front(node);
         ++m_nbrUsedInArray;
      }
   } else {
      m_dests.enqueue(node);
   }
}

RoutingNode*
ArrayBucket::dequeue()
{
   if ( m_nbrUsedInArray != 0 ) {
      if ( m_nbrUsedInArray <= m_arraySize  ) {
         return m_array[--m_nbrUsedInArray];
      } else {
         RoutingNode* node = m_overflow.back();
         m_overflow.pop_back();
         --m_nbrUsedInArray;
         return node;
      }
   } else {
      // Only destinations left
      return m_dests.dequeue();
   }          
}

bool
ArrayBucket::isEmpty() const
{
   return m_nbrUsedInArray == 0 && m_dests.isEmpty();
}

RoutingNode*
ArrayBucket::dequeueUnordered()
{
   if ( !isEmpty() ) {
      return dequeue();
   } else {
      return NULL;
   }
}

#endif // Bucket == ArrayBucket

#endif


/**
 *  A class describing the BucketHeap ADT. It is a heap used by
 *  Dijkstra's algorithm in RouteModule.
 * 
 */
class BucketHeap
{
public:
   
   /**
    * Constructor for the BucketHeap.
    */
   BucketHeap(RoutingMap* theMap);

   /**
    * Destructor for the BucketHeap.
    */
   ~BucketHeap();

   /**
    * Puts a RoutingNode into the heap.
    * 
    * @param routingNode A pointer to the RoutingNode to be inserted
    * into the heap.
    */
   inline void enqueue(RoutingNode* routingNode);
   
   /**
    * Removes the RoutingNode with the minimum cost from the heap. If heap
    * is empty, NULL is returned.
    *
    * @return The node with the least cost.
    */
   inline RoutingNode* dequeue();

#ifdef OLD_BUCKET_HEAP
   inline RoutingNode* dequeue2();
#endif
   
   /**
    * Is the heap empty?
    *
    * @return True if the heap is empty.
    */
   inline bool isEmpty();
   
   /**
    * Restores the heap back to its original state. If overflow has
    * occurred in any bucket, the overflow lists are cleared. The array of
    * the number of used elements and all other integer member variables
    * are set to zero.
    */
   void reset();
   
   /**
    * Updates the minimum cost and the start bucket.
    *
    * @param cost Is the minimum start cost.
    */
   inline void updateStartIndex (uint32 cost);

#ifdef OLD_BUCKET_HEAP
   /**
    * Prints debugging information on the screen.
    * Use after a route but before reset.
    */
   void dump();
   
   /**
    * Checks the consistency of the heap. Used for debugging.
    * Suitable to use before and after enqueue and dequeue.
    *
    * @return true True will be returned if the heap is erroneous,
    *         false otherwise.
    */
   bool checkHeapConsistency();
#endif
   
  private:
   
   /**
    * Updates the m_heapStartIndex and m_leastExpectedCostInHeap to make
    * memory use more efficient.
    */
   inline void flushHeap();
   
#ifndef OLD_BUCKET_HEAP
   inline uint32 calcRowIndex(uint32 cost);
#endif
   
   /**
    * Tries to empty the largeCostsBucket after each ROW overflow.
    */
   void tryToEmptyLargeCostsBucket();

#ifdef OLD_BUCKET_HEAP
   /**
    * Function that takes care of the overflow enqueuing.
    *
    * @param routingNode The node to be enqueued.
    * @param rowIndex The row of the list to enqueue in (MAX_INT32 if in
    *        the large costs bucket's list).
    */
   void overflowEnqueue(RoutingNode* routingNode, int32 rowIndex);
   
   /**
    * Function that takes care of the overflow dequeuing. It browses
    * both the minimum cost bucket and the corresponding list to find the
    * minimum cost node.
    *
    * @return The minimum cost node.
    */
   RoutingNode* overflowDequeue();

   /**
    * Function that tries to empty the list of the large costs bucket.
    * If there is space left in the large costs bucket the function will
    * put those nodes with too high costs to fit in the normal heap into
    * the large costs bucket.
    * 
    * @param costsHandled This is the costs handled by the normal heap.
    */
   void emptyLargeCostsOverflow(uint32 costsHandled);
#endif
   
/////////////////////////////////////////////////////////////////
// Constant variables used by the BucketHeap
/////////////////////////////////////////////////////////////////

   /**
    * The number of rows in the heap.
    */
   static const uint32 ROWS = 8000;

   /**
    * The number of columns in the heap.
    */
   static const uint32 COLUMNS = 256;

   /**
    * The number of elements in the large costs bucket.
    */
   static const uint32 LARGE_COSTS_BUCKET_SIZE = COLUMNS * COLUMNS;

   /**
    * The 2^BUCKET_COST_DIFFERENCE_EXP is the difference between the
    * rows in the matrix.
    */
   static const uint32 BUCKET_COST_DIFFERENCE_EXP = 10;

   /**
    * The difference in cost between two buckets.
    */
   static const uint32 BUCKET_COST_DIFFERENCE = 1024;
   
   /**
    * The range of costs handled by the heap.
    */
   static const uint32 RANGE_OF_COSTS_IN_HEAP =
      (ROWS * BUCKET_COST_DIFFERENCE - 1);

/////////////////////////////////////////////////////////////////
// Member variables
/////////////////////////////////////////////////////////////////
#ifdef OLD_BUCKET_HEAP
   /**
    *   Describes the heap itself.
    */
   RoutingNode* m_matrix[ROWS][COLUMNS];
   
   /**
    * The bucket where the nodes with too large costs are put.
    */
   RoutingNode* m_largeCostsBucket[COLUMNS * COLUMNS];

   /**
    * The number of spaces used in each bucket.
    */
   uint32 m_nbrUsed[ROWS];

   /**
    *   The number of destinations in the bucket at ROW.
    *   If m_nbrDestinations for a bucket == 0 the nodes
    *   will not be taken out of the bucket in order.
    */
   uint32 m_nbrDestinations[ROWS];
   
   /**
    * The lists of overflows of the buckets.
    */
   Head* m_overflowLists[ROWS];
   
   /**
    * The number of spaces used in m_largeCostsBucket.
    */
   uint32 m_nbrUsedInLargeCostsBucket;

   /**
    * The list of overflow in the m_largeCostsBucket.
    */
   Head* m_largeCostsOverflow;
   
   /**
    * Determines if overflow has occurred.
    * @see reset().
    */
   bool m_overflowHasOccurred;
#else 
   /**
    *   Bucketpointers. One extra for largecosts.
    */
   Bucket* m_buckets[ROWS+1];
#endif

   /**
    * The starting number of the heap cost counter.
    */
   uint32 m_heapStartIndex;
   
   /**
    * The lower cost bound.
    */
   uint32 m_leastExpectedCostInHeap;

   /**
    * Total used number of spaces in matrix.
    */
   uint32 m_totalNbrUsed;

   /**
    *   The map is needed for costs.
    */
   RoutingMap* m_map;

}; // BucketHeap

inline bool BucketHeap::isEmpty()
{
   return m_totalNbrUsed == 0;
}

//////////////////////////////////////////////////////////////////////////
// Inline functions for new BucketHeap
//////////////////////////////////////////////////////////////////////////
#ifndef OLD_BUCKET_HEAP
inline
void
BucketHeap::reset()
{
   m_leastExpectedCostInHeap   = 0;
   m_totalNbrUsed              = 0;
   m_heapStartIndex            = 0;
   for(uint32 i=0; i < ROWS + 1 ; ++i ) {
      m_buckets[i]->reset();
   }
}

inline uint32
BucketHeap::calcRowIndex(uint32 cost)
{
   if ( cost > ( m_leastExpectedCostInHeap + RANGE_OF_COSTS_IN_HEAP) ) {
      return ROWS; // Largecosts bucket.
   }
   if ( cost < m_leastExpectedCostInHeap ) {
      return m_heapStartIndex;
   } else {
      uint32 rowIndex = ((cost - m_leastExpectedCostInHeap)
                         >> BUCKET_COST_DIFFERENCE_EXP) + m_heapStartIndex;
      if (rowIndex >= (int32) ROWS) {
         rowIndex -= ROWS;
      }
      return rowIndex;
   }
}

inline void BucketHeap::enqueue(RoutingNode* node)
{
   const uint32 rowIdx = calcRowIndex(node->getEstCost(m_map));
   // Largecosts bucket should be the last (at ROWS) 
   m_buckets[rowIdx]->enqueue(node);
   ++m_totalNbrUsed;
}

inline void
BucketHeap::flushHeap()
{
   if ( ! isEmpty() ) {
      while( m_buckets[m_heapStartIndex]->isEmpty() ) {
         if ( ++m_heapStartIndex == ROWS ) {
            m_heapStartIndex = 0;
            m_leastExpectedCostInHeap += BUCKET_COST_DIFFERENCE;
            if ( ! m_buckets[ROWS]->isEmpty() ) {
               tryToEmptyLargeCostsBucket();
            }
         } else {
            m_leastExpectedCostInHeap += BUCKET_COST_DIFFERENCE;
         }
      }
   }
}

inline RoutingNode*
BucketHeap::dequeue()
{
   flushHeap();
   RoutingNode* res = m_buckets[m_heapStartIndex]->dequeue();
   if ( res ) {
      --m_totalNbrUsed;
   }
   return res;
}

#endif
//////////////////////////////////////////////////////////////////////////
// Inline functions for old BucketHeap
//////////////////////////////////////////////////////////////////////////
#ifdef OLD_BUCKET_HEAP
inline void BucketHeap::enqueue(RoutingNode* routingNode)
{
   uint32 cost = routingNode->getEstCost(m_map);
//     if ( cost == MAX_UINT32 )
//        mc2log << warn << "enqueueing node with cost MAX_UINT32" << endl;
   
   int32 rowIndex;
   if (cost > (m_leastExpectedCostInHeap + RANGE_OF_COSTS_IN_HEAP)) {
      // Too large cost, put it in largeCostsBucket
      if (m_nbrUsedInLargeCostsBucket < LARGE_COSTS_BUCKET_SIZE) {
         m_largeCostsBucket[m_nbrUsedInLargeCostsBucket++] = routingNode;
      } else {
         overflowEnqueue(routingNode, MAX_INT32);
      }
   } else {
      if (cost < m_leastExpectedCostInHeap) {
         mc2dbg4 << "Too small cost in heap" << endl;
         rowIndex = m_heapStartIndex;
      } else {
         rowIndex = ((cost - m_leastExpectedCostInHeap)
                     >> BUCKET_COST_DIFFERENCE_EXP) + m_heapStartIndex;
         if (rowIndex >= (int32) ROWS)
            rowIndex -= ROWS;
      }
      
      if (m_nbrUsed[rowIndex] < COLUMNS) {
         m_matrix[rowIndex][m_nbrUsed[rowIndex]++] = routingNode;
#ifdef UNORDERED_BUCKET_HEAP
         // Check if the node is a destination and increase counter.
//           if ( routingNode->isDest(m_map) )
//              m_nbrDestinations[rowIndex]++;
         m_nbrDestinations[rowIndex] += (routingNode->isDest(m_map) == true);
#endif
      } else {
         overflowEnqueue(routingNode, rowIndex);
      }
   }
   
   m_totalNbrUsed++;
   
} // enqueue


inline void BucketHeap::flushHeap()
{
   if( m_totalNbrUsed > 0  ) {
      while (m_nbrUsed[m_heapStartIndex] == 0) {
         if (++m_heapStartIndex == ROWS) {
            m_heapStartIndex = 0;
            m_leastExpectedCostInHeap += BUCKET_COST_DIFFERENCE;
            if (m_nbrUsedInLargeCostsBucket > 0)
               tryToEmptyLargeCostsBucket();
         }
         else
            m_leastExpectedCostInHeap += BUCKET_COST_DIFFERENCE;
      }
   } else {
      MC2ERROR("BucketHeap::flushHeap with empty heap");
   }
}


inline RoutingNode*
BucketHeap::dequeue()
{
   RoutingNode* minNode = NULL;
   uint32 minCost = MAX_UINT32;
   
   flushHeap();

   if ( !m_overflowLists[m_heapStartIndex]->empty() ){
      minNode = overflowDequeue();
   } else {
      uint32 minIndex = 0;
      uint32 nbrElements = m_nbrUsed[m_heapStartIndex];
      RoutingNode** minimumCostBucket = m_matrix[m_heapStartIndex];
#ifdef UNORDERED_BUCKET_HEAP
      // Don't dequeue in order if the bucket doesn't contain a destination.
      const bool checkOrder = m_nbrDestinations[m_heapStartIndex];
#else
      // Always dequeue in order.
      const bool checkOrder = true; // Should be optimized away, I hope.
#endif      
      if ( checkOrder ) {
         for (int i = nbrElements - 1; i >= 0; --i) {      
            RoutingNode* const tempNode = minimumCostBucket[i];
            if (tempNode->getEstCost(m_map) <= minCost) {
               minCost  = tempNode->getEstCost(m_map);
               minNode  = tempNode;
               minIndex = i;
            }
         }
         minimumCostBucket[minIndex] = minimumCostBucket[nbrElements - 1];
#ifdef UNORDERED_BUCKET_HEAP
         m_nbrDestinations[m_heapStartIndex] -= ( minNode->isDest(m_map) == true);
/*           if ( minNode->isDest(m_map) ) */
/*              m_nbrDestinations[m_heapStartIndex]--; */
#endif
      } else {
#ifdef UNORDERED_BUCKET_HEAP         
         // Dequeue the middle one. This is an experiment. Call it
         // heuristics.
         // Shouldn't be a destination, since we aren't checking order.
         minIndex = nbrElements >> 1;
         minNode = minimumCostBucket[minIndex];
         minimumCostBucket[minIndex] = minimumCostBucket[nbrElements - 1];

#endif
      }      
      m_nbrUsed[m_heapStartIndex]--;
   }
   m_totalNbrUsed--;
   return minNode;
} // dequeue

inline RoutingNode*
BucketHeap::dequeue2()
{
   mc2dbg2 << "BucketHeap::dequeue2" << endl;
   RoutingNode* minNode = NULL;
   uint32 minCost = MAX_UINT32;
   
   mc2dbg4 << "Before flushheap" << endl;
   flushHeap();
   mc2dbg4 << "After flushheap" << endl;

   if (m_overflowHasOccurred && (!m_overflowLists[m_heapStartIndex]->empty()))
      minNode = overflowDequeue();
   else {
      mc2dbg4 << "else" << endl;

      RoutingNode* tempNode;
      uint32 minIndex = 0;
      uint32 nbrElements = m_nbrUsed[m_heapStartIndex];
      RoutingNode** minimumCostBucket = m_matrix[m_heapStartIndex];
      
      for (uint32 i = 0; i < nbrElements; i++) {      
         tempNode = minimumCostBucket[i];
         if (tempNode->getEstCost(m_map) < minCost) {
            minCost  = tempNode->getEstCost(m_map);
            minNode  = tempNode;
            minIndex = i;
         }
      }
      
      minimumCostBucket[minIndex] = minimumCostBucket[nbrElements - 1];
      m_nbrUsed[m_heapStartIndex]--;
   }
   m_totalNbrUsed--;

   return minNode;
} // dequeue
#endif // OLD_BUCKET_HEAP

// Common functions for old and new bucketheap

inline void
BucketHeap::updateStartIndex(uint32 cost)
{
   if( cost > BUCKET_COST_DIFFERENCE ){
      while (m_leastExpectedCostInHeap < (cost - BUCKET_COST_DIFFERENCE)) {
         m_heapStartIndex++;
         if(m_heapStartIndex == ROWS)
            m_heapStartIndex = 0;
         m_leastExpectedCostInHeap += BUCKET_COST_DIFFERENCE;
      } 
   }
}
   
#endif // BUCKETHEAP_H
