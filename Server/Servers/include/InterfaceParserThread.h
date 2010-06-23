/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INTERFACEPARSERTHREAD_H
#define INTERFACEPARSERTHREAD_H

#include "config.h"
#include "ParserThreadConfig.h"
#include "ParserThread.h"
#include "InterfaceParserThreadConfig.h"


// Forwards
class ThreadRequestHandler;
class InterfaceRequest;


/**
 * Class that handles a  of interface parser threads.
 *
 */
class InterfaceParserThread : public ParserThread {
   public:
      /** 
       * Creates a new InterfaceParserThread.
       *
       * @param group The InterfaceParserThreadGroup that this thread
       *              belongs to.
       * @param threadName The name of the thread.
       */
      InterfaceParserThread( InterfaceParserThreadGroupHandle group,
                             const char* threadName = 
                             "InterfaceParserThread" );

      
      /**
       * Destructor.
       * Don't call this use terminate.
       */
      virtual ~InterfaceParserThread();


   protected:

      /**
       * This function is called when a InterfaceRequest has been received.
       * Implement in subclass.
       *
       * @param ireq The InterfaceRequest to process.
       */
      virtual void handleInterfaceRequest( InterfaceRequest* ireq ) = 0;

   private:
      /**
       * Does it, don't call this call start.
       */
      void run();

};


#endif // INTERFACEPARSERTHREAD_H

