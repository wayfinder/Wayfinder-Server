/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __STANDARDREADER_H__
#define __STANDARDREADER_H__

#include "config.h"

#include "Reader.h"
#include "ReaderBase.h"
#include "NetPacketQueue.h"

class Packet;              // forward decl
class VotePacket;          // forward decl
class StatisticsPacket;    // forward decl
class PacketReaderThread;  // forward decl
class MapNotice;           // forward decl
class Queue;               // forward decl
class Fifo;                // forward decl
class ModuleList;          // forward decl
class ModuleNotice;        // forward decl
class LoadMapReplyPacket;  // forward decl
class DatagramSender;      // forward decl
class RequestPacket;       // forward decl

class AliveRequestPacket;  // forward decl
class AliveReplyPacket;    // forward decl
class AllMapReplyPacket;

class ModulePushList;      // forward decl
class PushService;
class PushServices;
class Balancer;
class JobTimeoutHandler;


#include "PushService.h" // For PacketContainerList
#include "ModuleTypes.h"
#include "PointerFifoTemplate.h"

/** 
  *   The standard implementation for a Reader, many of the
  *   modules have their own sub-classes.
  * 
  */
class StandardReader : public Reader, protected ReaderBase {
   /// Rate of heartbeat ($ms^{-1}$)
#define HEARTBEAT_RATE 1000

   /// Rate of sending vote-packets during election (ms)
#define ELECTION_WON_RATE 500

public:
   /** The only constructor for the reader. Initialize the module 
    * {\bf not} to be a leader.
    *
    * @param moduleType      The type of module.
    * @param q               The queue that the jobs are inserted in.
    * @param preferedPort    Portnumber to be used for communication
    *                        with only this module. If the portnumber
    *                        is already in use it will b increased until
    *                        a free one is found.
    * @param leaderIP        The IP-address to the leader-multicast
    *                        group in hex.
    * @param leaderPort      The port used for leader-group.
    * @param availableIP     The IP-address to the available-multicast
    *                        group in hex.
    * @param availablePort   The port used for available-gruop.
    * @param definedRank     How ``good'' this module is when selecting
    *                        new leader. {\em This might change when the
    *                        ``ini-fil-object'' is in use.}
    * @param usesMaps        True if the module uses maps, otherwise false.
    * @param wantedSubscriptions
    *                        Vector containing PushServices wanted
    *                        by the module. Should not be deleted by
    *                        the caller.
    *
    */
   StandardReader( moduletype_t moduleType,
           Queue* inQueue, NetPacketQueue& sendQueue,
           MapSafeVector* p_loadedMaps,
           PacketReaderThread& packetReader,
           uint16 listenPort,
           int32 definedRank,
           bool usesMaps = true,
           vector<PushService*>* wantedSubscriptions = NULL,
           bool waitForPush = false);


   /**
    * Destructor.
    */
   virtual ~StandardReader();

      
   /**
    *    The main loop in this reader. {\it {\bf NB!} This might {\bf
    *    not} be called explicitly, the threads are started by calling
    *    the start() method}.
    */
   virtual void run();

   void setJobTimeoutHandler( JobTimeoutHandler* handler );

   /**
    *    Return true if leader
    *   @return true if this module is a leader
    */
   bool isLeader() const;
   
   /**
    * Returns an object which can be queried about 
    * the leader/available-status.
    * @see LeaderStatus
    */
   const LeaderStatus* getLeaderStatus() const;

   /**
    *    Return the number of leader/available votes since startup
    *    @return number of votes
    */
   uint32 getNbrVotes();

   /**
    *    Return the number of processed requests since startup
    *    @return number of requests
    */
   uint32 getNbrRequests();

   /**
    *    Return the rank of this module
    *    @return the module's rank
    */
   int32 getRank();

   /**
    *    Set the rank of this module
    *    @param newRank the module's new rank
    */
   void setRank(int32 newRank);

   /**
    *    Initiate a vote
    */
   void vote();
   /// a last vote (not send via PacketSenderReceiver) used when shutting down reader
   void lastVote();

   /**
    *   Initializes the leader (retreives all available maps 
    *   from the MapModule).
    */
   virtual void initLeader(set<MapID>& allMaps);
   
   /**
    *    Get the name of this module.
    *    @return  The name of this module taken from the module type.
    */
   inline const char* getName();

   /**
    *    Returns the vector of loaded maps. 
    */
   MapSafeVector* getLoadedMaps() { return m_loadedMaps; }

   /**
    * Set "start as leader" state.
    * @param state The new state.
    */
   void setStartAsLeader( bool state ) { m_startAsLeader = state; }
   
   /// @reimpl
   int getQueueLength() { return ReaderBase::getQueueLength(); }
   
protected:

   /**
    *    Requests allmappacket from the MapModule. Not to be used
    *    in the MapModule.
    *    @return NULL if failure.
    */
   AllMapReplyPacket* requestAllMapPacket();
   
   /**
    *    Processes an AliveRequestPacket and returns an AliveReplyPacket.
    *    @param arp The packet to process.
    *    @return A reply to that packet.
    */
   inline AliveReplyPacket*
   processAliveRequestPacket(const AliveRequestPacket* arp) const;
      
   /**
    *   Handles a timeout when waiting for something to be int
    *   the inqueue.
    *   @param p     Packet received from socket. Should be null.
    *   @param time1 How long the reader waited for packet.
    *   @param nbrHeartbeat The number of heartbeats (for debugging).
    */
   void handleReceiveTimeout(int32 time1,
                             int &nbrHeartbeat);
   /**
    *   Handle the voting.
    */
   void doVote();

   /**
    *  Read from the property file to get the initial values.
    *  Virtual to make it possible for modules to read other values.
    *  Subclasses readPropValues() can call this method after setting
    *  their special values as only MAX_UINTXX values will be affected.
    *  @return True if all values were found. False if one or more values
    *          couldn't be found and defaults were used.
    */
   virtual bool readPropValues();
      
   /**
    *   This function will clean things up if the
    *   terminate function is called, since the threads
    *   shouldn't be destroyed with the destructor
    */
   void cleanUp();
      
   /**
    *    @name Handle a packet
    *    @memo Methods to handle one incomming packet.
    *    @doc  Methods to handle one incomming packet. What method
    *          to call depends on if the module is running as leader
    *          or available and if the incomming packet is a 
    *          control-packet or not. A control-packet is a packet
    *          that have the control bit set in the subtype.
    *    @see  Packet
    */
   //@{
   /** 
    *    Function that processes the incomming packet. Will call 
    *    one of the following methods:
    *    \begin{description}
    *       \item[availableProcessCtrlPacket(Packet* p)]
    *          This method is called if this module is running as
    *          available and p is a control packet.
    *       \item[availableProcessNonCtrlPacket(Packet* p)]
    *          This method is called if this module is running as
    *          available and p not is a control packet.
    *       \item[leaderProcessCtrlPacket(Packet* p)]
    *          This method is called if this module is running as
    *          leader and p is a control packet.
    *       \item[leaderProcessNonCtrlPacket(Packet* p)]
    *          This method is called if this module is running as
    *          leader and p not is a control packet.
    *    \end{description}
    *
    *    @param p The packet to be processed.
    */
   void processPacket(Packet *p);

   /**
    *    Sends vote packet to availables and leaders.
    *    A time limit will be added here.
    */
   void sendVoteToAll();

   /**
    *    Sends back vote packet to sender only.
    */
   void replyToVote( const VotePacket* p );
   
   /**
    *    Processes a VotePacket. If the VotePacket comes from
    *    a module that is a better leader no packet will be sent.
    *    If this module is a better leader a VotePacket will be
    *    sent and this module will become leader and continue to
    *    send VotePackets until the other modules are silent.
    *    @param p The received VotePacket.
    *    @param wasLeader True if the module was leader when the
    *                     VotePacket was received.
    */
   void processVotePacket(VotePacket* p, bool wasLeader);
         
   /**
    *    Method, that is called by processPacket. If needed, 
    *    overload in subclasses.
    *    @param p The packet.
    */
   virtual void printDebugPacketInfo(Packet *p) {};
         
   /**
    *    Process one control packet, when running as available.
    *    @param p The packet to process.
    *    @return  True if the packet have been handled, false 
    *             otherwise.
    *
    */
   virtual bool availableProcessCtrlPacket(Packet* p);

   /**
    *    Process one non-control packet, when running as available.
    *    @param p The packet to process.
    *    @return  True if the packet have been handled, false 
    *             otherwise.
    *
    */
   virtual bool availableProcessNonCtrlPacket(Packet* p);
         
   /**
    *    Process one control packet, when running as leader.
    *    @param p The packet to process.
    *    @return  True if the packet have been handled, false 
    *             otherwise.
    *
    */
   virtual bool leaderProcessCtrlPacket(Packet* p);
         
   /**
    *    Process one non-control packet, when running as leader.
    *    @param p The packet to process.
    *    @return  True if the packet have been handled, false 
    *             otherwise.
    *
    */
   virtual bool leaderProcessNonCtrlPacket(Packet* p);
   //@}

   /**
    *    @name Initialization
    *    @memo Methods to initialize module as leader or available.
    *    @doc  Methods to initialize module as leader or available.
    */
   //@{
   /**
    *    Change the necessary variables to become leader.
    */ 
   virtual void becomeLeader();

   /**
    *    Change the necessary variables to become an available module.
    */
   virtual void becomeAvailable();
                  
   //@}

   /**
    *    Defines what to do when a heartbeat is received.
    */
   virtual void replyToHeartBeat();

   /**
    *    @name Load map
    *    @memo Methods to make modules load map.
    *    @doc  Methods to make modules load map.
    */
   //@{
         

   //@}

      
   
   /**
    *    @name Send packet
    *    @memo Distribute a packet to a module.
    *    @doc  Distribute a packet to a module.
    */
   //@{
   /**
    *    Send a request packet to the best module. This might
    *    be this module. The packet is either deleted or put in
    *    a queue.
    *    @param   request  The packet to send.
    */
   virtual void sendRequestPacket( RequestPacket* request );
                   
   //@}
   

         
   /**
    *    This method is called when a LoadMapReplyPacket is received
    *    by the processor. If the map was loaded OK the moduleList is
    *    updated so that the module that loaded the map can start
    *    receiving requests right away, without having to wait for the
    *    StatisticsPacket. If the map was not loaded OK or the module
    *    was not in the module list then a HeartBeatPacket will be sent
    *    out to all modules.
    *    <br />
    *    Used to be a virtual method which the RouteModule used
    *    to create DisturbanceSubscriptions, but not anymore.
    *    @param replyPacket The LoadMapReplyPacket from the module
    *                       that has finished loading the map.
    */
   void reactToMapLoaded(LoadMapReplyPacket* replyPacket);
   
   /**
    *   Checks if there are packets to send because of
    *   changed resources.
    *   @return True if there were packets to send.
    */
   bool checkPushResources();
       
   /**
    *   Check if push contents have arrived.
    *   @return True if there was push content.
    */
   bool handlePush();

   /**
    *   Sends the packets in <code>packets</code> to the 
    *   module that provides push.
    *   @param packets PacketContainerList with packets to
    *                  send.
    */
   int sendPushPackets(PacketContainerList& packets);

   /**
    *   Return true if the jobthread has been working too long.
    */
   bool jobThreadWorkingTooLong();
       
   /**
    *   Set the log prefix to correspond to the passed packet
    *   @param  p  Pointer to packet to base the prefix on, NULL
    *              to set prefix to NULL
    */
   void setLogPrefix(Packet* p);

   /**
    * Called by reader periodically, subclasses may override this to
    * make things.
    */
   virtual void periodicMethod();

   /**
    *    Create a new StatisticsPacket and fill it with data.
    *    {\it {\bf NB!} The packet that is returned is created inside
    *    this method, and must be deleted by the caller.}
    *    @return  A statistics packet, filled with information.
    */
   virtual StatisticsPacket* createStatisticsPacket();

   /**
    *    Overloaded in modules which don't use country maps,
    *    like RouteModule and SearchModule.
    *    @see initLeader()
    */
   virtual bool usesCountryMaps() const { return true; }

       
   // -------------------------- Fields --------------------
       
   /** 
    *   Time for last heartbeat (in us.) send if leader, 
    *   recved if available.
    */
   uint32 m_lastHeartBeat;
       
   /** 
    *   The rank defined when starting the module. 
    */
   int32 m_definedRank;
   
   /** 
    *   Status in voting procedure.
    *   Possible values:
    *   \begin{description}
    *      \item[NO_VOTING] Currently no election (default). 
    *      \item[LOST] Beaten in the election will not become leader. 
    *      \item[WON_1] Sent "VOTE" once.
    *      \tem[WON_2] Sent "VOTE" twice.
    *      \item[WON_3] Sent "VOTE" three times.
    *   \end{description}
    */ 
   enum  { NO_VOTING, LOST, WON_1, WON_2, WON_3} m_internalStatus;

   /**
    *    Queue with the packets that are read from the network.
    */
   PointerFifoTemplate<Packet>& m_inPacketQueue;

   /**
    *    The thread that reads the packets from the network and inserts
    *    them into the m_inPacketQueue.
    */
   PacketReaderThread& m_packetReader;

   /// The number of heartbeats sent so far
   int m_nbrHeartBeat;
   
   /// The total number of requests recieved so far.
   uint32 m_nbrOfReq;
      

   /// Number of votes since startup
   uint32 m_nbrVotes;

   /// Queue where push info is queued by packetreaderthread
   ModulePushList* m_pushList;

   /// Pushservices object if were using push
   PushServices* m_pushServices;

   /// Wanted subscriptions
   vector<PushService*>* m_wantedServices;
      
   /// Current jobthread number that has timed out
   int m_currentJobThreadTimeOutJob;

   /// The load balancer.
   Balancer* m_balancer;

   /// the logging prefix
   char m_logPrefix[32];
   /// start as leader state.
   bool m_startAsLeader;

private:
   /**
    *   Sets the name of the module depending on the
    *   type of module.
    */
   void setModuleName();

   /** 
    *   Is this module the leader? 
    */
   std::auto_ptr<LeaderStatus> m_leaderStatus;

   /// Module type of this module.
   const moduletype_t m_moduleType;

   /// Name of the module type
   MC2String m_moduleTypeName;

   ///  The memory level the module tries not to exceed.
   uint64 m_optMem;
   
   ///  The memory level the module can not exceed.
   uint64 m_maxMem;

   /// Handles what happens when JobThread been working too long.
   std::auto_ptr< JobTimeoutHandler > m_timeoutHandler;
};

// - Implementation of inlined methods
inline const char* 
StandardReader::getName()
{
   return m_moduleTypeName.c_str();
}

#endif // __STANDARDREADER_H__
