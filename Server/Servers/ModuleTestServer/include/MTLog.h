/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MTLOG_H
#define MTLOG_H

#include <stdio.h>
#include "CCSet.h"
#include "ISABThread.h"

class MTLogHead : public Head
{
};


class MTLogLink : public Link
{
   public:

       /**
         *  Creates, initializes and adds a link to a list
         */
      MTLogLink(uint32 packID, char* reqStr, MTLogHead* head);

      MTLogLink(uint32 packID, char* reqStr);


       /**
         *  Frees the strings
         */
      virtual ~MTLogLink();

      uint32 getPackID()
      {
         return packetID;
      }

      bool answered()
      {
         return reply != NULL;
      }

      void setReply(char *str)
      {
         reply = str;
      }

      void print(FILE* file, bool compactOutput);

   private:
      void tabbify(char* str);

      uint32 packetID;
      char* request;
      char* reply;
};


 /**
   *  This class is used for logging the requests and replys in the
   *  module test server.
   *  MultiThread safe.
   */
class MTLog : public ISABMonitor 
{
   public:

       /**
         *  Creates a MTLog object.
         *  @param   filename The name of the file to log to.
         *  @param   compactOutput is if the output of a packet should be 
         *           one line.
         */
      MTLog(const char* filename, bool compactOutput);


       /**
         *  Defaul destructor.
         */
      virtual ~MTLog();


       /**
         *  Add a request to the list. The packID must come in
         *  consecutive order.
         *  Monitor procedure.
         */
      void putRequest(uint16 packID, char* str);


       /**
         *  Add a reply to the list. Prints the answer to file if
         *  possible and/or neccesary.
         *  The ownership of the string is transfered to <this>.
         *  Monitor procedure.
         */
      void putReply(uint16 packID, char* str);


       /**
         *  Prints all requests and answer to the file and flushes
         *  the file cache,
         *  Monitor procedure.
         */
      void flush();

      
      /**
       * Returns the number of outstanding packets.
       *  Monitor procedure.
       */
      uint32 getNbrWaiting();


      /**
       * Sets the maximum amount of outstanding packets.
       * @param size is the number of outstanding packets.
       * Monitor procedure.
       */
      void setReceiveBufferSize(uint32 size);


      /**
       * Returns the recievebuffersize.
       * @return the amout of outstandingpackets.
       * Monitor function.
       */
      uint32 getReceiveBufferSize();
   private:

      FILE* logfile;

      MTLogHead* list;

      uint32 m_receivedBufferSize;

      bool m_compactOutput;
};

#endif //MTLOG_H
