/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TestPacket.h"

TestRequestPacket::TestRequestPacket(uint32 mapID,uint32 time,
                                     uint32 ip, uint16 port)
   : RequestPacket(REQUEST_HEADER_SIZE+32+1024,
                   TEST_REQUEST_PRIO, 
                   PACKETTYPE_TESTREQUEST, 
                   0,
                   0, 
                   mapID)
{
   setOriginIP( ip );
   setOriginPort( port );
   int pos = REQUEST_HEADER_SIZE;
   incWriteLong(pos, 1111111   );
   incWriteLong(pos, time);
   setLength(pos+300);
}

TestRequestPacket::~TestRequestPacket()
{

}

void
TestRequestPacket::setTime(uint32 time)
{
//     int pos = getLength();
//     incWriteLong(pos, time);
//     setLength(pos);
}

uint32
TestRequestPacket::getTime(timetype_t type) const
{
   if(int(type) < 3)
      return readLong(REQUEST_HEADER_SIZE+4 + 4* uint32(type));
   else
   {
      if(getLength() == REQUEST_HEADER_SIZE+28)
      {
         
         return readLong(REQUEST_HEADER_SIZE+4+ 4*uint32(type));
      }
      else
      {
         if(int(type) < 5)
            return MAX_UINT32;
         else
            return readLong(REQUEST_HEADER_SIZE+4+ 4*uint32(type)-8);
      }
   }
}

void
TestRequestPacket::setModuleNbr(uint32 moduleNbr)
{
   writeLong(REQUEST_HEADER_SIZE, moduleNbr);
}


uint32
TestRequestPacket::getModuleNbr() const
{
   return readLong(REQUEST_HEADER_SIZE); 
}

// --------------------------- TestReplyPacket ---------------------


TestReplyPacket::TestReplyPacket( const TestRequestPacket& p, uint32 status,
                                 uint32 replyTime)
   : ReplyPacket( REPLY_HEADER_SIZE+44,
                  PACKETTYPE_TESTREPLY, 
                  &p,
                  status )
{
   int pos = REPLY_HEADER_SIZE;
   // Do nothing. Prevent crash
   setLength( pos );
   incWriteLong(pos, p.getModuleNbr());
   incWriteLong(pos, p.getMapID());
   incWriteLong(pos, p.getTime(TestRequestPacket::timetype_t(0)));
   incWriteLong(pos, p.getTime(TestRequestPacket::timetype_t(1)));
   incWriteLong(pos, p.getTime(TestRequestPacket::timetype_t(2)));

   if (p.getLength() == REQUEST_HEADER_SIZE+32) {
      incWriteLong(pos, p.getTime(TestRequestPacket::timetype_t(3)));
      incWriteLong(pos, p.getTime(TestRequestPacket::timetype_t(4)));
   }

   incWriteLong(pos, p.getTime(TestRequestPacket::timetype_t(5)));
   incWriteLong(pos, replyTime);
   setLength(pos);
}

TestReplyPacket::~TestReplyPacket()
{

}


uint32
TestReplyPacket::getTotalTime(uint32 currentTime)
{
   return(currentTime - readLong(REPLY_HEADER_SIZE+8));
}

uint32
TestReplyPacket::getProcessTime()
{
   if(getLength() == REPLY_HEADER_SIZE+36)
      return(readLong(REPLY_HEADER_SIZE+32)- readLong(REPLY_HEADER_SIZE+28));
   else
      return(readLong(REPLY_HEADER_SIZE+24)- readLong(REPLY_HEADER_SIZE+20));
}

uint32
TestReplyPacket::getReaderTime()
{
   if(getLength() != REPLY_HEADER_SIZE+36)
   {
      return 0;
   }
   
   else
      return(readLong(REPLY_HEADER_SIZE+24)- readLong(REPLY_HEADER_SIZE+20));
}

uint32
TestReplyPacket::getModuleTime()
{
   if(getLength() == REPLY_HEADER_SIZE+36)
      return (readLong(REPLY_HEADER_SIZE+32)- readLong(REPLY_HEADER_SIZE+20));
   else
      return(readLong(REPLY_HEADER_SIZE+24) - readLong(REPLY_HEADER_SIZE+16));
}


uint32
TestReplyPacket::getLeaderTime()
{
   return(readLong(REPLY_HEADER_SIZE+16)- readLong(REPLY_HEADER_SIZE+12));
}

uint32
TestReplyPacket::getNetworkTime(uint32 currentTime)
{
   return(getTotalTime(currentTime) - getLeaderTime() - getModuleTime());
}

uint32
TestReplyPacket::getModuleNbr()
{
   return(readLong(REPLY_HEADER_SIZE));
}

uint32
TestReplyPacket::getMapID()
{
   return(readLong(REPLY_HEADER_SIZE+4));
}

void
TestReplyPacket::dumpPacket()
{
   int pos = REPLY_HEADER_SIZE;
   while(pos < int(getLength()))
   {
      cerr << pos << " : " << incReadLong(pos)<< endl;
   }
}

TestPushPacket::TestPushPacket(uint32 mapID,
                               uint32 serviceID,
                               uint32 timeStamp)
      : PushPacket( MAX_PACKET_SIZE,
                    Packet::PACKETTYPE_TEST_PUSH,
                    mapID,
                    serviceID,
                    timeStamp)
{
}
   

