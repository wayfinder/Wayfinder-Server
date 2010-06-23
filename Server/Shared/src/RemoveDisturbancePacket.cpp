/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RemoveDisturbancePacket.h"
#include <numeric>
#include "STLUtility.h"
#define REMOVE_DISTURBANCE_REPLY_SIZE REPLY_HEADER_SIZE

// Request packet
namespace {
   uint32
   requestSize(const std::vector<MC2String>& tobekept = 
                  std::vector<MC2String>(), 
               const MC2String& situationReference = "",
               const MC2String& supplier = "")
   {
      MC2String::size_type size = 4; //disturbanceID
      size += 4; // tobekept.size()
      //the tobekept strings
      size += tobekept.size(); //'\0' for each string
      size += std::accumulate(tobekept.begin(), tobekept.end(), size_t(0), 
                              STLUtility::accumulate_cb( &MC2String::size ) );
      size += situationReference.size() + 1;
      size += supplier.size() + 1;
      size += 1; //removeall
      return size;
   }
}  

RemoveDisturbanceRequestPacket::
RemoveDisturbanceRequestPacket(uint32 disturbanceID) :
   RequestPacket(REQUEST_HEADER_SIZE + requestSize(),
                 DEFAULT_PACKET_PRIO,
                 PACKETTYPE_REMOVEDISTURBANCEREQUEST,
                 0, 0, MAX_UINT32)
{
   encodeRequest(disturbanceID, "", "", false, vector<MC2String>());
}

RemoveDisturbanceRequestPacket::
RemoveDisturbanceRequestPacket(const MC2String& situationReference) :
   RequestPacket(REQUEST_HEADER_SIZE + requestSize(std::vector<MC2String>(),
                                                   situationReference),
                 DEFAULT_PACKET_PRIO,
                 PACKETTYPE_REMOVEDISTURBANCEREQUEST,
                 0, 0, MAX_UINT32)
{
   encodeRequest(MAX_UINT32, situationReference, 
                 "", false, vector<MC2String>());
}

RemoveDisturbanceRequestPacket::
RemoveDisturbanceRequestPacket(const MC2String& supplier,
                               const vector<MC2String>& toBeKept) :
   RequestPacket(REQUEST_HEADER_SIZE + requestSize(toBeKept, "", supplier),
                 DEFAULT_PACKET_PRIO,
                 PACKETTYPE_REMOVEDISTURBANCEREQUEST,
                 0, 0, MAX_UINT32)
{
   encodeRequest(MAX_UINT32, "", supplier, true, toBeKept);
}

void
RemoveDisturbanceRequestPacket::encodeRequest(
                                      uint32 disturbanceID,
                                      MC2String situationReference,
                                      MC2String supplier,
                                      bool removeAll,
                                      vector<MC2String> toBeKept)
{
   int position = REQUEST_HEADER_SIZE;
   incWriteLong(position, disturbanceID);
   incWriteLong(position, toBeKept.size());
   for(uint32 i = 0; i < toBeKept.size(); i++) {
      incWriteString(position, toBeKept[i].c_str());
   }
   incWriteString(position, situationReference.c_str());
   incWriteString(position, supplier.c_str());
   if( removeAll ) {
      byte b = 0x01;
      incWriteByte(position, b);
   } else {
      byte b = 0x00;
      incWriteByte(position, b);
   }
   setLength(position);
}

void
RemoveDisturbanceRequestPacket::decodeRequest(
                                    uint32 &disturbanceID,
                                    MC2String &situationReference,
                                    MC2String &supplier,
                                    bool &removeAll,
                                    vector<MC2String> &toBeKept) const
{
   int position = REQUEST_HEADER_SIZE;
   disturbanceID = incReadLong(position);
   uint32 nbrStrings = incReadLong(position);
   char* temp;
   for(uint32 i = 0; i < nbrStrings; i++) {
      incReadString(position, temp);
      toBeKept.push_back(MC2String(temp));
   }
   incReadString(position, temp);
   situationReference = temp;
   incReadString(position, temp);
   supplier = temp;
   byte b = incReadByte(position);
   if( b == 0 ) {
      removeAll = false;
   } else {
      removeAll = true;
   }
}

DisturbanceBBox::DisturbanceBBox(uint32 disturbanceID, 
                                 const MC2String& situationReference,
                                 const MC2BoundingBox& bbox) : 
   m_disturbanceID(disturbanceID),
   m_situationReference(situationReference),
   m_bbox(bbox)
{}

DisturbanceBBox::DisturbanceBBox() : 
   m_disturbanceID(MAX_UINT32)
{}

size_t DisturbanceBBox::size() const
{
   return 4 +                           // m_disturbanceID
      AlignUtility::alignToLong(m_situationReference.length() + 1) // m_situationReference
      + 4*4;                            // m_bbox
}

void DisturbanceBBox::save(Packet* packet, int& pos) const
{
   packet->incWriteLong( pos, m_disturbanceID );
   packet->incWriteBBox( pos, m_bbox );
   packet->incWriteString( pos, m_situationReference );
}

void DisturbanceBBox::load(const Packet* packet, int& pos)
{
   m_disturbanceID = packet->incReadLong( pos );
   packet->incReadBBox( pos, m_bbox );
   packet->incReadString( pos, m_situationReference );
}

const MC2BoundingBox& DisturbanceBBox::getBBox() const
{
   return m_bbox;
}

const MC2String& DisturbanceBBox::getSituationReference() const
{
   return m_situationReference;
}

uint32 DisturbanceBBox::getDisturbanceID() const
{
   return m_disturbanceID;
}

ostream& operator<<(ostream& strm, const DisturbanceBBox& bbox)
{
   return strm << "(" << bbox.getDisturbanceID() << "," 
               << bbox.getSituationReference() << "," << bbox.getBBox() << ")";
}


// Reply packet
namespace {
   size_t replySize(const DisturbanceBBox* first, const DisturbanceBBox* last)
   {
      const size_t size = 
         accumulate(first, last, 0, 
                    STLUtility::accumulate_cb(&DisturbanceBBox::size));
      return (REPLY_HEADER_SIZE + //header
              4 +                 //length
              4 +                 //count
              size);              //data
   }
}

RemoveDisturbanceReplyPacket::RemoveDisturbanceReplyPacket(
   const RequestPacket* p,
   uint32 status) :
   ReplyPacket(replySize(0,0),
               PACKETTYPE_REMOVEDISTURBANCEREPLY,
               p,
               status)
{
   const DisturbanceBBox* b = NULL;
   addBoxes(b,b);
}

RemoveDisturbanceReplyPacket::RemoveDisturbanceReplyPacket(const RequestPacket* p,
                                                           uint32 status,
                                                           const DisturbanceBBox* boxes, 
                                                           int boxcount) :
   ReplyPacket(replySize(boxes, boxes + boxcount),
               PACKETTYPE_REMOVEDISTURBANCEREPLY, 
               p, status)
{
   addBoxes(boxes, boxes + boxcount);
}

RemoveDisturbanceReplyPacket::~RemoveDisturbanceReplyPacket()
{
}

int RemoveDisturbanceReplyPacket::count() const
{
   int pos = REQUEST_HEADER_SIZE + 4; //skip header and length
   const int count = incReadLong( pos );
   return count;
}
