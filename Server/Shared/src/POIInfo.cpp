/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "POIInfo.h"

#include "DataBuffer.h"
#include "DeleteHelpers.h"

#include <memory>

POIInfo::POIInfo():
   m_staticID( 0 ),
   m_dynamicInfo( false ) {

}

POIInfo::POIInfo( uint32 staticID, bool dynamicInfo,
                  uint32 poiInfoSize ):
   m_staticID( staticID ),
   m_dynamicInfo( dynamicInfo ) {
   m_poiInfos.reserve( poiInfoSize );
}

POIInfo::POIInfo( const POIInfo& copyThis,
                  const POIInfo::DataTypeSet& keyTypes ):
   m_staticID( copyThis.m_staticID ),
   m_dynamicInfo( copyThis.m_dynamicInfo ) 
{

   if ( keyTypes.empty() ) {
      // copy all data
      for ( uint32 i = 0; i < copyThis.getInfos().size(); ++i ) {
         m_poiInfos.push_back( new POIInfoData( *copyThis.getInfos()[ i ] ) );
      }
   } else { // only copy those keys the client are interested in
      for ( uint32 i = 0; i < copyThis.getInfos().size(); ++i ) {
         if ( keyTypes.find( copyThis.getInfos()[ i ]->getType() ) != 
              keyTypes.end() ) {
            m_poiInfos.
               push_back( new POIInfoData( *copyThis.getInfos()[ i ] ) );
         }
      }
   }
}

POIInfo::~POIInfo() {
   STLUtility::deleteValues( m_poiInfos );
}


void POIInfo::load( DataBuffer& buff ) {
   mc2dbg8 << "[POIInfo] loading." << endl;
   buff.alignToLong();
   m_staticID = buff.readNextLong();
   m_dynamicInfo = buff.readNextLong();
   uint32 nbrPOIInfos = buff.readNextLong();

   mc2dbg8 << "static id: " << m_staticID << endl;
   mc2dbg8 << "dynamicInfo: " << m_dynamicInfo << endl;
   mc2dbg8 << "[POIInfo] #" << nbrPOIInfos 
           << " poi infos." << endl;

   STLUtility::deleteValues( m_poiInfos );

   m_poiInfos.resize( nbrPOIInfos );

   // read infos
   for ( uint32 i = 0; i < m_poiInfos.size(); ++i ) {
      buff.alignToLong();
      uint32 type = buff.readNextLong();
      LangTypes::language_t lang = 
         static_cast<LangTypes::language_t>( buff.readNextLong() );

      MC2String str = buff.readNextString();

      m_poiInfos[ i ] = new POIInfoData( type, lang, str );
   }

}

bool POIInfo::load( const MC2String& filename, uint32 offset ) {

   DataBuffer fileBuff;
   fileBuff.memMapFile( filename.c_str() );

   DataBuffer offsetBuff( fileBuff.getBufferAddress() + offset,
                          fileBuff.getBufferSize() - offset );
   load( offsetBuff );

   return true;
}
namespace {
struct TypeIDFind {
   bool operator()( POIInfoData::type_t type, POIInfoData* other ) {
      return type < other->getType();
   }
   bool operator()( POIInfoData* other, POIInfoData::type_t type ) {
      return other->getType() < type;
   }
};
}


POIInfo::InfoArray POIInfo::findData( POIInfoData::type_t type ) const {
   if ( m_poiInfos.empty() ) {
      return InfoArray( NULL, 0 );
   }

   // now, since the data must be ordered before it is saved to disc, 
   // we can do this
   pair< InfoDataVector::const_iterator, 
      InfoDataVector::const_iterator > info = 
      equal_range( m_poiInfos.begin(), m_poiInfos.end(),
                   type, TypeIDFind() );

   return InfoArray( (*info.first), distance( info.first, info.second ) );
}

ostream& operator << ( ostream& ostr, const POIInfo& info ) {
   ostr << "-- POIInfo --" << endl
        << " static ID = " << info.getStaticID() << endl
        << " has dynamic info = " << info.hasDynamicInfo() << endl
        << " nbr infos = " << info.getInfos().size() << endl;
   for ( uint32 i = 0; i < info.getInfos().size(); ++i ) {
      ostr << *info.getInfos()[ i ];
   }

   return ostr;
}

ostream& operator << ( ostream& ostr, const POIInfoData& info ) {
   ostr << info.getType() << " : " << info.getLang() 
        << " : " << info.getInfo() << endl;
   return ostr;
}
