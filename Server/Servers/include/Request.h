/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REQUEST_H
#define REQUEST_H

#include "config.h"
#include <map>
#include "PacketContainerTree.h"
#include "CCSet.h"
#include "StringTable.h"
#include "RequestData.h"

class PacketContainer;
class RequestWithStatus;
class RequestPacket;

/**
  *   A complex Request object. The Request is built up of a collection
  *   of Packets and intelligence to create the different Packets.
  *
  *   New requests must inherit from RequestWithStatus
  *   instead of Request.
  *
  */
class Request : public Link {
  public:

   typedef vector<uint32> RequestTime;

      /**
       *    Delete this request.
       */
      virtual ~Request();

      /**
       *    Returns a good debugname for the request.
       */
      virtual const char* getName() const;
      
      /**
       *    Returns the user, if any.
       */
      const UserUser* getUser() const;
   
      /**
       *    Get the ID of this request.
       *    @return the unique request ID.
       */
      inline uint16 getID() const;

      /**
       *    Get the timestamp of this request.
       *    @return the timestamp
       */
      inline uint32 getTimestamp() const;

      /**
       *    Get the originator of this request.
       *    @return the originator
       */
      inline uint16 getOriginator() const;

      /**
       *    Get the originator of this request.
       *    @return the unique request ID.
       */
      inline void setOriginator(uint16 originator);

      /**
       *    Get the ID for the next packet.
       *    @return the next available packetID
       */
      inline uint32 getNextPacketID();
      
      /**
        *   Return the next Packet with no unresolved datadependencies
        *   from the list of packets ready to send in m_packetsReadyToSend.
        *   Subclasses may, but doesn't need to, override this method.
        *   The default implementation here is to return a packet from
        *   m_packetsReadyToSend.
        *
        *   @return Packet if available, NULL otherwise
        */
      virtual PacketContainer* getNextPacket();

      /**
        *   Process the data returned by a module.
        *   @param   pack  Packet container with the packet that was
        *                  processed by the modules.
        */
      virtual void processPacket( PacketContainer* pack ) = 0;

      /**
        *   Get the status of the pending request.
        *
        *   @return  True if all packets are processed, false 
        *            otherwise.
        */
      bool virtual requestDone();

      /**
        *   Get the answer from this request, is only used if request
        *   returns a PacketContainer as answer. The default 
        *   implementation returns NULL.
        *
        *   @return  A packet containing the answer to the request.
        */
      virtual PacketContainer* getAnswer();

      /**
        *   Sets the Parent Request for this Request.
        *   The Parent Request's getNextPacketID is used instead of this
        *   Requests getNextPacketID.
        *   @param    parent    The Request to get packetIDs from.
        */
      inline void setParentRequest( Request* parent );

      /**
       *    Increases the number of original packets sent by the request.
       *    (Statistics). Used by RequestHandler.
       *    @return The number of original packets sent by the request.
       */
      inline int incNbrSentPackets();

      /**
       *    Returns the number of packets created by this request.
       *    Updated by RequestHandler.
       *    @return The number of packets sent by this request.
       *            Resent packets are not counted.
       */
      inline int getNbrSentPackets() const;

      /**
       *    Increases the number of packets that had to be resent.
       *    Updated by RequestHandler.
       *    @param number The number to increase the sum with.
       *    @return The new number of resent packets.
       */
      inline int incNbrResentPackets( int number );

      /**
       *    Returns the number of resent packets.
       *    Updated by RequestHandler.
       *    @return The number of resent packets.
       */
      inline int getNbrResentPackets() const;

      /**
       *    Sets the processingtime for the request.
       *    The total processingtime is calculated by adding
       *    up the debinfo of the packets.
       */
      inline void setProcessingTime( uint32 time );

      /**
       *    Returns the processingtime for the request.
       *    The total processing time in the modules for the
       *    request.
       */
      inline uint32 getProcessingTime() const;

   /**
    * Set time spent in a specific function
    * @param function the function name
    * @param timeMs time in milli seconds
    */
   inline void setTime( uint32 requestType, uint32 timeMs );
   /**
    * Add time to a specific function. @see setTime
    * @param function the function name
    * @param timeMs time in milli seconds
    */
   inline void addTime( uint32 requestType, uint32 timeMs );
   /// @return vector of time values 
   inline const RequestTime& getTimes() const;
   /// increase total number of resends from answers
   inline void addTotalResentNbr( uint32 more );
   /// @return total number of resends in answers
   inline uint32 getTotalResendNbr() const;

      /** 
       *    Adds processing time for a module type.
       * 
       */
      inline void addProcessingTime( moduletype_t type,
                                     uint32 time );

      /**
       *    Returns a copy of the moduletype processingtime map.
       *    First is module type, second is processingtime.
       */
      inline map<moduletype_t, uint32> 
         getProcessingTimePerModuleType() const;

      /**
       * Increases the number of packets received by the request.
       * (Statistics). Used by RequestHandler.
       * @return The number of packets received by the request.
       */
      inline int incNbrReceivedPackets();

      /**
       * Returns the number of packets received by this request.
       * Updated by RequestHandler.
       * @return The number of packets received by this request.
       */
      inline int getNbrReceivedPackets() const;

      /**
       * Increases the number of bytes received by the request.
       * (Statistics). Used by RequestHandler.
       *
       * @param bytes The number of bytes to add.
       * @return The number of bytes received by the request.
       */
      inline int addNbrReceivedBytes( uint32 bytes );

      /**
       * Returns the number of bytes received by the request.
       * Updated by RequestHandler.
       *
       * @return The number of bytes received by the request.
       */
      inline int getNbrReceivedBytes() const;

      /**
       *    Returns the number of packets to wait for.
       *    <b>NB!</b> The packetcontainertree must be used for
       *    this to work. (Takes the number of packets in the
       *    queue + the number of sent packets - nbr of received packets).
       */
      inline int getNbrOutstandingPackets() const;

      /**
       *    Puts a packetcontainer into the outgoing packet queueue.
       *    @param pack Packetcontainer to add.
       */
      inline void enqueuePacketContainer(PacketContainer* pack);

      /**
       *    Puts a packet into the outgoing packet queue.
       *    @param pack       The packet.
       *    @param moduleType Type of module to send the packet to. 
       */
      inline void enqueuePacket(Packet* pack, moduletype_t moduleType);

      /**
       *    Calls getNextPacket on the other request until NULL is
       *    returned.
       *    @param req The other request.
       *    @return Number of packets dequeued.
       */
      int enqueuePacketsFromRequest(Request* req);

      /**
       *    Handles the processpacket of a subrequest then calls
       *    enqueuePacketsFromRequest and checks if the other request is
       *    done.
       *    @param req  The other request.
       *    @param pack Packetcontainer to process.
       *    @param status Outparameter. Set if the other request is done.
       *    @return True if the other request is done. Status should be
       *            valid if true is returned.
       */
      bool processSubRequestPacket(RequestWithStatus* req,
                                   PacketContainer* pack,
                                   StringTable::stringCode& status);


      /**
       *    Updates the ID:s of the packet (request ID and packetID).
       *    Use the function like this
       *    <code>enqueuePacket(updatedIDs(pack))</code>
       *    @param packet Packet to update.
       *    @return The same packet, but now it has updated ID:s.
       */
      RequestPacket* updateIDs( RequestPacket* packet );
         
      /**
       *    Updates the ID:s of the packet (request ID and packetID).
       *    Use the function like this
       *    <code>enqueuePacket(updatedIDs(pack))</code>
       *    @param packet Packet to update.
       *    @return The same packet, but now it has updated ID:s.
       */
      Packet* updateIDs( Packet* packet );
      
      /**
       *    Updates the ID:s of the packet (request ID and packetID).
       *    Use the function like this
       *    <code>enqueuePacket(updatedIDs(pack))</code>
       *    @param packet Packet to update.
       *    @return The same packet, but now it has updated ID:s.
       */
      PacketContainer* updateIDs( PacketContainer* pc );
         
   protected:
      /**
       *    Update the status (the done-flag) of this Request to a 
       *    new value (true or false).
       *    @param done The new done status.
       */
      inline void setDone( bool done );

      /**
        *   True when all packets are processed, false otherwise.
        */
      bool m_done; 

      /**
       *    Unique identifier for each packet, used by the 
       *    getNextPacketID() method to get the next ID.
       */
      uint32 m_packetID;

      /**
       * The Parent Request for this Request.
       */
      Request* m_parent;
   private:
      /// The number of packets sent by the request
      int m_nbrSentPackets;

      /// The number of packets received by the request
      int m_nbrReceivedPackets;

      /// The number of bytes received by the request
      int m_nbrReceivedBytes;

      /// The number of resent packets
      int m_nbrResentPackets;

      /// The total processing time in the modules
      uint32 m_moduleProcessingTime;

      /// Processing time per module
      map<moduletype_t, uint32> m_moduleTypeProcessingTime;

   protected:
      /**
       * Containers with the packets that are ready to be processed
       * by the modules. Used by getNextPacket method.
       */
      PacketContainerTree m_packetsReadyToSend;
      
      /// Server can do things.
      friend class Server;

      /// RequestList is a part of Server
      friend class RequestList;

private:
   /// RequestWithStatus must be able to use the constructor
   friend class RequestWithStatus;

   /// AllUserRequest uses deprecated interface.
   friend class AllUserRequest;

   /// CallerInfoRequest uses deprecated interface.
   friend class CallerInfoRequest;

   /// CoordinateOnItemRequest uses deprecated interface.
   friend class CoordinateOnItemRequest;
   
   /// DATEXRequest uses deprecated interface.
   friend class DATEXRequest;

   /// ExpandRouteRequest uses deprecated interface
   friend class ExpandRouteRequest;

   /// GetNavDestRequest uses deprecated interface
   friend class GetNavDestRequest;

   /// GfxRouteRequest uses deprecated interface.
   friend class GfxRouteRequest;

   /// ItemInfoRequest uses deprecated interface.
   friend class ItemInfoRequest;

   /// LocalMapMessageRequest uses deprecated interface.
   friend class LocalMapMessageRequest;

   /// MapRequest uses deprecated interface.
   friend class MapRequest;

   /// MapUpdateRequest uses deprecated interface.
   friend class MapUpdateRequest;

   /// MultiplePacketsRequest uses deprecated interface.
   friend class MultiplePacketsRequest;

   /// MC2BoundingBoxRequest uses deprecated interface.
   friend class MC2BoundingBoxRequest;

   /// PositionUserRequest uses deprecated interface
   friend class PositionUserRequest;

   /// RouteMessageRequest uses deprecated interface
   friend class RouteMessageRequest;

   /// SetNavDestRequest uses deprecated interface.
   friend class SetNavDestRequest;

   /// SinglePacketRequest uses deprecated interface.
   friend class SinglePacketRequest;

   /// SMSSendRequest uses deprecated interface.
   friend class SMSSendRequest;

   /// SMSListenRequest uses deprecated interface.
   friend class SMSListenRequest;

   /// SlimMapRequest uses deprecated interface.
   friend class SlimMapRequest;
   
   /// SlimMapRouteRequest uses deprecated interface.
   friend class SlimMapRouteRequest;

   /// SortDistRequest uses deprecated interface
   friend class SortDistRequest;

   /// StringTableRequest uses deprecated interface.
   friend class StringTableRequest;
   
  protected:
   /// Request id and user data.
   RequestData m_requestData;
   
   /**
    *   Request timestamp, should not be altered by request 
    *   object. Should also not be used by subclasses.
    */
   uint32 m_timestamp;

   /**
    *   Request originator, should not be altered by request 
    *   object. Should also not be used by subclasses.
    */
   uint16 m_originator;
   
   /**
    *    Create a stand-alone request or one with a parent
    *    depending on the contents of requestOrID.
    */
   Request( const RequestPtrOrRequestID& requestOrID );

   /**
    *    Create a request with the supplied requestdata.
    *    (best).
    */
   Request( const RequestData& requestData );


private:

   RequestTime m_timeVector; //< vector with time values of functions
   uint32 m_totalResendNbr; //< accumulated nbr of resends for all packages
};


/**
 *    RequestWithStatus is a Request which
 *    also has a getStatus()-method which is needed
 *    in processSubRequestPacket. This will also make it
 *    easier for the users of the request to know if everything
 *    went OK without the need to do tricks with packets etc.
 *    <br />
 *    New Requests <b>must</b>
 *    implement getStatus and inherit from RequestWithStatus.
 *    <br />
 *    New requests should also be able to be part of another
 *    request, so implement both constructors that takes reqID
 *    and parent request, please.
 */
class RequestWithStatus : public Request {
public:
   
   /**
    *    Create a stand-alone request or one with a parent
    *    depending on the contents of requestOrID.
    *    Recommended constructor.
    */
   RequestWithStatus(
      const RequestData& requestOrID ) : 
            Request(requestOrID), m_isCacheable(true) {}

   /**
    *    Create request to be run inside other request.
    */
   RequestWithStatus( Request* request )
      : Request( RequestData(request) ), m_isCacheable( true ) {}

   
   /**
    *    Delete this request.
    */
   virtual ~RequestWithStatus() {}
   
   /**
    *   Returns the status code of the request.
    *   StringTable::OK means OK.
    */
   virtual StringTable::stringCode getStatus() const = 0;

   /**
    *   Is this request or request result cacheable?
    *   @return true if cacheable, false otherwise
    */
   virtual bool isCacheable() const;
   
   /**
    *    Create a stand-alone request or one with a parent
    *    depending on the contents of requestOrID.
    *    Not the recommended constructor.
    */
   RequestWithStatus(
      const RequestPtrOrRequestID& requestOrID ) : 
            Request(requestOrID), m_isCacheable(true) {}
   

protected:
   /// cacheable flag, used by default implementation of isCacheable
   bool m_isCacheable;
};

//**********************************************************************
// Inlines
//**********************************************************************

inline uint32 
Request::getNextPacketID()
{
   if ( m_parent == NULL ) {
      return m_packetID++;
   } else {
      return m_parent->getNextPacketID();
   }
}


inline uint16 
Request::getID() const
{
   if ( m_parent == NULL ) {
      return m_requestData.getID();
   } else {
      return m_parent->getID();
   }
}

inline uint32 
Request::getTimestamp() const
{
   if ( m_parent == NULL ) {
      return m_timestamp;
   } else {
      return m_parent->getTimestamp();
   }
}

inline uint16 
Request::getOriginator() const
{
   if ( m_parent == NULL ) {
      return m_originator;
   } else {
      return m_parent->getOriginator();
   }
}

inline void
Request::setOriginator(uint16 originator)
{
   if ( m_parent == NULL )
      m_originator = originator;
}

inline void 
Request::setDone( bool done ) {
   m_done = done;
}

inline void 
Request::setParentRequest( Request* parent ) {
   m_parent = parent;
}

inline int
Request::incNbrSentPackets()
{
   return ++m_nbrSentPackets;
}

inline int
Request::getNbrSentPackets() const
{
   return m_nbrSentPackets;
}

inline int
Request::incNbrReceivedPackets()
{
   return ++m_nbrReceivedPackets;
}

inline int
Request::getNbrReceivedPackets() const
{
   return m_nbrReceivedPackets;
}

inline int
Request::addNbrReceivedBytes( uint32 bytes ) {
   return (m_nbrReceivedBytes += bytes);
}

inline int
Request::getNbrReceivedBytes() const
{
   return m_nbrReceivedBytes;
}

inline int
Request::incNbrResentPackets( int number )
{
   m_nbrResentPackets += number;
   return m_nbrResentPackets;
}

inline int
Request::getNbrResentPackets() const
{
   return m_nbrResentPackets;
}

inline void
Request::setProcessingTime( uint32 time )
{
   m_moduleProcessingTime = time;
}

inline uint32
Request::getProcessingTime() const
{
   return m_moduleProcessingTime;
}

inline void
Request::addProcessingTime( moduletype_t type,
                            uint32 time )
{
   m_moduleTypeProcessingTime[type] += time;
}

inline map<moduletype_t, uint32>
Request::getProcessingTimePerModuleType() const
{
   return m_moduleTypeProcessingTime;
}

inline int
Request::getNbrOutstandingPackets() const
{
   int nbr;
   nbr = (m_packetsReadyToSend.getCardinal() + getNbrSentPackets())
             - getNbrReceivedPackets();
   if (m_parent != NULL)
      nbr += m_parent->getNbrOutstandingPackets();
   return nbr;
}

inline void
Request::enqueuePacketContainer(PacketContainer* pack)
{
   m_packetsReadyToSend.add(pack);
}

inline void
Request::enqueuePacket(Packet* pack,
                       moduletype_t moduleType)
{
   enqueuePacketContainer(new PacketContainer(pack,
                                              0,
                                              0,
                                              moduleType));
}

inline void
Request::setTime( uint32 type, uint32 timeMs ) 
{
   if ( type >= m_timeVector.size() ) {
      m_timeVector.resize( type + 1 );
   }

   m_timeVector[ type ] = timeMs;
}

inline void
Request::addTime( uint32 type, uint32 timeMs ) 
{
   if ( type >= m_timeVector.size() ) {
      m_timeVector.resize( type + 1 );
   }

   m_timeVector[ type ] += timeMs;

}

inline const std::vector< uint32 >&
Request::getTimes() const {
   return m_timeVector;
}

inline void
Request::addTotalResentNbr( uint32 more ) {
   m_totalResendNbr += more;
}

inline uint32
Request::getTotalResendNbr() const {
   return m_totalResendNbr;
}

#endif // REQUEST_H

