/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BUFFERSENDER_H
#define BUFFERSENDER_H

#include "config.h"
#include "TimedSelectable.h"
#include "SimpleArray.h"
#include "SocketException.h"
#include <memory>

class IPnPort;

/**
 * Sends a buffer to a specific destination with TCP in
 * non-blocking mode.
 */
class BufferSender: public TimedSelectable  {
public:
   typedef SimpleArray<byte> BufferType;

   /// Different states this sender can be in
   enum State { 
      CONNECTING, //< still trying to connect
      READY, //< this state exist between connect and send
      SENDING, //< sending buffer
      ERROR, //< something whent wrong with send or connect
      DONE //< a buffer has been sent
   };
   /**
    * Connects to destination and goes in to DONE state
    * Can throw socket exception if connection failed
    * @param destination the destination
    */
   explicit BufferSender( const IPnPort& destination ) 
      throw (SocketException);

   virtual ~BufferSender();

   /**
    * Try to send a piece of the databuffer.
    * The state will tell if the entire databuffer was sent or not
    */
   void send();

   /**
    * Sets the buffer to send, the current buffer will be discared.
    * If the sender is in state DONE or SENDING it will go into state SENDING
    * and start sending the buffer.
    * Note however that if state isSENDING state, there could be some 
    * unsent data in the old buffer.
    * So check the state before calling setBuffer
    * @param buffer the buffer to send, the data will be swapped with the old buffer
    */
   void setBuffer( BufferType& buffer );

   /// @return address to databuffer destination
   const IPnPort& getDestination() const;
   /// tries to reconnect to destination
   void reconnect() throw (SocketException);

   /// @return current state 
   State getState() const;

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

#endif
