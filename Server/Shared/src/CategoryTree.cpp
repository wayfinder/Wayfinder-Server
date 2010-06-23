/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CategoryTree.h"

#include "StringSearchUtility.h"
#include "File.h"
#include "XMLNodeIterator.h"
#include "XMLTool.h"
#include "XPathExpression.h"
#include "CategoryTranslationLoader.h"
#include "STLUtility.h"
#include "LocaleUtility.h"
#include "CategoryTreeDefaultConfiguration.h"

#include <boost/tokenizer.hpp>

using namespace XMLTool;
using namespace CategoryTreeUtils;

namespace {

/** 
 * When splitting search strings and categories into tokens
 * these separators are used. 
 */ 
const char* STRING_SEPARATORS = " ,.";

/**
 * Checks if a string consists of several words. 
 * @param str  The string to check.
 * @return     Wether the string consists of several words. 
 */
bool isCompositeString( const MC2String& str ) {
   return str.find_first_of( STRING_SEPARATORS ) != MC2String::npos;
}

/**
 * Splits a string into tokens.
 * @param str  The search string to split.
 * @return     The tokens.
 */
vector<MC2String> splitCompositeString( const MC2String& str ) {
   vector<MC2String> res;
   
   typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
   boost::char_separator<char> sep( STRING_SEPARATORS );
   tokenizer tokens( str, sep );
   
   for ( tokenizer::iterator tok_iter = tokens.begin();
         tok_iter != tokens.end(); ++tok_iter ) {
      res.push_back( *tok_iter );
   }
   
   return res;
}


class TreeTranslationLoader: public CategoryTranslationLoader {
public:
   /**
    * @param names category id to translation map.
    * @param strings category names.
    * @param searchStrings A vector of converted strings into lower case.
    * @param searchMap A language to string map for converted strings and 
    *                  category IDs
    * @param searchNameSearchMap Map for names of categories that is suitable 
    *                            for users to search in.
    */
   TreeTranslationLoader( 
      CategoryTree::CategoryNames& names,
      vector<MC2String>& strings,
      CategoryTree::LanguageToIndex& searchMap,
      vector< CategoryTree::SearchStringAndCategory >& searchStrings,
      CategoryTree::LanguageToStringMap& searchNameSearchMap )
         : m_names( names ),
           m_strings( strings ),
           m_searchMap( searchMap ),
           m_searchStrings( searchStrings ),
           m_searchNameSearchMap( searchNameSearchMap ) {
   }
   

protected:
   void newTranslationValue( uint32 catID, 
                             LangTypes::language_t lang, 
                             const MC2String& stringValue ) {
      const CategoryTree::StringID stringID = m_strings.size();
      // add string id
      m_names[ catID ][ lang ] = stringID;
      
      // add to names and converted names
      m_strings.push_back( stringValue );

      // split the category name into words and add each word to the indexer
      vector<MC2String> words = splitCompositeString( stringValue );
      for ( size_t i = 0; i < words.size(); ++i) {
         MC2String wordClose = StringSearchUtility::
                               convertIdentToClose( words[ i ] );
         m_searchMap[ lang ].push_back( make_pair( wordClose, catID ) );
      }
   }

   void newSearchNameValue( uint32 catID, 
                            LangTypes::language_t lang, 
                            const MC2String& stringValue ) {
      //( catID, lang, stringValue, m_searchNameSearchMap );
      // Add search word to full name and categoryId
      
      const CategoryTree::StringID stringID = m_searchStrings.size();
      m_searchStrings.push_back( make_pair( stringValue, catID ) );
      
      // split the category name into words and add each word to the indexer
      vector<MC2String> words = splitCompositeString( stringValue );
      for ( size_t i = 0; i < words.size(); ++i) {
         MC2String wordClose = StringSearchUtility::
            convertIdentToClose( words[ i ] );
         m_searchNameSearchMap[ lang ].push_back( 
            make_pair( wordClose, stringID ) );
      }
   }

private:
   CategoryTree::CategoryNames& m_names;
   vector<MC2String>& m_strings;
   CategoryTree::LanguageToIndex& m_searchMap;
   vector< CategoryTree::SearchStringAndCategory >& m_searchStrings; 
   CategoryTree::LanguageToStringMap& m_searchNameSearchMap;
};

/**
 * Used by the copy constructor to copy the Node objects and the
 * id to NodeVector map.
 *
 * @param newRoots      The new, copied, roots are placed here.
 * @param oldRoots      The old roots from which to copy.
 * @param idToChildren  The id to NodeVector map which should be built by
 *                      this function.
 */
void copyTreeStructure( CategoryTree::Node::NodeVector& newRoots,
                        const CategoryTree::Node::NodeVector& oldRoots,
                        CategoryTree::IdToSubCategories& idToChildren ) {
   typedef CategoryTree::Node::NodeVector::const_iterator Iterator;
   
   for ( Iterator itr = oldRoots.begin(); itr != oldRoots.end(); ++itr ) {
      CategoryID id = (*itr)->getID();
      CategoryTree::Node::NodeVector children;

      // first copy the children of this node
      if ( (*itr)->getChildren().size() > 0 ) {
         copyTreeStructure( children, (*itr)->getChildren(), idToChildren );
      }

      // create the new root
      CategoryTree::Node* newRoot = new CategoryTree::Node( id, children );

      // add it to the mapping, but only if there are children,
      // if there are no children it could be because this is a link
      if ( newRoot->getChildren().size() > 0 ) {
         idToChildren[ id ] = &newRoot->getChildren();
      }

      newRoots.push_back( newRoot );
   }
}

}

namespace CategoryTreeUtils {

CategoryTree::Node::~Node() {
   STLUtility::deleteValues( m_children );
}

CategoryTree::CategoryTree( const CategoryTree& other )
      : m_strings( other.m_strings ),
        m_searchMap( other.m_searchMap ),
        m_names( other.m_names ),
        m_searchStrings( other.m_searchStrings ),
        m_searchNameSearchMap( other.m_searchNameSearchMap ),
        m_categoryIconMap( other.m_categoryIconMap ) {

   copyTreeStructure( m_rootNodes, other.m_rootNodes,
                      m_idToChildren );
}

CategoryTree::~CategoryTree() {
   STLUtility::deleteValues( m_rootNodes );
}

bool 
CategoryTree::load( const MC2String& categoryTreeFileName, 
                    const MC2String& defaultConfigFileName) try {

   // Load the category tree xml into the DOM parser
   XercesDOMParser parser;
   parser.setValidationScheme( XercesDOMParser::Val_Auto );
   parser.setIncludeIgnorableWhitespace( false );

   parser.parse( categoryTreeFileName.c_str() );

   if ( parser.getDocument() == NULL ) {
      return false;
   }
   
   // Load the default configuration for the catgory tree
   std::auto_ptr< DefaultConfiguration > defConf( 
      loadDefaultConfiguration( defaultConfigFileName ) );
   
   // Parse the category tree i.e. the structure of the tree
   Node::NodeVector rootNodes;
   IdToSubCategories childIDs;
   parseIDs( parser.getDocument(), rootNodes, childIDs, 
             defConf.get(), "tat_shop");

   // Load category tree translation strings
   CategoryNames names;
   vector<MC2String> strings;
   LanguageToIndex searchMap;
   vector< SearchStringAndCategory > searchStrings; 
   LanguageToStringMap searchNameSearchMap;
   ::TreeTranslationLoader( names, strings, searchMap, 
                            searchStrings, searchNameSearchMap ).
        loadTranslations( parser.getDocument() );
   
   // Now it's safe to destroy old data and assign new
   m_idToChildren.swap( childIDs );
   STLUtility::deleteValues( m_rootNodes );
   m_rootNodes.swap( rootNodes );
   m_names.swap( names );
   m_strings.swap( strings );
   m_searchMap.swap( searchMap );
   m_searchStrings.swap( searchStrings );
   m_searchNameSearchMap.swap( searchNameSearchMap );
   
   // Sort the category index for each language so we can search in O(log n)
   // with std::equal_range
   for ( LanguageToIndex::iterator it = m_searchMap.begin(), 
            endIt = m_searchMap.end() ; it != endIt ; ++it ) {
      std::sort( (*it).second.begin(), (*it).second.end() );
   }
   for ( LanguageToStringMap::iterator it = m_searchNameSearchMap.begin(), 
            endIt = m_searchNameSearchMap.end() ; it != endIt ; ++it ) {
      std::sort( (*it).second.begin(), (*it).second.end() );
   }
   
   return true;
} catch ( const MC2Exception& e ) {
   mc2log << warn << "Failed to load category tree: " << e.what() << endl;
   return false;
} catch ( ... ) {
   return false;
}


void 
CategoryTree::parseTree( const DOMNode* node, 
                         Node::NodeVector& root,
                         IdToSubCategories& childMap,
                         DefaultConfiguration* defaultConfig,
                         const MC2String& parentIcon ) {

   // only interested in cat_node
   if ( ! XMLString::equals( node->getNodeName(), "cat_node" ) ) {
      return;
   }

   uint32 id;
   XMLTool::getAttrib( id, "cat_id", node );

   MC2String icon = parentIcon;
   if ( defaultConfig != NULL ) {
      // Fetch the default icon
      icon = defaultConfig->getCategoryIcon( id );
      if ( icon.empty() ) {
         icon = parentIcon;
      }
   }

   // check children
   CategoryTree::Node::NodeVector childIDs;
   ElementConstIterator elIt( node->getFirstChild() );
   ElementConstIterator elItEnd( NULL );
   for (; elIt != elItEnd; ++elIt ) {
      parseTree( *elIt, childIDs, childMap, 
                 defaultConfig, icon );
   }

   m_categoryIconMap.insert( make_pair( id, icon ) );
   Node* child = new Node( id, childIDs );
   root.push_back( child );
   // map child id to it's children
   if ( ! childIDs.empty() ) {
      childMap[ id ] = &child->getChildren();
   }
}


void 
CategoryTree::parseIDs( const DOMNode* node, 
                        Node::NodeVector& root,
                        IdToSubCategories& childMap,
                        DefaultConfiguration* defaultConfig,
                        const MC2String& parentIcon ) {
   if ( node == NULL ) {
      return;
   }

   // get root category id nodes
   XPath::Expression::result_type rootNodes = 
      XPath::Expression( "/category_data/category_tree/cat_node*" ).
      evaluate( node->getFirstChild() );
   for ( uint32 i = 0; i < rootNodes.size(); ++i ) {
      // get all category child nodes of these nodes
      parseTree( rootNodes[ i ], root, childMap, 
                   defaultConfig, parentIcon );
   }
}


void 
CategoryTree::setIcon( CategoryID id, const MC2String& icon ) {
   // Remove the icon from the default config
   m_categoryIconMap.erase( id );

   // Put the icon into the map
   m_categoryIconMap.insert( make_pair( id, icon ) );
}


const MC2String&
CategoryTree::getTranslation( CategoryID categoryID, 
                              LangTypes::language_t lang,
                              LangTypes::language_t fallbackLang ) const {

   CategoryNames::const_iterator it = m_names.find( categoryID );
   if ( it == m_names.end() ) {
      // did not find category
      return m_emptyTransl;
   }

   TranslationMap::const_iterator transIt = (*it).second.find( lang );
   if ( transIt == (*it).second.end() ) {
      if ( fallbackLang == LangTypes::invalidLanguage ) {
         // did not find correct language
         return m_emptyTransl;
      } 
      // else lets try fallback language
      transIt = (*it).second.find( fallbackLang );
      if ( transIt == (*it).second.end() ) {
         // fallback language did not succed either.
         return m_emptyTransl;
      }
   }

   return m_strings[ (*transIt).second ];
}

namespace {

/// Compares beginining of a string
struct BeginningOfWord {
   typedef CategoryTree::CategoryIndex::value_type CompareType;
   typedef CategoryTree::CatStringMap::value_type SearchType;

   BeginningOfWord( ) {
   }

   bool operator ()( const CompareType& a, const MC2String& b) const {
      return strncmp( a.first.c_str(), b.c_str(), b.size() ) < 0;
   }

   bool operator ()( const MC2String& b, const CompareType& a ) const {
      return strncmp( b.c_str(), a.first.c_str(), b.size() ) < 0;
   }

   bool operator ()( const SearchType& a, const MC2String& b) const {
      return strncmp( a.first.c_str(), b.c_str(), b.size() ) < 0;
   }

   bool operator ()( const MC2String& b, const SearchType& a ) const {
      return strncmp( b.c_str(), a.first.c_str(), b.size() ) < 0;
   }
};


/// Compares for an exact match of a word
struct ExactMatchOfWord {
   typedef CategoryTree::CategoryIndex::value_type CompareType;
   typedef CategoryTree::CatStringMap::value_type SearchType;

   ExactMatchOfWord() {
   }

   bool operator ()( const CompareType& a, const MC2String& b) const {
      return strcmp( a.first.c_str(), b.c_str() ) < 0;
   }

   bool operator ()( const MC2String& b, const CompareType& a ) const {
      return strcmp( b.c_str(), a.first.c_str() ) < 0;
   }

   bool operator ()( const SearchType& a, const MC2String& b) const {
      return strcmp( a.first.c_str(), b.c_str() ) < 0;
   }

   bool operator ()( const MC2String& b, const SearchType& a ) const {
      return strcmp( b.c_str(), a.first.c_str() ) < 0;
   }
};

}
namespace {

/**
 * Adds current id and its children to the set of ids
 * @param ids the set of added ids
 * @param currentID the id to insert into ids
 * @param childMap a category id to its children vector map
 */
void addChildren( CategoryTreeUtils::CategorySet* ids,
                  CategoryID id,
                  const CategoryTree::IdToSubCategories& childMap ) {
   if ( !ids->insert( id ).second ) {
      // already inserted this id, skip the children
      return;
   }

   // add children
   CategoryTree::IdToSubCategories::const_iterator children = 
      childMap.find( id );

   if ( children != childMap.end() ) {
      typedef CategoryTree::Node::NodeVector::const_iterator Iterator;
      Iterator itrEnd = children->second->end();
      for ( Iterator itr = children->second->begin(); itr != itrEnd; ++itr ) {
         addChildren( ids, (*itr)->getID(), childMap );
      }
   }
}

}

template <typename Iterator, typename IDSelector>
void CategoryTree::
getAllCategories( Iterator begin, Iterator end, IDSelector selectID,
                  CategorySet& categoryIDs, bool validateIDs ) const { 

   // add ids as matches, add the children as matches too
   for ( Iterator it = begin; it != end; ++it ) {
      const CategoryID currentID = selectID( *it );
      
      // validation? then skip invalid ids
      if ( validateIDs && m_names.find( currentID ) == m_names.end() ) {
         continue;
      }
      
      addChildren( &categoryIDs, currentID, m_idToChildren );
   }
}

void 
CategoryTree::getAllCategories( CategoryID rootID,
                                CategorySet& subcatsIncludingRoots ) const {
   CategorySet cats;
   cats.insert( rootID );
   getAllCategories( cats.begin(), cats.end(),
                     STLUtility::makeIdentity( cats ),
                     subcatsIncludingRoots, 
                     true // validate the id 
                     );
}


uint32 
CategoryTree::findCategories( const MC2String& nameOrg, 
                              LangTypes::language_t language,
                              CategorySet& matchingCategories,
                              bool exactMatch,
                              vector< MC2String >* noCategoryWords ) const {
   // does the search string consist of several words?
   if ( isCompositeString( nameOrg ) ) {
      // split the string into words, do one recursive search for each word and
      // return the results which turn up in all recursive searches.
      vector<MC2String> names = splitCompositeString( nameOrg );

      if ( names.empty() ) {
         // the string was only composed by separators.
         return 0;
      }

      findCategories( names[ 0 ], language, matchingCategories, exactMatch, noCategoryWords );
      
      for ( size_t i = 1; i < names.size(); ++i ) {
         CategorySet nextCategories;
         CategorySet intersection;
         findCategories( names[ i ], language, nextCategories, exactMatch, noCategoryWords );
         set_intersection( matchingCategories.begin(), matchingCategories.end(),
                           nextCategories.begin(), nextCategories.end(),
                           inserter( intersection, intersection.begin() ) );
         if ( ! exactMatch ) {
            matchingCategories.swap( intersection );
         } else {
            if ( intersection.empty() ) {
               matchingCategories.insert( nextCategories.begin(),
                                          nextCategories.end() );
            } else {
               matchingCategories.swap( intersection );
            }
         }
      }
   }
   else {
      // search for just one word
      
      // first get the index for the specified language 
      LanguageToIndex::const_iterator langIt = m_searchMap.find( language );
      if ( langIt == m_searchMap.end() ) {
         // fallback on english
         langIt = m_searchMap.find( LangTypes::english );
         if ( langIt == m_searchMap.end() ) {
            // no match
          return 0;
         }
      }     
      
      // convert client search string to a "better" search string
      MC2String name = StringSearchUtility::convertIdentToClose( nameOrg );   
      // search in this language and return the ids
      typedef const CategoryIndex::value_type* RangeType;
      std::pair< RangeType, RangeType > range;
      if ( exactMatch ) {
            range = std::equal_range( &(*langIt).second[0],
                                      &(*langIt).second[ (*langIt).second.size() ], 
                                      name,
                                      ::ExactMatchOfWord() );
      } else {
            range = std::equal_range( &(*langIt).second[0],
                                      &(*langIt).second[ (*langIt).second.size() ], 
                                      name,
                                      ::BeginningOfWord() );
      }

      // get all matching categories and their sub categories
      getAllCategories( range.first, range.second, 
                        STLUtility::select2nd<CategoryIndex::value_type>(),
                        matchingCategories, 
                        false // don't validate, the ids are valid
                      );
      // Add names with out a category to the vector
      if ( matchingCategories.empty() && noCategoryWords != NULL ) {
         noCategoryWords->push_back( name );
      }
   }
   // return number of matches, including children
   return matchingCategories.size();
}

uint32
CategoryTree::findNameCategories( 
   const MC2String& nameOrg, 
   LangTypes::language_t language,
   CategorySet& matchingCategories,
   bool exactMatch,
   vector< MC2String >* noCategoryWords ) const 
{
   // Split input string into words
   vector<MC2String> names = splitCompositeString( nameOrg );
   // The used words, initialized to none
   vector<bool> namesUsed( names.size(), false );
   vector<MC2String> closeNames;
   closeNames.resize( names.size() );

   mc2dbg4 << "Names: " << STLUtility::it_dumper<vector<MC2String>::iterator>( 
      names.begin(), names.end() ) << endl;

   // Find all candidate categories, with at least one word same as input
   SearchCategorySet candidateCategories;
   // The index for the specified language TODO: Make function of this
   LanguageToStringMap::const_iterator langIt = m_searchNameSearchMap.find( 
      language );
   if ( langIt == m_searchNameSearchMap.end() ) {
      // fallback on english
      langIt = m_searchNameSearchMap.find( LangTypes::english );
      if ( langIt == m_searchNameSearchMap.end() ) {
         // Not even english!? No input words where used
         if ( noCategoryWords != NULL ) {
            for ( size_t i = 0, end = names.size() ; i < end ; ++i ) {
               noCategoryWords->push_back( names[ i ] );
            }
         }
         return 0;
      }
   }

   // Now get the categories with matching name in the language
   typedef const CatStringMap::value_type* RangeType;
   std::pair< RangeType, RangeType > range;
   for ( size_t i = 0, end = names.size() ; i < end ; ++i ) {
      closeNames[ i ] = StringSearchUtility::convertIdentToClose( names[ i ] );
      mc2dbg4 << "Word " << closeNames[ i ] << endl;
      if ( exactMatch ) {
         range = std::equal_range( 
            &(*langIt).second[0], &(*langIt).second[ (*langIt).second.size() ],
            closeNames[ i ], ::ExactMatchOfWord() );
      } else {
         range = std::equal_range( 
            &(*langIt).second[0], &(*langIt).second[ (*langIt).second.size() ],
            closeNames[ i ], ::BeginningOfWord() );
      }
      for ( RangeType it = range.first ; it != range.second ; ++it ) {
         candidateCategories.insert( (*it).second );
      }
   }

   // For all candidates check if all words is present in input (and together)
   CategorySet fullyMatchedCategories;
   for ( SearchCategorySet::iterator it = candidateCategories.begin(), endIt = 
            candidateCategories.end() ; it != endIt ; ++it ) {
      MC2String catName = m_searchStrings[ *it ].first;
      mc2dbg4 << "Category " << catName << endl;
      vector<MC2String> catNames = splitCompositeString( catName );
      mc2dbg4 << "CatNames: (" << catNames.size() << ") " 
              << STLUtility::it_dumper<vector<MC2String>::iterator>( 
                catNames.begin(), catNames.end() ) << endl;
      vector<MC2String>::iterator catWordIt = catNames.begin();
      vector<MC2String>::iterator firstNameWordIt = 
         std::find( closeNames.begin(), closeNames.end(), 
                    StringSearchUtility::convertIdentToClose( *catWordIt ) );
      vector<MC2String>::iterator nameWordIt = firstNameWordIt;
      size_t nbrWordsFound = 0;
      // Check if the categories names are in names
      mc2dbg4 << "Matching CatNames:";
      while( nameWordIt != closeNames.end() && catWordIt != catNames.end() ) {
         MC2String catItName( StringSearchUtility::convertIdentToClose(  
                                 *catWordIt ) );
         mc2dbg4 << " " << catItName;
         if ( *nameWordIt == catItName ) {
            ++nbrWordsFound;
         } else {
            // Not same
            break;
         }
         ++nameWordIt;
         ++catWordIt;
      }
      mc2dbg4 << endl;
      mc2dbg4 << "nbrWordsFound " << nbrWordsFound << " of " << catNames.size()
             << endl;
      if ( nbrWordsFound == catNames.size() ) {
         MC2String catCandidate = getTranslation( m_searchStrings[ *it ].second,
                                                  langIt->first );

         // Check if this category is a substring of existing category or
         // if it is a better match than an existing.
         bool subCat = false;
         for( CategorySet::iterator iter = fullyMatchedCategories.begin();
              iter != fullyMatchedCategories.end(); ++iter ) {
            MC2String str = getTranslation( *iter, langIt->first);
            if ( str.length() >= catCandidate.length() ) {
               const char* stringPos = strcasestr( str.c_str(), catCandidate.c_str() );
               subCat == (stringPos != NULL) && stringPos != str.c_str();
               if( subCat ) {
                  mc2dbg << "Rejected category: " << catCandidate 
                         << " better match exists." << endl;
               }
            } else if ( strcasestr(catCandidate.c_str(), str.c_str() ) != NULL ) {
               // Remove saved category because this one is better
               mc2dbg/*4*/ << "Removing category " << getTranslation( *iter, LangTypes::english )
                           << "(" << *iter << ")" << " because a better category match was found." 
                           << endl;
               fullyMatchedCategories.erase( iter );
            }
         }

         if ( !subCat ) {     
            // Add this category
            mc2dbg/*4*/ << "Adding category "
                        << getTranslation( m_searchStrings[ *it ].second,
                                           LangTypes::english ) << "(" 
                        << m_searchStrings[ *it ].second << ") from " 
                        << catName << endl;
            fullyMatchedCategories.insert( m_searchStrings[ *it ].second );
            // Mark all words in input that resulted in a full category
            for ( size_t i = 0 ; i < nbrWordsFound ; ++i ) {
               namesUsed[ (firstNameWordIt - closeNames.begin()) + i ] = true;
            }
         }
      }
   }


   // Remaining words 
   if ( noCategoryWords != NULL ) {
      for ( size_t i = 0, end = names.size() ; i < end ; ++i ) {
         if ( !namesUsed[ i ] ) {
            noCategoryWords->push_back( names[ i ] );
         }
      }
   }
   
   // Expand matched categories to subcategories
   getAllCategories( fullyMatchedCategories.begin(), 
                     fullyMatchedCategories.end(), 
                     STLUtility::makeIdentity( fullyMatchedCategories ),
                     matchingCategories, 
                     false/* don't validate, the ids are valid*/ );

   return matchingCategories.size();
}

/**
 * Used to sort a sequence of pair<MC2String, CategoryID> alphabetically
 * according to a specific language's locale.
 */
class LocaleComparePair {
public:
   LocaleComparePair( LangTypes::language_t language )
         : m_locale( language ) {}

   bool operator()( const pair<MC2String, CategoryID>& lhs,
                    const pair<MC2String, CategoryID>& rhs ) const {
      return m_locale.collate( lhs.first, rhs.first );
   }

private:
   LocaleUtility m_locale;
};

vector<CategoryTree::StandaloneNode> 
CategoryTree::getStandaloneNodes( LangTypes::language_t language ) const {
   CategorySet categorySet;
   
   // add all ids in the tree to categorySet
   for ( Node::NodeVector::const_iterator itr = m_rootNodes.begin();
         itr != m_rootNodes.end(); ++itr ) {
      addChildren( &categorySet, (*itr)->getID(), m_idToChildren );
   }

   vector<StandaloneNode> result;

   for ( CategorySet::iterator itr = categorySet.begin();
         itr != categorySet.end(); ++itr ) {

      MC2String icon;
      MC2String name = getTranslation( *itr, language, LangTypes::english );
      CategoryIdToIconMap::const_iterator findIt = m_categoryIconMap.find(*itr);
      if ( findIt != m_categoryIconMap.end() ) {
         icon = findIt->second;
      }

      // Get the children together with their names so we can sort
      vector< pair< MC2String, CategoryID> > childrenWithNames;

      // get the children for this category
      IdToSubCategories::const_iterator children = 
         m_idToChildren.find( *itr );
      
      if ( children != m_idToChildren.end() ) {
         for ( Node::NodeVector::const_iterator itr =  children->second->begin();
               itr != children->second->end(); ++itr) {
            CategoryID childID = (*itr)->getID();
            MC2String childName = getTranslation( childID, language, 
                                                  LangTypes::english );
            childrenWithNames.push_back( make_pair( childName, childID ) );
         }
      }

      sort( childrenWithNames.begin(), childrenWithNames.end(),
            LocaleComparePair( language ));

      // Transform to a vector with only the ids
      vector<CategoryID> childIDs;
      transform( childrenWithNames.begin(),
                 childrenWithNames.end(),
                 back_inserter( childIDs ),
                 STLUtility::select2nd< pair<MC2String, CategoryID> >()
                 );

      result.push_back( StandaloneNode( *itr, childIDs, name, icon ) );
   }
   
   return result;
}

void CategoryTree::removeCategories( const InclusionPredicate& predicate ) {
   // first remove the nodes from the tree structure
   removeCategories( predicate, m_rootNodes );

   // remove references to removed categories in various maps...

   // m_searchMap
   for ( LanguageToIndex::iterator itr = m_searchMap.begin();
         itr != m_searchMap.end(); ++itr ) {
      CategoryIndex& catIndex = (*itr).second;
      for ( CategoryIndex::iterator jtr = catIndex.begin(); 
            jtr != catIndex.end(); ) {
         if ( !predicate( (*jtr).second ) ) {
            jtr = catIndex.erase( jtr );
         } else {
            ++jtr;
         }
      }
   }

   // m_names
   for ( CategoryNames::iterator itr = m_names.begin(); 
         itr != m_names.end(); ) {
      if ( !predicate( (*itr).first ) ) {
         // the iterator pointing to the next pos isn't invalidated by erase
         m_names.erase( itr++ );
      } else {
         ++itr;
      }
   }

   // m_searchStrings
   for ( vector<SearchStringAndCategory>::iterator itr = m_searchStrings.begin();
         itr != m_searchStrings.end(); ) {
      if ( !predicate( (*itr).second ) ) {
         itr = m_searchStrings.erase( itr );
      } else {
         ++itr;
      }
   }
}

void CategoryTree::removeCategories( const InclusionPredicate& predicate,
                                     Node::NodeVector& rootNodes ) {
   // This function will remove Nodes from the tree structure if predicate
   // doesn't want to include them. If a child to a removed Node should be
   // included, it will propagate upwards until it reaches an ancestor which
   // should be included, or becomes a new root.
   
   // A map of the Nodes which should be in rootNodes after this call is done.
   set<Node*> nodesToKeep;
   set<Node*> nodesToDelete;

   for ( size_t i = 0; i < rootNodes.size(); ++i ) {
      // first remove recursively on this root's children
      Node::NodeVector& children = rootNodes[ i ]->getChildren();
      removeCategories( predicate, children );

      if ( !predicate( rootNodes[ i ]->getID() ) ) {
         // this root shouldn't be included, so if it has children that
         // should be included, make them new roots here
         nodesToKeep.insert( children.begin(), children.end() );
         // Children no longer in this root and it is not in the tree
         rootNodes[ i ]->getChildren().clear();
         nodesToDelete.insert( rootNodes[ i ] );
      } else {
         // keep this root
         nodesToKeep.insert( rootNodes[ i ] );
      }
   }

   // Delete the removed Nodes, but not their children.
   STLUtility::deleteValues( nodesToDelete );
   nodesToDelete.clear();
   // now keep only the stuff in nodesToKeep
   rootNodes.clear();
   rootNodes.insert( rootNodes.begin(),
                     nodesToKeep.begin(), nodesToKeep.end() );
}

}
