/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2JSON_JPATH_ASSIGNER
#define MC2JSON_JPATH_ASSIGNER

#include "JPathMultiExpression.h"
#include <boost/lexical_cast.hpp>
#include <vector>

namespace MC2JSON {

namespace JPath {

/**
 * Assigns a value from the node string value when it is evaluated
 * in the multi expression evaluator.
 * Default it will use \c boost::lexical_cast, but this can be specialized.
 */
template <typename T>
class Assigner: public MultiExpression::NodeEvaluator {
public:
   Assigner( T& val ):m_value( val ) { }
   virtual ~Assigner() {
   }
   inline void operator() ( const NodeType& node ) try {
      m_value = node.get_value< T >();
   } catch ( const std::exception& e ) {
      if ( typeid ( T ) != typeid( MC2String ) ) {
         // the any was not T, so lets try with string and 
         // lexical cast it.
         try {
            m_value =
               boost::lexical_cast< T >
               ( node.get_str() );
         } catch ( const std::exception& convE ) {
            throw EvaluateException( convE.what() );
         } catch ( ... ) {
            throw EvaluateException( "Unknown exception" );
         }
      } else {
         throw EvaluateException( e.what() );
      }
   }

private:
   T& m_value;
};

/**
 * Special case for vector.
 * @see Assigner
 */
template <typename T >
class VectorAssigner: public MultiExpression::NodeEvaluator {
public:
   typedef typename std::vector<T> Vector;
   explicit VectorAssigner( Vector& val ):m_value( val ) { }
   virtual ~VectorAssigner() {
   }

   void operator() ( const NodeType& node ) {
      T value;
      Assigner<T> helper( value );
      helper( node );
      m_value.push_back( value );
   }

private:
   Vector& m_value;
};

template <typename T>
VectorAssigner<T>*
makeAssigner( typename std::vector<T>& value ) {
   return new VectorAssigner< T >( value );
}

/// helper to create an assigner
template <typename T>
Assigner<T>* makeAssigner( T& value ) {
   return new Assigner<T>( value );
}

} // JPath

} // MC2JSON

#endif // MC2JSON_JPATH_ASSIGNER
