/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CommandlineOptionHandler.h"

#include "OldGenericMap.h"
#include "OldStreetSegmentItem.h"
#include "OldPointOfInterestItem.h"
#include "OldFerryItem.h"
#include "OldExternalConnections.h"
#include "OldNode.h"
#include "OldGroupItem.h"
#include "UTF8Util.h"
#include "MapBits.h"

struct notice_t
{

      notice_t() {
         m_nbrItems = 0;
         m_nbrGfxDatas = 0;
         m_nbrBuaLocations = 0;
         m_nbrMunicipalLocations = 0;
         m_nbrNoLocations = 0;
      }
      
      void updateNotice( OldGenericMap* theMap, OldItem* item ) 
      {
         m_nbrItems++;


         if ( item->getGfxData() != NULL ) {
            m_nbrGfxDatas++;
         }

         // Use location of the SSI of the POI instead.
         if ( item->getItemType() == ItemTypes::pointOfInterestItem ){
            OldPointOfInterestItem* poiItem = 
               dynamic_cast<OldPointOfInterestItem*>(item);
            uint32 ssiID = poiItem->getStreetSegmentItemID();
            OldItem* ssi = theMap->itemLookup( ssiID );
            if ( ssi != NULL ){
               item = ssi;
            }
         }
        
         uint32 nbrBuaLoc = 
            theMap->getNbrRegions( item, ItemTypes::builtUpAreaItem );
         m_nbrBuaLocations += nbrBuaLoc;
         
         uint32 nbrMunLoc = 
            theMap->getNbrRegions( item, ItemTypes::municipalItem );
         m_nbrMunicipalLocations += nbrMunLoc;
         
         if ( nbrBuaLoc + nbrMunLoc == 0 ) {
            m_nbrNoLocations++;
         }
      }

      uint32 m_nbrItems;
      uint32 m_nbrGfxDatas;
      uint32 m_nbrBuaLocations;
      uint32 m_nbrMunicipalLocations;
      uint32 m_nbrNoLocations;
   
};

int
verifyMap( OldGenericMap* theMap )
{
   int retVal = 0;
   uint32 mapID = theMap->getMapID();
   
   uint32 nbrConnections = 0;
   uint32 nbrExternalConns = 0;
   // Total nbr turndecriptions that are not UNDEFINED.
   uint32 nbrTurnDescs = 0;
   // Total nbr turndecriptions that are not UNDEFINED, AHEAD, FOLLOW ROAD.
   uint32 nbrNonAheadTurnDescs = 0;
   uint32 nbrCrossingTypes = 0;
   //uint32 nbrSignposts = 0;
   uint32 nbrForbiddenTurns = 0;
  
   // Nbr of missing groups for items.
   uint32 nbrMissingGroups = 0;
   // Nbr of missing items for groups.
   uint32 nbrMissingItemsInGroup = 0; 
   
   map<uint32, notice_t> noticeByType;
   

   bool undetailedMap = false;
   MC2String mapOrigin = theMap->getMapOrigin();
   // Check if this map is a AND level 2 quality map.
   MC2String compareStr = "_lev2";
   if ( mapOrigin.find(compareStr) != MC2String::npos ){
      undetailedMap = true;
   }   

   for ( uint32 i = 0; i < ItemTypes::numberOfItemTypes; ++i ) {
      notice_t notice;
      noticeByType[ i ] = notice;
   }
   
   // Count items etc.
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldItem* item = theMap->getItem(z, i);
         if ( item != NULL ) {
            
            if ( ! MapBits::isCountryMap(theMap->getMapID())){
               // We can't expect to find groups with the right ID in 
               // country overview maps.

               // Check groups
               for ( uint32 j = 0; j < item->getNbrGroups(); ++j ) {
                  if ( theMap->itemLookup( item->getGroup( j ) ) == NULL ){
                     ++nbrMissingGroups;
                     mc2dbg1 << "OldItem " << item->getID() << " group "
                             << j << " = " << item->getGroup( j )
                             << " is not present in the map."
                             << endl;
                  }
               }
               OldGroupItem* group = dynamic_cast<OldGroupItem*> ( item );
               if ( group != NULL ) {
                  for ( uint32 j = 0;
                        j < group->getNbrItemsInGroup();
                        ++j )
                  { 
                     if (theMap->itemLookup( group->getItemNumber( j ) ) ==
                          NULL ) {
                        ++nbrMissingItemsInGroup;
                     }
                  }
               }
            }
            
            noticeByType[ item->getItemType() ].
               updateNotice( theMap, item );
          
            switch ( item->getItemType() ) {
               case ( ItemTypes::streetSegmentItem ) :
               case ( ItemTypes::ferryItem ) : {
                  OldRouteableItem* routable = 
                     static_cast<OldRouteableItem*> ( item );
                  for ( uint32 j = 0; j < 2; ++j ) {
                     OldNode* node = routable->getNode( j );
                     nbrConnections += node->getNbrConnections();
                     // Go through all the connections and count stuff:
                     for ( uint32 k = 0; k < node->getNbrConnections();
                           ++k ) {
                        OldConnection* conn = node->getEntryConnection( k );
                        
                        //nbrSignposts += conn->getNbrSignPost();
                        
                        if ( ( node->getEntryRestrictions() == 
                               ItemTypes::noRestrictions ) && 
                             ( conn->getVehicleRestrictions() != 
                               MAX_UINT32 ) ) {
                           nbrForbiddenTurns++;
                        }

                        if ( conn->getCrossingKind() != 
                             ItemTypes::UNDEFINED_CROSSING )  {
                           nbrCrossingTypes++;
                        }

                        if ( conn->getTurnDirection() != 
                             ItemTypes::UNDEFINED ) {
                           nbrTurnDescs++;
                        } 
                        if ( ( conn->getTurnDirection() != 
                               ItemTypes::UNDEFINED ) &&
                             ( conn->getTurnDirection() != 
                               ItemTypes::AHEAD ) &&
                             ( conn->getTurnDirection() != 
                               ItemTypes::FOLLOWROAD ) ) {
                           nbrNonAheadTurnDescs++;

                        }
                        // Check connection sanity for non country maps.
                        if ( ! MapBits::isCountryMap( mapID ) ){
                           if ( ! conn->isMultiConnection() ) {
                              OldConnection* oppconn = 
                                 theMap->getOpposingConnection( conn, 
                                                                node->getNodeID() );
                              if ( oppconn == NULL ) {
                                 mc2log << error 
                                        << "Opposing connection "
                                        << "missing: " << " nodeID = " 
                                        << MC2HEX(node->getNodeID())
                                        << " connNbr = " << k << endl;
                                 retVal++;
                              }
                           }
                        }
                     }
                  }
                  
                  
               } break;
               case (ItemTypes::builtUpAreaItem): {
                  if (item->getNbrNames() == 0){
                     mc2log << error << "Built up area with no name." 
                            << "itemID: " << item->getID() << endl;
                     retVal++;
                  }

               } break;
               case (ItemTypes::cityPartItem): {
                  if (item->getNbrNames() == 0){
                     mc2log << error << "City part with no name." 
                            << "itemID: " << item->getID() << endl;
                     retVal++;
                  }

               } break;
               case (ItemTypes::municipalItem): {
                  if (item->getNbrNames() == 0){
                     mc2log << error << "Municipal with no name." 
                            << "itemID: " << item->getID() << endl;
                     retVal++;
                  }

               } break;
               
               default:
                  break;
            } // END: getItemType
         } 
      } // END: getNbrItemsWithZoom
   }
   
   // Make summaries from the notices:
   uint32 totNbrItems = 0;
   uint32 totNbrGfxDatas = 0;
   uint32 totNbrNoLocations = 0;
   
   for ( map<uint32, notice_t>::const_iterator it =
         noticeByType.begin(); it != noticeByType.end(); ++it ) {
      totNbrItems += it->second.m_nbrItems; 
      totNbrGfxDatas += it->second.m_nbrGfxDatas;
      totNbrNoLocations += it->second.m_nbrNoLocations;
   }



   
   
   uint32 nbrBoundrySegments = 0; 
   OldBoundrySegmentsVector* boundrySegments = theMap->getBoundrySegments();

   if ( boundrySegments != NULL ) {
      nbrBoundrySegments = boundrySegments->getSize();
      nbrExternalConns += boundrySegments->getTotNbrConnections();
   } 
   

   // =====================================================================
   // Check the information that is collected. 
   // =====================================================================
   
   // Only output data if an error is found.

   // General checks ------------------------------------------------------
   
   // Check totals:
   if ( totNbrItems == 0 ) {
      mc2log << error << "No items found." << endl;
      retVal++;
   }

   if ( totNbrGfxDatas == 0 ) {
      mc2log << error << "No GfxDatas found." << endl;
      retVal++;
   }      

   if ( totNbrNoLocations > totNbrItems * 0.75 ) {
      if ( ! MapBits::isOverviewMap( theMap->getMapID() ) ){
         mc2log << error  
                << "More than 75% of the items has no location." << endl;
         retVal++;
      }
   }      
 
   if ( nbrMissingGroups > 0 ) {
      mc2log << error
             << "Found " << nbrMissingGroups 
             << " missing groups for items." << endl;
      retVal++;
   }

   if ( nbrMissingItemsInGroup > 0 ) {
      mc2log << error
             << "Found " << nbrMissingItemsInGroup
             << " missing items in groups." << endl;
      retVal++;
   }


   // Checks UTF-8 in names ----------------------------------------------
   mc2dbg << "Nbr names: " << theMap->getTotalNbrItemNames() << endl;
   uint32 nbrValidNames=0;
   uint32 nbrInvalidNames=0;
   // Check
   for ( uint32 s = 0; s < theMap->getTotalNbrItemNames(); s++ ) {
      if ( ! UTF8Util::isValidUTF8( theMap->getName( s ),
                                    true // report conversions
                                    ) ) {
         nbrInvalidNames++;
         mc2log << error << "String: " << MC2CITE( theMap->getName(s) )
                << " is not valid UTF-8" << endl;
         // Name character codes as hex.
         const char* name = theMap->getName(s);
         mc2dbg << " "; // Indent. 
         for ( uint32 idx=0; idx<strlen(name); idx++ ){
            mc2dbg << " 0x" << hex 
                   << (uint32)(unsigned char)name[idx] 
                   << dec;
            
            if ( (idx % 10 == 9) && (strlen(name) != (idx+1)) ){
               // Break the row.
               mc2dbg << endl;
               mc2dbg << " "; // Indent. 
            }
         }
         mc2dbg << endl;
         

         for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
            for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
               OldItem* item = theMap->getItem(z, i);
               if (item == NULL){
                  continue;
               }
               for ( uint32 n=0; n< item->getNbrNames(); n++){
                  uint32 strIdx = item->getRawStringIndex(n);
                  if (strIdx == s){
                     mc2log << error << "   Item: " << item->getID() 
                            << endl;
                  }
               }
            }
         }
      }
      else {
         nbrValidNames++;
         //mc2log << info << "Valid name: " 
         //       <<  MC2CITE( theMap->getName(s)) << endl;
      }
   }
   mc2log << "RESULT valid names:" << nbrValidNames
          << " invalid names: " << nbrInvalidNames << endl;
   

   
   // Checks for all maps but higher level overview maps ------------------
   
   if ( MapBits::getMapLevel( mapID ) < 2 ) {
      
      if ( noticeByType[ ItemTypes::builtUpAreaItem ].m_nbrItems == 0 ) {
         mc2log << error << "No buas found." << endl;
         retVal++;
      }
      
      if ( noticeByType[ ItemTypes::municipalItem ].m_nbrItems == 0 ) {
         mc2log << error << "No municipals found." << endl;
         retVal++;
      }
   }


   // Checks for underview and first level of overview maps. --------------
   
   if ( ( MapBits::getMapLevel( mapID ) < 2 ) && 
        ( ! MapBits::isCountryMap( mapID ) ) ) {
      
      if ( boundrySegments == NULL ) {
         mc2log << error << "BoundrySegments == NULL" 
                << endl;
         retVal++;
      } 
      
      if ( nbrBoundrySegments == 0 ) {
         mc2log << error << "Nbr BoundrySegments == 0" 
                << endl;
         retVal++;
      }
      
      if ( nbrExternalConns == 0 ) {
         mc2log << warn
                << "Nbr external connections == 0" << endl;
         retVal++;
      }
      //cout << "External connections: " << nbrExternalConns << endl;

   }

   
   // Checks for all but country maps -------------------------------------
   
   if ( ! MapBits::isCountryMap( mapID ) ) {
      // Check for items that must be present.
      if ( noticeByType[ ItemTypes::streetSegmentItem ].m_nbrItems == 0 ) {
         mc2log << error << "No street segments found." 
                << endl;
         retVal++;
      }
      
      // Check connection related things.
      if ( nbrConnections == 0 ) {
         mc2log << error << "No connections found." << endl;
         retVal++;
      }
      
      // Check connection related things.
      if ( nbrTurnDescs < nbrConnections * 0.75 ) {
         mc2log << error  
                << "More than 25% of the connections have "
                << "no turndescription."
                << endl;
         retVal++;
      }
      
      if ( nbrForbiddenTurns == 0 ) {
         if ( !undetailedMap ){
            mc2log << error  
                   << "No forbidden turns found to noRestrictions-nodes."
                   << endl;
            retVal++;
         }
      }

      if ( nbrCrossingTypes == 0 ) {
         mc2log << error  
                << "All crossings are undefined." << endl;
         retVal++;
      }
   }
   

   // Checks for underview maps -------------------------------------------
   
   if ( MapBits::isUnderviewMap( mapID ) ) {
      if ( noticeByType[ 
            ItemTypes::pointOfInterestItem ].m_nbrItems == 0 ) {
         mc2log << error << "No pois found." << endl;
         retVal++;
      }

      if ( noticeByType[ ItemTypes::streetItem ].m_nbrItems == 0 ) {
         mc2log << error << "No streets found." << endl;
         retVal++;
      }
      
      if ( nbrNonAheadTurnDescs == 0 ) {
         mc2log << error  
                << "No non ahead turndescriptions found." << endl;
         retVal++;
      }
   }
   
   return retVal;
}



int main(int argc, char* argv[])
{

   CommandlineOptionHandler coh(argc, argv, 0);
   coh.setTailHelp("mcm-file");
   coh.setSummary("Checks the validity of a mcm map by doing some simple tests.");

   if (!coh.parse()) {
      mc2log << error << argv[0] 
             << ": Error on commandline, (-h for help)" << endl;
      exit(1);
   }
   
   int returnCode = 0;
   
   if ( coh.getTailLength() == 1 ) {
      // Process map.
      const char* mcmName = coh.getTail( 0 );
      OldGenericMap* theMap = OldGenericMap::createMap(mcmName);
      mc2log << info << "Map name: " << theMap->getMapName() << endl;
      if ( theMap != NULL ) {
         returnCode = verifyMap( theMap );          
      } else {
         mc2log << error << "Failed to load " << mcmName << endl;
         exit(1);
      }
      delete theMap; 
   } else {
      mc2log << error << "Must give mcm map in tail!" << endl;
      exit(1);
   }

   return returnCode;
}


