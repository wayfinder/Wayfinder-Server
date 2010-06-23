/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TOPREGIONMATCH_H
#define TOPREGIONMATCH_H

#include "config.h"
#include "MC2BoundingBox.h"
#include "ItemIDTree.h"
#include "NameCollection.h"
#include "Name.h"

#include <memory>

class LocaleCompareLess;

/**
 *    Represents a Top Region.
 *
 */
class TopRegionMatch {
   public:
      /**
       * Type of topregion.
       */
      enum topRegion_t {
         country = 0,
         state = 1,
         internationalRegion = 2,
         metaregion = 3,

         /// Number of top region types
         nbr_topregion_t
      };


      /**
       * Creates a new TopRegionMatch. Add names using addName method.
       * 
       * @param id The id of this top region.
       * @param type The type of top region.
       */
      TopRegionMatch( uint32 id, topRegion_t type );

      
      /**
       * Creates a new TopRegionMatch.
       * 
       * @param id The id of this top region.
       * @param type The type of top region.
       * @param idTree Tree of map and item ids.
       * @param bbox Boundingbox of the region.
       * @param names The names of the region.
       */
      TopRegionMatch( uint32 id, 
                      topRegion_t type,
                      const ItemIDTree& idTree,
                      const MC2BoundingBox& bbox,
                      const NameCollection& names );
      
      
      /**
       * Deletes all contents.
       */
      virtual ~TopRegionMatch();


      /**
       * Get the top region id of this top region.
       * 
       * @return The top region id of this top region.
       */
      uint32 getID() const;

      /**
       *   Returns true if overview search must be performed
       *   before searching for streets etc. Should not be
       *   necessary for e.g. Öresundsregionen, but should be
       *   necessary for Sweden.
       *   @return true If a search for municipal/bua must be performed
       *                before searching for streets etc.
       */
      inline bool overviewSearchRequired() const;

      /**
       * Get the type of top region.
       *
       * @return The type of top region.
       */
      topRegion_t getType() const;


      /**
       * Add a name in a specific language.
       *
       * @param name The name.
       * @param lang The language of the name. 
       * @param type The type of the name. Default set to officialName.
       */
      void addName( const char* name, 
                    LangTypes::language_t lang,
                    ItemTypes::name_t type = ItemTypes::officialName );

      
      /**
       * Add a name.
       *
       * @param   name The name.
       */
      void addName( const Name* name );
      
      
      /**
       * Add names.
       *
       * @param   names Collection of names.
       */
      void addNames( const NameCollection* name );

      
      /**
       * Returns the name in language lang, notFoundValue if no name 
       * in language lang.
       *
       * @param lang The language of name to look for.
       * @param notFoundValue What to return if no name for language lang
       *                      is found, default NULL.
       * @return  The name or the notFoundValue if not found.
       */
      const char* getName( LangTypes::language_t lang, 
                           const char* notFoundValue = NULL ) const;
      

      /**
       * Returns the collection of names.
       * @return the collection of names.
       */
      const NameCollection* getNames() const;

      
      /**
       * Set the BoundingBox.
       *
       * @param bbox The new BoundingBox for this top region.
       */
      void setBoundingBox( const MC2BoundingBox& bbox );


      /**
       * Get the BoundingBox for this top region.
       *
       * @return The BoundingBox for this top region.
       */
      const MC2BoundingBox& getBoundingBox() const;

      /**
       *   Returns a reference to the ItemIDTree of this
       *   TopRegionMatch.
       */
      inline const ItemIDTree& getItemIDTree() const;

   protected:
      /// MapModuleNoticeContainer needs to be able to add stuff
      friend class OldMapModuleNoticeContainer;
      friend class MapModuleNoticeContainer;
      /// TopRegionReplyPacket needs to be able to add stuff.
      friend class TopRegionReplyPacket;
      
      
      /**
       *   Returns the ItemIDTree so that it is possible to
       *   write to it.
       */
      inline ItemIDTree* getItemIDTreeForWriting();

      
      /**
       *   The top region id of this top region.
       */
      uint32 m_id;


      /**
       *   The type of top region match.
       */
      topRegion_t m_type;


      /**
       *   The names of the top region.
       */
      NameCollection m_names;

      
      /**
       *   Tree describing the contents of the top region.
       *   (MapID:s and ItemID:s).
       */
      ItemIDTree m_itemTree;
      
      
      /**
       *   The BoundingBox for this top region.
       */
      MC2BoundingBox m_bbox;
};


// A vector of TopRegionMatches
typedef vector< TopRegionMatch* > TopRegionMatchesVector;

// A vector of const TopRegionMatches
typedef vector< const TopRegionMatch* > ConstTopRegionMatchesVector;

// Top region name compare 
class topRegionMatchCompareLess {
   public:
      topRegionMatchCompareLess( LangTypes::language_t language );

      ~topRegionMatchCompareLess();

      topRegionMatchCompareLess( const topRegionMatchCompareLess& o );


      bool operator() ( const TopRegionMatch* a, 
                        const TopRegionMatch* b ) const;

private:
   const topRegionMatchCompareLess& operator=( 
         const topRegionMatchCompareLess& o );

   std::auto_ptr<LocaleCompareLess> m_compare;
   LangTypes::language_t m_language;
};


// ------ Implementation of inlined methods -----------

inline const ItemIDTree& 
TopRegionMatch::getItemIDTree() const
{
   return m_itemTree;
}

inline ItemIDTree*
TopRegionMatch::getItemIDTreeForWriting()
{
   return &m_itemTree;
}

inline bool
TopRegionMatch::overviewSearchRequired() const
{
   return true;
}

#endif // TOPREGIONMATCH_H





