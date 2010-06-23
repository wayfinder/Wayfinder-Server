/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CATEGORYTREE_H
#define CATEGORYTREE_H

#include "config.h"
#include "MC2String.h"
#include "LangTypes.h"
#include "NotCopyable.h"
#include "CategoryTreeUtils.h"
#include "XMLTool.h"

#include <vector>
#include <map>
#include <set>
#include <boost/shared_ptr.hpp>
#include <dom/DOM.hpp>

namespace CategoryTreeUtils {

class CategoryTree;
class DefaultConfiguration;
typedef boost::shared_ptr<CategoryTree> CategoryTreePtr;

/**
 * Holds category tree and the category translations.
 *
 * 
 * It holds two maps ( category ID -> language -> string ) and
 * ( language -> string -> category id ).
 *
 * The first one is to translate categories and the
 * second one is for searching in specific languages.
 *
 * The second map holds converted and sorted words from the categories 
 * for quickly finding which categories a given search word is included in.
 *
 * The tree itself is only a tree of ids, and to get name for each id
 * call getTranslation with the language and optional fallback language.
 *
 * TODO: No implementation in h-file, only inlines after class. 
 *       Improve documentation, not too long lines.
 *       Cleanup code to follow coding standard.
 *       
 *
 */
class CategoryTree {
public:
   typedef uint32 StringID; ///< strings index into string vector
   /// maps language id to string ID
   typedef std::map< LangTypes::language_t, StringID > TranslationMap;
   /// maps category id to translation map
   typedef std::map< CategoryID, TranslationMap > CategoryNames;
   /// maps words to category ids
   typedef std::vector< pair< MC2String, CategoryID > > CategoryIndex;
   /// maps each language to an indexing data structure
   typedef std::map< LangTypes::language_t, CategoryIndex > LanguageToIndex;

   /// The full string index and category id.
   typedef std::pair< CategoryID, StringID > SearchCatData;
   /// The full string and category id.
   typedef std::pair< MC2String, CategoryID > SearchStringAndCategory;
   /// Maps each string to Search string id.
   typedef std::vector< pair< MC2String, StringID > > CatStringMap;
   /// CatStringMap for languages.
   typedef std::map< LangTypes::language_t, CatStringMap > LanguageToStringMap;
   typedef std::set< StringID > SearchCategorySet;
   /// CategoryID to icon map
   typedef std::map< CategoryID, MC2String > CategoryIdToIconMap;

   /// a single category and it's children
   class Node: private NotCopyable {
   public:
      typedef std::vector<Node*> NodeVector;
      /**
       * @param id The category id of this node.
       * @param children all child nodes for this node.
       */
      Node( CategoryID id, 
            const NodeVector& children ):
         m_id( id ),
         m_children( children ) {
      }
      ~Node();
      /// @return category id
      CategoryID getID() const { return m_id; }
      /// @return children
      const NodeVector& getChildren() const { return m_children; }
      /// non-const version of getChildren
      NodeVector& getChildren() { return m_children; }
      
   private:
      CategoryID m_id; ///< category id
      NodeVector m_children; ///< child nodes of this category
   };

   /**
    * Describes a node completely. A list of these is enough to define a
    * localized category tree.
    *
    * Includes name and icon, refers to sub-categories 
    * by id instead of with pointers.
    */
   class StandaloneNode {
   public:
      StandaloneNode( CategoryID id,
                      const vector<CategoryID>& children,
                      const MC2String& name,
                      const MC2String& icon ):
            m_id( id ),
            m_children( children ),
            m_name( name ),
            m_icon( icon ) {
      }
      /// @return category id
      CategoryID getID() const { return m_id; }
      /// @return children
      const vector<CategoryID>& getChildren() const { return m_children; }
      /// @return name
      const MC2String getName() const { return m_name; }
      /// @return icon
      const MC2String getIcon() const { return m_icon; }
      
   private:
      CategoryID m_id; ///< category id
      vector<CategoryID> m_children; ///< child categories, sorted alphabetically
      MC2String m_name; ///< the localized name of this category
      MC2String m_icon; ///< the icon for this category
   };


   /// a map of category id to its sub categories
   typedef std::map< CategoryID, const Node::NodeVector* > IdToSubCategories;

   /// Default constructor
   CategoryTree() {}

   /// Copy constructor
   CategoryTree( const CategoryTree& other );

   ~CategoryTree();

   /**
    * Loads categories and category translations. If load fails the old data
    * will intact.
    * @param categoryTreeFileName  File name of the category tree file
    * @param defaultConfigFileName File name of the default config file.
    * @return true if the load was successful
    */
   bool load( const MC2String& categoryTreeFileName,
              const MC2String& defaultConfigFileName );


   /**
    * Gets a translated category name for category id.
    * @param categoryID id of the category.
    * @param lang requested language for the category
    * @param fallbackLang The fallback language to use if the first language
    *                     fails.
    * @return translated string
    */
   const MC2String&
   getTranslation( CategoryID categoryID, 
                   LangTypes::language_t lang,
                   LangTypes::language_t fallbackLang =
                   LangTypes::invalidLanguage ) const;

   /// @return root nodes of the tree
   const Node::NodeVector& getRootNodes() const {
      return m_rootNodes;
   }

   /**
    * Get all sub categories for the "rootCategory" including the
    * "rootCategory".
    * @param rootCategory the root category to fetch all categories for
    * @param subcatsIncludingRoots subcategories including the root categories
    */
   void getAllCategories( CategoryID rootCatetory,
                          CategorySet& subcatsIncludingRoots ) const;

   /**
    * Searches for matching categories.
    *
    * @param name The name of the categories to match, i.e "rest", matches
    *             restaurant, restroom etc.
    * @param lang The language to search in.
    * @param matchingCategories a vector containing matching categories
    *                           including all the children.
    * @param exactMatch If the name of the categories need to be an exact
    *                   match.
    * @param noCategoryWords If a word is not matched to a category it will be
    *                        added to this.
    * @return number of matching categories including the sub categories
    */
   uint32 findCategories( const MC2String& name,
                          LangTypes::language_t lang,
                          CategorySet& matchingCategories,
                          bool exactMatch = false,
                          vector< MC2String >* noCategoryWords = NULL ) const;

   /**
    * Searches for matching categories.
    *
    * @param name The string to find categories for.
    * @param lang The language to search in.
    * @param matchingCategories a vector containing matching categories
    *                           including all the children.
    * @param exactMatch If the name of the categories need to be an exact
    *                   match.
    * @param noCategoryWords If a word is not matched to a category it will be
    *                        added to this.
    * @return number of matching categories including the sub categories.
    */
   uint32 findNameCategories( 
      const MC2String& name,
      LangTypes::language_t lang,
      CategorySet& matchingCategories,
      bool exactMatch = false,
      vector< MC2String >* noCategoryWords = NULL ) const;

   /**
    * Returns localized categories in a flat standalone format. Is used to
    * serialize the tree, @see serializeTree
    *
    * The vector is sorted by category id.
    *
    * @param language The language to localize to.
    */
   vector<StandaloneNode> getStandaloneNodes( 
      LangTypes::language_t language ) const;

   /**
    * Functor which determines whether a category id should be included
    * in the tree or not.
    */
   class InclusionPredicate {
   public:
      virtual bool operator()( CategoryID id ) const = 0;
   };

   /**
    * Removes all categories which the predicate doesn't want to include.
    *
    * @param predicate Functor which says which categories to keep.
    */
   void removeCategories( const InclusionPredicate& predicate );

   /**
    * Adds/replaces an icon for a category
    *
    * @param id   Icon valid for this category id.
    * @param icon New icon name.
    */
   void setIcon( CategoryID id, const MC2String& icon );

private:
   /** 
    * Parse the category DOM tree.
    *
    * @param node          Node to parse
    * @param root          [out] New root structure
    * @param childMap      [out] Ids of all children 
    * @param defaultConfig The default tree configuration e.g. icons.
    * @param parentIcon    Icon of the parent. Used if none exist for child.
    */
   void parseIDs( const DOMNode* node, 
                  Node::NodeVector& root,
                  IdToSubCategories& childMap,
                  DefaultConfiguration* defaultConfig,
                  const MC2String& parentIcon );
   
   /**
    * Parses a node in the category DOM tree recursively.
    * For parameteres see parseIDs
    */
   void parseTree( const DOMNode* node,
                   Node::NodeVector& root,
                   IdToSubCategories& childMap,
                   DefaultConfiguration* defaultConfig,
                   const MC2String& parentIcon );
   /**
    * @param begin start of range to add.
    * @param end end of range to add.
    * @param selectID Functor to select the category id from the iterators.
    * @param categoryIDs Will be filled in with category ids including 
    *                    subcategories.
    * @param validateIDs If true the ids will be checked for validity
    *                    before insertion.
    */
   template <typename Iterator, typename IDSelector>
   void getAllCategories( Iterator begin, Iterator end, 
                          IDSelector selectID,
                          CategorySet& categoryIDs,
                          bool validateIDs ) const;

   /**
    * Help function to removeCategories, does actual recursive
    * removal of the Nodes in the tree structure.
    */
   void removeCategories( const InclusionPredicate& predicate,
                          Node::NodeVector& rootNodes );
   
   ///< The names of all categories in all languages (unsorted).
   std::vector<MC2String> m_strings; 

   /**
    * For each language this map contains an index for quickly looking up
    * which categories a word is included in.
    */
   LanguageToIndex m_searchMap;

   /**
    * Map for category ID -> language -> string ID.
    * Used to retrieve the name of a category given a category ID and
    * a language. The string ID is an index into m_strings.
    */
   CategoryNames m_names;

   /// Root nodes of the category tree
   Node::NodeVector m_rootNodes; 

   /** a map of category id to its sub categories.
    * This could as well been a map to the actual node, but that doesn't really matter
    * for the current implementation.
    */
   IdToSubCategories m_idToChildren; 

   /** For empty translation return value
    * @see getTranslation
    */
   MC2String m_emptyTransl; 


    /// The full search names of all categories in all languages (unsorted).
   vector< SearchStringAndCategory > m_searchStrings; 
 
   /**
    * For each language this map contains an index for quickly looking up
    * a category's search name to identifier.
    */
   LanguageToStringMap m_searchNameSearchMap;

   
   /// Category icon map
   CategoryIdToIconMap m_categoryIconMap;

};

namespace CategoryFunctor {

/// functor to translate category id to the format "name"
class Translate: public std::unary_function< CategoryID, const MC2String& > {
public:
   /**
    * @param tree category tree in which the category id resides.
    * @param lang language to translate the category id to.
    */
   Translate( const CategoryTree& tree, LangTypes::language_t lang ):
    m_tree( tree ), m_lang( lang ) { }

   const MC2String& operator()( const CategoryID& id ) const {
      return m_tree.getTranslation( id, m_lang );
   }
private:
   const CategoryTree& m_tree;
   LangTypes::language_t m_lang;
};

/// translates to the format "name(id)", @see Translate
struct TranslateWithID: public Translate {
   TranslateWithID( const CategoryTree& tree, LangTypes::language_t lang ):
      Translate( tree, lang ) { }

   const MC2String& operator() ( const CategoryID& id ) const {
      char buff[64];
      sprintf( buff, "(%d)", id );
      m_trans = Translate::operator ()( id ) + buff;
      return m_trans;
   }
private:
   mutable MC2String m_trans;
};

}

}

#endif // CATEGORYTREE_H
