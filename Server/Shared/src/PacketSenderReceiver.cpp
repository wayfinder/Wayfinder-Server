/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PacketSenderReceiver.h"

#include "NetPacketQueue.h"
#include "SelectableSelector.h"
#include "ScopedArray.h"
#include "DatagramSocket.h"
#include "TCPSocket.h"
#include "NetPacket.h"
#include "ISABThread.h"
#include "NetUtility.h"
#include "BufferSender.h"
#include "TimedSelector.h"
#include "MultiPacketSender.h"
#include "QueuedPacketReceiver.h"
#include "NetUtility.h"
#include "DeleteHelpers.h"

#include <memory>
#include <typeinfo>

namespace {

// helps to create a timed selectable for listen sockets
template <typename T>
class TimedSelectorHelper: public TimedSelectable {
public:
   typedef PacketSenderReceiver::ReceiveQueue ReceiveQueue;

   TimedSelectorHelper( T& socket,
                        TimedSelector& selector,
                        ReceiveQueue& packetQueue ):
      m_socket( socket ),
      m_selector( selector ),
      m_packetQueue( packetQueue ) {
      // no timeout
   }

   selectable getSelectable() const { return m_socket.getSelectable(); }
   bool wantRead() const { return true; }
   bool wantWrite() const { return false; }
   void handleIO( bool readyRead, bool readyWrite );
   void handleTimeout() { 
      mc2dbg2 << "[PacketSenderReceiver]: timed selector helper timed out" << endl;
      // not used
      resetTimeout();
   } 
private:
   T& m_socket;
   TimedSelector& m_selector;
   ReceiveQueue& m_packetQueue;
};
// specialization for TCPSocket
template <>
void TimedSelectorHelper<TCPSocket>::
handleIO( bool readyRead, bool readyWrite ) {
   mc2dbg2 << "[PSR] accept connection...TCP" << endl;
   TCPSocket *receiver = m_socket.accept();
   
   if ( receiver != NULL ) {
      m_selector.addSelectable( new QueuedPacketReceiver( receiver,
                                                          m_packetQueue ) );
   }
}
// specialization for UDP Socket
template <>
void TimedSelectorHelper<DatagramReceiver>::
handleIO( bool readyRead, bool readyWrite ) 
{
   // create receive buffer and receive data to it
   if ( ! readyRead ) {
      return;
   }

   ScopedArray<byte>
      buffer( new byte[ Packet::MAX_UDP_PACKET_SIZE ] );

   int size = m_socket.receive( buffer.get(), 
                                Packet::MAX_UDP_PACKET_SIZE );
   if ( size < 0 ) {
      return;
   }

   Packet* packet = Packet::makePacket( buffer.release(), 
                                        Packet::MAX_UDP_PACKET_SIZE );
   packet->setLength( size );

   m_packetQueue.enqueue( packet );
}

class DatagramSenderHelper: public TimedSelectable {
public:
   DatagramSenderHelper() {
   }

   ~DatagramSenderHelper() {
      // any packets not sent before shutdown will be
      // deleted
      STLUtility::deleteQueueValues( m_sendQueue );
   }

   void addPacket( NetPacket* packet ) { 
      m_sendQueue.push( packet );
   }

   bool wantWrite() const { return ! m_sendQueue.empty(); }
   bool wantRead() const { return false; }

   selectable getSelectable() const { return m_sender.getSelectable(); }
   void handleIO( bool read, bool write ) {
      if ( write && ! m_sendQueue.empty() ) {
         auto_ptr<NetPacket> packet( m_sendQueue.front() );
         m_sendQueue.pop();

         DebugClock clock;

         if ( ! m_sender.
              send( &packet->getPacket(), packet->getDestination() ) ) {
            mc2log << warn << " Failed to send packet ["
                   << packet->getPacket().getSubTypeAsString()
                   << "] via UDP to destination "
                   << packet->getDestination() 
                   << " error: " << strerror( errno ) << endl;
         }

         if ( clock.getTime() > 800 ) {
            mc2log << warn << "[PSR] Took " << clock << " to send UDP" << endl;
         } 
      }
   }
   void handleTimeout() {
      // should never happen
      MC2_ASSERT( false );
   }
private:
   queue<NetPacket*> m_sendQueue;
   DatagramSender m_sender;
};

typedef SelectableSelector::selSet SelectableSet;

class WorkThread: public ISABThread {
public:
   WorkThread( PacketSenderReceiver& psr,
               uint32 port );

   void run();

   const IPnPort& getAddr() const { 
      return m_addr;
   }

   void notify() {
      m_selector.getSelector().notify();
   }

   bool isPermanent( Selectable& sel ) const { 
      if ( m_permanent.find( &sel ) != m_permanent.end() ) {
         return true;
      }
      // the pointer might be a "bridge" class
      return 
         find( m_selectableBridges.begin(),
               m_selectableBridges.end(), &sel ) != m_selectableBridges.end();
   }

   void addPermanent( Selectable& sel, int iotype ) try {
      ISABSync sync( m_permMonitor );
      // zero means that we want to be permanent but 
      // not to be included in the selectable list.
      if ( iotype == 0 ) {
         m_permanent.insert( &sel );
         return;
      }

      if ( typeid( sel ) == typeid( TCPSocket ) ) {
         mc2dbg2 << "Adding tcp listen socket" << endl;
         //
         // Create helper for TCPSocket (only accept new connections)
         //
         TimedSelectable* bridge = 
            new TimedSelectorHelper<TCPSocket>( static_cast<TCPSocket&>( sel ), 
                                                m_selector, m_receiveQueue );
         m_selector.addSelectable( bridge );
         // we are must delete this one, add it to delete container
         m_selectableBridges.push_back( bridge );
      } else if ( typeid( sel ) == typeid( DatagramReceiver ) ) {
         mc2dbg2 << "Adding udp listen socket" << endl;
         
         //
         // Create helper for datagram receiver
         //
         TimedSelectable* bridge = 
            new TimedSelectorHelper<DatagramReceiver>( static_cast<DatagramReceiver&>( sel ), 
                                                       m_selector, m_receiveQueue );
         m_selector.addSelectable( bridge );
         // we are must delete this one, add it to delete container
         m_selectableBridges.push_back( bridge );
      } else {
         m_selector.addSelectable( &dynamic_cast<TimedSelectable&>(sel) );
      } 

      m_permanent.insert( &sel );
   } catch (std::bad_cast e ) {  
      //
      // Very bad, strange selectable!
      //
      mc2dbg << fatal << "[PSR] Unknown selectable type!" << endl;
      mc2dbg << "[PSR] typeid is " << typeid( sel ).name() << endl;
      throw;
   }

   void removePermanent( Selectable& sel ) {
      ISABSync sync( m_permMonitor );
      m_permanent.erase( &sel );
      if ( typeid( sel ) == typeid( TimedSelectable ) ) {
         // remove by pointer, fast
         m_selector.removeSelectable( static_cast<TimedSelectable*>(&sel) );
      } else {
         // there can be some dead pointers in TimedSelector at shutdown
         // so we skip this for now until we have a better solution.
         // There is no memory leak though.
         //
         // remove by Selector::selectable, slow
         //  m_selector.removeSelectable( sel.getSelectable() );
      }

      // find and remove the bridge pointer too.

      SelectableBridges::iterator bridgeIt =
         find( m_selectableBridges.begin(),
               m_selectableBridges.end(),
               &sel );
      if ( bridgeIt != m_selectableBridges.end() ) {
         m_selector.removeSelectable( *bridgeIt );
         delete *bridgeIt;
         m_selectableBridges.erase( bridgeIt );
      } else {
         // if pointer search failed try getSelectable() search.
         // should use find_if + compose, but im too lazy
         bridgeIt = m_selectableBridges.begin();
         SelectableBridges::iterator bridgeItEnd = m_selectableBridges.end();
         for ( ; bridgeIt != bridgeItEnd; ++bridgeIt ) {
            if ( (*bridgeIt)->getSelectable() == sel.getSelectable()  ) {
               m_selector.removeSelectable( *bridgeIt );
               delete *bridgeIt;
               m_selectableBridges.erase( bridgeIt );
               break;
            }
         }
      }

   }

   bool hasCachedConnection( const IPnPort& addr ) const;

   void clearIdleAndCachedConnections();

private:
   void select();
   void dequeueSendPackets();
   bool sendWithCached( const NetPacket& packet );

   void addCache( const IPnPort& addr, MultiPacketSender* sender );
   void removeCache( const IPnPort& addr );
   
   typedef map<IPnPort, MultiPacketSender*> DestinationMap;
   DestinationMap m_destinationCache; ///< cached connections

   ::DatagramSenderHelper m_datagramSender; ///< for sending packets
   auto_ptr<DatagramReceiver> m_datagramReceiver; ///< receiving udp socket

   TCPSocket m_tcpSocket; ///< receiving tcp socket
   
   TimedSelector m_selector; ///< selects among selectables (in this case sockets)

   PacketSenderReceiver::ReceiveQueue& m_receiveQueue; ///< packets received
   PacketSenderReceiver::SendQueue& m_sendQueue; ///< packets to send
   PacketSenderReceiver& m_handler;
   IPnPort m_addr;

   SelectableSet m_permanent;
   ISABMonitor m_permMonitor;
   ISABMonitor m_cacheMonitor;
   /**
    * Contains selector bridges (aka selectable helpers) that must be deleted
    * by this instance
    */
   typedef STLUtility::AutoContainer< std::vector<TimedSelectable*> > SelectableBridges;
   SelectableBridges m_selectableBridges; 

};

}


struct PacketSenderReceiver::Impl {
   SendQueue m_sendQueue; ///< send queue
   ReceiveQueue m_receiveQueue; ///< receive queue

   ::WorkThread* m_workThread; ///< working thread for receive
   ISABThreadHandle m_workHandle; ///< thread handle for work thread
};


//////////////////////////////////////////////////////////////////////

PacketSenderReceiver::PacketSenderReceiver( uint32 port ):
   m_impl( new PacketSenderReceiver::Impl ) 
{
   m_impl->m_workThread = new ::WorkThread( *this, port );
   m_impl->m_workHandle = m_impl->m_workThread;
}

PacketSenderReceiver::~PacketSenderReceiver() 
{
   stop();
   while ( m_impl->m_workHandle->isAlive() ) try {
      // wake up the selector
      m_impl->m_workThread->notify();
      m_impl->m_workHandle->join();
   } catch (...) {
   }
}

void PacketSenderReceiver::start() 
{

   m_impl->m_workThread->start();
}

void PacketSenderReceiver::stop() 
{
   m_impl->m_workThread->terminate();
   m_impl->m_workThread->notify();
}

 
PacketSenderReceiver::SendQueue&
PacketSenderReceiver::getSendQueue()
{
   return m_impl->m_sendQueue;
}

PacketSenderReceiver::ReceiveQueue&
PacketSenderReceiver::getReceiveQueue()
{
   return m_impl->m_receiveQueue;
}

const IPnPort& PacketSenderReceiver::getAddr() const {
   return m_impl->m_workThread->getAddr();
}

uint16 PacketSenderReceiver::getPort() const {
   return getAddr().getPort();
}

void PacketSenderReceiver::addPermanentSelectable( Selectable& sel, int iotype ) 
{
   m_impl->m_workThread->addPermanent( sel, iotype );
}

void PacketSenderReceiver::removePermanentSelectable( Selectable& sel ) 
{
   m_impl->m_workThread->removePermanent( sel );
}


void PacketSenderReceiver::selectDone() 
{
}

bool PacketSenderReceiver::hasCachedConnection( const IPnPort& addr ) const {
   return m_impl->m_workThread->hasCachedConnection( addr );
}

void PacketSenderReceiver::clearIdleAndCachedConnections() {
   m_impl->m_workThread->clearIdleAndCachedConnections();
}
//////////////////////////////////////////////////////////////////////

namespace {


WorkThread::WorkThread( PacketSenderReceiver& psr,
                        uint32 port ):
   ISABThread( NULL, "PacketSenderReceiver::WorkThread" ),
   m_receiveQueue( psr.getReceiveQueue() ),
   m_sendQueue( psr.getSendQueue() ),
   m_handler( psr )
{
   m_sendQueue.setSelectNotify( &m_selector.getSelector() );

   //
   // Find a free port for UDP and try to match 
   // the TCP port
   //

   mc2dbg << "[PSR] find free port: "<< port << endl;

   m_tcpSocket.open();

   uint32 nbrTries = 100;

   // find free port
   port--; // start from "port"
   do {
      port = m_tcpSocket.listen( port + 1, TCPSocket::FINDFREEPORT );
      m_datagramReceiver.reset( new DatagramReceiver( port ) );
   } while ( port != m_datagramReceiver->getPort() && 
             --nbrTries > 0 );

   m_addr = IPnPort( NetUtility::getLocalIP(), port );

   // still no match?, this is not good.   
   if ( m_addr.getPort() != port ) {
      mc2log << fatal
             << "[PSR] Failed to open listening port for tcp and udp. Exiting!"
             << endl;
      exit( 2 );
   }

   mc2dbg << "[PSR] free port done. address is " << m_addr << endl;

   if ( ! m_tcpSocket.setBlocking( false ) ) {
      mc2dbg << fatal << "[PSR] Failed to set nonblocking."  << endl;
      exit( 2 );
   }

   m_sendQueue.setSelectNotify( &m_selector.getSelector() );

   // add UDP and TCP listeners to permanent sockets so we
   // dont delete them
    
   addPermanent( *m_datagramReceiver, 
                 PacketSenderReceiver::READ );
   addPermanent( m_tcpSocket,
                 PacketSenderReceiver::READ );
   addPermanent( m_datagramSender,
                 PacketSenderReceiver::WRITE );
}

void WorkThread::run() 
{
   // 
   // Select reads/writes and handle them
   //
   while ( ! terminated ) {
      
      dequeueSendPackets();

      select();

      // notify handler that we are done with select/read/write
      m_handler.selectDone();

   }

   mc2dbg2 << "[PSR] WorkThread is closing." << endl;

   // destroy sockets
   TimedSelector::SelectableSet selectables;
   m_selector.getSelectables( selectables );
   TimedSelector::SelectableSet::iterator it = selectables.begin();
   TimedSelector::SelectableSet::iterator itEnd = selectables.end();
   for (; it != itEnd; ++it ) {
      // if permanent we did not allocate this selectable
      // thus we do not delete it here.
      if ( ! isPermanent( **it ) ) {
         delete *it;
      }
   }

   mc2dbg << "[PacketSenderReceiver::WorkThread] end." << endl;
}

void WorkThread::select() {

   TimedSelector::SelectableSet timeouts;

   m_selector.select( timeouts );

   TimedSelector::SelectableSet::iterator it = timeouts.begin();
   TimedSelector::SelectableSet::iterator itEnd = timeouts.end();
   for (; it != itEnd; ++it ) {
      if ( isPermanent( **it ) ) {
         m_selector.addSelectable( *it );
      } else {
         try {
            MultiPacketSender& sender = dynamic_cast<MultiPacketSender&>( **it );
            removeCache( sender.getDestination() );
            mc2dbg2 << "[PSR::WorkThread]: Erase dest cache:" 
                    << sender.getDestination() << endl;
         } catch ( bad_cast err ) { }

         mc2dbg2 << "[PSR]: destroy socket: " << (*it)->getSelectable() << endl;
         delete *it;
      }
   }
}

void WorkThread::dequeueSendPackets() 
{
   typedef vector<NetPacket*> PacketVector;
   // get all packets from the queue
   PacketVector sendPackets;
   m_sendQueue.dequeue( sendPackets );

   PacketVector::iterator it = sendPackets.begin();
   PacketVector::iterator itEnd = sendPackets.end();
   for (; it != itEnd; ++it ) {
      MC2_ASSERT( *it );

      auto_ptr<NetPacket> packet( *it );

      mc2dbg4 << "[PSR] sending packet: " << packet->getPacket().getSubTypeAsString()
              << ", destination: " << packet->getDestination() 
              << ", length: " << packet->getPacket().getLength() << endl;

      switch ( packet->getType() ) {
      case NetPacket::UDP:
         mc2dbg4 << " via UDP" << endl;
         //
         // If its multicast we dont check cache else
         // we try to send it with cached TCP connection
         //
         if ( NetUtility::isMulticast( packet->getDestination().getIP() ) ||
              ! sendWithCached( *packet ) ) {
            m_datagramSender.addPacket( packet.release() );
            m_selector.updateTimeout( m_datagramSender );
         }
      
         break;
      case NetPacket::TCP: 
         mc2dbg4 << " via TCP" << endl;

         // first try to send with cached connection
         if ( sendWithCached( *packet ) ) {
            break;
         }

         // else no cache, create new connection and add to cache
         try {
            MultiPacketSender* sender = 
               new MultiPacketSender( packet->getDestination() );
            sender->addPacket( packet->getPacket() );
            m_selector.addSelectable( sender );
            //
            // Add destination to cache
            //
            addCache( packet->getDestination(), sender );


         } catch ( SocketException error ) {
            mc2log << warn 
                   << "[PSR]: Can not connect: " << error.what() << endl;
         }

         break;
      }
   }
}

bool WorkThread::sendWithCached( const NetPacket& packet ) 
{
   //
   // find cached connection
   //
   DestinationMap::iterator findIt =
      m_destinationCache.find( packet.getDestination() );
   if ( findIt != m_destinationCache.end() ) {

      mc2dbg2 << "[PSR] CACHE sending packet: " 
              << packet.getPacket().getSubTypeAsString()
              << ", destination: " << packet.getDestination() 
              << ", length: " << packet.getPacket().getLength() << endl;
      //
      // Found cache!, add packet to sender and update timeout
      //
      MultiPacketSender* sender = (*findIt).second;
      sender->addPacket( packet.getPacket() );
      m_selector.updateTimeout( *sender );
      return true;
   }

   return false;
}


bool WorkThread::hasCachedConnection( const IPnPort& addr ) const 
{
   ISABSync sync( m_cacheMonitor );
   return m_destinationCache.find( addr ) != m_destinationCache.end();
}

void WorkThread::addCache( const IPnPort& addr, MultiPacketSender* sender ) 
{
   ISABSync sync( m_cacheMonitor );
   m_destinationCache[ addr ] = sender;
}

void WorkThread::removeCache( const IPnPort& addr ) {
   ISABSync sync( m_cacheMonitor );
   m_destinationCache.erase( addr );
}

void WorkThread::clearIdleAndCachedConnections() {
   ISABSync sync( m_cacheMonitor );
   DestinationMap::const_iterator it = m_destinationCache.begin();
   DestinationMap::const_iterator itEnd = m_destinationCache.end();
   for (; it != itEnd; ++it ) {
      TimedSelectable* sel = (*it).second;
      if ( ! isPermanent( *sel ) ) {
         m_selector.removeSelectable( sel );
         delete sel;
      } 
   }
   m_destinationCache.clear();
   
   TimedSelector::SelectableSet selectables;
   m_selector.getSelectables( selectables );

   TimedSelector::SelectableSet::const_iterator sit = selectables.begin();
   TimedSelector::SelectableSet::const_iterator sitEnd = selectables.end();
   for ( ; sit != sitEnd; ++sit ) { 
      BufferReceiver* receiver = dynamic_cast<BufferReceiver*>(*sit);
      // remove receiver, 
      // TODO: check for idle connections first.
      //       but there seems to be a bug with
      //       receiver->idle state change in BufferReciver.
      //       The receiver seems to be in receiving state even
      //       when it is suppose to be idle.
      if ( receiver && 
           !isPermanent( *receiver ) ) {
         m_selector.removeSelectable( receiver );
         delete receiver;
      }
   }
}

} // end anonymous namespace
