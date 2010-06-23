/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if 0
#ifndef BUCKETLIFO_H
#define BUCKETLIFO_H

#include "config.h"
#include "RoutingNode.h"
#include <math.h>

// The number of rows and columns in the heap
const uint32 ROWS    = 4000;
const uint32 COLUMNS = 100;

// The maximum number of elemenst in the heap
const uint32 MAX_NBR_OF_ELEMENTS = ROWS * COLUMNS;

/* The 2^MATRIX_DIFF_EXPONENTIAL is the difference between the rows
   in the matrix */
const uint32 MATRIX_DIFF_EXPONENTIAL = 10;

// The difference between two rows in the matrix
const uint32 MATRIX_DIFF = 1024;

/**
 * A class describing the BucketLIFO ADT. It is a heap supposed to be
 * used by Dijkstra's algorithm in RouteModule. The only operations on the
 * heap are EMPTY, ENQUEUE, DEQUEUE and RESET. It is quite similar to
 * BucketHeap, but it needs a different CalcRoute.
 *
 * This seems like the original BucketHeap that we first came up with at
 * Vipeholm. The difference between the BucketLIFO and the BucketHeap is
 * that the BucketLIFO does not guarantee that dequeue returns the node
 * with the smallest cost, only one of the nodes in the bucket containing
 * the node with the smallest cost. The difference in CalcRoute is that
 * it cannot determine when to stop just by seeing that it has dequeued
 * a destination, because there can still be a node with a cheaper cost
 * in the bucket.
 * 
 * Maybe the lifo can be modified so that it will return the nodes in
 * order if the bucket contains a destination node. Don't know if that
 * will speed things up over the ordinary BucketHeap though. I think 
 * that it should, because there will be many unordered accesses before
 * the sorting dequeue must occur.
 *
 * @see     CalcRoute
 */
class BucketLIFO {
   public:
      /**
       * Constructor for the BucketLIFO.
       */
      BucketLIFO();

      /**
       * Destructor.
       */
      ~BucketLIFO();
         
      /**
       * Puts a RoutingNode into the heap.
       * @param routingNode a pointer to the RoutingNode to be inserted
       * into the heap
       */
      inline void enqueue(RoutingNode* routingNode);
         
      /**
       * Removes the RoutingNode with the least cost from the heap. If
       * the heap is empty, NULL is returned.
       * @return the node with the least cost
       */
      inline RoutingNode* dequeue();
         
      /**
       * Is the heap empty?
       * @return true if the heap is empty
       */
      inline bool isEmpty();

      /**
       * @return true if there are things left in the minimum cost bucket
       */
      inline bool thingsLeftInBucket();

      /**
       * Marks a bucket when a destination has been reached 
       */
      inline void markBucket();
      
      /**
       * Sets all the pointers in the heap to null.
       */
      void reset();

      /**
       * Prints debugging information on the screen.
       * Use after a route but before reset.
       */
      void dump();
 
   private:
      /**
       * Updates the m_heapStartIndex and m_costsStartAt to make memory
       * use more efficient.
       */
      inline void flushHeap();

      /// Describes the heap itself
      RoutingNode* m_matrix[ROWS][COLUMNS];
         
      /// The number of spaces used in each bucket
      uint32 m_nbrUsed[ROWS];

      /// The starting number of the heap cost counter
      uint32 m_heapStartIndex;

      /// The lower cost bound
      uint32 m_costsStartAt;
         
      /// Total used number of spaces in matrix
      uint32 m_totalNbrUsed;

      /// Largest value of m_totalNbrUsed
      uint32 m_largestTotalNbrUsed;    // remove

      /// True if a destination has been reached
      bool m_bucketMarked;
   
}; // BucketLIFO

// =======================================================================
//                                      Implementation of inline methods =

inline void BucketLIFO::markBucket()
{
   m_bucketMarked = true;
}

inline bool BucketLIFO::isEmpty()
{
   return (m_totalNbrUsed == 0) || 
          ( m_bucketMarked && (m_nbrUsed[m_heapStartIndex] == 0) );
}

inline bool BucketLIFO::thingsLeftInBucket()
{
   return (m_nbrUsed[m_heapStartIndex] > 0);
}

inline void BucketLIFO::flushHeap()
{
   while (m_nbrUsed[m_heapStartIndex] == 0) {
      if (++m_heapStartIndex == ROWS)
         m_heapStartIndex = 0;
      m_costsStartAt += MATRIX_DIFF;
   }
}     

inline void
BucketLIFO::enqueue(RoutingNode* routingNode)
{
   uint32 cost = routingNode->getEstCost();
   int32 rowIndex;

   if (cost < m_costsStartAt) {
      mc2dbg1 << "ERROR: Low cost in heap: " << cost << endl;
      rowIndex = m_heapStartIndex;
   }
   else
      if (cost >= (m_costsStartAt + (ROWS << MATRIX_DIFF_EXPONENTIAL))) {
         mc2dbg8 << "Higher cost of node than allowed in heap: " <<
            cost << endl;
         rowIndex = m_heapStartIndex - 1;
         if (rowIndex < 0)
            rowIndex += ROWS;
      }
      else {
         rowIndex = ((cost - m_costsStartAt) >> MATRIX_DIFF_EXPONENTIAL) +
            m_heapStartIndex;
         
         while (rowIndex >= (int32)ROWS)
            rowIndex -= ROWS;
      }
   
   m_matrix[rowIndex][m_nbrUsed[rowIndex]++] = routingNode;
   m_totalNbrUsed++;
   if (m_totalNbrUsed > m_largestTotalNbrUsed)
      m_largestTotalNbrUsed = m_totalNbrUsed;

} // enqueue

inline RoutingNode*
BucketLIFO::dequeue()
{
   RoutingNode* minNode = NULL;
   uint32 minCost = MAX_UINT32;
   bool again = false;
   
   do {
      flushHeap();
      minNode = m_matrix[m_heapStartIndex][--m_nbrUsed[m_heapStartIndex]];
      minCost = minNode->getEstCost();
      again = (minCost >= (m_costsStartAt + MATRIX_DIFF));
      if(again){
         m_totalNbrUsed--; // enqueue increases this
         enqueue(minNode);
      }
      
   } while(again);
   /*
   if (m_totalNbrUsed) {
      while (minCost >= (m_costsStartAt + MATRIX_DIFF)) {
         flushHeap();
         minNode = m_matrix[m_heapStartIndex][--m_nbrUsed[m_heapStartIndex]];
         minCost = minNode->getEstCost();
         if (minCost >= (m_costsStartAt + MATRIX_DIFF)) {
            m_totalNbrUsed--; // enqueue increases this
            enqueue(minNode);
         }
      }
      m_totalNbrUsed--;
   }
   */
   m_totalNbrUsed--;
   return minNode;
}

#endif
#endif
