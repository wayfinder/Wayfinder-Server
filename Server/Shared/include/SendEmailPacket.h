/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SENDEMAILPACKET_H
#define SENDEMAILPACKET_H

#include "Packet.h"
#include "config.h"

/**
 *    A packet to send text (and later mabye pictures etc.) via email.
 *    After the usual header the request packet contains:
 *    @packetdesc
 *       @row 0 @sep 2 bytes @sep To address length @endrow
 *       @row 2 @sep 2 bytes @sep From address length @endrow
 *       @row 4 @sep 2 bytes @sep Subject length @endrow
 *       @row 6 @sep 2 bytes @sep The number of optionalheaderlines @endrow
 *       @row 8 @sep 4 bytes @sep Data length @endrow
 *       @row 12@sep string @sep Recipient email address @endrow
 *       @row x @sep To address length @sep The recipient address @endrow
 *       @row x @sep From address length @sep The from address @endrow
 *       @row x @sep Subject length @sep The subject @endrow
 *       @row x @sep Data length @sep The data (body) of the message 
 *                               @endrow
 *       @row x @sep nbroptionalheaderlines*2 strings @sep 
 *                                          The optionalheaderlines @endrow
 *    @endpacketdesc
 */
class SendEmailRequestPacket : public RequestPacket 
{
   public:
      /**
       *    Create a new SendEmailRequestPacket.
       */
      SendEmailRequestPacket();

      /**
       *    Create a new SendEmailRequestPacket, with a given 
       *    request ID.
       *    @param   requestID   The ID of the request that should have
       *                         the answer of this request packet.
       */
      SendEmailRequestPacket(uint16 requestID);

      /**
       *    Add content to this packet.
       *    @param   adr     The email address of the recipent.
       *    @param   fromAdr The email address in the "from"-field.
       *    @param   subj    The subject of the email.
       *    @param   data    The data that will be sent to the recipent.
       *    @param   nbrOptionalHeaders The number of headerlines in
       *                                optionalHeaderTypes and 
       *                                optionalHeaderValues.
       *    @param   optionalHeaderTypes The types of the optional 
       *                                 headerlines.
       *    @param   optionalHeaderValues The values of the optional 
       *                                  headerlines.
       *    @return  True if the values are inserted into the packet,
       *             false otherwise.
       */
      bool setData(const char* adr, const char* fromAdr,
                   const char* subj, const char* data,
                   uint32 nbrOptionalHeaders = 0, 
                   const char * const * const optionalHeaderTypes = NULL,
                   const char * const * const optionalHeaderValues = NULL);

      /**
       *    Get the length of the email address.
       *    @return  The length (in bytes, including the last NULL_char)
       *             of the email address in bytes.
       */
      inline uint16 getAddressLength() const;

      /**
       *    Get the length of the "from email address".
       *    @return  The length (in bytes, including the last NULL_char)
       *             of the from address in bytes.
       */
      inline uint16 getFromAddressLength() const;

      /**
       *    Get the length of the subject-field.
       *    @return  The length (in bytes, including the last NULL_char)
       *             of the subject field in bytes.
       */
      inline uint16 getSubjectLength() const;

      /**
       *    Get the length of the data (body of the mail-message).
       *    @return  The length (in bytes, including the last NULL_char)
       *             of the datafield in bytes.
       */
      inline uint32 getDataLength() const;

      /**
       *    Get the number of optional header lines.
       *    @return The number of optional header lines.
       */
      inline uint16 getNbrOptionalHeaderLines() const;

      /**
       *    Get the main content in this packet. The strings must be
       *    allocated, and at least have the length of the getAddressLength()
       *    and getDataLength(), when calling this method, 
       *    with the exception of optionalHeaderLines that is allocated by
       *    this method and must be deleted by caller!
       *    
       *    @param   adr   The email address of the recipent is copied into
       *                   this outparameter. Must be allocated by the 
       *                   caller (at least getAddressLength() bytes).
       *    @param   fromAdr  The address of the sender of the email. Must 
       *                   be allocated by the caller (at least 
       *                   getFromAddressLength() bytes).
       *    @param   subject  The subject of the email. Must be allocated 
       *                   by the caller (at least getSubjectLength() bytes).
       *    @param   data  The data that should be sent to the recipent is
       *                   copied into this outparameter. Must be allocated
       *                   by the caller (at least getDataLength() bytes).
       *    @param   nbrOptionalHeaders Set to the number of optional
       *                                header lines.
       *    @param   optionalHeaderTypes Set to a new vector with pointers
       *                                 into the packet, caller must 
       *                                 delete vector but not strings.
       *    @param   optionalHeaderValues Set to a new vector with pointers
       *                                  into the packet, caller must 
       *                                  delete vector but not strings.
       *    @return  True if the values are copied to the outparameters
       *             correctly, false otherwise.
       */
   bool getData( MC2String& adr, MC2String& fromAdr, MC2String& subject,
                 MC2String& data,
                 uint32& nbrOptionalHeaders, char**& optionalHeaderTypes,
                 char**& optionalHeaderValues ) const;
   private:
      /**
       *    @name Positions
       *    Constants describing the positions for the datafields in the 
       *    packet.
       */
      //@{
         /// Addresslength.
         static const uint32 m_addressLengthPos;

         /// From address length.
         static const uint32 m_fromAddressLengthPos;

         /// Subject length.
         static const uint32 m_subjectLengthPos;

         /// Datalength.
         static const uint32 m_dataLengthPos;

         /// Number of optional headerlines.
         static const uint32 m_nbrHeaderLinesPos;

         /// Start of address.
         static const uint32 m_addressStartPos;
      //@}
      
      /**
       *    Set the length of the to email address.
       *    @param l The length (in bytes, including the last NULL_char)
       *             of the email address in bytes.
       */
      inline void setAddressLength(uint16 l);

      /**
       *    Set the length of the from email address.
       *    @param l The length (in bytes, including the last NULL_char)
       *             of the From-email address in bytes.
       */
      inline void setFromAddressLength(uint16 l);

      /**
       *    Set the length of the subject of the email.
       *    @param l The length (in bytes, including the last NULL_char)
       *             of the subject in bytes.
       */
      inline void setSubjectLength(uint16 l);

      /**
       *    Set the length of the data.
       *    @param l The length (in bytes, including the last NULL_char)
       *             of the datafield in bytes.
       */
      inline void setDataLength(uint32 l);

      /**
       *    Set the number of optional header lines.
       *    @param nbr The number of optional header lines to set.
       */
      inline void setNbrOptionalHeaderLines(uint16 nbr);

      /**
       *    Get the start position for the datafield.
       *    @return  The start position for the datafield.
       */
      inline uint32 getDataStartPos() const;
};


/**
 *    The reply of a SendEmailRequestPacket. Only contains the status of
 *    the transaction.
 *
 */
class SendEmailReplyPacket : public ReplyPacket 
{
   public:
      /**
       *    Create a new SendEmailReplyPacket.
       *    @param   p        The request packet that this new packet
       *                      is a reply to.
       *    @param   status   The status of this reply (in terms of the
       *                      StringTable).
       */
      SendEmailReplyPacket(const SendEmailRequestPacket* p,
                           uint32 status);
   private:
      /**
       *    Default constructor, private to avoid usage.
       */
      SendEmailReplyPacket();
};

// =======================================================================
//                                     Implementation of inlined methods =

inline uint16
SendEmailRequestPacket::getAddressLength() const
{
   return (readShort(m_addressLengthPos));
}

inline uint16 
SendEmailRequestPacket::getFromAddressLength() const
{
   return (readShort(m_fromAddressLengthPos));
}

inline uint16 
SendEmailRequestPacket::getSubjectLength() const
{
   return (readShort(m_subjectLengthPos));
}

inline uint32
SendEmailRequestPacket::getDataLength() const
{
   return (readLong(m_dataLengthPos));
}

inline uint16 
SendEmailRequestPacket::getNbrOptionalHeaderLines() const {
   return readShort(m_nbrHeaderLinesPos);
}

inline void
SendEmailRequestPacket::setAddressLength(uint16 l) 
{
   writeShort(m_addressLengthPos, l);
}

inline void
SendEmailRequestPacket::setFromAddressLength(uint16 l)
{
   writeShort(m_fromAddressLengthPos, l);
}

inline void
SendEmailRequestPacket::setSubjectLength(uint16 l)
{
   writeShort(m_subjectLengthPos, l);
}

inline void
SendEmailRequestPacket::setDataLength(uint32 l)
{
   writeLong(m_dataLengthPos, l);
}

inline void 
SendEmailRequestPacket::setNbrOptionalHeaderLines(uint16 nbr) {
   writeShort( m_nbrHeaderLinesPos, nbr );
}

inline uint32 
SendEmailRequestPacket::getDataStartPos() const
{
   return (m_addressStartPos + getAddressLength());
}


#endif
