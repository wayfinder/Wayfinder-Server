/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKETRESENDHANDLER_H
#define PACKETRESENDHANDLER_H

#include "config.h"
#include "ISABThread.h"
#include "NotCopyable.h"

#include <memory>

class PacketContainerTree;
class PacketSenderReceiver;
class ServerPacketSender;
class PacketContainer;
class Packet;

/**
 * Packet resender class. Has two ISABThreads, one for resending and one 
 * for reading from the network.
 *
 */
class PacketResendHandler: private NotCopyable {
public:
      /**
       * Creates a new PacketResendHandler.
       * Use start to begin receving and resending.
       *
       * @param port The port to listen for incomming packets on, 
       *             if port number port is allready used next number 
       *             is tested and so on, in other words find free port 
       *             is used.
       */
      PacketResendHandler( uint16 port );

      
      /**
       * Terminates the two threads and notifies the answer monitor.
       * Might take up to 1000 ms before threads really ends so
       * this destructor sleeps 3000 ms.
       */
      ~PacketResendHandler();

      
      /**
       * Starts the two threads.
       */
      void start();

      /**
       * Adds and sends a new PacketContainer.
       * If moduleType in cont is MODULE_TYPE_INVALID the getSendIP and
       * getSendPort is used as destination.
       * Monitor function.
       *
       * @param cont The PacketContainer with packet to send.
       */
      void addAndSend( PacketContainer* cont );


      /**
       * Returns a received answer.
       * Function will hang until there is a packet to return or
       * notify is called.
       * AnswerMonitor function.
       *
       * @return The requesting PacketContainer with answer in the 
       *         containers getAnswer, which can be NULL. Returns NULL
       *         if NotifyAnswer was called.
       */
      PacketContainer* getAnswer();


      /**
       * Notifies the answer so that threads hanging on getAnswer is 
       * awakend.
       * AnswerMonitor function.
       */
      void notifyAnswer();

      
      /**
       * Returns a received answer if there is one, NULL if no answers.
       * Will not wait for answer if there is no ready answer.
       * AnswerMonitor function.
       *
       * @return The requesting PacketContainer with answer in the 
       *         containers getAnswer, which can be NULL. Returns NULL
       *         if no answer is ready.
       */
      PacketContainer* getAnswerNow(); 


      /**
       * Return the port used to listen on returning packets on.
       *
       * @return The used port number.
       */
      uint16 getPort();


private:
      /**
       *   Check the timestamp on the first packetContainer in the 
       *   PriorityQueue.
       *   If the timeout is due - resend the packet.
       *   If the maximum resend count is reached - remove and return the 
       *   PacketContainer.
       *   Monitor function.
       *
       * @return If first PacketContainer has reached maximun resend count
       *         
       */
      PacketContainer* checkAndResend();
      

      /**
       *   Find a packet matching a packetID, requestID.
       *   Remove it from the list if it is found and has an answer when
       *   pack has been handled.
       *   Monitor function.
       * 
       *   @param pack The answer Packet.
       *   @param res Is set to the requesting PacketContainer, NULL
       *          if no matching requesting PacketContainer or not
       *          compelete answer.
       *   @return true if pack was properly processed, complete or not, 
       *           false if no matching PacketContainer.
		 */
      bool findAndRemoveIfDone( Packet* pack, PacketContainer*& res );


      /**
       * Handles a AcknowledgeRequestReplyPacket.
       * Monitor function.
       * 
       * @param pack Is the AcknowledgeRequestReplyPacket.
       * @param res Set the the reqeuesting PacketContainer if aborted.
       * @return true if updated ok, false if request should be aborted.
       */
      bool upDateRequestPacket( Packet* pack, PacketContainer*& res );

      
      /**
       * Add answer.
       * AnswerMonitor function.
       *
       * @param cont The answer to add.
       */
      void addAnswer( PacketContainer* cont );


      /**
       *   Send the packet contained in the packetcontainer.
       */
      bool send( PacketContainer* cont );

      
      /**
       *   Sends the TCP packets in the queue.
       * Monitor function.
       *
       * @param pack The LeaderIPReplyPacket to use when sending the 
       *             enqueued packets.
       */ 
      void sendTCPPackets( Packet* pack );

      

      /**
       *  Reads a packet from the network.
       *
       *  @param micros The max time to wait for a packet, in microseconds.
       *  @return  The packet that was received.
       */
      Packet* receive( uint32 maxWaitTime );


      /**
       * Removes any packets from the tcpqueue for a PacketContainer.
       *
       * @param cont The PacketContainer to be removed fron the tcpqueue.
       */
      void removeFromTCP( PacketContainer* cont );



      /**
       * Monitor protecting this class functions.
       */
      ISABMonitor m_monitor;

      
      /**
       * The tree of outstanding packets.
       */
      auto_ptr<PacketContainerTree> m_contTree;


      /**
       * Monitor protecting the answer queue.
       */
      ISABMonitor m_answerMonitor;

      
      /**
       * The tree of answers.
       */
      auto_ptr<PacketContainerTree> m_answerTree;
      

      /**
       * If handler is going down.
       */
      bool m_running;

      /** 
       * Thread to handle receiving of packets from the network.
       *  
       */
      class ReceiveThread : public ISABThread {
         public:
            /**
              * Create a new ReadThread.
              *
              * @param handler The PacketResendHandler this thread is 
              *        part of.
              */
            ReceiveThread( PacketResendHandler* handler );


            /**
              *  Destructor of the ReceiveThread.
              */
            virtual ~ReceiveThread();


            /**
              *   Start the ReceiveThread.
              */
            virtual void run();


         private:
            /**   
              *   The PacketResendHandler this ReceiveThread is part of.
              */
            PacketResendHandler* m_handler;
	    
      };

      

      /**   
       * Thread to resend lost packets.
       *
       */
      class TimeoutThread : public ISABThread {
         public:
            /**
              * Create a new TimeoutThread.
              * @param handler The PacketResendHandler this thread
              *        is part of.
              */
            TimeoutThread( PacketResendHandler* handler );


            /**
              *   Destructor of the TimeoutThread.
              */
            virtual ~TimeoutThread();


            /**
              *   Start the TimeoutThread.
              */
            virtual void run();


         private:
            /**   
              *   The PacketResendHandler this TimeoutThread is part of.
              */
            PacketResendHandler* m_handler;  
      };


      /**
       * The thread that handles receiving.
       */
      ReceiveThread* m_receiveThread;

      
      /**
       * The thread that handles resending.
       */
      TimeoutThread* m_timeoutThread;
   
      /// ReceiveThread is a friend
      friend class ReceiveThread;


      /// TimeoutThread is a friend
      friend class TimeoutThread;

   auto_ptr<PacketSenderReceiver> m_senderReceiver;
   auto_ptr<ServerPacketSender> m_packetSender;
};


// =======================================================================
//                                 Implementation of the inlined methods =

#endif // PACKETRESENDHANDLER_H









