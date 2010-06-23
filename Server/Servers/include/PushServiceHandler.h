/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PUSHSERVICEHANDLER_H
#define PUSHSERVICEHANDLER_H

#include "config.h"
#include "PushServices.h"
#include "ISABThread.h"
#include <list>


typedef list< pair< pair< uint32, SubscriptionResource* >, 
   PushPacket* > > PushPacketList;

class DatagramSender;
class SocketReceiver;
class TCPSocket;

/**
 * Class that handles Push in a Server.
 *
 */
class PushServiceHandler {
   public:
      /**
       * Handles Push in a Server.
       * Has one thread.
       *
       * @param port The port to listen on.
       */
      PushServiceHandler( uint16 port );


      /**
       * Destructor, notifies threads waiting in getPushPacket.
       */
      virtual ~PushServiceHandler();

      
      /**
       * Starts the thread.
       */
      void start();

      
      /**
       * Returns received a PushPacket.
       * GetMonitor function.
       * 
       * @param serviceID Set to the serviceID of the PushPacket.
       * @param resource Set to the resource of the PushPacket or NULL
       *                 if notified. Deleted by caller.
       * @return a PushPacket or NULL if notified.
       */
      PushPacket* getPushPacket( uint32& serviceID, 
                                 SubscriptionResource*& resource );

      
      /**
       * Awakens all threads waiting in getPushPacket;
       * GetMonitor function.
       */
      void notifyGetPushPacket();


      /**
       * Add a PushService.
       * Mutex function.
       * 
       * @param service The PushService to add. The object is owned by 
       *                this class after the call to this method.
       * @param lastUpdateTime For all resources the lastUpdateTime is 
       *                       added to this vector.
       */
      void addPushService( PushService* service,
                           vector<uint32>& lastUpdateTime );


      /**
       * Removes a PushService entierly.
       * Mutex function.
       *
       * @param serviceID The ID of the PushService to remove.
       * @return True if serviceID did exist, false if it didn't.
       */
      bool removePushService( uint32 serviceID );


      /**
       * Removes a certain resource from a PushService.
       * Mutex function.
       *
       * @param serviceID The ID of the PushService to remove.
       * @param resource The resource to stop having push for.
       * @return True if resource did exist, false if it didn't.
       */
      bool removePushServiceResource( uint32 serviceID, 
                                      SubscriptionResource& resource );


   private:
      /**
       * The PushServices.
       */
      PushServices* m_PushServices;

      
      /**
       * The mutex.
       */
      ISABMutex m_mutex;

      
      /**
       * The GetMotitor
       */
      ISABMonitor m_getMonitor;


      /**
       * The SendMutex.
       */
      ISABMutex m_sendMutex;

      
      /**
       * The ready PushPackets.
       */
      PushPacketList m_pushPackets;


      /**
       * Handle a received push packet.
       * Mutex function.
       *
       * @param pushPacket The push packet to handle.
       * @param pushSocket The socket that the pushPacket was received on.
       * @param isDataPacket Set to true if the pushPacket is a data 
       *                     packet.
       * @param serviceID  Set to the serviceID of the pushPacket if it is
       *                   a data packet.
       * @param resource Set to the resource of the pushPacket if it is a
       *                 data packet or NULL if not.
       */
      void handlePushPacket( PushPacket* pushPacket, TCPSocket* pushSocket,
                             bool& isDataPacket,
                             uint32& serviceID, 
                             SubscriptionResource*& resource );


      /**
       * Handle a broken socket.
       * Mutex function.
       *
       * @param pushSocket The socket that is broken.
       */
      void handleBrokenSocket( TCPSocket* pushSocket );


      /**
       * Make all packets that has to be sent and produce a new timeout.
       * Mutex function.
       *
       * @return Time for next timeout.
       */
      uint32 checkAndCalculateTimeout();


      /**
       * Send a list of packets.
       * SendMutex function.
       *
       * @param packetList The list of packets to send.
       */
      void sendPackets( PacketContainerList& packetList );


      /**
       * Add an outgoing PushPacket.
       * Mutex function.
       *
       * @param pushPacket The PushPacket to add to outgoing packets.
       * @param serviceID The serviceID of the pushPacket.
       * @param resource The SubscriptionResource of the pushPacket.
       */
      void addPushPacket( PushPacket* pushPacket, 
                          uint32 serviceID, 
                          SubscriptionResource* resource );



      /**
       * Class to handle waiting for pushdata and resubscription.
       */
      class PushServiceThread : public ISABThread {
         public:
            /**
             * Create a new PushServiceThread
             *
             * @param handler PushServiceHandler this is part of.
             * @param listenSocket The socket to listen for new Push
             *                     connctions on.
             */
            PushServiceThread( PushServiceHandler* handler,
                               TCPSocket* listenSocket );


            /**
             * Destructor for PushServiceThread.
             */
            virtual ~PushServiceThread();


            /**
             * PushServiceThread's run.
             */
            virtual void run();


            /**
             * Force restart so that new timeout may be calculated.
             */
            void forceRestart();


            /**
             * Terminate on SocketReceiver, ei. goto end.
             */
            void terminateSocketSelection();


         private:
            /**   
             * PushServiceHandler this PushServiceThread is part of.
             */
            PushServiceHandler* m_handler;


            /**
             * The SocketReceiver.
             */
            SocketReceiver* m_receiver;

            
            /**
             * The listenSocket
             */
            TCPSocket* m_listenSocket;


            /**
             * The added sockets.
             */
            list<TCPSocket*> m_addedSockets;
      };

      
      /**
       * PushServiceThread is a friend;
       */
      friend class PushServiceThread;


      /**
       * The PushServiceThread.
       */
      PushServiceThread* m_pushServiceThread;


      /**
       * The DatagramSender.
       */
      DatagramSender* m_datagramSender;

      
      /**
       * The listenSocket.
       */
      TCPSocket* m_listenSocket;


      /**
       * The IP of this handler.
       */
      uint32 m_ip;


      /**
       * The TCP listenSocket port.
       */
      uint16 m_listenPort;
};


#endif // PUSHSERVICEHANDLER_H

