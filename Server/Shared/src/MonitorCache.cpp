/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MonitorCache.h"


MonitorCache::MonitorCache( CacheElement::CACHE_TYPE type,
                            uint32 maxCacheSize, 
                            uint32 maxElements ) 
      : Cache( type,
               maxCacheSize, 
               maxElements )
{  }


MonitorCache::~MonitorCache() {
   ISABSync sync(*this);
}


CacheElement*
MonitorCache::find(uint32 ID, CacheElement::CACHE_ELEMENT_TYPE type) {
   ISABSync sync(*this); 
   return Cache::find(ID, type);
}

CacheElement*
MonitorCache::find(CacheElement* keyElement) {
   return find(keyElement->getID(), keyElement->getType());
}


bool
MonitorCache::exists(uint32 ID, CacheElement::CACHE_ELEMENT_TYPE type) {
   ISABSync sync(*this);
   return Cache::exists(ID, type);
}


bool
MonitorCache::exists(CacheElement* keyElement) {
   return exists(keyElement->getID(), keyElement->getType());
}


void
MonitorCache::add(CacheElement* element) {
   ISABSync sync(*this);
   Cache::add( element );
}


CacheElement*
MonitorCache::addAndLock(CacheElement* element) {
   ISABSync sync(*this);
   return Cache::addAndLock( element );
}


bool 
MonitorCache::remove(CacheElement* element) {
   ISABSync sync(*this);
   return Cache::remove( element );
}


void
MonitorCache::releaseCacheElement(CacheElement* element) {
   ISABSync sync(*this);
   Cache::releaseCacheElement( element );
}


void
MonitorCache::setMaxSize(uint32 newMax) {
   ISABSync sync(*this);
   Cache::setMaxSize(newMax);
}


uint32
MonitorCache::getCurrentSize() {
   ISABSync sync(*this);
   return Cache::getCurrentSize();
}


void
MonitorCache::setMaxElements(uint32 newMax) {
   ISABSync sync(*this);
   Cache::setMaxElements(newMax);
}


uint32
MonitorCache::getCurrentElementcount() {
   ISABSync sync(*this);
   return Cache::getCurrentElementcount();
}

uint32
MonitorCache::getCurrentRemovedSize( uint32* nbr ) const {
   ISABSync sync( *this );
   return Cache::getCurrentRemovedSize( nbr );
}

