/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapConfig.h"
#include "TileFeature.h"
#include "TileMapFormatDesc.h"
#include "TileMapException.h"

// ------------------------------- TilePrimitiveFeature -------------------

// ------------------------------- TileFeature ----------------------------

TileFeature::TileFeature(int32 type) : TilePrimitiveFeature(type)
{
   m_pixelBox     = NULL;
   m_screenCoords = NULL;
}

TileFeature::~TileFeature() 
{
   for ( vector<TileFeatureArg*>::iterator it = m_args.begin();
         it != m_args.end(); ++it ) {
      delete *it;
   } 
//     delete m_pixelBox;
//     delete m_screenCoords;
//     m_pixelBox = NULL;
//     m_screenCoords = NULL;
}

// This copy constructor may be used in vector.
TileFeature::TileFeature(const TileFeature& other)
      : TilePrimitiveFeature(other)
      
{
   m_pixelBox = other.m_pixelBox;
   m_screenCoords = other.m_screenCoords;
#ifndef MC2_SYSTEM
      m_args.clear();
#endif
}

const TileFeature&
TileFeature::operator=(const TileFeature& other)
{
   if ( this != &other ) {
      m_pixelBox     = other.m_pixelBox;
      m_screenCoords = other.m_screenCoords;
#ifndef MC2_SYSTEM
      m_args.clear();
#else
      m_args = other.m_args;
      m_type = other.m_type;
#endif
   }
   return *this;
}

#ifdef MC2_SYSTEM
bool 
TileFeature::save( BitBuffer& buf, const TileMap& tileMap,
                   const TileFeature* prevFeature ) const 
{
   bool sameType = false;
   if ( ( prevFeature == NULL ) ||
        ( getType() != prevFeature->getType() ) ) {
      // Not same as previous.
      buf.writeNextBits( 0, 1 );
      // Write the type.
      buf.writeNextBits( m_type, 8 );
      prevFeature = NULL;
      mc2dbg8 << "TileFeature::save Explicitly saving the type" << endl;
   } else {
      sameType = true;
      // Same as previous.
      buf.writeNextBits( 1, 1 );
      mc2dbg8 << "TileFeature::save Implicitly saving the type" << endl;
   }
   
   if ( sameType ) {
      // Submit the previous feature into the save method for the 
      // arguments.
      vector<TileFeatureArg*>::const_iterator prevIt = 
         prevFeature->m_args.begin();
      for ( vector<TileFeatureArg*>::const_iterator it = 
            m_args.begin(); it != m_args.end(); ++it ) {
         (*it)->save( buf, tileMap, *prevIt );
         ++prevIt;
      }
   } else {
      // No previous feature of same type.
      for ( vector<TileFeatureArg*>::const_iterator it = 
            m_args.begin(); it != m_args.end(); ++it ) {
         (*it)->save( buf, tileMap, NULL );
      }
   }
   return true;
}

void 
TileFeature::dump( ostream& stream ) const {
   stream << "Feature type " << (int) m_type << endl;
   for ( vector<TileFeatureArg*>::const_iterator it = 
         m_args.begin(); it != m_args.end(); ++it ) {
      (*it)->dump( stream );
   }
}

#endif

bool 
TilePrimitiveFeature::internalLoad( BitBuffer& buf, TileMap& tileMap,
                                    const TilePrimitiveFeature* prevFeature ) 
{
   if ( prevFeature != NULL ) {
      // Use previous arguments
      vector<TileFeatureArg*>::const_iterator prevIt = 
         prevFeature->m_args.begin();
      for ( vector<TileFeatureArg*>::const_iterator it = 
            m_args.begin(); it != m_args.end(); ++it ) {
         (*it)->load( buf, tileMap, *prevIt );
         ++prevIt;
      }
   } else {
      // Don't use previous arguments.
      for ( vector<TileFeatureArg*>::const_iterator it = 
            m_args.begin(); it != m_args.end(); ++it ) {
         (*it)->load( buf, tileMap, NULL );
      }
   }
   return true;
}

void
TilePrimitiveFeature::createFromStream( TilePrimitiveFeature& target,
                                        BitBuffer& buf,
                                        const TileMapFormatDesc& desc,
                                        TileMap& tileMap,
                                        const TilePrimitiveFeature* prevFeature )
{
   MC2_ASSERT( target.m_args.empty() );
   // Create TileFeature of correct dynamic type.
   bool sameAsPrevious = buf.readNextBits( 1 ) != 0;
   int16 type;
   if ( sameAsPrevious ) {
      if ( prevFeature == NULL ) {
         throw TileMapException::
            BadTileMap("Missing previous feature while loading TileFeature.");
      }
      // Same as previous.
      type = prevFeature->getType();
   } else {
      // Not same as previous.
      // Read the actual type.
      type = int16( buf.readNextSignedBits( 8 ) ); 
   }
   target.setType( type );

   desc.getArgsForFeatureType( type, target.m_args );
   // Load the feature with internalLoad. 
   // Supply the previous feature if it is of the same type.
   target.internalLoad( buf, tileMap, 
                        sameAsPrevious ? prevFeature : NULL  );
}



