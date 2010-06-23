/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVASSIGNER_H
#define NAVASSIGNER_H

#include "config.h"

#include "NParam.h"
#include "NParamBlock.h"
#include "MC2String.h"
#include "MC2Coordinate.h"

namespace Nav {

/**
 * Interface for a navigator param block assigner
 */
class Assigner {
public:
   explicit Assigner( uint32 id ):
      m_id( id ) 
   { }

   virtual ~Assigner() {
   }

   virtual void assign( const NParam& block ) = 0;

   /// Sets variable value from parameter with specific ID in the block.   
   virtual void assignBlock( const NParamBlock& values ) {
      const NParam* param = values.getParam( getParamID() );
      if ( param != NULL ) {
         assign( *param );
      }
   }
   uint32 getParamID() const { return m_id; }
private:
   const uint32 m_id; ///< ID in the parameter block 
};


/** Type specific assigner.
 */
template <typename T>
class AssignerT: public Assigner {
public:
   /** 
    * Setup an assigner with specific type, id and variable to assign
    * @param type Parameter type
    * @param id Specific parameter id
    * @param val Variable to assign
    */
   AssignerT( uint32 id, T& val ):
      Assigner( id ),
      m_value( val )
   { }

   virtual ~AssignerT() { 
   }

   /// Sets variable value from a paramter
   void assign( const NParam& param );

   /// Sets variable value, can be assign from other types
   template <typename O>
   void setValue( const O& t ) {
      m_value = t;
   }
private:
   T& m_value; ///< Variable to assign from paramter block
};

/** Type specific assigner that assigns default value if 
 * parameter is missing.
 */
template <typename T>
class AssignerTDefault: public AssignerT< T > { 
public:
   AssignerTDefault( uint32 id, T& assignThis, const T& defaultValue ):
      AssignerT<T>( id, assignThis ),
      m_defaultValue( defaultValue ) 
   { }

   void assignBlock( const NParamBlock& values ) {
      const NParam* param = values.getParam( Assigner::getParamID() );
      if ( param != NULL ) {
         AssignerT<T>::assign( *param );
      } else {
         setValue( m_defaultValue );
      }
   }

private:
   T m_defaultValue;
};

template <>
void AssignerT<MC2Coordinate>::assign( const NParam& param ) {
   int32 lat = param.getInt32Array( 0 ); 
   int32 lon = param.getInt32Array( 1 );
   m_value = MC2Coordinate( Nav2Coordinate( lat, lon ) );
}

template <>
void AssignerT<MC2String>::assign( const NParam& param ) {
   m_value = param.getString( false ); // not using latin1 here
}

template <>
void AssignerT<byte>::assign( const NParam& param ) {
   m_value = param.getByte();
}

template <>
void AssignerT<uint16>::assign( const NParam& param ) {
   m_value = param.getUint16();
}

template <>
void AssignerT<uint32>::assign( const NParam& param ) {
   m_value = param.getUint32();
}

template <>
void AssignerT<int32>::assign( const NParam& param ) {
   m_value = param.getInt32();
}

template <>
void AssignerT<bool>::assign( const NParam& param ) {
   m_value = param.getBool();
}


/** Creates an assigner for a specific type.
 *
 * @return allocated assigner
 */
template <typename T>
Assigner* makeAssigner( uint32 id, T& assignThis ) {
   return new AssignerT<T>( id, assignThis );
}

/** Creates an assigner that has default value which it will use if the param
 * does not exist.
 * @return allocated assigner 
 */
template <typename T>
Assigner* makeAssigner( uint32 id, T& assignThis, const T& defaultValue ) {
   return new AssignerTDefault<T>( id, assignThis, defaultValue );
}


}

#endif 
