/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SMSLISTENERENTRY_H
#define SMSLISTENERENTRY_H

#include "VectorElement.h"
#include "TCPSocket.h"

/**
 *    SMSListnerElement.
 *
 */
class SMSListenerElement : public VectorElement {
   public:
      /**
       */
      SMSListenerElement(uint32 ip, uint16 inport) {
         m_port   = inport;
         m_ip     = ip;
         m_socket = NULL;
      };

      /**
       */
      uint32 getIP() const {
         return m_ip;
      };

      /**
       */
      uint16 getPort() const {
         return m_port;
      };

      /**
       */
      TCPSocket* getSocket() const {
         return m_socket;
      };
      
      /**
       */
      void setSocket(TCPSocket* socket) {
         m_socket = socket;
      };
      
      /**
       *    Overloaded operater, used to determine if this object is
       *    equal to the other.
       *    @return True if equal.
       */
      bool virtual operator == (const VectorElement& elm) const {
         const SMSListenerElement* selm =
            dynamic_cast<const SMSListenerElement *>(&elm);      
         return getPort() == selm->getPort() && getIP() == selm->getIP();
      };
      
      /**
       *    Overloaded operater, used to determine if this object not 
       *    is equal to the other.
       *    @return True if not equal.
       */
      bool virtual operator != (const VectorElement& elm) const {
         const SMSListenerElement* selm =
            dynamic_cast<const SMSListenerElement *>(&elm);  
         return getPort() != selm->getPort() || getIP() != selm->getIP();
      };
   
   private:
      /**
       */
      uint32 m_ip;

      /**
       */
      uint16 m_port;

      /**
       */
      TCPSocket* m_socket;
};

#endif


