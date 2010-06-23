/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "BucketHeap.h"
#include "TimeUtility.h"

#ifndef OLD_BUCKET_HEAP
// ////////////////////////////////////////////////////////////////////////
// Functions for new BucketHeap
// ////////////////////////////////////////////////////////////////////////
BucketHeap::BucketHeap(RoutingMap* theMap)
{
   m_map = theMap;
   for(uint32 i=0; i < ROWS; ++i ) {
      m_buckets[i] = new Bucket(theMap, COLUMNS);
   }
   // Largecosts bucket.
   m_buckets[ROWS] = new Bucket(theMap, COLUMNS*ROWS);
   reset();
}

BucketHeap::~BucketHeap()
{
   for(uint32 i=0; i < (ROWS + 1) ; ++i ) {
      delete m_buckets[i];
   }
}

void
BucketHeap::tryToEmptyLargeCostsBucket()
{
   uint32 startTime = TimeUtility::getCurrentTime();
   mc2dbg << "[BH]: tryToEmptyLargeCostsBucket m_totalNbrUsed = "
          << m_totalNbrUsed << endl;
   Bucket* oldLargeCosts = m_buckets[ROWS];
   m_buckets[ROWS] = new Bucket(m_map, ROWS*COLUMNS);
   for( RoutingNode* node = oldLargeCosts->dequeue();
        node != NULL;
        node = oldLargeCosts->dequeueUnordered()) {
      enqueue(node);
      // Enqueue increases the value of m_totalNbrUsed
      --m_totalNbrUsed;
   }
   delete oldLargeCosts;
   uint32 stopTime = TimeUtility::getCurrentTime();
   uint32 totTime = stopTime - startTime;
   if ( totTime > 5 ) {
      mc2dbg << "[BH]: tryToEmptyLargeCostsBucket time = "
             << totTime << endl;
   }
}

#else

// ////////////////////////////////////////////////////////////////////////
// Functions for old BucketHeap
// ////////////////////////////////////////////////////////////////////////
BucketHeap::BucketHeap()
{
   m_overflowHasOccurred = false;
   reset();

   for (uint32 i = 0; i < ROWS; i++)
      m_overflowLists[i] = new Head;

   m_largeCostsOverflow = new Head;
}


BucketHeap::~BucketHeap()
{
   for (uint32 i = 0; i < ROWS; i++)
      delete m_overflowLists[i];

   delete m_largeCostsOverflow;
}


void BucketHeap::reset()
{
   if (m_overflowHasOccurred) {
      for (uint32 j = 0; j < ROWS; j++)
         m_overflowLists[j]->clear();
      
      m_largeCostsOverflow->clear();
      m_overflowHasOccurred    = false;      
   }
   
   for (uint32 i = 0; i < ROWS; i++) {
      m_nbrUsed[i] = 0;
#ifdef UNORDERED_BUCKET_HEAP
      m_nbrDestinations[i] = 0;
#endif
   }

   m_nbrUsedInLargeCostsBucket = 0;
   m_heapStartIndex            = 0;
   m_leastExpectedCostInHeap   = 0;
   m_totalNbrUsed              = 0;
}


void BucketHeap::dump()
{
   cout << "--------------------------------------" << endl;
   cout << "Dumping heap data......." << endl;
   for (uint32 i = 0; i < ROWS; i++) {
      if (m_nbrUsed[i]) {
         cout << "Number used in bucket " << i << ": " << m_nbrUsed[i];
         if (!m_overflowLists[i]->empty())
            cout << "Number in overflowList " << i << ": "
                 << m_overflowLists[i]->cardinal();
         cout << "  ===> ";
      }
      
      for (uint32 j = 0; j < m_nbrUsed[i]; j++)
         cout << m_matrix[i][j]->getEstCost(m_map) << " ";
      if (m_nbrUsed[i])
         cout << endl;
   }
   
   cout << "Large costs Bucket: " << m_nbrUsedInLargeCostsBucket <<  endl;
   for (uint32 j = 0; j < m_nbrUsedInLargeCostsBucket; j++)
      cout << m_largeCostsBucket[j]->getEstCost(m_map) << " ";
   if (m_nbrUsedInLargeCostsBucket)
      cout << endl;
   
   cout << "Current status: " << endl;
   cout << "m_heapStartIndex: " << m_heapStartIndex << endl;
   cout << "m_nbrUsed[m_heapStartIndex]: " << m_nbrUsed[m_heapStartIndex]
        << endl;
   cout << "m_leastExpectedCostInHeap: " << m_leastExpectedCostInHeap
        << endl;
   cout << "m_nbrUsedInLargeCostsBucket: " << m_nbrUsedInLargeCostsBucket
        << endl;
   cout << "m_totalNbrUsed: " << m_totalNbrUsed << endl;
   cout << "--------------------------------------" << endl;
}


void BucketHeap::tryToEmptyLargeCostsBucket()
{
   mc2dbg << "[BH]: tryToEmptyLargeCostsBucket - m_totalNbrUsed = "
          << m_totalNbrUsed << endl;
   uint32 nbrElements  = m_nbrUsedInLargeCostsBucket;
   uint32 index        = 0;
   uint32 costsHandled = m_leastExpectedCostInHeap +
      RANGE_OF_COSTS_IN_HEAP;
   
   for (uint32 i = 0; i < nbrElements; i++) {
         // Check if the node should not be touched
      uint32 cost = m_largeCostsBucket[index]->getEstCost(m_map);
      if (cost > costsHandled)
            // Just go to next space
         index++;
      else {
            // replace the node with the last in the bucket
         RoutingNode* nodeToEnqueue = m_largeCostsBucket[index];
         m_largeCostsBucket[index] =
            m_largeCostsBucket[--m_nbrUsedInLargeCostsBucket];
         
            // enqueue the node
         int32 rowIndex;
         if (cost < m_leastExpectedCostInHeap)
            rowIndex = m_heapStartIndex;
         else
            rowIndex = ((cost - m_leastExpectedCostInHeap)
                        >> BUCKET_COST_DIFFERENCE_EXP) + m_heapStartIndex;

         while (rowIndex >= (int32)ROWS)
            rowIndex -= ROWS;

         if (m_nbrUsed[rowIndex] < COLUMNS) {
            m_matrix[rowIndex][m_nbrUsed[rowIndex]++] = nodeToEnqueue;
#ifdef UNORDERED_BUCKET_HEAP
//              if ( nodeToEnqueue->isDest() ) 
//                 m_nbrDestinations[rowIndex]++;
            m_nbrDestinations[rowIndex] += nodeToEnqueue->isDest();
#endif
         } else {
            overflowEnqueue(nodeToEnqueue, rowIndex);
         }
      }
   }
} // tryToEmptyLargeCostsBucket


void BucketHeap::overflowEnqueue(RoutingNode* routingNode, int32 rowIndex)
{
   if (false && !m_overflowHasOccurred) {
      mc2dbg << "Overflow in BucketHeap, normal heap"
             << "Row " << rowIndex << " nbr over "
             << m_overflowLists[rowIndex]->cardinal()
             <<  endl;
   }

   m_overflowHasOccurred = true;
   
   OverflowNode* listNode = new OverflowNode(routingNode);
   if (rowIndex != MAX_INT32)
      listNode->into(m_overflowLists[rowIndex]);
   else
      listNode->into(m_largeCostsOverflow);
}


RoutingNode* BucketHeap::overflowDequeue()
{
   RoutingNode* minNode = NULL;
   RoutingNode* tempNode;
   uint32 minCost = MAX_UINT32;

   bool minimumCostNodeFoundInList = true;

      // First browse the list to find its minimum cost node
   OverflowNode* listNode =
      static_cast<OverflowNode*>(m_overflowLists[m_heapStartIndex]->first());
   OverflowNode* minOverflowNode = listNode;

   while (listNode != NULL) {
      tempNode = listNode->getNode();
      if (tempNode->getEstCost(m_map) < minCost) {
         minNode = tempNode;
         minCost = tempNode->getEstCost(m_map);
         minOverflowNode = listNode;
      }
      listNode = static_cast<OverflowNode*>(listNode->suc());
   }

      // Then browse the bucket
      // See dequeue
   uint32 minIndex = 0;
   RoutingNode** minimumCostBucket = m_matrix[m_heapStartIndex];

   for (uint32 i = 0; i < COLUMNS; i++) {
      tempNode = minimumCostBucket[i];
      if (tempNode->getEstCost(m_map) < minCost) {
         minCost  = tempNode->getEstCost();
         minNode  = tempNode;
         minIndex = i;
         minimumCostNodeFoundInList = false;
      }
   }

   if (minimumCostNodeFoundInList) { // Found min in list
      minOverflowNode->out();
      delete minOverflowNode;
   } else { // found min in bucket
      OverflowNode* firstInList = static_cast<OverflowNode*>
         (m_overflowLists[m_heapStartIndex]->first());
      firstInList->out();
      minimumCostBucket[minIndex] = firstInList->getNode();
      delete firstInList;
#ifdef UNORDERED_BUCKET_HEAP
         // Check if the node is a destination and decrease counter.
//           if ( minNode->isDest() )
//              --m_nbrDestinations[minIndex];
      m_nbrDestinations[m_heapStartIndex] +=
         (minimumCostBucket[minIndex]->isDest() == true);
      m_nbrDestinations[m_heapStartIndex] -= (minNode->isDest() == true);
#endif
   }
   return minNode;
} // overflowDequeue


void BucketHeap::emptyLargeCostsOverflow(uint32 costsHandled)
{
      // This is used to print the MC2INFO statements below only once
   static bool firstLargeCostsOverflow = true;

   if (firstLargeCostsOverflow) {   
      MC2INFO("Trying to empty the overflow of large costs bucket");
      MC2INFO("This is _very_ seldom seen");
      firstLargeCostsOverflow = false;
   }

   OverflowNode* nodeInList = static_cast<OverflowNode*>
      (m_largeCostsOverflow->first());
   
   while (nodeInList != NULL) {
      OverflowNode* newNodeInList = static_cast<OverflowNode*>
         (nodeInList->suc());
      RoutingNode* routingNode = nodeInList->getNode();
      uint32 cost = routingNode->getEstCost(m_map);

      if (cost > costsHandled) {
            // keep this node in the overflow bucket/list
         if (m_nbrUsedInLargeCostsBucket < LARGE_COSTS_BUCKET_SIZE) {
            m_largeCostsBucket[m_nbrUsedInLargeCostsBucket++] = routingNode;
            nodeInList->out();
            delete nodeInList;
         }
      }
      else {
            // put this node at its right place in the heap
         int32 rowIndex = ((cost - m_leastExpectedCostInHeap) >>
                           BUCKET_COST_DIFFERENCE_EXP) + m_heapStartIndex;
         while (rowIndex >= (int32) ROWS)
            rowIndex -= ROWS;

         if (m_nbrUsed[rowIndex] < COLUMNS) {
            m_matrix[rowIndex][m_nbrUsed[rowIndex]++] = routingNode;
#ifdef UNORDERED_BUCKET_HEAP
//              if ( routingNode->isDest() )
//                 m_nbrDestinations[rowIndex]++;
            m_nbrDestinations[rowIndex] += ( routingNode->isDest() == true );
#endif
         } else {
            overflowEnqueue(routingNode, rowIndex);
         }

         nodeInList->out();
         delete nodeInList;
      }
      
      nodeInList = newNodeInList;
   }
} // emptyLargeCostsOverflow


bool BucketHeap::checkHeapConsistency()
{
   bool anyErrors = false;
   uint32 lowestCorrectCost = m_leastExpectedCostInHeap -
      m_heapStartIndex * BUCKET_COST_DIFFERENCE;
   uint32 highestCorrectCost = lowestCorrectCost +
      BUCKET_COST_DIFFERENCE - 1;
   
      // First check the matrix for wrongly placed elements
   for (uint32 i = 0; i < ROWS; i++) {

      if (m_nbrUsed[i] > 0)
         for (uint32 j = 0; j < m_nbrUsed[i]; j++) {
            uint32 cost = m_matrix[i][j]->getEstCost(m_map);
            if ((cost < lowestCorrectCost) ||
                (cost > highestCorrectCost)) {
               anyErrors = true;
               cout << "Consistency failure" << endl;
               cout << "Bucket, space " << i << ", " << j << endl;
               cout << "Cost range in bucket is between " <<
                  lowestCorrectCost << " and " << highestCorrectCost
                    << endl;
               cout << "Cost found is " << cost << endl;
               RoutingNode* noNode = NULL;
               noNode->getItemID();
            }
         }
      
      if (!m_overflowLists[i]->empty()) {
         cout << "overflowList " << i << " is not NULL" << endl;
         if (m_overflowHasOccurred) {
            anyErrors = true;
            cout << "overflowList is not empty, but m_overflowHasOccurred"
                 << " says true" << endl;
            RoutingNode* noNode = NULL;
            noNode->getItemID();
         }
      }
      
      lowestCorrectCost  += BUCKET_COST_DIFFERENCE;
      highestCorrectCost += BUCKET_COST_DIFFERENCE;
   }

      // And now the large costs Bucket
   uint32 minimumTooLargeCost = m_leastExpectedCostInHeap +
      ROWS * BUCKET_COST_DIFFERENCE;
   for (uint32 k = 0; k < m_nbrUsedInLargeCostsBucket; k++)
      if (m_largeCostsBucket[k]->getEstCost(m_map) < minimumTooLargeCost) {
         cout << "Consistency failure" << endl;
         cout << "Space " << k << " in large costs bucket" <<endl;
         cout << "The lowest cost to be too large is " <<
              minimumTooLargeCost << endl;
         cout << "Cost found is " << m_largeCostsBucket[k]->getEstCost(m_map)
              << endl;
         anyErrors = true;
      }

   if (!m_largeCostsOverflow->empty()) {
      MC2INFO("large costs bucket's overflow list is not NULL");
      if (m_overflowHasOccurred) {
         anyErrors = true;
         cout << "This will never happen" << endl;
         RoutingNode* noNode = NULL;
         noNode->getItemID();
      }
   }

   return anyErrors;
} // checkHeapConsistency
#endif
