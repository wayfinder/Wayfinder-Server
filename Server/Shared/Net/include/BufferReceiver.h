/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BUFFERRECEIVER_H
#define BUFFERRECEIVER_H

#include "TimedSelectable.h"
#include "SimpleArray.h"
#include <memory>

class TCPSocket;
/**
 * Receives a buffer from a tcp socket and handles timeout
 */
class BufferReceiver: public TimedSelectable {
public:
   typedef SimpleArray<byte> BufferType;

   /// Different states this sender can be in
   enum State { 
      RECEIVING, //< receiving buffer
      ERROR, //< something whent wrong with send or connect
      IDLE, //< a buffer has been received and ready
      DONE //< all received
   };

   /**
    * Sets the RECEIVING state 
    * @param receiver the receiving socket
    */
   explicit BufferReceiver( TCPSocket* receiver );

   virtual ~BufferReceiver();

   /**
    * Tries to receive a piece of data.
    * The state will tell if the entire databuffer is received
    */
   void receive();

   /// @return current state 
   State getState() const;

   /** 
    * Returns a buffer if the current buffer is ready for reading.
    * The internal buffer will be cleared and 
    * state will change to receiving.
    * @param buffer the buffer to set
    */
   void getBuffer( BufferType& buffer );

   /// @return selectable
   selectable getSelectable() const;

   bool wantRead() const;
   bool wantWrite() const;
   void handleIO( bool readyRead, bool readyWrite );
   void handleTimeout();

private:
   struct Impl;   
   std::auto_ptr<Impl> m_impl; //< hidden implementation
};

#endif // BUFFERECEIVER_H
