/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UpdateDisturbancePacket.h"

UpdateDisturbanceRequestPacket::UpdateDisturbanceRequestPacket(
                                  uint16 packetID,
                                  uint32 requestID,
                                  uint32 disturbanceID,
                                  TrafficDataTypes::disturbanceType type,
                                  TrafficDataTypes::severity severity,
                                  uint32 startTime,
                                  uint32 endTime,
                                  uint32 costFactor,
                                  MC2String text)
      : RequestPacket(REQUEST_HEADER_SIZE + 24 + text.size() + 1,
                      1,
                      PACKETTYPE_UPDATEDISTURBANCEREQUEST,
                      packetID,
                      requestID,
                      MAX_UINT32)
{
   int pos = REQUEST_HEADER_SIZE;
   incWriteLong(pos, disturbanceID);
   incWriteLong(pos, type);
   incWriteLong(pos, severity);
   incWriteLong(pos, startTime);
   incWriteLong(pos, endTime);
   incWriteLong(pos, costFactor);
   incWriteString(pos, text.c_str());
   
   setLength(pos);
}


void
UpdateDisturbanceRequestPacket::getUpdateData(
                      uint32 &disturbanceID,
                      TrafficDataTypes::disturbanceType &type,
                      TrafficDataTypes::severity &severity,
                      uint32 &startTime,
                      uint32 &endTime,
                      uint32 &costFactor,
                      MC2String &text) const
{
   int pos = REQUEST_HEADER_SIZE;
   disturbanceID = uint32(incReadLong(pos));
   type = (TrafficDataTypes::disturbanceType) incReadLong(pos);
   severity = (TrafficDataTypes::severity) incReadLong(pos);
   startTime = uint32(incReadLong(pos));
   endTime = uint32(incReadLong(pos));
   costFactor = uint32(incReadLong(pos));
   char* temp;
   incReadString(pos, temp);
   text = temp;
}


UpdateDisturbanceReplyPacket::UpdateDisturbanceReplyPacket(
                                                 const RequestPacket* p,
                                                 uint32 status)
      : ReplyPacket(REPLY_HEADER_SIZE,
                    PACKETTYPE_UPDATEDISTURBANCEREPLY,
                    p,
                    status)
   
{
   
}

UpdateDisturbanceReplyPacket::~UpdateDisturbanceReplyPacket()
{
   
}
