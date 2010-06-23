/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GDFREF_H
#define GDFREF_H

#include "config.h"
#include "MC2String.h"
#include "OldItem.h"
#include <map>

class GDFID;
class SectionPair;

/**
  *   GDFRef description.
  *
  */
class GDFRef
{
   public:

      /**
       * Constructor.
       */
      GDFRef();

      /**
       * Destructor.
       */
      virtual ~GDFRef();

      /**
       * @return Returns the path to an appropriate gdfRef file of the map
       *         supplied as an argument, or empty string if none was found.
       */
      static MC2String getPathAndFileName(const OldGenericMap& map);

      /**
       * Loads the gdf ref table in fileName to this GDFRef object.
       *
       * @param fileName A GDF reference file previously created by the
       *        map generation. Typically they are called *.mcm.gdf_ref.
       */
      bool readGDFRefFile(MC2String& fileName);

      /**
       * Get ID in GDF of a specific item. Only returns one GDF ID per
       * mcm ID (the first found). Ohter may be present.
       *
       * @return Returns GDF dataset, section and feature ID corresponding
       *         to the ID of item. Returns MAX_UINT32 in all fields if
       *         the item could not be found.
       */
      pair< pair<uint32, uint32>, 
         uint32> getGdfIDPairs(const OldItem* item) const;

      /**
       * Get ID in GDF of a specific item. Only returns one GDF ID per
       * mcm ID (the first found). Ohter may be present.
       *
       * @return A GDFID object telling the GDF ID of the item.
       */
      GDFID getGdfID(const OldItem* item) const;

     
      /**
       * Get an mcm ID from a GDF ID.
       *
       * @param gdfID The GDF id to get the mcm ID for.
       *        first.first  GDF dataset
       *        frist.second GDF section
       *        second       GDF feature ID
       * @return Returns mcm ID if found, otherwise MAX_UINT32.
       */
      uint32 getMcmID( pair< pair<uint32, uint32>, uint32> gdfID ) const;

      
      /**
       * Returns all mcm IDs of the map, matching a single GDF ID. 
       * Implemented for use of the MapEditor. Prints info of what GDF IDs
       * that are found.
       *
       * @param gdfFeatID Only the feature ID part of a full GDF ID.
       *
       * @return All matched mcm IDs.
       */
      set<uint32> getAllMcmIDs( uint32 gdfFeatID ) const;

      
      /**
       * Get all sections stored in this GDFRef
       */
      //@{
      set <pair <uint32, uint32> >getAllSections() const;
      set <SectionPair> getAllSectionPairs() const;
      set <uint32> getAllSectionIDs() const;
      //@}

      typedef multimap<uint32, uint32> mc2IDByGdfID_t;
      typedef map<uint32, mc2IDByGdfID_t> idsByFeatCat_t;
      typedef map< pair<uint32, uint32>, idsByFeatCat_t> gdfRefBySection_t;


   protected:

      
      struct fullGdfID_t {

         /// GDF dataset ID.
         uint32 datasetID;

         /// GDF Section ID
         uint32 sectionID;

         /// The GDF feature ID 
         uint32 featID;

         /// Feature category.
         uint32 featCat;
      };


      /**
       * Inits the m_gdfIdByMcmId member, which is used for fast 
       * mcm ID lookup.
       */
      void initGdfByMCM() const;


      /*
       * Member variables.
       */
      //@{
      
      /**
       *      gdfRefBySection_t       idsByFeatCat_t     mc2IDByGdfID_t
       *   |-----------------|---|   |---------|---|    |-------|-------|
       *   |(dataset|section)| +-|-->| featCat | +-|--->| gdfID | mcmID |
       *   |-----------------|---|   |---------|---|    |-------|-------|
       *   |                 |   |   |         |   |    |       |       |
       *   |-----------------|---|   |---------|---|    |-------|-------|
       *   |                 |   |   |         |   |    |       |       |
       *   |-----------------|---|   |---------|---|    |-------|-------|
       *   |                 |   |   |         |   |    |       |       |
       *   |                 |   |   |         |   |    |       |       |
       *
       *   OldMaps GDF ID to mcm ID. The table is sorted on GDF dataset and 
       *   section, and also GDF feature category, see figure above.
       */
      GDFRef::gdfRefBySection_t m_gdfRefTable;
      //@}


      /**
       * OldMaps GDF ID by mcm ID. It is faster to look for at GDF ID in this
       * one than it is to look in m_gdfRefTable.
       */
      mutable multimap< uint32, fullGdfID_t >m_gdfIdByMcmId;

      /*
       * Protected methods.
       */
      //@{

      /**
       * Returns the feature cateory in GDF of the GDF feature 
       * corresponding to the item.
       *
       * @param item The item to get GDF feature category of.
       *
       * $return Returns the GDF feature category of the GDF feature
       *         corresponding to the item.
       */
      uint32 getFeatCategory( const OldItem* item ) const;
      //@}

}; // GDFRef

#endif // GDFREF_H.

