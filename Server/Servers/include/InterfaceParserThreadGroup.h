/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INTERFACEPARSERTHREADGROUP_H
#define INTERFACEPARSERTHREADGROUP_H

#include "config.h"

#include "ParserThreadGroup.h"
#include "InterfaceParserThreadConfig.h"
#include "InterfaceHandleIO.h"
#include "NotifyPipe.h"
#include "ServerTypes.h"

class ThreadRequestHandler;
class InterfaceIO;
class InterfaceRequest;
class InterfaceFactory;

/**
 * Class that handles a group of interface parser threads.
 *
 */
class InterfaceParserThreadGroup : public ParserThreadGroup, 
                                   public InterfaceHandleIO 
{
   public:
      /** 
       * Creates a new InterfaceParserThreadGroup.
       *
       * @param serverType The type of server
       * @param threadGroupName The name of the group.
       * @param minNbrThreads The lowest number of parserthreads.
       * @param maxNbrThreads The highest number of parserthreads.
       * @param queueFactor The number of irequests, calculated as
       *                    queueFactor*nbrThreads, there can be in
       *                    waiting queue before it is concidered full.
       * @param queueOverFullFactor The number of irequests, calculated as
       *        queueFactor*queueOverFullFactor*nbrThreads, there can be in
       *        waiting queue before it is concidered over full.
       */
      InterfaceParserThreadGroup( ServerTypes::servertype_t serverType,
                                  const char* threadGroupName =
                                  "InterfaceParserThreadGroup",
                                  uint32 minNbrThreads = 5,
                                  uint32 maxNbrThreads = 20,
                                  uint32 queueFullFactor = 3,
                                  uint32 queueOverFullFactor = 9 );

      
      /**
       * Destructor.
       */
      virtual ~InterfaceParserThreadGroup();


      /**
       * Start the parser threads.
       * @param handler The ThreadRequestHandler that this group should
       *                use.
       * @param interfaceIO The class handling the IO, handled 
       *                    InterfaceRequest are returned to it.
       * @param interfaceFactories The InterfaceFactories to terminate on
       *                           shut down.
       */
      void start( ThreadRequestHandler* handler,
                  InterfaceIO* interfaceIO,
                  set< InterfaceFactory* > interfaceFactories,
                  TimedOutSocketLogger* socketLogger);

      /**
       * Begin shutdown of server, terminates all InterfaceFactories,
       * empties the queue and then ends all threads.
       * Monitor method.
       */
      void shutDown();

      /**
       * Return a handled InterfaceRequest and 
       * get a new InterfaceRequest to handle.
       * Monitor method, might hang for a while.
       *
       * @param ireply The handled InterfaceRequest, may be NULL.
       * @param ireq Set to the InterfaceParserThread to handle, 
       *             maybe NULL.
       * @param me The calling InterfaceParserThread.
       * @return True if thread me should continue to run, false if me 
       *         should terminate.
       */
      bool returnAndGetInterfaceRequest( InterfaceRequest* ireply,
                                         InterfaceRequest*& ireq, 
                                         ParserThreadHandle me );

      /**
       * @name InterfaceHandleIO methods.
       */
      //@{
         /**
          * Put a InterfaceRequest to handle, put ready and new
          * InterfaceRequests here.
          * Monitor method.
          *
          * @param ireq The InterfaceRequest to handle, set to NULL if
          *             put into this else caller must handle it by
          *             making a minimal effort returning an 
          *             "Server overloaded" error.
          * @return Queue full number, the amount of irequests that are 
          *         over the full limit.
          */
         virtual int putInterfaceRequest( InterfaceRequest*& ireq );

         /**
          * Handle a done InterfaceRequest, default implementation here
          * simply deletes it.
          *
          * @param ireply The InterfaceRequest that is done.
          */
         virtual void handleDoneInterfaceRequest( InterfaceRequest* ireply );
      //@}

      /**
       * Get the minimum number of threads.
       * Monitor method.
       */
      uint32 getMinNbrThreads();


      /**
       * Get the maximum number of threads.
       * Monitor method.
       */
      uint32 getMaxNbrThreads();


      /**
       * Set min and max number threads.
       * Monitor method.
       */
      void setNbrThreads( uint32 minNbrThreads, uint32 maxNbrThreads );


      /**
       * Set queue factors.
       * Monitor method.
       */
      void setQueueFactors( uint32 queueFullFactor, 
                            uint32 queueOverFullFactor );

      /**
       * Set the pipe to nofity when shutdown starts.
       */
   void setShutdownPipe( NotifyPipe& pipe ) { m_shutdownPipe = &pipe; }

   /**
    * The PreCacheTileMapHandler is done, no more requestes will be sent.
    */
   virtual void preCacheDone( ParserThreadHandle preCahce );
   
   
   /**
    * Print the status on s. Don't call often locks alot.
    */
   void printStatus( ostream& s );

   protected:
      /**
       * Create a new thread for processing, subclasses should override
       * this with their own implementation that creates parserthreads
       * of the desired type.
       *
       * @return A new ParserThread.
       */
      virtual ParserThreadHandle spawnThread() = 0;


      /**
       * Get the first InterfaceFactory.
       */
      InterfaceFactory* getFirstInterfaceFactory();

  private:
      /**
       * Updates the m_queueFullNbr and m_queuOverFullNbr. Call when
       * changing nbr of threads.
       */
      void updateQueueNbr();


      /**
       * Get the number of irequests over the full limit.
       */
      int getNbrOverQueueFull() const;


      /**
       * Get the number of irequests over the over full limit.
       */
      int getNbrOverQueueOverFull() const;


      /**
       * Minimum number of parsing threads.
       */
      uint32 m_minNbrThreads;


      /**
       * Maximum number of parsing threads.
       */
      uint32 m_maxNbrThreads;


      /**
       * The queue full factor.
       */
      uint32 m_queueFullFactor;


      /**
       * The queue full number.
       */
      uint32 m_queueFullNbr;


      /**
       * The queue over full factor.
       */
      uint32 m_queueOverFullFactor;


      /**
       * The queue over full number.
       */
      uint32 m_queueOverFullNbr;


      /**
       * The ready InterfaceRequests.
       */
      std::list < InterfaceRequest* > m_irequests;


      /**
       * The InterfaceIO that handles IO for InterfaceRequests.
       */
      InterfaceIO* m_interfaceIO;

      /**
       * The InterfaceFactories to terminate on shutdown.
       */
      set< InterfaceFactory* > m_interfaceFactories;


      /**
       * The Monitor for InterfaceRequests.
       */
      ISABMonitor m_irequestsMonitor;


      /**
       * The number of idle threads.
       */
      uint32 m_nbrIdleThreads;
   /// the shutdown pipe, listen to this for a shutdown notice
   NotifyPipe* m_shutdownPipe;

};


#endif // INTERFACEPARSERTHREADGROUP_H
