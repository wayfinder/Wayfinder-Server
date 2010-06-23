/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BBOX_PARKCGH_ET_H
#define BBOX_PARKCGH_ET_H

#include "config.h"
#include "Packet.h"
#include <vector>

/**
 *   Contains data for the BBoxRequestPacket
 */
class BBoxReqPacketData {
public:
   /**
    *   @param bbox      The bounding box to examine.
    *   @param underview If underview maps should be included.
    *   @param country   If country maps should be included.
    *   @param overview  If overview maps should be included.
    */
   BBoxReqPacketData( const MC2BoundingBox& bbox,
                      bool underview,
                      bool country );

   /// Creates uninitialized data for later load
   BBoxReqPacketData() {}

   /// Returns the bounding box
   const MC2BoundingBox& getBBox() const { return m_bbox; }
   /// True if underview maps should be included
   bool underview() const { return m_underview; }
   /// True if country maps should be included.
   bool country() const { return m_country; }
   /// Always false
   bool overview() const { return m_overview; }
   
   
   /// Saves the data in the packet.
   void save( Packet* p, int& pos ) const;
   /// Loads the data from the packet.
   void load( const Packet* p, int& pos );
   /// Returns the size of the data when saved in a packet
   int getSizeInPacket() const;
private:
   /// Bounding box
   MC2BoundingBox m_bbox;
   /// Should include underview maps
   bool m_underview;
   /// Should include country maps
   bool m_country;
   /// Should include overview maps.
   bool m_overview;
};

ostream& operator<<( ostream& o, const BBoxReqPacketData& data);

/**
 *   Packet to use instead of the custom ones
 *   for requesting the maps covered by bounding
 *   boxes.
 */
class BBoxRequestPacket : public RequestPacket {
public:

   /**
    *   @param data The data to send.
    */
   BBoxRequestPacket( const BBoxReqPacketData& data );

   /// Returns the data of the packet.
   void get( BBoxReqPacketData& data ) const;

   
};

class BBoxReplyPacket : public ReplyPacket {
public:
   
   /**
    *    Reply to the BBoxRequestPacket
    */
   BBoxReplyPacket( const BBoxRequestPacket* req,
                    const vector<uint32>& mapIDs );

   /// Gets the data of the packet.
   void get( MC2BoundingBox& bbox,
             vector<uint32>& mapIDs ) const;
   
};

#endif
