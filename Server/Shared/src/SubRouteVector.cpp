/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "SubRouteVector.h"
#include "SubRoute.h"

SubRouteVector::SubRouteVector( const SubRouteVector& other )
{
   reserve( other.size() );
   // Loop through all the SubRoutes 
   for ( const_iterator it = other.begin() ;
         it != other.end() ;
         ++it ) {
      if ( *it == NULL ) {
         push_back(NULL);
      } else {
         push_back(new SubRoute(**it));
      }
   }
   m_originalDestIndex = other.m_originalDestIndex;
}

SubRouteVector::~SubRouteVector()
{
   // Delete the SubRoutes
   for ( iterator it = begin() ; it != end() ; ++it ) {
      delete *it;
   }
}

void
SubRouteVector::swap( SubRouteVector& other )
{
   vector<SubRoute*>::swap( other );
   std::swap( m_originalDestIndex, other.m_originalDestIndex );
}

SubRoute*
SubRouteVector::getSubRouteAt(uint32 index) const
{
   return (*this)[index];
}

void
SubRouteVector::resetIndex(uint32 index)
{
   (*this)[index] = NULL;
}

void
SubRouteVector::resetAll( )
{
   uint32 index;
   for (index = 0; index < (*this).size(); index++ ) {
      (*this)[index] = NULL;
   }
}

void
SubRouteVector::insertSubRoute(SubRoute* pSubRoute)
{
   push_back(pSubRoute);
}

bool
SubRouteVector::compareCosts(SubRouteVector* a,
                             SubRouteVector* b)
{
   MC2_ASSERT( a != NULL );
   MC2_ASSERT( b != NULL );
   MC2_ASSERT( a->back() != NULL );
   MC2_ASSERT( b->back() != NULL );
   return a->back()->getCost() < b->back()->getCost();
}



