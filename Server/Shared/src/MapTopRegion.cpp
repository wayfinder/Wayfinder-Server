/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapTopRegion.h"
#include "DataBuffer.h"
#include "MC2BoundingBoxBuffer.h"

MapTopRegion::MapTopRegion( uint32 id, TopRegionMatch::topRegion_t type )
   : TopRegionMatch( id, type )
{

}

MapTopRegion::MapTopRegion( uint32 id, 
                 topRegion_t type,
                 const ItemIDTree& idTree,
                 const MC2BoundingBox& bbox,
                 const NameCollection& names ) 
   : TopRegionMatch( id, type, idTree, bbox, names )
{

}

bool
MapTopRegion::load(DataBuffer* buf, uint32 mapSet)
{   
   int version = buf->readNextLong();
   version = version;
   // Currently only one version exists
   m_id   = buf->readNextLong();
   m_type = topRegion_t(buf->readNextLong());
   m_names.load( buf );
   MC2BoundingBoxBuffer::readNextBBox( *buf, m_bbox );
   m_itemTree.load( buf, mapSet);
   return true;   
}

bool
MapTopRegion::save(DataBuffer* buf) const
{
   const int version = 1;
   buf->writeNextLong(version);
   buf->writeNextLong( m_id );    // Write id           4 bytes 
   buf->writeNextLong( m_type );  // Write type         4 bytes    
   m_names.save( buf );           // Save the namecollection
   MC2BoundingBoxBuffer::
      writeNextBBox( *buf, m_bbox );  // Write bbox     16 bytes
   m_itemTree.save( buf );        // Save the itemtree  x bytes
   return true;
}
   
