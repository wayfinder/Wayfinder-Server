/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DummySMSTalker.h"
#include "StringTable.h"
#include "SMSPacket.h"
#include "StringUtility.h"
#include "SMSConvUtil.h"
#include "Utility.h"

#ifdef DEBUG_SMS_SERVER
static const char* fileName = "dummysmstalker.txt";
#endif


DummySMSTalker::DummySMSTalker()
{
   // We don't have to do anything
  DEBUG1(cout << "DummySMSTalker created" << endl);
}

DummySMSTalker::~DummySMSTalker()
{
   // We do not have to do anything.
}

bool DummySMSTalker::init(SMSProcessor *processor)
{
   SMSTalker::init(processor);

#ifdef DEBUG_SMS_SERVER
   // Read test strings
   FILE* data;
   
   if ( (data = fopen( fileName, "r" )) == NULL ) {
      DEBUG1(cerr << "DummySMSTalker::init can't open debugfile: " 
             << fileName << endl;);
      return false;
   }

   int length = 0;
   char buff[1024];
   length = Utility::readLine( data, buff, 1024, "\r\n" );
   while ( length != -1 ) {
      char* sep = strchr( buff, '\t' );
      if ( sep != NULL ) {
         *sep = '\0';
         m_vector.push_back( StringUtility::newStrDup( buff ) );
         m_vector.push_back( StringUtility::newStrDup( sep + 1 ) );
      } else {
         DEBUG1(cerr << "DummySMSTalker::init debugfile format error! #"
                << buff << "#" << endl;);
      }
      length = Utility::readLine( data, buff, 1024, "\r\n" );
   }
   DEBUG2(cerr << "DummySMSTalker::init read " << (m_vector.size()/2)
          << " debugSMS strings from " << fileName<< "." << endl;);
   // Start the thread
   this->start();
#endif
   return true;
}


SMSSendReplyPacket* DummySMSTalker::sendSMS(SMSSendRequestPacket* p)
{
   SMSSendReplyPacket* rp = (SMSSendReplyPacket*)p;
   // In a real SMSTalker we would send the SMSPacket here.
   // Instead we print it to the screen!
   int dataLength = 0;
   byte* data = NULL;
   char* senderPhone   = NULL;
   char* recipientPhone = NULL;
   int encodingType = CONVERSION_NO;

   p->getPacketContents(encodingType, senderPhone, recipientPhone,
                        dataLength, data);
   
   char text[ 6*dataLength + 1 ];
   SMSConvUtil::gsmToMc2( text, (char*)data, dataLength);
   text[ dataLength ] = '\0';

   cout << "DummySMSTalker got an SMSReqeustPacket: " << endl
        << "\tSenderPhone = " << senderPhone << endl
        << "\tRecipientPhone = " << recipientPhone << endl
        << "\tDataLength = " << dataLength << endl
        << "\tData = " << endl 
        << "====" << endl 
        << text << endl
        << "====" << endl;
   rp->setSubType(Packet::PACKETTYPE_SMSREPLY);
   rp->setStatus(StringTable::OK);

   delete [] data;

   return rp;
}

uint32 DummySMSTalker::checkStatus()
{
   return StringTable::OK;
}


void 
DummySMSTalker::run() {
   DEBUG1(cerr << "DummySMSTalker::run " << endl;);
#ifdef DEBUG_SMS_SERVER
   JTCThread::sleep( 10000 ); // ms Startup has to finnish
   srand( TimeUtility::getRealTime() );
   uint32 averageTime = 2000; // ms
   uint32 i = 0;
   uint32 nbr = m_vector.size();
   char* str = NULL;
   const char* senderPhone = "6666";
   char* recipientPhone = NULL;
   while ( ! terminated ) {
      JTCThread::sleep( 
         1 + (uint32)((float64)averageTime*rand()/(RAND_MAX + 1.0) ) );
      recipientPhone = m_vector[ i ];
      str = m_vector[ i + 1 ];
      DEBUG2(cerr << "DummySMSTalker::run() sending str: " << str 
             << " from: " << recipientPhone << endl;);
      SMSSendRequestPacket *srp = new SMSSendRequestPacket();
      srp->fillPacket( CONVERSION_NO,
                       recipientPhone, senderPhone,
                       strlen( str ), (byte*)str);
      m_processor->SMSReceived( srp );
      i += 2;
      if ( i == nbr ) i = 0;
   }
#endif
}
