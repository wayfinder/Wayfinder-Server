/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MULTIPLE_PACKETS_REQUEST_H
#define MULTIPLE_PACKETS_REQUEST_H

#include "Request.h"
#include "PacketContainer.h"

/**
 * Class used to send several RequestPackets to one module.
 * @see SinglePacketRequest
 *
 */
class MultiplePacketsRequest : public Request
{
   public:

      /**
       * Constructor for the request.
       *
       * @param reqID      A unique request ID.
       * @param nbrPackets The number of packets to allocate space for.
       */
      MultiplePacketsRequest(uint16 reqID, int nbrPackets);

      /**
       * Destructor that removes any contained packets not given away by
       * getNextPacket or getAnswer(int).
       */
      virtual ~MultiplePacketsRequest();

      /**
       * Add a PacketContainer to the collection of containers in this
       * request.
       *
       * @param  container The PacketContainer to add.
       * @return           True if container was added, false if index was
       *                   out of array range.
       */
      bool add(PacketContainer* container);

      /**
       * Get the next packet from the collection of packets contained in
       * this request. This PacketContainer must be deleted elsewhere.
       *
       * @return A PacketContainer containing the next packet.
       */
      PacketContainer* getNextPacket();

      /**
       * Process the data returned by a module.
       *
       * @param container A PacketContainer containing the data to process.
       */
      void processPacket(PacketContainer* container);

      /**
       * This function has an implementation only because it is defined
       * as true virtual in the base class Request.
       *
       * @return A PacketContainer* that is always NULL.
       */
      PacketContainer* getAnswer();

      /**
       * Get the specified answer from the collection of packets. NULL will
       * be returned if index is out of range. The returned PacketContainer
       * must be deleted elsewhere. This function can only be used once for
       * each position, thereafter NULL will always be returned.
       *
       * @param  i The position of the wanted packet.
       * @return   The PacketContainer containing the packet at position i.
       */
      PacketContainer* getAnswer(int i);

   private:

      /**
       * The array of the PacketContainers containing the request packets.
       */
      PacketContainer** m_requestPackets;

      /**
       * The array of the PacketContainers containing the reply packets.
       */
      PacketContainer** m_replyPackets;

      /**
       * The number of request and reply packets (the size of the arrays).
       */
      int m_nbrAllocPackets;

      /**
       * The current position to add packets in the requestPackets array.
       */
      int m_curAddRequestPacketPos;

      /**
       * The current position to get packets in the requestPackets array.
       */
      int m_curGetRequestPacketPos;

      /**
       * The number of packets in m_replyPackets.
       */
      int m_nbrProcessedPackets;

}; // MultiplePacketsRequest

#endif // MULTIPLE_PACKETS_REQUEST_H

