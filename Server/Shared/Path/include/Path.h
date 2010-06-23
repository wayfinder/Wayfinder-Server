/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PATH_PATH_H
#define PATH_PATH_H

#include "PathNode.h"
#include "PathTypes.h"

#include "StringUtility.h"


/**
 * @namespace Common components for creating JPath and XPath evaluators.
 */
namespace Path {

/**
 * @namespace Implementation specific, do not use directly.
 */
namespace Impl {

/**
 * @param node the node to get filled with new nodes
 * @param nodeNames the name of each new node, in depth order
 * @return last node created
 */
NameNode* fillNode( NameNode* node, const vector<MC2String>& nodeNames );

/**
 * @param lastNode
 * @param names in depth order names to search for
 * @return node to where a new expression can be inserted with specific depth
 *         into names vector
 */
pair<NameNode*, uint32> findDepth( NodeList& list,
                                   const vector<MC2String>& names );

/// prints name node tree recursively
void printNode( ostream& ostr, const NameNode& node, uint32 depth = 2 );

template < typename T >
void createStringMatrix( vector< vector< MC2String > >& stringsMatrix,
                         const T& desc,
                         const MC2String& nspace ) {
   MC2String realNamespace = nspace + ":";
   for ( uint32 i = 0; i < desc.size(); ++i ) {
      vector<MC2String> strings;
      StringUtility::splitToVector( desc[ i ].m_name, '/', strings );
      if ( ! nspace.empty() ) {
         // prefix namespace
         for ( uint32 nspaceIdx = 0;
               nspaceIdx < strings.size(); ++nspaceIdx ) {
            // modify string if its not empty and not an
            // attribute
            if ( ! strings[ nspaceIdx ].empty() &&
                 strings[ nspaceIdx ][ 0 ] != '@' ) {
               strings[ nspaceIdx ].insert( 0, realNamespace );
            }
         }
      }

      stringsMatrix.push_back( strings );
   }

}

} // Impl

/**
 * Creates rootNode from description
 * @param desc description of the tree in XPath style
 * @param nspace the namespace name
 */
template < typename EvaluatorMap, typename Desc >
void createNodeTree( NameNode& rootNode,
                     EvaluatorMap& functions,
                     const Desc& desc,
                     const MC2String& nspace ) {

   // create a matrix of tokenized strings
   vector< vector< MC2String > > stringsMatrix;
   Impl::createStringMatrix( stringsMatrix, desc, nspace );


   // now stringMatrix should have desc.size() number of columns

   NodeList nodes;

   //
   // For each matrix, find a matching tree node
   //   if no tree node is found
   //      create a new node and fill it with matrix row values
   //   else
   //      insert new nodes at specific depth with the matrix row values
   //
   //   map leaf node of the newly inserted branch to the functor
   //
   for ( uint32 i = 0; i < stringsMatrix.size(); ++i ) {
      // we are not interested in empty strings
      if ( stringsMatrix[ i ].empty() ) {
         delete desc[ i ].m_evaluator;
         continue;
      }
      NameNode* leaf = NULL;


      // find depth
      pair<NameNode*, uint32> nodeAndDepth =
         Impl::findDepth( nodes, stringsMatrix[ i ] );

      // depth found?
      if ( nodeAndDepth.first == NULL ) {
         // no node found, create a new tree and attach it
         NameNode* newTree = new NameNode( stringsMatrix[ i ][ 0 ] );
         nodes.push_back( newTree );
         leaf = Impl::fillNode( newTree, stringsMatrix[ i ] );

      } else {
         // found a (sub)tree, attach values
         leaf = Impl::
            fillNode( nodeAndDepth.first,
                      vector<MC2String>( stringsMatrix[ i ].begin() +
                                         nodeAndDepth.second - 1,
                                         stringsMatrix[ i ].end() ) );
      }

      // in cases of duplicated sequences like:
      //  /a/b/c
      //  /a/b/c
      // the function map will leak memory unless we search it
      // and destroy the duplicate
      if ( functions.find( leaf ) != functions.end() ) {
         delete functions[ leaf ];
      }

      // map leaf to functor
      functions[ leaf ] = desc[ i ].m_evaluator;
   }
   // copy all root nodes to the final root node
   rootNode.copyList( nodes );
   // For debug:
   //   printNode( mc2dbg, rootNode );

}

} // Path

#endif // PATH_PATH_H
