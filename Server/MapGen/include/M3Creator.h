/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef M3CREATOR_H
#define M3CREATOR_H


#include <vector>
#include <utility>

#include "OldExternalConnections.h"
#include "FileException.h"

class OldGenericMap;
class GenericMap;
class OldItem;
class Item;
class OldConnection;
class Connection;
class OldNode;
class Node;
class Map;
class OldMap;
class OverviewMap;
class OldOverviewMap;
class CountryOverviewMap;
class OldCountryOverviewMap;
class RouteableItem;

class M3Creator
{
public:

   /**
    * @param oldMap the old generic map type
    * @param filename the file to save to.
    * @throws file exception
    */
   void saveM3Map( const OldGenericMap& oldMap, 
                   const MC2String& filename ) 
      const throw (FileUtils::FileException);

private:
   /**
    * Creates a GenericMap from a GMSMap
    * @param oldMap the map that will be copied
    * @return allocated GenericMap pointer
    */    
   GenericMap* createM3Map( const OldGenericMap& oldMap ) const;

   /**
    * @param oldMap the old generic map which the map team uses
    * @param map the new server map type which the server uses
    * @param filename the file to store poi information
    * @throws file exception if something whent wrong.
    */
   void generatePOIInfo( const OldGenericMap& oldMap,
                         GenericMap& map,
                         const MC2String& filename ) 
      const throw (FileUtils::FileException);

   void copyExternalConnectionVector( GenericMap &map,
                                      std::vector< std::pair<uint32, Connection* > > &dest, 
                                      const OldBoundrySegment::ExternalConnectionVector &src ) const;
   void copyGroupItem( Item& newItem, const OldItem& oldItem ) const;
   void copyConnection( GenericMap& map, Connection& con, const OldConnection& oldCon) const;
   void copySignPosts( GenericMap& map, Node& node, Connection& con,
                       const vector<GMSSignPost*>& signPosts ) const;
   void copyNode( GenericMap& map, Node& node, const OldGenericMap& oldMap, 
                  const OldNode& oldNode, const RouteableItem& route ) const;
   void copyRouteable( GenericMap &map, Item& newItem, 
                       const OldGenericMap& oldMap,
                       const OldItem& oldItem ) const;
   void copyItem( GenericMap& map, Item& newItem, 
                  const OldGenericMap& oldMap, const OldItem& oldItem ) const;
   void copyOverviewMap( OverviewMap& map, const OldOverviewMap& oldMap ) const;
   void copyUnderviewMap( Map& map, const OldMap& oldMap ) const;
   void copyCountryOverviewMap( CountryOverviewMap& map, const OldCountryOverviewMap& oldMap ) const;
};
#endif 
