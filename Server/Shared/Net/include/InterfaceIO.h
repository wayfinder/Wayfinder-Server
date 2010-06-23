/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INTERFACEIO_H
#define INTERFACEIO_H

#include "config.h"


class InterfaceHandleIO;
class InterfaceRequest;

/**
 * Abstract super class for handling InterfaceRequests IO.
 *
 */
class InterfaceIO {
   public:
      /**
       * Creates a new InterfaceIO.
       *
       * @param group The InterfaceHandleIO to send
       *              InterfaceRequests ready to be processed to.
       */
      InterfaceIO( InterfaceHandleIO* group );


      /**
       * Destructor.
       */
      virtual ~InterfaceIO();


      /**
       * A new InterfaceRequest to do IO for.
       */
      virtual void putInterfaceRequest( InterfaceRequest* ireq ) = 0;


      /**
       * If no InterfaceRequests in this.
       */
      virtual bool empty() const = 0;

      /**
       * The number of InterfaceRequest in this.
       */
      virtual uint32 size() const = 0;

      /**
       * Shutdown, stop idle IOs.
       * Default is to do nothing.
       */
      virtual void shutdownStarts();


   protected:
      /**
       * The InterfaceHandleIO.
       */
      InterfaceHandleIO* m_group;
};


#endif // INTERFACEIO_H


