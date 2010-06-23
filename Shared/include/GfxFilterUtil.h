/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFX_FILTER_UTIL_H
#define GFX_FILTER_UTIL_H

#include "config.h"
#include "math.h"
#include "stdlib.h"

#include "GfxUtility.h"

#include "MC2Coordinate.h"
#include <vector>
#include <list>

/**
 *                     -+-+-+- READ ME -+-+-+-
 * 
 * To gain greater understanding of this code one should read 
 * Alexander Kolesnikovs paper "A fast near-optimal min-# polygonal
 * approximation of digitized curves" (P1) or "Reduced-search dynamic 
 * programing for approximation of polygonal curves" (P2).
 * Both papers can be found (@ the time this was written) on 
 * Alexanders homepage http://www.cs.joensuu.fi/~koles/. 
 * 
 * "iterations" tells the algorithm how many times it should
 * try to run on its own out data as in data to improve it. If
 * will break if no improvement is done at any given step. It is
 * reasonable to keep this value around 10.
 *
 * "maxSizeOfSegment" tells the algorithm how long pieces of a 
 * polygon it should try to optimize in one run, if a polygon
 * is longer than this it will be split up into pieces that are 
 * approximately maxSizeOfSegment long. This is done to avoid
 * cases where N and M are large and thus time consuming.
 * 
 * "W" should be between 6 and 8. Altho it should be ok to use
 * anything >= 1. If W = 1 is used this filter will do 
 * nothing useful. If W >= M a complete search of the statespace
 * will be done and we guarantee global minimum according to
 * the given costFunction. While W < M we can not give such
 * guarantees altho it is usually the case that we find global
 * minimum if we keep W between 6 - 8.
 * 
 */
class GfxFilterUtil {
   public:

      /**
       * Calculates the smallest distance between point k and line i - j 
       * times the distance between points k-1 and k+1 divided by 2.
       * 
       * @param i the index of point i.
       * @param j the index of point j.
       * @param k the index of point k.
       * @param x the x coords.
       * @param y the y coords.
       * @param nbrOfPointsInBetweenij the number of points in the 
       * polygon between point i and point j.
       * 
       * @return the cost for point k in regard to line i-j.
       */
      static double dist( int i, int j, int k,
            int* x, int* y );

      /** 
       * Calculates the sum of all cost on the line Pi - Pj and the point.
       * 
       * @param i the index for one of the points we want to use as 
       * an approximation of part of the polygon.
       * @param j the index for one of the points we want to use as 
       * an approximation of part of the polygon.
       * @param x contains the x values of the coordinates of the polygon.
       * @param y contains the y values of the coordinates of the polygon.
       *
       * @return the cost for approximating part of the polygon with 
       * points i and j. That is removing all the points between i and j.
       */
      static double costFunction( int i, int j, int* x, int* y);

      /**
       * Returns the total error in an approximation. The error
       * is calculated using costFunction().
       *
       * @param referencePath tha path that we use as a guide when we 
       * determine where to search to be as efficient as possible.
       * @param xyhelper converts from mc2coords to non projected coords.
       * @param pointsBegin doh.
       * @param pointsEnd doooh.
       *
       * @return Return the total error of the approximation.
       */ 
      template<class INT_SEQUENCE, class XYHELPER, class POINT_ITERATOR>
         static double getTotalError( 
               const INT_SEQUENCE& referencePath,
               const XYHELPER& xyhelper,
               const POINT_ITERATOR& pointsBegin,
               const POINT_ITERATOR& pointsEnd );


      /**
       * Calls the next step in the filtering process and ensures that 
       * some of the inparams is set to values that wont create a
       * problem for the later steps in the algorithm.
       * 
       * @param outIndeces the container that we return our final 
       * indeces to.
       * @param referencePath tha path that we use as a guide when 
       * we determine where to search to be as efficient as possible.
       * @param xyhelper converts from mc2coords to non projected coords.
       * @param pointsBegin doh.
       * @param pointsEnd doooh.
       * @param W tells the algirithm how wide the bounding corridor 
       * should be, good values seams to be around 6-8. 
       * 
       */
      template<class INT_PUSH_BACKABLE, class INT_SEQUENCE,
      class XYHELPER, class POINT_ITERATOR>
         static double filter(
               INT_PUSH_BACKABLE& outIndeces,
               const INT_SEQUENCE& referencePath,
               const XYHELPER& xyhelper,
               const POINT_ITERATOR& pointsBegin,
               const POINT_ITERATOR& pointsEnd,
               int   W, 
               int   iterations,
               int   maxSizeOfSegment );

      /**
       * Iterates the next step of the algorithm until it dosent
       * give any improvements or until "iterations" iterations
       * have been done.
       *
       * @param outIndeces the container that we return our final 
       * indeces to.
       * @param referencePath tha path that we use as a guide when 
       * we determine where to search to be as efficient as possible.
       * @param xyhelper converts from mc2coords to non projected coords.
       * @param pointsBegin doh.
       * @param pointsEnd doooh.
       * @param W tells the algirithm how wide the bounding corridor 
       * should be, good values seams to be around 6-8. 
       * 
       */
      template<class INT_PUSH_BACKABLE, class INT_SEQUENCE,
      class XYHELPER, class POINT_ITERATOR>
         static void iterationFilter(
               INT_PUSH_BACKABLE& outIndeces,
               const INT_SEQUENCE& referencePath,
               const XYHELPER& xyhelper,
               const POINT_ITERATOR& pointsBegin,
               const POINT_ITERATOR& pointsEnd,
               int   W, 
               int   iterations,
               int   maxSizeOfSegment );
      /**
       * Splits the polygon in pieces that are around maxSizeOfSegment 
       * big and lets the final step of the algorithm run on those pieces.
       * This is done to speed things up a bit.
       * 
       * @param outIndeces the container that we return our final 
       * indeces to.
       * @param referencePath tha path that we use as a guide when 
       * we determine where to search to be as efficient as possible.
       * @param xyhelper converts from mc2coords to non projected coords.
       * @param pointsBegin doh.
       * @param pointsEnd doooh.
       * @param W tells the algirithm how wide the bounding corridor 
       * should be, good values seams to be around 6-8. 
       * 
       */
      template<class INT_PUSH_BACKABLE, class INT_SEQUENCE,
      class XYHELPER, class POINT_ITERATOR>
         static void polygonSplitFilter(
               INT_PUSH_BACKABLE& outIndeces,
               const INT_SEQUENCE& referencePath,
               const XYHELPER& xyhelper,
               const POINT_ITERATOR& pointsBegin,
               const POINT_ITERATOR& pointsEnd,
               int   W, 
               int   maxSizeOfSegment );

      /**
       * Takes a polygon and some indexes as in data that gives an 
       * approximation of the given polygon, that is try to represent
       * it in some way. The filter then try to improve the 
       * approximation using an algorithm that is described in P1 and P2.  
       * Returns the total error for the approximation and does a pushback
       * of al the indexes that it considers to be useful.
       *
       * @param outIndeces the container that we return our final 
       * indeces to.
       * @param referencePath tha path that we use as a guide when 
       * we determine where to search to be as efficient as possible.
       * @param xyhelper converts from mc2coords to non projected coords.
       * @param pointsBegin doh.
       * @param pointsEnd doooh.
       * @param W tells the algirithm how wide the bounding corridor 
       * should be, good values seams to be around 6-8. 
       * 
       */
      template<class INT_PUSH_BACKABLE, class INT_SEQUENCE, 
      class XYHELPER, class POINT_ITERATOR>
         static void runFilter(
               INT_PUSH_BACKABLE& outIndeces,
               const INT_SEQUENCE& rsPath,
               const XYHELPER& xyhelper,
               const POINT_ITERATOR& pointsBegin,
               const POINT_ITERATOR& pointsEnd,
               int   W );
};

template<class INT_PUSH_BACKABLE, class INT_SEQUENCE, 
   class XYHELPER, class POINT_ITERATOR>
   double GfxFilterUtil::filter(
         INT_PUSH_BACKABLE& outIndeces,
         const INT_SEQUENCE& referencePath,
         const XYHELPER& xyhelper,
         const POINT_ITERATOR& pointsBegin,
         const POINT_ITERATOR& pointsEnd,
         int W = 9, 
         int iterations = 10,
         int maxSizeOfSegment = 1000 ) {

      // Help confused users set good values if they try to set bad
      // values so that we dont give realy poor results..
      if( W < 3 ) {
         W = 3;
      }
      if( iterations <= 1) {
         iterations = 1;      
      }
      if( maxSizeOfSegment < 10 ) {
         maxSizeOfSegment = 10;
      }

      // Let iterationFilter do its iterative work.   
      GfxFilterUtil::iterationFilter( 
            outIndeces, 
            referencePath, 
            xyhelper,
            pointsBegin,
            pointsEnd,
            W,
            iterations,
            maxSizeOfSegment );

      // Return the error for this polygon
      return GfxFilterUtil::getTotalError(
            outIndeces,
            xyhelper,
            pointsBegin, pointsEnd);

   }

template<class INT_PUSH_BACKABLE, class INT_SEQUENCE, 
   class XYHELPER, class POINT_ITERATOR>
   void GfxFilterUtil::iterationFilter(
         INT_PUSH_BACKABLE& outIndeces,
         const INT_SEQUENCE& referencePath,
         const XYHELPER& xyhelper,
         const POINT_ITERATOR& pointsBegin,
         const POINT_ITERATOR& pointsEnd,
         int W, 
         int iterations,
         int maxSizeOfSegment ) {

      // Calculate the error so that we might cut the search
      // after just one run if it dosent improve the path.
      double error = GfxFilterUtil::getTotalError(
            referencePath,
            xyhelper,
            pointsBegin, pointsEnd);

      // Use to indeces vectors so that we can swap between them
      // after each run of the algorithm and use out data as 
      // in data in the next run.
      vector<int> indecesB;
      vector<int> indecesA;

      // Put in data in indecesB for the first run to work.
      for ( typename INT_SEQUENCE::const_iterator 
            it = referencePath.begin();
            it != referencePath.end(); ++it ) {
         indecesA.push_back( *it );
      }
      // Iterate over indata until ( i < iterations ) or local
      // optimum found. ( temp_error == error )
      int i = 0;
      while( i < iterations ) {
         mc2dbg8 << "GfxFilterUtil::iterationFilter i:" << i << endl;
         // Swap indata and out data.
         indecesA.swap( indecesB );
         indecesA.clear();
         i++;
         GfxFilterUtil::polygonSplitFilter( 
               indecesA, 
               indecesB, 
               xyhelper,
               pointsBegin,
               pointsEnd,
               W,
               maxSizeOfSegment );
         // Calculate the error so that we can break the search if 
         // we have reached a local optimum.
         double temp_error = GfxFilterUtil::getTotalError(
               indecesA,
               xyhelper,
               pointsBegin, pointsEnd); 
         mc2dbg8 << "GfxFilterUtil::iterationFilter temp_error:" 
                << temp_error << endl;

         // Break on local optimum.
         if( temp_error >= error ) {
            i = iterations;
         }
         error = temp_error;
      }
      // Send the resulting path back.
      for ( typename INT_SEQUENCE::const_iterator 
            it = indecesA.begin(); 
            it != indecesA.end(); ++it ) {
         outIndeces.push_back( *it );
      }  
   }

template<class INT_PUSH_BACKABLE, class INT_SEQUENCE, 
         class XYHELPER, class POINT_ITERATOR>
   void GfxFilterUtil::polygonSplitFilter(
         INT_PUSH_BACKABLE& outIndeces,
         const INT_SEQUENCE& referencePath,
         const XYHELPER& xyhelper,
         const POINT_ITERATOR& pointsBegin,
         const POINT_ITERATOR& pointsEnd,
         int W, 
         int maxSizeOfSegment ) {
      // Calculate how big the referencePathLength is. That is how many 
      // vertexes the refferencePath consists of.   
      int M = 0;
      for ( typename INT_SEQUENCE::const_iterator 
            it = referencePath.begin();
            it != referencePath.end(); ++it ) {
         M++;
      }

      // Create and init the reference path (index values in this array).
      // gets the indexes from a INT_SEQUENCE and puts them in a int array.
      int* rsPath = new int[M];
      int M_index = 0;
      for ( typename INT_SEQUENCE::const_iterator 
            it = referencePath.begin();
            it != referencePath.end(); ++it ) {
         mc2dbg8 << "Ref path:" << *it << endl;
         rsPath[M_index] = *it;
         ++M_index;
      }

      // Points to begin and end getting x and y values from.   
      POINT_ITERATOR localPointsBegin = pointsBegin;
      POINT_ITERATOR localPointsEnd = pointsBegin;

      // Pushback the first point in the path ( it is always 0 ) so that
      // we dont have to worry about that later when we dont pushback the 
      // first one in every segment. If we would pushback the first one 
      // in every segment that would overlapp with the last one in the 
      // pervious segment.
      outIndeces.push_back( 0 );

      // Init important split values.
      int startCoord = 0;
      int endCoord = 0;
      int t = 0;
      int old_t = 0;
      // Do this until we reach the end of the path.
      while( endCoord < rsPath[ M - 1 ]) {
         endCoord += maxSizeOfSegment;
         if( endCoord > rsPath[ M - 1 ] ) {
            endCoord = rsPath[ M - 1 ];
         }
         // Search for the next point that is in that rsPath.
         // And use that point as the last point in this split.
         while( rsPath[ t ] < endCoord && t < M - 1 ) { t++; }
         endCoord = rsPath[ t ];

         // Select all the indeces that we want to use in this 
         // split.
         vector<int> tempRSPath;
         for( int i = old_t; i <= t; i++ ) {
            tempRSPath.push_back( rsPath[ i ] - startCoord );
         }


         // Select all the x and y values that we want to use in this
         // split.
         for( int i = startCoord; i <= endCoord; i++) {
            localPointsEnd++;
         }
         vector<int> tempIndeces;
         // Run the filter on this part of the polygon.
         mc2dbg8 << "Running filter on spilt start: " << startCoord
                << " end: " << endCoord << endl;
         GfxFilterUtil::runFilter( 
               tempIndeces, 
               tempRSPath, 
               xyhelper, 
               localPointsBegin, 
               localPointsEnd, 
               W );

         typename INT_SEQUENCE::const_iterator it = tempIndeces.begin(); 
         it++;      
         mc2dbg4 << "ut: ";
         // Send the resulting part of the path back.
         for ( ; it != tempIndeces.end(); ++it ) {
            outIndeces.push_back( *it + startCoord );
            mc2dbg4 << *it << " "; 
         }
         mc2dbg4 << endl;

         // Init some values for tha next part of the polygon.
         localPointsBegin = --localPointsEnd;
         startCoord = endCoord;
         old_t = t;
      }
      // Clean up
      delete[] rsPath;
      rsPath=NULL;
   }
template<class INT_PUSH_BACKABLE, class INT_SEQUENCE, 
   class XYHELPER, class POINT_ITERATOR>
   void GfxFilterUtil::runFilter( 
         INT_PUSH_BACKABLE& outIndeces,
         const INT_SEQUENCE& referencePath, 
         const XYHELPER& xyhelper,
         const POINT_ITERATOR& pointsBegin,
         const POINT_ITERATOR& pointsEnd,
         int W )
{
   // If the user is stupid and wants to use a W that is to small,
   // we will help him/her and correct the errors of it's ways.
   if( W < 3 ) {
      W = 3;
   }

   // Calculate how big N is. That is how many vertexes the 
   // original polygon consists of.
   int N = 0;
   for( POINT_ITERATOR it = pointsBegin;
         it != pointsEnd; ++it ) {
      N++;
   }

   if( N <= 2 ) {
      outIndeces.push_back( 0 );
      if( N == 2 ) {
         outIndeces.push_back( 1 );
         return ;
      }
      return ;
   }

   // Calculate how big the referencePathLength is. That is how many 
   // vertexes the refferencePath consists of.   
   int M = 0;
   for ( typename INT_SEQUENCE::const_iterator 
         it = referencePath.begin();
         it != referencePath.end(); ++it ) {
      M++;
   }

   // Create and init the reference path (index values in this array).
   // gets the indexes from a INT_SEQUENCE and puts them in a int array.
   int* rsPath = new int[M];
   int M_index = 0;
   for ( typename INT_SEQUENCE::const_iterator 
         it = referencePath.begin();
         it != referencePath.end(); ++it ) {
      rsPath[M_index] = *it;
      ++M_index;
   }

   DEBUG4(
   mc2dbg4 << "Path before DP: ";
   for( int i = 0; i < M; i++ ) {
      mc2dbg4 << rsPath[i] << " ";
   }
   mc2dbg4 << endl;
   );

   // Precalculates the boundaries for the bounding corridor.
   // More can be read about this in Kolesnikovs paper. L, R, 
   // T, B stands for Left, Right, Top, Bottom just as one
   // might suspect.
   int c1 = W / 2;
   int c2 = W - c1;

   mc2dbg4 << "Creating bounds." << endl;

   // Create and fill L with bounding coords.
   int* L = new int[ M + 1 ];
   for( int m = 1; m < M; m++ ) {
      if( m < c1 ) {
         L[ m ] = m;
      } else {
         L[ m ] = max( m, rsPath[ m - c1 ] );
      }
   }
   L[ 0 ] = 0;
   L[ M ] = N;

   mc2dbg4 << "L done." << endl;

   // Create and fill R with bounding coords.
   int* R = new int[ M + 1];
   for( int m = 1; m < M; m++ ) {
      if(m < M - c2){
         R[ m ] = min( N - ( M - m ), rsPath[ m + c2 ] - 1 );
      } else {
         R[ m ] = N - ( M - m );
      }
   } 
   R[ 0 ] = 0;
   R[ M ] = N;

   mc2dbg4 << "R done." << endl;

   // Create and fill B with bounding coords.
   int* B = new int[ N + 1];
   int t = 0;
   int m = 0;
   for( int n = 1; n < N; n++ ) {
      if( n >= rsPath[ t ] ) {
         m = ++t - c1 - ( W % 2 );
         if( m < 1 ) {
            m = 1;
         }
         if( m >= N ) {
            m = N - 1;
         }
      }
      if( m < n - ( N - M ) ) {
         m = n - ( N - M );
      }                
      B[ n ] = m;
   }
   B[ 0 ] = 0;
   B[ N ] = M;

   mc2dbg4 << "B done." << endl;

   // Create and fill T with bounding coords.
   int* T = new int[ N + 1 ];
   t = 0;
   m = 0;
   for( int n = 1; n < N; n++ ) {
      if( n >= rsPath[ t ] ) {
         m = ++t + c1 - 1;
         if( m >= M ) {
            m = M - 1;
         }
         if( m > n ) {
            t--;
            m = n;
         }
      }
      T[ n ] = m;
   }
   T[ 0 ] = 0;
   T[ N ] = M;

   mc2dbg4 << "T done." << endl;

   // Arrays to store the points x and y values in.
   int* x = new int[ N + 1 ];
   int* y = new int[ N + 1 ];
   for(int i = 0; i <= N; i++) {
      x[ i ] = 0;
      y[ i ] = 0;
   }
   // Array to store precalculated approximation error values in.
   double* v = new double[ N + 1 ];
   for(int i = 0; i <= N; i++) {
      v[ i ] = 0;
   }

   mc2dbg4 << "Put the x's and y's in arrays for later use." << endl;

   // Put the x's and y's in arrays for later use.
   int i = 0;
   for( POINT_ITERATOR it = pointsBegin;
         it != pointsEnd;
         ++it ) {
      mc2dbg4 << "x " << xyhelper.getX(*it) << 
         " y " << xyhelper.getY(*it) << endl;      
      x[ i ] = xyhelper.getX(*it);
      y[ i++ ] = xyhelper.getY(*it);
   }

   mc2dbg4 << "Creating statespace." << endl;

   // Create statespace array. In this array we will keep track of 
   // the costs of smaler instances of the current problem. That is
   // smaller M and N values, the errors for the smaller instances
   // are used to calculate the larger instances with dynamic
   // programming.
   double** D = new double*[ N + 1 ];
   for ( int i = 0; i <= N; ++i ) {
      D[ i ] = new double[ W + 1 ]; 
   }

   mc2dbg4 << "Creating parrent array." << endl;  

   // Create parrent array. This is used for backtracking after
   // the dp part is done. Backtracking is done from A[ N ][ 0 ] 
   // to A[ 0 ][ 0 ].
   int** A = new int*[ N + 1 ];
   for ( int i = 0; i <= N; ++i ) {
      A[ i ] = new int[ W + 1 ]; 
   }

   mc2dbg4 << "N = " << N << " M = " << M << " " << endl;

   // Init the statespace array, everything inside should be 0.
   // The same goes for the A (parrent) array.
   for( int n = 0; n <= N; n++ ) {
      for( int w = 0; w <= W; w++ ) {
         D[ n ][ w ] = 0;
         A[ n ][ w ] = 0;
      }
   }

   mc2dbg4 << "DP run." << endl;

   // Reduced-search algorithm start, a good description of this 
   // can be found in P2.
   for( int n = 2; n <= N; n++ ) {
      // Precalculate the costFunction before we enter the "m" loop
      // and gain speed this way.
      for( int j = L[ B[ n ] - 1 ]; j <= n - 1; j++ ) {
         v[ n - j ] = costFunction( j, n, x, y );
      }
      // Loop between bottom and top of the corridor.
      for( int m = B[ n ]; m <= T[ n ]; m++ ) {
         double cmin = DBL_MAX;
         int jmin = -1;
         for( int j = L[ m - 1 ]; j <= min( n - 1, R[ m - 1 ] ); j++ ) {
            // "- B[ j ]" is used to puch the size of D down to N * W
            // instead  of N * M that would be needed if we used 
            // D[ j ][ m - 1 ] when writing to and reading from the 
            // statespace.
            // What we do here is calculate the cost of paths that are 
            // longer and loger as we progres in the statespace untill 
            // we find a path that is M long. That is when we reach the
            // end of the search in the statespace array and backtracks 
            // to see what path was the minimal one.
            double c = D[ j ][ m - 1 - B[ j ] ] + v[ n - j ];
            if( c < cmin ) {
               cmin = c;
               jmin = j;
            }
         }
         // again "- B[ n ]" is used to keep size of both A and D down.
         // A is used to keep track of al the j indeces that got us to 
         // where we are in the end. A and the values keept inside will
         // help us find the correct way from end to start when we 
         // backtrack. 
         D[ n ][ m - B[ n ] ] = cmin;
         A[ n ][ m - B[ n ] ] = jmin;
         mc2dbg4 << D[n][m-B[n]] << " ";
      }
      mc2dbg4 << endl;
   }

   // Backtracking to find resulting path in A.
   // Create array to temporary store the new path.
   int* H = new int[ M ];
   for( int m = 0; m < M; ++m ) {
      H[ m ] = 0;
   }
   // Backtracking begins at A[ N ][ 0 ] (that would have been A[ N ][ M ]
   // if we dident use "- B[ n ]" to reduce memmory usage).
   H[ M - 1 ] = A[ N ][ 0 ];
   for( int m = M - 1; m >= 1; m-- ) {
      H[ m - 1 ] = A[ H[ m ] ][ m - B[ H[ m ] ] ];
      mc2dbg4 << H[ m ] << " ";
   }
   mc2dbg4 << endl;
   // The first and last point in the path needs to be 0 and N - 1.
   H[ 0 ] = 0;
   H[ M - 1 ] = N - 1;

   // Send the indeces that the backtracking resulted in back using
   // push_back.
   mc2dbg4 << "Resulting path: ";
   for( int m = 0; m < M ; m++ ) {
      mc2dbg4 << H[ m ] << " ";
      outIndeces.push_back( H[ m ] );
   }
   mc2dbg4 << endl; 

   // Due to "- B[ n ]" the total error will be found in D[ N ][ 0 ] and
   // not in D[ N ][ M ] as one could expect.
   //double error = D[ N ][ 0 ];

   // Reduced-search algorithm end

   // Delete arrays.
   delete[] H;

   delete[] x;
   delete[] y;

   delete[] v;

   delete[] rsPath;

   delete[] L;
   delete[] R;
   delete[] B;
   delete[] T;

   for ( int i = 0; i <= N; i++ ) {
      delete[] A[ i ];
      delete[] D[ i ];
   }
   delete[] A;
   delete[] D;

   // Delete complete.
}

inline double 
GfxFilterUtil::costFunction( int i, int j, int* x, int* y)
{
   double cost = 0;
   if( i > j ){
      int temp = i;
      i = j;
      j = temp;
   }
   for( int k = i; k < j; k++ ){
      cost += GfxUtility::closestDistVectorToPoint(
            x[ i ], y[ i ], x[ j ], y[ j ], x[ k ], y[ k ], 1 ) + 
         GfxUtility::squareP2Pdistance_linear(
               x[ i ], y[ i ], x[ j ], y[ j ], 1 ) / 2;
   }
   return cost;
}

template<class INT_SEQUENCE, class XYHELPER, class POINT_ITERATOR>
double GfxFilterUtil::getTotalError( 
      const INT_SEQUENCE& referencePath,
      const XYHELPER& xyhelper,
      const POINT_ITERATOR& pointsBegin,
      const POINT_ITERATOR& pointsEnd )
{

   // Calculate how big N is. That is how many vertexes the 
   // original polygon consists of.
   int N = 0;
   for( POINT_ITERATOR it = pointsBegin;
         it != pointsEnd; ++it ) {
      N++;
   }

   // This one seems unnecessary.
   // Array to store precalculated approximation error values in.
   //double* v = new double[ N ];
   //for(int i = 0; i != N; i++) {
   //   v[ i ] = 0;
   //}

   // Arrays to store the x and y coords in.
   int* x = new int[ N ];
   int* y = new int[ N ]; 
   for(int i = 0; i != N; i++) {
      x[ i ] = 0;
      y[ i ] = 0;
   }

   //Put the x's and y's in arrays for later use.
   int i = 0;
   for( POINT_ITERATOR it = pointsBegin;
         it != pointsEnd;
         ++it ) {
      x[ i ] = xyhelper.getX(*it);
      y[ i++ ] = xyhelper.getY(*it);
   }

   // Calculate how big the referencePathLength is. That is how many 
   // vertexes the referencePath consists of.
   int M = 0;
   for ( typename INT_SEQUENCE::const_iterator it = referencePath.begin();
         it != referencePath.end(); ++it ) {
      M++;
   }
   // Create and init the reference path (index values in this array).
   // gets the indexes from a INT_SEQUENCE and puts them in a int array.
   int* rsPath = new int[M];
   int M_index = 0;
   for ( typename INT_SEQUENCE::const_iterator it = referencePath.begin();
         it != referencePath.end(); ++it ) {
      rsPath[M_index] = *it;
      ++M_index;
   }

   double totalError = 0;

   // Loop through the approximation.
   for( int i = 1; i < M; i++ ) {
      // Calculate the cost for each an d everyone of the new lines
      // in the approximated polygon. Sum them up and return the
      // total error.
      totalError += costFunction( rsPath[i - 1], rsPath[ i ], x, y );
   }
   // Clean up.
   delete[] x;
   x=NULL;
   delete[] y;
   y=NULL;
   delete[] rsPath;
   rsPath=NULL;

   return totalError;
}


class MC2CoordXYHelper {
   public:
      inline int32 getX(const MC2Coordinate& coord) const {
         return int32( coord.lon * 
               GfxUtility::getCoslat( coord.lat, coord.lat ) );
      }

      inline int32 getY(const MC2Coordinate& coord) const {
         return coord.lat;
      }
};

#endif
