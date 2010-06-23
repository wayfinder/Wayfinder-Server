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
#include "NavSession.h"


NavSession::NavSession()
{
   //m_data = NULL;
/*    m_sizeOfData = 0; */

   m_callDone = 0;

   // This variable should be set by NavThread when it gets 
   // a parameter from the client, telling how large buffer 
   // it can receive.
   // It is set as to a default value in clearSessionState.
   m_maxLengthToSendToClient = MAX_UINT16;
}

   

NavSession::~NavSession()
{
   clearSessionState();
}

   
bool
NavSession::connected()
{
   return true;
}


void
NavSession::clearSessionState()
{
   mc2dbg2 << "Clearing session state" << endl;

   // This variable should be set by NavThread when it gets 
   // a parameter from the client, telling how large buffer 
   // it can receive. 
   m_maxLengthToSendToClient = 14000;

   clearData();
   //   delete m_data;
   //   m_data = NULL;

/*    m_sizeOfData = 0; */
   m_callDone = 0;
}

   
void 
NavSession::setData( byte* data, uint32 size )
{
   if (size > m_maxLengthToSendToClient){
      mc2log << warn << here
             << " The size of added data is larger than the"
             << " max length of data possible to send to the"
             << " client. Maybe client forgot to set it, trying to send it"
             << " anyway." << endl;
   }
   clearData();
   m_dataBytePtrs.insert( 
      make_pair( static_cast<int32>(size), data ) );
}


void 
NavSession::addData( byte* data, uint32 size )
{
   if (size > m_maxLengthToSendToClient){
      mc2log << error << here
             << " The size of added data is larger than the"
             << " max length of data possible to send to the"
             << " client, size " << size << " max " 
             << m_maxLengthToSendToClient << endl;
   }
   int32 currentDataSize = getSizeOfData();
   m_dataBytePtrs.insert(
      make_pair( static_cast<int32>(currentDataSize + size), data ) );
}

void 
NavSession::clearData()
{
   map<int32, byte*>::const_iterator it = m_dataBytePtrs.begin();
   while( it != m_dataBytePtrs.end() ){
      delete [] it->second;
      it++;
   }
   m_dataBytePtrs.clear();
}

pair<int32, byte*> 
NavSession::getData(int32 offset) const
{
   // Find the first element in the vector, to return.
   map<int32, byte*>::const_iterator it = m_dataBytePtrs.begin();
   int32 offsetForPriorBuffer = 0;
   while (  ( it != m_dataBytePtrs.end() ) &&
            ( it->first <= offset )   )
   {
      offsetForPriorBuffer = it->first;
      it++;
   }
   
   // If there is some data after the offset, return it.
   byte* data = NULL;
   int32 sizeOfData = MAX_INT32;
   if ( it != m_dataBytePtrs.end() ){
      data = it->second;
      sizeOfData = it->first - offsetForPriorBuffer;

      mc2dbg8 << here << " getData() returns data." << endl;
   } else {
      data = NULL;
      sizeOfData = MAX_INT32;

      mc2dbg2 << here
             << " Offset set to high or no data in session."
             << " offset =" << offset << endl;
   }
   return make_pair( sizeOfData, data );
}

int32
NavSession::getSizeOfData() const
{
   int32 currentDataSize = 0;

   if (m_dataBytePtrs.size() != 0){
      map<int32, byte*>::const_iterator it = --m_dataBytePtrs.end();
      currentDataSize = it->first;
   } else {
      currentDataSize = 0;
   }

   return (currentDataSize);
}

uint32 
NavSession::getMinBufferLength() const
{
   return m_minLengthToSendToClient;
}

uint32 
NavSession::getMaxBufferLength() const
{
   return m_maxLengthToSendToClient;
}

void
NavSession::setMaxBufferLength( uint32 val ) {
   m_maxLengthToSendToClient = val;
}


void 
NavSession::dump( ostream& out ) {
   out << "NavSession " << endl
       << " m_callDone " << m_callDone << endl
       << " m_maxLengthToSendToClient " << m_maxLengthToSendToClient 
       << endl
       << " m_downloadAdditionalRouteData " 
       << m_downloadAdditionalRouteData << endl
       << " m_dataBytePtrs.size() " << m_dataBytePtrs.size() << endl
       << " getSizeOfData() " << getSizeOfData() << endl;
}
