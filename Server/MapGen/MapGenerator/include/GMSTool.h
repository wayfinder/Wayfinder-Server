/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSTOOL_H
#define GMSTOOL_H

#include "config.h"
#include "MC2String.h"
#include "ItemTypes.h"

class GMSCountryOverviewMap;
class GMSMap;

/**
  *   GMSTool gathers methods for getting info of and manipulating the map.
  *   Main method is processMap. Its behaviour is controlled by a string 
  *   option.
  *
  */
class GMSTool{
 public:
   /**
    * Main method of this class. Includes the option interpretter.
    */
   static bool processMap(const MC2String& option, GMSMap* theMap);


   /**
    * @param itemType Only the coordinates of items of this item type
    *                 are counted. If numberOfItemsTypes is given,
    *                 all items coordinates are counted.
    * 
    * @return Returns total number of coords used for all items of item type
    *         itemType.
    */
   static uint32 getNbrCoordsOfItems(GMSMap* theMap,
                                     ItemTypes::itemType itemType = 
                                     ItemTypes::numberOfItemTypes);


   /** Count and print what items have center points.
    */
   static void checkCenterPoints(GMSMap* theMap);


   /** Prints the hierarchy of regions including BUAs and municipals.
    */
   static void printMunBuaTree(GMSMap* theMap);

   // Parent in first and child in second of the pair. MAX_INT32 in frist if 
   // a child have no parent.
   typedef multimap< uint32, uint32 > treeType_t;
   
   /**
    * @param theMap The map the items are located in. Used for finding types
    *               and names.
    * @param indent Used for indenting the text when printing. Increased for
    *               each level in the tree.
    * @param itemID The ID of the item to print the childs of.
    * @param tree   The tree with the stucture to print. The nodes are removed
    *               from this tree as they are printed.
    */
   static void printTreeNode(GMSMap* theMap, 
                             MC2String indent, 
                             uint32 itemID, 
                             treeType_t& tree);


   /// Print the SSI IDs of SSIs that have lane information.
   static void printLanesSsiIDs(GMSMap* theMap);

   /// Print the SSI IDs of SSIs that are multi dig.
   static void printMultiDigSsiIDs(GMSMap* theMap);
   
   /// Print the SSI IDs of SSIs that have no throughfare
   static void printNoThroughfareSsiIDs(GMSMap* theMap);
   /// Print the SSI IDs of SSIs that have controlled access
   static void printCtrlAccSsiIDs(GMSMap* theMap);

   /** Print item IDs of all items of which at least one node has a valid
    *  connecting lanes value.
    */
   static void printConnLanesItemIDs(GMSMap* theMap);


   /**
    *    Print to stdout number of coordinates in the different filter
    *    levels of the map gfx data.
    */
   static void printNbrCoordsInCoPolFiltLevels(
                     GMSCountryOverviewMap* countryMap);



}; // class GMSTool





#endif // GMSTOOL_H
