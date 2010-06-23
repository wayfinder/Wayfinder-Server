/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TopRegionMatch.h"
#include "Name.h"
#include "NameCollection.h"
#include "LocaleUtility.h"


TopRegionMatch::TopRegionMatch( uint32 id, topRegion_t type ) 
      : m_id( id ),
        m_type( type )
{
}

TopRegionMatch::TopRegionMatch( uint32 id, 
                                topRegion_t type,
                                const ItemIDTree& idTree,
                                const MC2BoundingBox& bbox,
                                const NameCollection& names ) 
      : m_id( id ), m_type( type ), m_itemTree( idTree ), m_bbox( bbox )
{
   addNames( &names );
}

TopRegionMatch::~TopRegionMatch() {
}


uint32
TopRegionMatch::getID() const {
   return m_id;
}


TopRegionMatch::topRegion_t 
TopRegionMatch::getType() const {
   return m_type;
}


void
TopRegionMatch::addName( const char* name, 
                         LangTypes::language_t lang,
                         ItemTypes::name_t type )
{
   m_names.addName( new Name( name, lang, type ) );
}


void
TopRegionMatch::addName( const Name* name )
{
   m_names.addName( new Name( *name ) );
}


void
TopRegionMatch::addNames( const NameCollection* names )
{
   for ( uint32 i = 0; i < names->getSize(); i++ ) {
      m_names.addName( new Name( *(names->getName( i )) ) );
   }
}


const char* 
TopRegionMatch::getName( LangTypes::language_t lang, 
                         const char* notFoundValue ) const
{
   set<const Name*> names = m_names.getNames( lang );
   if ( names.empty() ) {
      return notFoundValue;
   } else {
      return (*names.begin())->getName();
   }
}

const NameCollection* 
TopRegionMatch::getNames() const
{
   return &m_names;
}


void
TopRegionMatch::setBoundingBox( const MC2BoundingBox& bbox ) {
   m_bbox = bbox;
}


const MC2BoundingBox& 
TopRegionMatch::getBoundingBox() const {
   return m_bbox;
}

topRegionMatchCompareLess::
topRegionMatchCompareLess( LangTypes::language_t language ):
   m_compare( new LocaleCompareLess( language ) ),
   m_language( language )
{}

topRegionMatchCompareLess::~topRegionMatchCompareLess() {
   
}

topRegionMatchCompareLess::
topRegionMatchCompareLess( const topRegionMatchCompareLess& o ):
   m_compare( new LocaleCompareLess( o.m_language ) ),
   m_language( o.m_language )
{
   
}

const topRegionMatchCompareLess& 
topRegionMatchCompareLess::operator = ( const topRegionMatchCompareLess& o )
{
   if ( &o != this ) {
      m_compare.reset( new LocaleCompareLess( o.m_language ) );
      m_language = o.m_language;
   }

   return *this;
}

bool
topRegionMatchCompareLess::operator() ( const TopRegionMatch* a, 
                                        const TopRegionMatch* b ) const
{
   return (*m_compare)( a->getNames()->getBestName( m_language )->getName(), 
                        b->getNames()->getBestName( m_language )->getName() );
}
