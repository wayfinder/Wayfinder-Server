/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GDFRef.h"

#include "config.h"
#include "MC2String.h"
#include "GfxData.h"
#include "GDFID.h"
#include "SectionPair.h"
#include "MapGenUtil.h"
#include "OldGenericMap.h"

GDFRef::GDFRef()
{

}

GDFRef::~GDFRef()
{

}


uint32
GDFRef::getMcmID( pair< pair<uint32, uint32>, uint32> gdfID ) const 
{
   uint32 result = MAX_UINT32;

   pair<uint32, uint32> dsatAndSection = gdfID.first;
   uint32 featID = gdfID.second;

   GDFRef::gdfRefBySection_t::const_iterator sectionIt = 
      m_gdfRefTable.find(dsatAndSection);
   if ( sectionIt != m_gdfRefTable.end() ){

      // Go trough all feature categories.
      idsByFeatCat_t::const_iterator featCatIt =      
         sectionIt->second.begin();
      bool found = false;
      while ( !found && featCatIt != sectionIt->second.end() ){

         // Find the item.
         mc2IDByGdfID_t::const_iterator idIt = 
            featCatIt->second.find(featID);
         if ( idIt != featCatIt->second.end() ){
            result = idIt->second;
            found = true;
         }
      }
      ++featCatIt;
   }
   return result;

} // getMcmID


set<uint32>
GDFRef::getAllMcmIDs( uint32 gdfFeatID ) const
{
   set<uint32> result;

   mc2dbg << "(dsatID:secID)[featCat]:gdfFeatID->mcmID" << endl;

   gdfRefBySection_t::const_iterator secIt = m_gdfRefTable.begin();
   while (secIt != m_gdfRefTable.end() ){
      idsByFeatCat_t::const_iterator fCatIt = secIt->second.begin();
      while ( fCatIt != secIt->second.end() ){
         mc2IDByGdfID_t::const_iterator gdfIdIt = fCatIt->second.begin();
         while (gdfIdIt != fCatIt->second.end() ){
            if ( gdfIdIt->first == gdfFeatID ){
               mc2dbg << "(" << secIt->first.first << ":" 
                      << secIt->first.second
                      << ")["  << fCatIt->first << "]:"
                      << gdfIdIt->first << "->" << gdfIdIt->second << endl;
               result.insert(gdfIdIt->second);
            }
            ++gdfIdIt;
         }
         ++fCatIt;
      }
      ++secIt;
   }
   return result;

} // getAllMcmIDs

void 
GDFRef::initGdfByMCM() const
{
   mc2log << "GDFRef::initGdfByMCM: Called."
          << endl;

   if ( !m_gdfIdByMcmId.empty() ){
      mc2log << warn << "GDFRef::initGdfByMCM: Initing when already inited"
             << endl;
   }

   // Count
   uint32 nbrDsatSec = 0;
   uint32 nbrFeatCat = 0;
   uint32 nbrGdfID = 0;

   // For all dataset, section pairs.
   gdfRefBySection_t::const_iterator dsetSecIt = m_gdfRefTable.begin();
   while ( dsetSecIt != m_gdfRefTable.end() ){
      uint32 dataset = dsetSecIt->first.first;
      uint32 section = dsetSecIt->first.second;
      nbrDsatSec++;

      // For all feature categories.
      idsByFeatCat_t::const_iterator featIt = dsetSecIt->second.begin();
      while ( featIt != dsetSecIt->second.end() ){
         uint32 featCat = featIt->first;
         nbrFeatCat++;

         // For all gdf IDs.
         mc2IDByGdfID_t::const_iterator gdfIt = featIt->second.begin();
         while ( gdfIt != featIt->second.end() ){
            uint32 gdfID = gdfIt->first;
            uint32 mcmID = gdfIt->second;
            nbrGdfID++;
            
            fullGdfID_t fullGdfID;
            fullGdfID.datasetID = dataset;
            fullGdfID.sectionID = section;
            fullGdfID.featID = gdfID;
            fullGdfID.featCat = featCat;

            m_gdfIdByMcmId.insert( make_pair( mcmID, fullGdfID ) );

            ++gdfIt;
         }

         ++featIt;
      }

      ++dsetSecIt;
   }
   
   mc2dbg << "GDFRef::initGdfByMCM: nbrDsatSec: " << nbrDsatSec 
          << " nbrFeatCat: " << nbrFeatCat 
          << " nbrGdfID: " << nbrGdfID << endl;
   mc2dbg8 << "GDFRef::initGdfByMCM: Size: " << m_gdfIdByMcmId.size() 
           << endl;

} // initGdfByMCM

GDFID
GDFRef::getGdfID(const OldItem* item) const {
   pair< pair<uint32, uint32>, uint32> gdfIDPairs = getGdfIDPairs(item);
   return GDFID(gdfIDPairs.first.first, 
                gdfIDPairs.first.second, 
                gdfIDPairs.second );
} // getGdfID


pair< pair<uint32, uint32>, uint32> 
GDFRef::getGdfIDPairs(const OldItem* item) const
{
   // If the m_gdfIdByMcmId has not been inited, init it.
   if ( m_gdfIdByMcmId.empty() ){
      initGdfByMCM();
   }

   uint32 featCat = getFeatCategory(item);
   mc2dbg8 << "Feat cat: " << featCat << endl;
   fullGdfID_t fullGdfID;
   fullGdfID.featID = MAX_UINT32;

   pair<multimap< uint32, fullGdfID_t >::const_iterator, 
        multimap< uint32, fullGdfID_t >::const_iterator> idRange = 
      m_gdfIdByMcmId.equal_range( item->getID() );
   multimap< uint32, fullGdfID_t >::const_iterator idIt = idRange.first;
   bool found = false;
   while ( !found && idIt != idRange.second ){
      if ( fullGdfID.featID == MAX_UINT32 ){
         // We set the fist GDF ID even tough the feature category may not 
         // match.
         fullGdfID = idIt->second;
         if ( fullGdfID.featCat == featCat ){
            found = true;
         }
      }
      else if ( idIt->second.featCat == featCat ){
         mc2dbg8 << "Found" << endl;
         fullGdfID = idIt->second;
         found = true;
      }
      ++idIt;
   }

   pair< pair<uint32, uint32>, uint32> result;
   if ( fullGdfID.featID != MAX_UINT32 ){
      mc2dbg8 << "Returns valid" << endl;
      result.first.first = fullGdfID.datasetID;
      result.first.second = fullGdfID.sectionID;
      result.second = fullGdfID.featID;
   }
   else {
      mc2dbg8 << "Returns invalid" << endl;
      result.first.first = MAX_UINT32;
      result.first.second = MAX_UINT32;
      result.second = MAX_UINT32;
   }
   return result;

} // getGdfIDPairs


//typedef map< uint32, mc2ByFeature_t > featurePairByType_t 
//typedef multimap< uint32, uint32 > mc2ByFeature_t 


//SectionPair::getDataset ()
//SectionPair::getSection ()


//typedef map< SectionPair, featurePairByType_t > gdfRefBySection_t 
//gdfRefBySection_t m_gdfRefBySection
bool
GDFRef::readGDFRefFile( MC2String& fileName )
{
   mc2log << "GDFRef::readGDFRefFile: Reading file:" << fileName << endl;

   ifstream file( fileName.c_str(), ios::in );
   if ( ! file ) {
      mc2log << warn << "[GDFRef] Could not open " << fileName << endl;
      return false;
   }

   uint32 dataset, section;
   uint32 nbrIDs = 0;
   file >> dataset;
   while ( ! file.eof() ) {
      file >> section;
      
      pair<uint32, uint32> sectionPair = make_pair( dataset, section );
      
      // New section. Create a idsByFeatCat_t
      idsByFeatCat_t idsByFeatCat;      
      
      // Nbr feature types
      uint32 nbrFeatureCats;
      file >> nbrFeatureCats;
      for ( uint32 i = 0; i < nbrFeatureCats; ++i ) {
         // Feature type
         uint32 featureCat;
         file >> featureCat;

         // Create mc2IDByGdfID_t for this feature type.
         mc2IDByGdfID_t mc2IDByGdfID;
         
         // Nbr features
         uint32 nbrFeatures;
         file >> nbrFeatures;
         
         for ( uint32 j = 0; j < nbrFeatures; ++j ) {
            uint32 gdfID, mc2ID;
            // Feature ID and mc2 ID.
            file >> gdfID >> mc2ID;            
            // Update mc2IDByGdfID table.
            mc2IDByGdfID.insert( make_pair( gdfID, mc2ID ) );
            nbrIDs++;
         }
         // Store mc2IDByGdfID in idsByFeatCat.
         idsByFeatCat[ featureCat ] = mc2IDByGdfID;
      }

      // Store idsByFeatCat in m_gdfRefBySection.
      m_gdfRefTable[ sectionPair ] = idsByFeatCat;
      
      // Try to read next dataset
      file >> dataset;
   }
   mc2log << "GDFRef::readGDFRefFile: Read " << nbrIDs << " IDs." << endl;
   mc2dbg8 << "GDFRef::readGDFRefFile: m_gdfRefTable.size(): " 
           << m_gdfRefTable.size() << endl;
   return true;
}

uint32 
GDFRef::getFeatCategory( const OldItem* item ) const
{
   uint32 featureCategory = MAX_UINT32; // Invalid value.

   ItemTypes::itemType itemType = item->getItemType();
   switch (itemType){
   case (ItemTypes::municipalItem):
   case (ItemTypes::parkItem):
   case (ItemTypes::forestItem):
   case (ItemTypes::buildingItem):
   case (ItemTypes::islandItem):
   case (ItemTypes::zipCodeItem):
   case (ItemTypes::builtUpAreaItem):
   case (ItemTypes::cityPartItem):
   case (ItemTypes::zipAreaItem):
   case (ItemTypes::airportItem):
   case (ItemTypes::aircraftRoadItem):
   case (ItemTypes::pedestrianAreaItem):
   case (ItemTypes::militaryBaseItem):
   case (ItemTypes::cartographicItem):
   case (ItemTypes::individualBuildingItem):{

      featureCategory=3;

   }break;
   case (ItemTypes::waterItem):{
      // Water items can be both 2 and 3 depending on if their gfx data
      // is closed or not.

      GfxData* gfx = item->getGfxData();
      bool error = false;


      if ( gfx->getNbrPolygons() > 0 ){
         bool closed = gfx->getClosed(0);
         bool oldClosed = gfx->getClosed(0);
         
         for ( uint32 p=0; p<gfx->getNbrPolygons(); p++ ){
            closed = gfx->getClosed(p);
            if ( closed != oldClosed ){
               mc2log << error << "GDFRef::getFeatCategory Mixed closed"
                      << " properties of gfx data for water item. ID: " 
                      << item->getID() << endl;
               error = true;
            }
         }

         if ( error ){
            featureCategory = MAX_UINT32;
         }
         else if ( closed ){
            featureCategory = 3;
         }
         else {
            featureCategory = 2;
         }
      }
      else {
         mc2log << error << "GDFRef::getFeatCategory: Water item with no"
                << " gfx data, ID:" << item->getID() << endl;
         featureCategory = MAX_UINT32;
      }
      
   }break;
   
   case (ItemTypes::streetSegmentItem):
   case (ItemTypes::railwayItem):
   case (ItemTypes::ferryItem):
   case (ItemTypes::subwayLineItem):{
      
         featureCategory=2;
   
   }break;
   case (ItemTypes::pointOfInterestItem):{

         featureCategory=1;      

   }break;
   // the ones we made up ourselves...
   case (ItemTypes::streetItem):
   case (ItemTypes::nullItem):
   case (ItemTypes::categoryItem):
   case (ItemTypes::busRouteItem):
   case (ItemTypes::notUsedItemType):
   case (ItemTypes::numberOfItemTypes):
   case (ItemTypes::borderItem):
   case (ItemTypes::routeableItem):{
      
      featureCategory=MAX_UINT32;
      
   }break;
   } // swich

   return featureCategory;
} //getFeatCatFromItemType

set <pair <uint32, uint32> >
GDFRef::getAllSections() const
{
   set <pair <uint32, uint32> > result;
   
   GDFRef::gdfRefBySection_t::const_iterator it = m_gdfRefTable.begin();
   while ( it != m_gdfRefTable.end() ){
      result.insert( it->first );
      ++it;
   }
   return result;

} // getAllSections


set <uint32> 
GDFRef::getAllSectionIDs() const
{
   set <uint32> result;
   
   GDFRef::gdfRefBySection_t::const_iterator it = m_gdfRefTable.begin();
   while ( it != m_gdfRefTable.end() ){
      result.insert( it->first.second );
      ++it;
   }
   return result;
}

set <SectionPair>
GDFRef::getAllSectionPairs() const
{
   set <SectionPair> result;
   
   GDFRef::gdfRefBySection_t::const_iterator it = m_gdfRefTable.begin();
   while ( it != m_gdfRefTable.end() ){
      result.insert( SectionPair(it->first.first, it->first.second ));
      ++it;
   }
   return result;

} // getAllSectionPairs

MC2String
GDFRef::getPathAndFileName(const OldGenericMap& map){
   
   // Get the gdf ref file name.
   MC2String mapFileName = map.getFilename();
   uint32 pos = mapFileName.rfind("/");
   MC2String onlyMapFileName = mapFileName.substr(pos+1);
   pos = onlyMapFileName.rfind(".bz2");
   onlyMapFileName = onlyMapFileName.substr(0, pos);
   pos = onlyMapFileName.rfind(".gz");
   onlyMapFileName = onlyMapFileName.substr(0, pos);
   MC2String gdfRefFileName = onlyMapFileName + MC2String(".gdf_ref");

   // Add the path.
   MC2String fullPath;
   if (MapGenUtil::fileExists("./" + gdfRefFileName) ){
      fullPath =  "./" + gdfRefFileName;
   }
   else if (MapGenUtil::fileExists("./temp/" + gdfRefFileName) ){
      fullPath = "./temp/" + gdfRefFileName;
   }
   else if (MapGenUtil::fileExists("../temp/" + gdfRefFileName) ){
      fullPath = "../temp/" + gdfRefFileName;
   }

   return fullPath;

} // getGDFRefFileName
