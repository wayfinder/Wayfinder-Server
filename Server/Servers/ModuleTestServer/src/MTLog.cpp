/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MTLog.h"

MTLogLink::MTLogLink(uint32 packID, char* reqStr)
{
   packetID = packID;
   request = reqStr;
   reply = NULL;
}

MTLogLink::MTLogLink(uint32 packID, char* reqStr, MTLogHead* head)
{
   packetID = packID;
   request = reqStr;
   reply = NULL;
   this->into(head);
}


MTLogLink::~MTLogLink()
{
   delete [] request;
   delete [] reply;
}


void MTLogLink::print(FILE* file, bool compactOutput)
{
   char tmpStr[24];
   sprintf(tmpStr, "====================\n");
   if ( compactOutput ) tabbify(tmpStr);
   fputs(tmpStr, file);
   sprintf(tmpStr, "PacketID = %u\n", packetID);
   if ( compactOutput ) tabbify(tmpStr);
   fputs(tmpStr, file);
   if ( compactOutput ) tabbify(request);
   fputs(request, file);
   if (reply != NULL) {
      if ( compactOutput ) tabbify(reply);
      fputs(reply, file);
      if ( compactOutput ) fputs("\n", file);
   } else {
      sprintf(tmpStr, "No answer received!\n");
      if ( compactOutput ) tabbify(tmpStr);
      fputs(tmpStr, file);
      if ( compactOutput ) fputs("\n", file);
   }
}


void inline
MTLogLink::tabbify(char* str) {
   int i = 0;
   while ( str[i] != '\0' ) {
      if ( str[i] == '\n' )
         str[i] = '\t';
      i++;
   }
}


MTLog::MTLog(const char* filename, bool compactOutput)
{
   logfile = fopen(filename, "w");
   list = new MTLogHead();
   m_receivedBufferSize = 1000;
   m_compactOutput = compactOutput;
}


MTLog::~MTLog()
{
   flush();
   fclose(logfile);
}


void MTLog::putRequest(uint16 packID, char* str)
{
   ISABSync sync(*this);
   DEBUG8(cerr << "Putting request " << packID << endl);
   #ifdef __SVR4
      MTLogLink* foo = new MTLogLink(packID, str);
      foo->into(list);
   #else
      new MTLogLink(packID, str, list);
   #endif
}


void MTLog::putReply(uint16 packID, char* str)
{
   ISABSync* sync = new ISABSync(*this);
   MTLogLink* link = static_cast<MTLogLink*>(list->first());
   bool found = false;

   while ((link != NULL) && (!found)) {
      if (link->getPackID() == packID)
         found = true;
      else
         link = static_cast<MTLogLink*>(link->suc());
   }

   if (found) {
      link->setReply(str);

      while ((list->cardinal() > (int32)m_receivedBufferSize) ||
             ((link != NULL) &&
              link->answered()) ) {
         link->print(logfile, m_compactOutput);
         link->out();
         delete link;
         link = static_cast<MTLogLink*>(list->first());
      }
   } else {
      //PANIC("RECEIVED REPLY TO UNKNOWN REQUEST!!!", "");
      cerr << "RECEIVED REPLY TO UNKNOWN REQUEST!!!" 
           << " perhaps I droped it earlier id=" << packID << endl;
   }
   delete sync;
}


void MTLog::flush()
{
   ISABSync sync(*this);
   DEBUG8(cerr << "Flushing MTLinks starts" << endl;);
   MTLogLink* link = static_cast<MTLogLink*>(list->first());
   while (link != NULL) {
      DEBUG8(cerr << "About to print link" << endl;);
      link->print(logfile, m_compactOutput);
      DEBUG8(cerr << "About to take out link" << endl;);
      link->out();
      DEBUG8(cerr << "About to delete link" << endl;);
      delete link;
      if ( list->first() == NULL ) {
         DEBUG8(cerr << "List ends" << endl;);
      }
      link = static_cast<MTLogLink*>(list->first());
   }
   DEBUG8(cerr << "Flushing logfile" << endl;);
   fflush(logfile);
}


uint32 MTLog::getNbrWaiting() 
{
   ISABSync sync(*this);
   return list->cardinal();
}


void
MTLog::setReceiveBufferSize(uint32 size) {
   ISABSync sync(*this);
   if ( size > 0 )
      m_receivedBufferSize = size;
   else
      m_receivedBufferSize = 1;
}


uint32
MTLog::getReceiveBufferSize() {
   ISABSync sync(*this);
   return m_receivedBufferSize;
}

