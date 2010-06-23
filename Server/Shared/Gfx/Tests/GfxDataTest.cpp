/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxData.h"
#include "MC2Coordinate.h"
#include "Stack.h"
#include "MC2UnitTestMain.h"
#include "GfxDataFull.h"
#include "GMSGfxData.h"


MC2_UNIT_TEST_FUNCTION( convexHullTest ) {

    GfxDataFull gfxData;
    /*
     * Create a polygon with five points that looks like this:
     *
     *                               /-3
     *                         /-----  |
     *                   /-----        |
     *              /----              |
     *        /-----                    \
     *     2--                          |
     *      \                           |
     *       \                          |
     *        \                       --4
     *         1                 ----/
     *        /             ----/
     *       /          ---/
     *      /      ----/
     *     /  ----/
     *    0 -/
     *
     * The convext hull should be 0, 2, 3, and 4.
     *
     */
    
    cout << "getConvexHull() test1: " << endl;

    MC2_TEST_REQUIRED( gfxData.addCoordinate( 0, 0, true ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 500, 200 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 1000, 100 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 1500, 1000 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 500, 500 ) ) );

    Stack stack;
    bool convexHullCreated = gfxData.getConvexHull( &stack, 0);

    MC2_TEST_REQUIRED( convexHullCreated );

    for( uint32 i = 0; i < stack.getStackSize(); ++i)
    {
        cout << stack.getElementAt(i) << endl;
    }

    MC2_TEST_REQUIRED( stack.getStackSize() == 5 );
    MC2_TEST_REQUIRED( stack.getElementAt( 0 ) == 4 );
    MC2_TEST_REQUIRED( stack.getElementAt( 1 ) == 0 );
    MC2_TEST_REQUIRED( stack.getElementAt( 2 ) == 2 );
    MC2_TEST_REQUIRED( stack.getElementAt( 3 ) == 3 );
    MC2_TEST_REQUIRED( stack.getElementAt( 4 ) == 4 );

    cout << endl;
}

MC2_UNIT_TEST_FUNCTION( convexHullTest2 ) {

    GfxDataFull gfxData;
    /*
     * To confirm the fix of a bug that previously caused a crash; when the 
     * two of lowest angle (not counting the one to the farthest west) from 
     * north line were on a straight line:
     *
     *
     *
     *              2----
     *             /     \-----
     *            /            \-----
     *           1                   \-----
     *          /                          \-----
     *         /                                 \-3
     *        /                  ------------------
     *       0------------------/
     *
     *
     *
     *
     *
     *
     * The convex hull should be 0, 2 and 3.
     *
     */

    cout << "getConvexHull() test2: " << endl;

    MC2_TEST_REQUIRED( gfxData.addCoordinate( 0, 0, true ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 500, 500 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 1000, 1000 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 200, 1500 ) ) );

    Stack stack;
    bool convexHullCreated = gfxData.getConvexHull( &stack, 0);

    MC2_TEST_REQUIRED( convexHullCreated );

    for( uint32 i = 0; i < stack.getStackSize(); ++i)
    {
        cout << stack.getElementAt(i) << endl;
    }

    MC2_TEST_REQUIRED( stack.getStackSize() == 4 );
    MC2_TEST_REQUIRED( stack.getElementAt( 0 ) == 3 );
    MC2_TEST_REQUIRED( stack.getElementAt( 1 ) == 0 );
    MC2_TEST_REQUIRED( stack.getElementAt( 2 ) == 2 );
    MC2_TEST_REQUIRED( stack.getElementAt( 3 ) == 3 );

    cout << endl;
}

MC2_UNIT_TEST_FUNCTION( convexHullTest3 ) {

    GfxDataFull gfxData;
    /*
     * Changed algorithm to start with the most south west point (that is,
     * not only requiring furthest west) in order to fix bug. Test to 
     * make sure this works ok when we have several points on a line
     * to the far west. 
     *
     *
     *      3----
     *      |    \-------
     *      |            \------
     *      |                   \------
     *      |                          \-------
     *      2                                  \------
     *      |                                         \---4 
     *      |                                         ----
     *      |                                  ------/
     *      1                          -------/
     *      |                   ------/
     *      |            ------/
     *      |    -------/
     *      0---/
     *
     * The convex hull should be 0, 3 and 4.
     *
     */

    cout << "getConvexHull() test3: " << endl;

    MC2_TEST_REQUIRED( gfxData.addCoordinate( 0, 0, true ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 500, 0 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 1000, 0 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 1500, 0 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 750, 1000 ) ) );

    MC2_TEST_REQUIRED( gfxData.getNbrCoordinates( 0 ) == 5 );

    Stack stack;
    bool convexHullCreated = gfxData.getConvexHull( &stack, 0);

    MC2_TEST_REQUIRED( convexHullCreated );

    for( uint32 i = 0; i < stack.getStackSize(); ++i)
    {
        cout << stack.getElementAt(i) << endl;
    }

    MC2_TEST_REQUIRED( stack.getStackSize() == 4 );
    MC2_TEST_REQUIRED( stack.getElementAt( 0 ) == 4  );
    MC2_TEST_REQUIRED( stack.getElementAt( 1 ) == 0 );
    MC2_TEST_REQUIRED( stack.getElementAt( 2 ) == 3 );
    MC2_TEST_REQUIRED( stack.getElementAt( 3 ) == 4 );

    cout << endl;
}

MC2_UNIT_TEST_FUNCTION( convexHullTest4 ) {

    GfxDataFull gfxData;
    /*
     *
     *   Check that when two points have the same coordinates
     *   only one of these points is included in the convex hull.
     *
     *   Note; the sorting (seems to be) stable, that is, the order of two
     *   points with equal coordinates are kept after the sorting. This
     *   means, for a pair of points with equal coordinates, that the 
     *   one with the highest index will be included in the convex hull.
     *   The exception is for points that have the highest angle clockwise
     *   from north (the sorting order), where the one with the highest index
     *   will initially be placed on the stack. 
     *
     *      3----
     *      |    \-------
     *      |            \------
     *      |                   \------
     *      |                          \-------
     *      2, 7                               \------
     *      |                                         \---4, 8
     *      |                                         ----
     *      |                                  ------/
     *      1, 6                        -------/
     *      |                   ------/
     *      |            ------/
     *      |    -------/
     *      0, 5---/
     *
     * The convex hull should be 0, 3 and 4.
     *
     */

    cout << "getConvexHull() test4: " << endl;

    MC2_TEST_REQUIRED( gfxData.addCoordinate( 0, 0, true ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 500, 0 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 1000, 0 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 1500, 0 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 750, 100 ) ) );
    // Duplicate of #0
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 0, 0 ) ) );
    // Duplicate of #1
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 500, 0 ) ) );    
    // Duplicate of #2 
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 1000, 0 ) ) );    
    // Duplicate of #4
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 750, 100 ) ) );


    MC2_TEST_REQUIRED( gfxData.getNbrCoordinates( 0 ) == 9 );

    Stack stack;
    bool convexHullCreated = gfxData.getConvexHull( &stack, 0);

    MC2_TEST_REQUIRED( convexHullCreated );

    for( uint32 i = 0; i < stack.getStackSize(); ++i)
    {
        cout << stack.getElementAt(i) << endl;
    }

    MC2_TEST_REQUIRED( stack.getStackSize() == 4 );
    MC2_TEST_REQUIRED( stack.getElementAt( 0 ) == 8  );
    MC2_TEST_REQUIRED( stack.getElementAt( 1 ) == 0 );
    MC2_TEST_REQUIRED( stack.getElementAt( 2 ) == 3 );
    MC2_TEST_REQUIRED( stack.getElementAt( 3 ) == 4 );

    cout << endl;
}

MC2_UNIT_TEST_FUNCTION( convexHullTest5 ) {

    GfxDataFull gfxData;
    /*
     * More complex test case with larger number of points, and also, 
     * more colinear points.
     *
     *      2-------             /------4\
     *      |       \----3-------         \
     *      |                              \
     *      1                               \
     *      |                                \
     *      |                /9               5
     *      |               /  -\        /7    \
     *      0              /     \     -/  -\   \
     *      |             10      -\ -/      -\  \
     *      |            /          \8         -\ \
     *      12\         /                        --6
     *         ----\   /
     *              ---11
     *
     *
     * The convex hull should be 2, 4, 6, 11, 12.     
     *
     */

    cout << "getConvexHull() test5: " << endl;

    MC2_TEST_REQUIRED( gfxData.addCoordinate( 400, 100, true ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 600, 100 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 800, 100 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 600, 400 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 700, 800 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 500, 900 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 100, 1100 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 400, 800 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 200, 600 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 300, 500 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 200, 400 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 0, 200 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 200, 100 ) ) );

    MC2_TEST_REQUIRED( gfxData.getNbrCoordinates( 0 ) == 13 );

    Stack stack;
    bool convexHullCreated = gfxData.getConvexHull( &stack, 0);

    MC2_TEST_REQUIRED( convexHullCreated );

    for( uint32 i = 0; i < stack.getStackSize(); ++i)
    {
        cout << stack.getElementAt(i) << endl;
    }

    MC2_TEST_REQUIRED( stack.getStackSize() == 6 );
    MC2_TEST_REQUIRED( stack.getElementAt( 0 ) == 11  );
    MC2_TEST_REQUIRED( stack.getElementAt( 1 ) == 12 );
    MC2_TEST_REQUIRED( stack.getElementAt( 2 ) == 2 );
    MC2_TEST_REQUIRED( stack.getElementAt( 3 ) == 4 );
    MC2_TEST_REQUIRED( stack.getElementAt( 4 ) == 6 );
    MC2_TEST_REQUIRED( stack.getElementAt( 5 ) == 11 );

    cout << endl;
}

MC2_UNIT_TEST_FUNCTION( convexHullTest6 ) {

    GfxDataFull gfxData;
    /*
     *
     *   Check that the algorithm works for smallest possible 
     *   instance = 3 points.
     *
     *
     *
     *
     *                                    /-1
     *                               /----  |
     *                          /----       |
     *                     /----            |
     *                /----                 |
     *           /----                      |
     *        ---                           |
     *       0--\                           |
     *           -----\                     |
     *                 -----\               |
     *                       -----\         |
     *                             -----\   |
     *                                   ---2
     *
     *                                                   
     * The convex hull should be 0, 1, 2.     
     *
     */

    cout << "getConvexHull() test5: " << endl;

    MC2_TEST_REQUIRED( gfxData.addCoordinate( 400, 100, true ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 800, 1000 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 200, 1000 ) ) );

    MC2_TEST_REQUIRED( gfxData.getNbrCoordinates( 0 ) == 3 );

    Stack stack;
    bool convexHullCreated = gfxData.getConvexHull( &stack, 0);

    MC2_TEST_REQUIRED( convexHullCreated );

    for( uint32 i = 0; i < stack.getStackSize(); ++i)
    {
        cout << stack.getElementAt(i) << endl;
    }

    MC2_TEST_REQUIRED( stack.getStackSize() == 4 );
    MC2_TEST_REQUIRED( stack.getElementAt( 0 ) == 2  );
    MC2_TEST_REQUIRED( stack.getElementAt( 1 ) == 0 );
    MC2_TEST_REQUIRED( stack.getElementAt( 2 ) == 1 );
    MC2_TEST_REQUIRED( stack.getElementAt( 3 ) == 2 );

    cout << endl;
}

MC2_UNIT_TEST_FUNCTION( convexHullTest7 ) {

    GfxDataFull gfxData;
    /*
     *
     *   Check that the algorithm works for small instances, with
     *   special case that all points are on a line.
     *
     *
     *                           
     *                         2 
     *                         |
     *                         |
     *                         |
     *                         |
     *                         |
     *                         |
     *                         1
     *                         |
     *                         |
     *                         |
     *                         |
     *                         |
     *                         0
     *
     *                                                   
     * The convex hull should be 0, 2.     
     *
     */

    cout << "getConvexHull() test5: " << endl;

    MC2_TEST_REQUIRED( gfxData.addCoordinate( 200, 100, true ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 400, 100 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 600, 100 ) ) );

    MC2_TEST_REQUIRED( gfxData.getNbrCoordinates( 0 ) == 3 );

    Stack stack;
    bool convexHullCreated = gfxData.getConvexHull( &stack, 0);

    MC2_TEST_REQUIRED( convexHullCreated );

    for( uint32 i = 0; i < stack.getStackSize(); ++i)
    {
        cout << stack.getElementAt(i) << endl;
    }

    MC2_TEST_REQUIRED( stack.getStackSize() == 3 );
    MC2_TEST_REQUIRED( stack.getElementAt( 0 ) == 2  );
    MC2_TEST_REQUIRED( stack.getElementAt( 1 ) == 0 );
    MC2_TEST_REQUIRED( stack.getElementAt( 2 ) == 2 );

    cout << endl;
}

MC2_UNIT_TEST_FUNCTION( convexHullTest8 ) {

    GfxDataFull gfxData;
    /*
     *
     *   Test to make sure that a series of left turns are handled 
     *   correctly.                                  
     *                                               
     *                                               
     *               1                               
     *               /|                              
     *              / |                              
     *             |  |                              
     *             / /                               
     *            /  2--                             
     *           /      \-                           
     *          /         \-                         
     *         |            3------                  
     *         /                   \----4            
     *        /                 /------              
     *       /      /-----------                     
     *       0------                                 
     *                                               
     *                                               
     *                                                   
     * The convex hull should be 0, 1, 4             
     *                                               
     */                                              
                                                     
    cout << "getConvexHull() test5: " << endl;       
                                                     
    MC2_TEST_REQUIRED( gfxData.addCoordinate( 0, 0, true ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 700, 100 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 400, 200 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 200, 400 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 100, 700 ) ) );
                                                     
    MC2_TEST_REQUIRED( gfxData.getNbrCoordinates( 0 ) == 5 );
                                                     
    Stack stack;                                     
    bool convexHullCreated = gfxData.getConvexHull( &stack, 0);
                                                     
    MC2_TEST_REQUIRED( convexHullCreated );          
                                                     
    for( uint32 i = 0; i < stack.getStackSize(); ++i)
    {                                                
        cout << stack.getElementAt(i) << endl;       
    }                                                
                                                     
    MC2_TEST_REQUIRED( stack.getStackSize() == 4 );  
    MC2_TEST_REQUIRED( stack.getElementAt( 0 ) == 4  );
    MC2_TEST_REQUIRED( stack.getElementAt( 1 ) == 0 );
    MC2_TEST_REQUIRED( stack.getElementAt( 2 ) == 1 );
    MC2_TEST_REQUIRED( stack.getElementAt( 3 ) == 4 );
                                                     
    cout << endl;                                    
}                                                    
                                                     
MC2_UNIT_TEST_FUNCTION( convexHullTest9 ) {          
                                                     
    GfxDataFull gfxData;                             
    /*                                               
     *                                               
     *   Test to make sure that a series of right turns, followed by 
     *   a left turn, are handled correctly.          
     *                                               
     *                                               
     *              1                                
     *             /--                               
     *            |   \-                             
     *            /     2                            
     *           /      |                            
     *          /       |                        /--5
     *         /        3                /-------/
     *        /        --        /---------/
     *       |      4-/  /------------/ 
     *       /       ----  -----/       
     *      /         ----/             
     *     /    -----/                  
     *     0 --/                        
     *                                  
     *                                  
     *                                                   
     * The convex hull should be 0, 1, 5.     
     *                                  
     */                                 
                                        
    cout << "getConvexHull() test5: " << endl;
                                        
    MC2_TEST_REQUIRED( gfxData.addCoordinate( 0, 0, true ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 1000, 100 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 800, 400 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 600, 600 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 400, 700 ) ) );
    MC2_TEST_REQUIRED( gfxData.addCoordinate( MC2Coordinate( 400, 2000 ) ) );

    MC2_TEST_REQUIRED( gfxData.getNbrCoordinates( 0 ) == 6 );

    Stack stack;
    bool convexHullCreated = gfxData.getConvexHull( &stack, 0);

    MC2_TEST_REQUIRED( convexHullCreated );

    for( uint32 i = 0; i < stack.getStackSize(); ++i)
    {
        cout << stack.getElementAt(i) << endl;
    }

    MC2_TEST_REQUIRED( stack.getStackSize() == 4 );
    MC2_TEST_REQUIRED( stack.getElementAt( 0 ) == 5  );
    MC2_TEST_REQUIRED( stack.getElementAt( 1 ) == 0 );
    MC2_TEST_REQUIRED( stack.getElementAt( 2 ) == 1 );
    MC2_TEST_REQUIRED( stack.getElementAt( 3 ) == 5 );

    cout << endl;
}

MC2_UNIT_TEST_FUNCTION( mergePolygonsTest1 ) {          
                                                     
    //GMSGfxData gfxData;                            
    GMSGfxData* gfxData = GMSGfxData::createNewGfxData( NULL, false );
 
    /*
     *
     *  
     *   Basic merging
     *
     *                     
     *      1--------2,5--
     *      |        |    \---
     *      |        |        \--
     *      |        |           \---
     *      |        |               \--
     *      |        |                  \--
     *      |        |                     \---
     *      |   1    |                         \-
     *      |        |       2                /-6
     *      |        |                    /---
     *      |        |               /--o-
     *      |        |          /----
     *      |        |      /---
     *      4--------3,7----
     *                                                   
     * The result should correspond to 1, 2, 6, 7, 4 (but with different
     * indexes in mergedGfx).     
     *                                  
     */                                 
                                        
    cout << "mergePolygonsTest1: " << endl;
                              
    // Polygon 1, indexes 1-4 in picture 
    MC2_TEST_REQUIRED( gfxData->addCoordinate( 1000, 0 , true ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 1000, 200  ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 200, 200 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 200, 0 ) ) );

    MC2_TEST_REQUIRED( gfxData->getNbrCoordinates( 0 ) == 4 );

    // Polygon 2, indexes 5-7 in picture
    MC2_TEST_REQUIRED( gfxData->addCoordinate( 1000, 200 , true ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 600, 800 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 200, 200  ) ) );

    MC2_TEST_REQUIRED( gfxData->getNbrCoordinates( 1 ) == 3 );

    gfxData->setClosed(0, true);
    gfxData->setClosed(1, true);
    gfxData->updateLength();

    GMSGfxData* mergedGfx = GMSGfxData::mergePolygons( gfxData );
    
    MC2_TEST_REQUIRED( mergedGfx != NULL );

    // #1
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 0) == 1000 ); 
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 0) == 0 );
  
    // #2
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 1) == 1000 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 1) == 200 );

    // #6
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 2) == 600 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 2) == 800 );

    // #7
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 3) == 200 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 3) == 200 );
    
    // #4
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 4) == 200 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 4) == 0 );

    MC2_TEST_REQUIRED( mergedGfx->getNbrCoordinates( 0 ) == 5 );
}

MC2_UNIT_TEST_FUNCTION( mergePolygonsTest2 ) {          
                                                     
    //GMSGfxData gfxData;                            
    GMSGfxData* gfxData = GMSGfxData::createNewGfxData( NULL, false );
 
    /*
     *                            
     *                            
     *   Polygons touching in only one point should not be merged (unless
     *   they are surrounded by other polygons, see below).
     *                            
     *                            
     *                            
     *                            
     *                 /2                 /6
     *             /---  --           /---  --
     *          /--        \-      /--        \--
     *        --             \-  --              \--
     *        1                3,5                  \7
     *         -\              /  -\                --
     *           -\          -/     -\           --/
     *             --\     -/         -\       -/
     *                -- -/             --  --/
     *                 4/                 8/
     *
     *
     *
     * The result should be that no merging was performed. 
     *                                  
     */                                 
                                        
    cout << "mergePolygonsTest2: " << endl;
                              
    // Polygon 1, indexes 1-4 in picture 
    MC2_TEST_REQUIRED( gfxData->addCoordinate( 500, 0 , true ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 1000, 500  ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 500, 1000 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 0, 500 ) ) );

    MC2_TEST_REQUIRED( gfxData->getNbrCoordinates( 0 ) == 4 );

    // Polygon 2, indexes 5-8 in picture
    MC2_TEST_REQUIRED( gfxData->addCoordinate( 500, 1000 , true ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 1000, 1500 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 500, 2000  ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 0, 1500  ) ) );

    MC2_TEST_REQUIRED( gfxData->getNbrCoordinates( 1 ) == 4 );

    gfxData->setClosed(0, true);
    gfxData->setClosed(1, true);
    gfxData->updateLength();

    GMSGfxData* mergedGfx = GMSGfxData::mergePolygons( gfxData );
    
    MC2_TEST_REQUIRED( mergedGfx == NULL );
}

MC2_UNIT_TEST_FUNCTION( mergePolygonsTest3 ) {          
                                                     
    //GMSGfxData gfxData;                            
    GMSGfxData* gfxData = GMSGfxData::createNewGfxData( NULL, false );
 
    /*
     *                            
     *                            
     *   Polygons touching in only one point should (only) be merged when
     *   they are surrounded by other polygons. This solves the case 
     *   below where polygon 1 and are touching in one point (idxs 3 and 5)
     *   and where polygon 3 is self-touching in the same point (idxs 14, 18). 
     *
     *                                            /--10\
     *                                      /-----      \-
     *                                /-----              \
     *                          /-----                     \
     *                 /2,9 ----          /6,17      3      \-
     *             /---  --           /---  --                \
     *          /--        \-      /--        \--              \-
     *        --             \-  --              \--             \
     *        1        1       3,5,14,18   2        \7,16         \
     *         -\              /  -\                --             \-
     *           -\          -/     -\           --/                --11    
     *             --\     -/         -\       -/               ---/
     *                -- -/             --  --/              --/
     *                 4,13----\          8,15            --/
     *                          ------------\         ---/
     *                                       -------12
     *
     * The result should correspond to 1, 2, 10, 11, 12, 13 
     *                                  
     */                                 
                                        
    cout << "mergePolygonsTest3: " << endl;
                              
    // Polygon 1, indexes 1-4 in picture 
    MC2_TEST_REQUIRED( gfxData->addCoordinate( 500, 0 , true ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 1000, 500  ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 500, 1000 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 300, 500 ) ) );

    MC2_TEST_REQUIRED( gfxData->getNbrCoordinates( 0 ) == 4 );

    // Polygon 2, indexes 5-8 in picture
    MC2_TEST_REQUIRED( gfxData->addCoordinate( 500, 1000 , true ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 1000, 1500 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 500, 2000  ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 300, 1500  ) ) );

    MC2_TEST_REQUIRED( gfxData->getNbrCoordinates( 1 ) == 4 );
   
    // Polygon 3, indexes 9-18 in picture
    MC2_TEST_REQUIRED( gfxData->addCoordinate( 1000, 500 , true ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( 2000, 2000 ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 500, 3000 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 0, 2000  ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 300, 500  ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( 500, 1000 ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 300, 1500  ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 500, 2000  ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 1000, 1500 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 500, 1000 ) ) );

    MC2_TEST_REQUIRED( gfxData->getNbrCoordinates( 2 ) == 10 );


    gfxData->setClosed(0, true);
    gfxData->setClosed(1, true);
    gfxData->updateLength();

    GMSGfxData* mergedGfx = GMSGfxData::mergePolygons( gfxData );

    cout << "index0: " << mergedGfx->getLat(0, 0) << ", " 
         << mergedGfx->getLon(0, 0) << endl;

    cout << "index1: " << mergedGfx->getLat(0, 1) << ", " 
         << mergedGfx->getLon(0, 1) << endl;

    cout << "index2: " << mergedGfx->getLat(0, 2) << ", " 
         << mergedGfx->getLon(0, 2) << endl;

    cout << "index3: " << mergedGfx->getLat(0, 3) << ", " 
         << mergedGfx->getLon(0, 3) << endl;

    cout << "index4: " << mergedGfx->getLat(0, 4) << ", " 
         << mergedGfx->getLon(0, 4) << endl;

    cout << "index5: " << mergedGfx->getLat(0, 5) << ", " 
         << mergedGfx->getLon(0, 5) << endl;

   /*
    cout << "index6: " << mergedGfx->getLat(0, 6) << ", " 
         << mergedGfx->getLon(0, 6) << endl;

    cout << "index7: " << mergedGfx->getLat(0, 7) << ", " 
         << mergedGfx->getLon(0, 7) << endl;

    cout << "index8: " << mergedGfx->getLat(0, 8) << ", " 
         << mergedGfx->getLon(0, 8) << endl;

    cout << "index9: " << mergedGfx->getLat(0, 9) << ", " 
         << mergedGfx->getLon(0, 9) << endl;

    cout << "index10: " << mergedGfx->getLat(0, 10) << ", " 
         << mergedGfx->getLon(0, 10) << endl;
   */


   
    // #1

       cout << "index0befor: " << mergedGfx->getLat(0, 0) << ", " 
         << mergedGfx->getLon(0, 0) << endl;


    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 0) == 500 ); 
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 0) == 0 );
  
    // #2
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 1) == 1000 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 1) == 500 );

    // #10
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 2) == 2000 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 2) == 2000 );

    // #11
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 3) == 500 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 3) == 3000 );
    
    // #12 
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 4) == 0 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 4) == 2000 );
     
    // #13
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 5) == 300 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 5) == 500 );
 
}



MC2_UNIT_TEST_FUNCTION( mergePolygonsTest4 ) {          
                                                     
    GMSGfxData* gfxData = GMSGfxData::createNewGfxData( NULL, false );
 
    /*
     *
     *  
     *   Test case where polygon 2 has 'multiple touches' with poly 1,
     *   and where poly 3 fills the hole between poly 1 and 2.
     *
     *                     
     *      1--------2,8-- 
     *      |        | |  \---
     *      |        | |      \--
     *      |        3,13,14     \---
     *      |        | o--           \--
     *      |        |    \---          \--
     *      |   0    |        \---         \---
     *      |        4,17  2       \12,15 1     \-
     *      |        |          ----          /-9 
     *      |        |    -----/          /---   
     *      |        5,11,16         /--o-       
     *      |        ||         /----            
     *      |        ||     /---                 
     *      7--------6,10---
     *                                                   
     * The result should correspond to 1, 8, 9, 10, 7 
     *                                  
     */                                 
                                        
    cout << "mergePolygonsTest4: " << endl;
                              
    // Polygon 0, indexes 1-7 in picture
    MC2_TEST_REQUIRED( gfxData->addCoordinate( 1000, 0 , true ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 1000, 200  ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 800, 200 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 600, 200 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 400, 200 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 200, 200 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 200, 0 ) ) );

    MC2_TEST_REQUIRED( gfxData->getNbrCoordinates( 0 ) == 7 );

    // Polygon 1, indexes 8-14 in picture
    MC2_TEST_REQUIRED( gfxData->addCoordinate( 1000, 200 , true ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 600, 800 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 200, 200 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 400, 200 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 600, 400 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 800, 200  ) ) );

    MC2_TEST_REQUIRED( gfxData->getNbrCoordinates( 1 ) == 6 );
  
   // Polygon 2, indexes 14-16 in picture
    MC2_TEST_REQUIRED( gfxData->addCoordinate( 800, 200 , true ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 600, 400 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 400, 200 ) ) );
    MC2_TEST_REQUIRED( gfxData->addCoordinate( MC2Coordinate( 600, 200 ) ) );

    MC2_TEST_REQUIRED( gfxData->getNbrCoordinates( 2 ) == 4 );

    gfxData->setClosed(0, true);
    gfxData->setClosed(1, true);
    gfxData->setClosed(2, true);
    gfxData->updateLength();

    GMSGfxData* mergedGfx = GMSGfxData::mergePolygons( gfxData );
    
    MC2_TEST_REQUIRED( mergedGfx != NULL );

    cout << "index0: " << mergedGfx->getLat(0, 0) << ", " 
         << mergedGfx->getLon(0, 0) << endl;

    cout << "index1: " << mergedGfx->getLat(0, 1) << ", " 
         << mergedGfx->getLon(0, 1) << endl;

    cout << "index2: " << mergedGfx->getLat(0, 2) << ", " 
         << mergedGfx->getLon(0, 2) << endl;

    cout << "index3: " << mergedGfx->getLat(0, 3) << ", " 
         << mergedGfx->getLon(0, 3) << endl;

    cout << "index4: " << mergedGfx->getLat(0, 4) << ", " 
         << mergedGfx->getLon(0, 4) << endl;

    cout << "index5: " << mergedGfx->getLat(0, 5) << ", " 
         << mergedGfx->getLon(0, 5) << endl;

    cout << "index6: " << mergedGfx->getLat(0, 6) << ", " 
         << mergedGfx->getLon(0, 6) << endl;

    cout << "index7: " << mergedGfx->getLat(0, 7) << ", " 
         << mergedGfx->getLon(0, 7) << endl;

    cout << "index8: " << mergedGfx->getLat(0, 8) << ", " 
         << mergedGfx->getLon(0, 8) << endl;

    cout << "index9: " << mergedGfx->getLat(0, 9) << ", " 
         << mergedGfx->getLon(0, 9) << endl;

    cout << "index10: " << mergedGfx->getLat(0, 10) << ", " 
         << mergedGfx->getLon(0, 10) << endl;



    // #1
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 0) == 1000 ); 
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 0) == 0 );
  
    // #8
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 1) == 1000 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 1) == 200 );

    // #9
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 2) == 600 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 2) == 800 );

    // #10
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 3) == 200 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 3) == 200 );
    
    // #7 
    MC2_TEST_REQUIRED( mergedGfx->getLat(0, 4) == 200 );
    MC2_TEST_REQUIRED( mergedGfx->getLon(0, 4) == 0 );
}
