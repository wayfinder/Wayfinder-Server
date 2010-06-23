/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REMOVEDISTURBANCEPACKET_H
#define REMOVEDISTURBANCEPACKET_H

#include "Packet.h"
#include "MC2String.h"
#include "TrafficDataTypes.h"
#include "PacketHelpers.h"
#include <vector>
#include <iterator>

/**
  *   Packet used for removing a traffic cost from a segment 
  *
  */
class RemoveDisturbanceRequestPacket : public RequestPacket 
{
public:
   /** @name Constructors and destructor. */
   //@{
   /**
    * Constructor.
    * The constructed requestpacket will remove the disturbance
    * identified by the distrurbanceID argument. 
    * @param distrurbanceID The ID of the distrurbance that will be
    *                       removed.
    */
   RemoveDisturbanceRequestPacket(uint32 disturbanceID);
   /**
    * Constructor.
    * The constructed requestpacket will remove the disturbance
    * identified by the situationReference argument.
    * @param situationReference The reference string of the
    *                           distrurbance that will be removed.
    */
   RemoveDisturbanceRequestPacket(const MC2String& situationReference);
   /**
    * Constructor.
    *
    * The constructed requestpacket will remove all disturbances
    * generated from <code>supplier</code> except for the disturbances
    * listed in <code>toBeKept</code>.
    * @param supplier String that identifies a supplier.
    * @param toBeKept List of situations that should not be removed. 
    */
   RemoveDisturbanceRequestPacket(const MC2String& supplier,
                                  const vector<MC2String>& toBeKept);


   /**
    * Destructor.
    */
   virtual ~RemoveDisturbanceRequestPacket(){};
private:
   /**
    * Encodes the packet from the arguments.
    * @param disturbanceID A 32 bit identifier of the disturbance.
    * @param situations    A string identifier of the disturbance.
    * @param supplier      The supplier name. Used as a prefix in
    *                      situationReference strings.
    * @param removeAll     Whether to remove all disturbances 
    *                      associated with the supplier identified by
    *                      the supplier string.
    * @param toBeKept      A vector of situationReferences of 
    *                      disturbances that should not be removed
    *                      even if matched by the supplier string.
    */
   void encodeRequest(uint32 disturbanceID,
                      MC2String situationReference,
                      MC2String supplier,
                      bool removeAll,
                      vector<MC2String> toBeKept);
   
public:
   /**
    * Decode the packet contents into the output parameters.
    * @param disturbanceID      The disturbanceID.
    * @param situationReference The situationReference.
    * @param supplier           The supplier string. 
    * @param removeAll          The removeAll parameter.
    * @param toBeKept           The situationReferences excepted
    *                           from removeAll. 
    */
   void decodeRequest(uint32 &disturbanceID,
                      MC2String &situationReference,
                      MC2String &supplier,
                      bool &removeAll,
                      vector<MC2String> &toBeKept) const;
};


// Reply packet

/**
 * This class represents a disturbance and the area it covers. It
 * contains the disturbances ID, situationReference, and boundingbox.
 */
class DisturbanceBBox
{
public:
   /** @name Constructors. */
   //@{
   /**
    * Constructor with values. 
    * @param disturbanceID The ID of this disturbance.
    * @param situationReference The situationReference string of this
    *                           disturbance.
    * @param bbox The smallest boundingbox that contains all
    *             coordinates of this disturbance.
    */
   DisturbanceBBox(uint32 disturbanceID, const MC2String& situationReference,
                   const MC2BoundingBox& bbox);
   /**
    * Default constructor. The disturbanceID is set to MAX_UINT32, the
    * situationReference to the empty string and the boundingbox to
    * (0,0,0,0);
    */
   DisturbanceBBox();

   /**
    * Get the number of bytes needed to write this object in a Packet.
    * @return The needed size.
    */
   size_t size() const;
   /**
    * Save this object in a Packet.
    * @param packet The packet to save to. 
    * @param pos In/Out parameter. When the function is called it
    *            should hold the position in the packet where it
    *            should start writing. When the function returns it
    *            will hold the next byt to write at.
    */
   void save(Packet* packet, int& pos) const;
   /**
    * Load this object from a Packet.
    * @param packet The packet to load from. 
    * @param pos In/Out parameter. When the function is called it
    *            should hold the position in the packet where it
    *            should start reading. When the function returns it
    *            will hold the next byt to read at.
    */
   void load(const Packet* packet, int& pos);

   const MC2BoundingBox& getBBox() const;
   const MC2String& getSituationReference() const;
   uint32 getDisturbanceID() const;
private:
   /** The disturbanceID. */
   uint32 m_disturbanceID;
   /** The situationReference. */
   MC2String m_situationReference;
   /**
    * The boundingbox that contains all coordinates that are part of
    * this disturbance.
    */
   MC2BoundingBox m_bbox;
};

ostream& operator<<(ostream& strm, const DisturbanceBBox& bbox);

/**
  *   Packet used for replying to a removetrafficcost request.
  *
  */
class RemoveDisturbanceReplyPacket : public ReplyPacket
{ 
   public:
   /** Constructors and destructor. */
   //@{
   /**
    * Constructor.
    * @param p The request packet.
    * @param status The result of the addition.
    */
   RemoveDisturbanceReplyPacket(const RequestPacket* p,
                                uint32 status);
   /**
    * Constructor with boxes.
    * @param p The request packet.
    * @param status The result of the addition.
    * @param boxes Pointer to an array of DisturbanceBBox objects that
    *              shall be included in the packet.
    * @param boxcount The number of boxes in the <code>boxes</code> array.
    */
   RemoveDisturbanceReplyPacket(const RequestPacket* p,
                                uint32 status,
                                const DisturbanceBBox* boxes, int boxcount);

   /**
    * Virtual destructor.
    */
   virtual ~RemoveDisturbanceReplyPacket();   
   //@}
private:
   /**
    * Add a sequence of boxes to the packet.  Each time this function
    * is called it will overwrite any data written by previous calls.
    * Note that the packet must be large enough to write all the
    * boxes.
    * @param first Start iterator. 
    * @param last  End iterator. 
    */
   template<typename FwIt>
   void addBoxes(FwIt first, FwIt last)
   {
      int pos = REQUEST_HEADER_SIZE;
      // Save the length
      SaveLengthHelper slh( this, pos );
      slh.writeDummyLength( pos );

      typename iterator_traits<FwIt>::difference_type count = 
         std::distance( first, last );
      incWriteLong(pos, count);

      // Write the data
      for(FwIt it = first; it != last; ++it){
         it->save(this, pos);
      }
      
      slh.updateLengthUsingEndPos( pos );
      setLength(pos);
   }

public:
   /**
    * Get the number of DisturbanceBBoxes held by this Packet. 
    * @return The number of boxes.
    */
   int count() const;

   /**
    * Read a sequence of boxes from the packet. 
    * @param dest Output iterator. 
    */
   template<typename OutIt>
   void readBoxes(OutIt dest) const
   {
      int pos = REQUEST_HEADER_SIZE;
      LoadLengthHelper llh( this, pos, "[RemoveDisturbanceReplyPacket]" );
      llh.loadLength( pos );      
      
      // Read the data
      const int count = incReadLong( pos );
      for(int i = 0; i < count; ++i){
         DisturbanceBBox bbox;
         bbox.load(this, pos);
         *dest++ = bbox;
      }
      llh.skipUnknown( pos );   
   }

};

    
#endif   // REMOVEDISTURBANCEPACKET_H
